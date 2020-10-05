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

const char* libxcbdri2Name = "libxcb-dri2.so.0";
#define LIBNAME libxcbdri2
static library_t *my_lib = NULL;

typedef struct my_xcb_cookie_s {
    uint32_t        data;
} my_xcb_cookie_t;

typedef my_xcb_cookie_t (*XFpu_t)(void*, uint32_t);
typedef my_xcb_cookie_t (*XFpuu_t)(void*, uint32_t, uint32_t);
typedef my_xcb_cookie_t (*XFpuup_t)(void*, uint32_t, uint32_t, void*);

#define SUPER() \
    GO(xcb_dri2_authenticate, XFpuu_t)          \
    GO(xcb_dri2_connect, XFpuu_t)               \
    GO(xcb_dri2_query_version, XFpuu_t)         \

typedef struct xcbdri2_my_s {
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
    // functions
} xcbdri2_my_t;

void* getXcbdri2My(library_t* lib)
{
    xcbdri2_my_t* my = (xcbdri2_my_t*)calloc(1, sizeof(xcbdri2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeXcbdri2My(void* lib)
{
    //xcbdri2_my_t *my = (xcbdri2_my_t *)lib;
}

#define SUPER(F, P, ...)                        \
    EXPORT void* my_##F P                       \
    {                                           \
        xcbdri2_my_t *my = my_lib->priv.w.p2;   \
        *ret = my->F(__VA_ARGS__);              \
        return ret;                             \
    }

SUPER(xcb_dri2_authenticate, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint32_t magic), c, win, magic)
SUPER(xcb_dri2_connect, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint32_t magic), c, win, magic)
SUPER(xcb_dri2_query_version, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t major, uint32_t minor), c, major, minor)

#undef SUPER


#define CUSTOM_INIT \
    my_lib = lib;                \
    lib->priv.w.p2 = getXcbdri2My(lib);

#define CUSTOM_FINI \
    freeXcbdri2My(lib->priv.w.p2);  \
    free(lib->priv.w.p2);       \
    my_lib = NULL;

#include "wrappedlib_init.h"
