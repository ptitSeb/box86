#ifndef __BOX86CONTEXT_H_
#define __BOX86CONTEXT_H_
#include <stdint.h>
#include "pathcoll.h"

typedef struct elfheader_s elfheader_t;

typedef struct x86emu_s x86emu_t;
typedef struct zydis_s zydis_t;
typedef struct lib_s lib_t;
typedef struct bridge_s bridge_t;

typedef struct box86context_s {
    path_collection_t   box86_path;     // PATH env. variable
    path_collection_t   box86_ld_lib;   // LD_LIBRARY_PATH env. variable

    int                 x86trace;
    zydis_t             *zydis;         // dlopen the zydis dissasembler
    void*               box86lib;       // dlopen on box86 itself

    int                 argc;
    char**              argv;

    int                 envc;
    char**              envv;

    uint32_t            stacksz;
    int                 stackalign;
    void*               stack;          // alocated stack

    elfheader_t         **elfs;         // elf headers and memory
    int                 elfcap;
    int                 elfsize;        // number of elf loaded

    uintptr_t           ep;             // entry point

    x86emu_t            *emu;           // CPU / FPU / MMX&SSE regs

    lib_t               *maplib;        // lib and symbols handling

    bridge_t            *threads;       // threads
    bridge_t            *system;        // other bridges
    uintptr_t           vsyscall;       // vsyscall bridge value

} box86context_t;

box86context_t *NewBox86Context(int argc);
box86context_t *CopyBox86Context(box86context_t* from);
void FreeBox86Context(box86context_t** context);

int AddElfHeader(box86context_t* ctx, elfheader_t* head);    // return the index of header

#endif //__BOX86CONTEXT_H_