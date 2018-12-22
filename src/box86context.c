#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "box86context.h"
#include "elfloader.h"
#include "debug.h"
#include "x86trace.h"
#include "x86emu.h"
#include "librarian.h"
#include "bridge.h"

box86context_t *NewBox86Context(int argc)
{
    // init and put default values
    box86context_t *context = (box86context_t*)calloc(1, sizeof(box86context_t));

    context->maplib = NewLibrarian();

    context->argc = argc;
    context->argv = (char**)calloc(context->argc, sizeof(char*));

    return context;
}

box86context_t *CopyBox86Context(box86context_t* from)
{
    printf("ERROR: CopyBox86Context needs more works!");

    box86context_t *context = (box86context_t*)malloc(sizeof(box86context_t));

    context->maplib = NewLibrarian();   // todo!

    CopyCollection(&context->box86_path, &from->box86_path);
    CopyCollection(&context->box86_ld_lib, &context->box86_ld_lib);

    context->argc = from->argc;
    context->argv = (char**)calloc(context->argc, sizeof(char*));
    for (int i=0; i<context->argc; ++i)
        if(from->argv[i])
            context->argv[i] = strdup(from->argv[i]);
    context->envc = from->envc;
    context->envv = (char**)calloc(context->envc, sizeof(char*));
    for (int i=0; i<context->envc; ++i)
        if(from->envv[i])
            context->envv[i] = strdup(from->envv[i]);

    // x86emu TODO!

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
