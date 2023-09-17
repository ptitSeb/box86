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
#include "sdl1rwops.h"
#include "callback.h"

const char* sdl1mixerName = "libSDL_mixer-1.2.so.0";
#define LIBNAME sdl1mixer

#define ADDED_FUNCTIONS() \

#include "generated/wrappedsdl1mixertypes.h"

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

EXPORT void* my_Mix_LoadMUSType_RW(x86emu_t* emu, void* a, int32_t b, int32_t c)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->Mix_LoadMUSType_RW(rw, b, c);
    if(c==0)
        RWNativeEnd(rw);
    return r;
}
EXPORT void* my_Mix_LoadMUS_RW(x86emu_t* emu, void* a)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->Mix_LoadMUS_RW(rw);
    RWNativeEnd(rw);  // this one never free the RWops
    return r;
}
EXPORT void* my_Mix_LoadWAV_RW(x86emu_t* emu, void* a, int32_t b)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->Mix_LoadWAV_RW(rw, b);
    if(b==0)
        RWNativeEnd(rw);
    return r;
}

EXPORT void my_Mix_SetPostMix(x86emu_t* emu, void* a, void* b)
{
    (void)emu;
    my->Mix_SetPostMix(find_MixFunc_Fct(a), b);
}

EXPORT void my_Mix_ChannelFinished(x86emu_t* emu, void* cb)
{
    (void)emu;
    my->Mix_ChannelFinished(find_ChannelFinished_Fct(cb));
}

EXPORT void my_Mix_HookMusic(x86emu_t* emu, void* f, void* arg)
{
    (void)emu;
    my->Mix_HookMusic(find_MixFunc_Fct(f), arg);
}

EXPORT void my_Mix_HookMusicFinished(x86emu_t* emu, void* f)
{
    (void)emu;
    my->Mix_HookMusicFinished(find_MusicFinished_Fct(f));
}

EXPORT int my_Mix_RegisterEffect(x86emu_t* emu, int chan, void* f, void* d, void *arg)
{
    (void)emu;
    return my->Mix_RegisterEffect(chan, find_EffectFunc_Fct(f), find_EffectDone_Fct(d), arg);
}

EXPORT int my_Mix_UnregisterEffect(x86emu_t* emu, int channel, void* f)
{
    (void)emu;
    return my->Mix_UnregisterEffect(channel, find_EffectFunc_Fct(f));
}

#define CUSTOM_INIT \
    box86->sdl1mixerlib = lib; \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy(); \
    my_context->sdl1mixerlib = NULL;

#include "wrappedlib_init.h"
