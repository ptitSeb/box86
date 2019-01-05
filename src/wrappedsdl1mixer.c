#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "library_private.h"
#include "x86emu.h"
#include "x86emu_private.h"
#include "box86context.h"
#include "sdl1rwops.h"

typedef void* (*pFpi_t)(void*, int32_t);
typedef void* (*pFp_t)(void*);
typedef void* (*pFpii_t)(void*, int32_t, int32_t);

typedef struct sdl1mixer_my_s {
    pFpii_t Mix_LoadMUSType_RW;
    pFp_t Mix_LoadMUS_RW;
    pFpi_t Mix_LoadWAV_RW;
} sdl1mixer_my_t;

void* getSDL1MixerMy(library_t* lib)
{
    sdl1mixer_my_t* my = (sdl1mixer_my_t*)calloc(1, sizeof(sdl1mixer_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(Mix_LoadMUSType_RW,pFpii_t)
    GO(Mix_LoadMUS_RW,pFp_t)
    GO(Mix_LoadWAV_RW,pFpi_t)
    #undef GO
    return my;
}

void EXPORT *my_Mix_LoadMUSType_RW(x86emu_t* emu, void* a, int32_t b, int32_t c)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDLRWSave_t save;
    RWNativeStart(emu, (SDL1_RWops_t*)a, &save);
    void* r = my->Mix_LoadMUSType_RW(a, b, c);
    if(c==0)
        RWNativeEnd(emu, (SDL1_RWops_t*)a, &save);
    return r;
}
void EXPORT *my_Mix_LoadMUS_RW(x86emu_t* emu, void* a)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDLRWSave_t save;
    RWNativeStart(emu, (SDL1_RWops_t*)a, &save);
    void* r = my->Mix_LoadMUS_RW(a);
    RWNativeEnd(emu, (SDL1_RWops_t*)a, &save);  // this one never free the RWops
    return r;
}
void EXPORT *my_Mix_LoadWAV_RW(x86emu_t* emu, void* a, int32_t b)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    SDLRWSave_t save;
    RWNativeStart(emu, (SDL1_RWops_t*)a, &save);
    void* r = my->Mix_LoadWAV_RW(a, b);
    if(b==0)
        RWNativeEnd(emu, (SDL1_RWops_t*)a, &save);
    return r;
}



const char* sdl1mixerName = "libSDL_mixer-1.2.so.0";
#define LIBNAME sdl1mixer

#define CUSTOM_INIT \
    box86->sdl1mixerlib = lib; \
    lib->priv.w.p2 = getSDL1MixerMy(lib);

#define CUSTOM_FINI \
    free(lib->priv.w.p2); \
    ((box86context_t*)(lib->context))->sdl1mixerlib = NULL;

#include "wrappedlib_init.h"

