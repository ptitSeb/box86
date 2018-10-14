#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "box86context.h"

box86context_t *NewBox86Context()
{
    // init and put default values
    box86context_t *context = (box86context_t*)calloc(1, sizeof(box86context_t));

    return context;
}

box86context_t *CopyBox86Context(box86context_t* from)
{
    box86context_t *context = (box86context_t*)malloc(sizeof(box86context_t));

    CopyCollection(&context->box86_path, &from->box86_path);
    CopyCollection(&context->box86_ld_lib, &context->box86_ld_lib);

    return context;
}

void FreeBox86Context(box86context_t** context)
{
    if(!context)
        return;

    FreeCollection(&(*context)->box86_path);
    FreeCollection(&(*context)->box86_ld_lib);

    free(*context);
    *context = NULL;
}

