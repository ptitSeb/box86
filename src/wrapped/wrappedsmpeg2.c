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
#include "librarian.h"
#include "x86emu.h"
#include "callback.h"
#include "box86context.h"
#include "sdl2rwops.h"
#include "myalign.h"

const char* smpeg2Name = "libsmpeg2-2.0.so.0";
#define LIBNAME smpeg2

typedef void (*vFpp_t)(void*, void*);
typedef void (*vFpppp_t)(void*, void*, void*, void*);
typedef void* (*pFppi_t)(void*, void*, int32_t);
typedef void* (*pFipi_t)(int32_t, void*, int32_t);
typedef void* (*pFpipi_t)(void*, int32_t, void*, int32_t);
typedef void* (*pFppii_t)(void*, void*, int32_t, int32_t);

typedef struct smpeg2_my_s {
    // functions
    vFpppp_t    SMPEG_setdisplay;
    pFppi_t     SMPEG_new;
    pFipi_t     SMPEG_new_descr;
    pFpipi_t    SMPEG_new_data;
    pFppii_t    SMPEG_new_rwops;
    vFpp_t      SMPEG_getinfo;
} smpeg2_my_t;

static void* getSMPEG2My(library_t* lib)
{
    smpeg2_my_t* my = (smpeg2_my_t*)calloc(1, sizeof(smpeg2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(SMPEG_setdisplay, vFpppp_t)
    GO(SMPEG_new, pFppi_t)
    GO(SMPEG_new_descr, pFipi_t)
    GO(SMPEG_new_data, pFpipi_t)
    GO(SMPEG_new_rwops, pFppii_t)
    GO(SMPEG_getinfo, vFpp_t)
    #undef GO
    return my;
}

static void freeSMPEG2My(void* lib)
{
    //smpeg2_my_t *my = (smpeg2_my_t *)lib;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// dispcallback ...
#define GO(A)   \
static uintptr_t my_dispcallback_fct_##A = 0;                           \
static void my_dispcallback_##A(void* data, void* frame)                \
{                                                                       \
    RunFunction(my_context, my_dispcallback_fct_##A, 2, data, frame);   \
}
SUPER()
#undef GO
static void* find_dispcallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_dispcallback_fct_##A == (uintptr_t)fct) return my_dispcallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_dispcallback_fct_##A == 0) {my_dispcallback_fct_##A = (uintptr_t)fct; return my_dispcallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libsmpeg2 dispcallback callback\n");
    return NULL;
}
#undef SUPER


EXPORT void my2_SMPEG_setdisplay(x86emu_t* emu, void* mpeg, void* cb, void* data, void* lock)
{
    library_t* lib = GetLibInternal(smpeg2Name);
    smpeg2_my_t* my = (smpeg2_my_t*)lib->priv.w.p2;
    my->SMPEG_setdisplay(mpeg, find_dispcallback_Fct(cb), data, lock);
}

EXPORT void my2_SMPEG_getinfo(x86emu_t* emu, void* mpeg, void* info)
{
    library_t* lib = GetLibInternal(smpeg2Name);
    smpeg2_my_t* my = (smpeg2_my_t*)lib->priv.w.p2;
    my_SMPEG_Info_t inf = {0};
    my->SMPEG_getinfo(mpeg, &inf);
    UnalignSmpegInfo(info, &inf);
}

EXPORT void* my2_SMPEG_new(x86emu_t* emu, void* file, void* info, int sdl_audio)
{
    library_t* lib = GetLibInternal(smpeg2Name);
    smpeg2_my_t* my = (smpeg2_my_t*)lib->priv.w.p2;
    my_SMPEG_Info_t inf;
    AlignSmpegInfo(&inf, info);
    void* ret = my->SMPEG_new(file, &inf, sdl_audio);
    UnalignSmpegInfo(info, &inf);
    return ret;
}

EXPORT void* my2_SMPEG_new_descr(x86emu_t* emu, int file, void* info, int sdl_audio)
{
    library_t* lib = GetLibInternal(smpeg2Name);
    smpeg2_my_t* my = (smpeg2_my_t*)lib->priv.w.p2;
    my_SMPEG_Info_t inf;
    AlignSmpegInfo(&inf, info);
    void* ret = my->SMPEG_new_descr(file, &inf, sdl_audio);
    UnalignSmpegInfo(info, &inf);
    return ret;
}

EXPORT void* my2_SMPEG_new_data(x86emu_t* emu, void* data, int size, void* info, int sdl_audio)
{
    library_t* lib = GetLibInternal(smpeg2Name);
    smpeg2_my_t* my = (smpeg2_my_t*)lib->priv.w.p2;
    my_SMPEG_Info_t inf;
    AlignSmpegInfo(&inf, info);
    void* ret = my->SMPEG_new_data(data, size, &inf, sdl_audio);
    UnalignSmpegInfo(info, &inf);
    return ret;
}
EXPORT void* my2_SMPEG_new_rwops(x86emu_t* emu, void* src, void* info, int32_t f, int32_t audio)
{
    library_t* lib = GetLibInternal(smpeg2Name);
    smpeg2_my_t* my = (smpeg2_my_t*)lib->priv.w.p2;
    my_SMPEG_Info_t inf;
    AlignSmpegInfo(&inf, info);
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)src);
    void* ret = my->SMPEG_new_rwops(rw, &inf, f, audio);
    UnalignSmpegInfo(info, &inf);
    if(!f) {
        RWNativeEnd2(rw);
    }
    return ret;
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getSMPEG2My(lib); \
    lib->altmy = strdup("my2_");

#define CUSTOM_FINI \
    freeSMPEG2My(lib->priv.w.p2); \
    free(lib->priv.w.p2); \

#include "wrappedlib_init.h"

