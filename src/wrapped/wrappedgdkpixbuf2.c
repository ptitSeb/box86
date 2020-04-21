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

const char* gdkpixbuf2Name = "libgdk_pixbuf-2.0.so.0";
#define LIBNAME gdkpixbuf2

typedef void* (*pFpiiiiiipp_t)(void*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, void*, void*);

#define SUPER() \
    GO(gdk_pixbuf_new_from_data, pFpiiiiiipp_t)

typedef struct gdkpixbuf2_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} gdkpixbuf2_my_t;

void* getGdkpixbuf2My(library_t* lib)
{
    gdkpixbuf2_my_t* my = (gdkpixbuf2_my_t*)calloc(1, sizeof(gdkpixbuf2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeGdkpixbuf2My(void* lib)
{
    //gdkpixbuf2_my_t *my = (gdkpixbuf2_my_t *)lib;
}

static void my_destroy_pixbuf(void* pixels, void* data)
{
    x86emu_t *emu = (x86emu_t*)data;
    SetCallbackArg(emu, 0, pixels);
    RunCallback(emu);
    FreeCallback(emu);
}
EXPORT void* my_gdk_pixbuf_new_from_data(x86emu_t* emu, void* data, int32_t colorspace, int32_t has_alpha, int32_t bpp, int32_t w, int32_t h, int32_t stride, void* destroy_func, void* destroy_data)
{
    library_t * lib = GetLib(emu->context->maplib, gdkpixbuf2Name);
    gdkpixbuf2_my_t *my = (gdkpixbuf2_my_t*)lib->priv.w.p2;

    x86emu_t *emu_cb = NULL;
    if(destroy_func) {
        emu_cb = AddSmallCallback(emu, (uintptr_t)destroy_func, 2, NULL, destroy_data, NULL, NULL);
    }
    return my->gdk_pixbuf_new_from_data(data, colorspace, has_alpha, bpp, w, h, stride, destroy_func?my_destroy_pixbuf:NULL, emu_cb);
}


#define CUSTOM_INIT \
    lib->priv.w.p2 = getGdkpixbuf2My(lib);

#define CUSTOM_FINI \
    freeGdkpixbuf2My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"
