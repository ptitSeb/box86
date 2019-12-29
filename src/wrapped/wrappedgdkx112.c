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

const char* gdkx112Name = "libgdk-x11-2.0.so.0";
#define LIBNAME gdkx112

typedef void (*vFppp_t)(void*, void*, void*);

#define SUPER() \
    GO(gdk_event_handler_set, vFppp_t)

typedef struct gdkx112_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} gdkx112_my_t;

void* getGdkX112My(library_t* lib)
{
    gdkx112_my_t* my = (gdkx112_my_t*)calloc(1, sizeof(gdkx112_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeGdkX112My(void* lib)
{
    gdkx112_my_t *my = (gdkx112_my_t *)lib;
}

static void my_event_handler(void* event, void* data)
{
    x86emu_t *emu = (x86emu_t*)data;
    SetCallbackArg(emu, 0, event);
    RunCallback(emu);
}
static void my_event_notify(void* data)
{
    x86emu_t *emu = (x86emu_t*)data;
    uintptr_t f = (uintptr_t)GetCallbackArg(emu, 3);
    if(f) {
        SetCallbackArg(emu, 0, GetCallbackArg(emu, 1));
        SetCallbackNArg(emu, 1);
        SetCallbackAddress(emu, f);
        RunCallback(emu);
    }
    FreeCallback(emu);
}

EXPORT void my_gdk_event_handler_set(x86emu_t* emu, void* func, void* data, void* notify)
{
    library_t * lib = GetLib(emu->context->maplib, gdkx112Name);
    gdkx112_my_t *my = (gdkx112_my_t*)lib->priv.w.p2;

    if(!func)
        return my->gdk_event_handler_set(func, data, notify);

    x86emu_t* cb = AddCallback(emu, (uintptr_t)func, 2, NULL, data, NULL, notify);
    my->gdk_event_handler_set(my_event_handler, cb, my_event_notify);
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getGdkX112My(lib);        \
    lib->priv.w.needed = 3; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libgobject-2.0.so.0"); \
    lib->priv.w.neededlibs[1] = strdup("libgio-2.0.so.0");  \
    lib->priv.w.neededlibs[2] = strdup("libgdk_pixbuf-2.0.so.0");

#define CUSTOM_FINI \
    freeGdkX112My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"
