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
    const char* libxtName = "libXt.so";
#else
    const char* libxtName = "libXt.so.6";
#endif

#define LIBNAME libxt

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedlibxttypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \
GO(5)   \
GO(6)   \
GO(7)

#define GO(A)   \
static uintptr_t my_Event_fct_##A = 0;                                  \
static void my_Event_##A(void* w, void* data, void* event)              \
{                                                                       \
    RunFunctionFmt(my_Event_fct_##A, "ppp", w, data, event);\
}
SUPER()
#undef GO
static void* findEventFct(void* fct)
{
    if(!fct) return NULL;
    #define GO(A) if(my_Event_fct_##A == (uintptr_t)fct) return my_Event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Event_fct_##A == 0) {my_Event_fct_##A = (uintptr_t)fct; return my_Event_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libXt Event callback\n");
    return NULL;
}
#undef SUPER


EXPORT void my_XtAddEventHandler(x86emu_t* emu, void* w, uint32_t mask, int32_t maskable, void* cb, void* data)
{
    (void)emu;
    void* fct = findEventFct(cb);
    my->XtAddEventHandler(w, mask, maskable, fct, data);
}

#ifdef ANDROID
    #define CUSTOM_INIT \
        getMy(lib);   \
        setNeededLibs(lib, 2, "libX11.so", "libXext.so");
#else
    #define CUSTOM_INIT \
        getMy(lib);   \
        setNeededLibs(lib, 2, "libX11.so.6", "libXext.so.6");
#endif

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
