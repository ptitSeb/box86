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

const char* nspr4Name = "libnspr4.so";
#define LIBNAME nspr4

#define ADDED_FUNCTIONS()           \

#include "generated/wrappednspr4types.h"

#include "wrappercallback.h"

#undef SUPER

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// PRCallOnceWithArg ...
#define GO(A)   \
static uintptr_t my_PRCallOnceWithArg_fct_##A = 0;                                  \
static int my_PRCallOnceWithArg_##A(void* a)                                        \
{                                                                                   \
    return (int)RunFunctionFmt(my_PRCallOnceWithArg_fct_##A, "p", a);   \
}
SUPER()
#undef GO
static void* find_PRCallOnceWithArg_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_PRCallOnceWithArg_fct_##A == (uintptr_t)fct) return my_PRCallOnceWithArg_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_PRCallOnceWithArg_fct_##A == 0) {my_PRCallOnceWithArg_fct_##A = (uintptr_t)fct; return my_PRCallOnceWithArg_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for nspr4 PRCallOnceWithArg callback\n");
    return NULL;
}
// PRCallOnce ...
#define GO(A)   \
static uintptr_t my_PRCallOnce_fct_##A = 0;                             \
static int my_PRCallOnce_##A()                                          \
{                                                                       \
    return (int)RunFunctionFmt(my_PRCallOnce_fct_##A, "");  \
}
SUPER()
#undef GO
static void* find_PRCallOnce_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_PRCallOnce_fct_##A == (uintptr_t)fct) return my_PRCallOnce_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_PRCallOnce_fct_##A == 0) {my_PRCallOnce_fct_##A = (uintptr_t)fct; return my_PRCallOnce_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for nspr4 PRCallOnce callback\n");
    return NULL;
}

#undef SUPER

typedef struct my_PRLibrary_s {
    char*                       name;
    struct my_PRLibrary_s*      next;
    int                         refCount;
    void*                       staticTable;
    void*                       dlh;
} my_PRLibrary_t;

EXPORT int my_PR_CallOnceWithArg(x86emu_t* emu, void* once, void* f, void* arg)
{
    (void)emu;
    return my->PR_CallOnceWithArg(once, find_PRCallOnceWithArg_Fct(f), arg);
}

EXPORT int my_PR_CallOnce(x86emu_t* emu, void* once, void* f)
{
    (void)emu;
    return my->PR_CallOnce(once, find_PRCallOnce_Fct(f));
}

EXPORT void* my_PR_FindFunctionSymbol(x86emu_t* emu, void* symbol, void* name)
{
    (void)emu;
    //TODO!!!
    printf_log(LOG_NONE, "Error: using unimplemented PR_FindFunctionSymbol(%p, \"%s\")\n", symbol, (char*)name);
    return NULL;
}

EXPORT void* my_PR_CreateIOLayerStub(x86emu_t* emu, int ident, void* methods)
{
    (void)emu;
    //TODO!!!
    printf_log(LOG_NONE, "Error: using unimplemented PR_CreateIOLayerStub(%d, %p)\n", ident, methods);
    return NULL;
}

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
