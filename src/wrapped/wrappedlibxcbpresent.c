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
    const char* libxcbpresentName = "libxcb-present.so";
#else
    const char* libxcbpresentName = "libxcb-present.so.0";
#endif

#define LIBNAME libxcbpresent

typedef struct my_xcb_cookie_s {
    uint32_t        data;
} my_xcb_cookie_t;

typedef my_xcb_cookie_t (*XFpuu_t)(void*, uint32_t, uint32_t);
typedef my_xcb_cookie_t (*XFpuuu_t)(void*, uint32_t, uint32_t, uint32_t);
typedef my_xcb_cookie_t (*XFpuuUUU_t)(void*, uint32_t, uint32_t, uint64_t, uint64_t, uint64_t);
typedef my_xcb_cookie_t (*XFpuuuuuwwuuuuUUUup_t)(void*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, int16_t, int16_t, uint32_t, uint32_t, uint32_t, uint32_t, uint64_t, uint64_t, uint64_t, uint32_t, void*);

#define SUPER() \
    GO(xcb_present_notify_msc, XFpuuUUU_t)                  \
    GO(xcb_present_pixmap_checked, XFpuuuuuwwuuuuUUUup_t)   \
    GO(xcb_present_pixmap, XFpuuuuuwwuuuuUUUup_t)           \
    GO(xcb_present_query_version, XFpuu_t)                  \
    GO(xcb_present_select_input_checked, XFpuuu_t)          \

#include "wrappercallback.h"

#define SUPER(F, P, ...)           \
    EXPORT void* my_##F P          \
    {                              \
        (void)emu;                 \
        *ret = my->F(__VA_ARGS__); \
        return ret;                \
    }

SUPER(xcb_present_notify_msc, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint32_t serial, uint64_t target, uint64_t divisor, uint64_t rem), c, win, serial, target, divisor, rem)
SUPER(xcb_present_pixmap_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint32_t pix, uint32_t serial, uint32_t valid, uint32_t update, int16_t x_off, int16_t y_off, uint32_t target_crtc, uint32_t wait_fence, uint32_t idle_fence, uint32_t options, uint64_t target, uint64_t dividor, uint64_t rem, uint32_t len, void* notifies), c, win, pix, serial, valid, update, x_off, y_off, target_crtc, wait_fence, idle_fence, options, target, dividor, rem, len, notifies)
SUPER(xcb_present_pixmap, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint32_t pix, uint32_t serial, uint32_t valid, uint32_t update, int16_t x_off, int16_t y_off, uint32_t target_crtc, uint32_t wait_fence, uint32_t idle_fence, uint32_t options, uint64_t target, uint64_t dividor, uint64_t rem, uint32_t len, void* notifies), c, win, pix, serial, valid, update, x_off, y_off, target_crtc, wait_fence, idle_fence, options, target, dividor, rem, len, notifies)
SUPER(xcb_present_query_version, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t major, uint32_t minor), c, major, minor)
SUPER(xcb_present_select_input_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t eid, uint32_t w, uint32_t mask), c, eid, w, mask)

#undef SUPER


#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
