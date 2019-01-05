#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "bridge.h"
#include "library_private.h"
#include "x86emu.h"
#include "callback.h"
#include "librarian.h"
#include "x86emu_private.h"
#include "box86context.h"
#include "sdl1rwops.h"

#include "x86trace.h"

const char* sdl1Name = "libSDL-1.2.so.0";
#define LIBNAME sdl1

int sdl_Yes() { return 1; }
int sdl_No() { return 0; }
int EXPORT my_SDL_Has3DNow() __attribute__((alias("sdl_No")));
int EXPORT my_SDL_Has3DNowExt() __attribute__((alias("sdl_No")));
int EXPORT my_SDL_HasAltiVec() __attribute__((alias("sdl_No")));
int EXPORT my_SDL_HasMMX() __attribute__((alias("sdl_Yes")));
int EXPORT my_SDL_HasMMXExt() __attribute__((alias("sdl_Yes")));
int EXPORT my_SDL_HasRDTSC() __attribute__((alias("sdl_Yes")));
int EXPORT my_SDL_HasSSE() __attribute__((alias("sdl_Yes")));
int EXPORT my_SDL_HasSSE2() __attribute__((alias("sdl_Yes")));

typedef struct {
  int32_t freq;
  uint16_t format;
  uint8_t channels;
  uint8_t silence;
  uint16_t samples;
  uint32_t size;
  void (*callback)(void *userdata, uint8_t *stream, int32_t len);
  void *userdata;
} SDL_AudioSpec;

typedef int (*PFNOPENAUDIO)(SDL_AudioSpec *desired, void *obtained);

// TODO: put the wrapper type in a dedicate include
typedef void* (*pFpi_t)(void*, int32_t);
typedef void* (*pFpp_t)(void*, void*);
typedef int32_t (*iFppi_t)(void*, void*, int32_t);
typedef void* (*pFpippp_t)(void*, int32_t, void*, void*, void*);
typedef void  (*vFp_t)(void*);
typedef uint32_t (*uFp_t)(void*);
typedef uint64_t (*UFp_t)(void*);

typedef struct sdl1_my_s {
    pFpi_t     SDL_LoadBMP_RW;
    pFpi_t     SDL_RWFromConstMem;
    pFpi_t     SDL_RWFromFP;
    pFpp_t     SDL_RWFromFile;
    pFpi_t     SDL_RWFromMem;
    iFppi_t    SDL_SaveBMP_RW;
    pFpippp_t  SDL_LoadWAV_RW;
    vFp_t      SDL_FreeRW;
    uFp_t      SDL_ReadBE16;
    uFp_t      SDL_ReadBE32;
    UFp_t      SDL_ReadBE64;
    uFp_t      SDL_ReadLE16;
    uFp_t      SDL_ReadLE32;
    UFp_t      SDL_ReadLE64;
} sdl1_my_t;

void* getSDL1My(library_t* lib)
{
    sdl1_my_t* my = (sdl1_my_t*)calloc(1, sizeof(sdl1_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(SDL_LoadBMP_RW, pFpi_t)
    GO(SDL_RWFromConstMem, pFpi_t)
    GO(SDL_RWFromFP, pFpi_t)
    GO(SDL_RWFromFile, pFpp_t)
    GO(SDL_RWFromMem, pFpi_t)
    GO(SDL_SaveBMP_RW, iFppi_t)
    GO(SDL_LoadWAV_RW, pFpippp_t)
    GO(SDL_FreeRW, vFp_t)
    GO(SDL_ReadBE16, uFp_t)
    GO(SDL_ReadBE32, uFp_t)
    GO(SDL_ReadBE64, UFp_t)
    GO(SDL_ReadLE16, uFp_t)
    GO(SDL_ReadLE32, uFp_t)
    GO(SDL_ReadLE64, UFp_t)
    #undef GO
    return my;
}

void sdl1Callback(void *userdata, uint8_t *stream, int32_t len)
{
    x86emu_t *emu = (x86emu_t*) userdata;
    SetCallbackArg(emu, 1, stream);
    SetCallbackArg(emu, 2, (void*)len);
    RunCallback(emu);
}

// TODO: track the memory for those callback
int EXPORT my_SDL_OpenAudio(x86emu_t* emu, void* d, void* o)
{
    SDL_AudioSpec *desired = (SDL_AudioSpec*)d;
    library_t *sdllib = GetLib(emu->context->maplib, sdl1Name);
    PFNOPENAUDIO openaudio = (PFNOPENAUDIO)dlsym(sdllib->priv.w.lib, "SDL_OpenAudio");
    // create a callback
    void *fnc = (void*)desired->callback;
    void *olduser = desired->userdata;
    x86emu_t *cbemu = (desired->callback)?AddCallback(emu, (uintptr_t)fnc, 3, olduser, NULL, NULL, NULL):NULL;
    desired->callback = sdl1Callback;
    desired->userdata = cbemu;
    int ret = openaudio(desired, (SDL_AudioSpec*)o);
    if (ret!=0) {
        // error, clean the callback...
        desired->callback = fnc;
        desired->userdata = olduser;
        FreeCallback(cbemu);
        return ret;
    }
    // put back stuff in place?
    desired->callback = fnc;
    desired->userdata = olduser;

    return ret;
}

void EXPORT *my_SDL_LoadBMP_RW(x86emu_t* emu, void* a, int b)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDLRWSave_t save;
    RWNativeStart(emu, (SDL1_RWops_t*)a, &save);
    void* r = my->SDL_LoadBMP_RW(a, b);
    if(b==0)
        RWNativeEnd(emu, (SDL1_RWops_t*)a, &save);
    return r;
}
int32_t EXPORT my_SDL_SaveBMP_RW(x86emu_t* emu, void* a, void* b, int c)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDLRWSave_t save;
    RWNativeStart(emu, (SDL1_RWops_t*)a, &save);
    int32_t r = my->SDL_SaveBMP_RW(a, b, c);
    if(c==0)
        RWNativeEnd(emu, (SDL1_RWops_t*)a, &save);
    return r;
}
void EXPORT *my_SDL_LoadWAV_RW(x86emu_t* emu, void* a, int b, void* c, void* d, void* e)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDLRWSave_t save;
    RWNativeStart(emu, (SDL1_RWops_t*)a, &save);
    void* r = my->SDL_LoadWAV_RW(a, b, c, d, e);
    if(b==0)
        RWNativeEnd(emu, (SDL1_RWops_t*)a, &save);
    return r;
}
uint32_t EXPORT my_SDL_ReadBE16(x86emu_t* emu, void* a)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDLRWSave_t save;
    RWNativeStart(emu, (SDL1_RWops_t*)a, &save);
    uint32_t r = my->SDL_ReadBE16(a);
    RWNativeEnd(emu, (SDL1_RWops_t*)a, &save);
    return r;
}
uint32_t EXPORT my_SDL_ReadBE32(x86emu_t* emu, void* a)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDLRWSave_t save;
    RWNativeStart(emu, (SDL1_RWops_t*)a, &save);
    uint32_t r = my->SDL_ReadBE32(a);
    RWNativeEnd(emu, (SDL1_RWops_t*)a, &save);
    return r;
}
uint64_t EXPORT my_SDL_ReadBE64(x86emu_t* emu, void* a)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDLRWSave_t save;
    RWNativeStart(emu, (SDL1_RWops_t*)a, &save);
    uint64_t r = my->SDL_ReadBE64(a);
    RWNativeEnd(emu, (SDL1_RWops_t*)a, &save);
    return r;
}
uint32_t EXPORT my_SDL_ReadLE16(x86emu_t* emu, void* a)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDLRWSave_t save;
    RWNativeStart(emu, (SDL1_RWops_t*)a, &save);
    uint32_t r = my->SDL_ReadLE16(a);
    RWNativeEnd(emu, (SDL1_RWops_t*)a, &save);
    return r;
}
uint32_t EXPORT my_SDL_ReadLE32(x86emu_t* emu, void* a)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDLRWSave_t save;
    RWNativeStart(emu, (SDL1_RWops_t*)a, &save);
    uint32_t r = my->SDL_ReadLE32(a);
    RWNativeEnd(emu, (SDL1_RWops_t*)a, &save);
    return r;
}
uint64_t EXPORT my_SDL_ReadLE64(x86emu_t* emu, void* a)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDLRWSave_t save;
    RWNativeStart(emu, (SDL1_RWops_t*)a, &save);
    uint64_t r = my->SDL_ReadLE64(a);
    RWNativeEnd(emu, (SDL1_RWops_t*)a, &save);
    return r;
}

void EXPORT *my_SDL_RWFromConstMem(x86emu_t* emu, void* a, int b)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    void* r = my->SDL_RWFromConstMem(a, b);
    AddNativeRW(emu, (SDL1_RWops_t*)r);
    return r;
}
void EXPORT *my_SDL_RWFromFP(x86emu_t* emu, void* a, int b)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    void* r = my->SDL_RWFromFP(a, b);
    AddNativeRW(emu, (SDL1_RWops_t*)r);
    return r;
}
void EXPORT *my_SDL_RWFromFile(x86emu_t* emu, void* a, void* b)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    void* r = my->SDL_RWFromFile(a, b);
    AddNativeRW(emu, (SDL1_RWops_t*)r);
    return r;
}
void EXPORT *my_SDL_RWFromMem(x86emu_t* emu, void* a, int b)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    void* r = my->SDL_RWFromMem(a, b);
    AddNativeRW(emu, (SDL1_RWops_t*)r);
    return r;
}

void EXPORT my_SDL_FreeRW(x86emu_t* emu, void* a)
{
    sdl1_my_t *my = (sdl1_my_t *)emu->context->sdl1lib->priv.w.p2;
    SDLRWSave_t save;
    RWNativeStart(emu, (SDL1_RWops_t*)a, &save);
    my->SDL_FreeRW(a);
}


#define CUSTOM_INIT \
    box86->sdl1lib = lib; \
    lib->priv.w.priv = NewSDL1RWops(); \
    lib->priv.w.p2 = getSDL1My(lib);

#define CUSTOM_FINI \
    FreeSDL1RWops((sdl1rwops_t**)&lib->priv.w.priv); \
    free(lib->priv.w.p2); \
    ((box86context_t*)(lib->context))->sdl1lib = NULL;

#include "wrappedlib_init.h"

