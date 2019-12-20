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
    int ret;
    k = kh_get(pulsecb, list, (uintptr_t)context);
    if(k == kh_end(list) || !kh_exist(list, k))
        return;
    my_pulse_context_cb_t* p = kh_value(list, k);
    freeCB(p);
    kh_del(pulsecb, list, (uintptr_t)context);
}

typedef int (*iFp_t)(void*);
typedef void* (*pFpp_t)(void*, void*);
typedef void* (*pFppp_t)(void*, void*, void*);
typedef void (*vFppp_t)(void*, void*, void*);
typedef int (*iFppip_t)(void*, void*, int, void*);
typedef void* (*pFpppp_t)(void*, void*, void*, void*);
typedef void* (*pFpupp_t)(void*, uint32_t, void*, void*);
typedef void* (*pFppppp_t)(void*, void*, void*, void*, void*);
typedef void* (*pFpuupp_t)(void*, uint32_t, uint32_t, void*, void*);

#if 0
#ifdef NOALIGN
typedef void (*vFipippV_t)(int, void*, int, void*, void*, va_list);
#else
typedef void (*vFipippV_t)(int, void*, int, void*, void*, void*);
#endif
    GO(pa_log_level_meta, vFipippV_t)           
#endif

#define SUPER() \
    GO(pa_context_new, pFpp_t)                  \
    GO(pa_stream_get_state, iFp_t)              \
    GO(pa_context_get_state, iFp_t)             \
    GO(pa_context_set_state_callback, vFppp_t)  \
    GO(pa_context_set_default_sink, pFpppp_t)   \
    GO(pa_context_move_sink_input_by_index, pFpuupp_t)  \
    GO(pa_context_get_module_info_list, pFppp_t)        \
    GO(pa_context_get_server_info, pFppp_t)     \
    GO(pa_context_get_sink_input_info_list, pFppp_t)    \
    GO(pa_context_get_sink_info_list, pFppp_t)  \
    GO(pa_context_unload_module, pFpupp_t)      \
    GO(pa_context_load_module, pFppppp_t)       \
    GO(pa_context_connect, iFppip_t)            \
    GO(pa_stream_drain, pFppp_t)                \
    GO(pa_stream_flush, pFppp_t)                \
    GO(pa_stream_set_latency_update_callback, vFppp_t)  \
    GO(pa_stream_set_read_callback, vFppp_t)    \
    GO(pa_stream_set_state_callback, vFppp_t)

typedef struct pulse_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
    kh_pulsecb_t   *list;
    // other
} pulse_my_t;

static box86context_t *my_context = NULL;

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

// Context functions
//  but first, all cb from the pa_mainloop_api (serioulsy, how many callback and callback inside callback there is in pulse audio?!!!)
static uintptr_t my_io_new_fct = 0;
static void* my_io_new(void* a, int fd, int events, void* cb, void *userdata)
{
    bridge_t* bridge = GetLib(my_context->maplib, pulseName)->priv.w.bridge;
    uintptr_t b = 0;
    if(cb) {
        b = CheckBridged(bridge, cb);
        if(!b)
            b = AddBridge(bridge, vFppiip, cb, 0);
    }
    return (void*)RunFunction(my_context, my_io_new_fct, 5, a, fd, events, b, userdata);
}
static uintptr_t my_io_enable_fct = 0;
static void my_io_enable(void* e, int events)
{
    RunFunction(my_context, my_io_enable_fct, 2, e, events);
}
static uintptr_t my_io_free_fct = 0;
static void my_io_free(void* e)
{
    RunFunction(my_context, my_io_free_fct, 1, e);
}
static uintptr_t my_io_set_destroy_fct = 0;
static void my_io_set_destroy(void* e, void* cb)
{
    bridge_t* bridge = GetLib(my_context->maplib, pulseName)->priv.w.bridge;
    uintptr_t b = 0;
    if(cb) {
        b = CheckBridged(bridge, cb);
        if(!b)
            b = AddBridge(bridge, vFppp, cb, 0);
    }
    RunFunction(my_context, my_io_set_destroy_fct, 2, e, b);
}
static uintptr_t my_time_new_fct = 0;
static void* my_time_new(void* a, void* tv, void* cb, void* data)
{
    bridge_t* bridge = GetLib(my_context->maplib, pulseName)->priv.w.bridge;
    uintptr_t b = 0;
    if(cb) {
        b = CheckBridged(bridge, cb);
        if(!b)
            b = AddBridge(bridge, vFpppp, cb, 0);
    }
    return (void*)RunFunction(my_context, my_time_new_fct, 4, a, tv, b, data);
}
static uintptr_t my_time_restart_fct = 0;
static void my_time_restart(void* e, int tv)
{
    RunFunction(my_context, my_time_restart_fct, 2, e, tv);
}
static uintptr_t my_time_free_fct = 0;
static void my_time_free(void* e)
{
    RunFunction(my_context, my_time_free_fct, 1, e);
}
static uintptr_t my_time_set_destroy_fct = 0;
static void my_time_set_destroy(void* e, void* cb)
{
    bridge_t* bridge = GetLib(my_context->maplib, pulseName)->priv.w.bridge;
    uintptr_t b = 0;
    if(cb) {
        b = CheckBridged(bridge, cb);
        if(!b)
            b = AddBridge(bridge, vFppp, cb, 0);
    }
    RunFunction(my_context, my_time_set_destroy_fct, 2, e, b);
}
static uintptr_t my_defer_new_fct = 0;
static void* my_defer_new(void* a, void* cb, void* data)
{
    bridge_t* bridge = GetLib(my_context->maplib, pulseName)->priv.w.bridge;
    uintptr_t b = 0;
    if(cb) {
        b = CheckBridged(bridge, cb);
        if(!b)
            b = AddBridge(bridge, vFppp, cb, 0);
    }
    return (void*)RunFunction(my_context, my_defer_new_fct, 3, a, b, data);
}
static uintptr_t my_defer_enable_fct = 0;
static void my_defer_enable(void* e, int b)
{
    RunFunction(my_context, my_defer_enable_fct, 2, e, b);
}
static uintptr_t my_defer_free_fct = 0;
static void my_defer_free(void* e)
{
    RunFunction(my_context, my_defer_free_fct, 1, e);
}
static uintptr_t my_defer_set_destroy_fct = 0;
static void my_defer_set_destroy(void* e, void* cb)
{
    bridge_t* bridge = GetLib(my_context->maplib, pulseName)->priv.w.bridge;
    uintptr_t b = 0;
    if(cb) {
        b = CheckBridged(bridge, cb);
        if(!b)
            b = AddBridge(bridge, vFppp, cb, 0);
    }
    RunFunction(my_context, my_defer_set_destroy_fct, 2, e, b);
}
static uintptr_t my_quit_fct = 0;
static void my_quit(void* e, int retval)
{
    RunFunction(my_context, my_quit_fct, 2, e, retval);
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
    void*   defer_set_destroy;
    void*   quit;
} my_pa_mainloop_api_t;

EXPORT void* my_pa_context_new(x86emu_t* emu, my_pa_mainloop_api_t* mainloop, void* name)
{
    pulse_my_t* my = (pulse_my_t*)GetLib(emu->context->maplib, pulseName)->priv.w.p2;
    if(!mainloop) {
        return my->pa_context_new(mainloop, name);
    }
    static my_pa_mainloop_api_t m = {0};
    #define GO(A) if(mainloop->A) {m.A = my_##A; my_##A##_fct = (uintptr_t)mainloop->A;} else m.A = 0
    GO(io_new);
    GO(io_enable);
    GO(io_free);
    GO(io_set_destroy);
    GO(time_new);
    GO(time_restart);
    GO(time_free);
    GO(time_set_destroy);
    GO(defer_new);
    GO(defer_enable);
    GO(defer_set_destroy);
    GO(quit);
    #undef GO
    m.data = mainloop->data;
    return my->pa_context_new(&m, name);
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

EXPORT void my_pa_context_set_state_callback(x86emu_t* emu, void* context, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)GetLib(emu->context->maplib, pulseName)->priv.w.p2;
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
    pulse_my_t* my = (pulse_my_t*)GetLib(emu->context->maplib, pulseName)->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_context_set_default_sink(context, name, cb?my_success_context:NULL, c);
}

EXPORT void* my_pa_context_move_sink_input_by_index(x86emu_t* emu, void* context, uint32_t idx, uint32_t sink_idx, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)GetLib(emu->context->maplib, pulseName)->priv.w.p2;
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
    pulse_my_t* my = (pulse_my_t*)GetLib(emu->context->maplib, pulseName)->priv.w.p2;
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
    pulse_my_t* my = (pulse_my_t*)GetLib(emu->context->maplib, pulseName)->priv.w.p2;
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
    pulse_my_t* my = (pulse_my_t*)GetLib(emu->context->maplib, pulseName)->priv.w.p2;
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
    pulse_my_t* my = (pulse_my_t*)GetLib(emu->context->maplib, pulseName)->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_get_sink_info_list(context, cb?my_module_info:NULL, c);
}

EXPORT void* my_pa_context_unload_module(x86emu_t* emu, void* context, uint32_t idx, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)GetLib(emu->context->maplib, pulseName)->priv.w.p2;
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
    pulse_my_t* my = (pulse_my_t*)GetLib(emu->context->maplib, pulseName)->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_context(my->list, context);
    if(cb) {
        c = insertCB(getContext(my->list, context));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    return my->pa_context_load_module(context, name, arg, cb?my_context_index:NULL, c);
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
    pulse_my_t* my = (pulse_my_t*)GetLib(emu->context->maplib, pulseName)->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_stream_drain(stream, cb?my_stream_success:NULL, c);
}

EXPORT void* my_pa_stream_flush(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)GetLib(emu->context->maplib, pulseName)->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_stream_flush(stream, cb?my_stream_success:NULL, c);
}

static void my_stream_notify(void* s, void* data)
{
    my_pulse_cb_t* cb = (my_pulse_cb_t*)data;
    RunFunction(my_context, cb->fnc, 2, s, cb->data);
}
EXPORT void my_pa_stream_set_latency_update_callback(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)GetLib(emu->context->maplib, pulseName)->priv.w.p2;
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
    pulse_my_t* my = (pulse_my_t*)GetLib(emu->context->maplib, pulseName)->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_stream_set_read_callback(stream, cb?my_stream_request:NULL, c);
}

EXPORT void my_pa_stream_set_state_callback(x86emu_t* emu, void* stream, void* cb, void* data)
{
    pulse_my_t* my = (pulse_my_t*)GetLib(emu->context->maplib, pulseName)->priv.w.p2;
    my_pulse_cb_t* c = NULL;
    my_check_stream(my->list, stream);
    if(cb) {
        c = insertCB(getContext(my->list, stream));
        c->fnc = (uintptr_t)cb;
        c->data = data;
    }
    my->pa_stream_set_state_callback(stream, my_stream_state, c);
}

#if 0
EXPORT void my_pa_log_level_meta(x86emu_t* emu, int level, void* file, int line, void* func, void* format, void* b, va_list V)
{
    pulse_my_t* my = (pulse_my_t*)GetLib(emu->context->maplib, pulseName)->priv.w.p2;
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
    my_context = box86; \
    lib->priv.w.p2 = getPulseMy(lib);

#define CUSTOM_FINI \
    freePulseMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

