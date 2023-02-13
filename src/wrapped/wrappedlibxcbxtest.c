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

const char* libxcbxtestName = "libxcb-xtest.so.0";
#define LIBNAME libxcbxtest

typedef struct my_xcb_cookie_s {
    uint32_t        data;
} my_xcb_cookie_t;

typedef my_xcb_cookie_t (*XFpCCuuwwC_t) (void*, uint8_t, uint8_t, uint32_t, uint32_t, int16_t, int16_t, uint8_t);

#define SUPER() \
    GO(xcb_test_fake_input, XFpCCuuwwC_t)           \
    GO(xcb_test_fake_input_checked, XFpCCuuwwC_t)   \


#include "wrappercallback.h"

#define SUPER(F, P, ...)           \
    EXPORT void* my_##F P          \
    {                              \
        (void)emu;                 \
        *ret = my->F(__VA_ARGS__); \
        return ret;                \
    }

SUPER(xcb_test_fake_input, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t type, uint8_t detail, uint32_t time, uint32_t win, int16_t x, int16_t y, uint8_t id), c, type, detail, time, win, x, y, id)
SUPER(xcb_test_fake_input_checked, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint8_t type, uint8_t detail, uint32_t time, uint32_t win, int16_t x, int16_t y, uint8_t id), c, type, detail, time, win, x, y, id)

#undef SUPER


#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
