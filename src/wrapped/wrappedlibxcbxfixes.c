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

const char* libxcbxfixesName = "libxcb-xfixes.so.0";
#define LIBNAME libxcbxfixes

typedef struct my_xcb_cookie_s {
    uint32_t        data;
} my_xcb_cookie_t;

typedef my_xcb_cookie_t (*XFpu_t)(void*, uint32_t);
typedef my_xcb_cookie_t (*XFpuu_t)(void*, uint32_t, uint32_t);
typedef my_xcb_cookie_t (*XFpuup_t)(void*, uint32_t, uint32_t, void*);
typedef my_xcb_cookie_t (*XFpuuwwu_t)(void*, uint32_t, uint32_t, int16_t, int16_t, uint32_t);

#define SUPER() \
    GO(xcb_xfixes_query_version_unchecked, XFpuu_t)             \
    GO(xcb_xfixes_create_region, XFpuup_t)                      \
    GO(xcb_xfixes_destroy_region, XFpu_t)                       \
    GO(xcb_xfixes_set_window_shape_region, XFpuuwwu_t)          \
    GO(xcb_xfixes_set_window_shape_region_checked, XFpuuwwu_t)  \

typedef struct xcbxfixes_my_s {
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
    // functions
} xcbxfixes_my_t;

void* getXcbxfixesMy(library_t* lib)
{
    xcbxfixes_my_t* my = (xcbxfixes_my_t*)calloc(1, sizeof(xcbxfixes_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeXcbxfixesMy(void* lib)
{
    //xcbxfixes_my_t *my = (xcbxfixes_my_t *)lib;
}

#define SUPER(F, P, ...)                                            \
    EXPORT void* my_##F P                                           \
    {                                                               \
        xcbxfixes_my_t *my = (xcbxfixes_my_t*)emu->context->libxcbxfixes->priv.w.p2;  \
        *ret = my->F(__VA_ARGS__);                                  \
        return ret;                                                 \
    }

SUPER(xcb_xfixes_query_version_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t majver, uint32_t minver), c, majver, minver)
SUPER(xcb_xfixes_create_region, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t region, uint32_t rectangles_len, void* rectangles), c, region, rectangles_len, rectangles)
SUPER(xcb_xfixes_destroy_region, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t region), c, region)
SUPER(xcb_xfixes_set_window_shape_region, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint32_t kind, int16_t x, int16_t y, uint32_t region), c, win, kind, x, y, region)
SUPER(xcb_xfixes_set_window_shape_region_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint32_t kind, int16_t x, int16_t y, uint32_t region), c, win, kind, x, y, region)
#undef SUPER


#define CUSTOM_INIT \
    box86->libxcbxfixes = lib;                \
    lib->priv.w.p2 = getXcbxfixesMy(lib);

#define CUSTOM_FINI \
    freeXcbxfixesMy(lib->priv.w.p2);  \
    free(lib->priv.w.p2);       \
    ((box86context_t*)(lib->context))->libxcbxfixes = NULL;

#include "wrappedlib_init.h"
