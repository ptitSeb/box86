#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>

#include "box86context.h"
#include "elfloader.h"
#include "debug.h"
#include "x86trace.h"
#include "x86emu.h"
#include "librarian.h"
#include "bridge.h"
#include "library.h"
#include "callback.h"
#include "wrapper.h"
#include "myfts.h"
#include "threads.h"
#include "x86trace.h"
#include "signals.h"
#include <sys/mman.h>
#include "custommem.h"
#include "threads.h"
#ifdef DYNAREC
#include "dynablock.h"
#include "dynarec/arm_lock_helper.h"
#include "khash.h"

#define USE_MMAP

// init inside dynablocks.c
KHASH_MAP_INIT_INT(dynablocks, dynablock_t*)
static dynablocklist_t*    dynmap[DYNAMAP_SIZE];     // 4G of memory mapped by 4K block
static mmaplist_t          *mmaplist = NULL;
static int                 mmapsize = 0;
static int                 mmapcap = 0;
static kh_dynablocks_t     *dblist_oversized;      // store the list of oversized dynablocks (normal sized are inside mmaplist)
static uintptr_t           *box86_jumptable[JMPTABL_SIZE];
static uintptr_t           box86_jmptbl_default[1<<JMPTABL_SHIFT];
#endif
#define MEMPROT_SHIFT 12
#define MEMPROT_SIZE (1<<(32-MEMPROT_SHIFT))
static uint8_t             memprot[MEMPROT_SIZE] = {0};    // protection flags by 4K block
static int inited = 0;

typedef struct mapmem_s {
    uintptr_t begin, end;
    struct mapmem_s *next;
} mapmem_t;

static mapmem_t *mapmem = NULL;

typedef struct blocklist_s {
    void*               block;
    int                 maxfree;
    size_t              size;
} blocklist_t;

#define MMAPSIZE (256*1024)      // allocate 256kb sized blocks

static pthread_mutex_t     mutex_blocks;
static int                 n_blocks = 0;       // number of blocks for custom malloc
static blocklist_t*        p_blocks = NULL;    // actual blocks for custom malloc

typedef union mark_s {
    struct {
        unsigned int    size:31;
        unsigned int    fill:1;
    };
    uint32_t            x32;
} mark_t;
typedef struct blockmark_s {
    mark_t  prev;
    mark_t  next;
} blockmark_t;

#define NEXT_BLOCK(b) (blockmark_t*)((uintptr_t)(b) + (b)->next.size + sizeof(blockmark_t))
#define PREV_BLOCK(b) (blockmark_t*)(((uintptr_t)(b) - (b)->prev.size) - sizeof(blockmark_t))
#define LAST_BLOCK(b, s) (blockmark_t*)(((uintptr_t)(b)+(s))-sizeof(blockmark_t))

static void printBlock(blockmark_t* b, void* start)
{
    printf_log(LOG_INFO, "========== Block is:\n");
    do {
        printf_log(LOG_INFO, "%c%p, fill=%d, size=0x%x (prev=%d/0x%x)\n", b==start?'*':' ', b, b->next.fill, b->next.size, b->prev.fill, b->prev.size);
        b = NEXT_BLOCK(b);
    } while(b->next.x32);
    printf_log(LOG_INFO, "===================\n");
}

// get first subblock free in block. Return NULL if no block, else first subblock free (mark included), filling size
static void* getFirstBlock(void* block, size_t maxsize, size_t* size, void* start)
{
    // get start of block
    blockmark_t *m = (blockmark_t*)((start)?start:block);
    while(m->next.x32) {    // while there is a subblock
        if(!m->next.fill && m->next.size>=maxsize) {
            *size = m->next.size;
            return m;
        }
        m = NEXT_BLOCK(m);
    }

    return NULL;
}

static size_t getMaxFreeBlock(void* block, size_t block_size, void* start)
{
    // get start of block
    if(start) {
        blockmark_t *m = (blockmark_t*)start;
        int maxsize = 0;
        while(m->next.x32) {    // while there is a subblock
            if(!m->next.fill && m->next.size>maxsize) {
                maxsize = m->next.size;
            }
            m = NEXT_BLOCK(m);
        }
        return (maxsize>=sizeof(blockmark_t))?maxsize:0;
    } else {
        blockmark_t *m = LAST_BLOCK(block, block_size); // start with the end
        int maxsize = 0;
        while(m->prev.x32) {    // while there is a subblock
            if(!m->prev.fill && m->prev.size>maxsize) {
                maxsize = m->prev.size;
                if((uintptr_t)block+maxsize>(uintptr_t)m)
                    return (maxsize>=sizeof(blockmark_t))?maxsize:0; // no block large enough left...
            }
            m = PREV_BLOCK(m);
        }
        return (maxsize>=sizeof(blockmark_t))?maxsize:0;
    }
}

static void* allocBlock(void* block, void *sub, size_t size, void** pstart)
{
    (void)block;

    blockmark_t *s = (blockmark_t*)sub;
    blockmark_t *n = NEXT_BLOCK(s);

    s->next.fill = 1;
    // check if a new mark is worth it
    if(s->next.size>size+2*sizeof(blockmark_t))
        s->next.size = size;
    blockmark_t *m = NEXT_BLOCK(s);   // this is new n
    m->prev.fill = 1;
    if(n!=m) {
        // new mark
        m->prev.size = s->next.size;
        m->next.fill = 0;
        m->next.size = ((uintptr_t)n - (uintptr_t)m) - sizeof(blockmark_t);
        n->prev.fill = 0;
        n->prev.size = m->next.size;
    }

    if(pstart && sub==*pstart) {
        // get the next free block
        m = (blockmark_t*)*pstart;
        while(m->next.fill)
            m = NEXT_BLOCK(m);
        *pstart = (void*)m;
    }
    return (void*)((uintptr_t)sub + sizeof(blockmark_t));
}
static size_t freeBlock(void *block, void* sub, void** pstart)
{
    blockmark_t *m = (blockmark_t*)block;
    blockmark_t *s = (blockmark_t*)sub;
    blockmark_t *n = NEXT_BLOCK(s);
    if(block!=sub)
        m = PREV_BLOCK(s);
    s->next.fill = 0;
    n->prev.fill = 0;
    // check if merge with previous
    if (s->prev.x32 && !s->prev.fill) {
        // remove s...
        m->next.size += s->next.size + sizeof(blockmark_t);
        n->prev.size = m->next.size;
        s = m;
    }
    // check if merge with next
    if(n->next.x32 && !n->next.fill) {
        blockmark_t *n2 = NEXT_BLOCK(n);
        //remove n
        s->next.size += n->next.size + sizeof(blockmark_t);
        n2->prev.size = s->next.size;
    }
    if(pstart && (uintptr_t)*pstart>(uintptr_t)sub) {
        *pstart = (void*)s;
    }
    // return free size at current block (might be bigger)
    return s->next.size;
}
// return 1 if block has been expanded to new size, 0 if not
static int expandBlock(void* block, void* sub, size_t newsize)
{
    (void)block;

    newsize = (newsize+3)&~3;
    blockmark_t *s = (blockmark_t*)sub;
    blockmark_t *n = NEXT_BLOCK(s);
    if(s->next.size>=newsize)
        // big enough, no shrinking...
        return 1;
    if(s->next.fill)
        return 0;   // next block is filled
    // unsigned bitfield of this length gets "promoted" to *signed* int...
    if((size_t)(s->next.size + n->next.size + sizeof(blockmark_t)) < newsize)
        return 0;   // free space too short
    // ok, doing the alloc!
    if((s->next.size+n->next.size+sizeof(blockmark_t))-newsize<2*sizeof(blockmark_t))
        s->next.size += n->next.size+sizeof(blockmark_t);
    else
        s->next.size = newsize+sizeof(blockmark_t);
    blockmark_t *m = NEXT_BLOCK(s);   // this is new n
    m->prev.fill = 1;
    m->prev.size = s->next.size;
    if(n!=m) {
        // new mark
        m->prev.fill = 1;
        m->prev.size = s->next.size;
        m->next.fill = 0;
        m->next.size = (uintptr_t)n - (uintptr_t)m;
        n->prev.fill = 0;
        n->prev.size = m->next.size;
    }
    return 1;
}
// return size of block
static size_t sizeBlock(void* sub)
{
    blockmark_t *s = (blockmark_t*)sub;
    return s->next.size;
}

void* customMalloc(size_t size)
{
    // look for free space
    void* sub = NULL;
    pthread_mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if(p_blocks[i].maxfree>=size) {
            size_t rsize = 0;
            sub = getFirstBlock(p_blocks[i].block, size, &rsize, NULL);
            if(sub) {
                void* ret = allocBlock(p_blocks[i].block, sub, size, NULL);
                if(rsize==p_blocks[i].maxfree)
                    p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size, NULL);
                pthread_mutex_unlock(&mutex_blocks);
                return ret;
            }
        }
    }
    // add a new block
    int i = n_blocks++;
    p_blocks = (blocklist_t*)realloc(p_blocks, n_blocks*sizeof(blocklist_t));
    size_t allocsize = MMAPSIZE;
    if(size+2*sizeof(blockmark_t)>allocsize)
        allocsize = size+2*sizeof(blockmark_t);
    #ifdef USE_MMAP
    void* p = mmap(NULL, allocsize, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    memset(p, 0, allocsize);
    #else
    void* p = calloc(1, allocsize);
    #endif
    p_blocks[i].block = p;
    p_blocks[i].size = allocsize;
    // setup marks
    blockmark_t* m = (blockmark_t*)p;
    m->prev.x32 = 0;
    m->next.fill = 0;
    m->next.size = allocsize-2*sizeof(blockmark_t);
    blockmark_t* n = NEXT_BLOCK(m);
    n->next.x32 = 0;
    n->prev.fill = 0;
    n->prev.size = m->next.size;
    // alloc 1st block
    void* ret  = allocBlock(p_blocks[i].block, p, size, NULL);
    p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size, NULL);
    pthread_mutex_unlock(&mutex_blocks);
    return ret;
}
void* customCalloc(size_t n, size_t size)
{
    size_t newsize = n*size;
    void* ret = customMalloc(newsize);
    memset(ret, 0, newsize);
    return ret;
}
void* customRealloc(void* p, size_t size)
{
    if(!p)
        return customMalloc(size);
    uintptr_t addr = (uintptr_t)p;
    pthread_mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if ((addr>(uintptr_t)p_blocks[i].block) 
         && (addr<((uintptr_t)p_blocks[i].block+p_blocks[i].size))) {
            void* sub = (void*)(addr-sizeof(blockmark_t));
            if(expandBlock(p_blocks[i].block, sub, size)) {
                p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size, NULL);
                pthread_mutex_unlock(&mutex_blocks);
                return p;
            }
            pthread_mutex_unlock(&mutex_blocks);
            void* newp = customMalloc(size);
            memcpy(newp, p, sizeBlock(sub));
            customFree(p);
            return newp;
        }
    }
    pthread_mutex_unlock(&mutex_blocks);
    if(n_blocks)
        dynarec_log(LOG_NONE, "Warning, block %p not found in p_blocks for realloc, malloc'ing again without free\n", (void*)addr);
    return customMalloc(size);
}
void customFree(void* p)
{
    if(!p)
        return;
    uintptr_t addr = (uintptr_t)p;
    pthread_mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if ((addr>(uintptr_t)p_blocks[i].block) 
         && (addr<((uintptr_t)p_blocks[i].block+p_blocks[i].size))) {
            void* sub = (void*)(addr-sizeof(blockmark_t));
            size_t newfree = freeBlock(p_blocks[i].block, sub, NULL);
            if(p_blocks[i].maxfree < newfree) p_blocks[i].maxfree = newfree;
            pthread_mutex_unlock(&mutex_blocks);
            return;
        }
    }
    pthread_mutex_unlock(&mutex_blocks);
    if(n_blocks)
        dynarec_log(LOG_NONE, "Warning, block %p not found in p_blocks for Free\n", (void*)addr);
}

#ifdef DYNAREC
typedef struct mmaplist_s {
    void*               block;
    int                 maxfree;
    size_t              size;
    kh_dynablocks_t*    dblist;
    uint8_t*            helper;
    void*               first;  // first free block, to speed up things
    int                 locked; // don't try to add stuff on locked block
} mmaplist_t;

uintptr_t FindFreeDynarecMap(dynablock_t* db, int size)
{
    // look for free space
    void* sub = NULL;
    for(int i=0; i<mmapsize; ++i) {
        if(mmaplist[i].maxfree>=size+sizeof(blockmark_t) && !mmaplist[i].locked) {
            mmaplist[i].locked = 1;
            size_t rsize = 0;
            sub = getFirstBlock(mmaplist[i].block, size, &rsize, mmaplist[i].first);
            if(sub) {
                uintptr_t ret = (uintptr_t)allocBlock(mmaplist[i].block, sub, size, &mmaplist[i].first);
                if(rsize==mmaplist[i].maxfree) {
                    mmaplist[i].maxfree = getMaxFreeBlock(mmaplist[i].block, mmaplist[i].size, mmaplist[i].first);
                }
                kh_dynablocks_t *blocks = mmaplist[i].dblist;
                if(!blocks) {
                    blocks = mmaplist[i].dblist = kh_init(dynablocks);
                    kh_resize(dynablocks, blocks, 64);
                }
                khint_t k;
                int r;
                k = kh_put(dynablocks, blocks, (uintptr_t)ret, &r);
                kh_value(blocks, k) = db;
                int size255=(size<256)?size:255;
                for(size_t j=0; j<size255; ++j)
                    mmaplist[i].helper[(uintptr_t)ret-(uintptr_t)mmaplist[i].block+j] = j;
                if(size!=size255)
                    memset(&mmaplist[i].helper[(uintptr_t)ret-(uintptr_t)mmaplist[i].block+256], -1, size-255);
                mmaplist[i].locked = 0;
                return ret;
            } else {
                printf_log(LOG_INFO, "BOX86: Warning, sub not found, corrupted mmaplist[%d] info?\n", i);
                if(box86_log >= LOG_DEBUG)
                    printBlock(mmaplist[i].block, mmaplist[i].first);
            }
        }
    }
    return 0;
}

uintptr_t AddNewDynarecMap(dynablock_t* db, int size)
{
    int i = mmapsize++;    // yeah, useful post incrementation
    dynarec_log(LOG_DEBUG, "Ask for DynaRec Block Alloc #%zu/%zu\n", mmapsize, mmapcap);
    if(mmapsize>mmapcap) {
        mmapcap += 32;
        mmaplist = (mmaplist_t*)realloc(mmaplist, mmapcap*sizeof(mmaplist_t));
    }
    #ifndef USE_MMAP
    void *p = NULL;
    if(posix_memalign(&p, box86_pagesize, MMAPSIZE)) {
        dynarec_log(LOG_INFO, "Cannot create memory map of %d byte for dynarec block #%d\n", MMAPSIZE, i);
        --mmapsize;
        return 0;
    }
    mprotect(p, MMAPSIZE, PROT_READ | PROT_WRITE | PROT_EXEC);
    #else
    void* p = mmap(NULL, MMAPSIZE, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if(p==(void*)-1) {
        dynarec_log(LOG_INFO, "Cannot create memory map of %d byte for dynarec block #%d\n", MMAPSIZE, i);
        --mmapsize;
        return 0;
    }
    #endif
    setProtection((uintptr_t)p, MMAPSIZE, PROT_READ | PROT_WRITE | PROT_EXEC);

    mmaplist[i].locked = 1;
    mmaplist[i].block = p;
    mmaplist[i].size = MMAPSIZE;
    mmaplist[i].helper = (uint8_t*)calloc(1, MMAPSIZE);
    mmaplist[i].first = p;
    // setup marks
    blockmark_t* m = (blockmark_t*)p;
    m->prev.x32 = 0;
    m->next.fill = 0;
    m->next.size = MMAPSIZE-2*sizeof(blockmark_t);
    blockmark_t* n = NEXT_BLOCK(m);
    n->next.x32 = 0;
    n->prev.fill = 0;
    n->prev.size = m->next.size;
    // alloc 1st block
    uintptr_t sub  = (uintptr_t)allocBlock(mmaplist[i].block, p, size, &mmaplist[i].first);
    mmaplist[i].maxfree = getMaxFreeBlock(mmaplist[i].block, mmaplist[i].size, mmaplist[i].first);
    kh_dynablocks_t *blocks = mmaplist[i].dblist = kh_init(dynablocks);
    kh_resize(dynablocks, blocks, 64);
    khint_t k;
    int ret;
    k = kh_put(dynablocks, blocks, (uintptr_t)sub, &ret);
    kh_value(blocks, k) = db;
    for(int j=0; j<size; ++j)
        mmaplist[i].helper[(uintptr_t)sub-(uintptr_t)mmaplist[i].block + j] = (j<256)?j:255;
    mmaplist[i].locked = 0;
    return sub;
}

void ActuallyFreeDynarecMap(dynablock_t* db, uintptr_t addr, int size)
{
    if(!addr || !size)
        return;
    for(int i=0; i<mmapsize; ++i) {
        if ((addr>(uintptr_t)mmaplist[i].block) 
         && (addr<((uintptr_t)mmaplist[i].block+mmaplist[i].size))) {
            void* sub = (void*)(addr-sizeof(blockmark_t));
            size_t newfree = freeBlock(mmaplist[i].block, sub, &mmaplist[i].first);
            if(mmaplist[i].maxfree < newfree) mmaplist[i].maxfree = newfree;
            kh_dynablocks_t *blocks = mmaplist[i].dblist;
            if(blocks) {
                khint_t k = kh_get(dynablocks, blocks, (uintptr_t)sub);
                if(k!=kh_end(blocks))
                    kh_del(dynablocks, blocks, k);
                memset(&mmaplist[i].helper[(uintptr_t)sub-(uintptr_t)mmaplist[i].block], 0, size);
            }
            if(mmaplist[i].locked) {
                printf_log(LOG_INFO, "BOX86: Warning, Free a chunk in a locked mmaplist[%d]\n", i);
                ++mmaplist[i].locked;
            }
            return;
        }
    }
    if(mmapsize)
        dynarec_log(LOG_NONE, "Warning, block %p (size %d) not found in mmaplist for Free\n", (void*)addr, size);
}

dynablock_t* FindDynablockFromNativeAddress(void* addr)
{
    // look in actual list
    for(int i=0; i<mmapsize; ++i) {
        if ((uintptr_t)addr>=(uintptr_t)mmaplist[i].block 
        && ((uintptr_t)addr<(uintptr_t)mmaplist[i].block+mmaplist[i].size)) {
            if(!mmaplist[i].helper)
                return FindDynablockDynablocklist(addr, mmaplist[i].dblist);
            else {
                uintptr_t p = (uintptr_t)addr - (uintptr_t)mmaplist[i].block;
                while(mmaplist[i].helper[p]) p -= mmaplist[i].helper[p];
                khint_t k = kh_get(dynablocks, mmaplist[i].dblist, (uintptr_t)mmaplist[i].block + p);
                if(k!=kh_end(mmaplist[i].dblist))
                    return kh_value(mmaplist[i].dblist, k);
                return NULL;
            }
        }
    }
    // look in oversized
    return FindDynablockDynablocklist(addr, dblist_oversized);
}

uintptr_t AllocDynarecMap(dynablock_t* db, int size)
{
    if(!size)
        return 0;
    if(size>MMAPSIZE-2*sizeof(blockmark_t)) {
        #ifndef USE_MMAP
        void *p = NULL;
        if(posix_memalign(&p, box86_pagesize, size)) {
            dynarec_log(LOG_INFO, "Cannot create dynamic map of %d bytes\n", size);
            return 0;
        }
        mprotect(p, size, PROT_READ | PROT_WRITE | PROT_EXEC);
        #else
        void* p = mmap(NULL, size, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
        if(p==(void*)-1) {
            dynarec_log(LOG_INFO, "Cannot create dynamic map of %d bytes\n", size);
            return 0;
        }
        #endif
        setProtection((uintptr_t)p, size, PROT_READ | PROT_WRITE | PROT_EXEC);
        kh_dynablocks_t *blocks = dblist_oversized;
        if(!blocks) {
            blocks = dblist_oversized = kh_init(dynablocks);
            kh_resize(dynablocks, blocks, 64);
        }
        khint_t k;
        int ret;
        k = kh_put(dynablocks, blocks, (uintptr_t)p, &ret);
        kh_value(blocks, k) = db;
        return (uintptr_t)p;
    }
    
    uintptr_t ret = FindFreeDynarecMap(db, size);
    if(!ret)
        ret = AddNewDynarecMap(db, size);

    return ret;
}

void FreeDynarecMap(dynablock_t* db, uintptr_t addr, uint32_t size)
{
    if(!addr || !size)
        return;
    if(size>MMAPSIZE-2*sizeof(blockmark_t)) {
        #ifndef USE_MMAP
        free((void*)addr);
        #else
        munmap((void*)addr, size);
        #endif
        kh_dynablocks_t *blocks = dblist_oversized;
        if(blocks) {
            khint_t k = kh_get(dynablocks, blocks, addr);
            if(k!=kh_end(blocks))
                kh_del(dynablocks, blocks, k);
        }
        return;
    }
    ActuallyFreeDynarecMap(db, addr, size);
}

dynablocklist_t* getDB(uintptr_t idx)
{
    return dynmap[idx];
}

// each dynmap is 64k of size

void addDBFromAddressRange(uintptr_t addr, uintptr_t size)
{
    dynarec_log(LOG_DEBUG, "addDBFromAddressRange %p -> %p\n", (void*)addr, (void*)(addr+size-1));
    uintptr_t idx = (addr>>DYNAMAP_SHIFT);
    uintptr_t end = ((addr+size-1)>>DYNAMAP_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        if(!dynmap[i]) {
            dynablocklist_t* p = NewDynablockList(i<<DYNAMAP_SHIFT, 1<<DYNAMAP_SHIFT, 0);
            if(arm_lock_storeifnull(&dynmap[i], p)!=p)
                FreeDynablockList(&p);
        }
    }
}

void cleanDBFromAddressRange(uintptr_t addr, uintptr_t size, int destroy)
{
    dynarec_log(LOG_DEBUG, "cleanDBFromAddressRange %p -> %p %s\n", (void*)addr, (void*)(addr+size-1), destroy?"destroy":"mark");
    uintptr_t idx = (addr>>DYNAMAP_SHIFT);
    uintptr_t end = ((addr+size-1)>>DYNAMAP_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        dynablocklist_t* dblist = dynmap[i];
        if(dblist) {
            if(destroy)
                FreeRangeDynablock(dblist, addr, size);
            else
                MarkRangeDynablock(dblist, addr, size);
        }
    }
}

#ifdef ARM
void arm_next(void);
#endif

void addJumpTableIfDefault(void* addr, void* jmp)
{
    const uintptr_t idx = ((uintptr_t)addr>>JMPTABL_SHIFT);
    if(box86_jumptable[idx] == box86_jmptbl_default) {
        uintptr_t* tbl = (uintptr_t*)malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = (uintptr_t)arm_next;
        if(arm_lock_storeifref(&box86_jumptable[idx], tbl, box86_jmptbl_default)!=tbl)
            free(tbl);
    }
    const uintptr_t off = (uintptr_t)addr&((1<<JMPTABL_SHIFT)-1);
    arm_lock_storeifref(&box86_jumptable[idx][off], jmp, arm_next);
}
void setJumpTableDefault(void* addr)
{
    const uintptr_t idx = ((uintptr_t)addr>>JMPTABL_SHIFT);
    if(box86_jumptable[idx] == box86_jmptbl_default) {
        return;
    }
    const uintptr_t off = (uintptr_t)addr&((1<<JMPTABL_SHIFT)-1);
    box86_jumptable[idx][off] = (uintptr_t)arm_next;
}
int isJumpTableDefault(void* addr)
{
    const uintptr_t idx = ((uintptr_t)addr>>JMPTABL_SHIFT);
    if(box86_jumptable[idx] == box86_jmptbl_default) {
        return 1;
    }
    const uintptr_t off = (uintptr_t)addr&((1<<JMPTABL_SHIFT)-1);
    return (box86_jumptable[idx][off]==(uintptr_t)arm_next)?1:0;
}
uintptr_t getJumpTable()
{
    return (uintptr_t)box86_jumptable;
}

uintptr_t getJumpTableAddress(uintptr_t addr)
{
    const uintptr_t idx = ((uintptr_t)addr>>JMPTABL_SHIFT);
    if(box86_jumptable[idx] == box86_jmptbl_default) {
        uintptr_t* tbl = (uintptr_t*)malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = (uintptr_t)arm_next;
        if(arm_lock_storeifref(&box86_jumptable[idx], tbl, box86_jmptbl_default)!=tbl)
            free(tbl);
    }
    const uintptr_t off = (uintptr_t)addr&((1<<JMPTABL_SHIFT)-1);
    return (uintptr_t)&box86_jumptable[idx][off];
}

// Remove the Write flag from an adress range, so DB can be executed
// no log, as it can be executed inside a signal handler
void protectDB(uintptr_t addr, uintptr_t size)
{
    dynarec_log(LOG_DEBUG, "protectDB %p -> %p\n", (void*)addr, (void*)(addr+size-1));
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        uint32_t prot = memprot[i];
        if(!(prot&PROT_DYNAREC)) {
            if(!prot)
                prot = PROT_READ | PROT_WRITE;    // comes from malloc & co, so should not be able to execute
            memprot[i] = prot|PROT_DYNAREC;
            mprotect((void*)(i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, prot&~PROT_WRITE);
        }
    }
}

// Add the Write flag from an adress range, and mark all block as dirty
// no log, as it can be executed inside a signal handler
void unprotectDB(uintptr_t addr, uintptr_t size)
{
    dynarec_log(LOG_DEBUG, "unprotectDB %p -> %p\n", (void*)addr, (void*)(addr+size-1));
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        uint32_t prot = memprot[i];
        if(prot&PROT_DYNAREC) {
            memprot[i] = prot&~PROT_DYNAREC;
            mprotect((void*)(i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, prot&~PROT_DYNAREC);
            cleanDBFromAddressRange((i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, 0);
        }
    }
}

#endif

void printMapMem()
{
    mapmem_t* m = mapmem;
    while(m) {
        printf_log(LOG_INFO, " %p-%p\n", (void*)m->begin, (void*)m->end);
        m = m->next;
    }
}

void addMapMem(uintptr_t begin, uintptr_t end)
{
    begin &=~0xfff;
    end = (end&~0xfff)+0xfff; // full page
    // sanitize values
    if(end<0x10000) return;
    if(!begin) begin = 0x10000;
    // find attach point (cannot be the 1st one by construction)
    mapmem_t* m = mapmem;
    while(m->next && begin>m->next->begin) {
        m = m->next;
    }
    // attach at the end of m
    mapmem_t* newm;
    if(m->end>=begin-1) {
        if(end<=m->end)
            return; // zone completly inside current block, nothing to do
        m->end = end;   // enlarge block
        newm = m;
    } else {
    // create a new block
        newm = (mapmem_t*)calloc(1, sizeof(mapmem_t));
        newm->next = m->next;
        newm->begin = begin;
        newm->end = end;
        m->next = newm;
    }
    while(newm->next && (newm->next->begin-1)<=newm->end) {
        // fuse with next
        if(newm->next->end>newm->end)
            newm->end = newm->next->end;
        mapmem_t* tmp = newm->next;
        newm->next = tmp->next;
        free(tmp);
    }
    // all done!
}
void removeMapMem(uintptr_t begin, uintptr_t end)
{
    begin &=~0xfff;
    end = (end&~0xfff)+0xfff; // full page
    // sanitize values
    if(end<0x10000) return;
    if(!begin) begin = 0x10000;
    mapmem_t* m = mapmem, *prev = NULL;
    while(m) {
        // check if block is beyond the zone to free
        if(m->begin > end)
            return;
        // check if the block is completly inside the zone to free
        if(m->begin>=begin && m->end<=end) {
            // just free the block
            mapmem_t *tmp = m;
            if(prev) {
                prev->next = m->next;
                m = prev;
            } else {
                mapmem = m->next; // change attach, but should never happens
                m = mapmem;
                prev = NULL;
            }
            free(tmp);
        } else if(begin>m->begin && end<m->end) { // the zone is totaly inside the block => split it!
            mapmem_t* newm = (mapmem_t*)calloc(1, sizeof(mapmem_t));    // create a new "next"
            newm->end = m->end;
            m->end = begin - 1;
            newm->begin = end + 1;
            newm->next = m->next;
            m->next = newm;
            // nothing more to free
            return;
        } else if(begin>m->begin && begin<m->end) { // free the tail of the block
            m->end = begin - 1;
        } else if(end>m->begin && end<m->end) { // free the head of the block
            m->begin = end + 1;
        }
        prev = m;
        m = m->next;
    }
}

void updateProtection(uintptr_t addr, uintptr_t size, uint32_t prot)
{
    addMapMem(addr, addr+size-1);
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    const uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        uint32_t dyn=(memprot[i]&PROT_DYNAREC);
        if(dyn && (prot&PROT_WRITE))    // need to remove the write protection from this block
            mprotect((void*)(i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, prot&~PROT_WRITE);
        memprot[i] = prot|dyn;
    }
}

void forceProtection(uintptr_t addr, uintptr_t size, uint32_t prot)
{
    addMapMem(addr, addr+size-1);
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    const uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        mprotect((void*)(i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, prot&~PROT_DYNAREC);
        memprot[i] = prot;
    }
}

void setProtection(uintptr_t addr, uintptr_t size, uint32_t prot)
{
    addMapMem(addr, addr+size-1);
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    const uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        memprot[i] = prot;
    }
}

void freeProtection(uintptr_t addr, uintptr_t size)
{
    removeMapMem(addr, addr+size-1);
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    const uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        memprot[i] = 0;
    }
}

uint32_t getProtection(uintptr_t addr)
{
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uint32_t ret = memprot[idx];
    return ret;
}

void allocProtection(uintptr_t addr, uintptr_t size, uint32_t prot)
{
    addMapMem(addr, addr+size-1);
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    const uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        if(!memprot[i])
            memprot[i] = prot;
    }
}

void loadProtectionFromMap()
{
    if(box86_mapclean)
        return;
    char buf[500];
    FILE *f = fopen("/proc/self/maps", "r");
    if(!f)
        return;
    while(!feof(f)) {
        char* ret = fgets(buf, sizeof(buf), f);
        (void)ret;
        char r, w, x;
        uintptr_t s, e;
        if(sscanf(buf, "%lx-%lx %c%c%c", &s, &e, &r, &w, &x)==5) {
            int prot = ((r=='r')?PROT_READ:0)|((w=='w')?PROT_WRITE:0)|((x=='x')?PROT_EXEC:0);
            allocProtection(s, e-s, prot);
        }
    }
    fclose(f);
    box86_mapclean = 1;
}

#define LOWEST (void*)0x10000
void* findBlockNearHint(void* hint, size_t size)
{
    mapmem_t* m = mapmem;
    while(m) {
        // granularity 0x10000
        uintptr_t addr = (m->end+1+0xffff)&~0xffff;
        uintptr_t end = (m->next)?(m->next->begin-1):0xffffffff;
        // check hint and availble saize
        if(addr>=(uintptr_t)hint && end-addr+1>=size)
            return (void*)addr;
        m = m->next;
    }
    return hint;
}
void* find32bitBlock(size_t size)
{
    return findBlockNearHint(LOWEST, size);
}

int unlockCustommemMutex()
{
    int ret = 0;
    int i = 0;
    #define GO(A, B)                    \
        i = checkUnlockMutex(&A);       \
        if(i) {                         \
            ret|=(1<<B);                \
        }
    GO(mutex_blocks, 0)
    #undef GO
    return ret;
}

void relockCustommemMutex(int locks)
{
    #define GO(A, B)                    \
        if(locks&(1<<B))                \
            pthread_mutex_lock(&A);     \

    GO(mutex_blocks, 0)
    #undef GO
}

static void init_mutexes(void)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&mutex_blocks, &attr);

    pthread_mutexattr_destroy(&attr);
}

static void atfork_child_custommem(void)
{
    // (re))init mutex if it was lock before the fork
    init_mutexes();
}

void init_custommem_helper(box86context_t* ctx)
{
    if(inited) // already initialized
        return;
    inited = 1;
    init_mutexes();
#ifdef DYNAREC
#ifdef ARM
    for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
        box86_jmptbl_default[i] = (uintptr_t)arm_next;
    for(int i=0; i<JMPTABL_SIZE; ++i)
        box86_jumptable[i] = box86_jmptbl_default;
#else
#error Unsupported architecture!
#endif
#endif
    pthread_atfork(NULL, NULL, atfork_child_custommem);
    // init mapmem list
    mapmem = (mapmem_t*)calloc(1, sizeof(mapmem_t));
    mapmem->begin = 0x0;
    mapmem->end = (uintptr_t)LOWEST - 1;
    loadProtectionFromMap();
}

void fini_custommem_helper(box86context_t *ctx)
{
    if(!inited)
        return;
    inited = 0;
#ifdef DYNAREC
    if(box86_dynarec) {
        dynarec_log(LOG_DEBUG, "Free global Dynarecblocks\n");
        for (int i=0; i<mmapsize; ++i) {
            if(mmaplist[i].block)
                #ifdef USE_MMAP
                munmap(mmaplist[i].block, mmaplist[i].size);
                #else
                free(mmaplist[i].block);
                #endif
            if(mmaplist[i].dblist) {
                kh_destroy(dynablocks, mmaplist[i].dblist);
                mmaplist[i].dblist = NULL;
            }
            if(mmaplist[i].helper) {
                free(mmaplist[i].helper);
                mmaplist[i].helper = NULL;
            }
        }
        if(dblist_oversized) {
            kh_destroy(dynablocks, dblist_oversized);
            dblist_oversized = NULL;
        }
        mmapsize = 0;
        mmapcap = 0;
        dynarec_log(LOG_DEBUG, "Free dynamic Dynarecblocks\n");
        uintptr_t idx = 0;
        uintptr_t end = ((0xFFFFFFFF)>>DYNAMAP_SHIFT);
        for (uintptr_t i=idx; i<=end; ++i)
            if(dynmap[i])
                FreeDynablockList(&dynmap[i]);
        free(mmaplist);
        for (int i=0; i<DYNAMAP_SIZE; ++i)
            if(box86_jumptable[i]!=box86_jmptbl_default)
                free(box86_jumptable[i]);
    }
#endif
    for(int i=0; i<n_blocks; ++i)
        #ifdef USE_MMAP
        munmap(p_blocks[i].block, p_blocks[i].size);
        #else
        free(p_blocks[i].block);
        #endif
    free(p_blocks);
    pthread_mutex_destroy(&mutex_blocks);
    while(mapmem) {
        mapmem_t *tmp = mapmem;
        mapmem = mapmem->next;
        free(tmp);
    }
}
