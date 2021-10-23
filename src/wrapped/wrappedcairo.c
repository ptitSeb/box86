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
#include "callback.h"

static library_t* my_lib = NULL;

const char* cairoName = "libcairo.so.2";
#define LIBNAME cairo

#include "generated/wrappedcairotypes.h"

typedef struct cairo_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} cairo_my_t;

void* getCairoMy(library_t* lib)
{
    my_lib = lib;
    cairo_my_t* my = (cairo_my_t*)calloc(1, sizeof(cairo_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeCairoMy(void* lib)
{
    //cairo_my_t *my = (cairo_my_t *)lib;
    my_lib = NULL;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// cairo_destroy
#define GO(A)   \
static uintptr_t my_cairo_destroy_fct_##A = 0;                  \
static void my_cairo_destroy_##A(void* data)                    \
{                                                               \
    RunFunction(my_context, my_cairo_destroy_fct_##A, 1, data); \
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

EXPORT int my_cairo_set_user_data(x86emu_t* emu, void* cr, void* key, void* data, void* destroy)
{
    cairo_my_t *my = (cairo_my_t*)my_lib->priv.w.p2;

    return my->cairo_set_user_data(cr, key, data, find_cairo_destroy_Fct(destroy));
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getCairoMy(lib);

#define CUSTOM_FINI \
    freeCairoMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

