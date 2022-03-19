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
#include "callback.h"
#include "librarian.h"
#include "box86context.h"
#include "emu/x86emu_private.h"

const char* libx11Name = "libX11.so.6";
#define LIBNAME libx11

extern int x11threads;
extern int x11glx;

typedef int (*XErrorHandler)(void *, void *);
void* my_XSetErrorHandler(x86emu_t* t, XErrorHandler handler);
typedef int (*XIOErrorHandler)(void *);
void* my_XSetIOErrorHandler(x86emu_t* t, XIOErrorHandler handler);
void* my_XESetCloseDisplay(x86emu_t* emu, void* display, int32_t extension, void* handler);
typedef int (*WireToEventProc)(void*, void*, void*);
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
typedef uint32_t (*uFv_t)(void);
typedef int32_t (*iFpl_t)(void*, intptr_t);
typedef uintptr_t (*LFpii_t)(void*, int32_t, int32_t);
typedef int32_t (*iFpiiL_t)(void*, int32_t, int32_t, uintptr_t);
typedef void* (*pFpiiuu_t)(void*, int32_t, int32_t, uint32_t, uint32_t);

#define ADDED_FUNCTIONS()       \
    GO(XInitThreads, uFv_t)     \

#include "generated/wrappedlibx11types.h"

typedef struct x11_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} x11_my_t;


void* getX11My(library_t* lib)
{
    x11_my_t* my = (x11_my_t*)calloc(1, sizeof(x11_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}

void freeX11My(void* lib)
{
    // x11_my_t *my = (x11_my_t *)lib;
}
#undef SUPER

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \
GO(5)   \
GO(6)   \
GO(7)   \
GO(8)   \
GO(9)   \
GO(10)  \
GO(11)  \
GO(12)  \
GO(13)  \
GO(14)  \
GO(15)  

// wire_to_event
#define GO(A)   \
static uintptr_t my_wire_to_event_fct_##A = 0;                      \
static int my_wire_to_event_##A(void* dpy, void* re, void* event)   \
{                                                                   \
    return (int)RunFunction(my_context, my_wire_to_event_fct_##A, 3, dpy, re, event);\
}
SUPER()
#undef GO
static void* findwire_to_eventFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_wire_to_event_fct_##A == (uintptr_t)fct) return my_wire_to_event_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_wire_to_event_fct_##A == 0) {my_wire_to_event_fct_##A = (uintptr_t)fct; return my_wire_to_event_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 wire_to_event callback\n");
    return NULL;
}
static void* reverse_wire_to_eventFct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->priv.w.bridge, fct))
        return (void*)CheckBridged(lib->priv.w.bridge, fct);
    #define GO(A) if(my_wire_to_event_##A == fct) return (void*)my_wire_to_event_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->priv.w.bridge, iFppp, fct, 0, NULL);
}

// event_to_wire
#define GO(A)   \
static uintptr_t my_event_to_wire_fct_##A = 0;                      \
static int my_event_to_wire_##A(void* dpy, void* re, void* event)   \
{                                                                   \
    return (int)RunFunction(my_context, my_event_to_wire_fct_##A, 3, dpy, re, event);\
}
SUPER()
#undef GO
static void* findevent_to_wireFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_event_to_wire_fct_##A == (uintptr_t)fct) return my_event_to_wire_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_event_to_wire_fct_##A == 0) {my_event_to_wire_fct_##A = (uintptr_t)fct; return my_event_to_wire_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 event_to_wire callback\n");
    return NULL;
}
static void* reverse_event_to_wireFct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->priv.w.bridge, fct))
        return (void*)CheckBridged(lib->priv.w.bridge, fct);
    #define GO(A) if(my_event_to_wire_##A == fct) return (void*)my_event_to_wire_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->priv.w.bridge, iFppp, fct, 0, NULL);
}

// error_handler
#define GO(A)   \
static uintptr_t my_error_handler_fct_##A = 0;                      \
static int my_error_handler_##A(void* dpy, void* error)   \
{                                                                   \
    return (int)RunFunction(my_context, my_error_handler_fct_##A, 2, dpy, error);\
}
SUPER()
#undef GO
static void* finderror_handlerFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_error_handler_fct_##A == (uintptr_t)fct) return my_error_handler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_error_handler_fct_##A == 0) {my_error_handler_fct_##A = (uintptr_t)fct; return my_error_handler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 error_handler callback\n");
    return NULL;
}
static void* reverse_error_handlerFct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->priv.w.bridge, fct))
        return (void*)CheckBridged(lib->priv.w.bridge, fct);
    #define GO(A) if(my_error_handler_##A == fct) return (void*)my_error_handler_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->priv.w.bridge, iFpp, fct, 0, NULL);
}

// ioerror_handler
#define GO(A)   \
static uintptr_t my_ioerror_handler_fct_##A = 0;                      \
static int my_ioerror_handler_##A(void* dpy)   \
{                                                                   \
    return (int)RunFunction(my_context, my_ioerror_handler_fct_##A, 1, dpy);\
}
SUPER()
#undef GO
static void* findioerror_handlerFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_ioerror_handler_fct_##A == (uintptr_t)fct) return my_ioerror_handler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ioerror_handler_fct_##A == 0) {my_ioerror_handler_fct_##A = (uintptr_t)fct; return my_ioerror_handler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 ioerror_handler callback\n");
    return NULL;
}
static void* reverse_ioerror_handlerFct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->priv.w.bridge, fct))
        return (void*)CheckBridged(lib->priv.w.bridge, fct);
    #define GO(A) if(my_ioerror_handler_##A == fct) return (void*)my_ioerror_handler_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->priv.w.bridge, iFp, fct, 0, NULL);
}

// exterror_handler
#define GO(A)   \
static uintptr_t my_exterror_handler_fct_##A = 0;                      \
static int my_exterror_handler_##A(void* dpy, void* err, void* codes, int* ret_code)   \
{                                                                   \
    return (int)RunFunction(my_context, my_exterror_handler_fct_##A, 4, dpy, err, codes, ret_code);\
}
SUPER()
#undef GO
static void* findexterror_handlerFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_exterror_handler_fct_##A == (uintptr_t)fct) return my_exterror_handler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_exterror_handler_fct_##A == 0) {my_exterror_handler_fct_##A = (uintptr_t)fct; return my_exterror_handler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 exterror_handler callback\n");
    return NULL;
}
static void* reverse_exterror_handlerFct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->priv.w.bridge, fct))
        return (void*)CheckBridged(lib->priv.w.bridge, fct);
    #define GO(A) if(my_exterror_handler_##A == fct) return (void*)my_exterror_handler_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->priv.w.bridge, iFpppp, fct, 0, NULL);
}

// close_display
#define GO(A)   \
static uintptr_t my_close_display_fct_##A = 0;                      \
static int my_close_display_##A(void* dpy, void* codes)   \
{                                                                   \
    return (int)RunFunction(my_context, my_close_display_fct_##A, 2, dpy, codes);\
}
SUPER()
#undef GO
static void* findclose_displayFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_close_display_fct_##A == (uintptr_t)fct) return my_close_display_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_close_display_fct_##A == 0) {my_close_display_fct_##A = (uintptr_t)fct; return my_close_display_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 close_display callback\n");
    return NULL;
}
static void* reverse_close_displayFct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->priv.w.bridge, fct))
        return (void*)CheckBridged(lib->priv.w.bridge, fct);
    #define GO(A) if(my_close_display_##A == fct) return (void*)my_close_display_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->priv.w.bridge, iFpp, fct, 0, NULL);
}

// register_im
#define GO(A)   \
static uintptr_t my_register_im_fct_##A = 0;                        \
static void my_register_im_##A(void* dpy, void* u, void* d)         \
{                                                                   \
    RunFunction(my_context, my_register_im_fct_##A, 3, dpy, u, d);  \
}
SUPER()
#undef GO
static void* findregister_imFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_register_im_fct_##A == (uintptr_t)fct) return my_register_im_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_register_im_fct_##A == 0) {my_register_im_fct_##A = (uintptr_t)fct; return my_register_im_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 register_im callback\n");
    return NULL;
}
static void* reverse_register_imFct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->priv.w.bridge, fct))
        return (void*)CheckBridged(lib->priv.w.bridge, fct);
    #define GO(A) if(my_register_im_##A == fct) return (void*)my_register_im_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->priv.w.bridge, iFppp, fct, 0, NULL);
}

// XConnectionWatchProc
#define GO(A)   \
static uintptr_t my_XConnectionWatchProc_fct_##A = 0;                               \
static void my_XConnectionWatchProc_##A(void* dpy, void* data, int op, void* d)     \
{                                                                                   \
    RunFunction(my_context, my_XConnectionWatchProc_fct_##A, 4, dpy, data, op, d);  \
}
SUPER()
#undef GO
static void* findXConnectionWatchProcFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_XConnectionWatchProc_fct_##A == (uintptr_t)fct) return my_XConnectionWatchProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XConnectionWatchProc_fct_##A == 0) {my_XConnectionWatchProc_fct_##A = (uintptr_t)fct; return my_XConnectionWatchProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XConnectionWatchProc callback\n");
    return NULL;
}
// xifevent
#define GO(A)   \
static uintptr_t my_xifevent_fct_##A = 0;                                   \
static int my_xifevent_##A(void* dpy, void* event, void* d)                 \
{                                                                           \
    return RunFunction(my_context, my_xifevent_fct_##A, 3, dpy, event, d);  \
}
SUPER()
#undef GO
static void* findxifeventFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_xifevent_fct_##A == (uintptr_t)fct) return my_xifevent_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_xifevent_fct_##A == 0) {my_xifevent_fct_##A = (uintptr_t)fct; return my_xifevent_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 xifevent callback\n");
    return NULL;
}
// XInternalAsyncHandler
#define GO(A)   \
static uintptr_t my_XInternalAsyncHandler_fct_##A = 0;                                              \
static int my_XInternalAsyncHandler_##A(void* dpy, void* rep, void* buf, int len, void* data)       \
{                                                                                                   \
    return RunFunction(my_context, my_XInternalAsyncHandler_fct_##A, 5, dpy, rep, buf, len, data);  \
}
SUPER()
#undef GO
static void* findXInternalAsyncHandlerFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_XInternalAsyncHandler_fct_##A == (uintptr_t)fct) return my_XInternalAsyncHandler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XInternalAsyncHandler_fct_##A == 0) {my_XInternalAsyncHandler_fct_##A = (uintptr_t)fct; return my_XInternalAsyncHandler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XInternalAsyncHandler callback\n");
    return NULL;
}

// XSynchronizeProc
#define GO(A)   \
static uintptr_t my_XSynchronizeProc_fct_##A = 0;                       \
static int my_XSynchronizeProc_##A()                                    \
{                                                                       \
    return (int)RunFunction(my_context, my_XSynchronizeProc_fct_##A, 0);\
}
SUPER()
#undef GO
static void* findXSynchronizeProcFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_XSynchronizeProc_fct_##A == (uintptr_t)fct) return my_XSynchronizeProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_XSynchronizeProc_fct_##A == 0) {my_XSynchronizeProc_fct_##A = (uintptr_t)fct; return my_XSynchronizeProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libX11 XSynchronizeProc callback\n");
    return NULL;
}
static void* reverse_XSynchronizeProcFct(library_t* lib, void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(lib->priv.w.bridge, fct))
        return (void*)CheckBridged(lib->priv.w.bridge, fct);
    #define GO(A) if(my_XSynchronizeProc_##A == fct) return (void*)my_XSynchronizeProc_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(lib->priv.w.bridge, iFppp, fct, 0, NULL);
}

#undef SUPER

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

typedef void (*XIMProc)(void*, void*, void*);
typedef int (*XICProc)(void*, void*, void*);
typedef struct {
    void* client_data;
    XIMProc callback;
} XIMCallback;

typedef struct {
    void* client_data;
    XICProc callback;
} XICCallback;

#define XNGeometryCallback "geometryCallback"
#define XNDestroyCallback "destroyCallback"
#define XNPreeditStartCallback "preeditStartCallback"
#define XNPreeditDoneCallback "preeditDoneCallback"
#define XNPreeditDrawCallback "preeditDrawCallback"
#define XNPreeditCaretCallback "preeditCaretCallback"
#define XNPreeditStateNotifyCallback "preeditStateNotifyCallback"
#define XNStatusStartCallback "statusStartCallback"
#define XNStatusDoneCallback "statusDoneCallback"
#define XNStatusDrawCallback "statusDrawCallback"
#define XNR6PreeditCallback "r6PreeditCallback"
#define XNStringConversionCallback "stringConversionCallback"

#define SUPER()                     \
GO(XNGeometryCallback)              \
GO(XNDestroyCallback)               \
GO(XNPreeditStartCallback)          \
GO(XNPreeditDoneCallback)           \
GO(XNPreeditDrawCallback)           \
GO(XNPreeditCaretCallback)          \
GO(XNPreeditStateNotifyCallback)    \
GO(XNStatusStartCallback)           \
GO(XNStatusDoneCallback)            \
GO(XNStatusDrawCallback)            \
GO(XNR6PreeditCallback)             \
GO(XNStringConversionCallback)

#define GO(A)                                                               \
static uintptr_t my_XICProc_fct_##A = 0;                                    \
static int my_XICProc_##A(void* a, void* b, void* c)                        \
{                                                                           \
    if (my_XICProc_fct_##A == 0)                                            \
        printf_log(LOG_NONE, "%s cannot find XICProc callback\n", __func__);\
    return (int)RunFunction(my_context, my_XICProc_fct_##A, 3, a, b, c);    \
}                                                                           \
static uintptr_t my_XIMProc_fct_##A = 0;                                    \
static void my_XIMProc_##A(void* a, void* b, void* c)                       \
{                                                                           \
    if (my_XIMProc_fct_##A == 0)                                            \
        printf_log(LOG_NONE, "%s cannot find XIMProc callback\n", __func__);\
    RunFunction(my_context, my_XIMProc_fct_##A, 3, a, b, c);                \
}
SUPER()
#undef GO

#define VA_CALL(FUNC, FIRST_ARG, VAARGS, VAARGSZ, RESULT)       \
switch (VAARGSZ)                                                \
{                                                               \
case 2:                                                         \
    RESULT = FUNC(FIRST_ARG, VAARGS[0], VAARGS[1], NULL);       \
    break;                                                      \
case 4:                                                         \
    RESULT = FUNC(FIRST_ARG, VAARGS[0], VAARGS[1], VAARGS[2], VAARGS[3], NULL);     \
    break;                                                                          \
case 6:                                                                             \
    RESULT = FUNC(FIRST_ARG, VAARGS[0], VAARGS[1], VAARGS[2], VAARGS[3], VAARGS[4], VAARGS[5], NULL);   \
    break;                                                                                              \
case 8:                                                                                                 \
    RESULT = FUNC(FIRST_ARG, VAARGS[0], VAARGS[1], VAARGS[2], VAARGS[3], VAARGS[4], VAARGS[5], VAARGS[6], VAARGS[7], NULL); \
    break;                                                                                                                  \
case 10:                                                                                                                    \
    RESULT = FUNC(FIRST_ARG, VAARGS[0], VAARGS[1], VAARGS[2], VAARGS[3], VAARGS[4], VAARGS[5], VAARGS[6], VAARGS[7], VAARGS[8], VAARGS[9], NULL);   \
    break;                                                                                                                                          \
case 12:                                                                                                                                            \
    RESULT = FUNC(FIRST_ARG, VAARGS[0], VAARGS[1], VAARGS[2], VAARGS[3], VAARGS[4], VAARGS[5], VAARGS[6], VAARGS[7], VAARGS[8], VAARGS[9],  VAARGS[10], VAARGS[11], NULL);  \
    break;                                                                                                                                                                  \
case 14:                                                                                                                                                                    \
    RESULT = FUNC(FIRST_ARG, VAARGS[0], VAARGS[1], VAARGS[2], VAARGS[3], VAARGS[4], VAARGS[5], VAARGS[6], VAARGS[7], VAARGS[8], VAARGS[9],  VAARGS[10], VAARGS[11], VAARGS[12], VAARGS[13], NULL);  \
    break;                                                                                                                                                                                          \
case 16:                                                                                                                                                                                            \
    RESULT = FUNC(FIRST_ARG, VAARGS[0], VAARGS[1], VAARGS[2], VAARGS[3], VAARGS[4], VAARGS[5], VAARGS[6], VAARGS[7], VAARGS[8], VAARGS[9],  VAARGS[10], VAARGS[11], VAARGS[12], VAARGS[13], VAARGS[14], VAARGS[15], NULL);  \
    break;                                                                                                                                                                                                                  \
case 18:                                                                                                                                                                                                                    \
    RESULT = FUNC(FIRST_ARG, VAARGS[0], VAARGS[1], VAARGS[2], VAARGS[3], VAARGS[4], VAARGS[5], VAARGS[6], VAARGS[7], VAARGS[8], VAARGS[9],  VAARGS[10], VAARGS[11], VAARGS[12], VAARGS[13], VAARGS[14], VAARGS[15], VAARGS[16], VAARGS[17], NULL);  \
    break;                                                                                                                                                                                                                                          \
case 20:                                                                                                                                                                                                                                            \
    RESULT = FUNC(FIRST_ARG, VAARGS[0], VAARGS[1], VAARGS[2], VAARGS[3], VAARGS[4], VAARGS[5], VAARGS[6], VAARGS[7], VAARGS[8], VAARGS[9],  VAARGS[10], VAARGS[11], VAARGS[12], VAARGS[13], VAARGS[14], VAARGS[15], VAARGS[16], VAARGS[17], VAARGS[18], VAARGS[19], NULL);  \
    break;                                                                                                                                                                                                                                                                  \
default:                                                                                                                \
    printf_log(LOG_NONE, "warning: %s's vasize (%d) is too large, need create new call case!\n", __func__, VAARGSZ);    \
    break;                                                                                                              \
}

#define GO(A)                                                                       \
if (va[i] && strcmp((char*)va[i], A) == 0) {                                        \
    XICCallback* origin = (XICCallback*)va[i+1];                                    \
    XICCallback* new = callbacks + i;                                               \
    new->client_data = origin->client_data;                                         \
    my_XICProc_fct_##A = (uintptr_t)origin->callback;                               \
    new->callback = my_XICProc_##A;                                                 \
    new_va[i+1] = new;                                                              \
}

EXPORT void* my_XVaCreateNestedList(x86emu_t* emu, int unused, void** va) {
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    int n = 0;
    while (va[n]) n+=2;
    void** new_va = malloc(sizeof(void*) * n);
    XICCallback* callbacks = (XICCallback*)malloc(sizeof(XIMCallback) * n);

    for (int i = 0; i < n; i += 2) {
        new_va[i] = va[i];
        new_va[i+1] = va[i+1];
        SUPER()
    }

    void* res;
    VA_CALL(my->XVaCreateNestedList, unused, new_va, n, res);
    free(new_va);
    free(callbacks);
    return res;
}

EXPORT void* my_XCreateIC(x86emu_t* emu, void* xim, void** va) {
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    int n = 0;
    while (va[n]) n+=2;
    void** new_va = malloc(sizeof(void*) * n);
    XICCallback* callbacks = (XICCallback*)malloc(sizeof(XIMCallback) * n);

    for (int i = 0; i < n; i += 2) {
        new_va[i] = va[i];
        new_va[i+1] = va[i+1];
        SUPER()
    }

    void* res;
    VA_CALL(my->XCreateIC, xim, new_va, n, res);
    free(new_va);
    free(callbacks);
    return res;
}

EXPORT void* my_XSetICValues(x86emu_t* emu, void* xic, void** va) {
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    int n = 0;
    while (va[n]) n+=2;
    void** new_va = malloc(sizeof(void*) * n);
    XICCallback* callbacks = (XICCallback*)malloc(sizeof(XIMCallback) * n);

    for (int i = 0; i < n; i += 2) {
        new_va[i] = va[i];
        new_va[i+1] = va[i+1];
        SUPER()
    }

    void* res; 
    VA_CALL(my->XSetICValues, xic, new_va, n, res);
    free(new_va);
    free(callbacks);
    return res;
}
#undef GO

EXPORT void* my_XSetIMValues(x86emu_t* emu, void* xim, void** va) {
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    int n = 0;
    while (va[n]) n+=2;
    void** new_va = malloc(sizeof(void*) * n);
    XIMCallback* callbacks = (XIMCallback*)malloc(sizeof(XIMCallback) * n);

    #define GO(A)                                                                       \
    if (va[i] && strcmp((char*)va[i], A) == 0) {                                        \
        XIMCallback* origin = (XIMCallback*)va[i+1];                                    \
        XIMCallback* new = callbacks + i;                                               \
        new->client_data = origin->client_data;                                         \
        my_XIMProc_fct_##A = (uintptr_t)origin->callback;                               \
        new->callback = my_XIMProc_##A;                                                 \
        new_va[i+1] = new;                                                              \
    }
    for (int i = 0; i < n; i += 2) {
        new_va[i] = va[i];
        new_va[i+1] = va[i+1];
        SUPER()
    }
    #undef GO
    
    void* res;
    VA_CALL(my->XSetIMValues, xim, new_va, n, res)
    free(new_va);
    free(callbacks);
    return res;
}
#undef VA_CALL
#undef SUPER

EXPORT void* my_XSetErrorHandler(x86emu_t* emu, XErrorHandler handler)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    void* ret = my->XSetErrorHandler(finderror_handlerFct(handler));
    return reverse_error_handlerFct(lib, ret);
}

EXPORT void* my_XSetIOErrorHandler(x86emu_t* emu, XIOErrorHandler handler)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    void* ret = my->XSetIOErrorHandler(findioerror_handlerFct(handler));
    return reverse_ioerror_handlerFct(lib, ret);
}

EXPORT void* my_XESetError(x86emu_t* emu, void* display, int32_t extension, void* handler)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    void* ret = my->XESetError(display, extension, findexterror_handlerFct(handler));
    return reverse_exterror_handlerFct(lib, ret);
}

EXPORT void* my_XESetCloseDisplay(x86emu_t* emu, void* display, int32_t extension, void* handler)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    void* ret = my->XESetCloseDisplay(display, extension, findclose_displayFct(handler));
    return reverse_close_displayFct(lib, ret);
}

EXPORT int32_t my_XIfEvent(x86emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    int32_t ret = my->XIfEvent(d, ev, findxifeventFct(h), arg);
    return ret;
}

EXPORT int32_t my_XCheckIfEvent(x86emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    int32_t ret = my->XCheckIfEvent(d, ev, findxifeventFct(h), arg);
    return ret;
}

EXPORT int32_t my_XPeekIfEvent(x86emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    int32_t ret = my->XPeekIfEvent(d, ev, findxifeventFct(h), arg);
    return ret;
}

void sub_image_wrapper(x86emu_t *emu, uintptr_t fnc);
typedef void* (*sub_image_wrapper_t)(void*, int32_t, int32_t, uint32_t, uint32_t);


void BridgeImageFunc(x86emu_t *emu, XImage *img)
{
    bridge_t* system = emu->context->system;

    #define GO(A, W) \
    fnc = CheckBridged(system, img->f.A); \
    if(!fnc) fnc = AddAutomaticBridge(emu, system, W, img->f.A, 0); \
    img->f.A = (W##_t)fnc;

    uintptr_t fnc;

    GO(create_image, pFppuiipuuii)
    GO(destroy_image, iFp)
    GO(get_pixel, LFpii)
    GO(put_pixel, iFpiiL)
    GO(sub_image, sub_image_wrapper)
    GO(add_pixel, iFpl)
    #undef GO
}

void UnbridgeImageFunc(x86emu_t *emu, XImage *img)
{
    #define GO(A, W) \
    fnc = GetNativeFnc((uintptr_t)img->f.A); \
    if(fnc) \
        img->f.A = (W##_t)fnc;

    void* fnc;

    GO(create_image, pFppuiipuuii)
    GO(destroy_image, iFp)
    GO(get_pixel, LFpii)
    GO(put_pixel, iFpiiL)
    GO(sub_image, sub_image_wrapper)
    GO(add_pixel, iFpl)
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
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    XImage *img = my->XCreateImage(disp, vis, depth, fmt, off, data, w, h, pad, bpl);
    if(!img)
        return img;
    // bridge all access functions...
    BridgeImageFunc(emu, img);
    return img;
}

EXPORT int32_t my_XInitImage(x86emu_t* emu, void* img)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    int ret = my->XInitImage(img);
    // bridge all access functions...
    BridgeImageFunc(emu, img);
    return ret;
}

EXPORT void* my_XGetImage(x86emu_t* emu, void* disp, void* drawable, int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

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
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
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
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    UnbridgeImageFunc(emu, (XImage*)image);
    XImage *img = my->XGetSubImage(disp, drawable, x, y, w, h, plane, fmt, image, dst_x, dst_y);
    if(img)
        BridgeImageFunc(emu, img);

    BridgeImageFunc(emu, (XImage*)image);
    return img;
}

EXPORT void my_XDestroyImage(x86emu_t* emu, void* image)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

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

EXPORT void my__XDeqAsyncHandler(x86emu_t* emu, void* cb, void* data)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    my->_XDeqAsyncHandler(findXInternalAsyncHandlerFct(cb), data);
}

#ifdef PANDORA
EXPORT void* my_XLoadQueryFont(x86emu_t* emu, void* d, void* name)
{
    // basic font substitution...
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

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
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    if(x11color16)
        c = recode32to16(c);
    return my->XSetBackground(d, gc, c);
}
EXPORT int32_t my_XSetForeground(x86emu_t* emu, void* d, void* gc, uint32_t c)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
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
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

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


EXPORT void* my_XESetWireToEvent(x86emu_t* emu, void* display, int32_t event_number, void* proc)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    void* ret = NULL;

    ret = my->XESetWireToEvent(display, event_number, findwire_to_eventFct(proc));

    return reverse_wire_to_eventFct(lib, ret);
}
EXPORT void* my_XESetEventToWire(x86emu_t* emu, void* display, int32_t event_number, void* proc)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    void* ret = NULL;

    ret = my->XESetEventToWire(display, event_number, findevent_to_wireFct(proc));

    return reverse_event_to_wireFct(lib, ret);
}

EXPORT int my_XCloseDisplay(x86emu_t* emu, void* display)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    int ret = my->XCloseDisplay(display);
    return ret;
}

EXPORT int my_XRegisterIMInstantiateCallback(x86emu_t* emu, void* d, void* db, void* res_name, void* res_class, void* cb, void* data)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    return my->XRegisterIMInstantiateCallback(d, db, res_name, res_class, findregister_imFct(cb), data);
}
    
EXPORT int my_XUnregisterIMInstantiateCallback(x86emu_t* emu, void* d, void* db, void* res_name, void* res_class, void* cb, void* data)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    return my->XUnregisterIMInstantiateCallback(d, db, res_name, res_class, reverse_register_imFct(lib, cb), data);
}

EXPORT int my_XQueryExtension(x86emu_t* emu, void* display, char* name, int* major, int* first_event, int* first_error)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    int ret = my->XQueryExtension(display, name, major, first_event, first_error);
    if(!ret && name && !strcmp(name, "GLX") && x11glx) {
        // hack to force GLX to be accepted, even if not present
        // left major and first_XXX to default...
        ret = 1;
    }
    return ret;
}

EXPORT int my_XAddConnectionWatch(x86emu_t* emu, void* display, char* f, void* data)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    return my->XAddConnectionWatch(display, findXConnectionWatchProcFct(f), data);
}

EXPORT int my_XRemoveConnectionWatch(x86emu_t* emu, void* display, char* f, void* data)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    return my->XRemoveConnectionWatch(display, findXConnectionWatchProcFct(f), data);
}

EXPORT void* my_XSetAfterFunction(x86emu_t* emu, void* display, void* f)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    return reverse_XSynchronizeProcFct(lib, my->XSetAfterFunction(display, findXSynchronizeProcFct(f)));
}

EXPORT void* my_XSynchronize(x86emu_t* emu, void* display, int onoff)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;

    return reverse_XSynchronizeProcFct(lib, my->XSynchronize(display, onoff));
}

typedef unsigned long XID;
struct my_XFreeFuncs {
    void* atoms;
    void* modifiermap;
    void* key_bindings;
    void* context_db;
    void* defaultCCCs;
    void* clientCmaps;
    void* intensityMaps;
    void* im_filters;
    void* xkb;
};

struct my_XExten {
        struct my_XExten *next;
        void* codes;    // XExtCodes
        void* create_GC;    // CreateGCType
        void* copy_GC;  // CopyGCType
        void* flush_GC; // FlushGCType
        void* free_GC;  // FreeGCType
        void* create_Font;  // CreateFontType
        void* free_Font;    // FreeFontType
        void* close_display;    // CloseDisplayType
        void* error;    // ErrorType
        void* error_string; // ErrorStringType
        char *name;
        void* error_values; // PrintErrorType
        void* before_flush; // BeforeFlushType
        struct my_XExten *next_flush;
};

struct my_XInternalAsync {
    struct my_XInternalAsync *next;
    int (*handler)(void*, void*, char*, int, void*);
    void* data;
};

struct my_XLockPtrs {
    void (*lock_display)(void* dpy);
    void (*unlock_display)(void *dpy);
};

struct my_XConnectionInfo {
    int fd;
    void* read_callback;    // _XInternalConnectionProc
    void* call_data;
    void* *watch_data;
    struct my_XConnectionInfo *next;
};

struct my_XConnWatchInfo {
    void* fn;   // XConnectionWatchProc
    void* client_data;
    struct _XConnWatchInfo *next;
};

typedef struct my_XDisplay_s
{
        void *ext_data;
        struct my_XFreeFuncs *free_funcs;
        int fd;
        int conn_checker;
        int proto_major_version;
        int proto_minor_version;
        char *vendor;
        XID resource_base;
        XID resource_mask;
        XID resource_id;
        int resource_shift;
        XID (*resource_alloc)(void*);
        int byte_order;
        int bitmap_unit;
        int bitmap_pad;
        int bitmap_bit_order;
        int nformats;
        void *pixmap_format;
        int vnumber;
        int release;
        void *head, *tail;
        int qlen;
        unsigned long last_request_read;
        unsigned long request;
        char *last_req;
        char *buffer;
        char *bufptr;
        char *bufmax;
        unsigned max_request_size;
        void* *db;
        int (*synchandler)(void*);
        char *display_name;
        int default_screen;
        int nscreens;
        void *screens;
        unsigned long motion_buffer;
        volatile unsigned long flags;
        int min_keycode;
        int max_keycode;
        void *keysyms;
        void *modifiermap;
        int keysyms_per_keycode;
        char *xdefaults;
        char *scratch_buffer;
        unsigned long scratch_length;
        int ext_number;
        struct my_XExten *ext_procs;
        int (*event_vec[128])(void *, void *, void *);
        int (*wire_vec[128])(void *, void *, void *);
        XID lock_meaning;
        void* lock;
        struct my_XInternalAsync *async_handlers;
        unsigned long bigreq_size;
        struct my_XLockPtrs *lock_fns;
        void (*idlist_alloc)(void *, void *, int);
        void* key_bindings;
        XID cursor_font;
        void* *atoms;
        unsigned int mode_switch;
        unsigned int num_lock;
        void* context_db;
        int (**error_vec)(void*, void*, void*);
        struct {
           void* defaultCCCs;
           void* clientCmaps;
           void* perVisualIntensityMaps;
        } cms;
        void* im_filters;
        void* qfree;
        unsigned long next_event_serial_num;
        struct my_XExten *flushes;
        struct my_XConnectionInfo *im_fd_info;
        int im_fd_length;
        struct my_XConnWatchInfo *conn_watchers;
        int watcher_count;
        void* filedes;
        int (*savedsynchandler)(void *);
        XID resource_max;
        int xcmisc_opcode;
        void* *xkb_info;
        void* *trans_conn;
        void* *xcb;
        unsigned int next_cookie;
        int (*generic_event_vec[128])(void*, void*, void*);
        int (*generic_event_copy_vec[128])(void*, void*, void*);
        void *cookiejar;
        void* error_threads;
        void* exit_handler;
        void* exit_handler_data;
} my_XDisplay_t;

EXPORT void* my_XOpenDisplay(x86emu_t* emu, void* d)
{
    library_t* lib = emu->context->x11lib;
    x11_my_t *my = (x11_my_t *)lib->priv.w.p2;
    void* ret = my->XOpenDisplay(d);
    // Added automatic bridge because of thos macro from Xlibint.h
    //#define LockDisplay(d)       if ((d)->lock_fns) (*(d)->lock_fns->lock_display)(d)
    //#define UnlockDisplay(d)     if ((d)->lock_fns) (*(d)->lock_fns->unlock_display)(d)

    my_XDisplay_t* dpy = (my_XDisplay_t*)ret;
    if(!ret)
        return ret;

    bridge_t* system = emu->context->system;

    #define GO(A, W)\
    if(dpy->A)      \
        if(!CheckBridged(system, dpy->A)) \
            AddAutomaticBridge(emu, system, W, dpy->A, 0); \

    #define GO2(A, B, W) \
    if(dpy->A && dpy->A->B)  \
        if(!CheckBridged(system, dpy->A->B)) \
            AddAutomaticBridge(emu, system, W, dpy->A->B, 0); \


    GO2(free_funcs, atoms, vFp)
    GO2(free_funcs, modifiermap, iFp)
    GO2(free_funcs, key_bindings, vFp)
    GO2(free_funcs, context_db, vFp)
    GO2(free_funcs, defaultCCCs, vFp)
    GO2(free_funcs, clientCmaps, vFp)
    GO2(free_funcs, intensityMaps, vFp)
    GO2(free_funcs, im_filters, vFp)
    GO2(free_funcs, xkb, vFp)
    GO(resource_alloc, LFp)
    GO(synchandler, iFp)
    //TODO: ext_procs?
    //TODO: event_vec?
    //TODO: wire_vec?
    //TODO: async_handlers?
    GO2(lock_fns, lock_display, vFp);
    GO2(lock_fns, unlock_display, vFp);
    GO(idlist_alloc, vFppi)
    //TODO: error_vec?
    //TODO: flushes
    //TODO: im_fd_info?
    //TODO: conn_watchers
    GO(savedsynchandler, iFp)
    //TODO: generic_event_vec?
    //TODO: generic_event_copy_vec?


    #undef GO
    #undef GO2

    return ret;
}

#define CUSTOM_INIT                 \
    box86->x11lib = lib;            \
    lib->priv.w.p2 = getX11My(lib); \
    lib->priv.w.needed = 1; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libdl.so.2"); \
    if(x11threads) ((x11_my_t*)lib->priv.w.p2)->XInitThreads();

#define CUSTOM_FINI \
    freeX11My(lib->priv.w.p2); \
    free(lib->priv.w.p2);   \
    ((box86context_t*)(lib->context))->x11lib = NULL; \

#include "wrappedlib_init.h"
