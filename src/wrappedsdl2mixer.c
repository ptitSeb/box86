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
#include "sdl2rwops.h"
#include "callback.h"

typedef void* (*pFpi_t)(void*, int32_t);
typedef void* (*pFp_t)(void*);
typedef void* (*pFpii_t)(void*, int32_t, int32_t);
typedef void  (*vFpp_t)(void*, void*);

typedef struct sdl2mixer_my_s {
    pFpii_t Mix_LoadMUSType_RW;
    pFp_t Mix_LoadMUS_RW;
    pFpi_t Mix_LoadWAV_RW;
    vFpp_t Mix_SetPostMix;

    x86emu_t* PostCallback;
} sdl2mixer_my_t;

static void* getSDL2MixerMy(library_t* lib)
{
    sdl2mixer_my_t* my = (sdl2mixer_my_t*)calloc(1, sizeof(sdl2mixer_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(Mix_LoadMUSType_RW,pFpii_t)
    GO(Mix_LoadMUS_RW,pFp_t)
    GO(Mix_LoadWAV_RW,pFpi_t)
    GO(Mix_SetPostMix,vFpp_t)
    #undef GO
    return my;
}

void EXPORT *my2_Mix_LoadMUSType_RW(x86emu_t* emu, void* a, int32_t b, int32_t c)
{
    sdl2mixer_my_t *my = (sdl2mixer_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    void* r = my->Mix_LoadMUSType_RW(a, b, c);
    if(c==0)
        RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
void EXPORT *my2_Mix_LoadMUS_RW(x86emu_t* emu, void* a)
{
    sdl2mixer_my_t *my = (sdl2mixer_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    void* r = my->Mix_LoadMUS_RW(a);
    RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);  // this one never free the RWops
    return r;
}
void EXPORT *my2_Mix_LoadWAV_RW(x86emu_t* emu, void* a, int32_t b)
{
    sdl2mixer_my_t *my = (sdl2mixer_my_t *)emu->context->sdl2mixerlib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    void* r = my->Mix_LoadWAV_RW(a, b);
    if(b==0)
        RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}

static void sdl2mixerPostCallback(void *userdata, uint8_t *stream, int32_t len)
{
    x86emu_t *emu = (x86emu_t*) userdata;
    SetCallbackArg(emu, 1, stream);
    SetCallbackArg(emu, 2, (void*)len);
    RunCallback(emu);
}

void EXPORT my2_Mix_SetPostMix(x86emu_t* emu, void* a, void* b)
{
    sdl2mixer_my_t *my = (sdl2mixer_my_t *)emu->context->sdl2mixerlib->priv.w.p2;
    // create a callback
    if(a) {
        x86emu_t *cbemu = AddCallback(emu, (uintptr_t)a, 3, b, NULL, NULL, NULL);
        my->Mix_SetPostMix(sdl2mixerPostCallback, cbemu);
        if(my->PostCallback)
            FreeCallback(my->PostCallback);
        my->PostCallback = cbemu;
    } else {
        my->Mix_SetPostMix(NULL, b);
        FreeCallback(my->PostCallback);
        my->PostCallback = NULL;
    }
}
const char* sdl2mixerName = "libSDL2_mixer-2.0.so.0";
#define LIBNAME sdl2mixer

#define CUSTOM_INIT \
    box86->sdl2mixerlib = lib; \
    lib->priv.w.p2 = getSDL2MixerMy(lib); \
    lib->altmy = strdup("my2_");

#define CUSTOM_FINI \
    free(lib->priv.w.p2); \
    ((box86context_t*)(lib->context))->sdl2mixerlib = NULL;

#include "wrappedlib_init.h"

