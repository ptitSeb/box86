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

void x86Syscall(x86emu_t *emu);

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

    return context;
}

void FreeBox86Context(box86context_t** context)
{
    if(!context)
        return;
    
    if(--(*context)->forked >= 0)
        return;

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

    if((*context)->maplib)
        FreeLibrarian(&(*context)->maplib);
    
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

    FreeBridge(&(*context)->system);

    if((*context)->glwrappers)
        freeProcWrapper(&(*context)->glwrappers);
    if((*context)->alwrappers)
        freeProcWrapper(&(*context)->alwrappers);

    FreeCallbackList(&(*context)->callbacks);

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
