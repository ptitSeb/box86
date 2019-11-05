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
#include "box86context.h"
#include "sdl1rwops.h"

typedef void* (*pFpii_t)(void*, int32_t, int32_t);
typedef void* (*pFpiii_t)(void*, int32_t, int32_t, int32_t);

typedef struct sdl1ttf_my_s {
    pFpii_t     TTF_OpenFontRW;
    pFpiii_t    TTF_OpenFontIndexRW;
} sdl1ttf_my_t;

static void* getSDL1TTFMy(library_t* lib)
{
    sdl1ttf_my_t* my = (sdl1ttf_my_t*)calloc(1, sizeof(sdl1ttf_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(TTF_OpenFontIndexRW,pFpiii_t)
    GO(TTF_OpenFontRW, pFpii_t)
    #undef GO
    return my;
}

void EXPORT *my_TTF_OpenFontIndexRW(x86emu_t* emu, void* a, int32_t b, int32_t c, int32_t d)
{
    sdl1ttf_my_t *my = (sdl1ttf_my_t *)emu->context->sdl1ttflib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->TTF_OpenFontIndexRW(rw, b, c, d);
    if(b==0)
        RWNativeEnd(rw);
    return r;
}

void EXPORT *my_TTF_OpenFontRW(x86emu_t* emu, void* a, int32_t b, int32_t c)
{
    sdl1ttf_my_t *my = (sdl1ttf_my_t *)emu->context->sdl1ttflib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->TTF_OpenFontRW(rw, b, c);
    if(b==0)
        RWNativeEnd(rw);
    return r;
}

const char* sdl1ttfName = "libSDL_ttf-2.0.so.0";
#define LIBNAME sdl1ttf

#define CUSTOM_INIT \
    box86->sdl1ttflib = lib; \
    lib->priv.w.p2 = getSDL1TTFMy(lib);

#define CUSTOM_FINI \
    free(lib->priv.w.p2); \
    ((box86context_t*)(lib->context))->sdl1ttflib = NULL;

#include "wrappedlib_init.h"

