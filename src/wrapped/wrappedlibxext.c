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

const char* libxextName = "libXext.so.6";
#define LIBNAME libxext

typedef struct _XImage XImage;
void BridgeImageFunc(x86emu_t *emu, XImage *img);
void UnbridgeImageFunc(x86emu_t *emu, XImage *img);
typedef int (*XextErrorHandler)(void *, void *, void*);

typedef void* (*pFp_t)(void*);
typedef int32_t (*iFpppiiu_t)(void*, void*, void*, int32_t, int32_t, uint32_t);
typedef void* (*pFppuippuu_t)(void*, void*, uint32_t, int32_t, void*, void*, uint32_t, uint32_t);
typedef int32_t (*iFppppiiiiuui_t)(void*, void*, void*, void*, int32_t, int32_t, int32_t, int32_t, uint32_t, uint32_t, int32_t);

typedef struct xext_my_s {
    // functions
    pFppuippuu_t        XShmCreateImage;
    iFpppiiu_t          XShmGetImage;
    iFppppiiiiuui_t     XShmPutImage;
    pFp_t               XSetExtensionErrorHandler;
} xext_my_t;

void* getXextMy(library_t* lib)
{
    xext_my_t* my = (xext_my_t*)calloc(1, sizeof(xext_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(XShmCreateImage, pFppuippuu_t)
    GO(XShmGetImage, iFpppiiu_t)
    GO(XShmPutImage, iFppppiiiiuui_t)
    GO(XSetExtensionErrorHandler, pFp_t)
    #undef GO
    return my;
}

void freeXextMy(void* lib)
{
    xext_my_t *my = (xext_my_t *)lib;
}

EXPORT void* my_XShmCreateImage(x86emu_t* emu, void* disp, void* vis, uint32_t depth, int32_t fmt
                    , void* data, void* shminfo, uint32_t w, uint32_t h)
{
    library_t * lib = GetLib(emu->context->maplib, libxextName);
    xext_my_t *my = (xext_my_t*)lib->priv.w.p2;

    XImage *img = my->XShmCreateImage(disp, vis, depth, fmt, data, shminfo, w, h);
    if(!img)
        return img;
    // bridge all access functions...
    BridgeImageFunc(emu, img);
    return img;
}

EXPORT int32_t my_XShmPutImage(x86emu_t* emu, void* disp, void* drawable, void* gc, void* image
                    , int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y
                    , uint32_t w, uint32_t h, int32_t sendevt)
{
    library_t * lib = GetLib(emu->context->maplib, libxextName);
    xext_my_t *my = (xext_my_t*)lib->priv.w.p2;

    UnbridgeImageFunc(emu, (XImage*)image);
    int32_t r = my->XShmPutImage(disp, drawable, gc, image, src_x, src_y, dst_x, dst_y, w, h, sendevt);
    // bridge all access functions...
    BridgeImageFunc(emu, (XImage*)image);
    return r;
}

EXPORT int32_t my_XShmGetImage(x86emu_t* emu, void* disp, void* drawable, void* image, int32_t x, int32_t y, uint32_t plane)
{
    library_t * lib = GetLib(emu->context->maplib, libxextName);
    xext_my_t *my = (xext_my_t*)lib->priv.w.p2;

    UnbridgeImageFunc(emu, (XImage*)image);
    int32_t r = my->XShmGetImage(disp, drawable, image, x, y, plane);
    // bridge all access functions...
    BridgeImageFunc(emu, (XImage*)image);
    return r;
}

static x86emu_t *exterrorhandlercb = NULL;
static int my_exterrorhandle_callback(void* display, void* ext_name, void* reason)
{
    if(!exterrorhandlercb)
        return 0;
    SetCallbackArg(exterrorhandlercb, 0, display);
    SetCallbackArg(exterrorhandlercb, 1, ext_name);
    SetCallbackArg(exterrorhandlercb, 1, reason);
    return (int)RunCallback(exterrorhandlercb);
}


EXPORT void* my_XSetExtensionErrorHandler(x86emu_t* emu, void* handler)
{
    library_t * lib = GetLib(emu->context->maplib, libxextName);
    xext_my_t *my = (xext_my_t*)lib->priv.w.p2;

    x86emu_t *cb = NULL;
    void* ret = NULL;
    XextErrorHandler old = NULL;
    if(handler) {
        if(GetNativeFnc((uintptr_t)handler)) {
            old = (XextErrorHandler)my->XSetExtensionErrorHandler(GetNativeFnc((uintptr_t)handler));
        } else {
            cb = AddCallback(emu, (uintptr_t)handler, 3, NULL, NULL, NULL, NULL);
            old = (XextErrorHandler)my->XSetExtensionErrorHandler(my_exterrorhandle_callback);
        }
    } else {
        old = (XextErrorHandler)my->XSetExtensionErrorHandler(NULL);
    }
    if(old) {
        if(CheckBridged(lib->priv.w.bridge, old))
            ret = (void*)CheckBridged(lib->priv.w.bridge, old);
        else
            ret = (void*)AddBridge(lib->priv.w.bridge, iFppp, old, 0);
    }
    if(exterrorhandlercb) FreeCallback(exterrorhandlercb);
    exterrorhandlercb = cb;
    return ret;
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getXextMy(lib); \
    lib->priv.w.needed = 4; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libX11.so.6"); \
    lib->priv.w.neededlibs[1] = strdup("libxcb.so.1"); \
    lib->priv.w.neededlibs[2] = strdup("libXau.so.6"); \
    lib->priv.w.neededlibs[3] = strdup("libXdmcp.so.6");

#define CUSTOM_FINI \
    freeXextMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

