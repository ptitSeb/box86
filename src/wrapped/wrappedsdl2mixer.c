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
#include "callback.h"

const char* sdl2mixerName = "libSDL2_mixer-2.0.so.0";
#define LIBNAME sdl2mixer

typedef int (*iFiwC_t)(int, int16_t, uint8_t);

#define ADDED_FUNCTIONS() \
    GO(Mix_SetPosition, iFiwC_t)

#include "generated/wrappedsdl2mixertypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// EffectFunc
#define GO(A)   \
static uintptr_t my_EffectFunc_fct_##A = 0;                                             \
static void my_EffectFunc_##A(int chan, void *stream, int len, void *udata)             \
{                                                                                       \
    RunFunctionFmt(my_EffectFunc_fct_##A, "ipip", chan, stream, len, udata);\
}
SUPER()
#undef GO
static void* find_EffectFunc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_EffectFunc_fct_##A == (uintptr_t)fct) return my_EffectFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_EffectFunc_fct_##A == 0) {my_EffectFunc_fct_##A = (uintptr_t)fct; return my_EffectFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1Mixer EffectFunc callback\n");
    return NULL;
}

// EffectDone
#define GO(A)   \
static uintptr_t my_EffectDone_fct_##A = 0;                                 \
static void my_EffectDone_##A(int chan, void *udata)                        \
{                                                                           \
    RunFunctionFmt(my_EffectDone_fct_##A, "ip", chan, udata);   \
}
SUPER()
#undef GO
static void* find_EffectDone_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_EffectDone_fct_##A == (uintptr_t)fct) return my_EffectDone_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_EffectDone_fct_##A == 0) {my_EffectDone_fct_##A = (uintptr_t)fct; return my_EffectDone_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1Mixer EffectDone callback\n");
    return NULL;
}

// MixFunc
#define GO(A)   \
static uintptr_t my_MixFunc_fct_##A = 0;                                        \
static void my_MixFunc_##A(void *udata, uint8_t *stream, int len)               \
{                                                                               \
    RunFunctionFmt(my_MixFunc_fct_##A, "ppi", udata, stream, len);  \
}
SUPER()
#undef GO
static void* find_MixFunc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_MixFunc_fct_##A == (uintptr_t)fct) return my_MixFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_MixFunc_fct_##A == 0) {my_MixFunc_fct_##A = (uintptr_t)fct; return my_MixFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1Mixer MixFunc callback\n");
    return NULL;
}

// ChannelFinished
#define GO(A)   \
static uintptr_t my_ChannelFinished_fct_##A = 0;                            \
static void my_ChannelFinished_##A(int channel)                             \
{                                                                           \
    RunFunctionFmt(my_ChannelFinished_fct_##A, "i", channel);   \
}
SUPER()
#undef GO
static void* find_ChannelFinished_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_ChannelFinished_fct_##A == (uintptr_t)fct) return my_ChannelFinished_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ChannelFinished_fct_##A == 0) {my_ChannelFinished_fct_##A = (uintptr_t)fct; return my_ChannelFinished_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1Mixer ChannelFinished callback\n");
    return NULL;
}

// MusicFinished
#define GO(A)   \
static uintptr_t my_MusicFinished_fct_##A = 0;                  \
static void my_MusicFinished_##A()                              \
{                                                               \
    RunFunctionFmt(my_MusicFinished_fct_##A, "");   \
}
SUPER()
#undef GO
static void* find_MusicFinished_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_MusicFinished_fct_##A == (uintptr_t)fct) return my_MusicFinished_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_MusicFinished_fct_##A == 0) {my_MusicFinished_fct_##A = (uintptr_t)fct; return my_MusicFinished_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1Mixer MusicFinished callback\n");
    return NULL;
}

#undef SUPER

EXPORT void* my2_Mix_LoadMUSType_RW(x86emu_t* emu, void* a, int32_t b, int32_t c)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->Mix_LoadMUSType_RW(rw, b, c);
    if(c==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT void* my2_Mix_LoadMUS_RW(x86emu_t* emu, void* a, int32_t f)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->Mix_LoadMUS_RW(rw, f);
    if(f==0)
        RWNativeEnd2(rw);  // this one never free the RWops
    return r;
}
EXPORT void* my2_Mix_LoadWAV_RW(x86emu_t* emu, void* a, int32_t f)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->Mix_LoadWAV_RW(rw, f);
    if(f==0)
        RWNativeEnd2(rw);
    return r;
}

EXPORT void my2_Mix_SetPostMix(x86emu_t* emu, void* a, void* b)
{
    (void)emu;
    my->Mix_SetPostMix(find_MixFunc_Fct(a), b);
}

EXPORT int32_t my2_Mix_RegisterEffect(x86emu_t*emu, int32_t channel, void* cb_effect, void* cb_done, void* arg)
{
    (void)emu;
    return my->Mix_RegisterEffect(channel, find_EffectFunc_Fct(cb_effect), find_EffectDone_Fct(cb_done), arg);
}

EXPORT int32_t my2_Mix_UnregisterEffect(x86emu_t* emu, int channel, void* f)
{
    (void)emu;
    return my->Mix_UnregisterEffect(channel, find_EffectFunc_Fct(f));
}

EXPORT void my2_Mix_ChannelFinished(x86emu_t* emu, void* cb)
{
    (void)emu;
    my->Mix_ChannelFinished(find_ChannelFinished_Fct(cb));
}

EXPORT void my2_Mix_HookMusic(x86emu_t* emu, void* f, void* arg)
{
    (void)emu;
    my->Mix_HookMusic(find_MixFunc_Fct(f), arg);
}

EXPORT void my2_Mix_HookMusicFinished(x86emu_t* emu, void* f)
{
    (void)emu;
    my->Mix_HookMusicFinished(find_MusicFinished_Fct(f));
}

// This is a hack for AntiChamber
EXPORT int my2_MinorityMix_SetPosition(x86emu_t* emu, int channel, int16_t angle)
{
    (void)emu;
    return my->Mix_SetPosition(channel, angle, 0);
}

#define CUSTOM_INIT \
    box86->sdl2mixerlib = lib; \
    getMy(lib); \
    SETALT(my2_);

#define CUSTOM_FINI \
    freeMy(); \
    my_context->sdl2mixerlib = NULL;

#include "wrappedlib_init.h"
