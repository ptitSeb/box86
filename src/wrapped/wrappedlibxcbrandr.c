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
    const char* libxcbrandrName = "libxcb-randr.so";
#else
    const char* libxcbrandrName = "libxcb-randr.so.0";
#endif

#define LIBNAME libxcbrandr

typedef struct my_xcb_cookie_s {
    uint32_t        data;
} my_xcb_cookie_t;

typedef struct my_xcb_iterator_s {
    void*   data;
    int     rem;
    int     index;
} my_xcb_iterator_t;

typedef my_xcb_cookie_t (*XFpu_t)   (void*, uint32_t);
typedef my_xcb_cookie_t (*XFppu_t)  (void*, void*, uint32_t);
typedef my_xcb_cookie_t (*XFpuW_t)  (void*, uint32_t, uint16_t);
typedef my_xcb_cookie_t (*XFpuu_t)  (void*, uint32_t, uint32_t);
typedef my_xcb_iterator_t (*YFp_t)  (void*);

#define SUPER() \
    GO(xcb_randr_get_crtc_info, XFppu_t)                            \
    GO(xcb_randr_get_crtc_info_unchecked, XFppu_t)                  \
    GO(xcb_randr_get_screen_resources_outputs, XFpu_t)              \
    GO(xcb_randr_get_output_primary, XFpu_t)                        \
    GO(xcb_randr_get_output_primary_unchecked, XFpu_t)              \
    GO(xcb_randr_query_version, XFpuu_t)                            \
    GO(xcb_randr_query_version_unchecked, XFpuu_t)                  \
    GO(xcb_randr_select_input, XFpuW_t)                             \
    GO(xcb_randr_select_input_checked, XFpuW_t)                     \
    GO(xcb_randr_get_screen_resources, XFpu_t)                      \
    GO(xcb_randr_get_screen_resources_unchecked, XFpu_t)            \
    GO(xcb_randr_get_screen_resources_current, XFpu_t)              \
    GO(xcb_randr_get_screen_resources_current_unchecked, XFpu_t)    \
    GO(xcb_randr_get_output_info, XFppu_t)                          \
    GO(xcb_randr_get_output_info_unchecked, XFppu_t)                \
    GO(xcb_randr_get_screen_resources_current_outputs_end, YFp_t)   \

#include "wrappercallback.h"

#define SUPER(F, P, ...)           \
    EXPORT void* my_##F P          \
    {                              \
        (void)emu;                 \
        *ret = my->F(__VA_ARGS__); \
        return ret;                \
    }

SUPER(xcb_randr_get_crtc_info, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, void* crtc, uint32_t stamp), c, crtc, stamp)
SUPER(xcb_randr_get_crtc_info_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, void* crtc, uint32_t stamp), c, crtc, stamp)
SUPER(xcb_randr_get_screen_resources_outputs, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win), c, win)
SUPER(xcb_randr_get_output_primary, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win), c, win)
SUPER(xcb_randr_get_output_primary_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win), c, win)
SUPER(xcb_randr_query_version, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t major, uint32_t minor), c, major, minor)
SUPER(xcb_randr_query_version_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t major, uint32_t minor), c, major, minor)
SUPER(xcb_randr_select_input, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint16_t enable), c, win, enable)
SUPER(xcb_randr_select_input_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win, uint16_t enable), c, win, enable)
SUPER(xcb_randr_get_screen_resources, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win), c, win)
SUPER(xcb_randr_get_screen_resources_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win), c, win)
SUPER(xcb_randr_get_screen_resources_current, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win), c, win)
SUPER(xcb_randr_get_screen_resources_current_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t win), c, win)
SUPER(xcb_randr_get_output_info, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, void* output, uint32_t stamp), c, output, stamp)
SUPER(xcb_randr_get_output_info_unchecked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, void* output, uint32_t stamp), c, output, stamp)
SUPER(xcb_randr_get_screen_resources_current_outputs_end, (x86emu_t* emu, my_xcb_iterator_t* ret, void* c), c)

#undef SUPER


#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
