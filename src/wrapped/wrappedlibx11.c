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
#include "callback.h"
#include "librarian.h"
#include "box86context.h"
#include "emu/x86emu_private.h"

const char* libx11Name = "libX11.so.6";
#define LIBNAME libx11

typedef int (*XErrorHandler)(void *, void *);
void* my_XSetErrorHandler(x86emu_t* t, XErrorHandler handler);
typedef int (*XIOErrorHandler)(void *);
void* my_XSetIOErrorHandler(x86emu_t* t, XIOErrorHandler handler);
void* my_XESetCloseDisplay(x86emu_t* emu, void* display, int32_t extension, void* handler);

EXPORT void* my_XESetWireToEvent(x86emu_t* emu, void* display, int32_t event_number, void* proc)
{
    printf_log(LOG_NONE, "BOX86: Error, called unimplemented XESetWireToEvent(%p, %d, %p)\n", display, event_number, proc);
    emu->quit = 1;
    return NULL;
}
EXPORT void* my_XESetEventToWire(x86emu_t* emu, void* display, int32_t event_number, void* proc)
{
    printf_log(LOG_NONE, "BOX86: Error, called unimplemented XESetEventToWire(%p, %d, %p)\n", display, event_number, proc);
    emu->quit = 1;
    return NULL;
}

typedef int(*EventHandler) (void*,void*,void*);
int32_t my_XIfEvent(x86emu_t* emu, void* d,void* ev, EventHandler h, void* arg);

typedef struct XImageSave_s {
    int   anyEmu;
    void* create;
    void* destroy;
    void* get;
    void* put;
    void* sub;
    void* add;
} XImageSave_t;

typedef struct ximage_s {
    void*(*create_image)(
            void*           /* display */,
            void*           /* visual */,
            uint32_t        /* depth */,
            int32_t         /* format */,
            int32_t         /* offset */,
            void*           /* data */,
            uint32_t        /* width */,
            uint32_t        /* height */,
            int32_t         /* bitmap_pad */,
            int32_t         /* bytes_per_line */);
    int32_t (*destroy_image)        (void*);
    uint32_t (*get_pixel)           (void*, int32_t, int32_t);
    int32_t (*put_pixel)            (void*, int32_t, int32_t, uint32_t);
    void*(*sub_image)    (void*, int32_t, int32_t, uint32_t, uint32_t); //sub_image return a new XImage that need bridging => custom wrapper
    int32_t (*add_pixel)            (void*, int32_t);
} ximage_t;

typedef struct _XImage {
    int32_t width, height;          /* size of image */
    int32_t xoffset;                /* number of pixels offset in X direction */
    int32_t format;                 /* XYBitmap, XYPixmap, ZPixmap */
    void*   data;                   /* pointer to image data */
    int32_t byte_order;             /* data byte order, LSBFirst, MSBFirst */
    int32_t bitmap_unit;            /* quant. of scanline 8, 16, 32 */
    int32_t bitmap_bit_order;       /* LSBFirst, MSBFirst */
    int32_t bitmap_pad;             /* 8, 16, 32 either XY or ZPixmap */
    int32_t depth;                  /* depth of image */
    int32_t bytes_per_line;         /* accelarator to next line */
    int32_t bits_per_pixel;         /* bits per pixel (ZPixmap) */
    uint32_t red_mask;              /* bits in z arrangment */
    uint32_t green_mask;
    uint32_t blue_mask;
    void*    obdata;                 /* hook for the object routines to hang on */
    ximage_t f;
} XImage;

typedef void (*vFp_t)(void*);
typedef void* (*pFp_t)(void*);
typedef void (*vFpp_t)(void*, void*);
typedef void* (*pFpp_t)(void*, void*);
typedef void* (*pFpip_t)(void*, int32_t, void*);
typedef int32_t (*iFp_t)(void*);
typedef int32_t (*iFpi_t)(void*, int32_t);
typedef int32_t (*iFppu_t)(void*, void*, uint32_t);
typedef int32_t (*iFpppp_t)(void*, void*, void*, void*);
typedef uint32_t (*uFpii_t)(void*, int32_t, int32_t);
typedef int32_t (*iFpiiu_t)(void*, int32_t, int32_t, uint32_t);
typedef void* (*pFppup_t)(void*, void*, uint32_t, void*);
typedef void* (*pFpiiuu_t)(void*, int32_t, int32_t, uint32_t, uint32_t);
typedef void* (*pFppiiuuui_t)(void*, void*, int32_t, int32_t, uint32_t, uint32_t, uint32_t, int32_t);
typedef void* (*pFppuiipuuii_t)(void*, void*, uint32_t, int32_t, int32_t, void*, uint32_t, uint32_t, int32_t, int32_t);
typedef void* (*pFppiiuuuipii_t)(void*, void*, int32_t, int32_t, uint32_t, uint32_t, uint32_t, int32_t, void*, int32_t, int32_t);
typedef int32_t (*iFppppiiiiuu_t)(void*, void*, void*, void*, int32_t, int32_t, int32_t, int32_t, uint32_t, uint32_t);

typedef struct x11_my_s {
    // functions
    pFp_t           XSetErrorHandler;
    pFp_t           XSetIOErrorHandler;
    pFpip_t         XESetError;
    pFpip_t         XESetCloseDisplay;
    iFpppp_t        XIfEvent;
    iFpppp_t        XCheckIfEvent;
    iFpppp_t        XPeekIfEvent;
    pFppuiipuuii_t  XCreateImage;
    iFp_t           XInitImage;
    pFppiiuuui_t    XGetImage;
    iFppppiiiiuu_t  XPutImage;
    pFppiiuuuipii_t XGetSubImage;
    vFp_t           XDestroyImage;
    vFpp_t          _XDeqAsyncHandler;
    #ifdef PANDORA
    pFpp_t          XLoadQueryFont;
    pFppup_t        XCreateGC;
    iFppu_t         XSetBackground;
    iFppu_t         XSetForeground;
    #endif
} x11_my_t;

void* getX11My(library_t* lib)
{
    x11_my_t* my = (x11_my_t*)calloc(1, sizeof(x11_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(XSetErrorHandler, pFp_t)
    GO(XSetIOErrorHandler, pFp_t)
    GO(XESetError, pFpip_t)
    GO(XESetCloseDisplay, pFpip_t)
    GO(XIfEvent, iFpppp_t)
    GO(XCheckIfEvent, iFpppp_t)
    GO(XPeekIfEvent, iFpppp_t)
    GO(XCreateImage, pFppuiipuuii_t)
    GO(XInitImage, iFp_t)
    GO(XGetImage, pFppiiuuui_t)
    GO(XPutImage, iFppppiiiiuu_t)
    GO(XGetSubImage, pFppiiuuuipii_t)
    GO(XDestroyImage, vFp_t)
    GO(_XDeqAsyncHandler, vFpp_t)
    #ifdef PANDORA
    GO(XLoadQueryFont, pFpp_t)
    GO(XCreateGC, pFppup_t)
    GO(XSetBackground, iFppu_t)
    GO(XSetForeground, iFppu_t)
    #endif
    #undef GO
    return my;
}

void freeX11My(void* lib)
{
    x11_my_t *my = (x11_my_t *)lib;
}


void* my_XCreateImage(x86emu_t* emu, void* disp, void* vis, uint32_t depth, int32_t fmt, int32_t off
                    , void* data, uint32_t w, uint32_t h, int32_t pad, int32_t bpl);

int32_t my_XInitImage(x86emu_t* emu, void* img);

void* my_XGetImage(x86emu_t* emu, void* disp, void* drawable, int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt);

int32_t my_XPutImage(x86emu_t* emu, void* disp, void* drawable, void* gc, void* image
                    , int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y
                    , uint32_t w, uint32_t h);

void* my_XGetSubImage(x86emu_t* emu, void* disp, void* drawable
                    , int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt
                    , void* image, int32_t dst_x, int32_t dst_y);

void my_XDestroyImage(x86emu_t* emu, void* image);

#ifdef PANDORA
void* my_XLoadQueryFont(x86emu_t* emu, void* d, void* name);
#endif

void* my_XVaCreateNestedList(int dummy, void* p);

#define CUSTOM_INIT \
    lib->priv.w.p2 = getX11My(lib);

#define CUSTOM_FINI \
    freeX11My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

static x86emu_t *errorhandlercb = NULL;
static x86emu_t *ioerrorhandlercb = NULL;
static x86emu_t *exterrorhandlercb = NULL;  // should be set per screen and per extension!
static x86emu_t *extclosedisplaycb = NULL;  // should be set per screen and per extension!
static int my_errorhandle_callback(void* display, void* errorevent)
{
    if(!errorhandlercb)
        return 0;
    SetCallbackArg(errorhandlercb, 0, display);
    SetCallbackArg(errorhandlercb, 1, errorevent);
    return (int)RunCallback(errorhandlercb);
}
static int my_ioerrorhandle_callback(void* display)
{
    if(!ioerrorhandlercb)
        return 0;
    SetCallbackArg(ioerrorhandlercb, 0, display);
    return (int)RunCallback(ioerrorhandlercb);
}
static int my_exterrorhandle_callback(void* display, void* err, void* codes, int* ret_code)
{
    if(!exterrorhandlercb)
        return 0;
    SetCallbackArg(exterrorhandlercb, 0, display);
    SetCallbackArg(exterrorhandlercb, 1, err);
    SetCallbackArg(exterrorhandlercb, 2, codes);
    SetCallbackArg(exterrorhandlercb, 3, ret_code);
    return (int)RunCallback(exterrorhandlercb);
}
static int my_closedisplay_callback(void* display, void* codes)
{
    if(!extclosedisplaycb)
        return 0;
    SetCallbackArg(extclosedisplaycb, 0, display);
    SetCallbackArg(extclosedisplaycb, 1, codes);
    return (int)RunCallback(extclosedisplaycb);
}

EXPORT void* my_XSetErrorHandler(x86emu_t* emu, XErrorHandler handler)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;
    x86emu_t *cb = NULL;
    void* ret = NULL;
    XErrorHandler old = NULL;
    if(handler) {
        if(GetNativeFnc((uintptr_t)handler)) {
            old = (XErrorHandler)my->XSetErrorHandler(GetNativeFnc((uintptr_t)handler));
        } else {
            cb = AddCallback(emu, (uintptr_t)handler, 2, NULL, NULL, NULL, NULL);
            old = (XErrorHandler)my->XSetErrorHandler(my_errorhandle_callback);
        }
    } else {
        old = (XErrorHandler)my->XSetErrorHandler(NULL);
    }
    if(old) {
        if(CheckBridged(lib->priv.w.bridge, old))
            ret = (void*)CheckBridged(lib->priv.w.bridge, old);
        else
            ret = (void*)AddBridge(lib->priv.w.bridge, iFpp, old, 0);
    }
    if(errorhandlercb) FreeCallback(errorhandlercb);
    errorhandlercb = cb;
    return ret;
}

EXPORT void* my_XSetIOErrorHandler(x86emu_t* emu, XIOErrorHandler handler)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;
    if(ioerrorhandlercb) { FreeCallback(ioerrorhandlercb); ioerrorhandlercb=NULL;}
    x86emu_t *cb = NULL;
    void* ret;
    XIOErrorHandler old = NULL;
    if(GetNativeFnc((uintptr_t)handler)) {
        old = (XIOErrorHandler)my->XSetIOErrorHandler(GetNativeFnc((uintptr_t)handler));
    } else {
        cb = AddCallback(emu, (uintptr_t)handler, 2, NULL, NULL, NULL, NULL);
        old = (XIOErrorHandler)my->XSetIOErrorHandler(my_ioerrorhandle_callback);
    }
    if(CheckBridged(lib->priv.w.bridge, old))
        ret = (void*)CheckBridged(lib->priv.w.bridge, old);
    else
        ret = (void*)AddBridge(lib->priv.w.bridge, iFp, old, 0);
    if(ioerrorhandlercb) FreeCallback(ioerrorhandlercb);
    ioerrorhandlercb = cb;
    return ret;
}

EXPORT void* my_XESetError(x86emu_t* emu, void* display, int32_t extension, void* handler)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;
    x86emu_t *cb = NULL;
    void* ret;
    void* old = NULL;
    if(GetNativeFnc((uintptr_t)handler)) {
        old = my->XESetError(display, extension, GetNativeFnc((uintptr_t)handler));
    } else {
        cb = AddCallback(emu, (uintptr_t)handler, 4, NULL, NULL, NULL, NULL);
        old = my->XESetError(display, extension, my_exterrorhandle_callback);
    }
    if(CheckBridged(lib->priv.w.bridge, old))
        ret = (void*)CheckBridged(lib->priv.w.bridge, old);
    else
        ret = (void*)AddBridge(lib->priv.w.bridge, iFpip, old, 0);
    if(exterrorhandlercb) FreeCallback(exterrorhandlercb);
    exterrorhandlercb = cb;
    return ret;
}

EXPORT void* my_XESetCloseDisplay(x86emu_t* emu, void* display, int32_t extension, void* handler)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;
    x86emu_t *cb = NULL;
    void* ret;
    void* old = NULL;
    if(GetNativeFnc((uintptr_t)handler)) {
        old = my->XESetCloseDisplay(display, extension, GetNativeFnc((uintptr_t)handler));
    } else {
        cb = AddCallback(emu, (uintptr_t)handler, 2, NULL, NULL, NULL, NULL);
        old = my->XESetCloseDisplay(display, extension, my_closedisplay_callback);
    }
    if(CheckBridged(lib->priv.w.bridge, old))
        ret = (void*)CheckBridged(lib->priv.w.bridge, old);
    else
        ret = (void*)AddBridge(lib->priv.w.bridge, iFpip, old, 0);
    if(extclosedisplaycb) FreeCallback(extclosedisplaycb);
    extclosedisplaycb = cb;
    return ret;
}

int32_t xifevent_callback(void* dpy, void *event, void* arg)
{
    x86emu_t *emu = (x86emu_t*)arg;
    SetCallbackArg(emu, 0, dpy);
    SetCallbackArg(emu, 1, event);
    return (int32_t)RunCallback(emu);
}

EXPORT int32_t my_XIfEvent(x86emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;
    x86emu_t *cb = NULL;
    cb = AddSharedCallback(emu, (uintptr_t)h, 3, NULL, NULL, arg, NULL);
    int32_t ret = my->XIfEvent(d, ev, xifevent_callback, (void*)cb);
    FreeCallback(cb);
    return ret;
}

EXPORT int32_t my_XCheckIfEvent(x86emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;
    x86emu_t *cb = NULL;
    cb = AddSharedCallback(emu, (uintptr_t)h, 3, NULL, NULL, arg, NULL);
    int32_t ret = my->XCheckIfEvent(d, ev, xifevent_callback, (void*)cb);
    FreeCallback(cb);
    return ret;
}

EXPORT int32_t my_XPeekIfEvent(x86emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;
    x86emu_t *cb = NULL;
    cb = AddSharedCallback(emu, (uintptr_t)h, 3, NULL, NULL, arg, NULL);
    int32_t ret = my->XPeekIfEvent(d, ev, xifevent_callback, (void*)cb);
    FreeCallback(cb);
    return ret;
}

void sub_image_wrapper(x86emu_t *emu, uintptr_t fnc);
typedef void* (*sub_image_wrapper_t)(void*, int32_t, int32_t, uint32_t, uint32_t);


void BridgeImageFunc(x86emu_t *emu, XImage *img)
{
    bridge_t* system = emu->context->system;

    #define GO(A, W) \
    fnc = CheckBridged(system, img->f.A); \
    if(!fnc) fnc = AddBridge(system, W, img->f.A, 0); \
    img->f.A = (W##_t)fnc;

    uintptr_t fnc;

    GO(create_image, pFppuiipuuii)
    GO(destroy_image, iFp)
    GO(get_pixel, uFpii)
    GO(put_pixel, iFpiiu)
    GO(sub_image, sub_image_wrapper)
    GO(add_pixel, iFpi)
    #undef GO
}

void UnbridgeImageFunc(x86emu_t *emu, XImage *img)
{
    bridge_t* system = emu->context->system;

    #define GO(A, W) \
    fnc = GetNativeFnc((uintptr_t)img->f.A); \
    if(fnc) \
        img->f.A = (W##_t)fnc;

    void* fnc;

    GO(create_image, pFppuiipuuii)
    GO(destroy_image, iFp)
    GO(get_pixel, uFpii)
    GO(put_pixel, iFpiiu)
    GO(sub_image, sub_image_wrapper)
    GO(add_pixel, iFpi)
    #undef GO
}

void sub_image_wrapper(x86emu_t *emu, uintptr_t fnc)
{
    pFpiiuu_t fn = (pFpiiuu_t)fnc; 
    void* img = fn(*(void**)(R_ESP + 4), *(int32_t*)(R_ESP + 8), *(int32_t*)(R_ESP + 12), *(uint32_t*)(R_ESP + 16), *(uint32_t*)(R_ESP + 20));
    BridgeImageFunc(emu, (XImage*)img);
    R_EAX=(uintptr_t)img;
}


EXPORT void* my_XCreateImage(x86emu_t* emu, void* disp, void* vis, uint32_t depth, int32_t fmt, int32_t off
                    , void* data, uint32_t w, uint32_t h, int32_t pad, int32_t bpl)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;

    XImage *img = my->XCreateImage(disp, vis, depth, fmt, off, data, w, h, pad, bpl);
    if(!img)
        return img;
    // bridge all access functions...
    BridgeImageFunc(emu, img);
    return img;
}

EXPORT int32_t my_XInitImage(x86emu_t* emu, void* img)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;

    int ret = my->XInitImage(img);
    // bridge all access functions...
    BridgeImageFunc(emu, img);
    return ret;
}

EXPORT void* my_XGetImage(x86emu_t* emu, void* disp, void* drawable, int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;

    XImage *img = my->XGetImage(disp, drawable, x, y, w, h, plane, fmt);
    if(!img)
        return img;
    // bridge all access functions...
    BridgeImageFunc(emu, img);
    return img;
}

EXPORT int32_t my_XPutImage(x86emu_t* emu, void* disp, void* drawable, void* gc, void* image
                    , int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y
                    , uint32_t w, uint32_t h)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;

    UnbridgeImageFunc(emu, (XImage*)image);
    int32_t r = my->XPutImage(disp, drawable, gc, image, src_x, src_y, dst_x, dst_y, w, h);
    // bridge all access functions...
    BridgeImageFunc(emu, (XImage*)image);
    return r;
}

EXPORT void* my_XGetSubImage(x86emu_t* emu, void* disp, void* drawable
                    , int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt
                    , void* image, int32_t dst_x, int32_t dst_y)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;

    UnbridgeImageFunc(emu, (XImage*)image);
    XImage *img = my->XGetSubImage(disp, drawable, x, y, w, h, plane, fmt, image, dst_x, dst_y);
    if(img)
        BridgeImageFunc(emu, img);

    BridgeImageFunc(emu, (XImage*)image);
    return img;
}

EXPORT void my_XDestroyImage(x86emu_t* emu, void* image)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;

    UnbridgeImageFunc(emu, (XImage*)image);
    my->XDestroyImage(image);
}

typedef struct xintasync_s {
    struct xintasync_s *next;
    int (*handler)(
                    void*,
                    void*,
                    void*,
                    int,
                    void*
                    );
    void* data;
} xintasync_t;

static int my_XInternalAsyncHandler(void* dpy, void* rep, void* buf, int len, void* data)
{
    if(!data)
        return 0;
    x86emu_t *emu = (x86emu_t*)data;
    SetCallbackArg(emu, 0, dpy);
    SetCallbackArg(emu, 1, rep);
    SetCallbackArg(emu, 2, buf);
    SetCallbackArg(emu, 3, (void*)len);
    // data is already se as 4th arg
    int ret = RunCallback(emu);
    return ret;
}

EXPORT void my__XDeqAsyncHandler(x86emu_t* emu, void* cb, void* data)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;

    if(!data) {
        my->_XDeqAsyncHandler(cb, data);
        return;
    }
    x86emu_t *cbemu = AddCallback(emu, (uintptr_t)cb, 5, NULL, NULL, NULL, NULL);
    SetCallbackArg(cbemu, 4, data);
    my->_XDeqAsyncHandler(my_XInternalAsyncHandler, cbemu);
}
#if 0
typedef struct my_XIMArg_s {
    char    *name;
    void    *value;
} my_XIMArg_t;
#define my_XNVaNestedList                       "XNVaNestedList"

EXPORT void* my_XVaCreateNestedList(int dummy, void* b)
{
    // need to create a similar function here...
    void* p = b;
    int n = 0;
    while(p++) ++n;
    void** ret = (void**)malloc(sizeof(void*)*n);
    p = b;
    n = 0;
    while(p++)
        ret[n++] = p;
    return ret;
}
#endif
#ifdef PANDORA
EXPORT void* my_XLoadQueryFont(x86emu_t* emu, void* d, void* name)
{
    // basic font substitution...
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;

    if(strcmp(name, "9x15")==0)
        return my->XLoadQueryFont(d, "6x13");
    if(strcmp(name, "9x15B")==0)
        return my->XLoadQueryFont(d, "6x13B");
    return my->XLoadQueryFont(d, name);
}
extern int x11color16;
static uint32_t recode32to16(uint32_t c)
{
    uint32_t r, g, b;
    r = (c>>16)&0xff;
    g = (c>>8)&0xff;
    b = (c>>0)&0xff;
    return ((r>>3)<<11) | ((g>>2)<<5) | ((b>>3));
}
EXPORT int32_t my_XSetBackground(x86emu_t* emu, void* d, void* gc, uint32_t c)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;
    if(x11color16)
        c = recode32to16(c);
    return my->XSetBackground(d, gc, c);
}
EXPORT int32_t my_XSetForeground(x86emu_t* emu, void* d, void* gc, uint32_t c)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;
    if(x11color16)
        c = recode32to16(c);
    return my->XSetForeground(d, gc, c);
}
typedef struct XGCValues_s {
	int32_t function;
	uint32_t plane_mask;
	uint32_t foreground;
	uint32_t background;
	int32_t line_width;
	int32_t line_style;
	int32_t cap_style;
	int32_t join_style;
	int32_t fill_style;
	int32_t fill_rule;
	int32_t arc_mode;
	void* tile;
	void* stipple;
	int32_t ts_x_origin;
	int32_t ts_y_origin;
	void* font;
	int32_t subwindow_mode;
	int graphics_exposures;
	int32_t clip_x_origin;
	int32_t clip_y_origin;
	void* clip_mask;
	int32_t dash_offset;
	char dashes;
} XGCValues_t;

EXPORT void* my_XCreateGC(x86emu_t *emu, void* disp, void* d, uint32_t v, void* vs)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;

    int setfore = 0;
    int setback = 0;
    uint32_t fore = 0; 
    uint32_t back = 0;
    XGCValues_t *values = (XGCValues_t*)vs;
    if(v&(1<<2)) {
        setfore = 1;
        fore = values->foreground;
    }
    if(v&(1<<3)) {
        setback = 1;
        back = values->background;
    }
    void* gc = my->XCreateGC(disp, d, v, vs);
    if(x11color16) {
    if(setfore)
        my->XSetForeground(disp, gc, recode32to16(fore));
    if(setback)
        my->XSetBackground(disp, gc, recode32to16(back));
    }
    return gc;
}
#endif
