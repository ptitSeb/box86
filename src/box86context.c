#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
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
#ifdef DYNAREC
#include <sys/mman.h>
#include "dynablock.h"
#include "wrapper.h"

typedef struct mmaplist_s {
    void*         block;
    uintptr_t     offset;           // offset in the block
} mmaplist_t;

#define MMAPSIZE (4*1024*1024)       // allocate 4Mo sized blocks

uintptr_t AllocDynarecMap(box86context_t *context, int size)
{
    // make size 0x10 bytes aligned
    size = (size+0x0f)&~0x0f;
    pthread_mutex_lock(&context->mutex_mmap);
    // look for free space
    for(int i=0; i<context->mmapsize; ++i) {
        if(context->mmaplist[i].offset+size < MMAPSIZE) {
            uintptr_t ret = context->mmaplist[i].offset + (uintptr_t)context->mmaplist[i].block;
            context->mmaplist[i].offset+=size;
            pthread_mutex_unlock(&context->mutex_mmap);
            return ret;
        }
    }
    // no luck, add a new one !
    int i = context->mmapsize++;    // yeah, usefull post incrementation
    dynarec_log(LOG_DEBUG, "Ask for DynaRec Block Alloc #%d\n", context->mmapsize);
    context->mmaplist = (mmaplist_t*)realloc(context->mmaplist, context->mmapsize*sizeof(mmaplist_t));
    void* p = mmap(NULL, MMAPSIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(p==MAP_FAILED) {
        dynarec_log(LOG_INFO, "Cannot create memory map of %d byte for dynarec block #%d\n", MMAPSIZE, i);
        --context->mmapsize;
        pthread_mutex_unlock(&context->mutex_mmap);
        return 0;
    }
    context->mmaplist[i].block = p;
    context->mmaplist[i].offset=size;
    pthread_mutex_unlock(&context->mutex_mmap);
    return (uintptr_t)p;
}
#endif

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

box86context_t *NewBox86Context(int argc)
{
    // init and put default values
    box86context_t *context = (box86context_t*)calloc(1, sizeof(box86context_t));

    context->deferedInit = 1;

    context->maplib = NewLibrarian(context);
    context->system = NewBridge();
    // create vsyscall
    context->vsyscall = AddBridge(context->system, vFv, x86Syscall, 0);

    context->box86lib = dlopen(NULL, RTLD_NOW|RTLD_GLOBAL);
    context->dlprivate = NewDLPrivate();

    context->callbacks = NewCallbackList();

    context->argc = argc;
    context->argv = (char**)calloc(context->argc, sizeof(char*));

    pthread_mutex_init(&context->mutex_once, NULL);
    pthread_mutex_init(&context->mutex_once2, NULL);
    pthread_mutex_init(&context->mutex_trace, NULL);
    pthread_mutex_init(&context->mutex_lock, NULL);

    pthread_key_create(&context->tlskey, free);

#ifdef DYNAREC
    pthread_mutex_init(&context->mutex_blocks, NULL);
    pthread_mutex_init(&context->mutex_mmap, NULL);
    context->dynablocks = NewDynablockList(0, 0, 0);
#endif

    for (int i=0; i<4; ++i) context->canary[i] = 1 +  getrand(255);
    context->canary[getrand(4)] = 0;
    printf_log(LOG_DEBUG, "Setting up canary (for Stack protector) at GS:0x14, value:%08X\n", *(uint32_t*)context->canary);

    return context;
}

void FreeBox86Context(box86context_t** context)
{
    if(!context)
        return;
    
    if(--(*context)->forked >= 0)
        return;

    if((*context)->maplib)
        FreeLibrarian(&(*context)->maplib);

#ifdef DYNAREC
    dynarec_log(LOG_INFO, "Free global Dynarecblocks\n");
    if((*context)->dynablocks)
        FreeDynablockList(&(*context)->dynablocks);
    for (int i=0; i<(*context)->mmapsize; ++i)
        if((*context)->mmaplist[i].block)
            munmap((*context)->mmaplist[i].block, MMAPSIZE);
    free((*context)->mmaplist);
    pthread_mutex_destroy(&(*context)->mutex_blocks);
    pthread_mutex_destroy(&(*context)->mutex_mmap);
#endif
    
    if((*context)->emu)
        FreeX86Emu(&(*context)->emu);

    FreeCollection(&(*context)->box86_path);
    FreeCollection(&(*context)->box86_ld_lib);
    if((*context)->zydis)
        DeleteX86Trace(*context);

    if((*context)->deferedInitList)
        free((*context)->deferedInitList);

    if((*context)->box86lib)
        dlclose((*context)->box86lib);

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

    FreeBridge(&(*context)->system);

    if((*context)->glwrappers)
        freeProcWrapper(&(*context)->glwrappers);
    if((*context)->alwrappers)
        freeProcWrapper(&(*context)->alwrappers);

    FreeCallbackList(&(*context)->callbacks);

    void* ptr;
    if ((ptr = pthread_getspecific((*context)->tlskey)) != NULL) {
        free(ptr);
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
    if((*context)->signal_emu)
        FreeX86Emu(&(*context)->signal_emu);


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
    context->tlssize += tlssize;
    context->tlsdata = realloc(context->tlsdata, context->tlssize);
    return -context->tlssize;   // negative offset
}