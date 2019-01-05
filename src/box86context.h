#ifndef __BOX86CONTEXT_H_
#define __BOX86CONTEXT_H_
#include <stdint.h>
#include "pathcoll.h"

typedef struct elfheader_s elfheader_t;

typedef struct x86emu_s x86emu_t;
typedef struct zydis_s zydis_t;
typedef struct lib_s lib_t;
typedef struct bridge_s bridge_t;
typedef struct dlprivate_s dlprivate_t;
typedef struct kh_symbolmap_s kh_symbolmap_t;
typedef struct callbacklist_s callbacklist_t;
typedef struct library_s library_t;

typedef void* (*procaddess_t)(const char* name);

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
    dlprivate_t         *dlprivate;     // dlopen library map
    kh_symbolmap_t      *glwrappers;    // the map of wrapper for glProcs (for GLX or SDL1/2)
    procaddess_t        glxprocaddress;

    callbacklist_t      *callbacks;     // all callbacks

    library_t           *sdl1lib;       // shortcut to SDL1 library (if loaded)
    library_t           *sdl1mixerlib;
    library_t           *sdl1imagelib;
    library_t           *sdl2lib;       // shortcut to SDL2 library (if loaded)
    library_t           *sdl2mixerlib;
    library_t           *sdl2imagelib;
} box86context_t;

box86context_t *NewBox86Context(int argc);
void FreeBox86Context(box86context_t** context);

int AddElfHeader(box86context_t* ctx, elfheader_t* head);    // return the index of header

#endif //__BOX86CONTEXT_H_