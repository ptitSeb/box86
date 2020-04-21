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

#if 0
typedef void* (*pFppiiLpppip_t)(void*, void*, int, int, unsigned long, void*, void*, void*, int, void*);

#define SUPER() \
    GO(SmcOpenConnection, pFppiiLpppip_t)

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
    libusb1_my_t *my = (libusb1_my_t *)lib;
}


#define CUSTOM_INIT \
    lib->priv.w.p2 = getUsb1My(lib);

#define CUSTOM_FINI \
    freeUsb1My(lib->priv.w.p2); \
    free(lib->priv.w.p2);
#endif

#include "wrappedlib_init.h"

