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

const char* libxcbshapeName = "libxcb-shape.so.0";
#define LIBNAME libxcbshape


typedef struct my_xcb_cookie_s {
    uint32_t        data;
} my_xcb_cookie_t;

typedef my_xcb_cookie_t (*XFpuuuwwu_t)  (void*, uint32_t, uint32_t, uint32_t, int16_t, int16_t, uint32_t);

#define SUPER() \
    GO(xcb_shape_mask, XFpuuuwwu_t)             \
    GO(xcb_shape_mask_checked, XFpuuuwwu_t)     \


typedef struct xcbshape_my_s {
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
    // functions
} xcbshape_my_t;

void* getXcbshapeMy(library_t* lib)
{
    xcbshape_my_t* my = (xcbshape_my_t*)calloc(1, sizeof(xcbshape_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeXcbshapeMy(void* lib)
{
    //xcbshape_my_t *my = (xcbshape_my_t *)lib;
}

#define SUPER(F, P, ...)                                            \
    EXPORT void* my_##F P                                           \
    {                                                               \
        xcbshape_my_t *my = (xcbshape_my_t*)emu->context->libxcbshape->priv.w.p2;  \
        *ret = my->F(__VA_ARGS__);                                  \
        return ret;                                                 \
    }

SUPER(xcb_shape_mask, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t op, uint32_t kind, uint32_t dest, int16_t x, int16_t y, uint32_t bitmap), c, op, kind, dest, x, y, bitmap)
SUPER(xcb_shape_mask_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t op, uint32_t kind, uint32_t dest, int16_t x, int16_t y, uint32_t bitmap), c, op, kind, dest, x, y, bitmap)

#undef SUPER


#define CUSTOM_INIT \
    box86->libxcbshape = lib;                \
    lib->priv.w.p2 = getXcbshapeMy(lib);

#define CUSTOM_FINI \
    freeXcbshapeMy(lib->priv.w.p2);  \
    free(lib->priv.w.p2);       \
    ((box86context_t*)(lib->context))->libxcb = NULL;

#include "wrappedlib_init.h"
