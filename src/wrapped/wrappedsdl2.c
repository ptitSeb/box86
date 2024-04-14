#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "threads.h"
#include "gltools.h"

#include "generated/wrappedsdl2defs.h"

static void* my_glhandle = NULL;
// DL functions from wrappedlibdl.c
void* my_dlopen(x86emu_t* emu, void *filename, int flag);
int my_dlclose(x86emu_t* emu, void *handle);
void* my_dlsym(x86emu_t* emu, void *handle, void *symbol);

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
  uint16_t padding;
  uint32_t size;
  void (*callback)(void *userdata, uint8_t *stream, int32_t len);
  void *userdata;
} SDL2_AudioSpec;

typedef struct {
    uint8_t data[16];
} SDL_JoystickGUID;

typedef union {
    SDL_JoystickGUID guid;
    uint32_t         u[4];
    uint16_t         u16[8];
} SDL_JoystickGUID_Helper;

typedef struct
{
    uint32_t bindType;   // enum
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

const char* sdl2Name = "libSDL2-2.0.so.0";
#define LIBNAME sdl2

typedef void    (*vFv_t)();
typedef void    (*vFiupp_t)(int32_t, uint32_t, void*, void*);
typedef int32_t (*iFpLpp_t)(void*, size_t, void*, void*);
typedef int32_t (*iFpupp_t)(void*, uint32_t, void*, void*);

#define ADDED_FUNCTIONS() \
    GO(SDL_Quit, vFv_t)           \
    GO(SDL_AllocRW, sdl2_allocrw) \
    GO(SDL_FreeRW, sdl2_freerw)   \
    GO(SDL_LogMessageV, vFiupp_t)
#include "generated/wrappedsdl2types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// Timer
#define GO(A)   \
static uintptr_t my_Timer_fct_##A = 0;                                          \
static uint32_t my_Timer_##A(uint32_t a, void* b)                               \
{                                                                               \
    return (uint32_t)RunFunctionFmt(my_Timer_fct_##A, "up", a, b);  \
}
SUPER()
#undef GO
static void* find_Timer_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_Timer_fct_##A == (uintptr_t)fct) return my_Timer_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Timer_fct_##A == 0) {my_Timer_fct_##A = (uintptr_t)fct; return my_Timer_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL2 Timer callback\n");
    return NULL;
    
}
// AudioCallback
#define GO(A)   \
static uintptr_t my_AudioCallback_fct_##A = 0;                              \
static void my_AudioCallback_##A(void* a, void* b, int c)                   \
{                                                                           \
    RunFunctionFmt(my_AudioCallback_fct_##A, "ppi", a, b, c);   \
}
SUPER()
#undef GO
static void* find_AudioCallback_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_AudioCallback_fct_##A == (uintptr_t)fct) return my_AudioCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AudioCallback_fct_##A == 0) {my_AudioCallback_fct_##A = (uintptr_t)fct; return my_AudioCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL2 AudioCallback callback\n");
    return NULL;
    
}
// eventfilter
#define GO(A)   \
static uintptr_t my_eventfilter_fct_##A = 0;                                                \
static int my_eventfilter_##A(void* userdata, void* event)                                  \
{                                                                                           \
    return (int)RunFunctionFmt(my_eventfilter_fct_##A, "pp", userdata, event);  \
}
SUPER()
#undef GO
static void* find_eventfilter_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_eventfilter_fct_##A == (uintptr_t)fct) return my_eventfilter_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_eventfilter_fct_##A == 0) {my_eventfilter_fct_##A = (uintptr_t)fct; return my_eventfilter_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL2 eventfilter callback\n");
    return NULL;
    
}
static void* reverse_eventfilter_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_eventfilter_##A == fct) return (void*)my_eventfilter_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, iFpp, fct, 0, NULL);
}

// LogOutput
#define GO(A)   \
static uintptr_t my_LogOutput_fct_##A = 0;                                  \
static void my_LogOutput_##A(void* a, int b, int c, void* d)                \
{                                                                           \
    RunFunctionFmt(my_LogOutput_fct_##A, "piip", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_LogOutput_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_LogOutput_fct_##A == (uintptr_t)fct) return my_LogOutput_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_LogOutput_fct_##A == 0) {my_LogOutput_fct_##A = (uintptr_t)fct; return my_LogOutput_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL2 LogOutput callback\n");
    return NULL;
}
static void* reverse_LogOutput_Fct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_LogOutput_##A == fct) return (void*)my_LogOutput_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, vFpiip, fct, 0, NULL);
}

#undef SUPER

// TODO: track the memory for those callback
EXPORT int32_t my2_SDL_OpenAudio(x86emu_t* emu, void* d, void* o)
{
    (void)emu;
    SDL2_AudioSpec *desired = (SDL2_AudioSpec*)d;

    // create a callback
    void *fnc = (void*)desired->callback;
    desired->callback = find_AudioCallback_Fct(fnc);
    int ret = my->SDL_OpenAudio(desired, (SDL2_AudioSpec*)o);
    if (ret!=0) {
        // error, clean the callback...
        desired->callback = fnc;
        return ret;
    }
    // put back stuff in place?
    desired->callback = fnc;

    return ret;
}

EXPORT int32_t my2_SDL_OpenAudioDevice(x86emu_t* emu, void* device, int32_t iscapture, void* d, void* o, int32_t allowed)
{
    (void)emu;
    SDL2_AudioSpec *desired = (SDL2_AudioSpec*)d;

    // create a callback
    void *fnc = (void*)desired->callback;
    desired->callback = find_AudioCallback_Fct(fnc);
    int ret = my->SDL_OpenAudioDevice(device, iscapture, desired, (SDL2_AudioSpec*)o, allowed);
    if (ret<=0) {
        // error, clean the callback...
        desired->callback = fnc;
        return ret;
    }
    // put back stuff in place?
    desired->callback = fnc;

    return ret;
}

EXPORT void *my2_SDL_LoadFile_RW(x86emu_t* emu, void* a, void* b, int c)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->SDL_LoadFile_RW(rw, b, c);
    if(c==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT void *my2_SDL_LoadBMP_RW(x86emu_t* emu, void* a, int b)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->SDL_LoadBMP_RW(rw, b);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT int32_t my2_SDL_SaveBMP_RW(x86emu_t* emu, void* a, void* b, int c)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int32_t r = my->SDL_SaveBMP_RW(rw, b, c);
    if(c==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT void *my2_SDL_LoadWAV_RW(x86emu_t* emu, void* a, int b, void* c, void* d, void* e)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    void* r = my->SDL_LoadWAV_RW(rw, b, c, d, e);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT int32_t my2_SDL_GameControllerAddMappingsFromRW(x86emu_t* emu, void* a, int b)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int32_t r = my->SDL_GameControllerAddMappingsFromRW(rw, b);
    if(b==0)
        RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_ReadU8(x86emu_t* emu, void* a)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_ReadU8(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_ReadBE16(x86emu_t* emu, void* a)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_ReadBE16(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_ReadBE32(x86emu_t* emu, void* a)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_ReadBE32(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_ReadBE64(x86emu_t* emu, void* a)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadBE64(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_ReadLE16(x86emu_t* emu, void* a)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_ReadLE16(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_ReadLE32(x86emu_t* emu, void* a)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_ReadLE32(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint64_t my2_SDL_ReadLE64(x86emu_t* emu, void* a)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint64_t r = my->SDL_ReadLE64(rw);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_WriteU8(x86emu_t* emu, void* a, uint8_t v)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_WriteU8(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_WriteBE16(x86emu_t* emu, void* a, uint16_t v)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_WriteBE16(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_WriteBE32(x86emu_t* emu, void* a, uint32_t v)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_WriteBE32(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_WriteBE64(x86emu_t* emu, void* a, uint64_t v)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_WriteBE64(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_WriteLE16(x86emu_t* emu, void* a, uint16_t v)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_WriteLE16(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_WriteLE32(x86emu_t* emu, void* a, uint32_t v)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_WriteLE32(rw, v);
    RWNativeEnd2(rw);
    return r;
}
EXPORT uint32_t my2_SDL_WriteLE64(x86emu_t* emu, void* a, uint64_t v)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t r = my->SDL_WriteLE64(rw, v);
    RWNativeEnd2(rw);
    return r;
}

EXPORT void *my2_SDL_RWFromConstMem(x86emu_t* emu, void* a, int b)
{
    void* r = my->SDL_RWFromConstMem(a, b);
    return AddNativeRW2(emu, (SDL2_RWops_t*)r);
}
EXPORT void *my2_SDL_RWFromFP(x86emu_t* emu, void* a, int b)
{
    void* r = my->SDL_RWFromFP(a, b);
    return AddNativeRW2(emu, (SDL2_RWops_t*)r);
}
EXPORT void *my2_SDL_RWFromFile(x86emu_t* emu, void* a, void* b)
{
    void* r = my->SDL_RWFromFile(a, b);
    return AddNativeRW2(emu, (SDL2_RWops_t*)r);
}
EXPORT void *my2_SDL_RWFromMem(x86emu_t* emu, void* a, int b)
{
    void* r = my->SDL_RWFromMem(a, b);
    return AddNativeRW2(emu, (SDL2_RWops_t*)r);
}

EXPORT int64_t my2_SDL_RWseek(x86emu_t* emu, void* a, int64_t offset, int32_t whence)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int64_t ret = RWNativeSeek2(rw, offset, whence);
    RWNativeEnd2(rw);
    return ret;
}
EXPORT int64_t my2_SDL_RWsize(x86emu_t* emu, void* a)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int64_t ret = my->SDL_RWsize(rw);
    RWNativeEnd2(rw);
    return ret;
}
EXPORT int64_t my2_SDL_RWtell(x86emu_t* emu, void* a)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    int64_t ret = RWNativeSeek2(rw, 0, 1);  //1 == RW_SEEK_CUR
    RWNativeEnd2(rw);
    return ret;
}
EXPORT uint32_t my2_SDL_RWread(x86emu_t* emu, void* a, void* ptr, uint32_t size, uint32_t maxnum)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t ret = RWNativeRead2(rw, ptr, size, maxnum);
    RWNativeEnd2(rw);
    return ret;
}
EXPORT uint32_t my2_SDL_RWwrite(x86emu_t* emu, void* a, const void* ptr, uint32_t size, uint32_t maxnum)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t ret = RWNativeWrite2(rw, ptr, size, maxnum);
    RWNativeEnd2(rw);
    return ret;
}
EXPORT int my2_SDL_RWclose(x86emu_t* emu, void* a)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    return RWNativeClose2(rw);
}

EXPORT int my2_SDL_SaveAllDollarTemplates(x86emu_t* emu, void* a)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t ret = my->SDL_SaveAllDollarTemplates(rw);
    RWNativeEnd2(rw);
    return ret;
}

EXPORT int my2_SDL_SaveDollarTemplate(x86emu_t* emu, int gesture, void* a)
{
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)a);
    uint32_t ret = my->SDL_SaveDollarTemplate(gesture, rw);
    RWNativeEnd2(rw);
    return ret;
}

EXPORT void *my2_SDL_AddTimer(x86emu_t* emu, uint32_t a, void* f, void* p)
{
    (void)emu;
    return my->SDL_AddTimer(a, find_Timer_Fct(f), p);
}

EXPORT int my2_SDL_RemoveTimer(x86emu_t* emu, void *t)
{
    (void)emu;
    return my->SDL_RemoveTimer(t);
}

EXPORT void my2_SDL_SetEventFilter(x86emu_t* emu, void* p, void* userdata)
{
    (void)emu;
    my->SDL_SetEventFilter(find_eventfilter_Fct(p), userdata);
}
EXPORT int my2_SDL_GetEventFilter(x86emu_t* emu, void** f, void* userdata)
{
    (void)emu;
    int ret = my->SDL_GetEventFilter(f, userdata);
    *f = reverse_eventfilter_Fct(*f);
    return ret;
}

EXPORT void my2_SDL_LogGetOutputFunction(x86emu_t* emu, void** f, void* arg)
{
    (void)emu;
    my->SDL_LogGetOutputFunction(f, arg);
    if(*f) *f = reverse_LogOutput_Fct(*f);
}
EXPORT void my2_SDL_LogSetOutputFunction(x86emu_t* emu, void* f, void* arg)
{
    (void)emu;
    my->SDL_LogSetOutputFunction(find_LogOutput_Fct(f), arg);
}

EXPORT int my2_SDL_vsnprintf(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    void* f = vsnprintf;
    return ((iFpLpp_t)f)(buff, s, fmt, VARARGS);
    #else
    (void)emu;
    return vsnprintf(buff, s, fmt, b);
    #endif
}

EXPORT void* my2_SDL_CreateThread(x86emu_t* emu, void* f, void* n, void* p)
{
    void* et = NULL;
    return my->SDL_CreateThread(my_prepare_thread(emu, f, p, 0, &et), n, et);
}

EXPORT int my2_SDL_snprintf(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    void* f = vsnprintf;
    return ((iFpLpp_t)f)(buff, s, fmt, VARARGS);
    #else
    (void)emu;
    return vsnprintf((char*)buff, s, (char*)fmt, b);
    #endif
}

static int get_sdl_priv(x86emu_t* emu, const char *sym_str, void **w, void **f)
{
    #define GO(sym, _w) \
        else if (strcmp(#sym, sym_str) == 0) \
        { \
            *w = _w; \
            *f = dlsym(emu->context->sdl2lib->w.lib, #sym); \
            return *f != NULL; \
        }
    #define GO2(sym, _w, sym2) \
        else if (strcmp(#sym, sym_str) == 0) \
        { \
            *w = _w; \
            *f = dlsym(emu->context->sdl2lib->w.lib, #sym2); \
            return *f != NULL; \
        }
    #define GOM(sym, _w) \
        else if (strcmp(#sym, sym_str) == 0) \
        { \
            *w = _w; \
            *f = dlsym(emu->context->box86lib, "my2_"#sym); \
            return *f != NULL; \
        }
    #define GOS(sym, _w) GOM(sym, _w)
    #define DATA
    
    if(0);
    #include "wrappedsdl2_private.h"

    #undef GO
    #undef GOM
    #undef GO2
    #undef GOS
    #undef DATA
    return 0;
}

int EXPORT my2_SDL_DYNAPI_entry(x86emu_t* emu, uint32_t version, uintptr_t *table, uint32_t tablesize)
{
    uint32_t i = 0;
    uintptr_t tab[tablesize];
    int r = my->SDL_DYNAPI_entry(version, tab, tablesize);
    (void)r;
    
    #define SDL_DYNAPI_PROC(ret, sym, args, parms, ...) \
        if (i < tablesize) { \
            void *w = NULL; \
            void *f = NULL; \
            if (get_sdl_priv(emu, #sym, &w, &f)) { \
                table[i] = AddCheckBridge(my_lib->w.bridge, w, f, 0, #sym); \
            } \
            else \
                table[i] = (uintptr_t)NULL; \
            printf_log(LOG_DEBUG, "SDL_DYNAPI_entry: %s => %p (%p)\n", #sym, (void*)table[i], f); \
            i++; \
        }

    #include "SDL_dynapi_procs.h"
    return 0;
}

EXPORT void *my2_SDL_CreateWindow(x86emu_t* emu, const char *title, int x, int y, int w, int h, uint32_t flags)
{
    #ifdef GOA_CLONE
    // For GO Advance clones, ignores the requested resolution and just uses the entire screen
    x = y = 0;
    w = h = -1;
    #endif

    // Set BOX86_FORCE_ES=MN or BOX86_FORCE_ES=M to force a specific OpenGL ES version.
    // M = major version, N = minor version
    const char *force_es = getenv("BOX86_FORCE_ES");
    if (force_es && *force_es && (force_es[0] != '0')) {
        #define SDL_GL_CONTEXT_PROFILE_MASK 21
        #define SDL_GL_CONTEXT_PROFILE_ES 4
        #define SDL_GL_CONTEXT_MAJOR_VERSION 17
        #define SDL_GL_CONTEXT_MINOR_VERSION 18
        #define SDL_WINDOW_OPENGL 2
        // Is BOX86_FORCE_ES incorrectly formatted?
        if (!isdigit(force_es[0]) || (force_es[1] != '\0' && !isdigit(force_es[1]))) {
            printf_log(LOG_NONE, "Warning: ignoring malformed BOX86_FORCE_ES.\n");
        } else {
            int (*SDL_GL_SetAttribute_p)(uint32_t, int) = dlsym(emu->context->sdl2lib->w.lib, "SDL_GL_SetAttribute");
            SDL_GL_SetAttribute_p(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
            SDL_GL_SetAttribute_p(SDL_GL_CONTEXT_MAJOR_VERSION, force_es[0] - '0');
            SDL_GL_SetAttribute_p(SDL_GL_CONTEXT_MINOR_VERSION, force_es[1] ? (force_es[1] - '0') : 0);
            flags |= SDL_WINDOW_OPENGL;
        }
    }

    return my->SDL_CreateWindow((void*)title, x, y, w, h, flags);
}

char EXPORT *my2_SDL_GetBasePath(x86emu_t* emu) {
    char* p = strdup(emu->context->fullpath);
    char* b = strrchr(p, '/');
    if(b)
        *(b+1) = '\0';
    return p;
}

EXPORT void my2_SDL_LogCritical(x86emu_t* emu, int32_t cat, void* fmt, void *b) {
    // SDL_LOG_PRIORITY_CRITICAL == 6
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    my->SDL_LogMessageV(cat, 6, fmt, VARARGS);
    #else
    (void)emu;
    my->SDL_LogMessageV(cat, 6, fmt, b);
    #endif
}

EXPORT void my2_SDL_LogError(x86emu_t* emu, int32_t cat, void* fmt, void *b) {
    // SDL_LOG_PRIORITY_ERROR == 5
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    my->SDL_LogMessageV(cat, 5, fmt, VARARGS);
    #else
    (void)emu;
    my->SDL_LogMessageV(cat, 5, fmt, b);
    #endif
}

EXPORT void my2_SDL_LogWarn(x86emu_t* emu, int32_t cat, void* fmt, void *b) {
    // SDL_LOG_PRIORITY_WARN == 4
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    my->SDL_LogMessageV(cat, 4, fmt, VARARGS);
    #else
    (void)emu;
    my->SDL_LogMessageV(cat, 4, fmt, b);
    #endif
}

EXPORT void my2_SDL_LogInfo(x86emu_t* emu, int32_t cat, void* fmt, void *b) {
    // SDL_LOG_PRIORITY_INFO == 3
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    my->SDL_LogMessageV(cat, 3, fmt, VARARGS);
    #else
    (void)emu;
    my->SDL_LogMessageV(cat, 3, fmt, b);
    #endif
}

EXPORT void my2_SDL_LogDebug(x86emu_t* emu, int32_t cat, void* fmt, void *b) {
    // SDL_LOG_PRIORITY_DEBUG == 2
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    my->SDL_LogMessageV(cat, 2, fmt, VARARGS);
    #else
    (void)emu;
    my->SDL_LogMessageV(cat, 2, fmt, b);
    #endif
}

EXPORT void my2_SDL_LogVerbose(x86emu_t* emu, int32_t cat, void* fmt, void *b) {
    // SDL_LOG_PRIORITY_VERBOSE == 1
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    my->SDL_LogMessageV(cat, 1, fmt, VARARGS);
    #else
    (void)emu;
    my->SDL_LogMessageV(cat, 1, fmt, b);
    #endif
}

EXPORT void my2_SDL_Log(x86emu_t* emu, void* fmt, void *b) {
    // SDL_LOG_PRIORITY_INFO == 3
    // SDL_LOG_CATEGORY_APPLICATION == 0
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    my->SDL_LogMessageV(0, 3, fmt, VARARGS);
    #else
    (void)emu;
    my->SDL_LogMessageV(0, 3, fmt, b);
    #endif
}

EXPORT void* my2_SDL_GL_GetProcAddress(x86emu_t* emu, void* name) 
{
    khint_t k;
    const char* rname = (const char*)name;
    void* ret = getGLProcAddress(emu, (glprocaddress_t)my->SDL_GL_GetProcAddress, rname);
    #ifndef GOA_CLONE
    static int lib_checked = 0;
    if(!lib_checked) {
        lib_checked = 1;
            // check if libGL is loaded, load it if not (helps some Haxe games, like DeadCells or Nuclear Blaze)
        if(!my_glhandle && !GetLibInternal(box86_libGL?box86_libGL:"libGL.so.1"))
            // use a my_dlopen to actually open that lib, like SDL2 is doing...
            my_glhandle = my_dlopen(emu, box86_libGL?box86_libGL:"libGL.so.1", RTLD_LAZY|RTLD_GLOBAL);
    }
    #endif
    return ret;
}

#define nb_once	16
typedef void(*sdl2_tls_dtor)(void*);
static uintptr_t dtor_emu[nb_once] = {0};
static void tls_dtor_callback(int n, void* a)
{
	if(dtor_emu[n]) {
        RunFunctionFmt(dtor_emu[n], "p", a);
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
    (void)emu;
	if(!dtor)
		return my->SDL_TLSSet(id, value, NULL);
	int n = 0;
	while (n<nb_once) {
		if(!dtor_emu[n] || (dtor_emu[n])==((uintptr_t)dtor)) {
			dtor_emu[n] = (uintptr_t)dtor;
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
    (void)emu;
    *(SDL_JoystickGUID*)p = my->SDL_JoystickGetDeviceGUID(idx);
    return p;
}

EXPORT void* my2_SDL_JoystickGetGUID(x86emu_t* emu, void* p, void* joystick)
{
    (void)emu;
    *(SDL_JoystickGUID*)p = my->SDL_JoystickGetGUID(joystick);
    return p;
}

EXPORT void* my2_SDL_JoystickGetGUIDFromString(x86emu_t* emu, void* p, void* pchGUID)
{
    (void)emu;
    *(SDL_JoystickGUID*)p = my->SDL_JoystickGetGUIDFromString(pchGUID);
    return p;
}

EXPORT void* my2_SDL_GameControllerGetBindForAxis(x86emu_t* emu, void* p, void* controller, int32_t axis)
{
    (void)emu;
    *(SDL_GameControllerButtonBind*)p = my->SDL_GameControllerGetBindForAxis(controller, axis);
    return p;
}

EXPORT void* my2_SDL_GameControllerGetBindForButton(x86emu_t* emu, void* p, void* controller, int32_t button)
{
    (void)emu;
    *(SDL_GameControllerButtonBind*)p = my->SDL_GameControllerGetBindForButton(controller, button);
    return p;
}

EXPORT void my2_SDL_AddEventWatch(x86emu_t* emu, void* p, void* userdata)
{
    (void)emu;
    my->SDL_AddEventWatch(find_eventfilter_Fct(p), userdata);
}
EXPORT void my2_SDL_DelEventWatch(x86emu_t* emu, void* p, void* userdata)
{
    (void)emu;
    my->SDL_DelEventWatch(find_eventfilter_Fct(p), userdata);
}

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

EXPORT void my2_SDL_GetJoystickGUIDInfo(x86emu_t* emu, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint16_t* vendor, uint16_t* product, uint16_t* version, uint16_t* crc16)
{
    (void)emu;
    uint16_t dummy = 0;
    SDL_JoystickGUID_Helper guid;
    guid.u[0] = a;
    guid.u[1] = b;
    guid.u[2] = c;
    guid.u[3] = d;
    if(my->SDL_GetJoystickGUIDInfo) {
        my->SDL_GetJoystickGUIDInfo(guid.guid, vendor, product, version, box86_sdl2_jguid?(&dummy):crc16);
    } else {
        // dummy, set everything to "unknown"
        if (guid.u16[1]==0x0000 && guid.u16[3]==0x0000 && guid.u16[5]==0x0000)
            {
            if(vendor) *vendor = guid.u16[2];
            if(product) *product = guid.u16[4];
            if(version)  *version  = guid.u16[6];
        } else {
            if(vendor)  *vendor = 0;
            if(product) *product = 0;
            if(version) *version = 0;
        }
    }
}

EXPORT int32_t my2_SDL_IsJoystickPS4(x86emu_t* emu, uint16_t vendor, uint16_t product_id)
{
    (void)emu;
    if(my->SDL_IsJoystickPS4)
        return my->SDL_IsJoystickPS4(vendor, product_id);
    // fallback
    return 0;
}
EXPORT int32_t my2_SDL_IsJoystickNintendoSwitchPro(x86emu_t* emu, uint16_t vendor, uint16_t product_id)
{
    (void)emu;
    if(my->SDL_IsJoystickNintendoSwitchPro)
        return my->SDL_IsJoystickNintendoSwitchPro(vendor, product_id);
    // fallback
    return 0;
}
EXPORT int32_t my2_SDL_IsJoystickSteamController(x86emu_t* emu, uint16_t vendor, uint16_t product_id)
{
    (void)emu;
    if(my->SDL_IsJoystickSteamController)
        return my->SDL_IsJoystickSteamController(vendor, product_id);
    // fallback
    return 0;
}
EXPORT int32_t my2_SDL_IsJoystickXbox360(x86emu_t* emu, uint16_t vendor, uint16_t product_id)
{
    (void)emu;
    if(my->SDL_IsJoystickXbox360)
        return my->SDL_IsJoystickXbox360(vendor, product_id);
    // fallback
    return 0;
}
EXPORT int32_t my2_SDL_IsJoystickXboxOne(x86emu_t* emu, uint16_t vendor, uint16_t product_id)
{
    (void)emu;
    if(my->SDL_IsJoystickXboxOne)
        return my->SDL_IsJoystickXboxOne(vendor, product_id);
    // fallback
    return 0;
}
EXPORT int32_t my2_SDL_IsJoystickXInput(x86emu_t* emu, SDL_JoystickGUID p)
{
    (void)emu;
    if(my->SDL_IsJoystickXInput)
        return my->SDL_IsJoystickXInput(p);
    // fallback
    return 0;
}
EXPORT int32_t my2_SDL_IsJoystickHIDAPI(x86emu_t* emu, SDL_JoystickGUID p)
{
    (void)emu;
    if(my->SDL_IsJoystickHIDAPI)
        return my->SDL_IsJoystickHIDAPI(p);
    // fallback
    return 0;
}

void* my_vkGetInstanceProcAddr(x86emu_t* emu, void* device, void* name);
EXPORT void* my2_SDL_Vulkan_GetVkGetInstanceProcAddr(x86emu_t* emu)
{
    if(!emu->context->vkprocaddress)
        emu->context->vkprocaddress = (vkprocaddress_t)my->SDL_Vulkan_GetVkGetInstanceProcAddr();

    if(emu->context->vkprocaddress)
        return (void*)AddCheckBridge(my_lib->w.bridge, pFEpp, my_vkGetInstanceProcAddr, 0, "vkGetInstanceProcAddr");
    return NULL;
}

#define CUSTOM_INIT \
    box86->sdl2lib = lib;                   \
    getMy(lib);                             \
    box86->sdl2allocrw = my->SDL_AllocRW;   \
    box86->sdl2freerw  = my->SDL_FreeRW;    \
    SETALT(my2_);                           \
    setNeededLibs(lib, 4,                   \
        "libdl.so.2",                       \
        "libm.so.6",                        \
        "librt.so.1",                       \
        "libpthread.so.0");

#define CUSTOM_FINI \
    my->SDL_Quit();                                             \
    if(my_glhandle) my_dlclose(thread_get_emu(), my_glhandle);  \
    my_glhandle = NULL;                                         \
    freeMy();                                                   \
    my_context->sdl2lib = NULL;                                 \
    my_context->sdl2allocrw = NULL;                             \
    my_context->sdl2freerw = NULL;


#include "wrappedlib_init.h"
