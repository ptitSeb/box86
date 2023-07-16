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
#include "box86context.h"
#include "librarian.h"
#include "callback.h"

const char* lcms2Name =
#ifdef ANDROID
    "liblcms2.so"
#else
    "liblcms2.so.2"
#endif
    ;
#define LIBNAME lcms2

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedlcms2types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// cmsLogErrorHandlerFunction ...
#define GO(A)   \
static uintptr_t my_cmsLogErrorHandlerFunction_fct_##A = 0;                             \
static void my_cmsLogErrorHandlerFunction_##A(void* a, uint32_t b, void* c)             \
{                                                                                       \
    RunFunctionFmt(my_cmsLogErrorHandlerFunction_fct_##A, "pup", a, b, c);  \
}
SUPER()
#undef GO
static void* find_cmsLogErrorHandlerFunction_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_cmsLogErrorHandlerFunction_fct_##A == (uintptr_t)fct) return my_cmsLogErrorHandlerFunction_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cmsLogErrorHandlerFunction_fct_##A == 0) {my_cmsLogErrorHandlerFunction_fct_##A = (uintptr_t)fct; return my_cmsLogErrorHandlerFunction_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for liblcms2 cmsLogErrorHandlerFunction callback\n");
    return NULL;
}
#undef SUPER

EXPORT void my_cmsSetLogErrorHandler(x86emu_t* emu, void* f)
{
    (void)emu;
    my->cmsSetLogErrorHandler(find_cmsLogErrorHandlerFunction_Fct(f));
}

#define PRE_INIT    \
    if(box86_nogtk) \
        return -1;

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
