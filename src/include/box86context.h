#ifndef __BOX86CONTEXT_H_
#define __BOX86CONTEXT_H_
#include <stdint.h>
#include <pthread.h>
#include "pathcoll.h"
#include "dictionnary.h"
#ifdef DYNAREC
#include "dynarec/arm_lock_helper.h"
#endif
#ifdef DYNAREC
// disabling for now, seems to have a negative impact on performances
//#define USE_CUSTOM_MUTEX
#endif

typedef struct elfheader_s elfheader_t;
typedef struct cleanup_s cleanup_t;
typedef struct x86emu_s x86emu_t;
typedef struct zydis_s zydis_t;
typedef struct lib_s lib_t;
typedef struct bridge_s bridge_t;
typedef struct dlprivate_s dlprivate_t;
typedef struct kh_symbolmap_s kh_symbolmap_t;
typedef struct kh_defaultversion_s kh_defaultversion_t;
typedef struct kh_mapsymbols_s kh_mapsymbols_t;
typedef struct library_s library_t;
typedef struct linkmap_s linkmap_t;
typedef struct kh_fts_s kh_fts_t;
typedef struct kh_threadstack_s kh_threadstack_t;
typedef struct zydis_dec_s zydis_dec_t;
typedef struct atfork_fnc_s {
    uintptr_t prepare;
    uintptr_t parent;
    uintptr_t child;
    void*     handle;
} atfork_fnc_t;
#ifdef DYNAREC
typedef struct dynablock_s      dynablock_t;
typedef struct mmaplist_s       mmaplist_t;
typedef struct kh_dynablocks_s  kh_dynablocks_t;
#endif
#define DYNAMAP_SHIFT 16
#define DYNAMAP_SIZE (1<<(32-DYNAMAP_SHIFT))
#define JMPTABL_SHIFT 16
#define JMPTABL_SIZE (1<<(32-JMPTABL_SHIFT))
#define JMPTABLE_MASK ((1<<JMPTABL_SHIFT)-1)

typedef void* (*procaddress_t)(const char* name);
typedef void* (*vkprocaddress_t)(void* instance, const char* name);

#define MAX_SIGNAL 64

typedef struct tlsdatasize_s {
    int         tlssize;
    int         n_elfs;
    void*       data;
    void*       ptr;
} tlsdatasize_t;

void free_tlsdatasize(void* p);

typedef struct needed_libs_s {
    int         cap;
    int         size;
    char**      names;
    library_t** libs;
    int         nb_done;
} needed_libs_t;

void free_neededlib(needed_libs_t* needed);
needed_libs_t* new_neededlib(int n);
needed_libs_t* copy_neededlib(needed_libs_t* needed);
void add1_neededlib(needed_libs_t* needed);
void add1lib_neededlib(needed_libs_t* needed, library_t* lib, const char* name);

typedef struct base_segment_s {
    uintptr_t       base;
    uint32_t        limit;
    int             present;
    pthread_key_t   key;
} base_segment_t;

typedef struct box86context_s {
    path_collection_t   box86_path;     // PATH env. variable
    path_collection_t   box86_ld_lib;   // LD_LIBRARY_PATH env. variable

    path_collection_t   box86_emulated_libs;    // Collection of libs that should not be wrapped

    int                 x86trace;
    int                 trace_tid;

    uint32_t            sel_serial;     // will be increment each time selectors changes

    zydis_t             *zydis;         // dlopen the zydis dissasembler
    void*               box86lib;       // dlopen on box86 itself

    int                 argc;
    char**              argv;

    int                 envc;
    char**              envv;

    int                 orig_argc;
    char**              orig_argv;

    char*               fullpath;
    char*               box86path;      // path of current box86 executable
    char*               box64path;      // path of box64 executable (beside box86, if it exists)
    char*               bashpath;       // path of x86 bash (defined with BOX86_BASH or by running bash directly)

    uint32_t            stacksz;
    uint32_t            stackalign;
    void*               stack;          // alocated stack

    elfheader_t         **elfs;         // elf headers and memory
    int                 elfcap;
    int                 elfsize;        // number of elf loaded

    needed_libs_t       *neededlibs;    // needed libs for main elf
    needed_libs_t       *preload;

    uintptr_t           ep;             // entry point

    void*               brk;            // current brk (should be the end of bss segment of main elf)
    int                 brksz;          // current added sz for brk

    lib_t               *maplib;        // lib and symbols handling
    lib_t               *local_maplib;  // libs and symbols openned has local (only collection of libs, no symbols)
    dic_t               *versym;        // dictionnary of versioned symbols
    kh_mapsymbols_t     *globdata;      // GLOBAL_DAT relocation for COPY mapping in main elf

    kh_threadstack_t    *stacksizes;    // stack sizes attributes for thread (temporary)
    bridge_t            *system;        // other bridges
    uintptr_t           exit_bridge;    // exit bridge value
    uintptr_t           vsyscall;       // vsyscall bridge value
    dlprivate_t         *dlprivate;     // dlopen library map
    kh_symbolmap_t      *alwrappers;    // the map of wrapper for alGetProcAddress
    kh_symbolmap_t      *almymap;       // link to the mysymbolmap if libOpenAL
    kh_symbolmap_t      *vkwrappers;    // the map of wrapper for VulkanProcs (TODO: check SDL2)
    kh_symbolmap_t      *vkmymap;       // link to the mysymbolmap of libGL
    vkprocaddress_t     vkprocaddress;

    #ifndef DYNAREC
    pthread_mutex_t     mutex_once;
    pthread_mutex_t     mutex_once2;
    pthread_mutex_t     mutex_trace;
    pthread_mutex_t     mutex_tls;
    pthread_mutex_t     mutex_thread;
    pthread_mutex_t     mutex_bridge;
    pthread_mutex_t     mutex_lock;
    #else
    #ifdef USE_CUSTOM_MUTEX
    uint32_t            mutex_once;
    uint32_t            mutex_once2;
    uint32_t            mutex_trace;
    uint32_t            mutex_tls;
    uint32_t            mutex_thread;
    uint32_t            mutex_bridge;
    uint32_t            mutex_dyndump;
    #else
    pthread_mutex_t     mutex_once;
    pthread_mutex_t     mutex_once2;
    pthread_mutex_t     mutex_trace;
    pthread_mutex_t     mutex_tls;
    pthread_mutex_t     mutex_thread;
    pthread_mutex_t     mutex_bridge;
    pthread_mutex_t     mutex_dyndump;
    #endif
    uintptr_t           max_db_size;    // the biggest (in x86_64 instructions bytes) built dynablock
    int                 trace_dynarec;
    pthread_mutex_t     mutex_lock;     // this is for the Test interpreter
    #endif

    library_t           *libclib;       // shortcut to libc library (if loaded, so probably yes)
    library_t           *sdl1mixerlib;
    library_t           *sdl2lib;       // shortcut to SDL2 library (if loaded)
    library_t           *sdl2mixerlib;

    linkmap_t           *linkmap;

    void*               sdl1allocrw;    // AllocRW/FreeRW functions from SDL1
    void*               sdl1freerw;
    void*               sdl2allocrw;    // AllocRW/FreeRW functions from SDL2
    void*               sdl2freerw;

    int                 deferredInit;
    elfheader_t         **deferredInitList;
    int                 deferredInitSz;
    int                 deferredInitCap;

    pthread_key_t       tlskey;     // then tls key to have actual tlsdata
    void*               tlsdata;    // the initial global tlsdata
    int32_t             tlssize;    // wanted size of tlsdata
    base_segment_t      segtls[3+16];  // only handling 0/1/2 descriptors + some special cases

    uintptr_t           *auxval_start;

    cleanup_t           *cleanups;      // atexit functions
    int                 clean_sz;
    int                 clean_cap;
#ifndef NOALIGN
    kh_fts_t            *ftsmap;
#endif
    zydis_dec_t         *dec;           // trace

    int                 forked;         //  how many forks... cleanup only when < 0

    atfork_fnc_t        *atforks;       // fnc for atfork...
    int                 atfork_sz;
    int                 atfork_cap;

    uint8_t             canary[4];

    uintptr_t           signals[MAX_SIGNAL+1];
    uintptr_t           restorer[MAX_SIGNAL+1];
    int                 onstack[MAX_SIGNAL+1];
    int                 is_sigaction[MAX_SIGNAL+1];
    x86emu_t            *emu_sig;       // the emu with stack used for signal handling (must be separated from main ones)
    int                 no_sigsegv;
    int                 no_sigill;
    void*               stack_clone;
    int                 stack_clone_used;

    // rolling logs
    char*               *log_call;
    char*               *log_ret;
    int                 current_line;
} box86context_t;

#ifndef USE_CUSTOM_MUTEX
#define mutex_lock(A)       pthread_mutex_lock(A)
#define mutex_trylock(A)    pthread_mutex_trylock(A)
#define mutex_unlock(A)     pthread_mutex_unlock(A)
#else
int GetTID();
#define mutex_lock(A)       {int tid = GetTID(); while(arm_lock_storeifnull(A, (void*)tid)) sched_yield();}
#define mutex_trylock(A)    (uintptr_t)arm_lock_storeifnull(A, (void*)GetTID())
#define mutex_unlock(A)     arm_lock_storeifref(A, NULL, (void*)GetTID())
#endif

extern box86context_t *my_context; // global context

box86context_t *NewBox86Context(int argc);
void FreeBox86Context(box86context_t** context);

// Cycle log handling
void freeCycleLog(box86context_t* ctx);
void initCycleLog(box86context_t* context);
void print_cycle_log(int loglevel);

// return the index of the added header
int AddElfHeader(box86context_t* ctx, elfheader_t* head);
// remove an elf from list (but list is never reduced, so there can be holes)
void RemoveElfHeader(box86context_t* ctx, elfheader_t* head);

// return the tlsbase (negative) for the new TLS partition created (no partition index is stored in the context)
int AddTLSPartition(box86context_t* context, int tlssize);

// defined in fact in threads.c
void thread_set_emu(x86emu_t* emu);
x86emu_t* thread_get_emu();

// unlock mutex that are locked by current thread (for signal handling). Return a mask of unlock mutex
int unlockMutex();
// relock the muxtex that were unlocked
void relockMutex(int locks);

#endif //__BOX86CONTEXT_H_
