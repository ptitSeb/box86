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
#ifdef DYNAREC
#include <sys/mman.h>
#include "dynablock.h"

#define MMAPSIZE (4*1024*1024)      // allocate 4Mo sized blocks
#define MMAPBLOCK   256             // minimum size of a block

typedef struct mmaplist_s {
    void*         block;
    uint8_t       map[MMAPSIZE/(8*MMAPBLOCK)];  // map of allocated sub-block
} mmaplist_t;

// get first subblock free in map, stating at start. return -1 if no block, else first subblock free, filling size (in subblock unit)
static int getFirstBlock(mmaplist_t *map, int start, int maxsize, int* size)
{
    #define ISFREE(A) (((map->map[(A)>>3]>>((A)&7))&1)?0:1)
    if(start<0) start = 0;
    while(start<MMAPSIZE/(8*MMAPBLOCK)) {   // still a chance...
        if(ISFREE(start)) {
            // free, now get size...
            int end = start+1;
            while(end<MMAPSIZE/(8*MMAPBLOCK)) {
                if(!ISFREE(end) || (end-start==maxsize)) {
                    if(size) *size = end-start;
                    return start;
                }
                ++end;
            }
            if(size) *size = end-start;
            return start;
        }
        ++start;
    }
    return -1;
}

static void allocBlock(mmaplist_t *map, int start, int size)
{
    for(int i=0; i<size; ++i) {
        map->map[start/8]|=(1<<(start&7));
        ++start;
    }
}
static void freeBlock(mmaplist_t *map, int start, int size)
{
    for(int i=0; i<size; ++i) {
        map->map[start/8]&=~(1<<(start&7));
        ++start;
    }
}

uintptr_t AllocDynarecMap(int size, int nolinker)
{
    if(size>MMAPSIZE) {
        void* p = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if(p==MAP_FAILED) {
            dynarec_log(LOG_DEBUG, "Cannot create dynamic map of %d bytes\n", size);
            return 0;
        }
        return (uintptr_t)p;
    }
    pthread_mutex_lock(&my_context->mutex_mmap);
    int bsize = (size+MMAPBLOCK-1)/MMAPBLOCK;
    // look for free space
    for(int i=0; i<my_context->mmapsize; ++i) {
        int rsize = 0;
        int start = 0;
        do {
            start = getFirstBlock(my_context->mmaplist+i, start, bsize, &rsize);
            if(start!=-1 && rsize>=bsize) {
                allocBlock(my_context->mmaplist+i, start, bsize);
                uintptr_t ret = (uintptr_t)my_context->mmaplist[i].block + start*MMAPBLOCK;
                pthread_mutex_unlock(&my_context->mutex_mmap);
                return ret;
            }
            if(start!=-1)
                start += rsize;
        } while (start!=-1);
    }
    // no luck, add a new one !
    int i = my_context->mmapsize++;    // yeah, useful post incrementation
    dynarec_log(LOG_DEBUG, "Ask for DynaRec Block Alloc #%d\n", my_context->mmapsize);
    my_context->mmaplist = (mmaplist_t*)realloc(my_context->mmaplist, my_context->mmapsize*sizeof(mmaplist_t));
    void* p = mmap(NULL, MMAPSIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(p==MAP_FAILED) {
        dynarec_log(LOG_DEBUG, "Cannot create memory map of %d byte for dynarec block #%d\n", MMAPSIZE, i);
        --my_context->mmapsize;
        pthread_mutex_unlock(&my_context->mutex_mmap);
        return 0;
    }
    my_context->mmaplist[i].block = p;
    memset(my_context->mmaplist[i].map, 0, sizeof(my_context->mmaplist[i].map));
    allocBlock(my_context->mmaplist+i, 0, bsize);
    pthread_mutex_unlock(&my_context->mutex_mmap);
    return (uintptr_t)p;
}

void FreeDynarecMap(uintptr_t addr, uint32_t size)
{
    if(size>MMAPSIZE) {
        munmap((void*)addr, size);
        return;
    }
    pthread_mutex_lock(&my_context->mutex_mmap);
    int bsize = (size+MMAPBLOCK-1)/MMAPBLOCK;
    // look for free space
    for(int i=0; i<my_context->mmapsize; ++i) {
        if(addr>=(uintptr_t)my_context->mmaplist[i].block && ((uintptr_t)my_context->mmaplist[i].block+MMAPSIZE)>=addr+size) {
            int start = (addr - (uintptr_t)my_context->mmaplist[i].block) / MMAPBLOCK;
            freeBlock(my_context->mmaplist+i, start, bsize);
            pthread_mutex_unlock(&my_context->mutex_mmap);
            return;
        }
    }
    pthread_mutex_unlock(&my_context->mutex_mmap);
}

// each dynmap is 64k of size
typedef struct dynmap_s {
    dynablocklist_t* dynablocks;    // the dynabockist of the block
} dynmap_t;

dynablocklist_t* getDBFromAddress(uintptr_t addr)
{
    int idx = (addr>>16);
    if(!my_context->dynmap[idx]) {
        return NULL;
    }
    return my_context->dynmap[idx]->dynablocks;
}

void addDBFromAddressRange(uintptr_t addr, uintptr_t size)
{
    dynarec_log(LOG_DEBUG, "addDBFromAddressRange %p -> %p\n", (void*)addr, (void*)(addr+size));
    int idx = (addr>>16);
    int end = ((addr+size)>>16);
    for (int i=idx; i<=end; ++i) {
        if(!my_context->dynmap[i]) {
            my_context->dynmap[i] = (dynmap_t*)calloc(1, sizeof(dynmap_t));
            my_context->dynmap[i]->dynablocks = NewDynablockList(0, i<<16, 65536, 1, 1);
        }
    }
}

void cleanDBFromAddressRange(uintptr_t addr, uintptr_t size)
{
    dynarec_log(LOG_DEBUG, "cleanDBFromAddressRange %p -> %p\n", (void*)addr, (void*)(addr+size));
    int idx = (addr>>16);
    int end = ((addr+size)>>16);
    for (int i=idx; i<=end; ++i) {
        dynmap_t* dynmap = my_context->dynmap[i];
        if(dynmap) {
            uintptr_t startdb = StartDynablockList(dynmap->dynablocks);
            uintptr_t enddb = EndDynablockList(dynmap->dynablocks);
            if(addr<=startdb && (addr+size)>=enddb) {
                my_context->dynmap[i] = NULL;
                FreeDynablockList(&dynmap->dynablocks);
                free(dynmap);
            } else
                FreeDirectDynablock(dynmap->dynablocks, addr, addr+size);
        }
    }
}

#endif

void initAllHelpers(box86context_t* context)
{
    static int inited = 0;
    if(inited)
        return;
    my_context = context;
    init_pthread_helper();
    inited = 1;
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

box86context_t *NewBox86Context(int argc)
{
    // init and put default values
    box86context_t *context = (box86context_t*)calloc(1, sizeof(box86context_t));

    context->deferedInit = 1;

    context->maplib = NewLibrarian(context);
    context->system = NewBridge();
    // create vsyscall
    context->vsyscall = AddBridge(context->system, vFv, x86Syscall, 0);
#ifdef BUILD_LIB
    context->box86lib = RTLD_DEFAULT;   // not ideal
#else
    context->box86lib = dlopen(NULL, RTLD_NOW|RTLD_GLOBAL);
#endif
    context->dlprivate = NewDLPrivate();

    context->callbacks = NewCallbackList();

    context->argc = argc;
    context->argv = (char**)calloc(context->argc+1, sizeof(char*));

    pthread_mutex_init(&context->mutex_once, NULL);
    pthread_mutex_init(&context->mutex_once2, NULL);
    pthread_mutex_init(&context->mutex_trace, NULL);
    pthread_mutex_init(&context->mutex_lock, NULL);

    pthread_key_create(&context->tlskey, free_tlsdatasize);

#ifdef DYNAREC
    pthread_mutex_init(&context->mutex_blocks, NULL);
    pthread_mutex_init(&context->mutex_mmap, NULL);
    context->dynablocks = NewDynablockList(0, 0, 0, 0, 0);
#endif
    InitFTSMap(context);

    for (int i=0; i<4; ++i) context->canary[i] = 1 +  getrand(255);
    context->canary[getrand(4)] = 0;
    printf_log(LOG_DEBUG, "Setting up canary (for Stack protector) at GS:0x14, value:%08X\n", *(uint32_t*)context->canary);

    initAllHelpers(context);

    return context;
}

void FreeBox86Context(box86context_t** context)
{
    if(!context)
        return;
    
    if(--(*context)->forked >= 0)
        return;

    FreeFTSMap(*context);

    if((*context)->maplib)
        FreeLibrarian(&(*context)->maplib);

#ifdef DYNAREC
    dynarec_log(LOG_DEBUG, "Free global Dynarecblocks\n");
    if((*context)->dynablocks)
        FreeDynablockList(&(*context)->dynablocks);
    for (int i=0; i<(*context)->mmapsize; ++i)
        if((*context)->mmaplist[i].block)
            munmap((*context)->mmaplist[i].block, MMAPSIZE);
    free((*context)->mmaplist);
    pthread_mutex_destroy(&(*context)->mutex_blocks);
    pthread_mutex_destroy(&(*context)->mutex_mmap);
    dynarec_log(LOG_DEBUG, "Free dynamic Dynarecblocks\n");
    cleanDBFromAddressRange(0, 0xffffffff);
#endif
    
    if((*context)->emu)
        FreeX86Emu(&(*context)->emu);

    CleanStackSize(*context);
    FreeCollection(&(*context)->box86_path);
    FreeCollection(&(*context)->box86_ld_lib);
    FreeCollection(&(*context)->box86_emulated_libs);
    if((*context)->zydis)
        DeleteX86Trace(*context);

    if((*context)->deferedInitList)
        free((*context)->deferedInitList);

#ifndef BUILD_LIB
    if((*context)->box86lib)
        dlclose((*context)->box86lib);
#endif

    FreeDLPrivate(&(*context)->dlprivate);

    for(int i=0; i<(*context)->argc; ++i)
        free((*context)->argv[i]);
    free((*context)->argv);
    
    for (int i=0; i<(*context)->envc; ++i)
        free((*context)->envv[i]);
    free((*context)->envv);

    for(int i=0; i<(*context)->elfsize; ++i) {
        FreeElfHeader(&(*context)->elfs[i]);
    }
    free((*context)->elfs);

    free((*context)->stack);

    free((*context)->fullpath);
    free((*context)->box86path);

    FreeBridge(&(*context)->system);

    freeGLProcWrapper(*context);
    freeALProcWrapper(*context);

    FreeCallbackList(&(*context)->callbacks);

    void* ptr;
    if ((ptr = pthread_getspecific((*context)->tlskey)) != NULL) {
        free_tlsdatasize(ptr);
        pthread_setspecific((*context)->tlskey, NULL);
    }
    pthread_key_delete((*context)->tlskey);

    if((*context)->tlsdata)
        free((*context)->tlsdata);

    pthread_mutex_destroy(&(*context)->mutex_once);
    pthread_mutex_destroy(&(*context)->mutex_once2);
    pthread_mutex_destroy(&(*context)->mutex_trace);
    pthread_mutex_destroy(&(*context)->mutex_lock);

    if((*context)->atfork_sz) {
        free((*context)->atforks);
        (*context)->atforks = NULL;
        (*context)->atfork_sz = (*context)->atfork_cap = 0;
    }

    for(int i=0; i<MAX_SIGNAL; ++i)
        if((*context)->signals[i]!=0 && (*context)->signals[i]!=1) {
            signal(i, SIG_DFL);
        }


    free(*context);
    *context = NULL;
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
    return -context->tlssize;   // negative offset
}