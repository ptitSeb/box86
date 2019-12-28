#define _GNU_SOURCE
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

const char* gtkx112Name = "libgtk-x11-2.0.so.0";
#define LIBNAME gtkx112

typedef void          (*vFpp_t)(void*, void*);
typedef void*         (*pFppi_t)(void*, void*, int32_t);
typedef int32_t       (*iFppp_t)(void*, void*, void*);
typedef unsigned long (*LFppppppii_t)(void*, void*, void*, void*, void*, void*, int32_t, int32_t);

#define SUPER() \
    GO(gtk_signal_connect_full, LFppppppii_t)   \
    GO(gtk_dialog_add_button, pFppi_t)  \
    GO(gtk_message_dialog_format_secondary_text, vFpp_t)

typedef struct gtkx112_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} gtkx112_my_t;

void* getGtkx112My(library_t* lib)
{
    gtkx112_my_t* my = (gtkx112_my_t*)calloc(1, sizeof(gtkx112_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeGtkx112My(void* lib)
{
    gtkx112_my_t *my = (gtkx112_my_t *)lib;
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
        printf_log(LOG_NONE, "Warning, Gtk-x11-2 signal callback but no data found!");
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
static void signal_delete(void* a)
{
    x86emu_t* emu = (x86emu_t*)a;
    void* d = (void*)GetCallbackArg(emu, 9);
    if(d) {
        SetCallbackNArg(emu, 1);
        SetCallbackArg(emu, 0, GetCallbackArg(emu, 7));
        SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 9));
        RunCallback(emu);
    }
    FreeCallback(emu);
}
EXPORT uintptr_t my_gtk_signal_connect_full(x86emu_t* emu, void* object, void* name, void* c_handler, void* unsupported, void* data, void* closure, uint32_t signal, int after)
{
    library_t * lib = GetLib(emu->context->maplib, gtkx112Name);
    gtkx112_my_t *my = (gtkx112_my_t*)lib->priv.w.p2;

    if(!context)
        context = emu->context;

    x86emu_t *cb = AddSmallCallback(emu, (uintptr_t)c_handler, 0, NULL, NULL, NULL, NULL);
    SetCallbackArg(cb, 7, data);
    SetCallbackArg(cb, 8, c_handler);
    SetCallbackArg(cb, 9, closure);
    uintptr_t ret = my->gtk_signal_connect_full(object, name, signal_cb, NULL, cb, signal_delete, signal, after);
    return ret;
}

EXPORT void my_gtk_dialog_add_buttons(x86emu_t* emu, void* dialog, void* first, uintptr_t* b)
{
    library_t * lib = GetLib(emu->context->maplib, gtkx112Name);
    gtkx112_my_t *my = (gtkx112_my_t*)lib->priv.w.p2;

    void* btn = first;
    while(btn) {
        int id = (int)*(b++);
        my->gtk_dialog_add_button(dialog, btn, id);
        btn = (void*)*(b++);
    }
}

EXPORT void my_gtk_message_dialog_format_secondary_text(x86emu_t* emu, void* dialog, void* fmt, void* b)
{
    library_t * lib = GetLib(emu->context->maplib, gtkx112Name);
    gtkx112_my_t *my = (gtkx112_my_t*)lib->priv.w.p2;

    char* buf = NULL;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, emu->scratch);
    #else
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, b);
    #endif
    // pre-bake the fmt/vaarg, because there is no "va_list" version of this function
    my->gtk_message_dialog_format_secondary_text(dialog, buf);
    free(buf);
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getGtkx112My(lib); \
    lib->priv.w.needed = 1; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libgdk-x11-2.0.so.0");

#define CUSTOM_FINI \
    freeGtkx112My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"
