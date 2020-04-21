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
typedef int32_t (*iFpppp_t)(void*, void*, void*, void*);
typedef int32_t (*iFppip_t)(void*, void*, int32_t, void*);
typedef int32_t (*iFpipp_t)(void*, int32_t, void*, void*);
typedef int32_t (*iFpppppp_t)(void*, void*, void*, void*, void*, void*);

#define SUPER() \
    GO(dbus_timeout_set_data, vFppp_t)  \
    GO(dbus_connection_set_timeout_functions, iFpppppp_t)   \
    GO(dbus_connection_add_filter, iFpppp_t)    \
    GO(dbus_connection_remove_filter, vFppp_t)  \
    GO(dbus_message_get_args_valist, iFppip_t)  \
    GO(dbus_message_set_data, iFpipp_t)         \
    GO(dbus_pending_call_set_notify, iFpppp_t)  \
    GO(dbus_pending_call_set_data, iFpipp_t)

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
    #define GO(A) if(my_DBusPendingCallNotifyFunction_fct_##A == (uintptr_t)fct) return my_DBusPendingCallNotifyFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DBusPendingCallNotifyFunction_fct_##A == 0) {my_DBusPendingCallNotifyFunction_fct_##A = (uintptr_t)fct; return my_DBusPendingCallNotifyFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for dbus DBusPendingCallNotifyFunction callback\n");
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

#define CUSTOM_INIT \
    lib->priv.w.p2 = getDBusMy(lib);

#define CUSTOM_FINI \
    freeDBusMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

