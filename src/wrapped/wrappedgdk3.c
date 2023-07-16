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
#include "gtkclass.h"

const char* gdk3Name = "libgdk-3.so.0";
#define LIBNAME gdk3

static char* libname = NULL;

#define ADDED_FUNCTIONS()

#include "generated/wrappedgdk3types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

// GdkFilterFunc
#define GO(A)   \
static uintptr_t my_filter_fct_##A = 0;                                                     \
static int my_filter_##A(void* xevent, void* event, void* data)                             \
{                                                                                           \
    return (int)RunFunctionFmt(my_filter_fct_##A, "ppp", xevent, event, data);  \
}
SUPER()
#undef GO
static void* findFilterFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_filter_fct_##A == (uintptr_t)fct) return my_filter_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_filter_fct_##A == 0) {my_filter_fct_##A = (uintptr_t)fct; return my_filter_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for gtk-2 GdkFilterFunc callback\n");
    return NULL;
}


static void my3_event_handler(void* event, my_signal_t* sig)
{
    RunFunctionFmt(sig->c_handler, "pp", event, sig->data);
}

EXPORT void my3_gdk_event_handler_set(x86emu_t* emu, void* func, void* data, void* notify)
{
    (void)emu;
    if(!func)
        return my->gdk_event_handler_set(func, data, notify);

    my_signal_t* sig = new_mysignal(func, data, notify);
    my->gdk_event_handler_set(my3_event_handler, sig, my_signal_delete);
}


static void my3_input_function(my_signal_t* sig, int source, int condition)
{
    RunFunctionFmt(sig->c_handler, "pii", sig->data, source, condition);
}

EXPORT int my3_gdk_input_add(x86emu_t* emu, int source, int condition, void* f, void* data)
{
    (void)emu;
    if(!f)
        return my->gdk_input_add_full(source, condition, f, data, NULL);

    my_signal_t* sig = new_mysignal(f, data, NULL);
    return my->gdk_input_add_full(source, condition, my3_input_function, sig, my_signal_delete);
}

EXPORT int my3_gdk_input_add_full(x86emu_t* emu, int source, int condition, void* f, void* data, void* notify)
{
    (void)emu;
    if(!f)
        return my->gdk_input_add_full(source, condition, f, data, notify);
    
    my_signal_t* sig = new_mysignal(f, data, notify);
    return my->gdk_input_add_full(source, condition, my3_input_function, sig, my_signal_delete);
}

EXPORT void my3_gdk_init(x86emu_t* emu, void* argc, void* argv)
{
    (void)emu;
    my->gdk_init(argc, argv);
    my_checkGlobalGdkDisplay();
}

EXPORT int my3_gdk_init_check(x86emu_t* emu, void* argc, void* argv)
{
    (void)emu;
    int ret = my->gdk_init_check(argc, argv);
    my_checkGlobalGdkDisplay();
    return ret;
}

EXPORT void my3_gdk_window_add_filter(x86emu_t* emu, void* window, void* f, void* data)
{
    (void)emu;
    my->gdk_window_add_filter(window, findFilterFct(f), data);
}

EXPORT void my3_gdk_window_remove_filter(x86emu_t* emu, void* window, void* f, void* data)
{
    (void)emu;
    my->gdk_window_remove_filter(window, findFilterFct(f), data);
}

#define PRE_INIT    \
    if(box86_nogtk) \
        return -1;

#define CUSTOM_INIT \
    libname = lib->name;            \
    SETALT(my3_);                   \
    getMy(lib);                     \
    setNeededLibs(lib, 3,           \
        "libgobject-2.0.so.0",      \
        "libgio-2.0.so.0",          \
        "libgdk_pixbuf-2.0.so.0");

#define CUSTOM_FINI \
    freeMy();


#include "wrappedlib_init.h"
