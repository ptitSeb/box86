#define _GNU_SOURCE         /* See feature_test_macros(7) */
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
#include "emu/x86emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box86context.h"
#include "emu/x86emu_private.h"

#ifdef ANDROID
    const char* libxcbshapeName = "libxcb-shape.so";
#else
    const char* libxcbshapeName = "libxcb-shape.so.0";
#endif

#define LIBNAME libxcbshape

typedef struct my_xcb_cookie_s {
    uint32_t        data;
} my_xcb_cookie_t;

typedef my_xcb_cookie_t (*XFpuuuwwu_t)  (void*, uint32_t, uint32_t, uint32_t, int16_t, int16_t, uint32_t);

#define SUPER() \
    GO(xcb_shape_mask, XFpuuuwwu_t)             \
    GO(xcb_shape_mask_checked, XFpuuuwwu_t)     \

#include "wrappercallback.h"

#define SUPER(F, P, ...)           \
    EXPORT void* my_##F P          \
    {                              \
        (void)emu;                 \
        *ret = my->F(__VA_ARGS__); \
        return ret;                \
    }

SUPER(xcb_shape_mask, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t op, uint32_t kind, uint32_t dest, int16_t x, int16_t y, uint32_t bitmap), c, op, kind, dest, x, y, bitmap)
SUPER(xcb_shape_mask_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t op, uint32_t kind, uint32_t dest, int16_t x, int16_t y, uint32_t bitmap), c, op, kind, dest, x, y, bitmap)

#undef SUPER


#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
