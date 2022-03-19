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
static library_t* my_lib = NULL;

#define ADDED_FUNCTIONS()           \

#include "generated/wrappednspr4types.h"

typedef struct nspr4_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} nspr4_my_t;

void* getNspr4My(library_t* lib)
{
    my_lib = lib;
    nspr4_my_t* my = (nspr4_my_t*)calloc(1, sizeof(nspr4_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}

void freeNspr4My(void* lib)
{
    //nspr4_my_t *my = (nspr4_my_t *)lib;
}

#undef SUPER

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// PRCallOnceWithArg ...
#define GO(A)   \
static uintptr_t my_PRCallOnceWithArg_fct_##A = 0;                              \
static int my_PRCallOnceWithArg_##A(void* a)                                    \
{                                                                               \
    return (int)RunFunction(my_context, my_PRCallOnceWithArg_fct_##A, 1, a);    \
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
    nspr4_my_t* my = (nspr4_my_t*)my_lib->priv.w.p2;

    return my->PR_CallOnceWithArg(once, find_PRCallOnceWithArg_Fct(f), arg);
}

EXPORT void* my_PR_FindFunctionSymbol(x86emu_t* emu, void* symbol, void* name)
{
    nspr4_my_t* my = (nspr4_my_t*)my_lib->priv.w.p2;
    //TODO!!!
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getNspr4My(lib);

#define CUSTOM_FINI \
    freeNspr4My(lib->priv.w.p2); \
    free(lib->priv.w.p2);


#include "wrappedlib_init.h"

