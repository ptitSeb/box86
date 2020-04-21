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
#include "callback.h"
#include "box86context.h"
#include "librarian.h"
#include "myalign.h"
#include "khash.h"

const char* pulseName = "libpulse.so.0";
#define LIBNAME pulse

typedef struct my_pulse_cb_s {
    uintptr_t fnc;
    void*     data;
} my_pulse_cb_t;
typedef struct my_pulse_context_cb_s {
    my_pulse_cb_t cb[8];
    int        size;
    struct my_pulse_context_cb_s* next;
} my_pulse_context_cb_t;

KHASH_MAP_INIT_INT(pulsecb, my_pulse_context_cb_t*);

static void freeCB(my_pulse_context_cb_t* head) {
    if(!head)
        return;
    freeCB(head->next);
    free(head);
}

static my_pulse_context_cb_t* checkContext(kh_pulsecb_t* list, void* context) {
    khint_t k;
    k = kh_get(pulsecb, list, (uintptr_t)context);
    if(k != kh_end(list) && kh_exist(list, k))
        return kh_value(list, k);
    return NULL;
}

// get or insert a new context container
static my_pulse_context_cb_t* getContext(kh_pulsecb_t* list, void* context) {
    khint_t k;
    int ret;
    k = kh_get(pulsecb, list, (uintptr_t)context);
    if(k != kh_end(list) && kh_exist(list, k))
        return kh_value(list, k);

    // insert
    k = kh_put(pulsecb, list, (uintptr_t)context, &ret);
    my_pulse_context_cb_t* p = kh_value(list, k) = (my_pulse_context_cb_t*)calloc(1, sizeof(my_pulse_context_cb_t));

    return p;
}
static my_pulse_cb_t* insertCB(my_pulse_context_cb_t* cblist) {
    while(cblist->size==8) {
        if(!cblist->next)
            cblist->next = (my_pulse_context_cb_t*)calloc(1, sizeof(my_pulse_context_cb_t));
        cblist = cblist->next;
    }
    return cblist->cb+(cblist->size++);
}
static void freeContext(kh_pulsecb_t* list, void* context) {
    khint_t k;
    k = kh_get(pulsecb, list, (uintptr_t)context);
    if(k == kh_end(list) || !kh_exist(list, k))
        return;
    my_pulse_context_cb_t* p = kh_value(list, k);
    freeCB(p);
    kh_del(pulsecb, list, k);
}

typedef struct my_pa_mainloop_api_s {
    void*   data;
    void*   io_new;
    void*   io_enable;
    void*   io_free;
    void*   io_set_destroy;
    void*   time_new;
    void*   time_restart;
    void*   time_free;
    void*   time_set_destroy;
    void*   defer_new;
    void*   defer_enable;
    void*   defer_free;
    void*   defer_set_destroy;
    void*   quit;
} my_pa_mainloop_api_t;

typedef void (*vFp_t)(void*);
typedef void* (*pFp_t)(void*);
typedef int (*iFp_t)(void*);
typedef void (*vFpi_t)(void*, int32_t);
typedef void* (*pFpp_t)(void*, void*);
typedef void (*vFpp_t)(void*, void*);
typedef int (*iFppp_t)(void*, void*, void*);
typedef void* (*pFipp_t)(int32_t, void*, void*);
typedef void* (*pFppp_t)(void*, void*, void*);
typedef void (*vFppp_t)(void*, void*, void*);
typedef void* (*pFpipp_t)(void*, int32_t, void*, void*);
typedef int (*iFppip_t)(void*, void*, int, void*);
typedef void* (*pFpppp_t)(void*, void*, void*, void*);
typedef void* (*pFpupp_t)(void*, uint32_t, void*, void*);
typedef void* (*pFpiipp_t)(void*, int32_t, int32_t, void*, void*);
typedef void* (*pFppppp_t)(void*, void*, void*, void*, void*);
typedef void* (*pFpippp_t)(void*, int32_t, void*, void*, void*);
typedef void* (*pFpuupp_t)(void*, uint32_t, uint32_t, void*, void*);
typedef int (*iFppupIi_t)(void*, void*, uint32_t, void*, int64_t, int32_t);

#if 0
#ifdef NOALIGN
typedef void (*vFipippV_t)(int, void*, int, void*, void*, va_list);
#else
typedef void (*vFipippV_t)(int, void*, int, void*, void*, void*);
#endif
    GO(pa_log_level_meta, vFipippV_t)           
#endif

#define SUPER() \
    GO(pa_mainloop_free, vFp_t)                 \
    GO(pa_mainloop_get_api, pFp_t)              \
    GO(pa_threaded_mainloop_free, vFp_t)        \
    GO(pa_threaded_mainloop_get_api, pFp_t)     \
    GO(pa_signal_init, iFp_t)                   \
    GO(pa_signal_new, pFipp_t)                  \
    GO(pa_signal_set_destroy, vFpp_t)           \
    GO(pa_context_new, pFpp_t)                  \
    GO(pa_context_new_with_proplist, pFppp_t)   \
    GO(pa_context_get_state, iFp_t)             \
    GO(pa_context_exit_daemon, pFppp_t)         \
    GO(pa_context_set_state_callback, vFppp_t)  \
    GO(pa_context_set_default_sink, pFpppp_t)   \
    GO(pa_context_set_default_source, pFpppp_t) \
    GO(pa_context_move_sink_input_by_index, pFpuupp_t)  \
    GO(pa_context_get_module_info_list, pFppp_t)        \
    GO(pa_context_get_server_info, pFppp_t)     \
    GO(pa_context_get_sink_input_info_list, pFppp_t)    \
    GO(pa_context_get_sink_info_list, pFppp_t)  \
    GO(pa_context_get_sink_info_by_name, pFpppp_t)      \
    GO(pa_context_get_source_info_list, pFppp_t)\
    GO(pa_context_get_source_info_by_index, pFpupp_t)   \
    GO(pa_context_get_sink_info_by_index, pFpupp_t)     \
    GO(pa_context_unload_module, pFpupp_t)      \
    GO(pa_context_load_module, pFppppp_t)       \
    GO(pa_context_connect, iFppip_t)            \
    GO(pa_context_subscribe, pFpupp_t)          \
    GO(pa_context_set_subscribe_callback, vFppp_t)      \
    GO(pa_context_drain, pFppp_t)               \
    GO(pa_context_proplist_remove, pFpppp_t)    \
    GO(pa_context_proplist_update, pFpippp_t)   \
    GO(pa_context_set_event_callback, vFppp_t)  \
    GO(pa_context_set_name, pFpppp_t)           \
    GO(pa_context_set_source_volume_by_name, pFppppp_t) \
    GO(pa_context_get_source_info_by_name, pFpppp_t)    \
    GO(pa_stream_get_state, iFp_t)              \
    GO(pa_stream_drain, pFppp_t)                \
    GO(pa_stream_flush, pFppp_t)                \
    GO(pa_stream_set_latency_update_callback, vFppp_t)  \
    GO(pa_stream_set_read_callback, vFppp_t)    \
    GO(pa_stream_set_state_callback, vFppp_t)   \
    GO(pa_stream_write, iFppupIi_t)             \
    GO(pa_stream_update_timing_info, pFppp_t)   \
    GO(pa_stream_prebuf, pFppp_t)               \
    GO(pa_stream_proplist_remove, pFpppp_t)     \
    GO(pa_stream_proplist_update, pFpippp_t)    \
    GO(pa_stream_set_buffer_attr, pFpppp_t)     \
    GO(pa_stream_set_buffer_attr_callback, vFppp_t)     \
    GO(pa_stream_set_event_callback, vFppp_t)   \
    GO(pa_stream_set_moved_callback, vFppp_t)   \
    GO(pa_stream_set_name, pFpppp_t)            \
    GO(pa_stream_set_overflow_callback, vFppp_t)\
    GO(pa_stream_set_started_callback, vFppp_t) \
    GO(pa_stream_set_suspended_callback, vFppp_t)       \
    GO(pa_stream_set_underflow_callback, vFppp_t)       \
    GO(pa_stream_set_write_callback, vFppp_t)   \
    GO(pa_stream_trigger, pFppp_t)              \
    GO(pa_stream_update_sample_rate, pFpupp_t)  \
    GO(pa_stream_cork, pFpipp_t)                \
    GO(pa_proplist_setf, iFppp_t)               


typedef struct pulse_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
    kh_pulsecb_t   *list;
    // other
} pulse_my_t;

void* getPulseMy(library_t* lib)
{
    pulse_my_t* my = (pulse_my_t*)calloc(1, sizeof(pulse_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    my->list = kh_init(pulsecb);
    return my;
}
#undef SUPER

void freePulseMy(void* lib)
{
    pulse_my_t *my = (pulse_my_t *)lib;

    my_pulse_context_cb_t* p;
    kh_foreach_value(my->list, p, 
        freeCB(p);
    );
    kh_destroy(pulsecb, my->list);
}

// TODO: change that static for a map ptr2ptr?
static my_pa_mainloop_api_t my_mainloop_api = {0};
static my_pa_mainloop_api_t my_mainloop_native = {0};
static int mainloop_inited = 0;
static my_pa_mainloop_api_t* my_mainloop_ref = NULL;
static my_pa_mainloop_api_t* my_mainloop_orig = NULL;


// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \
GO(5)   \
GO(6)   \
GO(7)

#define GO(A)   \
static uintptr_t my_free_fct_##A = 0;   \
static void my_free_##A(void* data)     \
{                                       \
    RunFunction(my_context, my_free_fct_##A, 1, data);\
}
SUPER()
#undef GO
static void* findFreeFct(void* fct)
{
    if(!fct) return NULL;
    #define GO(A) if(my_free_fct_##A == (uintptr_t)fct) return my_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_fct_##A == 0) {my_free_fct_##A = (uintptr_t)fct; return my_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio free callback\n");
    return NULL;
}

#define GO(A)   \
static uintptr_t my_free_api_fct_##A = 0;   \
static void my_free_api_##A(my_pa_mainloop_api_t* api, void* p, void* data)     \
{                                       \
    RunFunction(my_context, my_free_api_fct_##A, 3, api, p, data);\
}
SUPER()
#undef GO
static void* findFreeAPIFct(void* fct)
{
    #define GO(A) if(my_free_api_fct_##A == (uintptr_t)fct) return my_free_api_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_api_fct_##A == 0) {my_free_api_fct_##A = (uintptr_t)fct; return my_free_api_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio free api callback\n");
    return NULL;
}

#define GO(A)   \
static uintptr_t my_io_event_fct_##A = 0;   \
static void my_io_event_##A(my_pa_mainloop_api_t* api, void* e, int fd, int events, void* data)     \
{                                       \
    if(api==my_mainloop_orig) api=my_mainloop_ref;                  \
    RunFunction(my_context, my_io_event_fct_##A, 5, api, e, fd, events, data);  \
}
SUPER()
#undef GO
static void* findIOEventFct(void* fct)
{
    if(!fct) return NULL;
    #define GO(A) if(my_io_event_fct_##A == (uintptr_t)fct) return my_io_event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_io_event_fct_##A == 0) {my_io_event_fct_##A = (uintptr_t)fct; return my_io_event_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio io_event api callback\n");
    return NULL;
}

#define GO(A)   \
static uintptr_t my_time_event_fct_##A = 0;   \
static void my_time_event_##A(my_pa_mainloop_api_t* api, void* e, void* tv, void* data)     \
{                                       \
    if(api==my_mainloop_orig) api=my_mainloop_ref;                  \
    RunFunction(my_context, my_time_event_fct_##A, 4, api, e, tv, data);  \
}
SUPER()
#undef GO
static void* findTimeEventFct(void* fct)
{
    if(!fct) return NULL;
    #define GO(A) if(my_time_event_fct_##A == (uintptr_t)fct) return my_time_event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_time_event_fct_##A == 0) {my_time_event_fct_##A = (uintptr_t)fct; return my_time_event_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio time_event api callback\n");
    return NULL;
}

#define GO(A)   \
static uintptr_t my_defer_event_fct_##A = 0;   \
static void my_defer_event_##A(my_pa_mainloop_api_t* api, void* e, void* data)     \
{                                       \
    if(api==my_mainloop_orig) api=my_mainloop_ref;                  \
    RunFunction(my_context, my_defer_event_fct_##A, 3, api, e, data);  \
}
SUPER()
#undef GO
static void* findDeferEventFct(void* fct)
{
    if(!fct) return NULL;
    #define GO(A) if(my_defer_event_fct_##A == (uintptr_t)fct) return my_defer_event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_defer_event_fct_##A == 0) {my_defer_event_fct_##A = (uintptr_t)fct; return my_defer_event_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pulse audio defer_event api callback\n");
    return NULL;
}

#undef SUPER


// Mainloop functions
//  but first, all cb from the pa_mainloop_api (serioulsy, how many callback and callback inside callback there is in pulse audio?!!!)

// Native version of the mailoop_api. Called from x86 space

static void* native_io_new(void* api, int fd, int events, void* cb, void *data)
{
    if(api==my_mainloop_ref) api=my_mainloop_orig;    // need native version
    // need to wrap the callback!
    void* c = findIOEventFct(cb);
    return ((pFpiipp_t)my_mainloop_native.io_new)(api, fd, events, c, data);
}
static void native_io_enable(void* e, int events)
{
    return ((vFpi_t)my_mainloop_native.io_enable)(e, events);
}
static void native_io_free(void* e)
{
    return ((vFp_t)my_mainloop_native.io_free)(e);
}
static void native_io_set_destroy(void* e, void* cb)
{
    // need to wrap the callback!
    void* c = findFreeAPIFct(cb);
    return ((vFpp_t)my_mainloop_native.io_set_destroy)(e, c);
}

static void* native_time_new(void* api, void* tv, void* cb, void* data)
{
    if(api==my_mainloop_ref) api=my_mainloop_orig;    // need native version
    // need to wrap the callback!
    void* c = findTimeEventFct(cb);
    return ((pFpppp_t)my_mainloop_native.time_new)(api, tv, c, data);
}
static void native_time_restart(void* e, void* tv)
{
    return ((vFpp_t)my_mainloop_native.time_restart)(e, tv);
}
static void native_time_free(void* e)
{
    return ((vFp_t)my_mainloop_native.time_free)(e);
}
static void native_time_set_destroy(void* e, void* cb)
{
    // need to wrap the callback!
    void* c = findFreeAPIFct(cb);
    return ((vFpp_t)my_mainloop_native.time_set_destroy)(e, c);
}

static void* native_defer_new(void* api, void* cb, void* data)
{
    if(api==my_mainloop_ref) api=my_mainloop_orig;    // need native version
    // need to wrap the callback!
    void* c = findDeferEventFct(cb);
    return ((pFppp_t)my_mainloop_native.defer_new)(api, c, data);
}
static void native_defer_enable(void* e, int b)
{
    return ((vFpi_t)my_mainloop_native.defer_enable)(e, b);
}
static void native_defer_free(void* e)
{
    return ((vFp_t)my_mainloop_native.defer_free)(e);
}
static void native_defer_set_destroy(void* e, void* cb)
{
    // need to wrap the callback!
    void* c = findFreeAPIFct(cb);
    return ((vFpp_t)my_mainloop_native.defer_set_destroy)(e, c);
}

static void native_quit(void* api, int retval)
{
    if(api==my_mainloop_ref) api=my_mainloop_orig;    // need native version
    return ((vFpi_t)my_mainloop_native.quit)(api, retval);
}

// Emulated version. Called from native space (meh, crossing stuff are hard to follow)
static void* my_io_new(void* api, int fd, int events, void* cb, void *userdata)
{
    uintptr_t b = (uintptr_t)cb;
    //pulse_my_t* my = (pulse_my_t*)my_context->pulse->priv.w.p2;

    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->io_new);
    if(fnc) {
        if(fnc==native_io_new) fnc=my_mainloop_native.io_new;
        if(api==my_mainloop_ref) api=my_mainloop_orig;    // need native version
        return ((pFpiipp_t)fnc)(api, fd, events, cb, userdata);
    }

    bridge_t* bridge = my_context->pulse->priv.w.bridge;
    if(cb) {
        b = CheckBridged(bridge, cb);
        if(!b)
            b = AddBridge(bridge, vFppiip, cb, 0);
    }
    if(api==my_mainloop_orig) api=my_mainloop_ref;    // need emulated version
    return (void*)RunFunction(my_context, (uintptr_t)my_mainloop_ref->io_new, 5, api, fd, events, b, userdata);
}
static void my_io_enable(void* e, int events)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->io_enable);
    if(fnc==native_io_enable) fnc=my_mainloop_native.io_enable;
    if(fnc)
        return ((vFpi_t)fnc)(e, events);

    RunFunction(my_context, (uintptr_t)my_mainloop_ref->io_enable, 2, e, events);
}
static void my_io_free(void* e)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->io_free);
    if(fnc==native_io_free) fnc=my_mainloop_native.io_free;
    if(fnc)
        return ((vFp_t)fnc)(e);

    RunFunction(my_context, (uintptr_t)my_mainloop_ref->io_free, 1, e);
}
static void my_io_set_destroy(void* e, void* cb)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->io_set_destroy);
    if(fnc==native_io_set_destroy) fnc=my_mainloop_native.io_set_destroy;
    if(fnc)
        return ((vFpp_t)fnc)(e, cb);

    bridge_t* bridge = my_context->pulse->priv.w.bridge;
    uintptr_t b = 0;
    if(cb) {
        b = CheckBridged(bridge, cb);
        if(!b)
            b = AddBridge(bridge, vFppp, cb, 0);
    }
    RunFunction(my_context, (uintptr_t)my_mainloop_ref->io_set_destroy, 2, e, b);
}

static void* my_time_new(void* api, void* tv, void* cb, void* data)
{
    uintptr_t b = (uintptr_t)cb;
    //pulse_my_t* my = (pulse_my_t*)my_context->pulse->priv.w.p2;

    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->time_new);
    if(fnc) {
        if(fnc==native_time_new) fnc=my_mainloop_native.time_new;
        if(api==my_mainloop_ref) api=my_mainloop_orig;    // need native version
        return ((pFpppp_t)fnc)(api, tv, (void*)b, data);
    }

    // need to bridge the callback!
    bridge_t* bridge = my_context->pulse->priv.w.bridge;
    if(cb) {
        b = CheckBridged(bridge, cb);
        if(!b)
            b = AddBridge(bridge, vFpppp, cb, 0);
    }
    if(api==my_mainloop_orig) api=my_mainloop_ref;    // need emulated version
    return (void*)RunFunction(my_context, (uintptr_t)my_mainloop_ref->time_new, 4, api, tv, b, data);
}
static void my_time_restart(void* e, void* tv)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->time_restart);
    if(fnc==native_time_restart) fnc=my_mainloop_native.time_restart;
    if(fnc)
        return ((vFpp_t)fnc)(e, tv);

    RunFunction(my_context, (uintptr_t)my_mainloop_ref->time_restart, 2, e, tv);
}
static void my_time_free(void* e)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->time_free);
    if(fnc==native_time_free) fnc=my_mainloop_native.time_free;
    if(fnc)
        return ((vFp_t)fnc)(e);

    RunFunction(my_context, (uintptr_t)my_mainloop_ref->time_free, 1, e);
}
static void my_time_set_destroy(void* e, void* cb)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->time_set_destroy);
    if(fnc==native_time_set_destroy) fnc=my_mainloop_native.time_set_destroy;
    if(fnc)
        return ((vFpp_t)fnc)(e, cb);

    bridge_t* bridge = my_context->pulse->priv.w.bridge;
    uintptr_t b = 0;
    if(cb) {
        b = CheckBridged(bridge, cb);
        if(!b)
            b = AddBridge(bridge, vFppp, cb, 0);
    }
    RunFunction(my_context, (uintptr_t)my_mainloop_ref->time_set_destroy, 2, e, b);
}

static void* my_defer_new(void* api, void* cb, void* data)
{
    uintptr_t b = (uintptr_t)cb;
    //pulse_my_t* my = (pulse_my_t*)my_context->pulse->priv.w.p2;

    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->defer_new);
    if(fnc) {
        if(api==my_mainloop_ref) api=my_mainloop_orig;    // need native version
        if(fnc==native_defer_new) fnc=my_mainloop_native.defer_new;
        return ((pFppp_t)fnc)(api, cb, data);
    }

    // need to bridge the callback!
    bridge_t* bridge = my_context->pulse->priv.w.bridge;
    if(cb) {
        b = CheckBridged(bridge, cb);
        if(!b)
            b = AddBridge(bridge, vFppp, cb, 0);
    }
    if(api==my_mainloop_orig) api=my_mainloop_ref;    // need emulated version
    return (void*)RunFunction(my_context, (uintptr_t)my_mainloop_ref->defer_new, 3, api, b, data);
}
static void my_defer_enable(void* e, int b)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->defer_enable);
    if(fnc==native_defer_enable) fnc=my_mainloop_native.defer_enable;
    if(fnc)
        return ((vFpi_t)fnc)(e, b);

    RunFunction(my_context, (uintptr_t)my_mainloop_ref->defer_enable, 2, e, b);
}
static void my_defer_free(void* e)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->defer_free);
    if(fnc==native_defer_free) fnc=my_mainloop_native.defer_free;
    if(fnc)
        return ((vFp_t)fnc)(e);

    RunFunction(my_context, (uintptr_t)my_mainloop_ref->defer_free, 1, e);
}
static void my_defer_set_destroy(void* e, void* cb)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->defer_set_destroy);
    if(fnc==native_defer_set_destroy) fnc=my_mainloop_native.defer_set_destroy;
    if(fnc)
        return ((vFpp_t)fnc)(e, cb);

    bridge_t* bridge = my_context->pulse->priv.w.bridge;
    uintptr_t b = 0;
    if(cb) {
        b = CheckBridged(bridge, cb);
        if(!b)
            b = AddBridge(bridge, vFppp, cb, 0);
    }
    RunFunction(my_context, (uintptr_t)my_mainloop_ref->defer_set_destroy, 2, e, b);
}

static void my_quit(void* api, int retval)
{
    void* fnc = GetNativeFnc((uintptr_t)my_mainloop_ref->quit);
    if(fnc) {
        if(fnc==native_quit) fnc=my_mainloop_native.quit;
        if(api==my_mainloop_ref) api=my_mainloop_orig;    // need native version
        return ((vFpi_t)fnc)(api, retval);
    }

    if(api==my_mainloop_orig) api=my_mainloop_ref;    // need emulated version
    RunFunction(my_context, (uintptr_t)my_mainloop_ref->quit, 2, api, retval);
}

static void bridgeMainloopAPI(bridge_t* bridge, my_pa_mainloop_api_t* api)
{
    if(!api) {
        return;
    }
    #define GO(A, W) my_mainloop_native.A = api->A; if(api->A) {my_mainloop_api.A = (void*)AddCheckBridge(bridge, W, native_##A, 0); api->A=my_##A;} else my_mainloop_api.A = NULL
    GO(io_new, pFpiipp);
    GO(io_enable, vFpi);
    GO(io_free, vFp);
    GO(io_set_destroy, vFpp);
    GO(time_new, pFpppp);
    GO(time_restart, vFpp);
    GO(time_free, vFp);
    GO(time_set_destroy, vFpp);
    GO(defer_new, pFppp);
    GO(defer_enable, vFpi);
    GO(defer_free, vFp);
    GO(defer_set_destroy, vFpp);
    GO(quit, vFpi);
    #undef GO
    my_mainloop_api.data = api->data;
    my_mainloop_orig = api;
    my_mainloop_ref = &my_mainloop_api;
    return;
}
/*static my_pa_mainloop_api_t* backMainloopAPI(my_pa_mainloop_api_t* mainloop)
{
    if(my_mainloop_ref!=mainloop) {
        printf_log(LOG_NONE, "Warning, Pulse mainloop_api is not expected value\n");
        return mainloop;
    }
    return my_mainloop_orig;
}*/

// mainloop_api: all the functions are wrapped, with custom function used...
// and a copy is sent to the emulated software. copy use wrapped function
// only one mainloop can be active at a given time!
EXPORT void my_pa_mainloop_free(x86emu_t* emu, void* mainloop)
{
    library_t* lib = GetLib(emu->context->maplib, pulseName);
    pulse_my_t* my = lib->priv.w.p2;
    my->pa_mainloop_free(mainloop);
    mainloop_inited = 0;
    my_mainloop_ref = my_mainloop_orig = NULL;
}
EXPORT void* my_pa_mainloop_get_api(x86emu_t* emu, void* mainloop)
{
    library_t* lib = GetLib(emu->context->maplib, pulseName);
    pulse_my_t* my = lib->priv.w.p2;
    my_pa_mainloop_api_t* api = my->pa_mainloop_get_api(mainloop);
    bridgeMainloopAPI(lib->priv.w.bridge, api);
    return my_mainloop_ref;
}

EXPORT void my_pa_threaded_mainloop_free(x86emu_t* emu, void* mainloop)
{
    library_t* lib = GetLib(emu->context->maplib, pulseName);
    pulse_my_t* my = lib->priv.w.p2;
    my->pa_threaded_mainloop_free(mainloop);
    mainloop_inited = 0;
    my_mainloop_ref = my_mainloop_orig = NULL;
}
EXPORT void* my_pa_threaded_mainloop_get_api(x86emu_t* emu, void* mainloop)
{
    library_t* lib = GetLib(emu->context->maplib, pulseName);
    pulse_my_t* my = lib->priv.w.p2;
    my_pa_mainloop_api_t* api = my->pa_threaded_mainloop_get_api(mainloop);
    bridgeMainloopAPI(lib->priv.w.bridge, api);
    return my_mainloop_ref;
}

// Context functions
EXPORT void* my_pa_context_new(x86emu_t* emu, my_pa_mainloop_api_t* mainloop, void* name)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    if(mainloop==my_mainloop_ref) mainloop=my_mainloop_orig;    // need native version
    return my->pa_context_new(mainloop, name);
}

EXPORT void* my_pa_context_new_with_proplist(x86emu_t* emu, my_pa_mainloop_api_t* mainloop, void* name, void* proplist)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    if(mainloop==my_mainloop_ref) mainloop=my_mainloop_orig;    // need native version
    return my->pa_context_new_with_proplist(mainloop, name, proplist);
}

EXPORT int my_pa_signal_init(x86emu_t* emu, my_pa_mainloop_api_t* mainloop)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    if(mainloop==my_mainloop_ref) mainloop=my_mainloop_orig;    // need native version
    return my->pa_signal_init(mainloop);
}

static void my_signal_cb(my_pa_mainloop_api_t* api, void* e, int sig, void *data)
{
    my_pulse_cb_t* cb = (my_pulse_cb_t*)data;
    if(api==my_mainloop_orig) api=my_mainloop_ref;    // need emulated version
    RunFunction(my_context, cb->fnc, 4, api, e, sig, cb->data);
}

static uintptr_t my_signal_destroy_fct = 0;
static void my_signal_destroy_cb(my_pa_mainloop_api_t* api, void* e, void *data)
{
    my_pulse_cb_t* cb = (my_pulse_cb_t*)data;
    if(api==my_mainloop_orig) api=my_mainloop_ref;    // need emulated version
    RunFunction(my_context, my_signal_destroy_fct, 3, api, e, cb->data);
}

EXPORT void* my_pa_signal_new(x86emu_t* emu, int sig, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    if(cb) {
        c = insertCB(getContext(my->list, (void*)sig));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_signal_new(sig, cb?my_signal_cb:NULL, c);
}

EXPORT void my_pa_signal_set_destroy(x86emu_t* emu, void* e, void* cb)
{
    // TODO: there is only one slot here. Check if more are needed
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_signal_destroy_fct = (uintptr_t)cb;
    return my->pa_signal_set_destroy(e, cb?my_signal_destroy_cb:NULL);
}


static uintptr_t my_prefork_fct = 0;
static void my_prefork()
{
    RunFunction(my_context, my_prefork_fct, 0);
}
static uintptr_t my_postfork_fct = 0;
static void my_postfork()
{
    RunFunction(my_context, my_postfork_fct, 0);
}
static uintptr_t my_atfork_fct = 0;
static void my_atfork()
{
    RunFunction(my_context, my_atfork_fct, 0);
}

typedef struct my_pa_spawn_api_s {
    void* prefork;
    void* postfork;
    void* atfork;
} my_pa_spawn_api_t;

EXPORT int my_pa_context_connect(x86emu_t* emu, void* context, void* server, int flags, my_pa_spawn_api_t* api)
{
    pulse_my_t* my = (pulse_my_t*)GetLib(my_context->maplib, pulseName)->priv.w.p2;
    if(!api) {
        return my->pa_context_connect(context, server, flags, api);
    }
    static my_pa_spawn_api_t a = {0};
    #define GO(A) if(api->A) {a.A = my_##A; my_##A##_fct = (uintptr_t)api->A;} else a.A = 0
    GO(prefork);
    GO(postfork);
    GO(atfork);
    #undef GO
    return my->pa_context_connect(context, server, flags, &a);
}



static void my_state_context(void* context, void* data)
{
    if(data) {  // need to check data, as it can be NULL if no state callback are yet defined
        my_pulse_cb_t* cb = (my_pulse_cb_t*)data;
        RunFunction(my_context, cb->fnc, 2, context, cb->data);
    }
    pulse_my_t* my = (pulse_my_t*)GetLib(my_context->maplib, pulseName)->priv.w.p2;
    int i = my->pa_context_get_state(context);
    if (i==6) {   // PA_CONTEXT_TERMINATED
        // clean the stream callbacks
        freeContext(my->list, context);
    }
}
static my_pulse_context_cb_t* my_check_context(kh_pulsecb_t* list, void* context)
{
    // check if that context exist, if not create it and set state callback
    my_pulse_context_cb_t *ret = checkContext(list, context);
    if (!ret) {
        ret = getContext(list, context);
        pulse_my_t* my = (pulse_my_t*)GetLib(my_context->maplib, pulseName)->priv.w.p2;
        my->pa_context_set_state_callback(context, my_state_context, NULL);
    }
    return ret;
}


static void my_notify_context(void* context, void* data)
{
    my_pulse_cb_t* cb = (my_pulse_cb_t*)data;
    RunFunction(my_context, cb->fnc, 2, context, cb->data);
}

static void my_success_context(void* context, int success, void* data)
{
    my_pulse_cb_t* cb = (my_pulse_cb_t*)data;
    RunFunction(my_context, cb->fnc, 3, context, success, cb->data);
}

static void my_event_context(void* context, void* name, void* p, void* data)
{
    my_pulse_cb_t* cb = (my_pulse_cb_t*)data;
    RunFunction(my_context, cb->fnc, 4, context, name, p, cb->data);
}

static void my_module_info(void* context, void* i, int eol, void* data)
{
    my_pulse_cb_t* cb = (my_pulse_cb_t*)data;
    RunFunction(my_context, cb->fnc, 4, context, i, eol, cb->data);
}

static void my_server_info(void* context, void* i, void* data)
{
    my_pulse_cb_t* cb = (my_pulse_cb_t*)data;
    RunFunction(my_context, cb->fnc, 3, context, i, cb->data);
}

static void my_context_index(void* context, uint32_t idx, void* data)
{
    my_pulse_cb_t* cb = (my_pulse_cb_t*)data;
    RunFunction(my_context, cb->fnc, 3, context, idx, cb->data);
}

static void my_subscribe_context(void* context, int t, uint32_t idx, void* data)
{
    my_pulse_cb_t* cb = (my_pulse_cb_t*)data;
    RunFunction(my_context, cb->fnc, 4, context, t, idx, cb->data);
}

EXPORT void my_pa_context_set_state_callback(x86emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_context_set_state_callback(context, my_state_context, c);
}

EXPORT void my_pa_context_set_default_sink(x86emu_t* emu, void* context, void* name, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_context_set_default_sink(context, name, cb?my_success_context:NULL, c);
}

EXPORT void my_pa_context_set_default_source(x86emu_t* emu, void* context, void* name, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_context_set_default_source(context, name, cb?my_success_context:NULL, c);
}

EXPORT void* my_pa_context_move_sink_input_by_index(x86emu_t* emu, void* context, uint32_t idx, uint32_t sink_idx, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_move_sink_input_by_index(context, idx, sink_idx, cb?my_success_context:NULL, c);
}

EXPORT void* my_pa_context_get_module_info_list(x86emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_get_module_info_list(context, cb?my_module_info:NULL, c);
}

EXPORT void* my_pa_context_get_server_info(x86emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_get_server_info(context, cb?my_server_info:NULL, c);
}

EXPORT void* my_pa_context_get_sink_input_info_list(x86emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_get_sink_input_info_list(context, cb?my_module_info:NULL, c);
}

EXPORT void* my_pa_context_get_sink_info_list(x86emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_get_sink_info_list(context, cb?my_module_info:NULL, c);
}

EXPORT void* my_pa_context_get_sink_info_by_name(x86emu_t* emu, void* context, void* name, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_get_sink_info_by_name(context, name, cb?my_module_info:NULL, c);
}

EXPORT void* my_pa_context_get_source_info_list(x86emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_get_source_info_list(context, cb?my_module_info:NULL, c);
}


EXPORT void* my_pa_context_get_sink_info_by_index(x86emu_t* emu, void* context, uint32_t idx, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_get_sink_info_by_index(context, idx, cb?my_module_info:NULL, c);
}

EXPORT void* my_pa_context_get_source_info_by_index(x86emu_t* emu, void* context, uint32_t idx, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_get_source_info_by_index(context, idx, cb?my_module_info:NULL, c);
}

EXPORT void* my_pa_context_unload_module(x86emu_t* emu, void* context, uint32_t idx, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_unload_module(context, idx, cb?my_success_context:NULL, c);
}

EXPORT void* my_pa_context_load_module(x86emu_t* emu, void* context, void* name, void* arg, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_load_module(context, name, arg, cb?my_context_index:NULL, c);
}

EXPORT void* my_pa_context_subscribe(x86emu_t* emu, void* context, uint32_t m, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_subscribe(context, m, cb?my_success_context:NULL, c);
}

EXPORT void my_pa_context_set_subscribe_callback(x86emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_set_subscribe_callback(context, cb?my_subscribe_context:NULL, c);
}

EXPORT void* my_pa_context_drain(x86emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_drain(context, cb?my_notify_context:NULL, c);
}

EXPORT void* my_pa_context_exit_daemon(x86emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_exit_daemon(context, cb?my_success_context:NULL, c);
}

EXPORT void* my_pa_context_proplist_remove(x86emu_t* emu, void* context, void* keys, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_proplist_remove(context, keys, cb?my_success_context:NULL, c);
}

EXPORT void* my_pa_context_proplist_update(x86emu_t* emu, void* context, int mode, void* p, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_proplist_update(context, mode, p, cb?my_success_context:NULL, c);
}

EXPORT void my_pa_context_set_event_callback(x86emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_context_set_event_callback(context, cb?my_event_context:NULL, c);
}

EXPORT void* my_pa_context_set_name(x86emu_t* emu, void* context, void* name, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_set_name(context, name, cb?my_success_context:NULL, c);
}

EXPORT void* my_pa_context_set_source_volume_by_name(x86emu_t* emu, void* context, void* name,void* volume, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_set_source_volume_by_name(context, name, volume, cb?my_success_context:NULL, c);
}

EXPORT void* my_pa_context_get_source_info_by_name(x86emu_t* emu, void* context, void* name, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_get_source_info_by_name(context, name, cb?my_module_info:NULL, c);
}

// Stream functions
static void my_stream_state(void* s, void* data)
{
    if(data) {  // need to check data, as it can be NULL if no state callback are yet defined
        my_pulse_cb_t* cb = (my_pulse_cb_t*)data;
        RunFunction(my_context, cb->fnc, 2, s, cb->data);
    }
    pulse_my_t* my = (pulse_my_t*)GetLib(my_context->maplib, pulseName)->priv.w.p2;
    int i = my->pa_stream_get_state(s);
    if (i==4) {   // PA_TERMINATED
        // clean the stream callbacks
        freeContext(my->list, s);
    }
}


static my_pulse_context_cb_t* my_check_stream(kh_pulsecb_t* list, void* stream)
{
    // check if that stream exist, if not create it and set state callback
    my_pulse_context_cb_t *ret = checkContext(list, stream);
    if (!ret) {
        ret = getContext(list, stream);
        pulse_my_t* my = (pulse_my_t*)GetLib(my_context->maplib, pulseName)->priv.w.p2;
        my->pa_stream_set_state_callback(stream, my_stream_state, NULL);
    }
    return ret;
}


static void my_stream_success(void* s, int success, void* data)
{
    my_pulse_cb_t* cb = (my_pulse_cb_t*)data;
    RunFunction(my_context, cb->fnc, 3, s, success, cb->data);
}
EXPORT void* my_pa_stream_drain(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_stream_drain(stream, cb?my_stream_success:NULL, c);
}

EXPORT void* my_pa_stream_flush(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_stream_flush(stream, cb?my_stream_success:NULL, c);
}

static void my_stream_notify(void* s, void* data)
{
    my_pulse_cb_t* cb = (my_pulse_cb_t*)data;
    RunFunction(my_context, cb->fnc, 2, s, cb->data);
}

static void my_stream_event(void* s, void* name, void* pl, void* data)
{
    my_pulse_cb_t* cb = (my_pulse_cb_t*)data;
    RunFunction(my_context, cb->fnc, 4, s, name, pl, cb->data);
}

EXPORT void my_pa_stream_set_latency_update_callback(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_stream_set_latency_update_callback(stream, cb?my_stream_notify:NULL, c);
}

static void my_stream_request(void* s, int nbytes, void* data)
{
    my_pulse_cb_t* cb = (my_pulse_cb_t*)data;
    RunFunction(my_context, cb->fnc, 3, s, nbytes, cb->data);
}
EXPORT void my_pa_stream_set_read_callback(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_stream_set_read_callback(stream, cb?my_stream_request:NULL, c);
}

EXPORT int my_pa_stream_write(x86emu_t* emu, void* stream, void* d, uint32_t nbytes, void* cb, int64_t offset, int seek)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_check_stream(my->list, stream);
    void* nat = NULL;
    if(cb) {
        nat = findFreeFct(cb);
    }
    return my->pa_stream_write(stream, d, nbytes, nat, offset, seek);
}

EXPORT void* my_pa_stream_update_timing_info(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_stream_update_timing_info(stream, cb?my_stream_success:NULL, c);
}

EXPORT void* my_pa_stream_prebuf(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_stream_prebuf(stream, cb?my_stream_success:NULL, c);
}

EXPORT void* my_pa_stream_proplist_remove(x86emu_t* emu, void* stream, void* keys, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_stream_proplist_remove(stream, keys, cb?my_stream_success:NULL, c);
}

EXPORT void* my_pa_stream_proplist_update(x86emu_t* emu, void* stream, int32_t mode, void* p, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_stream_proplist_update(stream, mode, p, cb?my_stream_success:NULL, c);
}

EXPORT void* my_pa_stream_set_buffer_attr(x86emu_t* emu, void* stream, void* attr, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_stream_set_buffer_attr(stream, attr, cb?my_stream_success:NULL, c);
}

EXPORT void my_pa_stream_set_buffer_attr_callback(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_stream_set_buffer_attr_callback(stream, cb?my_stream_notify:NULL, c);
}

EXPORT void my_pa_stream_set_event_callback(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_stream_set_event_callback(stream, cb?my_stream_event:NULL, c);
}

EXPORT void my_pa_stream_set_moved_callback(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_stream_set_moved_callback(stream, cb?my_stream_notify:NULL, c);
}

EXPORT void* my_pa_stream_set_name(x86emu_t* emu, void* stream, void* name, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_stream_set_name(stream, name, cb?my_stream_success:NULL, c);
}

EXPORT void my_pa_stream_set_overflow_callback(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_stream_set_overflow_callback(stream, cb?my_stream_notify:NULL, c);
}

EXPORT void my_pa_stream_set_started_callback(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_stream_set_started_callback(stream, cb?my_stream_notify:NULL, c);
}

EXPORT void my_pa_stream_set_state_callback(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_stream_set_state_callback(stream, my_stream_state, c);
}

EXPORT void my_pa_stream_set_suspended_callback(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_stream_set_suspended_callback(stream, cb?my_stream_notify:NULL, c);
}

EXPORT void my_pa_stream_set_underflow_callback(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_stream_set_underflow_callback(stream, cb?my_stream_notify:NULL, c);
}

EXPORT void my_pa_stream_set_write_callback(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_stream_set_write_callback(stream, cb?my_stream_request:NULL, c);
}

EXPORT void* my_pa_stream_trigger(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_stream_trigger(stream, cb?my_stream_success:NULL, c);
}

EXPORT void* my_pa_stream_update_sample_rate(x86emu_t* emu, void* stream, uint32_t rate, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_stream_update_sample_rate(stream, rate, cb?my_stream_success:NULL, c);
}

EXPORT void* my_pa_stream_cork(x86emu_t* emu, void* stream, int32_t b, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_stream_cork(stream, b, cb?my_stream_success:NULL, c);
}

EXPORT int my_pa_proplist_setf(x86emu_t* emu, void* p, void* key, void* fmt, void* b)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    iFppp_t f = (iFppp_t)vasprintf;
    char* format;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    f(&format, fmt, emu->scratch);
    #else
    f(&format, fmt, b);
    #endif
    int ret = my->pa_proplist_setf(p, key, format);
    free(format);
    return ret;
}


#if 0
EXPORT void my_pa_log_level_meta(x86emu_t* emu, int level, void* file, int line, void* func, void* format, void* b, va_list V)
{
    pulse_my_t* my = (pulse_my_t*)emu->context->pulse->priv.w.p2;
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)format, b, emu->scratch);
    return my->pa_log_level_meta(level, file, line, func, format, emu->scratch);
    #else
    return my->pa_log_level_meta(level, file, line, func, format, V);
    #endif
}
#endif

#define CUSTOM_INIT \
    lib->priv.w.p2 = getPulseMy(lib);   \
    box86->pulse = lib;

#define CUSTOM_FINI \
    lib->context->pulse = NULL;     \
    freePulseMy(lib->priv.w.p2);    \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

