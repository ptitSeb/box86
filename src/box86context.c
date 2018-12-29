#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "box86context.h"
#include "elfloader.h"
#include "debug.h"
#include "x86trace.h"
#include "x86emu.h"
#include "librarian.h"
#include "bridge.h"

void x86Syscall(x86emu_t *emu);

box86context_t *NewBox86Context(int argc)
{
    // init and put default values
    box86context_t *context = (box86context_t*)calloc(1, sizeof(box86context_t));

    context->maplib = NewLibrarian();
    context->system = NewBridge();
    // create vsyscall
    context->vsyscall = AddBridge(context->system, vFv, x86Syscall);

    context->box86lib = dlopen(NULL, RTLD_NOW|RTLD_GLOBAL);
    context->dlprivate = NewDLPrivate();

    context->argc = argc;
    context->argv = (char**)calloc(context->argc, sizeof(char*));

    return context;
}

void FreeBox86Context(box86context_t** context)
{
    if(!context)
        return;

    FreeCollection(&(*context)->box86_path);
    FreeCollection(&(*context)->box86_ld_lib);
    if((*context)->zydis)
        DeleteX86Trace(*context);

    if((*context)->box86lib)
        dlclose((*context)->box86lib);

    FreeDLPrivate(&(*context)->dlprivate);

    if((*context)->emu)
        FreeX86Emu(&(*context)->emu);

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
