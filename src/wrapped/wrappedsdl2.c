#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdarg.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "callback.h"
#include "librarian.h"
#include "librarian/library_private.h"
#include "emu/x86emu_private.h"
#include "box86context.h"
#include "sdl2rwops.h"
#include "myalign.h"

static int sdl_Yes() { return 1;}
static int sdl_No() { return 0;}
int EXPORT my2_SDL_Has3DNow() __attribute__((alias("sdl_No")));
int EXPORT my2_SDL_Has3DNowExt() __attribute__((alias("sdl_No")));
int EXPORT my2_SDL_HasAltiVec() __attribute__((alias("sdl_No")));
int EXPORT my2_SDL_HasMMX() __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasMMXExt() __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasNEON() __attribute__((alias("sdl_No")));   // No neon in x86 ;)
int EXPORT my2_SDL_HasRDTSC() __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasSSE() __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasSSE2() __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasSSE3() __attribute__((alias("sdl_Yes")));
int EXPORT my2_SDL_HasSSE41() __attribute__((alias("sdl_No")));
int EXPORT my2_SDL_HasSSE42() __attribute__((alias("sdl_No")));

typedef struct {
  int32_t freq;
  uint16_t format;
  uint8_t channels;
  uint8_t silence;
  uint16_t samples;
  uint32_t size;
  void (*callback)(void *userdata, uint8_t *stream, int32_t len);
  void *userdata;
} SDL2_AudioSpec;

KHASH_MAP_INIT_INT(timercb, x86emu_t*)

typedef struct {
    uint8_t data[16];
} SDL_JoystickGUID;

typedef struct
{
    int32_t bindType;   // enum
    union
    {
        int button;
        int axis;
        struct {
            int hat;
            int hat_mask;
        } hat;
    } value;
} SDL_GameControllerButtonBind;


// TODO: put the wrapper type in a dedicate include
typedef void* (*pFv_t)();
typedef int32_t (*iFp_t)(void*);
typedef int32_t (*iFip_t)(int32_t, void*);
typedef void* (*pFpi_t)(void*, int32_t);
typedef void* (*pFp_t)(void*);
typedef void* (*pFS_t)(SDL_JoystickGUID);
typedef void* (*pFpp_t)(void*, void*);
typedef int32_t (*iFppi_t)(void*, void*, int32_t);
typedef int32_t (*iFpippi_t)(void*, int32_t, void*, void*, int32_t);
typedef int32_t (*iFppp_t)(void*, void*, void*);
typedef void* (*pFpippp_t)(void*, int32_t, void*, void*, void*);
typedef void*  (*pFpp_t)(void*, void*);
typedef void*  (*pFppp_t)(void*, void*, void*);
typedef void  (*vFp_t)(void*);
typedef void  (*vFpp_t)(void*, void*);
typedef void  (*vFiupp_t)(int32_t, uint32_t, void*, void*);
typedef int32_t (*iFpupp_t)(void*, uint32_t, void*, void*);
typedef uint32_t (*uFu_t)(uint32_t);
typedef uint32_t (*uFp_t)(void*);
typedef uint32_t (*uFupp_t)(uint32_t, void*, void*);
typedef int64_t (*IFp_t)(void*);
typedef uint64_t (*UFp_t)(void*);
typedef int32_t (*iFpi_t)(void*, int32_t);
typedef int32_t (*iFpp_t)(void*, void*);
typedef int32_t (*iFupp_t)(uint32_t, void*, void*);
typedef uint32_t (*uFpC_t)(void*, uint8_t);
typedef uint32_t (*uFpW_t)(void*, uint16_t);
typedef uint32_t (*uFpu_t)(void*, uint32_t);
typedef uint32_t (*uFpU_t)(void*, uint64_t);
typedef SDL_JoystickGUID (*SFi_t)(int32_t);
typedef SDL_JoystickGUID (*SFp_t)(void*);
typedef SDL_GameControllerButtonBind (*SFpi_t)(void*, int32_t);

typedef struct sdl2_my_s {
    iFpp_t     SDL_OpenAudio;
    iFpippi_t  SDL_OpenAudioDevice;
    pFpi_t     SDL_LoadFile_RW;
    pFpi_t     SDL_LoadBMP_RW;
    pFpi_t     SDL_RWFromConstMem;
    pFpi_t     SDL_RWFromFP;
    pFpp_t     SDL_RWFromFile;
    pFpi_t     SDL_RWFromMem;
    iFppi_t    SDL_SaveBMP_RW;
    pFpippp_t  SDL_LoadWAV_RW;
    iFpi_t     SDL_GameControllerAddMappingsFromRW;
    sdl2_allocrw  SDL_AllocRW;
    sdl2_freerw   SDL_FreeRW;
    uFp_t      SDL_ReadU8;
    uFp_t      SDL_ReadBE16;
    uFp_t      SDL_ReadBE32;
    UFp_t      SDL_ReadBE64;
    uFp_t      SDL_ReadLE16;
    uFp_t      SDL_ReadLE32;
    UFp_t      SDL_ReadLE64;
    uFpC_t     SDL_WriteU8;
    uFpW_t     SDL_WriteBE16;
    uFpu_t     SDL_WriteBE32;
    uFpU_t     SDL_WriteBE64;
    uFpW_t     SDL_WriteLE16;
    uFpu_t     SDL_WriteLE32;
    uFpU_t     SDL_WriteLE64;
    uFupp_t    SDL_AddTimer;
    uFu_t      SDL_RemoveTimer;
    pFppp_t    SDL_CreateThread;
    vFp_t      SDL_KillThread;
    vFpp_t     SDL_SetEventFilter;
    vFpp_t     SDL_LogSetOutputFunction;
    vFiupp_t   SDL_LogMessageV;
    pFp_t      SDL_GL_GetProcAddress;
    iFupp_t    SDL_TLSSet;
    SFi_t      SDL_JoystickGetDeviceGUID;
    SFp_t      SDL_JoystickGetGUID;
    SFp_t      SDL_JoystickGetGUIDFromString;
    SFpi_t     SDL_GameControllerGetBindForAxis;
    SFpi_t     SDL_GameControllerGetBindForButton;
    vFpp_t     SDL_AddEventWatch;
    vFpp_t     SDL_DelEventWatch;
    pFS_t      SDL_GameControllerMappingForGUID;
    iFp_t      SDL_SaveAllDollarTemplates;
    iFip_t     SDL_SaveDollarTemplate;
    // timer map
    kh_timercb_t    *timercb;
    uint32_t        settimer;
    // threads
    kh_timercb_t    *threads;
    // evt filter
    x86emu_t        *sdl2_evtfilter;
    void*           sdl2_evtfnc;
    // log output
    x86emu_t        *sdl2_logouput;
} sdl2_my_t;

void* getSDL2My(library_t* lib)
{
    sdl2_my_t* my = (sdl2_my_t*)calloc(1, sizeof(sdl2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(SDL_OpenAudio, iFpp_t)
    GO(SDL_OpenAudioDevice, iFpippi_t)
    GO(SDL_LoadBMP_RW, pFpi_t)
    GO(SDL_RWFromConstMem, pFpi_t)
    GO(SDL_RWFromFP, pFpi_t)
    GO(SDL_RWFromFile, pFpp_t)
    GO(SDL_RWFromMem, pFpi_t)
    GO(SDL_SaveBMP_RW, iFppi_t)
    GO(SDL_LoadWAV_RW, pFpippp_t)
    GO(SDL_GameControllerAddMappingsFromRW, iFpi_t)
    GO(SDL_AllocRW, sdl2_allocrw)
    GO(SDL_FreeRW, sdl2_freerw)
    GO(SDL_ReadU8, uFp_t)
    GO(SDL_ReadBE16, uFp_t)
    GO(SDL_ReadBE32, uFp_t)
    GO(SDL_ReadBE64, UFp_t)
    GO(SDL_ReadLE16, uFp_t)
    GO(SDL_ReadLE32, uFp_t)
    GO(SDL_ReadLE64, UFp_t)
    GO(SDL_WriteU8, uFpC_t)
    GO(SDL_WriteBE16, uFpW_t)
    GO(SDL_WriteBE32, uFpu_t)
    GO(SDL_WriteBE64, uFpU_t)
    GO(SDL_WriteLE16, uFpW_t)
    GO(SDL_WriteLE32, uFpu_t)
    GO(SDL_WriteLE64, uFpU_t)
    GO(SDL_AddTimer, uFupp_t)
    GO(SDL_RemoveTimer, uFu_t)
    GO(SDL_CreateThread, pFppp_t)
    GO(SDL_KillThread, vFp_t)
    GO(SDL_SetEventFilter, vFpp_t)
    GO(SDL_LogSetOutputFunction, vFpp_t)
    GO(SDL_LogMessageV, vFiupp_t)
    GO(SDL_GL_GetProcAddress, pFp_t)
    GO(SDL_TLSSet, iFupp_t)
    GO(SDL_JoystickGetDeviceGUID, SFi_t)
    GO(SDL_JoystickGetGUID, SFp_t)
    GO(SDL_JoystickGetGUIDFromString, SFp_t)
    GO(SDL_GameControllerGetBindForAxis, SFpi_t)
    GO(SDL_GameControllerGetBindForButton, SFpi_t)
    GO(SDL_SetEventFilter, vFpp_t)
    GO(SDL_AddEventWatch, vFpp_t)
    GO(SDL_DelEventWatch, vFpp_t)
    GO(SDL_GameControllerMappingForGUID, pFS_t)
    GO(SDL_SaveAllDollarTemplates, iFp_t)
    GO(SDL_SaveDollarTemplate, iFip_t)
    #undef GO
    my->timercb = kh_init(timercb);
    my->threads = kh_init(timercb);
    return my;
}

void freeSDL2My(void* lib)
{
    sdl2_my_t *my = (sdl2_my_t *)lib;
    //x86emu_t *x;
    /*kh_foreach_value(my->timercb, x, 
        FreeCallback(x);
    );*/
    kh_destroy(timercb, my->timercb);

    /*kh_foreach_value(my->threads, x, 
        FreeCallback(x);
    );*/
    kh_destroy(timercb, my->threads);
    /*if(my->sdl2_evtfilter) {
        FreeCallback(my->sdl2_evtfilter);
    }*/
}


static void sdl2Callback(void *userdata, uint8_t *stream, int32_t len)
{
    x86emu_t *emu = (x86emu_t*) userdata;
    SetCallbackArg(emu, 1, stream);
    SetCallbackArg(emu, 2, (void*)len);
    RunCallback(emu);
}

static uint32_t sdl2TimerCallback(uint32_t interval, void *userdata)
{
    x86emu_t *emu = (x86emu_t*) userdata;
    SetCallbackArg(emu, 0, (void*)interval);
    uint32_t ret = RunCallback(emu);
    return ret;
}

static int32_t sdl2ThreadCallback(void *userdata)
{
    x86emu_t *emu = (x86emu_t*) userdata;
    int32_t ret = (int32_t)RunCallback(emu);
    FreeCallback(emu);
    return ret;
}

static int my2_eventfilter(void* userdata, void* event)
{
    x86emu_t *emu = (x86emu_t*)userdata;
    if(emu) {
        SetCallbackArg(emu, 1, event);
        return (int)RunCallback(emu);
    }
    return 0;
}

static void sdl2LogOutputCallback(void *userdata, int32_t category, uint32_t priority, const char* message)
{
    x86emu_t *emu = (x86emu_t*) userdata;
    SetCallbackArg(emu, 1, (void*)category);
    SetCallbackArg(emu, 2, (void*)priority);
    SetCallbackArg(emu, 3, (void*)message);
    RunCallback(emu);
}

// TODO: track the memory for those callback
EXPORT int32_t my2_SDL_OpenAudio(x86emu_t* emu, void* d, void* o)
{
    SDL2_AudioSpec *desired = (SDL2_AudioSpec*)d;

    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // create a callback
    void *fnc = (void*)desired->callback;
    void *olduser = desired->userdata;
    x86emu_t *cbemu = (fnc)?AddCallback(emu, (uintptr_t)fnc, 3, olduser, NULL, NULL, NULL):NULL;
    if(fnc) {
        desired->callback = sdl2Callback;
        desired->userdata = cbemu;
    }
    int ret = my->SDL_OpenAudio(desired, (SDL2_AudioSpec*)o);
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

EXPORT int32_t my2_SDL_OpenAudioDevice(x86emu_t* emu, void* device, int32_t iscapture, void* d, void* o, int32_t allowed)
{
    SDL2_AudioSpec *desired = (SDL2_AudioSpec*)d;

    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // create a callback
    void *fnc = (void*)desired->callback;
    void *olduser = desired->userdata;
    x86emu_t *cbemu = (fnc)?AddCallback(emu, (uintptr_t)fnc, 3, olduser, NULL, NULL, NULL):NULL;
    if(fnc) {
        desired->callback = sdl2Callback;
        desired->userdata = cbemu;
    }
    int ret = my->SDL_OpenAudioDevice(device, iscapture, desired, (SDL2_AudioSpec*)o, allowed);
    if (ret<=0) {
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

EXPORT void *my2_SDL_LoadFile_RW(x86emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->SDL_LoadFile_RW(rw, b);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT void *my2_SDL_LoadBMP_RW(x86emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->SDL_LoadBMP_RW(rw, b);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT int32_t my2_SDL_SaveBMP_RW(x86emu_t* emu, void* a, void* b, int c)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int32_t r = my->SDL_SaveBMP_RW(rw, b, c);
    if(c==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT void *my2_SDL_LoadWAV_RW(x86emu_t* emu, void* a, int b, void* c, void* d, void* e)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->SDL_LoadWAV_RW(rw, b, c, d, e);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT int32_t my2_SDL_GameControllerAddMappingsFromRW(x86emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int32_t r = my->SDL_GameControllerAddMappingsFromRW(rw, b);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_ReadU8(x86emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_ReadU8(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_ReadBE16(x86emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_ReadBE16(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_ReadBE32(x86emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_ReadBE32(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_ReadBE64(x86emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadBE64(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_ReadLE16(x86emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_ReadLE16(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_ReadLE32(x86emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_ReadLE32(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_ReadLE64(x86emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadLE64(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_WriteU8(x86emu_t* emu, void* a, uint8_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_WriteU8(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_WriteBE16(x86emu_t* emu, void* a, uint16_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_WriteBE16(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_WriteBE32(x86emu_t* emu, void* a, uint32_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_WriteBE32(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_WriteBE64(x86emu_t* emu, void* a, uint64_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_WriteBE64(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_WriteLE16(x86emu_t* emu, void* a, uint16_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_WriteLE16(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_WriteLE32(x86emu_t* emu, void* a, uint32_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_WriteLE32(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_WriteLE64(x86emu_t* emu, void* a, uint64_t v)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_WriteLE64(rw, v);
    RWNativeEnd2(rw);
    return r;
}

EXPORT void *my2_SDL_RWFromConstMem(x86emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    void* r = my->SDL_RWFromConstMem(a, b);
    return AddNativeRW2(emu, (SDL2_RWops_t*)r);
}
EXPORT void *my2_SDL_RWFromFP(x86emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    void* r = my->SDL_RWFromFP(a, b);
    return AddNativeRW2(emu, (SDL2_RWops_t*)r);
}
EXPORT void *my2_SDL_RWFromFile(x86emu_t* emu, void* a, void* b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    void* r = my->SDL_RWFromFile(a, b);
    return AddNativeRW2(emu, (SDL2_RWops_t*)r);
}
EXPORT void *my2_SDL_RWFromMem(x86emu_t* emu, void* a, int b)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    void* r = my->SDL_RWFromMem(a, b);
    return AddNativeRW2(emu, (SDL2_RWops_t*)r);
}

EXPORT int64_t my2_SDL_RWseek(x86emu_t* emu, void* a, int64_t offset, int32_t whence)
{
    //sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int64_t ret = RWNativeSeek2(rw, offset, whence);
    RWNativeEnd2(rw);
    return ret;
}
EXPORT int64_t my2_SDL_RWtell(x86emu_t* emu, void* a)
{
    //sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int64_t ret = RWNativeSeek2(rw, 0, 1);  //1 == RW_SEEK_CUR
    RWNativeEnd2(rw);
    return ret;
}
EXPORT uint32_t my2_SDL_RWread(x86emu_t* emu, void* a, void* ptr, uint32_t size, uint32_t maxnum)
{
    //sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t ret = RWNativeRead2(rw, ptr, size, maxnum);
    RWNativeEnd2(rw);
    return ret;
}
EXPORT uint32_t my2_SDL_RWwrite(x86emu_t* emu, void* a, const void* ptr, uint32_t size, uint32_t maxnum)
{
    //sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t ret = RWNativeWrite2(rw, ptr, size, maxnum);
    RWNativeEnd2(rw);
    return ret;
}
EXPORT int my2_SDL_RWclose(x86emu_t* emu, void* a)
{
    //sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    return RWNativeClose2(rw);
}

EXPORT int my2_SDL_SaveAllDollarTemplates(x86emu_t* emu, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t ret = my->SDL_SaveAllDollarTemplates(rw);
    RWNativeEnd2(rw);
    return ret;
}

EXPORT int my2_SDL_SaveDollarTemplate(x86emu_t* emu, int gesture, void* a)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t ret = my->SDL_SaveDollarTemplate(gesture, rw);
    RWNativeEnd2(rw);
    return ret;
}

EXPORT uint32_t my2_SDL_AddTimer(x86emu_t* emu, uint32_t a, void* cb, void* p)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    x86emu_t *cbemu = AddCallback(emu, (uintptr_t)cb, 2, NULL, p, NULL, NULL);
    uint32_t t = my->SDL_AddTimer(a, sdl2TimerCallback, cbemu);
    int ret;
    khint_t k = kh_put(timercb, my->timercb, t, &ret);
    kh_value(my->timercb, k) = cbemu;
    return t;
}

EXPORT void my2_SDL_RemoveTimer(x86emu_t* emu, uint32_t t)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    my->SDL_RemoveTimer(t);
    khint_t k = kh_get(timercb,my->timercb, t);
    if(k!=kh_end(my->timercb))
    {
        FreeCallback(kh_value(my->timercb, k));
        kh_del(timercb, my->timercb, k);
    }
}

EXPORT void my2_SDL_SetEventFilter(x86emu_t* emu, void* p, void* userdata)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    x86emu_t *oldcb = my->sdl2_evtfilter;
    if(p)
        my->sdl2_evtfilter = AddCallback(emu, (uintptr_t)p, 2, userdata, NULL, NULL, NULL);
    else
        my->sdl2_evtfilter = NULL;
    my->SDL_SetEventFilter(p?my2_eventfilter:NULL, my->sdl2_evtfilter);
    if(oldcb)
        FreeCallback(oldcb);
}
EXPORT void *my2_SDL_GetEventFilter(x86emu_t* emu)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    return my->sdl2_evtfnc;
}

EXPORT void my2_SDL_LogSetOutputFunction(x86emu_t* emu, void* cb, void* arg)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    if(my->sdl2_logouput) {
        my->SDL_LogSetOutputFunction(NULL, NULL);   // remove old one
        FreeCallback(my->sdl2_logouput);
        my->sdl2_logouput = NULL;
        my->sdl2_evtfnc = NULL;
    }
    if(cb) {
        my->sdl2_logouput = AddCallback(emu, (uintptr_t)cb, 4, arg, NULL, NULL, NULL);
        my->SDL_LogSetOutputFunction(sdl2LogOutputCallback, my->sdl2_logouput);
    }
}

EXPORT int my2_SDL_vsnprintf(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, *(uint32_t**)b, emu->scratch);
    void* f = vsnprintf;
    int r = ((iFpupp_t)f)(buff, s, fmt, emu->scratch);
    return r;
    #else
    void* f = vsnprintf;
    int r = ((iFpupp_t)f)(buff, s, fmt, *(uint32_t**)b);
    return r;
    #endif
}


void EXPORT *my2_SDL_CreateThread(x86emu_t* emu, void* cb, void* n, void* p)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    x86emu_t *cbemu = AddCallback(emu, (uintptr_t)cb, 1, p, NULL, NULL, NULL);
    void* t = my->SDL_CreateThread(sdl2ThreadCallback, n, cbemu);
    int ret;
    khint_t k = kh_put(timercb, my->threads, (uintptr_t)t, &ret);
    kh_value(my->threads, k) = cbemu;
    return t;
}

void EXPORT my2_SDL_KillThread(x86emu_t* emu, void* p)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    my->SDL_KillThread(p);
    khint_t k = kh_get(timercb,my->threads, (uintptr_t)p);
    if(k!=kh_end(my->threads))
    {
        FreeCallback(kh_value(my->threads, k));
        kh_del(timercb, my->threads, k);
    }
}

int EXPORT my2_SDL_snprintf(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, b, emu->scratch);
    void* f = vsnprintf;
    return ((iFpupp_t)f)(buff, s, fmt, emu->scratch);
    #else
    return vsnprintf((char*)buff, s, (char*)fmt, V);
    #endif
}

char EXPORT *my2_SDL_GetBasePath(x86emu_t* emu) {
    char* p = strdup(emu->context->fullpath);
    char* b = strrchr(p, '/');
    if(b)
        *(b+1) = '\0';
    return p;
}

EXPORT void my2_SDL_LogCritical(x86emu_t* emu, int32_t cat, void* fmt, void *b) {
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // SDL_LOG_PRIORITY_CRITICAL == 6
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    my->SDL_LogMessageV(cat, 6, fmt, emu->scratch);
    #else
    my->SDL_LogMessageV(cat, 6, fmt, b);
    #endif
}

EXPORT void my2_SDL_LogError(x86emu_t* emu, int32_t cat, void* fmt, void *b) {
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // SDL_LOG_PRIORITY_ERROR == 5
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    my->SDL_LogMessageV(cat, 5, fmt, emu->scratch);
    #else
    my->SDL_LogMessageV(cat, 5, fmt, b);
    #endif
}

EXPORT void my2_SDL_LogWarn(x86emu_t* emu, int32_t cat, void* fmt, void *b) {
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // SDL_LOG_PRIORITY_WARN == 4
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    my->SDL_LogMessageV(cat, 4, fmt, emu->scratch);
    #else
    my->SDL_LogMessageV(cat, 4, fmt, b);
    #endif
}

EXPORT void my2_SDL_LogInfo(x86emu_t* emu, int32_t cat, void* fmt, void *b) {
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // SDL_LOG_PRIORITY_INFO == 3
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    my->SDL_LogMessageV(cat, 3, fmt, emu->scratch);
    #else
    my->SDL_LogMessageV(cat, 3, fmt, b);
    #endif
}

EXPORT void my2_SDL_LogDebug(x86emu_t* emu, int32_t cat, void* fmt, void *b) {
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // SDL_LOG_PRIORITY_DEBUG == 2
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    my->SDL_LogMessageV(cat, 2, fmt, emu->scratch);
    #else
    my->SDL_LogMessageV(cat, 2, fmt, b);
    #endif
}

EXPORT void my2_SDL_LogVerbose(x86emu_t* emu, int32_t cat, void* fmt, void *b) {
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // SDL_LOG_PRIORITY_VERBOSE == 1
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    my->SDL_LogMessageV(cat, 1, fmt, emu->scratch);
    #else
    my->SDL_LogMessageV(cat, 1, fmt, b);
    #endif
}

EXPORT void my2_SDL_Log(x86emu_t* emu, void* fmt, void *b) {
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // SDL_LOG_PRIORITY_INFO == 3
    // SDL_LOG_CATEGORY_APPLICATION == 0
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    my->SDL_LogMessageV(0, 3, fmt, emu->scratch);
    #else
    my->SDL_LogMessageV(0, 3, fmt, b);
    #endif
}

void fillGLProcWrapper(box86context_t*);
EXPORT void* my2_SDL_GL_GetProcAddress(x86emu_t* emu, void* name) 
{
    khint_t k;
    const char* rname = (const char*)name;
    printf_log(LOG_DEBUG, "Calling SDL_GL_GetProcAddress(%s)\n", rname);
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    // check if glxprocaddress is filled, and search for lib and fill it if needed
    if(!emu->context->glwrappers)
        fillGLProcWrapper(emu->context);
    // get proc adress using actual glXGetProcAddress
    k = kh_get(symbolmap, emu->context->glmymap, rname);
    int is_my = (k==kh_end(emu->context->glmymap))?0:1;
    void* symbol;
    if(is_my) {
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        symbol = dlsym(emu->context->box86lib, tmp);
    } else 
        symbol = my->SDL_GL_GetProcAddress(name);
    if(!symbol)
        return NULL;    // easy
    // check if alread bridged
    uintptr_t ret = CheckBridged(emu->context->system, symbol);
    if(ret)
        return (void*)ret; // already bridged
    // get wrapper    
    k = kh_get(symbolmap, emu->context->glwrappers, rname);
    if(k==kh_end(emu->context->glwrappers) && strstr(rname, "ARB")==NULL) {
        // try again, adding ARB at the end if not present
        char tmp[200];
        strcpy(tmp, rname);
        strcat(tmp, "ARB");
        k = kh_get(symbolmap, emu->context->glwrappers, tmp);
    }
    if(k==kh_end(emu->context->glwrappers) && strstr(rname, "EXT")==NULL) {
        // try again, adding EXT at the end if not present
        char tmp[200];
        strcpy(tmp, rname);
        strcat(tmp, "EXT");
        k = kh_get(symbolmap, emu->context->glwrappers, tmp);
    }
    if(k==kh_end(emu->context->glwrappers)) {
        printf_log(LOG_INFO, "Warning, no wrapper for %s\n", rname);
        return NULL;
    }
    AddOffsetSymbol(emu->context->maplib, symbol, rname);
    return (void*)AddBridge(emu->context->system, kh_value(emu->context->glwrappers, k), symbol, 0);
}

#define nb_once	16
typedef void(*sdl2_tls_dtor)(void*);
static x86emu_t *dtor_emu[nb_once] = {0};
static void tls_dtor_callback(int n, void* a)
{
	if(dtor_emu[n]) {
		SetCallbackArg(dtor_emu[n], 0, a);
		RunCallback(dtor_emu[n]);
	}
}
#define GO(N) \
void tls_dtor_callback_##N(void* a) \
{ \
	tls_dtor_callback(N, a); \
}

GO(0)
GO(1)
GO(2)
GO(3)
GO(4)
GO(5)
GO(6)
GO(7)
GO(8)
GO(9)
GO(10)
GO(11)
GO(12)
GO(13)
GO(14)
GO(15)
#undef GO
static const sdl2_tls_dtor dtor_cb[nb_once] = {
	 tls_dtor_callback_0, tls_dtor_callback_1, tls_dtor_callback_2, tls_dtor_callback_3
	,tls_dtor_callback_4, tls_dtor_callback_5, tls_dtor_callback_6, tls_dtor_callback_7
	,tls_dtor_callback_8, tls_dtor_callback_9, tls_dtor_callback_10,tls_dtor_callback_11
	,tls_dtor_callback_12,tls_dtor_callback_13,tls_dtor_callback_14,tls_dtor_callback_15
};
EXPORT int32_t my2_SDL_TLSSet(x86emu_t* emu, uint32_t id, void* value, void* dtor)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;

	if(!dtor)
		return my->SDL_TLSSet(id, value, NULL);
	int n = 0;
	while (n<nb_once) {
		if(!dtor_emu[n] || GetCallbackAddress(dtor_emu[n])==((uintptr_t)dtor)) {
			if(!dtor_emu[n]) 
				dtor_emu[n] = AddCallback(emu, (uintptr_t)dtor, 1, NULL, NULL, NULL, NULL);
			return my->SDL_TLSSet(id, value, dtor_cb[n]);
		}
		++n;
	}
	printf_log(LOG_NONE, "Error: SDL2 SDL_TLSSet with destructor: no more slot!\n");
	//emu->quit = 1;
	return -1;
}

EXPORT void* my2_SDL_JoystickGetDeviceGUID(x86emu_t* emu, void* p, int32_t idx)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    *(SDL_JoystickGUID*)p = my->SDL_JoystickGetDeviceGUID(idx);
    return p;
}

EXPORT void* my2_SDL_JoystickGetGUID(x86emu_t* emu, void* p, void* joystick)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    *(SDL_JoystickGUID*)p = my->SDL_JoystickGetGUID(joystick);
    return p;
}

EXPORT void* my2_SDL_JoystickGetGUIDFromString(x86emu_t* emu, void* p, void* pchGUID)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    *(SDL_JoystickGUID*)p = my->SDL_JoystickGetGUIDFromString(pchGUID);
    return p;
}

EXPORT void* my2_SDL_GameControllerGetBindForAxis(x86emu_t* emu, void* p, void* controller, int32_t axis)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    *(SDL_GameControllerButtonBind*)p = my->SDL_GameControllerGetBindForAxis(controller, axis);
    return p;
}

EXPORT void* my2_SDL_GameControllerGetBindForButton(x86emu_t* emu, void* p, void* controller, int32_t button)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    *(SDL_GameControllerButtonBind*)p = my->SDL_GameControllerGetBindForButton(controller, button);
    return p;
}

EXPORT void* my2_SDL_GameControllerMappingForGUID(x86emu_t* emu, void* p)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    return my->SDL_GameControllerMappingForGUID(*(SDL_JoystickGUID*)p);
}

EXPORT void my2_SDL_AddEventWatch(x86emu_t* emu, void* p, void* userdata)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    x86emu_t *cb = NULL;
    if(p)
        cb = AddCallback(emu, (uintptr_t)p, 2, userdata, NULL, NULL, NULL);
    my->SDL_AddEventWatch(cb?my2_eventfilter:NULL, cb);
}
EXPORT void my2_SDL_DelEventWatch(x86emu_t* emu, void* p, void* userdata)
{
    sdl2_my_t *my = (sdl2_my_t *)emu->context->sdl2lib->priv.w.p2;
    x86emu_t *cb = NULL;
    // find callbacks that have function and userdata...    
    if(p)
        cb = GetCallback1Arg(emu, (uintptr_t)p, 2, userdata);
    my->SDL_AddEventWatch(cb?my2_eventfilter:NULL, cb);
}

// DL functions from wrappedlibdl.c
void* my_dlopen(x86emu_t* emu, void *filename, int flag);
int my_dlclose(x86emu_t* emu, void *handle);
void* my_dlsym(x86emu_t* emu, void *handle, void *symbol);
EXPORT void* my2_SDL_LoadObject(x86emu_t* emu, void* sofile)
{
    return my_dlopen(emu, sofile, 0);   // TODO: check correct flag value...
}
EXPORT void my2_SDL_UnloadObject(x86emu_t* emu, void* handle)
{
    my_dlclose(emu, handle);
}
EXPORT void* my2_SDL_LoadFunction(x86emu_t* emu, void* handle, void* name)
{
    return my_dlsym(emu, handle, name);
}

const char* sdl2Name = "libSDL2-2.0.so.0";
#define LIBNAME sdl2

#define CUSTOM_INIT \
    box86->sdl2lib = lib; \
    lib->priv.w.p2 = getSDL2My(lib); \
    box86->sdl2allocrw = ((sdl2_my_t*)lib->priv.w.p2)->SDL_AllocRW; \
    box86->sdl2freerw  = ((sdl2_my_t*)lib->priv.w.p2)->SDL_FreeRW; \
    lib->altmy = strdup("my2_"); \
    lib->priv.w.needed = 4; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libdl.so.2"); \
    lib->priv.w.neededlibs[1] = strdup("libm.so.6"); \
    lib->priv.w.neededlibs[2] = strdup("librt.so.1"); \
    lib->priv.w.neededlibs[3] = strdup("libpthread.so.0");

#define CUSTOM_FINI \
    freeSDL2My(lib->priv.w.p2); \
    free(lib->priv.w.p2); \
    ((box86context_t*)(lib->context))->sdl2lib = NULL; \
    ((box86context_t*)(lib->context))->sdl2allocrw = NULL; \
    ((box86context_t*)(lib->context))->sdl2freerw = NULL;


#include "wrappedlib_init.h"


