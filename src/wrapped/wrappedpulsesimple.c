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
#include "callback.h"
#include "box86context.h"
#include "librarian.h"
#include "myalign.h"

const char* pulsesimpleName = "libpulse-simple.so.0";
#define LIBNAME pulsesimple
#if 0
typedef void* (*pFppp_t)(void*, void*, void*);
typedef void (*vFppp_t)(void*, void*, void*);
#ifdef NOALIGN
typedef void (*vFipippV_t)(int, void*, int, void*, void*, va_list);
#else
typedef void (*vFipippV_t)(int, void*, int, void*, void*, void*);
#endif

typedef struct pulsesimple_my_s {
    // functions
    vFppp_t     pa_context_set_state_callback;
    vFipippV_t  pa_log_level_meta;
    pFppp_t     pa_stream_drain;
    pFppp_t     pa_stream_flush;
    vFppp_t     pa_stream_set_latency_update_callback;
    vFppp_t     pa_stream_set_read_callback;
    vFppp_t     pa_stream_set_state_callback;
    // other
} pulsesimple_my_t;

void* getPulseSimpleMy(library_t* lib)
{
    pulsesimple_my_t* my = (pulsesimple_my_t*)calloc(1, sizeof(pulsesimple_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(pa_context_set_state_callback, vFppp_t)
    GO(pa_log_level_meta, vFipippV_t)
    GO(pa_stream_drain, pFppp_t)
    GO(pa_stream_flush, pFppp_t)
    GO(pa_stream_set_latency_update_callback, vFppp_t)
    GO(pa_stream_set_read_callback, vFppp_t)
    GO(pa_stream_set_state_callback, vFppp_t)
    #undef GO
    return my;
}

void freePulseSimpleMy(void* lib)
{
    pulsesimple_my_t *my = (pulsesimple_my_t *)lib;
    if(my->request)
        FreeCallback(my->request);
    my->request = NULL;
}


// most function comes from libpulse.so
static void my_notify_context(void* context, void* data)
{
    x86emu_t* emu = (x86emu_t*)data;
    if(emu) {
        SetCallbackArg(emu, 0, context);
        RunCallback(emu);
    }
}
EXPORT void my_pa_context_set_state_callback(x86emu_t* emu, void* context, void* cb, void* data)
{
    pulsesimple_my_t* my = (pulsesimple_my_t*)GetLib(emu->context->maplib, openalName)->priv.w.p2;
    x86emu_t *cb_emu = (cb)?AddCallback(emu, (uintptr_t)cb, 2, NULL, data, NULL, NULL):NULL;
    my->pa_context_set_state_callback(context, cb?my_notify_context:NULL, cb_emu);  // what mecanism to free the CB
}

static void* my_stream_success(void* s, int success, void* data)
{
    x86emu_t* emu = (x86emu_t*)data;
    if(emu) {
        SetCallbackArg(emu, 0, s);
        SetCallbackArg(emu, 1, (void*)success);
        RunCallback(emu);
    }
}
EXPORT void* my_pa_stream_drain(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulsesimple_my_t* my = (pulsesimple_my_t*)GetLib(emu->context->maplib, openalName)->priv.w.p2;
    x86emu_t *cb_emu = (cb)?AddCallback(emu, (uintptr_t)cb, 3, NULL, NULL, data, NULL):NULL;
    my->pa_stream_drain(stream, cb?my_stream_success:NULL, cb_emu);  // what mecanism to free the CB
}

EXPORT void* my_pa_stream_flush(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulsesimple_my_t* my = (pulsesimple_my_t*)GetLib(emu->context->maplib, openalName)->priv.w.p2;
    x86emu_t *cb_emu = (cb)?AddCallback(emu, (uintptr_t)cb, 3, NULL, NULL, data, NULL):NULL;
    my->pa_stream_flush(stream, cb?my_stream_success:NULL, cb_emu);  // what mecanism to free the CB
}

static void* my_stream_notify(void* s, void* data)
{
    x86emu_t* emu = (x86emu_t*)data;
    if(emu) {
        SetCallbackArg(emu, 0, s);
        RunCallback(emu);
    }
}
EXPORT void my_pa_stream_set_latency_update_callback(x86emu_t* emu, void* s, void* cb, void* data)
{
    pulsesimple_my_t* my = (pulsesimple_my_t*)GetLib(emu->context->maplib, openalName)->priv.w.p2;
    x86emu_t *cb_emu = (cb)?AddCallback(emu, (uintptr_t)cb, 2, NULL, data, NULL, NULL):NULL;
    my->pa_stream_set_latency_update_callback(stream, cb?my_stream_notify:NULL, cb_emu);  // what mecanism to free the CB
}

static void* my_stream_request(void* s, int nbytes, void* data)
{
    x86emu_t* emu = (x86emu_t*)data;
    if(emu) {
        SetCallbackArg(emu, 0, s);
        SetCallbackArg(emu, 1, (void*)nbytes));
        RunCallback(emu);
    }
}
EXPORT void my_pa_stream_set_read_callback(x86emu_t* emu, void* s, void* cb, void* data)
{
    pulsesimple_my_t* my = (pulsesimple_my_t*)GetLib(emu->context->maplib, openalName)->priv.w.p2;
    x86emu_t *cb_emu = (cb)?AddCallback(emu, (uintptr_t)cb, 2, NULL, data, NULL, NULL):NULL;
    my->pa_stream_set_read_callback(stream, cb?my_stream_request:NULL, cb_emu);  // what mecanism to free the CB
}

EXPORT void my_pa_stream_set_state_callback(x86emu_t* emu, void* s, void* cb, void* data)
{
    pulsesimple_my_t* my = (pulsesimple_my_t*)GetLib(emu->context->maplib, openalName)->priv.w.p2;
    x86emu_t *cb_emu = (cb)?AddCallback(emu, (uintptr_t)cb, 2, NULL, data, NULL, NULL):NULL;
    my->pa_stream_set_state_callback(stream, cb?my_stream_notify:NULL, cb_emu);  // what mecanism to free the CB
}

EXPORT void my_pa_log_level_meta(x86emu_t* emu, int level, void* file, int line, void* func, void* format, void* b, va_list V)
{
    pulsesimple_my_t* my = (pulsesimple_my_t*)GetLib(emu->context->maplib, openalName)->priv.w.p2;
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)format, b, emu->scratch);
    return my->pa_log_level_meta(level, file, line, func, format, emu->scratch);
    #else
    return my->pa_log_level_meta(level, file, line, func, format, V);
    #endif
}


#define CUSTOM_INIT \
    lib->priv.w.p2 = getPulseSimpleMy(lib);
// should also load libpulse.so

#define CUSTOM_FINI \
    freePulseSimpleMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#endif

#include "wrappedlib_init.h"

