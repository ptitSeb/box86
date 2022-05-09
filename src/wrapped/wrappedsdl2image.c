#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x86emu.h"
#include "emu/x86emu_private.h"
#include "box86context.h"
#include "sdl2rwops.h"

const char* sdl2imageName = "libSDL2_image-2.0.so.0";
#define LIBNAME sdl2image

#define ADDED_FUNCTIONS() \

#include "generated/wrappedsdl2imagetypes.h"

#include "wrappercallback.h"

#define GO(A) \
EXPORT void *my2_##A(x86emu_t* emu, void* a) \
{ \
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a); \
    void* r = my->A(rw); \
    RWNativeEnd2(rw); \
    return r; \
}
GO(IMG_LoadBMP_RW)
GO(IMG_LoadCUR_RW)
GO(IMG_LoadGIF_RW)
GO(IMG_LoadICO_RW)
GO(IMG_LoadJPG_RW)
GO(IMG_LoadLBM_RW)
GO(IMG_LoadPCX_RW)
GO(IMG_LoadPNG_RW)
GO(IMG_LoadPNM_RW)
GO(IMG_LoadTGA_RW)
GO(IMG_LoadTIF_RW)
GO(IMG_LoadWEBP_RW)
GO(IMG_LoadXCF_RW)
GO(IMG_LoadXPM_RW)
GO(IMG_LoadXV_RW)
#undef GO

 EXPORT void *my2_IMG_LoadTyped_RW(x86emu_t* emu, void* a, int32_t b, void* c)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->IMG_LoadTyped_RW(rw, b, c);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT void *my2_IMG_Load_RW(x86emu_t* emu, void* a, int32_t b)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->IMG_Load_RW(rw, b);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT int32_t my2_IMG_SavePNG_RW(x86emu_t* emu, void* a, void* surf, int32_t compression)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int32_t r = my->IMG_SavePNG_RW(rw, surf, compression);
    RWNativeEnd2(rw);
    return r;
}

EXPORT void* my2_IMG_LoadTexture_RW(x86emu_t* emu, void* rend, void* a, int32_t b)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->IMG_LoadTexture_RW(rend, rw, b);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}

EXPORT void* my2_IMG_LoadTextureTyped_RW(x86emu_t* emu, void* rend, void* a, int32_t b, void* type)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->IMG_LoadTextureTyped_RW(rend, rw, b, type);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}

#define CUSTOM_INIT \
    getMy(lib);             \
    SETALT(my2_);           \
    setNeededLibs(lib, 1, "libSDL2-2.0.so.0");

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"

