#define _GNU_SOURCE
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
#include "callback.h"

const char* libnmName = "libnm.so.0";
#define LIBNAME libnm

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedlibnmtypes.h"

#include "wrappercallback.h"

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// GAsyncReadyCallback
#define GO(A)   \
static uintptr_t my_GAsyncReadyCallback_fct_##A = 0;                            \
static void my_GAsyncReadyCallback_##A(void* a, void* b, void* c)               \
{                                                                               \
    RunFunctionFmt(my_GAsyncReadyCallback_fct_##A, "ppp", a, b, c); \
}
SUPER()
#undef GO
static void* findGAsyncReadyCallbackFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_GAsyncReadyCallback_fct_##A == (uintptr_t)fct) return my_GAsyncReadyCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GAsyncReadyCallback_fct_##A == 0) {my_GAsyncReadyCallback_fct_##A = (uintptr_t)fct; return my_GAsyncReadyCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libnm GAsyncReadyCallback callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_nm_remote_connection_commit_changes_async(x86emu_t* emu, void* connection, int save, void* cancellable, void* cb, void* data)
{
    (void)emu;
    my->nm_remote_connection_commit_changes_async(connection, save, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_nm_remote_connection_get_secrets_async(x86emu_t* emu, void* connection, void* name, void* cancellable, void* cb, void* data)
{
    (void)emu;
    my->nm_remote_connection_get_secrets_async(connection, name, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_nm_remote_connection_delete_async(x86emu_t* emu, void* connection, void* cancellable, void* cb, void* data)
{
    (void)emu;
    my->nm_remote_connection_delete_async(connection, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_nm_client_add_and_activate_connection_async(x86emu_t* emu, void* client, void* partial, void* device, void* spec, void* cancellable, void* cb, void* data)
{
    (void)emu;
    my->nm_client_add_and_activate_connection_async(client, partial, device, spec, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_nm_device_disconnect_async(x86emu_t* emu, void* device, void* cancellable, void* cb, void* data)
{
    (void)emu;
    my->nm_device_disconnect_async(device, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_nm_device_wifi_request_scan_async(x86emu_t* emu, void* device, void* cancellable, void* cb, void* data)
{
    (void)emu;
    my->nm_device_wifi_request_scan_async(device, cancellable, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_nm_client_add_connection_async(x86emu_t* emu, void* client, void* connection, int save, void* cancel, void* cb, void* data)
{
    (void)emu;
    my->nm_client_add_connection_async(client, connection, save, cancel, findGAsyncReadyCallbackFct(cb), data);
}

EXPORT void my_nm_client_activate_connection_async(x86emu_t* emu, void* client, void* connection, void* device, void* obj, void* cancel, void* cb, void* data)
{
    (void)emu;
    my->nm_client_activate_connection_async(client, connection, device, obj, cancel, findGAsyncReadyCallbackFct(cb), data);
}

#define PRE_INIT    \
    if(box86_nogtk) \
        return -1;

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
