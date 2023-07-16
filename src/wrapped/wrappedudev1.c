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

#include "generated/wrappedudev1types.h"

#include "wrappercallback.h"

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
    RunFunctionFmt(my_log_fn_fct_##A, "pipippp", a, b, c, d, e, f, va);     \
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
    (void)emu;
    my->udev_set_log_fn(udev, find_log_fn_Fct(f));
}

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
