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
//#define USE_MMAP_MORE

// init inside dynablocks.c
KHASH_MAP_INIT_INT(dynablocks, dynablock_t*)
static dynablocklist_t*    dynmap[DYNAMAP_SIZE] = {0};     // 4G of memory mapped by 4K block
static mmaplist_t          *mmaplist = NULL;
static int                 mmapsize = 0;
static kh_dynablocks_t     *dblist_oversized;      // store the list of oversized dynablocks (normal sized are inside mmaplist)
static uintptr_t           *box86_jumptable[JMPTABL_SIZE];
static uintptr_t           box86_jmptbl_default[1<<JMPTABL_SHIFT];
// lock addresses
KHASH_SET_INIT_INT(lockaddress)
static kh_lockaddress_t    *lockaddress = NULL;
#endif
#define MEMPROT_SHIFT 12
#define MEMPROT_SIZE (1<<(32-MEMPROT_SHIFT))
static uint8_t             memprot[MEMPROT_SIZE] = {0};    // protection flags by 4K block
#ifdef DYNAREC
static uint8_t             hotpages[MEMPROT_SIZE] = {0};
#endif
static int inited = 0;

#define PROT_LOCK()     mutex_lock(&mutex_blocks)
#define PROT_UNLOCK()   mutex_unlock(&mutex_blocks)
#define PROT_GET(A)     memprot[A]
#define PROT_SET(A, B)  memprot[A] = B
#define PROT_SET_IF_0(A, B) if(!memprot[A]) memprot[A] = B

typedef struct mapmem_s {
    uintptr_t begin, end;
    struct mapmem_s *next;
} mapmem_t;

static mapmem_t *mapmem = NULL;

typedef struct blocklist_s {
    void*               block;
    size_t              maxfree;
    size_t              size;
} blocklist_t;

#define MMAPSIZE (256*1024)      // allocate 256kb sized blocks

#ifndef DYNAREC
static pthread_mutex_t     mutex_blocks;
#else
static uint32_t            mutex_blocks;
#endif
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

#ifdef DYNAREC
static void printBlock(blockmark_t* b, void* start)
{
    printf_log(LOG_INFO, "========== Block is:\n");
    do {
        printf_log(LOG_INFO, "%c%p, fill=%d, size=0x%x (prev=%d/0x%x)\n", b==start?'*':' ', b, b->next.fill, b->next.size, b->prev.fill, b->prev.size);
        b = NEXT_BLOCK(b);
    } while(b->next.x32);
    printf_log(LOG_INFO, "===================\n");
}
#endif

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
        size_t maxsize = 0;
        while(m->next.x32) {    // while there is a subblock
            if(!m->next.fill && m->next.size>maxsize) {
                maxsize = m->next.size;
            }
            m = NEXT_BLOCK(m);
        }
        return (maxsize>=sizeof(blockmark_t))?maxsize:0;
    } else {
        blockmark_t *m = LAST_BLOCK(block, block_size); // start with the end
        size_t maxsize = 0;
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
    mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if(p_blocks[i].maxfree>=size) {
            size_t rsize = 0;
            sub = getFirstBlock(p_blocks[i].block, size, &rsize, NULL);
            if(sub) {
                void* ret = allocBlock(p_blocks[i].block, sub, size, NULL);
                if(rsize==p_blocks[i].maxfree)
                    p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size, NULL);
                mutex_unlock(&mutex_blocks);
                return ret;
            }
        }
    }
    // add a new block
    int i = n_blocks++;
    p_blocks = (blocklist_t*)box_realloc(p_blocks, n_blocks*sizeof(blocklist_t));
    size_t allocsize = MMAPSIZE;
    if(size+2*sizeof(blockmark_t)>allocsize)
        allocsize = size+2*sizeof(blockmark_t);
    #ifdef USE_MMAP_MORE
    void* p = mmap(NULL, allocsize, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    memset(p, 0, allocsize);
    setProtection((uintptr_t)p, allocsize, PROT_READ|PROT_WRITE);
    #else
    void* p = box_calloc(1, allocsize);
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
    mutex_unlock(&mutex_blocks);
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
    mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if ((addr>(uintptr_t)p_blocks[i].block) 
         && (addr<((uintptr_t)p_blocks[i].block+p_blocks[i].size))) {
            void* sub = (void*)(addr-sizeof(blockmark_t));
            if(expandBlock(p_blocks[i].block, sub, size)) {
                p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size, NULL);
                mutex_unlock(&mutex_blocks);
                return p;
            }
            mutex_unlock(&mutex_blocks);
            void* newp = customMalloc(size);
            memcpy(newp, p, sizeBlock(sub));
            customFree(p);
            return newp;
        }
    }
    mutex_unlock(&mutex_blocks);
    if(n_blocks)
        dynarec_log(LOG_NONE, "Warning, block %p not found in p_blocks for realloc, malloc'ing again without free\n", (void*)addr);
    return customMalloc(size);
}
void customFree(void* p)
{
    if(!p)
        return;
    uintptr_t addr = (uintptr_t)p;
    mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if ((addr>(uintptr_t)p_blocks[i].block) 
         && (addr<((uintptr_t)p_blocks[i].block+p_blocks[i].size))) {
            void* sub = (void*)(addr-sizeof(blockmark_t));
            size_t newfree = freeBlock(p_blocks[i].block, sub, NULL);
            if(p_blocks[i].maxfree < newfree) p_blocks[i].maxfree = newfree;
            mutex_unlock(&mutex_blocks);
            return;
        }
    }
    mutex_unlock(&mutex_blocks);
    if(n_blocks)
        dynarec_log(LOG_NONE, "Warning, block %p not found in p_blocks for Free\n", (void*)addr);
}

#ifdef DYNAREC
typedef struct mmapchunk_s {
    void*               block;
    int                 maxfree;
    size_t              size;
    kh_dynablocks_t*    dblist;
    uint8_t*            helper;
    void*               first;  // first free block, to speed up things
    uint8_t             lock;   // don't try to add stuff on locked block
} mmapchunk_t;
#define NCHUNK          64
typedef struct mmaplist_s {
    mmapchunk_t         chunks[NCHUNK];
    mmaplist_t*         next;
} mmaplist_t;

mmapchunk_t* addChunk(int mmapsize) {
    if(!mmaplist)
        mmaplist = (mmaplist_t*)box_calloc(1, sizeof(mmaplist_t));
    mmaplist_t* head = mmaplist;
    int i = mmapsize;
    while(1) {
        if(i>=NCHUNK) {
            i-=NCHUNK;
            if(!head->next) {
                head->next = (mmaplist_t*)box_calloc(1, sizeof(mmaplist_t));
            }
            head=head->next;
        } else
            return &head->chunks[i];
    }
}

uintptr_t FindFreeDynarecMap(dynablock_t* db, int size)
{
    // look for free space
    void* sub = NULL;
    mmaplist_t* head = mmaplist;
    int i = mmapsize;
    while(head) {
        const int n = (i>NCHUNK)?NCHUNK:i;
        i-=n;
        for(int i=0; i<n; ++i) {
            mmapchunk_t* chunk = &head->chunks[i];
            if(chunk->maxfree>=size+sizeof(blockmark_t) && !arm_lock_incif0b(&chunk->lock)) {
                size_t rsize = 0;
                sub = getFirstBlock(chunk->block, size, &rsize, chunk->first);
                if(sub) {
                    uintptr_t ret = (uintptr_t)allocBlock(chunk->block, sub, size, &chunk->first);
                    if(rsize==chunk->maxfree) {
                        chunk->maxfree = getMaxFreeBlock(chunk->block, chunk->size, chunk->first);
                    }
                    kh_dynablocks_t *blocks = chunk->dblist;
                    if(!blocks) {
                        blocks = chunk->dblist = kh_init(dynablocks);
                        kh_resize(dynablocks, blocks, 64);
                    }
                    khint_t k;
                    int r;
                    k = kh_put(dynablocks, blocks, (uintptr_t)ret, &r);
                    kh_value(blocks, k) = db;
                    int size255=(size<256)?size:255;
                    for(size_t j=0; j<size255; ++j)
                        chunk->helper[(uintptr_t)ret-(uintptr_t)(chunk->block)+j] = j;
                    if(size!=size255)
                        memset(&chunk->helper[(uintptr_t)ret-(uintptr_t)(chunk->block)+256], -1, size-255);
                    arm_lock_decifnot0b(&chunk->lock);
                    return ret;
                } else {
                    printf_log(LOG_INFO, "BOX86: Warning, sub not found, corrupted %p->chunk[%d] info?\n", head, i);
                    arm_lock_decifnot0b(&chunk->lock);
                    if(box86_log >= LOG_DEBUG)
                        printBlock(chunk->block, chunk->first);
                }
            }
        }
        head = head->next;
    }
    return 0;
}

uintptr_t AddNewDynarecMap(dynablock_t* db, int size)
{
    dynarec_log(LOG_DEBUG, "Ask for DynaRec Block Alloc #%zu\n", mmapsize);
    mmapchunk_t* chunk = addChunk(mmapsize++);
    arm_lock_incb(&chunk->lock);
    #ifndef USE_MMAP
    void *p = NULL;
    if(posix_memalign(&p, box86_pagesize, MMAPSIZE)) {
        dynarec_log(LOG_INFO, "Cannot create memory map of %d byte for dynarec block #%d\n", MMAPSIZE, i);
        arm_lock_storeb(&chunk->lock, 0);
        --mmapsize;
        return 0;
    }
    mprotect(p, MMAPSIZE, PROT_READ | PROT_WRITE | PROT_EXEC);
    #else
    void* p = mmap(NULL, MMAPSIZE, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if(p==(void*)-1) {
        dynarec_log(LOG_INFO, "Cannot create memory map of %d byte for dynarec block #%d\n", MMAPSIZE, mmapsize-1);
        arm_lock_storeb(&chunk->lock, 0);
        --mmapsize;
        return 0;
    }
    #endif
    setProtection((uintptr_t)p, MMAPSIZE, PROT_READ | PROT_WRITE | PROT_EXEC);

    chunk->block = p;
    chunk->size = MMAPSIZE;
    chunk->helper = (uint8_t*)box_calloc(1, MMAPSIZE);
    chunk->first = p;
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
    uintptr_t sub  = (uintptr_t)allocBlock(chunk->block, p, size, &chunk->first);
    chunk->maxfree = getMaxFreeBlock(chunk->block, chunk->size, chunk->first);
    kh_dynablocks_t *blocks = chunk->dblist = kh_init(dynablocks);
    kh_resize(dynablocks, blocks, 64);
    khint_t k;
    int ret;
    k = kh_put(dynablocks, blocks, (uintptr_t)sub, &ret);
    kh_value(blocks, k) = db;
    for(int j=0; j<size; ++j)
        chunk->helper[(uintptr_t)sub-(uintptr_t)chunk->block + j] = (j<256)?j:255;
    arm_lock_decifnot0b(&chunk->lock);
    return sub;
}

void ActuallyFreeDynarecMap(dynablock_t* db, uintptr_t addr, int size)
{
    if(!addr || !size)
        return;
    mmaplist_t* head = mmaplist;
    int i = mmapsize;
    while(head) {
        const int n = (i>NCHUNK)?NCHUNK:i;
        i-=n;
        for(int i=0; i<n; ++i) {
            mmapchunk_t* chunk = &head->chunks[i];
            if ((addr>(uintptr_t)(chunk->block)) 
            && (addr<((uintptr_t)(chunk->block)+chunk->size))) {
                int loopedwait = 256;
                while (arm_lock_incif0b(&chunk->lock) && loopedwait) {
                    sched_yield();
                    --loopedwait;
                }
                if(!loopedwait) {
                    printf_log(LOG_INFO, "BOX86: Warning, Free a chunk in a locked mmaplist[%d]\n", i);
                    //arm_lock_incb(&chunk->lock);
                    if(cycle_log)
                        print_cycle_log(LOG_INFO);
                }
                void* sub = (void*)(addr-sizeof(blockmark_t));
                size_t newfree = freeBlock(chunk->block, sub, &chunk->first);
                if(chunk->maxfree < newfree) chunk->maxfree = newfree;
                kh_dynablocks_t *blocks = chunk->dblist;
                if(blocks) {
                    khint_t k = kh_get(dynablocks, blocks, (uintptr_t)sub);
                    if(k!=kh_end(blocks))
                        kh_del(dynablocks, blocks, k);
                    memset(&chunk->helper[(uintptr_t)sub-(uintptr_t)chunk->block], 0, size);
                }
                arm_lock_decifnot0b(&chunk->lock);
                return;
            }
        }
        head = head->next;
    }
    if(mmapsize)
        dynarec_log(LOG_NONE, "Warning, block %p (size %d) not found in mmaplist for Free\n", (void*)addr, size);
}

dynablock_t* FindDynablockFromNativeAddress(void* addr)
{
    // look in actual list
    mmaplist_t* head = mmaplist;
    int i = mmapsize;
    while(head) {
        const int n = (i>NCHUNK)?NCHUNK:i;
        i-=n;
        for(int i=0; i<n; ++i) {
            mmapchunk_t* chunk = &head->chunks[i];
            if ((uintptr_t)addr>=(uintptr_t)chunk->block 
            && ((uintptr_t)addr<(uintptr_t)chunk->block+chunk->size)) {
                if(!chunk->helper)
                    return FindDynablockDynablocklist(addr, chunk->dblist);
                else {
                    uintptr_t p = (uintptr_t)addr - (uintptr_t)chunk->block;
                    while(chunk->helper[p]) p -= chunk->helper[p];
                    khint_t k = kh_get(dynablocks, chunk->dblist, (uintptr_t)chunk->block + p);
                    if(k!=kh_end(chunk->dblist))
                        return kh_value(chunk->dblist, k);
                    return NULL;
                }
            }
        }
        head = head->next;
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
        box_free((void*)addr);
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
            if(arm_lock_storeifnull(&dynmap[i], p)!=NULL)
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
        int need_free = (destroy && addr<=(i<<DYNAMAP_SHIFT) && end>=((i+1)>>DYNAMAP_SHIFT)-1)?1:0;
        dynablocklist_t* dblist = (need_free)?((dynablocklist_t*)arm_lock_xchg(&dynmap[i], 0)):dynmap[i];
        if(dblist) {
            if(destroy) {
                FreeRangeDynablock(dblist, addr, size);
                if(need_free)
                    FreeDynablockList(&dblist);
            } else
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
        uintptr_t* tbl = (uintptr_t*)box_malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = (uintptr_t)arm_next;
        if(arm_lock_storeifref(&box86_jumptable[idx], tbl, box86_jmptbl_default)!=tbl)
            box_free(tbl);
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
        uintptr_t* tbl = (uintptr_t*)box_malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = (uintptr_t)arm_next;
        if(arm_lock_storeifref(&box86_jumptable[idx], tbl, box86_jmptbl_default)!=tbl)
            box_free(tbl);
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
    PROT_LOCK();
    for (uintptr_t i=idx; i<=end; ++i) {
        uint32_t prot = PROT_GET(i);
        if(!(prot&PROT_NOPROT)) {
            uint32_t dyn = prot&PROT_CUSTOM;
            prot&=~PROT_CUSTOM;
            if(!prot)
                prot = PROT_READ | PROT_WRITE | PROT_EXEC;      // comes from malloc & co, so should not be able to execute
            if(prot&PROT_WRITE) {
                PROT_SET(i, prot|PROT_DYNAREC);
                if(!dyn) mprotect((void*)(i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, prot&~PROT_WRITE);
            } else
                PROT_SET(i, prot|PROT_DYNAREC_R);
        }
    }
    PROT_UNLOCK();
}

// Add the Write flag from an adress range, and mark all block as dirty
// no log, as it can be executed inside a signal handler
void unprotectDB(uintptr_t addr, uintptr_t size, int mark)
{
    dynarec_log(LOG_DEBUG, "unprotectDB %p -> %p (mark=%d)\n", (void*)addr, (void*)(addr+size-1), mark);
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    PROT_LOCK();
    for (uintptr_t i=idx; i<=end; ++i) {
        uint32_t prot = PROT_GET(i);
        if(!(prot&PROT_NOPROT)) {
            if(prot&PROT_DYNAREC) {
                PROT_SET(i, prot&~PROT_CUSTOM);
                mprotect((void*)(i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, prot&~PROT_CUSTOM);
                if(mark)
                    cleanDBFromAddressRange((i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, 0);
            } else if(prot&PROT_DYNAREC_R)
                PROT_SET(i, prot&~PROT_CUSTOM);
        }
    }
    PROT_UNLOCK();
}

int isprotectedDB(uintptr_t addr, size_t size)
{
    dynarec_log(LOG_DEBUG, "isprotectedDB %p -> %p => ", (void*)addr, (void*)(addr+size-1));
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    PROT_LOCK();
    for (uintptr_t i=idx; i<=end; ++i) {
        uint32_t prot = PROT_GET(i);
        if(!(prot&PROT_NOPROT)) {
            if(!(prot&(PROT_DYNAREC|PROT_DYNAREC_R))) {
                dynarec_log(LOG_DEBUG, "0\n");
                PROT_UNLOCK();
                return 0;
            }
        }
    }
    dynarec_log(LOG_DEBUG, "1\n");
    PROT_UNLOCK();
    return 1;
}

#endif

void printMapMem()
{
    mapmem_t* m = mapmem;
    while(m) {
        printf_log(LOG_NONE, " %p-%p\n", (void*)m->begin, (void*)m->end);
        m = m->next;
    }
}

void addMapMem(uintptr_t begin, uintptr_t end)
{
    if(!mapmem)
        return;
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
        newm = (mapmem_t*)box_calloc(1, sizeof(mapmem_t));
        newm->next = m->next;
        newm->begin = begin;
        newm->end = end;
        m->next = newm;
    }
    while(newm && newm->next && (newm->next->begin-1)<=newm->end) {
        // fuse with next
        if(newm->next->end>newm->end)
            newm->end = newm->next->end;
        mapmem_t* tmp = newm->next;
        newm->next = tmp->next;
        box_free(tmp);
    }
    // all done!
}
void removeMapMem(uintptr_t begin, uintptr_t end)
{
    if(!mapmem)
        return;
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
            box_free(tmp);
        } else if(begin>m->begin && end<m->end) { // the zone is totaly inside the block => split it!
            mapmem_t* newm = (mapmem_t*)box_calloc(1, sizeof(mapmem_t));    // create a new "next"
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
    //dynarec_log(LOG_DEBUG, "updateProtection %p -> %p to 0x%02x\n", (void*)addr, (void*)(addr+size-1), prot);
    PROT_LOCK();
    addMapMem(addr, addr+size-1);
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    const uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        uint32_t dyn=(PROT_GET(i)&(PROT_DYNAREC|PROT_DYNAREC_R|PROT_NOPROT));
        if(!(dyn&PROT_NOPROT)) {
            if(dyn && (prot&PROT_WRITE)) {   // need to remove the write protection from this block
                dyn = PROT_DYNAREC;
                mprotect((void*)(i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, prot&~PROT_WRITE);
            } else if(dyn && !(prot&PROT_WRITE)) {
                dyn = PROT_DYNAREC_R;
            }
        }
        PROT_SET(i, prot|dyn);
    }
    PROT_UNLOCK();
}

void forceProtection(uintptr_t addr, uintptr_t size, uint32_t prot)
{
    //dynarec_log(LOG_DEBUG, "forceProtection %p -> %p to 0x%02x\n", (void*)addr, (void*)(addr+size-1), prot);
    PROT_LOCK();
    addMapMem(addr, addr+size-1);
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    const uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        mprotect((void*)(i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, prot&~PROT_CUSTOM);
        PROT_SET(i, prot);
    }
    PROT_UNLOCK();
}

void setProtection(uintptr_t addr, uintptr_t size, uint32_t prot)
{
    //dynarec_log(LOG_DEBUG, "setProtection %p -> %p to 0x%02x\n", (void*)addr, (void*)(addr+size-1), prot);
    PROT_LOCK();
    addMapMem(addr, addr+size-1);
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    const uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        PROT_SET(i, prot);
    }
    PROT_UNLOCK();
}

void freeProtection(uintptr_t addr, uintptr_t size)
{
    PROT_LOCK();
    removeMapMem(addr, addr+size-1);
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    const uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        PROT_SET(i, 0);
    }
    PROT_UNLOCK();
}

uint32_t getProtection(uintptr_t addr)
{
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    PROT_LOCK();
    uint32_t ret = PROT_GET(idx);
    PROT_UNLOCK();
    return ret;
}

void allocProtection(uintptr_t addr, uintptr_t size, uint32_t prot)
{
    PROT_LOCK();
    addMapMem(addr, addr+size-1);
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    const uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        PROT_SET_IF_0(i, prot);
    }
    PROT_UNLOCK();
}

void loadProtectionFromMap()
{
    if(box86_mapclean)
        return;
    char buf[500];
    FILE *f = fopen("/proc/self/maps", "r");
    uintptr_t current = 0x0;
    if(box86_log>=LOG_DEBUG || box86_dynarec_log>=LOG_DEBUG) {printf_log(LOG_NONE, "Refresh mmap allocated block start =============\n");}
    if(!f)
        return;
    while(!feof(f)) {
        char* ret = fgets(buf, sizeof(buf), f);
        (void)ret;
        char r, w, x;
        uintptr_t s, e;
        if(box86_log>=LOG_DEBUG || box86_dynarec_log>=LOG_DEBUG) {printf_log(LOG_NONE, "\t%s", buf);}
        if(sscanf(buf, "%x-%x %c%c%c", &s, &e, &r, &w, &x)==5) {
            if(current<s) {
                removeMapMem(current, s-1);
                current = e;
            }
            int prot = ((r=='r')?PROT_READ:0)|((w=='w')?PROT_WRITE:0)|((x=='x')?PROT_EXEC:0);
            allocProtection(s, e-s, prot);
        }
    }
    fclose(f);
    if(box86_log>=LOG_DEBUG || box86_dynarec_log>=LOG_DEBUG) {
        printf_log(LOG_NONE, "Refresh mmap allocated block done =============\n");
        printMapMem();
    }
    box86_mapclean = 1;
}

#define LOWEST (void*)0x10000
#define MEDIAN (void*)0x40000000
static void* findBlockHinted(void* hint, size_t size)
{
    mapmem_t* m = mapmem;
    uintptr_t h = (uintptr_t)hint;
    while(m) {
        // granularity 0x10000
        uintptr_t addr = (m->end+1+0xffff)&~0xffff;
        uintptr_t end = (m->next)?(m->next->begin-1):0xffffffff;
        // check hint and available size
        if(addr<=h && end>=h && end-h+1>=size)
            return hint;
        if(addr>=h && end-addr+1>=size)
            return (void*)addr;
        if(end>=0xc0000000 && h<0xc0000000)
            return NULL;
        m = m->next;
    }
    return NULL;
}
void* findBlockNearHint(void* hint, size_t size)
{   void* ret = findBlockHinted(hint, size);
    return ret?ret:hint;
}
void* find32bitBlock(size_t size)
{
    void* ret = findBlockHinted(MEDIAN, size);
    if(!ret) ret = findBlockHinted(LOWEST, size);
    return ret;
}

#ifdef DYNAREC
int IsInHotPage(uintptr_t addr) {
    if(addr<=(1LL<<48))
        return 0;
    int idx = (addr>>MEMPROT_SHIFT);
    if(!hotpages[idx])
        return 0;
    // decrement hot
    arm_lock_decifnot0b(&hotpages[idx]);
    return 1;
}

int AreaInHotPage(uintptr_t start, uintptr_t end_) {
    uintptr_t idx = (start>>MEMPROT_SHIFT);
    uintptr_t end = (end_>>MEMPROT_SHIFT);
    if(end<idx) { // memory addresses higher than 48bits are not tracked
        return 0;
    }
    int ret = 0;
    for (uintptr_t i=idx; i<=end; ++i) {
        if(hotpages[idx]) {
            // decrement hot
            arm_lock_decifnot0b(&hotpages[idx]);
            ret = 1;
        }
    }
    if(ret && box86_dynarec_log>LOG_INFO)
        dynarec_log(LOG_DEBUG, "BOX86: AreaInHotPage %p-%p\n", (void*)start, (void*)end_);
    return ret;

}

void AddHotPage(uintptr_t addr) {
    int idx = (addr>>MEMPROT_SHIFT);
    arm_lock_storeb(&hotpages[idx], box86_dynarec_hotpage);
}
#endif


int unlockCustommemMutex()
{
    int ret = 0;
    int i = 0;
    #ifdef DYNAREC
    void* tid = (void*)GetTID();
    #define GO(A, B)                    \
        i = (arm_lock_storeifref2(&A, NULL, (void*)tid)==tid);  \
        if(i) {                         \
            ret|=(1<<B);                \
        }
    #else
    #define GO(A, B)                    \
        i = checkUnlockMutex(&A);       \
        if(i) {                         \
            ret|=(1<<B);                \
        }
    #endif
    GO(mutex_blocks, 0)
    #undef GO
    return ret;
}

void relockCustommemMutex(int locks)
{
    #define GO(A, B)                    \
        if(locks&(1<<B))                \
            mutex_lock(&A);             \

    GO(mutex_blocks, 0)
    #undef GO
}

static void init_mutexes(void)
{
    #ifdef DYNAREC
    arm_lock_stored(&mutex_blocks, 0);
    #else
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&mutex_blocks, &attr);

    pthread_mutexattr_destroy(&attr);
    #endif
}

static void atfork_child_custommem(void)
{
    // (re))init mutex if it was lock before the fork
    init_mutexes();
}

void init_custommem_helper(box86context_t* ctx)
{
    (void)ctx;
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
    lockaddress = kh_init(lockaddress);
#endif
    pthread_atfork(NULL, NULL, atfork_child_custommem);
    // init mapmem list
    mapmem = (mapmem_t*)box_calloc(1, sizeof(mapmem_t));
    mapmem->begin = 0x0;
    mapmem->end = (uintptr_t)LOWEST - 1;
    loadProtectionFromMap();
}

void fini_custommem_helper(box86context_t *ctx)
{
    (void)ctx;
    if(!inited)
        return;
    inited = 0;
#ifdef DYNAREC
    if(box86_dynarec) {
        dynarec_log(LOG_DEBUG, "Free global Dynarecblocks\n");
        mmaplist_t* head = mmaplist;
        mmaplist = NULL;
        while(head) {
            for (int i=0; i<NCHUNK; ++i) {
                if(head->chunks[i].block)
                    #ifdef USE_MMAP
                    munmap(head->chunks[i].block, head->chunks[i].size);
                    #else
                    box_free(head->chunks[i].block);
                    #endif
                if(head->chunks[i].dblist) {
                    kh_destroy(dynablocks, head->chunks[i].dblist);
                    head->chunks[i].dblist = NULL;
                }
                if(head->chunks[i].helper) {
                    box_free(head->chunks[i].helper);
                    head->chunks[i].helper = NULL;
                }
            }
            mmaplist_t *old = head;
            head = head->next;
            free(old);
        }
        if(dblist_oversized) {
            kh_destroy(dynablocks, dblist_oversized);
            dblist_oversized = NULL;
        }
        mmapsize = 0;
        dynarec_log(LOG_DEBUG, "Free dynamic Dynarecblocks\n");
        uintptr_t idx = 0;
        uintptr_t end = ((0xFFFFFFFF)>>DYNAMAP_SHIFT);
        for (uintptr_t i=idx; i<=end; ++i)
            if(dynmap[i])
                FreeDynablockList(&dynmap[i]);
        box_free(mmaplist);
        for (int i=0; i<DYNAMAP_SIZE; ++i)
            if(box86_jumptable[i]!=box86_jmptbl_default)
                box_free(box86_jumptable[i]);
    }
    kh_destroy(lockaddress, lockaddress);
    lockaddress = NULL;
#endif
    for(int i=0; i<n_blocks; ++i)
        #ifdef USE_MMAP_MORE
        munmap(p_blocks[i].block, p_blocks[i].size);
        #else
        box_free(p_blocks[i].block);
        #endif
    box_free(p_blocks);
    #ifndef DYNAREC
    pthread_mutex_destroy(&mutex_blocks);
    #endif
    while(mapmem) {
        mapmem_t *tmp = mapmem;
        mapmem = mapmem->next;
        box_free(tmp);
    }
}

#ifdef DYNAREC
// add an address to the list of "LOCK"able
void addLockAddress(uintptr_t addr)
{
    int ret;
    kh_put(lockaddress, lockaddress, addr, &ret);
}

// return 1 is the address is used as a LOCK, 0 else
int isLockAddress(uintptr_t addr)
{
    khint_t k = kh_get(lockaddress, lockaddress, addr);
    return (k==kh_end(lockaddress))?0:1;
}

#endif
