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

const char* libusb1Name = "libusb-1.0.so.0";
#define LIBNAME libusb1

typedef int     (*iFpiiiiippp_t)    (void*, int, int, int, int, int, void*, void*, void*);

static library_t* my_lib = NULL;

#define SUPER() \
    GO(libusb_hotplug_register_callback, iFpiiiiippp_t)

typedef struct libusb1_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} libusb1_my_t;

void* getUsb1My(library_t* lib)
{
    libusb1_my_t* my = (libusb1_my_t*)calloc(1, sizeof(libusb1_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeUsb1My(void* lib)
{
//    libusb1_my_t *my = (libusb1_my_t *)lib;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// hotplug
#define GO(A)   \
static uintptr_t my_hotplug_fct_##A = 0;                                                    \
static int my_hotplug_##A(void* ctx, void* device, int event, void* data)                   \
{                                                                                           \
    return (int)RunFunction(my_context, my_hotplug_fct_##A, 4, ctx, device, event, data);   \
}
SUPER()
#undef GO
static void* findhotplugFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_hotplug_fct_##A == (uintptr_t)fct) return my_hotplug_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_hotplug_fct_##A == 0) {my_hotplug_fct_##A = (uintptr_t)fct; return my_hotplug_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libusb-1.0 hotplug callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_libusb_hotplug_register_callback(x86emu_t* emu, void* ctx, int event, int flags, int vendor, int product, int dev_class, void* f, void* data, void* handle)
{
    libusb1_my_t *my = (libusb1_my_t*)my_lib->priv.w.p2;

    return my->libusb_hotplug_register_callback(ctx, event, flags, vendor, product, dev_class, findhotplugFct(f), data, handle);
}

#define CUSTOM_INIT \
    my_lib = lib;   \
    lib->priv.w.p2 = getUsb1My(lib);

#define CUSTOM_FINI \
    freeUsb1My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

