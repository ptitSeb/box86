#ifndef __BOX86CONTEXT_H_
#define __BOX86CONTEXT_H_

#include "pathcoll.h"

typedef struct {
    path_collection_t   box86_path;     // PATH env. variable
    path_collection_t   box86_ld_lib;   // LD_LIBRARY_PATH env. variable

    int                 argc;
    char**              argv;

} box86context_t;

box86context_t *NewBox86Context(int argc);
box86context_t *CopyBox86Context(box86context_t* from);
void FreeBox86Context(box86context_t** context);

#endif //__BOX86CONTEXT_H_