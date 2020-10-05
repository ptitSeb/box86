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

const char* libxcbdri3Name = "libxcb-dri3.so.0";
#define LIBNAME libxcbdri3
static library_t *my_lib = NULL;

typedef struct my_xcb_cookie_s {
    uint32_t        data;
} my_xcb_cookie_t;

typedef my_xcb_cookie_t (*XFpu_t)(void*, uint32_t);
typedef my_xcb_cookie_t (*XFpuu_t)(void*, uint32_t, uint32_t);
typedef my_xcb_cookie_t (*XFpuuuWWWCCi_t)(void*, uint32_t, uint32_t, uint32_t, uint16_t, uint16_t, uint16_t, uint8_t, uint8_t, int32_t);

#define SUPER() \
    GO(xcb_dri3_open, XFpuu_t)                              \
    GO(xcb_dri3_pixmap_from_buffer_checked, XFpuuuWWWCCi_t) \
    GO(xcb_dri3_query_version, XFpuu_t)                     \

typedef struct xcbdri3_my_s {
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
    // functions
} xcbdri3_my_t;

void* getXcbdri3My(library_t* lib)
{
    xcbdri3_my_t* my = (xcbdri3_my_t*)calloc(1, sizeof(xcbdri3_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeXcbdri3My(void* lib)
{
    //xcbdri3_my_t *my = (xcbdri3_my_t *)lib;
}

#define SUPER(F, P, ...)                        \
    EXPORT void* my_##F P                       \
    {                                           \
        xcbdri3_my_t *my = my_lib->priv.w.p2;   \
        *ret = my->F(__VA_ARGS__);              \
        return ret;                             \
    }

SUPER(xcb_dri3_open, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t d, uint32_t provider), c, d, provider)
SUPER(xcb_dri3_pixmap_from_buffer_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t pix, uint32_t d, uint32_t size, uint16_t w, uint16_t h, uint16_t stride, uint8_t depth, uint8_t bpp, int32_t fd), c, pix, d, size, w, h, stride, depth, bpp, fd)
SUPER(xcb_dri3_query_version, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t major, uint32_t minor), c, major, minor)

#undef SUPER


#define CUSTOM_INIT \
    my_lib = lib;                \
    lib->priv.w.p2 = getXcbdri3My(lib);

#define CUSTOM_FINI \
    freeXcbdri3My(lib->priv.w.p2);  \
    free(lib->priv.w.p2);       \
    my_lib = NULL;

#include "wrappedlib_init.h"
