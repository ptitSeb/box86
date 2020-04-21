#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x86emu.h"
#include "emu/x86emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box86context.h"
#include "emu/x86emu_private.h"
#include "myalign.h"

const char* bz2Name = "libbz2.so.1";
#define LIBNAME bz2

typedef int  (*iFp_t)(void*);
typedef int  (*iFpi_t)(void*, int);
typedef int  (*iFpii_t)(void*, int, int);
typedef int  (*iFpiii_t)(void*, int, int, int);

#define SUPER() \
    GO(BZ2_bzCompressInit, iFpiii_t)    \
    GO(BZ2_bzCompress, iFpi_t)          \
    GO(BZ2_bzCompressEnd, iFp_t)        \
    GO(BZ2_bzDecompressInit, iFpii_t)   \
    GO(BZ2_bzDecompress, iFp_t)         \
    GO(BZ2_bzDecompressEnd, iFp_t)

typedef struct bz2_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} bz2_my_t;

void* getBz2My(library_t* lib)
{
    bz2_my_t* my = (bz2_my_t*)calloc(1, sizeof(bz2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeBz2My(void* lib)
{
    //bz2_my_t *my = (bz2_my_t *)lib;
}

typedef struct {
    char *next_in;
    unsigned int avail_in;
    unsigned int total_in_lo32;
    unsigned int total_in_hi32;

    char *next_out;
    unsigned int avail_out;
    unsigned int total_out_lo32;
    unsigned int total_out_hi32;

    void *state;

    void *(*bzalloc)(void *,int,int);
    void (*bzfree)(void *,void *);
    void *opaque;
} my_bz_stream_t;

static void* my_bz_malloc(void* opaque, int m, int n)
{
    x86emu_t* emu = (x86emu_t*)opaque;
    SetCallbackArg(emu, 0, GetCallbackArg(emu, 3));
    SetCallbackArg(emu, 1, (void*)m);
    SetCallbackArg(emu, 2, (void*)n);
    SetCallbackNArg(emu, 3);
    SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 4));
    return (void*)RunCallback(emu);
}
static void my_bz_free(void* opaque, void* p)
{
    x86emu_t* emu = (x86emu_t*)opaque;
    SetCallbackArg(emu, 0, GetCallbackArg(emu, 3));
    SetCallbackArg(emu, 1, p);
    SetCallbackNArg(emu, 2);
    SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 5));
    RunCallback(emu);
}

#define WRAP_BZ(A) if(A->bzalloc || A->bzfree) { \
    x86emu_t* cb = AddSharedCallback(emu, 0, 0, NULL, NULL, NULL, NULL); \
    SetCallbackArg(cb, 3, A->opaque);   \
    SetCallbackArg(cb, 4, A->bzalloc);  \
    SetCallbackArg(cb, 5, A->bzfree);   \
    if(A->bzalloc)  A->bzalloc = my_bz_malloc;  \
    if(A->bzfree)  A->bzfree = my_bz_free;      \
}
#define UNWRAP_BZ(A) if(A->bzalloc || A->bzfree) { \
    x86emu_t* cb = (x86emu_t*)A->opaque;\
    A->opaque = GetCallbackArg(cb, 3);  \
    A->bzalloc = GetCallbackArg(cb, 4); \
    A->bzfree = GetCallbackArg(cb, 5);  \
    FreeCallback(cb);                   \
}

EXPORT int my_BZ2_bzCompressInit(x86emu_t* emu, my_bz_stream_t* strm, int blocksize, int verbosity, int work)
{
    library_t * lib = GetLib(emu->context->maplib, bz2Name);
    bz2_my_t *my = (bz2_my_t*)lib->priv.w.p2;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzCompressInit(strm, blocksize, verbosity, work);
    UNWRAP_BZ(strm);
    return ret;
}

EXPORT int my_BZ2_bzCompress(x86emu_t* emu, my_bz_stream_t* strm, int action)
{
    library_t * lib = GetLib(emu->context->maplib, bz2Name);
    bz2_my_t *my = (bz2_my_t*)lib->priv.w.p2;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzCompress(strm, action);
    UNWRAP_BZ(strm);
    return ret;
}

EXPORT int my_BZ2_bzCompressEnd(x86emu_t* emu, my_bz_stream_t* strm)
{
    library_t * lib = GetLib(emu->context->maplib, bz2Name);
    bz2_my_t *my = (bz2_my_t*)lib->priv.w.p2;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzCompressEnd(strm);
    UNWRAP_BZ(strm);
    return ret;
}

EXPORT int my_BZ2_bzDecompressInit(x86emu_t* emu, my_bz_stream_t* strm, int verbosity, int small)
{
    library_t * lib = GetLib(emu->context->maplib, bz2Name);
    bz2_my_t *my = (bz2_my_t*)lib->priv.w.p2;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzDecompressInit(strm, verbosity, small);
    UNWRAP_BZ(strm);
    return ret;
}

EXPORT int my_BZ2_bzDecompress(x86emu_t* emu, my_bz_stream_t* strm)
{
    library_t * lib = GetLib(emu->context->maplib, bz2Name);
    bz2_my_t *my = (bz2_my_t*)lib->priv.w.p2;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzDecompress(strm);
    UNWRAP_BZ(strm);
    return ret;
}

EXPORT int my_BZ2_bzDecompressEnd(x86emu_t* emu, my_bz_stream_t* strm)
{
    library_t * lib = GetLib(emu->context->maplib, bz2Name);
    bz2_my_t *my = (bz2_my_t*)lib->priv.w.p2;
    WRAP_BZ(strm);
    int ret = my->BZ2_bzDecompressEnd(strm);
    UNWRAP_BZ(strm);
    return ret;
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getBz2My(lib);

#define CUSTOM_FINI \
    freeBz2My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

