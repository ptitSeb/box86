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
#include "emu/x86emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box86context.h"
#include "emu/x86emu_private.h"

const char* libxtstName = "libXtst.so.6";
#define LIBNAME libxtst

typedef int (*iFpppp_t)(void*, void*, void*, void*);

#define SUPER() \
    GO(XRecordEnableContextAsync, iFpppp_t)

typedef struct libxtst_my_s {
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
    // functions
} libxtst_my_t;

void* getXtstMy(library_t* lib)
{
    libxtst_my_t* my = (libxtst_my_t*)calloc(1, sizeof(libxtst_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeXtstMy(void* lib)
{
    //libxtst_my_t *my = (libxtst_my_t *)lib;
}

uintptr_t fnc_XRecordInterceptProc = 0;
static void my_XRecordInterceptProc(void* closure, void *recorded_data)
{
    RunFunction(my_context, fnc_XRecordInterceptProc, 2, closure, recorded_data);
}
EXPORT int my_XRecordEnableContextAsync(x86emu_t* emu, void* display, void* context, void* cb, void* closure)
{
    library_t* lib = GetLib(my_context->maplib, libxtstName);
    libxtst_my_t* my = (libxtst_my_t*)lib->priv.w.p2;

    if(GetNativeFnc((uintptr_t)cb)) {
        return my->XRecordEnableContextAsync(display, context, GetNativeFnc((uintptr_t)cb), closure);
    }
    if(fnc_XRecordInterceptProc && cb) {
        printf_log(LOG_NONE, "Warning, XRecordInterceptProc called 2 times\n");
    }
    fnc_XRecordInterceptProc = (uintptr_t)cb;
    return my->XRecordEnableContextAsync(display, context, my_XRecordInterceptProc, closure);
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getXtstMy(lib);   \
    lib->priv.w.needed = 2; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libX11.so.6"); \
    lib->priv.w.neededlibs[1] = strdup("libXext.so.6");

#define CUSTOM_FINI \
    freeXtstMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

