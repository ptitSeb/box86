#ifndef __BOX86CONTEXT_H_
#define __BOX86CONTEXT_H_
#include <stdint.h>
#include "pathcoll.h"

typedef struct elfheader_s elfheader_t;

typedef struct {
    path_collection_t   box86_path;     // PATH env. variable
    path_collection_t   box86_ld_lib;   // LD_LIBRARY_PATH env. variable

    int                 argc;
    char**              argv;

    uint32_t            stacksz;
    int                 stackalign;
    void*               stack;          // alocated stack

    elfheader_t         **elfs;         // elf headers and memory
    int                 elfcap;
    int                 elfsize;        // number of elf loaded

} box86context_t;

box86context_t *NewBox86Context(int argc);
box86context_t *CopyBox86Context(box86context_t* from);
void FreeBox86Context(box86context_t** context);

int AddElfHeader(box86context_t* ctx, elfheader_t* head);    // return the index of header

#endif //__BOX86CONTEXT_H_