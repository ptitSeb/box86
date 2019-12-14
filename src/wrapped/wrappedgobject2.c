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

const char* gobject2Name = "libgobject-2.0.so.0";
#define LIBNAME gobject2

typedef unsigned long (*LFpppppu_t)(void*, void*, void*, void*, void*, uint32_t);

#define SUPER() \
    GO(g_signal_connect_data, LFpppppu_t)

typedef struct gobject2_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} gobject2_my_t;

void* getGobject2My(library_t* lib)
{
    gobject2_my_t* my = (gobject2_my_t*)calloc(1, sizeof(gobject2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeGobject2My(void* lib)
{
    gobject2_my_t *my = (gobject2_my_t *)lib;
}

static box86context_t* context = NULL;
static int signal_cb(void* a, void* b, void* c, void* d)
{
    // signal can have many signature... so first job is to find the data!
    // hopefully, no callback have more than 4 arguments...
    x86emu_t* emu = NULL;
    int i = 0;
    if(a)
        if(IsCallback(context, (x86emu_t*)a)) {
            emu = (x86emu_t*)a;
            i = 1;
        }
    if(!emu && b)
        if(IsCallback(context, (x86emu_t*)b)) {
            emu = (x86emu_t*)b;
            i = 2;
        }
    if(!emu && c)
        if(IsCallback(context, (x86emu_t*)c)) {
            emu = (x86emu_t*)c;
            i = 3;
        }
    if(!emu && d)
        if(IsCallback(context, (x86emu_t*)d)) {
            emu = (x86emu_t*)d;
            i = 4;
        }
    if(!i) {
        printf_log(LOG_NONE, "Warning, GObject2 signal callback but no data found!");
        return 0;
    }
    SetCallbackNArg(emu, i);
    if(i>1)
        SetCallbackArg(emu, 0, a);
    if(i>2)
        SetCallbackArg(emu, 1, b);
    if(i>3)
        SetCallbackArg(emu, 2, c);
    SetCallbackArg(emu, i-1, GetCallbackArg(emu, 7));
    SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 8));
    return RunCallback(emu);
}
static void signal_delete(void* a, void* b)
{
    x86emu_t* emu = (x86emu_t*)a;
    void* d = (void*)GetCallbackArg(emu, 9);
    if(d) {
        SetCallbackNArg(emu, 2);
        SetCallbackArg(emu, 0, GetCallbackArg(emu, 7));
        SetCallbackArg(emu, 1, b);
        SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 9));
        RunCallback(emu);
    }
    FreeCallback(emu);
}
EXPORT uintptr_t my_g_signal_connect_data(x86emu_t* emu, void* instance, void* detailed, void* c_handler, void* data, void* closure, uint32_t flags)
{
    library_t * lib = GetLib(emu->context->maplib, gobject2Name);
    gobject2_my_t *my = (gobject2_my_t*)lib->priv.w.p2;

    if(!context)
        context = emu->context;

    x86emu_t *cb = AddSmallCallback(emu, (uintptr_t)c_handler, 0, NULL, NULL, NULL, NULL);
    SetCallbackArg(cb, 7, data);
    SetCallbackArg(cb, 8, c_handler);
    SetCallbackArg(cb, 9, closure);
    uintptr_t ret = my->g_signal_connect_data(instance, detailed, signal_cb, cb, signal_delete, flags);
    return ret;
}


#define CUSTOM_INIT \
    lib->priv.w.p2 = getGobject2My(lib); \
    lib->priv.w.needed = 1; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libglib-2.0.so.0");

#define CUSTOM_FINI \
    freeGobject2My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

