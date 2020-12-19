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
#ifdef DYNAREC
#include <sys/mman.h>
#include "dynablock.h"
#include "khash.h"

#define MMAPSIZE (256*1024)      // allocate 256kb sized blocks

// init inside dynablocks.c
KHASH_MAP_INIT_INT(dynablocks, dynablock_t*)

typedef union mark_s {
    struct {
        unsigned int    fill:1;
        unsigned int    size:31;
    };
    uint32_t            x32;
} mark_t;
typedef struct blockmark_s {
    mark_t  prev;
    mark_t  next;
} blockmark_t;

typedef struct mmaplist_s {
    void*               block;
    int                 maxfree;
    kh_dynablocks_t*    dblist;
} mmaplist_t;


// get first subblock free in block, stating at start (from block). return NULL if no block, else first subblock free (mark included), filling size
static void* getFirstBlock(void* block, int maxsize, int* size)
{
    // get start of block
    blockmark_t *m = (blockmark_t*)block;
    while(m->next.x32) {    // while there is a subblock
        if(!m->next.fill && m->next.size>=maxsize+sizeof(blockmark_t)) {
            *size = m->next.size;
            return m;
        }
        m = (blockmark_t*)((uintptr_t)m + m->next.size);
    }

    return NULL;
}

static int getMaxFreeBlock(void* block)
{
    // get start of block
    blockmark_t *m = (blockmark_t*)((uintptr_t)block+MMAPSIZE-sizeof(blockmark_t)); // styart with the end
    int maxsize = 0;
    while(m->prev.x32) {    // while there is a subblock
        if(!m->prev.fill && m->prev.size>maxsize) {
            maxsize = m->prev.size;
            if((uintptr_t)block+maxsize>(uintptr_t)m)
                return maxsize; // no block large enough left...
        }
        m = (blockmark_t*)((uintptr_t)m - m->prev.size);
    }
    return maxsize;
}

static void* allocBlock(void* block, void *sub, int size)
{
    blockmark_t *s = (blockmark_t*)sub;
    blockmark_t *n = (blockmark_t*)((uintptr_t)s + s->next.size);

    s->next.fill = 1;
    s->next.size = size+sizeof(blockmark_t);
    blockmark_t *m = (blockmark_t*)((uintptr_t)s + s->next.size);   // this is new n
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

    return (void*)((uintptr_t)sub + sizeof(blockmark_t));
}
static void freeBlock(void *block, void* sub, int size)
{
    blockmark_t *m = (blockmark_t*)block;
    blockmark_t *s = (blockmark_t*)sub;
    blockmark_t *n = (blockmark_t*)((uintptr_t)s + s->next.size);
    if(block!=sub)
        m = (blockmark_t*)((uintptr_t)s - s->prev.size);
    s->next.fill = 0;
    n->prev.fill = 0;
    // check if merge with previous
    if (s->prev.x32 && !s->prev.fill) {
        // remove s...
        m->next.size += s->next.size;
        n->prev.size = m->next.size;
        s = m;
    }
    // check if merge with next
    if(n->next.x32 && !n->next.fill) {
        blockmark_t *n2 = (blockmark_t*)((uintptr_t)n + n->next.size);
        //remove n
        s->next.size += n->next.size;
        n2->prev.size = s->next.size;
    }
}

uintptr_t FindFreeDynarecMap(dynablock_t* db, int size)
{
    // look for free space
    void* sub = NULL;
    for(int i=0; i<my_context->mmapsize; ++i) {
        if(my_context->mmaplist[i].maxfree>=size) {
            int rsize = 0;
            sub = getFirstBlock(my_context->mmaplist[i].block, size, &rsize);
            if(sub) {
                uintptr_t ret = (uintptr_t)allocBlock(my_context->mmaplist[i].block, sub, size);
                if(rsize==my_context->mmaplist[i].maxfree)
                    my_context->mmaplist[i].maxfree = getMaxFreeBlock(my_context->mmaplist[i].block);
                kh_dynablocks_t *blocks = my_context->mmaplist[i].dblist;
                if(!blocks) {
                    blocks = my_context->mmaplist[i].dblist = kh_init(dynablocks);
                    kh_resize(dynablocks, blocks, 64);
                }
                khint_t k;
                int r;
                k = kh_put(dynablocks, blocks, (uintptr_t)db, &r);
                kh_value(blocks, k) = db;
                return ret;
            }
        }
    }
    return 0;
}

uintptr_t AddNewDynarecMap(dynablock_t* db, int size)
{
    int i = my_context->mmapsize++;    // yeah, useful post incrementation
    dynarec_log(LOG_DEBUG, "Ask for DynaRec Block Alloc #%d\n", my_context->mmapsize);
    my_context->mmaplist = (mmaplist_t*)realloc(my_context->mmaplist, my_context->mmapsize*sizeof(mmaplist_t));
    void *p = NULL;
    if(posix_memalign(&p, box86_pagesize, MMAPSIZE)) {
        dynarec_log(LOG_INFO, "Cannot create memory map of %d byte for dynarec block #%d\n", MMAPSIZE, i);
        --my_context->mmapsize;
        return 0;
    }
    mprotect(p, MMAPSIZE, PROT_READ | PROT_WRITE | PROT_EXEC);

    my_context->mmaplist[i].block = p;
    // setup marks
    blockmark_t* m = (blockmark_t*)p;
    m->prev.x32 = 0;
    m->next.fill = 0;
    m->next.size = MMAPSIZE-sizeof(blockmark_t);
    m = (blockmark_t*)(p+MMAPSIZE-sizeof(blockmark_t));
    m->next.x32 = 0;
    m->prev.fill = 0;
    m->prev.size = MMAPSIZE-sizeof(blockmark_t);
    // alloc 1st block
    uintptr_t sub  = (uintptr_t)allocBlock(my_context->mmaplist[i].block, p, size);
    my_context->mmaplist[i].maxfree = getMaxFreeBlock(my_context->mmaplist[i].block);
    kh_dynablocks_t *blocks = my_context->mmaplist[i].dblist = kh_init(dynablocks);
    kh_resize(dynablocks, blocks, 64);
    khint_t k;
    int ret;
    k = kh_put(dynablocks, blocks, (uintptr_t)db, &ret);
    kh_value(blocks, k) = db;
    return sub;
}

void ActuallyFreeDynarecMap(dynablock_t* db, uintptr_t addr, int size)
{
    if(!addr || !size)
        return;
    for(int i=0; i<my_context->mmapsize; ++i) {
        if ((addr>(uintptr_t)my_context->mmaplist[i].block) 
         && (addr<((uintptr_t)my_context->mmaplist[i].block+MMAPSIZE))) {
            void* sub = (void*)(addr-sizeof(blockmark_t));
            freeBlock(my_context->mmaplist[i].block, sub, size);
            my_context->mmaplist[i].maxfree = getMaxFreeBlock(my_context->mmaplist[i].block);
            kh_dynablocks_t *blocks = my_context->mmaplist[i].dblist;
            if(blocks) {
                khint_t k = kh_get(dynablocks, blocks, (uintptr_t)db);
                if(k!=kh_end(blocks))
                    kh_del(dynablocks, blocks, k);
            }
            return;
        }
    }
    if(my_context->mmapsize)
        dynarec_log(LOG_NONE, "Warning, block %p (size %d) not found in mmaplist for Free\n", (void*)addr, size);
}

dynablock_t* FindDynablockFromNativeAddress(void* addr)
{
    // look in actual list
    for(int i=0; i<my_context->mmapsize; ++i) {
        if ((uintptr_t)addr>=(uintptr_t)my_context->mmaplist[i].block 
        && ((uintptr_t)addr<(uintptr_t)my_context->mmaplist[i].block+MMAPSIZE))
            return FindDynablockDynablocklist(addr, my_context->mmaplist[i].dblist);
    }
    // look in oversized
    return FindDynablockDynablocklist(addr, my_context->dblist_oversized);
}

uintptr_t AllocDynarecMap(dynablock_t* db, int size)
{
    if(!size)
        return 0;
    if(size>MMAPSIZE-2*sizeof(blockmark_t)) {
        void *p = NULL;
        if(posix_memalign(&p, box86_pagesize, size)) {
            dynarec_log(LOG_INFO, "Cannot create dynamic map of %d bytes\n", size);
            return 0;
        }
        mprotect(p, size, PROT_READ | PROT_WRITE | PROT_EXEC);
        kh_dynablocks_t *blocks = my_context->dblist_oversized;
        if(!blocks) {
            blocks = my_context->dblist_oversized = kh_init(dynablocks);
            kh_resize(dynablocks, blocks, 64);
        }
        khint_t k;
        int ret;
        k = kh_put(dynablocks, blocks, (uintptr_t)db, &ret);
        kh_value(blocks, k) = db;
        return (uintptr_t)p;
    }
    
    if(pthread_mutex_trylock(&my_context->mutex_mmap)) {
        sched_yield();  // give it a chance
        if(pthread_mutex_trylock(&my_context->mutex_mmap))
            return 0;   // cannot lock, baillout
    }

    uintptr_t ret = FindFreeDynarecMap(db, size);
    if(!ret)
        ret = AddNewDynarecMap(db, size);

    pthread_mutex_unlock(&my_context->mutex_mmap);

    return ret;
}

void FreeDynarecMap(dynablock_t* db, uintptr_t addr, uint32_t size)
{
    if(size>MMAPSIZE) {
        munmap((void*)addr, size);
        kh_dynablocks_t *blocks = my_context->dblist_oversized;
        if(blocks) {
            khint_t k = kh_get(dynablocks, blocks, (uintptr_t)db);
            if(k!=kh_end(blocks))
                kh_del(dynablocks, blocks, k);
        }
        return;
    }
    pthread_mutex_lock(&my_context->mutex_mmap);
    ActuallyFreeDynarecMap(db, addr, size);
    pthread_mutex_unlock(&my_context->mutex_mmap);
}

// each dynmap is 64k of size

void addDBFromAddressRange(box86context_t* context, uintptr_t addr, uintptr_t size, int nolinker)
{
    dynarec_log(LOG_DEBUG, "addDBFromAddressRange %p -> %p\n", (void*)addr, (void*)(addr+size-1));
    uintptr_t idx = (addr>>DYNAMAP_SHIFT);
    uintptr_t end = ((addr+size-1)>>DYNAMAP_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        if(!context->dynmap[i]) {
            context->dynmap[i] = NewDynablockList(i<<DYNAMAP_SHIFT, i<<DYNAMAP_SHIFT, 1<<DYNAMAP_SHIFT, nolinker, 0);
        }
    }
}

void cleanDBFromAddressRange(box86context_t* context, uintptr_t addr, uintptr_t size, int destroy)
{
    dynarec_log(LOG_DEBUG, "cleanDBFromAddressRange %p -> %p %s\n", (void*)addr, (void*)(addr+size-1), destroy?"destroy":"mark");
    uintptr_t idx = (addr>box86_dynarec_largest && !destroy)?((addr-box86_dynarec_largest)>>DYNAMAP_SHIFT):(addr>>DYNAMAP_SHIFT);
    uintptr_t end = ((addr+size-1)>>DYNAMAP_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        dynablocklist_t* dblist = context->dynmap[i];
        if(dblist) {
            uintptr_t startdb = StartDynablockList(dblist);
            uintptr_t enddb = EndDynablockList(dblist);
            uintptr_t startaddr = addr;
            if(startaddr<startdb) startaddr = startdb;
            uintptr_t endaddr = addr + size - 1;
            if(endaddr>enddb) endaddr = enddb;
            if(startaddr==startdb && endaddr==enddb) {
                if(destroy) {
                    context->dynmap[i] = NULL;
                    FreeDynablockList(&dblist);
                } else
                    MarkDynablockList(&dblist);
            } else
                if(destroy)
                    FreeRangeDynablock(dblist, startaddr, endaddr-startaddr+1);
                else
                    MarkRangeDynablock(dblist, startaddr, endaddr-startaddr+1);

        }
    }
}

void updateProtection(uintptr_t addr, uintptr_t size, uint32_t prot)
{
    uintptr_t idx = (addr>>DYNAMAP_SHIFT);
    uintptr_t end = ((addr+size-1)>>DYNAMAP_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        my_context->dynprot[i] = prot;
    }
}
// Remove the Write flag from an adress range, so DB can be executed
// no log, as it can be executed inside a signal handler
void protectDB(uintptr_t addr, uintptr_t size)
{
    uintptr_t idx = (addr>>DYNAMAP_SHIFT);
    uintptr_t end = ((addr+size-1)>>DYNAMAP_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        mprotect((void*)(i<<DYNAMAP_SHIFT), 1<<DYNAMAP_SHIFT, my_context->dynprot[i]&~PROT_WRITE);
    }
}

// Add the Write flag from an adress range, and mark all block as dirty
// no log, as it can be executed inside a signal handler
void unprotectDB(uintptr_t addr, uintptr_t size)
{
    uintptr_t idx = (addr>>DYNAMAP_SHIFT);
    uintptr_t end = ((addr+size-1)>>DYNAMAP_SHIFT);
    for (uintptr_t i=idx; i<=end; ++i) {
        mprotect((void*)(i<<DYNAMAP_SHIFT), 1<<DYNAMAP_SHIFT, my_context->dynprot[i]);
    }
}

#endif

EXPORTDYN
void initAllHelpers(box86context_t* context)
{
    static int inited = 0;
    if(inited)
        return;
    my_context = context;
    init_pthread_helper();
    init_signal_helper(context);
    #ifdef DYNAREC
    if(box86_dynarec)
        DynablockEmuMarker(context);
    #endif
    inited = 1;
}

EXPORTDYN
void finiAllHelpers(box86context_t* context)
{
    static int finied = 0;
    if(finied)
        return;
    fini_pthread_helper(context);
    fini_signal_helper();
    cleanAlternate();
    finied = 1;
}

void x86Syscall(x86emu_t *emu);

/// maxval not inclusive
int getrand(int maxval)
{
    if(maxval<1024) {
        return ((random()&0x7fff)*maxval)/0x7fff;
    } else {
        uint64_t r = random();
        r = (r*maxval) / RAND_MAX;
        return r;
    }
}

void free_tlsdatasize(void* p)
{
    if(!p)
        return;
    tlsdatasize_t *data = (tlsdatasize_t*)p;
    free(data->tlsdata);
    free(p);
}

EXPORTDYN
box86context_t *NewBox86Context(int argc)
{
#ifdef BUILD_DYNAMIC
    if(my_context) {
        ++my_context->count;
        return my_context;
    }
#endif
    // init and put default values
    box86context_t *context = (box86context_t*)calloc(1, sizeof(box86context_t));

#ifdef BUILD_LIB
    context->deferedInit = 0;
#else
    context->deferedInit = 1;
#endif
    context->sel_serial = 1;

#ifdef DYNAREC
    pthread_mutex_init(&context->mutex_blocks, NULL);
    pthread_mutex_init(&context->mutex_mmap, NULL);
    context->dynmap = (dynablocklist_t**)calloc(DYNAMAP_SIZE, sizeof(dynablocklist_t*));
    context->dynprot = (uint32_t*)calloc(DYNAMAP_SIZE, sizeof(uint32_t));
#endif

    context->maplib = NewLibrarian(context, 1);
    context->local_maplib = NewLibrarian(context, 1);
    context->system = NewBridge();
    // create vsyscall
    context->vsyscall = AddBridge(context->system, context, vFv, x86Syscall, 0);
#ifdef BUILD_LIB
    context->box86lib = RTLD_DEFAULT;   // not ideal
#else
    context->box86lib = dlopen(NULL, RTLD_NOW|RTLD_GLOBAL);
#endif
    context->dlprivate = NewDLPrivate();

    context->argc = argc;
    context->argv = (char**)calloc(context->argc+1, sizeof(char*));

    pthread_mutex_init(&context->mutex_once, NULL);
    pthread_mutex_init(&context->mutex_once2, NULL);
    pthread_mutex_init(&context->mutex_trace, NULL);
#ifndef DYNAREC
    pthread_mutex_init(&context->mutex_lock, NULL);
#endif
    pthread_mutex_init(&context->mutex_tls, NULL);
    pthread_mutex_init(&context->mutex_thread, NULL);
#ifdef DYNAREC
    pthread_mutex_init(&context->mutex_dyndump, NULL);
#endif
    pthread_key_create(&context->tlskey, free_tlsdatasize);

    InitFTSMap(context);

    for (int i=0; i<4; ++i) context->canary[i] = 1 +  getrand(255);
    context->canary[getrand(4)] = 0;
    printf_log(LOG_DEBUG, "Setting up canary (for Stack protector) at GS:0x14, value:%08X\n", *(uint32_t*)context->canary);

    initAllHelpers(context);

    return context;
}

EXPORTDYN
void FreeBox86Context(box86context_t** context)
{
#ifdef BUILD_DYNAMIC
    if(--(*context)->count)
        return;
#endif
    if(!context)
        return;
    
    if(--(*context)->forked >= 0)
        return;

    box86context_t* ctx = *context;   // local copy to do the cleanning

    for(int i=0; i<ctx->elfsize; ++i) {
        FreeElfHeader(&ctx->elfs[i]);
    }
    free(ctx->elfs);

    FreeFTSMap(ctx);

    if(ctx->maplib)
        FreeLibrarian(&ctx->maplib);
    if(ctx->local_maplib)
        FreeLibrarian(&ctx->local_maplib);

    FreeCollection(&ctx->box86_path);
    FreeCollection(&ctx->box86_ld_lib);
    FreeCollection(&ctx->box86_emulated_libs);
    // stop trace now
    if(ctx->dec)
        DeleteX86TraceDecoder(&ctx->dec);
    if(ctx->zydis)
        DeleteX86Trace(ctx);

    if(ctx->deferedInitList)
        free(ctx->deferedInitList);

    free(ctx->argv);
    
    for (int i=0; i<ctx->envc; ++i)
        free(ctx->envv[i]);
    free(ctx->envv);

    if(ctx->atfork_sz) {
        free(ctx->atforks);
        ctx->atforks = NULL;
        ctx->atfork_sz = ctx->atfork_cap = 0;
    }

    for(int i=0; i<MAX_SIGNAL; ++i)
        if(ctx->signals[i]!=0 && ctx->signals[i]!=1) {
            signal(i, SIG_DFL);
        }

#ifdef DYNAREC
    dynarec_log(LOG_DEBUG, "Free global Dynarecblocks\n");
    for (int i=0; i<ctx->mmapsize; ++i) {
        if(ctx->mmaplist[i].block)
            free(ctx->mmaplist[i].block);
        if(ctx->mmaplist[i].dblist) {
            kh_destroy(dynablocks, ctx->mmaplist[i].dblist);
            ctx->mmaplist[i].dblist = NULL;
        }
    }
    if(ctx->dblist_oversized) {
        kh_destroy(dynablocks, ctx->dblist_oversized);
        ctx->dblist_oversized = NULL;
    }
    ctx->mmapsize = 0;
    dynarec_log(LOG_DEBUG, "Free dynamic Dynarecblocks\n");
    cleanDBFromAddressRange(ctx, 0, 0xffffffff, 1);
    pthread_mutex_destroy(&ctx->mutex_blocks);
    pthread_mutex_destroy(&ctx->mutex_mmap);
    free(ctx->mmaplist);
#endif
    
    *context = NULL;                // bye bye my_context

    CleanStackSize(ctx);

#ifndef BUILD_LIB
    if(ctx->box86lib)
        dlclose(ctx->box86lib);
#endif

    FreeDLPrivate(&ctx->dlprivate);

    free(ctx->stack);

    free(ctx->fullpath);
    free(ctx->box86path);

    FreeBridge(&ctx->system);

    freeGLProcWrapper(ctx);
    freeALProcWrapper(ctx);

    void* ptr;
    if ((ptr = pthread_getspecific(ctx->tlskey)) != NULL) {
        free_tlsdatasize(ptr);
        pthread_setspecific(ctx->tlskey, NULL);
    }
    pthread_key_delete(ctx->tlskey);

    if(ctx->tlsdata)
        free(ctx->tlsdata);

    for(int i=0; i<3; ++i) {
        if(ctx->segtls[i].present) {
            // key content not handled by box86, so not doing anything with it
            pthread_key_delete(ctx->segtls[i].key);
        }
    }

    pthread_mutex_destroy(&ctx->mutex_once);
    pthread_mutex_destroy(&ctx->mutex_once2);
    pthread_mutex_destroy(&ctx->mutex_trace);
#ifndef DYNAREC
    pthread_mutex_destroy(&ctx->mutex_lock);
#endif
    pthread_mutex_destroy(&ctx->mutex_tls);
    pthread_mutex_destroy(&ctx->mutex_thread);
#ifdef DYNAREC
    pthread_mutex_destroy(&ctx->mutex_dyndump);
    free(ctx->dynmap);
    ctx->dynmap = NULL;
    free(ctx->dynprot);
    ctx->dynprot = NULL;
#endif

    free_neededlib(&ctx->neededlibs);

    if(ctx->emu_sig)
        FreeX86Emu(&ctx->emu_sig);

    finiAllHelpers(ctx);

    free(ctx);
}

int AddElfHeader(box86context_t* ctx, elfheader_t* head) {
    int idx = ctx->elfsize;
    if(idx==ctx->elfcap) {
        // resize...
        ctx->elfcap += 16;
        ctx->elfs = (elfheader_t**)realloc(ctx->elfs, sizeof(elfheader_t*) * ctx->elfcap);
    }
    ctx->elfs[idx] = head;
    ctx->elfsize++;
    printf_log(LOG_DEBUG, "Adding \"%s\" as #%d in elf collection\n", ElfName(head), idx);
    return idx;
}

int AddTLSPartition(box86context_t* context, int tlssize) {
    int oldsize = context->tlssize;
    context->tlssize += tlssize;
    context->tlsdata = realloc(context->tlsdata, context->tlssize);
    memmove(context->tlsdata+tlssize, context->tlsdata, oldsize);   // move to the top, using memmove as regions will probably overlap
    memset(context->tlsdata, 0, tlssize);           // fill new space with 0 (not mandatory)
    // clean GS segment for current emu
    if(my_context) {
        ResetSegmentsCache(thread_get_emu());
        if(!(++context->sel_serial))
            ++context->sel_serial;
    }

    return -context->tlssize;   // negative offset
}

void add_neededlib(needed_libs_t* needed, library_t* lib)
{
    if(!needed)
        return;
    if(needed->size == needed->cap) {
        needed->cap += 8;
        needed->libs = (library_t**)realloc(needed->libs, needed->cap*sizeof(library_t*));
    }
    needed->libs[needed->size++] = lib;
}

void free_neededlib(needed_libs_t* needed)
{
    if(!needed)
        return;
    needed->cap = 0;
    needed->size = 0;
    if(needed->libs)
        free(needed->libs);
    needed->libs = NULL;
}
