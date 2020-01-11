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
#include "callback.h"

typedef void* (*pFpi_t)(void*, int32_t);
typedef void* (*pFp_t)(void*);
typedef void* (*pFpii_t)(void*, int32_t, int32_t);
typedef void  (*vFp_t)(void*);
typedef void  (*vFpp_t)(void*, void*);

typedef struct sdl1mixer_my_s {
    pFpii_t Mix_LoadMUSType_RW;
    pFp_t   Mix_LoadMUS_RW;
    pFpi_t  Mix_LoadWAV_RW;
    vFpp_t  Mix_SetPostMix;
    vFp_t   Mix_ChannelFinished;
    vFpp_t  Mix_HookMusic;
    vFp_t   Mix_HookMusicFinished;

    x86emu_t* PostCallback;
    x86emu_t* hookMusicCB;
} sdl1mixer_my_t;

static x86emu_t* hookMusicFinitCB = NULL;

static void* getSDL1MixerMy(library_t* lib)
{
    sdl1mixer_my_t* my = (sdl1mixer_my_t*)calloc(1, sizeof(sdl1mixer_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(Mix_LoadMUSType_RW,pFpii_t)
    GO(Mix_LoadMUS_RW,pFp_t)
    GO(Mix_LoadWAV_RW,pFpi_t)
    GO(Mix_SetPostMix,vFpp_t)
    GO(Mix_ChannelFinished,vFp_t)
    GO(Mix_HookMusic, vFpp_t)
    GO(Mix_HookMusicFinished, vFp_t)
    #undef GO
    return my;
}

static void freeSDL1MixerMy(library_t* lib)
{
    sdl1mixer_my_t *my = lib->priv.w.p2;
    if(my->PostCallback)
        FreeCallback(my->PostCallback);
    if(my->hookMusicCB)
        FreeCallback(my->hookMusicCB);
}

void EXPORT *my_Mix_LoadMUSType_RW(x86emu_t* emu, void* a, int32_t b, int32_t c)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->Mix_LoadMUSType_RW(rw, b, c);
    if(c==0)
        RWNativeEnd(rw);
    return r;
}
void EXPORT *my_Mix_LoadMUS_RW(x86emu_t* emu, void* a)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->Mix_LoadMUS_RW(rw);
    RWNativeEnd(rw);  // this one never free the RWops
    return r;
}
void EXPORT *my_Mix_LoadWAV_RW(x86emu_t* emu, void* a, int32_t b)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->Mix_LoadWAV_RW(rw, b);
    if(b==0)
        RWNativeEnd(rw);
    return r;
}

static void sdl1mixerPostCallback(void *userdata, uint8_t *stream, int32_t len)
{
    x86emu_t *emu = (x86emu_t*) userdata;
    SetCallbackArg(emu, 1, stream);
    SetCallbackArg(emu, 2, (void*)len);
    RunCallback(emu);
}

void EXPORT my_Mix_SetPostMix(x86emu_t* emu, void* a, void* b)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    // create a callback
    if(a) {
        x86emu_t *cbemu = AddCallback(emu, (uintptr_t)a, 3, b, NULL, NULL, NULL);
        my->Mix_SetPostMix(sdl1mixerPostCallback, cbemu);
        if(my->PostCallback)
            FreeCallback(my->PostCallback);
        my->PostCallback = cbemu;
    } else {
        my->Mix_SetPostMix(NULL, b);
        FreeCallback(my->PostCallback);
        my->PostCallback = NULL;
    }
}

x86emu_t* sdl1channelfinished_emu = NULL;
static void sdl1ChannelFinishedCallback(int channel)
{
    if(!sdl1channelfinished_emu)
        return;
    SetCallbackArg(sdl1channelfinished_emu, 0, (void*)channel);
    RunCallback(sdl1channelfinished_emu);
}
void EXPORT my_Mix_ChannelFinished(x86emu_t* emu, void* cb)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    if(sdl1channelfinished_emu) {
        FreeCallback(sdl1channelfinished_emu);
        sdl1channelfinished_emu = NULL;
    }
    if(cb) {
        sdl1channelfinished_emu = AddCallback(emu, (uintptr_t)cb, 1, NULL, NULL, NULL, NULL);
        my->Mix_ChannelFinished(sdl1ChannelFinishedCallback);
    } else
        my->Mix_ChannelFinished(NULL);
}
static void sdl1mixer_hookMusicCallback(void* udata, uint8_t* stream, int32_t len)
{
    x86emu_t *emu = (x86emu_t*)udata;
    SetCallbackArg(emu, 1, (void*)stream);
    SetCallbackArg(emu, 2, (void*)len);
    RunCallback(emu);
}

EXPORT void my_Mix_HookMusic(x86emu_t* emu, void* f, void* arg)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    if(my->hookMusicCB) {
        my->Mix_HookMusic(NULL, NULL);
        FreeCallback(my->hookMusicCB);
        my->hookMusicCB = NULL;
    }
    if(!f)
        return;
    x86emu_t *cb = NULL;
    cb =  AddCallback(emu, (uintptr_t)f, 3, arg, NULL, NULL, NULL);
    my->hookMusicCB = cb;
    my->Mix_HookMusic(sdl1mixer_hookMusicCallback, cb);
}

static void sdl1mixer_hookMusicFinitCallback()
{
    x86emu_t *emu = hookMusicFinitCB;
    if(emu)
        RunCallback(emu);
}

EXPORT void my_Mix_HookMusicFinished(x86emu_t* emu, void* f)
{
    sdl1mixer_my_t *my = (sdl1mixer_my_t *)emu->context->sdl1mixerlib->priv.w.p2;
    if(hookMusicFinitCB) {
        my->Mix_HookMusicFinished(NULL);
        FreeCallback(hookMusicFinitCB);
        hookMusicFinitCB = NULL;
    }
    if(!f)
        return;
    hookMusicFinitCB =  AddCallback(emu, (uintptr_t)f, 0, NULL, NULL, NULL, NULL);
    my->Mix_HookMusicFinished(sdl1mixer_hookMusicFinitCallback);
}

const char* sdl1mixerName = "libSDL_mixer-1.2.so.0";
#define LIBNAME sdl1mixer

#define CUSTOM_INIT \
    box86->sdl1mixerlib = lib; \
    lib->priv.w.p2 = getSDL1MixerMy(lib);

#define CUSTOM_FINI \
    freeSDL1MixerMy(lib); \
    free(lib->priv.w.p2); \
    ((box86context_t*)(lib->context))->sdl1mixerlib = NULL;

#include "wrappedlib_init.h"

