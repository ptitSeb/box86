#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "box86context.h"

box86context_t *NewBox86Context(int argc)
{
    // init and put default values
    box86context_t *context = (box86context_t*)calloc(1, sizeof(box86context_t));

    context->argc = argc;
    context->argv = (char**)calloc(context->argc, sizeof(char*));

    return context;
}

box86context_t *CopyBox86Context(box86context_t* from)
{
    box86context_t *context = (box86context_t*)malloc(sizeof(box86context_t));

    CopyCollection(&context->box86_path, &from->box86_path);
    CopyCollection(&context->box86_ld_lib, &context->box86_ld_lib);

    context->argc = from->argc;
    context->argv = (char**)calloc(context->argc, sizeof(char*));
    for (int i=0; i<context->argc; ++i)
        if(from->argv[i])
            context->argv[i] = strdup(from->argv[i]);

    return context;
}

void FreeBox86Context(box86context_t** context)
{
    if(!context)
        return;

    FreeCollection(&(*context)->box86_path);
    FreeCollection(&(*context)->box86_ld_lib);

    for(int i=0; i<(*context)->argc; ++i)
        free((*context)->argv[i]);
    free((*context)->argv);

    free(*context);
    *context = NULL;
}

