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
    const char* libxcbdri2Name = "libxcb-dri2.so";
#else
    const char* libxcbdri2Name = "libxcb-dri2.so.0";
#endif

#define LIBNAME libxcbdri2

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

#include "wrappercallback.h"

#define SUPER(F, P, ...)           \
    EXPORT void* my_##F P          \
    {                              \
        (void)emu;                 \
        *ret = my->F(__VA_ARGS__); \
        return ret;                \
    }

SUPER(xcb_dri2_authenticate, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint32_t magic), c, win, magic)
SUPER(xcb_dri2_connect, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint32_t magic), c, win, magic)
SUPER(xcb_dri2_query_version, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t major, uint32_t minor), c, major, minor)

#undef SUPER


#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
