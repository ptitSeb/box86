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
    const char* libxcbimageName = "libxcb-image.so";
#else
    const char* libxcbimageName = "libxcb-image.so.0";
#endif

#define LIBNAME libxcbimage

typedef struct my_xcb_cookie_s {
    uint32_t        data;
} my_xcb_cookie_t;

typedef my_xcb_cookie_t (*XFpuupwwC_t)(void*, uint32_t, uint32_t, void*, int16_t, int16_t, uint8_t);

#define SUPER() \
    GO(xcb_image_put, XFpuupwwC_t)                  \

#include "wrappercallback.h"

#define SUPER(F, P, ...)           \
    EXPORT void* my_##F P          \
    {                              \
        (void)emu;                 \
        *ret = my->F(__VA_ARGS__); \
        return ret;                \
    }

SUPER(xcb_image_put, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t d, uint32_t gc, void* img, int16_t x, int16_t y, uint8_t pad), c, d, gc, img, x, y, pad)

#undef SUPER


#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
