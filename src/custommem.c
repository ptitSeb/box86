#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>
#include <errno.h>

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
#include "dynarec/dynablock_private.h"
#include "dynarec/arm_lock_helper.h"
#include "khash.h"

#define USE_MMAP
//#define USE_MMAP_MORE

// init inside dynablocks.c
static mmaplist_t          *mmaplist = NULL;
//#define TRACE_MEMSTAT
#ifdef TRACE_MEMSTAT
static size_t jmptbl_allocated = 0, jmptbl_allocated1 = 0;
#endif
static int                 mmapsize = 0;
static uintptr_t           *box86_jmptbl[JMPTABL_SIZE];
static uintptr_t           box86_jmptbl_default[1<<JMPTABL_SHIFT];
// lock addresses
KHASH_SET_INIT_INT(lockaddress)
static kh_lockaddress_t    *lockaddress = NULL;
static uint32_t            mutex_prot;
static uint32_t            mutex_blocks;
#else
static pthread_mutex_t     mutex_prot;
static pthread_mutex_t     mutex_blocks;
#endif
#define MEMPROT_SHIFT 12
#define MEMPROT_SIZE (1<<(32-MEMPROT_SHIFT))
static uint8_t             memprot[MEMPROT_SIZE] = {0};    // protection flags by 4K block
#ifdef DYNAREC
static uint8_t             hotpages[MEMPROT_SIZE] = {0};
#endif
#ifdef TRACE_MEMSTAT
static uint64_t  memprot_allocated = 0, memprot_max_allocated = 0;
#endif
static int inited = 0;


typedef struct mapmem_s {
    uintptr_t begin, end;
    struct mapmem_s *next;
} mapmem_t;

static mapmem_t *mapmem = NULL;

typedef struct blocklist_s {
    void*               block;
    size_t              maxfree;
    size_t              size;
    void*               first;
} blocklist_t;

#define MMAPSIZE (256*1024)      // allocate 256kb sized blocks

static int                 n_blocks = 0;       // number of blocks for custom malloc
static int                 c_blocks = 0;       // capacity of blocks for custom malloc
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

void printBlock(blockmark_t* b, void* start)
{
    printf_log(LOG_NONE, "========== Block is:\n");
    do {
        printf_log(LOG_NONE, "%c%p, fill=%d, size=0x%x (prev=%d/0x%x)\n", b==start?'*':' ', b, b->next.fill, b->next.size, b->prev.fill, b->prev.size);
        b = NEXT_BLOCK(b);
    } while(b->next.x32);
    printf_log(LOG_NONE, "===================\n");
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

static void* getNextFreeBlock(void* block)
{
    blockmark_t *m = (blockmark_t*)block;
    while (m->next.fill) {
         m = NEXT_BLOCK(m);
    };
    return m;
}
static void* getPrevFreeBlock(void* block)
{
    blockmark_t *m = (blockmark_t*)block;
    do {
         m = PREV_BLOCK(m);
    } while (m->next.fill);
    return m;
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
    if(pstart && (uintptr_t)*pstart>(uintptr_t)s) {
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

// return 1 if block is coherent, 0 if not (and printf the issues)
int printBlockCoherent(int i)
{
    if(i<0 || i>=n_blocks) {
        printf_log(LOG_NONE, "Error, %d should be between 0 and %d\n", i, n_blocks);
        return 0;
    }
    int ret = 1;
    blockmark_t* m = (blockmark_t*)p_blocks[i].block;
    // check if first is correct
    blockmark_t* first = getNextFreeBlock(m);
    if(p_blocks[i].first && p_blocks[i].first!=first) {printf_log(LOG_NONE, "First %p and stored first %p differs for block %d\n", first, p_blocks[i].first, i); ret = 0;}
    // check if maxfree is correct, with no hint
    size_t maxfree = getMaxFreeBlock(m, p_blocks[i].size, NULL);
    if(maxfree != p_blocks[i].maxfree) {printf_log(LOG_NONE, "Maxfree without hint %zd and stored maxfree %zd differs for block %d\n", maxfree, p_blocks[i].maxfree); ret = 0;}
    // check if maxfree from first is correct
    maxfree = getMaxFreeBlock(m, p_blocks[i].size, p_blocks[i].first);
    if(maxfree != p_blocks[i].maxfree) {printf_log(LOG_NONE, "Maxfree with hint %zd and stored maxfree %zd differs for block %d\n", maxfree, p_blocks[i].maxfree); ret = 0;}
    // check chain
    blockmark_t* last = (blockmark_t*)(((uintptr_t)m)+p_blocks[i].size-sizeof(blockmark_t));
    while(m<last) {
        blockmark_t* n = NEXT_BLOCK(m);
        if(!m->next.fill && !n->next.fill && n!=last) {
            printf_log(LOG_NONE, "Chain contains 2 subsequent free blocks %p (%d) and %p (%d) for block %d\n", m, m->next.size, n, n->next.size, i);
            ret = 0;
        }
        m = n;
    }
    if(m!=last) {
        printf_log(LOG_NONE, "Last block %p is behond expexted block %p for block %d\n", m, last, i);
        ret = 0;
    }

    return ret;
}

void testAllBlocks()
{
    size_t total = 0;
    size_t fragmented_free = 0;
    size_t max_free = 0;
    for(int i=0; i<n_blocks; ++i) {
        printBlockCoherent(i);
        total += p_blocks[i].size;
        if(max_free<p_blocks[i].maxfree)
            max_free = p_blocks[i].maxfree;
        blockmark_t* m = (blockmark_t*)p_blocks[i].block;
        while(m->next.x32) {
            if(!m->next.fill)
                fragmented_free += m->next.size;
            m = NEXT_BLOCK(m);
        }
    }
    printf_log(LOG_NONE, "Total %d blocks, for %zd allocated memory, max_free %zd, toatal fragmented free %zd\n", n_blocks, total, max_free, fragmented_free);
}

#define THRESHOLD   (128-2*sizeof(blockmark_t))

static size_t roundSize(size_t size)
{
    if(!size)
        return size;
    size = (size+7)&~7LL;   // 8 bytes align in size

    if(size<THRESHOLD)
        size = THRESHOLD;

    return size;
}


#ifdef TRACE_MEMSTAT
static size_t customMalloc_allocated = 0;
#endif
void* customMalloc(size_t size)
{
    size = roundSize(size);
    // look for free space
    void* sub = NULL;
    size_t fullsize = size+2*sizeof(blockmark_t);
    mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if(p_blocks[i].maxfree>=size) {
            size_t rsize = 0;
            sub = getFirstBlock(p_blocks[i].block, size, &rsize, p_blocks[i].first);
            if(sub) {
                if(rsize-size<THRESHOLD)
                    size = rsize;
                void* ret = allocBlock(p_blocks[i].block, sub, size, &p_blocks[i].first);
                if(rsize==p_blocks[i].maxfree)
                    p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size, p_blocks[i].first);
                mutex_unlock(&mutex_blocks);
                return ret;
            }
        }
    }
    // add a new block
    int i = n_blocks++;
    if(n_blocks>c_blocks) {
        c_blocks += 4;
        p_blocks = (blocklist_t*)box_realloc(p_blocks, c_blocks*sizeof(blocklist_t));
    }
    size_t allocsize = (fullsize>MMAPSIZE)?fullsize:MMAPSIZE;
    #ifdef USE_MMAP
    void* p = mmap(NULL, allocsize, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
    memset(p, 0, allocsize);
    #else
    void* p = box_calloc(1, allocsize);
    #endif
#ifdef TRACE_MEMSTAT
    customMalloc_allocated += allocsize;
#endif
    p_blocks[i].block = p;
    p_blocks[i].first = p;
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
    void* ret  = allocBlock(p_blocks[i].block, p, size, &p_blocks[i].first);
    p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size, p_blocks[i].first);
    mutex_unlock(&mutex_blocks);
    return ret;
}
void* customCalloc(size_t n, size_t size)
{
    size_t newsize = roundSize(n*size);
    void* ret = customMalloc(newsize);
    memset(ret, 0, newsize);
    return ret;
}
void* customRealloc(void* p, size_t size)
{
    if(!p)
        return customMalloc(size);
    size = roundSize(size);
    uintptr_t addr = (uintptr_t)p;
    mutex_lock(&mutex_blocks);
    for(int i=0; i<n_blocks; ++i) {
        if ((addr>(uintptr_t)p_blocks[i].block) 
         && (addr<((uintptr_t)p_blocks[i].block+p_blocks[i].size))) {
            void* sub = (void*)(addr-sizeof(blockmark_t));
            if(expandBlock(p_blocks[i].block, sub, size)) {
                if(sub<p_blocks[i].first && p+size+sizeof(blockmark_t)>=p_blocks[i].first)
                    p_blocks[i].first = getNextFreeBlock(sub);
                p_blocks[i].maxfree = getMaxFreeBlock(p_blocks[i].block, p_blocks[i].size, p_blocks[i].first);
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
            size_t newfree = freeBlock(p_blocks[i].block, sub, &p_blocks[i].first);
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
void arm_next(void);

#define NCHUNK          64
typedef struct mmaplist_s {
    blocklist_t         chunks[NCHUNK];
    mmaplist_t*         next;
} mmaplist_t;

dynablock_t* FindDynablockFromNativeAddress(void* p)
{
    if(!p)
        return NULL;
    
    uintptr_t addr = (uintptr_t)p;

    int i= 0;
    mmaplist_t* list = mmaplist;
    if(!list)
        return NULL;
    while(list) {
        if ((addr>(uintptr_t)list->chunks[i].block) 
         && (addr<((uintptr_t)list->chunks[i].block+list->chunks[i].size))) {
            blockmark_t* sub = (blockmark_t*)list->chunks[i].block;
            while((uintptr_t)sub<addr) {
                blockmark_t* n = NEXT_BLOCK(sub);
                if((uintptr_t)n>addr) {
                    // found it!
                    // self is the field of a block
                    return *(dynablock_t**)((uintptr_t)sub+sizeof(blockmark_t));
                }
                sub = n;
            }
            return NULL;
        }
        ++i;
        if(i==NCHUNK) {
            i = 0;
            list = list->next;
        }
    }
    return NULL;
}

#ifdef TRACE_MEMSTAT
static uint64_t dynarec_allocated = 0;
#endif
uintptr_t AllocDynarecMap(size_t size)
{
    if(!size)
        return 0;

    size = roundSize(size);

    mmaplist_t* list = mmaplist;
    if(!list)
        list = mmaplist = (mmaplist_t*)box_calloc(1, sizeof(mmaplist_t));
    // check if there is space in current open ones
    int i = 0;
    uintptr_t sz = size + 2*sizeof(blockmark_t);
    while(1) {
        if(list->chunks[i].maxfree>=size) {
            // looks free, try to alloc!
            size_t rsize = 0;
            void* sub = getFirstBlock(list->chunks[i].block, size, &rsize, list->chunks[i].first);
            if(sub) {
                void* ret = allocBlock(list->chunks[i].block, sub, size, NULL);
                if(sub==list->chunks[i].first)
                    list->chunks[i].first = getNextFreeBlock(sub);
                if(rsize==list->chunks[i].maxfree)
                    list->chunks[i].maxfree = getMaxFreeBlock(list->chunks[i].block, list->chunks[i].size, list->chunks[i].first);
                return (uintptr_t)ret;
            }
        }
        // check if new
        if(!list->chunks[i].size) {
            // alloc a new block, aversized or not, we are at the end of the list
            size_t allocsize = (sz>MMAPSIZE)?sz:MMAPSIZE;
            // allign sz with pagesize
            allocsize = (allocsize+(box86_pagesize-1))&~(box86_pagesize-1);
            #ifndef USE_MMAP
            void *p = NULL;
            if(!(p=box_memalign(box86_pagesize, allocsize))) {
                dynarec_log(LOG_INFO, "Cannot create dynamic map of %zu bytes\n", allocsize);
                return 0;
            }
            mprotect(p, allocsize, PROT_READ | PROT_WRITE | PROT_EXEC);
            #else
            void* p = mmap(NULL, allocsize, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
            if(p==(void*)-1) {
                dynarec_log(LOG_INFO, "Cannot create dynamic map of %zu bytes\n", allocsize);
                return 0;
            }
            #endif
#ifdef TRACE_MEMSTAT
            dynarec_allocated += allocsize;
#endif
            setProtection((uintptr_t)p, allocsize, PROT_READ | PROT_WRITE | PROT_EXEC);
            list->chunks[i].block = p;
            list->chunks[i].first = p;
            list->chunks[i].size = allocsize;
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
            void* ret  = allocBlock(list->chunks[i].block, p, size, NULL);
            list->chunks[i].maxfree = getMaxFreeBlock(list->chunks[i].block, list->chunks[i].size, NULL);
            if(list->chunks[i].maxfree)
                list->chunks[i].first = getNextFreeBlock(m);
            return (uintptr_t)ret;
        }
        // next chunk...
        ++i;
        if(i==NCHUNK) {
            i = 0;
            if(!list->next)
                list->next = (mmaplist_t*)box_calloc(1, sizeof(mmaplist_t));
            list = list->next;
        }
    }
}

void FreeDynarecMap(uintptr_t addr)
{
    if(!addr)
        return;
    
    int i= 0;
    mmaplist_t* list = mmaplist;

    while(list) {
        if ((addr>(uintptr_t)list->chunks[i].block) 
         && (addr<((uintptr_t)list->chunks[i].block+list->chunks[i].size))) {
            void* sub = (void*)(addr-sizeof(blockmark_t));
            size_t newfree = freeBlock(list->chunks[i].block, sub, &list->chunks[i].first);
            if(list->chunks[i].maxfree < newfree)
                list->chunks[i].maxfree = newfree;
            return;
        }
        ++i;
        if(i==NCHUNK) {
            i = 0;
            list = list->next;
        }
    }
}

static uintptr_t getDBSize(uintptr_t addr, size_t maxsize, dynablock_t** db)
{
    uintptr_t idx1 = addr>>JMPTABL_SHIFT;
    uintptr_t idx0 = addr&JMPTABLE_MASK;
    *db = *(dynablock_t**)(box86_jmptbl[idx1][idx0]- sizeof(void*));
    if(*db)
        return addr+1;
    uintptr_t* block = box86_jmptbl[idx1];
    if(block == box86_jmptbl_default)
        return ((idx1+1)<<JMPTABL_SHIFT);
    maxsize+=idx0;  // need to adjust maxsize to "end in current block"
    if (maxsize>JMPTABLE_MASK)
        maxsize = JMPTABLE_MASK;
    while(block[idx0]==(uintptr_t)arm_next) {
        ++idx0;
        if(idx0>maxsize)
            return (addr&~JMPTABLE_MASK)+idx0;
    }
    *db = *(dynablock_t**)(block[idx0]- sizeof(void*));
    return (addr&~JMPTABLE_MASK)+idx0+1;
}

// each dynmap is 64k of size

void addDBFromAddressRange(uintptr_t addr, size_t size)
{
    dynarec_log(LOG_DEBUG, "addDBFromAddressRange %p -> %p\n", (void*)addr, (void*)(addr+size-1));
    // do nothing, dynablock are allowed based on memory protection flags
}

void cleanDBFromAddressRange(uintptr_t addr, size_t size, int destroy)
{
    uintptr_t start_addr = my_context?((addr<my_context->max_db_size)?0:(addr-my_context->max_db_size)):addr;
    dynarec_log(LOG_DEBUG, "cleanDBFromAddressRange %p/%p -> %p %s\n", (void*)addr, (void*)start_addr, (void*)(addr+size-1), destroy?"destroy":"mark");
    dynablock_t* db = NULL;
    uintptr_t end = addr+size;
    while (start_addr<end) {
        start_addr = getDBSize(start_addr, end-start_addr, &db);
        if(db) {
            if(destroy)
                FreeRangeDynablock(db, addr, size);
            else
                MarkRangeDynablock(db, addr, size);
        }
    }
}

static uintptr_t *create_jmptbl(uintptr_t idx0, uintptr_t idx1)
{
    if(box86_jmptbl[idx1] == box86_jmptbl_default) {
        uintptr_t* tbl = (uintptr_t*)box_malloc((1<<JMPTABL_SHIFT)*sizeof(uintptr_t));
        for(int i=0; i<(1<<JMPTABL_SHIFT); ++i)
            tbl[i] = (uintptr_t)arm_next;
        if(arm_lock_storeifref(&box86_jmptbl[idx1], tbl, box86_jmptbl_default)!=tbl)
            box_free(tbl);
#ifdef TRACE_MEMSTAT
        else {
            jmptbl_allocated += (1<<JMPTABL_SHIFT)*sizeof(uintptr_t);
            ++jmptbl_allocated;
        }
#endif
    }
    return &box86_jmptbl[idx1][idx0];
}

int addJumpTableIfDefault(void* addr, void* jmp)
{
    uintptr_t idx1, idx0;
    idx1 = (((uintptr_t)addr)>>JMPTABL_SHIFT)&JMPTABLE_MASK;
    idx0 = (((uintptr_t)addr)               )&JMPTABLE_MASK;

    return (arm_lock_storeifref(create_jmptbl(idx0, idx1), jmp, arm_next)==jmp)?1:0;
}
void setJumpTableDefault(void* addr)
{
    uintptr_t idx1, idx0;
    idx1 = (((uintptr_t)addr)>>JMPTABL_SHIFT)&JMPTABLE_MASK;
    if(box86_jmptbl[idx1] == box86_jmptbl_default)
        return;
    idx0 = (((uintptr_t)addr)    )&JMPTABLE_MASK;
    arm_lock_stored(&box86_jmptbl[idx1][idx0], (uintptr_t)arm_next);
}
void setJumpTableDefaultRef(void* addr, void* jmp)
{
    uintptr_t idx1, idx0;
    idx1 = (((uintptr_t)addr)>>JMPTABL_SHIFT)&JMPTABLE_MASK;
    if(box86_jmptbl[idx1] == box86_jmptbl_default)
        return;
    idx0 = (((uintptr_t)addr)    )&JMPTABLE_MASK;
    arm_lock_storeifref(&box86_jmptbl[idx1][idx0], arm_next, jmp);
}
int setJumpTableIfRef(void* addr, void* jmp, void* ref)
{
    uintptr_t idx1, idx0;
    idx1 = (((uintptr_t)addr)>>JMPTABL_SHIFT)&JMPTABLE_MASK;
    idx0 = (((uintptr_t)addr)               )&JMPTABLE_MASK;
    return (arm_lock_storeifref(create_jmptbl(idx0, idx1), jmp, ref)==jmp)?1:0;
}
int isJumpTableDefault(void* addr)
{
    uintptr_t idx1, idx0;
    idx1 = (((uintptr_t)addr)>>JMPTABL_SHIFT)&JMPTABLE_MASK;
    if(box86_jmptbl[idx1] == box86_jmptbl_default)
        return 1;
    idx0 = (((uintptr_t)addr)               )&JMPTABLE_MASK;
    return (box86_jmptbl[idx1][idx0]==(uintptr_t)arm_next)?1:0;
}
uintptr_t getJumpTable()
{
    return (uintptr_t)box86_jmptbl;
}

uintptr_t getJumpTableAddress(uintptr_t addr)
{
    uintptr_t idx1, idx0;
    idx1 = ((addr)>>JMPTABL_SHIFT)&JMPTABLE_MASK;
    idx0 = ((addr)               )&JMPTABLE_MASK;
    return (uintptr_t)create_jmptbl(idx0, idx1);
}

dynablock_t* getDB(uintptr_t addr)
{
    uintptr_t idx1, idx0;
    idx1 = ((addr)>>JMPTABL_SHIFT)&JMPTABLE_MASK;
    idx0 = ((addr)               )&JMPTABLE_MASK;
    uintptr_t ret = (uintptr_t)box86_jmptbl[idx1][idx0];

    return *(dynablock_t**)(ret - sizeof(void*));
}

int getNeedTest(uintptr_t addr)
{
    uintptr_t idx1, idx0;
    idx1 = ((addr)>>JMPTABL_SHIFT)&JMPTABLE_MASK;
    idx0 = ((addr)               )&JMPTABLE_MASK;
    uintptr_t ret = (uintptr_t)box86_jmptbl[idx1][idx0];
    dynablock_t* db = *(dynablock_t**)(ret - sizeof(void*));
    return db?((ret!=(uintptr_t)db->block)?1:0):0;
}

uintptr_t getJumpAddress64(uintptr_t addr)
{
    uintptr_t idx1, idx0;
    idx1 = ((addr)>>JMPTABL_SHIFT)&JMPTABLE_MASK;
    idx0 = ((addr)               )&JMPTABLE_MASK;
    return (uintptr_t)box86_jmptbl[idx1][idx0];
}

// Remove the Write flag from an adress range, so DB can be executed
void protectDB(uintptr_t addr, uintptr_t size)
{
    dynarec_log(LOG_DEBUG, "protectDB %p -> %p\n", (void*)addr, (void*)(addr+size-1));
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    mutex_lock(&mutex_prot);
    for (uintptr_t i=idx; i<=end; ++i) {
        uint32_t prot = memprot[i];
        uint32_t dyn = prot&PROT_DYN;
        uint32_t mapped = prot&PROT_MMAP;
        if(!prot)
            prot = PROT_READ | PROT_WRITE | PROT_EXEC;      // comes from malloc & co, so should not be able to execute
        prot&=~PROT_CUSTOM;
        if(!(dyn&PROT_NOPROT)) {
            if(prot&PROT_WRITE) {
                if(!dyn) mprotect((void*)(i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, prot&~PROT_WRITE);
                    memprot[i] = prot|mapped|PROT_DYNAREC;   // need to use atomic exchange?
                } else 
                    memprot[i] = prot|mapped|PROT_DYNAREC_R;
        }
    }
    mutex_unlock(&mutex_prot);
}

// Add the Write flag from an adress range, and mark all block as dirty
// no log, as it can be executed inside a signal handler
void unprotectDB(uintptr_t addr, uintptr_t size, int mark)
{
    dynarec_log(LOG_DEBUG, "unprotectDB %p -> %p (mark=%d)\n", (void*)addr, (void*)(addr+size-1), mark);
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    mutex_lock(&mutex_prot);
    for (uintptr_t i=idx; i<=end; ++i) {
        uint32_t prot = memprot[i];
        if(!(prot&PROT_NOPROT)) {
            if(prot&PROT_DYNAREC) {
                prot&=~PROT_DYN;
                if(mark)
                    cleanDBFromAddressRange((i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, 0);
                mprotect((void*)(i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, prot&~PROT_MMAP);
                memprot[i] = prot;  // need to use atomic exchange?
            } else if(prot&PROT_DYNAREC_R)
                memprot[i] = prot&~PROT_CUSTOM;
        }
    }
    mutex_unlock(&mutex_prot);
}

int isprotectedDB(uintptr_t addr, size_t size)
{
    dynarec_log(LOG_DEBUG, "isprotectedDB %p -> %p => ", (void*)addr, (void*)(addr+size-1));
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        uint32_t prot = memprot[i];
        if(!(prot&PROT_DYN)) {
            dynarec_log(LOG_DEBUG, "0\n");
            return 0;
        }
    }
    dynarec_log(LOG_DEBUG, "1\n");
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
    begin &=~(box86_pagesize-1);
    end = (end&~(box86_pagesize-1))+(box86_pagesize-1); // full page
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
    begin &=~(box86_pagesize-1);
    end = (end&~(box86_pagesize-1))+(box86_pagesize-1); // full page
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
    mutex_lock(&mutex_prot);
    addMapMem(addr, addr+size-1);
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    const uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        uint32_t old_prot = memprot[i];
        uint32_t dyn=(old_prot&PROT_DYN);
        uint32_t mapped=(old_prot&PROT_MMAP);
        if(!(dyn&PROT_NOPROT)) {
            if(dyn && (prot&PROT_WRITE)) {   // need to remove the write protection from this block
                dyn = PROT_DYNAREC;
                mprotect((void*)(i<<MEMPROT_SHIFT), 1<<MEMPROT_SHIFT, prot&~PROT_WRITE);
            } else if(dyn && !(prot&PROT_WRITE)) {
                dyn = PROT_DYNAREC_R;
            }
        }
        memprot[i] = prot|dyn|mapped;
    }
    mutex_unlock(&mutex_prot);
}

void setProtection(uintptr_t addr, uintptr_t size, uint32_t prot)
{
    //dynarec_log(LOG_DEBUG, "setProtection %p -> %p to 0x%02x\n", (void*)addr, (void*)(addr+size-1), prot);
    mutex_lock(&mutex_prot);
    addMapMem(addr, addr+size-1);
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    const uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        memprot[i] = prot;
    }
    mutex_unlock(&mutex_prot);
}

void setProtection_mmap(uintptr_t addr, size_t size, uint32_t prot)
{
    setProtection(addr, size, prot|PROT_MMAP);
}

void refreshProtection(uintptr_t addr)
{
    mutex_lock(&mutex_prot);
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    int prot = memprot[idx];
    if(!(prot&PROT_DYNAREC)) {
        int ret = mprotect((void*)(idx<<MEMPROT_SHIFT), box86_pagesize, prot&~PROT_CUSTOM);
printf_log(LOG_INFO, "refreshProtection(%p): %p/0x%x (ret=%d/%s)\n", (void*)addr, (void*)(idx<<MEMPROT_SHIFT), prot, ret, ret?strerror(errno):"ok");
    }
    mutex_unlock(&mutex_prot);
}

void allocProtection(uintptr_t addr, size_t size, uint32_t prot)
{
    dynarec_log(LOG_DEBUG, "allocProtection %p:%p 0x%x\n", (void*)addr, (void*)(addr+size-1), prot);
    uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uintptr_t end = ((addr+size-1LL)>>MEMPROT_SHIFT);
    mutex_lock(&mutex_prot);
    addMapMem(addr, addr+size-1);
    mutex_unlock(&mutex_prot);
}

#ifdef DYNAREC
int IsInHotPage(uintptr_t addr) {
    if(addr>=(1LL<<48))
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
    int ret = 0;
    for (uintptr_t i=idx; i<=end; ++i) {
        uint32_t hot = hotpages[i];
        if(hot) {
            // decrement hot
            arm_lock_decifnot0b(&hotpages[i]);
            ret = 1;
        }
    }
    if(ret && box86_dynarec_log>LOG_INFO)
        dynarec_log(LOG_DEBUG, "BOX64: AreaInHotPage %p-%p\n", (void*)start, (void*)end_);
    return ret;

}

void AddHotPage(uintptr_t addr) {
    int idx = (addr>>MEMPROT_SHIFT);
    arm_lock_storeb(&hotpages[idx], box86_dynarec_hotpage);
}
#endif

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

void freeProtection(uintptr_t addr, uintptr_t size)
{
    mutex_lock(&mutex_prot);
    removeMapMem(addr, addr+size-1);
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    const uintptr_t end = ((addr+size-1)>>MEMPROT_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        memprot[i] = 0;
    }
    mutex_unlock(&mutex_prot);
}

uint32_t getProtection(uintptr_t addr)
{
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    mutex_lock(&mutex_prot);
    uint32_t ret = memprot[idx];
    mutex_unlock(&mutex_prot);
    return ret&~PROT_MMAP;
}

int getMmapped(uintptr_t addr)
{
    mutex_lock(&mutex_prot);
    const uintptr_t idx = (addr>>MEMPROT_SHIFT);
    uint32_t ret = memprot[idx];
    mutex_unlock(&mutex_prot);
    return (ret&PROT_MMAP)?1:0;
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
    GO(mutex_prot, 1)
    #undef GO
    return ret;
}

void relockCustommemMutex(int locks)
{
    #define GO(A, B)                    \
        if(locks&(1<<B))                \
            mutex_lock(&A);             \

    GO(mutex_blocks, 0)
    GO(mutex_prot, 1)
    #undef GO
}

static void init_mutexes(void)
{
    #ifdef DYNAREC
    arm_lock_stored(&mutex_blocks, 0);
    arm_lock_stored(&mutex_prot, 0);
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
        box86_jmptbl[i] = box86_jmptbl_default;
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
#ifdef TRACE_MEMSTAT
    uintptr_t njmps = 0, njmps_in_lv1_max = 0;
    for (uintptr_t idx1 = 0; idx1 < (1 << JMPTABL_SHIFT); ++idx1) {
        if (box86_jmptbl[idx1] == box86_jmptbl_default) continue;
        uintptr_t njmps_in_cur_lv1 = 0;
        for (uintptr_t idx0 = 0; idx0 < (1 << JMPTABL_SHIFT); ++idx0) {
            if (box86_jmptbl[idx1][idx0] == (uintptr_t)arm_next) continue;
            ++njmps;
            ++njmps_in_cur_lv1;
        }
        if (njmps_in_cur_lv1 > njmps_in_lv1_max) njmps_in_lv1_max = njmps_in_cur_lv1;
    }
    printf_log(LOG_INFO, "Allocation:\n- dynarec: %lld kio\n- customMalloc: %lld kio\n- memprot: %lld kio (peak at %lld kio)\n- jump table: %lld kio (%lld level 1 table allocated, for %lld jumps, with at most %lld per level 1)\n", dynarec_allocated / 1024, customMalloc_allocated / 1024, memprot_allocated / 1024, memprot_max_allocated / 1024, jmptbl_allocated / 1024, jmptbl_allocated1, njmps, njmps_in_lv1_max);
#endif
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
            }
            mmaplist_t *old = head;
            head = head->next;
            free(old);
        }
        mmapsize = 0;
        dynarec_log(LOG_DEBUG, "Free dynamic Dynarecblocks\n");
        uintptr_t idx = 0;
        uintptr_t end = ((0xFFFFFFFF)>>DYNAMAP_SHIFT);
        box_free(mmaplist);
        for (int i=0; i<DYNAMAP_SIZE; ++i)
            if(box86_jmptbl[i]!=box86_jmptbl_default)
                box_free(box86_jmptbl[i]);
    }
    kh_destroy(lockaddress, lockaddress);
    lockaddress = NULL;
#endif
    for(int i=0; i<n_blocks; ++i)
        #ifdef USE_MMAP
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
