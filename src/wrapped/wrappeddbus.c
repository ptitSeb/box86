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
typedef int32_t (*iFpppppp_t)(void*, void*, void*, void*, void*, void*);

typedef struct dbus_my_s {
    // functions
    vFppp_t        dbus_timeout_set_data;
    iFpppppp_t     dbus_connection_set_timeout_functions;
} dbus_my_t;

static void* getDBusMy(library_t* lib)
{
    dbus_my_t* my = (dbus_my_t*)calloc(1, sizeof(dbus_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(dbus_timeout_set_data, vFppp_t)
    GO(dbus_connection_set_timeout_functions, iFpppppp_t)
    #undef GO
    return my;
}

static void freeDBusMy(void* lib)
{
    dbus_my_t *my = (dbus_my_t *)lib;
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

#define CUSTOM_INIT \
    lib->priv.w.p2 = getDBusMy(lib);

#define CUSTOM_FINI \
    freeDBusMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

