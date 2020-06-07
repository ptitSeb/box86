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
#include "librarian.h"
#include "box86context.h"
#include "emu/x86emu_private.h"
#include "callback.h"

const char* dbusName = "libdbus-1.so.3";
#define LIBNAME dbus


typedef int32_t(* DBusHandleMessageFunction) (void* connection, void* message, void* user_data);
typedef void (* DBusFreeFunction) (void *memory);
typedef int32_t(* DBusAddTimeoutFunction) (void* timeout, void *data);
typedef void(* DBusRemoveTimeoutFunction) (void *timeout, void *data);
typedef void(* DBusTimeoutToggledFunction) (void *timeout, void *data);


typedef void (*vFppp_t)(void*, void*, void*);
typedef void (*vFpppp_t)(void*, void*, void*, void*);
typedef int32_t (*iFpppp_t)(void*, void*, void*, void*);
typedef int32_t (*iFppip_t)(void*, void*, int32_t, void*);
typedef int32_t (*iFpipp_t)(void*, int32_t, void*, void*);
typedef int32_t (*iFppppp_t)(void*, void*, void*, void*, void*);
typedef int32_t (*iFpppppp_t)(void*, void*, void*, void*, void*, void*);

#define SUPER() \
    GO(dbus_timeout_set_data, vFppp_t)  \
    GO(dbus_connection_set_timeout_functions, iFpppppp_t)   \
    GO(dbus_connection_add_filter, iFpppp_t)    \
    GO(dbus_connection_remove_filter, vFppp_t)  \
    GO(dbus_message_get_args_valist, iFppip_t)  \
    GO(dbus_message_set_data, iFpipp_t)         \
    GO(dbus_pending_call_set_notify, iFpppp_t)  \
    GO(dbus_pending_call_set_data, iFpipp_t)    \
    GO(dbus_watch_set_data, vFppp_t)            \
    GO(dbus_connection_set_dispatch_status_function, vFpppp_t)  \
    GO(dbus_connection_set_watch_functions, iFpppppp_t)         \
    GO(dbus_connection_try_register_object_path, iFppppp_t)

typedef struct dbus_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} dbus_my_t;

static void* getDBusMy(library_t* lib)
{
    dbus_my_t* my = (dbus_my_t*)calloc(1, sizeof(dbus_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

static void freeDBusMy(void* lib)
{
    //dbus_my_t *my = (dbus_my_t *)lib;
}

x86emu_t* dbus_timeout_free_emu = NULL;
static void my_dbus_timout_free_cb(void* memory)
{
    if(dbus_timeout_free_emu) {
        SetCallbackArg(dbus_timeout_free_emu, 0, memory);
        RunCallback(dbus_timeout_free_emu);
    }
}
EXPORT void my_dbus_timeout_set_data(x86emu_t* emu, void* e, void* p, void* f)
{
    library_t * lib = GetLib(emu->context->maplib, dbusName);
    dbus_my_t *my = (dbus_my_t*)lib->priv.w.p2;
    if(dbus_timeout_free_emu) FreeCallback(dbus_timeout_free_emu);
    dbus_timeout_free_emu = f?AddSmallCallback(emu, (uintptr_t)f, 1, NULL, NULL, NULL, NULL):NULL;
    my->dbus_timeout_set_data(e, p, f?my_dbus_timout_free_cb:NULL);
}

x86emu_t *dbus_connection_timout_add_emu = NULL;
x86emu_t *dbus_connection_timout_remove_emu = NULL;
x86emu_t *dbus_connection_timout_toggle_emu = NULL;
x86emu_t *dbus_connection_timout_free_emu = NULL;
int32_t my_dbus_connection_timout_add_cb(void* timeout, void *data)
{
    if(dbus_connection_timout_add_emu) {
        SetCallbackArg(dbus_connection_timout_add_emu, 0, timeout);
        SetCallbackArg(dbus_connection_timout_add_emu, 1, data);
        return RunCallback(dbus_connection_timout_add_emu);
    }
    return 0;
}
void my_dbus_connection_timout_remove_cb(void *timeout, void *data)
{
    if(dbus_connection_timout_remove_emu) {
        SetCallbackArg(dbus_connection_timout_remove_emu, 0, timeout);
        SetCallbackArg(dbus_connection_timout_remove_emu, 1, data);
        RunCallback(dbus_connection_timout_remove_emu);
    }
}
void my_dbus_connection_timout_toggle_cb(void *timeout, void *data)
{
    if(dbus_connection_timout_toggle_emu) {
        SetCallbackArg(dbus_connection_timout_toggle_emu, 0, timeout);
        SetCallbackArg(dbus_connection_timout_toggle_emu, 1, data);
        RunCallback(dbus_connection_timout_toggle_emu);
    }
}
void my_dbus_connection_timout_free_cb(void *memory)
{
    if(dbus_connection_timout_free_emu) {
        SetCallbackArg(dbus_connection_timout_free_emu, 0, memory);
        RunCallback(dbus_connection_timout_free_emu);
    }
}

EXPORT int32_t my_dbus_connection_set_timeout_functions(x86emu_t* emu, void* c, void* a, void* r, void* t, void* d, void* f)
{
    library_t * lib = GetLib(emu->context->maplib, dbusName);
    dbus_my_t *my = (dbus_my_t*)lib->priv.w.p2;

    if(dbus_connection_timout_add_emu) FreeCallback(dbus_connection_timout_add_emu);
    if(dbus_connection_timout_remove_emu) FreeCallback(dbus_connection_timout_remove_emu);
    if(dbus_connection_timout_toggle_emu) FreeCallback(dbus_connection_timout_toggle_emu);
    if(dbus_connection_timout_free_emu) FreeCallback(dbus_connection_timout_free_emu);

    dbus_connection_timout_add_emu = a?AddSmallCallback(emu, (uintptr_t)a, 2, NULL, NULL, NULL, NULL):NULL;
    dbus_connection_timout_remove_emu = r?AddSmallCallback(emu, (uintptr_t)r, 2, NULL, NULL, NULL, NULL):NULL;
    dbus_connection_timout_toggle_emu = t?AddSmallCallback(emu, (uintptr_t)t, 2, NULL, NULL, NULL, NULL):NULL;
    dbus_connection_timout_free_emu = f?AddSmallCallback(emu, (uintptr_t)f, 1, NULL, NULL, NULL, NULL):NULL;
    return my->dbus_connection_set_timeout_functions(c, 
            a?my_dbus_connection_timout_add_cb:NULL, 
            r?my_dbus_connection_timout_remove_cb:NULL, 
            t?my_dbus_connection_timout_toggle_cb:NULL, 
            d, f?my_dbus_connection_timout_free_cb:NULL);
}

#define NF 4
#define SUPER() \
    GO(0)   \
    GO(1)   \
    GO(2)   \
    GO(3)

static box86context_t *context = NULL;
static uintptr_t my_filter_fnc[NF] = {0};
static uintptr_t my_filter_free[NF] = {0};

#define GO(A) \
    static int32_t my_filter_handler_##A (void* connection, void* message, void* data) { \
        return RunFunction(context, my_filter_fnc[A], 3, connection, message, data); \
    }   \
    static void my_filter_free_##A (void* memory) { \
        RunFunction(context, my_filter_free[A], 1, memory); \
    }

SUPER()

#undef GO

static int getSetFilter(uintptr_t fnc, uintptr_t fr, void** cbfnc, void** cbfree) {
    #define GO(A) if(!my_filter_fnc[A]) {my_filter_fnc[A]=fnc; my_filter_free[A]=fr; *cbfnc = my_filter_handler_##A; *cbfree = my_filter_free_##A; return A+1;}
    SUPER()
    #undef GO
    return 0;
}

static int getFilter(uintptr_t fnc, void** cbfnc) {
    #define GO(A) if(my_filter_fnc[A]==fnc) {*cbfnc = my_filter_handler_##A; return A+1;}
    SUPER()
    #undef GO
    return 0;
}

static void freeFilter(uintptr_t fnc) {
    #define GO(A) if(fnc == my_filter_fnc[A]) {my_filter_fnc[A]=0; my_filter_free[A]=0; return;}
    SUPER()
    #undef GO
}
// free
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
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_free_fct_##A == (uintptr_t)fct) return my_free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_fct_##A == 0) {my_free_fct_##A = (uintptr_t)fct; return my_free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus free callback\n");
    return NULL;
}

// DBusPendingCallNotifyFunction
#define GO(A)   \
static uintptr_t my_DBusPendingCallNotifyFunction_fct_##A = 0;   \
static void my_DBusPendingCallNotifyFunction_##A(void* pending, void* data)     \
{                                       \
    RunFunction(my_context, my_DBusPendingCallNotifyFunction_fct_##A, 2, pending, data);\
}
SUPER()
#undef GO
static void* findDBusPendingCallNotifyFunctionFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusPendingCallNotifyFunction_fct_##A == (uintptr_t)fct) return my_DBusPendingCallNotifyFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusPendingCallNotifyFunction_fct_##A == 0) {my_DBusPendingCallNotifyFunction_fct_##A = (uintptr_t)fct; return my_DBusPendingCallNotifyFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus DBusPendingCallNotifyFunction callback\n");
    return NULL;
}

// DBusDispatchStatusFunction
#define GO(A)   \
static uintptr_t my_DBusDispatchStatusFunction_fct_##A = 0;   \
static void my_DBusDispatchStatusFunction_##A(void* connection, int new_status, void* data)     \
{                                       \
    RunFunction(my_context, my_DBusDispatchStatusFunction_fct_##A, 3, connection, new_status, data);\
}
SUPER()
#undef GO
static void* findDBusDispatchStatusFunctionFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusDispatchStatusFunction_fct_##A == (uintptr_t)fct) return my_DBusDispatchStatusFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusDispatchStatusFunction_fct_##A == 0) {my_DBusDispatchStatusFunction_fct_##A = (uintptr_t)fct; return my_DBusDispatchStatusFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus DBusDispatchStatusFunction callback\n");
    return NULL;
}

// DBusAddWatchFunction
#define GO(A)   \
static uintptr_t my_DBusAddWatchFunction_fct_##A = 0;   \
static int my_DBusAddWatchFunction_##A(void* watch, void* data)     \
{                                       \
    return (int)RunFunction(my_context, my_DBusAddWatchFunction_fct_##A, 2, watch, data);\
}
SUPER()
#undef GO
static void* findDBusAddWatchFunctionFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusAddWatchFunction_fct_##A == (uintptr_t)fct) return my_DBusAddWatchFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusAddWatchFunction_fct_##A == 0) {my_DBusAddWatchFunction_fct_##A = (uintptr_t)fct; return my_DBusAddWatchFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus DBusAddWatchFunction callback\n");
    return NULL;
}

// DBusRemoveWatchFunction
#define GO(A)   \
static uintptr_t my_DBusRemoveWatchFunction_fct_##A = 0;   \
static void my_DBusRemoveWatchFunction_##A(void* watch, void* data)     \
{                                       \
    RunFunction(my_context, my_DBusRemoveWatchFunction_fct_##A, 2, watch, data);\
}
SUPER()
#undef GO
static void* findDBusRemoveWatchFunctionFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusRemoveWatchFunction_fct_##A == (uintptr_t)fct) return my_DBusRemoveWatchFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusRemoveWatchFunction_fct_##A == 0) {my_DBusRemoveWatchFunction_fct_##A = (uintptr_t)fct; return my_DBusRemoveWatchFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus DBusRemoveWatchFunction callback\n");
    return NULL;
}

// DBusWatchToggledFunction
#define GO(A)   \
static uintptr_t my_DBusWatchToggledFunction_fct_##A = 0;   \
static void my_DBusWatchToggledFunction_##A(void* watch, void* data)     \
{                                       \
    RunFunction(my_context, my_DBusWatchToggledFunction_fct_##A, 2, watch, data);\
}
SUPER()
#undef GO
static void* findDBusWatchToggledFunctionFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusWatchToggledFunction_fct_##A == (uintptr_t)fct) return my_DBusWatchToggledFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusWatchToggledFunction_fct_##A == 0) {my_DBusWatchToggledFunction_fct_##A = (uintptr_t)fct; return my_DBusWatchToggledFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus DBusWatchToggledFunction callback\n");
    return NULL;
}

// DBusObjectPathUnregisterFunction
#define GO(A)   \
static uintptr_t my_DBusObjectPathUnregisterFunction_fct_##A = 0;   \
static void my_DBusObjectPathUnregisterFunction_##A(void* connection, void* data)     \
{                                       \
    RunFunction(my_context, my_DBusObjectPathUnregisterFunction_fct_##A, 2, connection, data);\
}
SUPER()
#undef GO
static void* findDBusObjectPathUnregisterFunctionFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusObjectPathUnregisterFunction_fct_##A == (uintptr_t)fct) return my_DBusObjectPathUnregisterFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusObjectPathUnregisterFunction_fct_##A == 0) {my_DBusObjectPathUnregisterFunction_fct_##A = (uintptr_t)fct; return my_DBusObjectPathUnregisterFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus DBusObjectPathUnregisterFunction callback\n");
    return NULL;
}

// DBusObjectPathMessageFunction
#define GO(A)   \
static uintptr_t my_DBusObjectPathMessageFunction_fct_##A = 0;   \
static void my_DBusObjectPathMessageFunction_##A(void* connection, void* message, void* data)     \
{                                       \
    RunFunction(my_context, my_DBusObjectPathMessageFunction_fct_##A, 3, connection, message, data);\
}
SUPER()
#undef GO
static void* findDBusObjectPathMessageFunctionFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DBusObjectPathMessageFunction_fct_##A == (uintptr_t)fct) return my_DBusObjectPathMessageFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusObjectPathMessageFunction_fct_##A == 0) {my_DBusObjectPathMessageFunction_fct_##A = (uintptr_t)fct; return my_DBusObjectPathMessageFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus DBusObjectPathMessageFunction callback\n");
    return NULL;
}

// dbus_internal_pad
#define GO(A)   \
static uintptr_t my_dbus_internal_pad_fct_##A = 0;   \
static void my_dbus_internal_pad_##A(void* a, void* b, void* c, void* d)     \
{                                       \
    RunFunction(my_context, my_dbus_internal_pad_fct_##A, 4, a, b, c, d);\
}
SUPER()
#undef GO
static void* finddbus_internal_padFct(void* fct)
{
    if(!fct) return NULL;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_dbus_internal_pad_fct_##A == (uintptr_t)fct) return my_dbus_internal_pad_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_dbus_internal_pad_fct_##A == 0) {my_dbus_internal_pad_fct_##A = (uintptr_t)fct; return my_dbus_internal_pad_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus dbus_internal_pad callback\n");
    return NULL;
}
#undef SUPER

EXPORT int my_dbus_connection_add_filter(x86emu_t* emu, void* connection, void* fnc, void* data, void* fr)
{
    library_t * lib = GetLib(emu->context->maplib, dbusName);
    dbus_my_t *my = (dbus_my_t*)lib->priv.w.p2;

    if (!context)
        context = emu->context;

    void* cbfnc = NULL;
    void* cbfree= NULL;
    if(getSetFilter((uintptr_t)fnc, (uintptr_t)fr, &cbfnc, &cbfree)) {
        return my->dbus_connection_add_filter(connection, cbfnc, data, fr?cbfree:NULL);
    }
    printf_log(LOG_NONE, "Error: no more slot for dbus_connection_add_filter\n");
    return 0;
}

EXPORT void my_dbus_connection_remove_filter(x86emu_t* emu, void* connection, void* fnc, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, dbusName);
    dbus_my_t *my = (dbus_my_t*)lib->priv.w.p2;

    if (!context)
        context = emu->context;

    void* cbfnc = NULL;
    if(getFilter((uintptr_t)fnc, &cbfnc)) {
        my->dbus_connection_remove_filter(connection, cbfnc, data);
    }
    freeFilter((uintptr_t)fnc);
    
}

EXPORT int my_dbus_message_get_args_valist(x86emu_t* emu, void* message, void* e, int arg, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, dbusName);
    dbus_my_t *my = (dbus_my_t*)lib->priv.w.p2;

    // need to develop this specific alignment!
    #if 0   //ndef NOALIGN
    myStackAlign((const char*)fmt, *(uint32_t**)b, emu->scratch);
    void* f = vprintf;
    return my->dbus_message_get_args_valist(message, e, arg, emu->scratch);
    #else
    return my->dbus_message_get_args_valist(message, e, arg, *(uint32_t**)b);
    #endif
}

EXPORT int my_dbus_message_get_args(x86emu_t* emu, void* message, void* e, int arg, void* V)
{
    library_t * lib = GetLib(emu->context->maplib, dbusName);
    dbus_my_t *my = (dbus_my_t*)lib->priv.w.p2;

    // need to develop this specific alignment!
    #if 0   //ndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    return my->dbus_message_get_args_valist(message, e, arg, emu->scratch);
    #else
    return my->dbus_message_get_args_valist(message, e, arg, V);
    #endif
}

EXPORT int my_dbus_message_set_data(x86emu_t* emu, void* message, int32_t slot, void* data, void* free_func)
{
    library_t * lib = GetLib(emu->context->maplib, dbusName);
    dbus_my_t *my = (dbus_my_t*)lib->priv.w.p2;

    return my->dbus_message_set_data(message, slot, data, findFreeFct(free_func));
}

EXPORT int my_dbus_pending_call_set_notify(x86emu_t* emu, void* pending, void* func, void* data, void* free_func)
{
    library_t * lib = GetLib(emu->context->maplib, dbusName);
    dbus_my_t *my = (dbus_my_t*)lib->priv.w.p2;

    return my->dbus_pending_call_set_notify(pending, findDBusPendingCallNotifyFunctionFct(func), data, findFreeFct(free_func));
}

EXPORT int my_dbus_pending_call_set_data(x86emu_t* emu, void* pending, int32_t slot, void* data, void* free_func)
{
    library_t * lib = GetLib(emu->context->maplib, dbusName);
    dbus_my_t *my = (dbus_my_t*)lib->priv.w.p2;

    return my->dbus_pending_call_set_data(pending, slot, data, findFreeFct(free_func));
}

EXPORT void my_dbus_watch_set_data(x86emu_t* emu, void* watch, void* data, void* free_func)
{
    library_t * lib = GetLib(emu->context->maplib, dbusName);
    dbus_my_t *my = (dbus_my_t*)lib->priv.w.p2;

    my->dbus_watch_set_data(watch, data, findFreeFct(free_func));
}

EXPORT void my_dbus_connection_set_dispatch_status_function(x86emu_t* emu, void* connection, void* dispatch, void* data, void* free_func)
{
    library_t * lib = GetLib(emu->context->maplib, dbusName);
    dbus_my_t *my = (dbus_my_t*)lib->priv.w.p2;

    my->dbus_connection_set_dispatch_status_function(connection, findDBusDispatchStatusFunctionFct(dispatch), data, findFreeFct(free_func));
}

EXPORT int my_dbus_connection_set_watch_functions(x86emu_t* emu, void* connection, void* add, void* remove, void* toggled, void* data, void* free_func)
{
    library_t * lib = GetLib(emu->context->maplib, dbusName);
    dbus_my_t *my = (dbus_my_t*)lib->priv.w.p2;

    return my->dbus_connection_set_watch_functions(connection, findDBusAddWatchFunctionFct(add), findDBusRemoveWatchFunctionFct(remove), findDBusWatchToggledFunctionFct(toggled), data, findFreeFct(free_func));
}

typedef struct my_DBusObjectPathVTable_s
{
   void*    unregister_function; 
   void*    message_function; 
   void*    pad1; 
   void*    pad2; 
   void*    pad3; 
   void*    pad4; 
} my_DBusObjectPathVTable_t;

EXPORT int my_dbus_connection_try_register_object_path(x86emu_t* emu, void* connection, void* path, my_DBusObjectPathVTable_t* vtable, void* data, void* error)
{
    library_t * lib = GetLib(emu->context->maplib, dbusName);
    dbus_my_t *my = (dbus_my_t*)lib->priv.w.p2;

    my_DBusObjectPathVTable_t vt = {0};
    if(vtable) {
        vt.unregister_function = findDBusObjectPathUnregisterFunctionFct(vtable->unregister_function);
        vt.message_function = findDBusObjectPathMessageFunctionFct(vtable->message_function);
        vt.pad1 = finddbus_internal_padFct(vtable->pad1);
        vt.pad2 = finddbus_internal_padFct(vtable->pad2);
        vt.pad3 = finddbus_internal_padFct(vtable->pad3);
        vt.pad4 = finddbus_internal_padFct(vtable->pad4);
    }

    return my->dbus_connection_try_register_object_path(connection, path, vtable?&vt:NULL, data, error);
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getDBusMy(lib);

#define CUSTOM_FINI \
    freeDBusMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

