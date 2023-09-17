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
#include "callback.h"

const char* cairoName = "libcairo.so.2";
#define LIBNAME cairo

#include "generated/wrappedcairotypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// cairo_destroy
#define GO(A)   \
static uintptr_t my_cairo_destroy_fct_##A = 0;                          \
static void my_cairo_destroy_##A(void* data)                            \
{                                                                       \
    RunFunctionFmt(my_cairo_destroy_fct_##A, "p", data);    \
}
SUPER()
#undef GO
static void* find_cairo_destroy_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_cairo_destroy_fct_##A == (uintptr_t)fct) return my_cairo_destroy_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cairo_destroy_fct_##A == 0) {my_cairo_destroy_fct_##A = (uintptr_t)fct; return my_cairo_destroy_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for cairo cairo_destroy callback\n");
    return NULL;
}

#undef SUPER

#define SET_USERDATA(A)                                                             \
EXPORT int my_##A (x86emu_t* emu, void* cr, void* key, void* data, void* destroy)   \
{                                                                                   \
    (void)emu;                                                                      \
    return my->A(cr, key, data, find_cairo_destroy_Fct(destroy));                   \
}

SET_USERDATA(cairo_set_user_data)
SET_USERDATA(cairo_pattern_set_user_data)
SET_USERDATA(cairo_scaled_font_set_user_data)
SET_USERDATA(cairo_surface_set_user_data)
SET_USERDATA(cairo_font_face_set_user_data)

#undef SET_USERDATA

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
