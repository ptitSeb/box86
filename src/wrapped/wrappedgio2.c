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
#include "myalign.h"

const char* gio2Name = "libgio-2.0.so.0";
#define LIBNAME gio2

static char* libname = NULL;
static box86context_t* my_context = NULL;

typedef void    (*vFpppiippp_t)     (void*, void*, void*, int, int, void*, void*, void*);
typedef void    (*vFpppiipppp_t)    (void*, void*, void*, int, int, void*, void*, void*, void*);
typedef void    (*vFiippppppp_t)    (int, int, void*, void*, void*, void*, void*, void*, void*);
typedef void*   (*pFiippppppp_t)    (int, int, void*, void*, void*, void*, void*, void*, void*);
typedef void    (*vFpippppppp_t)    (void*, int, void*, void*, void*, void*, void*, void*, void*);

#define SUPER() \
    GO(g_dbus_proxy_new, vFpippppppp_t)             \
    GO(g_dbus_proxy_new_for_bus, vFiippppppp_t)     \
    GO(g_dbus_proxy_call, vFpppiippp_t)             \
    GO(g_dbus_proxy_call_with_unix_fd_list, vFpppiipppp_t)              \
    GO(g_dbus_object_manager_client_new_for_bus_sync, pFiippppppp_t)

typedef struct gio2_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} gio2_my_t;

void* getGio2My(library_t* lib)
{
    gio2_my_t* my = (gio2_my_t*)calloc(1, sizeof(gio2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeGio2My(void* lib)
{
    gio2_my_t *my = (gio2_my_t *)lib;
}


#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// GAsyncReadyCallback
#define GO(A)   \
static uintptr_t my_GAsyncReadyCallback_fct_##A = 0;   \
static void my_GAsyncReadyCallback_##A(void* source, void* res, void* data)     \
{                                       \
    RunFunction(my_context, my_GAsyncReadyCallback_fct_##A, 3, source, res, data);\
}
SUPER()
#undef GO
static void* findGAsyncReadyCallbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GAsyncReadyCallback_fct_##A == (uintptr_t)fct) return my_GAsyncReadyCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GAsyncReadyCallback_fct_##A == 0) {my_GAsyncReadyCallback_fct_##A = (uintptr_t)fct; return my_GAsyncReadyCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GAsyncReadyCallback callback\n");
    return NULL;
}

// GDestroyNotify
#define GO(A)   \
static uintptr_t my_GDestroyNotify_fct_##A = 0;   \
static void my_GDestroyNotify_##A(void* data)     \
{                                       \
    RunFunction(my_context, my_GDestroyNotify_fct_##A, 1, data);\
}
SUPER()
#undef GO
static void* findGDestroyNotifyFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GDestroyNotify_fct_##A == (uintptr_t)fct) return my_GDestroyNotify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GDestroyNotify_fct_##A == 0) {my_GDestroyNotify_fct_##A = (uintptr_t)fct; return my_GDestroyNotify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GDestroyNotify callback\n");
    return NULL;
}

// GDBusProxyTypeFunc
#define GO(A)   \
static uintptr_t my_GDBusProxyTypeFunc_fct_##A = 0;   \
static int my_GDBusProxyTypeFunc_##A(void* manager, void* path, void* name, void* data)     \
{                                       \
    return (int)RunFunction(my_context, my_GDBusProxyTypeFunc_fct_##A, 4, manager, path, name, data);\
}
SUPER()
#undef GO
static void* findGDBusProxyTypeFuncFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GDBusProxyTypeFunc_fct_##A == (uintptr_t)fct) return my_GDBusProxyTypeFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GDBusProxyTypeFunc_fct_##A == 0) {my_GDBusProxyTypeFunc_fct_##A = (uintptr_t)fct; return my_GDBusProxyTypeFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gio2 GDBusProxyTypeFunc callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_g_dbus_proxy_new(x86emu_t* emu, void* connection, int flags, void* info, void* name, void* path, void* interface, void* cancellable, void* cb, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    gio2_my_t *my = (gio2_my_t*)lib->priv.w.p2;

    my->g_dbus_proxy_new(connection, flags, info, name, path, interface, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_g_dbus_proxy_new_for_bus(x86emu_t* emu, int bus_type, int flags, void* info, void* name, void* path, void* interface, void* cancellable, void* cb, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    gio2_my_t *my = (gio2_my_t*)lib->priv.w.p2;

    my->g_dbus_proxy_new_for_bus(bus_type, flags, info, name, path, interface, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_g_dbus_proxy_call(x86emu_t* emu, void* proxy, void* name, void* param, int flags, int timeout, void* cancellable, void* cb, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    gio2_my_t *my = (gio2_my_t*)lib->priv.w.p2;

    my->g_dbus_proxy_call(proxy, name, param, flags, timeout, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_g_dbus_proxy_call_with_unix_fd_list(x86emu_t* emu, void* proxy, void* name, void* param, int flags, int timeout, void* fd_list, void* cancellable, void* cb, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    gio2_my_t *my = (gio2_my_t*)lib->priv.w.p2;

    my->g_dbus_proxy_call_with_unix_fd_list(proxy, name, param, flags, timeout, fd_list, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void* my_g_dbus_object_manager_client_new_for_bus_sync(x86emu_t* emu, int bus, int flags, void* name, void* path, void* cb, void* data, void* destroy, void* cancellable, void* error)
{
    library_t * lib = GetLib(emu->context->maplib, libname);
    gio2_my_t *my = (gio2_my_t*)lib->priv.w.p2;

    return my->g_dbus_object_manager_client_new_for_bus_sync(bus, flags, name, path, findGDBusProxyTypeFuncFct(cb), data, findGDestroyNotifyFct(destroy), cancellable, error);
}

#define CUSTOM_INIT \
    my_context = box86;                 \
    libname = lib->name;                \
    lib->priv.w.p2 = getGio2My(lib);   \
    lib->priv.w.needed = 1;             \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libgmodule-2.0.so.0");

#define CUSTOM_FINI \
    freeGio2My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"
