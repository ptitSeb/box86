#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x86emu.h"
#include "debug.h"
#include "callback.h"
#include "emu/x86emu_private.h"
#include "box86context.h"

const char* libzName = "libz.so.1";
#define LIBNAME libz

// TODO: put the wrapper type in a dedicate include
typedef void* (*pFpi_t)(void*, int32_t);
typedef void* (*pFp_t)(void*);
typedef void* (*pFpp_t)(void*, void*);
typedef int32_t (*iFp_t)(void*);
typedef int32_t (*iFppi_t)(void*, void*, int32_t);
typedef void* (*pFpippp_t)(void*, int32_t, void*, void*, void*);
typedef void  (*vFp_t)(void*);
typedef void* (*pFpp_t)(void*, void*);
typedef uint32_t (*uFp_t)(void*);
typedef uint64_t (*UFp_t)(void*);
typedef uint32_t (*uFu_t)(uint32_t);
typedef int32_t (*iFpp_t)(void*, void*);
typedef uint32_t (*uFpW_t)(void*, uint16_t);
typedef uint32_t (*uFpu_t)(void*, uint32_t);
typedef uint32_t (*uFpU_t)(void*, uint64_t);
typedef uint32_t (*uFupp_t)(uint32_t, void*, void*);

typedef struct libz_my_s {
    // functions
    iFppi_t      inflateInit_;
    iFp_t        inflateInit;
    iFp_t        inflateEnd;
} libz_my_t;


void* getZMy(library_t* lib)
{
    libz_my_t* my = (libz_my_t*)calloc(1, sizeof(libz_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(inflateInit_, iFppi_t)
    GO(inflateInit, iFp_t)
    GO(inflateEnd, iFp_t)
    #undef GO
    return my;
}

void freeZMy(void* lib)
{
    //libz_my_t *my = (libz_my_t *)lib;
}


typedef struct z_stream_s {
    void *next_in;   
    uint32_t     avail_in;
    uint32_t    total_in;
    void    *next_out;
    uint32_t     avail_out;
    uint32_t    total_out;
    char *msg;
    void *state;
    void* zalloc;
    void*  zfree; 
    void*     opaque;
    int32_t     data_type;
    uint32_t   adler;    
    uint32_t   reserved; 
} z_stream;

static void* my_zlib_alloc(void* opaque, uint32_t items, uint32_t size)
{
    x86emu_t *emu = (x86emu_t*)opaque;
    SetCallbackNArg(emu, 3);
    SetCallbackArg(emu, 1, (void*)items);
    SetCallbackArg(emu, 2, (void*)size);
    SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 4));
    return (void*)RunCallback(emu);
}

static void my_zlib_free(void* opaque, void* address)
{
    x86emu_t *emu = (x86emu_t*)opaque;
    SetCallbackNArg(emu, 2);
    SetCallbackArg(emu, 1, address);
    SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 5));
    RunCallback(emu);
}

static void wrapper_stream_z(x86emu_t* emu, void* str)
{
    z_stream *stream = (z_stream*)str;
    x86emu_t *cb = NULL;
    if(stream->zalloc || stream->zfree) {
        cb = AddCallback(emu, 0, 3, stream->opaque, NULL, NULL, NULL);   // address will be put later
        SetCallbackArg(cb, 4, stream->zalloc);
        SetCallbackArg(cb, 5, stream->zfree);
        stream->opaque = cb;
        if(stream->zalloc) stream->zalloc = my_zlib_alloc;
        if(stream->zfree) stream->zfree = my_zlib_free;
    }
}

EXPORT int32_t my_inflateInit_(x86emu_t* emu, void* str, void* version, int32_t size)
{
    libz_my_t *my = (libz_my_t *)emu->context->zlib->priv.w.p2;
    wrapper_stream_z(emu, str);
    return my->inflateInit_(str, version, size);
}

EXPORT int32_t my_inflateInit(x86emu_t* emu, void* str)
{
    libz_my_t *my = (libz_my_t *)emu->context->zlib->priv.w.p2;
    wrapper_stream_z(emu, str);
    return my->inflateInit(str);
}

EXPORT int32_t my_inflateEnd(x86emu_t* emu, void* str)
{
    libz_my_t *my = (libz_my_t *)emu->context->zlib->priv.w.p2;
    z_stream *stream = (z_stream*)str;
    // check if function need changing...
    x86emu_t *cb = (x86emu_t*)stream->opaque;
    int32_t r = my->inflateEnd(str);
    if(cb) {
        stream->opaque = GetCallbackArg(cb, 0);
        FreeCallback(cb);   // will do nothing if cb is not a callback emu
    }
    return r;
}

#define CUSTOM_INIT \
    box86->zlib = lib; \
    lib->priv.w.p2 = getZMy(lib);

#define CUSTOM_FINI \
    freeZMy(lib->priv.w.p2); \
    free(lib->priv.w.p2); \
    ((box86context_t*)(lib->context))->zlib = NULL;

#include "wrappedlib_init.h"

