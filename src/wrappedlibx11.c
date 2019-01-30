#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "library_private.h"
#include "x86emu.h"
#include "x86emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box86context.h"
#include "x86emu_private.h"

typedef int (*XErrorHandler)(void *, void *);
XErrorHandler my_XSetErrorHandler(x86emu_t* t, XErrorHandler handler);

typedef int(*EventHandler) (void*,void*,void*);
int32_t my_XIfEvent(x86emu_t* emu, void* d,void* ev, EventHandler h, void* arg);

const char* libx11Name = "libX11.so.6";
#define LIBNAME libx11

#include "wrappedlib_init.h"

static x86emu_t *errorhandlercb = NULL;
static int my_errorhandle_callback(void* display, void* errorevent)
{
    if(!errorhandlercb)
        return 0;
    SetCallbackArg(errorhandlercb, 0, display);
    SetCallbackArg(errorhandlercb, 1, errorevent);
    return (int)RunCallback(errorhandlercb);
}

typedef void* (*pFp_t)(void*);
XErrorHandler EXPORT my_XSetErrorHandler(x86emu_t* emu, XErrorHandler handler)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    if(errorhandlercb) { FreeCallback(errorhandlercb); errorhandlercb=NULL;}
    x86emu_t *cb = NULL;
    if (handler!=NULL)
        cb = AddCallback(emu, (uintptr_t)handler, 2, NULL, NULL, NULL, NULL);
    errorhandlercb = cb;
    pFp_t fnc = (pFp_t)dlsym(lib->priv.w.lib, "XSetErrorHandler");
    XErrorHandler old = (XErrorHandler)fnc(cb);
    return (old)?((XErrorHandler)AddBridge(lib->priv.w.bridge, iFpp, old)):NULL;
}

int32_t xifevent_callback(void* dpy, void *event, void* arg)
{
    x86emu_t *emu = (x86emu_t*)arg;
    SetCallbackArg(emu, 0, dpy);
    SetCallbackArg(emu, 1, event);
    return (int32_t)RunCallback(emu);
}

typedef int32_t (*iFpppp_t)(void*, void*, void*, void*);
int32_t EXPORT my_XIfEvent(x86emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x86emu_t *cb = NULL;
    cb = AddCallback(emu, (uintptr_t)h, 3, NULL, NULL, arg, NULL);
    iFpppp_t fnc = (iFpppp_t)dlsym(lib->priv.w.lib, "XIfEvent");
    int32_t ret = fnc(d, ev, xifevent_callback, (void*)cb);
    FreeCallback(cb);
    return ret;
}