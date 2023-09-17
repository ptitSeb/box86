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
#include "gtkclass.h"

const char* gconf2Name = "libgconf-2.so.4";
#define LIBNAME gconf2

#include "generated/wrappedgconf2types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// GFreeFunc ...
#define GO(A)   \
static uintptr_t my_GFreeFunc_fct_##A = 0;                      \
static void my_GFreeFunc_##A(void* a)                           \
{                                                               \
    RunFunctionFmt(my_GFreeFunc_fct_##A, "p", a);   \
}
SUPER()
#undef GO
static void* find_GFreeFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GFreeFunc_fct_##A == (uintptr_t)fct) return my_GFreeFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GFreeFunc_fct_##A == 0) {my_GFreeFunc_fct_##A = (uintptr_t)fct; return my_GFreeFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gconf2 GFreeFunc callback\n");
    return NULL;
}
// GConfClientNotifyFunc ...
#define GO(A)   \
static uintptr_t my_GConfClientNotifyFunc_fct_##A = 0;                                  \
static void my_GConfClientNotifyFunc_##A(void* a, uint32_t b, void* c, void* d)         \
{                                                                                       \
    RunFunctionFmt(my_GConfClientNotifyFunc_fct_##A, "pupp", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_GConfClientNotifyFunc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_GConfClientNotifyFunc_fct_##A == (uintptr_t)fct) return my_GConfClientNotifyFunc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_GConfClientNotifyFunc_fct_##A == 0) {my_GConfClientNotifyFunc_fct_##A = (uintptr_t)fct; return my_GConfClientNotifyFunc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gconf2 GConfClientNotifyFunc callback\n");
    return NULL;
}

#undef SUPER

EXPORT uint32_t my_gconf_client_notify_add(x86emu_t* emu, void* client, void* section, void* func, void* data, void* destroy, void* err)
{
    (void)emu;
    return my->gconf_client_notify_add(client, section, find_GConfClientNotifyFunc_Fct(func), data, find_GFreeFunc_Fct(destroy), err);
}

#define PRE_INIT    \
    if(box86_nogtk) \
        return -1;

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
