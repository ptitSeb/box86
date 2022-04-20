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
#include "myalign.h"

const char* udev1Name = "libudev.so.1";
#define LIBNAME udev1
static library_t *my_lib = NULL;

#include "generated/wrappedudev1types.h"

typedef struct udev1_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} udev1_my_t;

void* getUdev1My(library_t* lib)
{
    my_lib = lib;
    udev1_my_t* my = (udev1_my_t*)calloc(1, sizeof(udev1_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}

void freeUdev1My(void* lib)
{
    //udev1_my_t *my = (udev1_my_t *)lib;
}

#undef SUPER

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// log_fn ...
#define GO(A)   \
static uintptr_t my_log_fn_fct_##A = 0;                                                 \
static void my_log_fn_##A(void* a, int b, void* c, int d, void* e, void* f, void* va)   \
{                                                                                       \
    RunFunction(my_context, my_log_fn_fct_##A, 7, a, b, c, d, e, f, va);                \
}
SUPER()
#undef GO
static void* find_log_fn_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_log_fn_fct_##A == (uintptr_t)fct) return my_log_fn_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_log_fn_fct_##A == 0) {my_log_fn_fct_##A = (uintptr_t)fct; return my_log_fn_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for udev1 log_fn callback\n");
    return NULL;
}

#undef SUPER

EXPORT void my_udev_set_log_fn(x86emu_t* emu, void* udev, void* f)
{
    udev1_my_t* my = (udev1_my_t*)my_lib->priv.w.p2;

    my->udev_set_log_fn(udev, find_log_fn_Fct(f));
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getUdev1My(lib);

#define CUSTOM_FINI \
    freeUdev1My(lib->priv.w.p2); \
    free(lib->priv.w.p2);



#include "wrappedlib_init.h"

