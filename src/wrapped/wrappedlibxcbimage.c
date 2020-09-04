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

const char* libxcbimageName = "libxcb-image.so.0";
#define LIBNAME libxcbimage

typedef struct my_xcb_cookie_s {
    uint32_t        data;
} my_xcb_cookie_t;


#define SUPER() \


typedef struct xcbimage_my_s {
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
    // functions
} xcbimage_my_t;

void* getXcbimageMy(library_t* lib)
{
    xcbimage_my_t* my = (xcbimage_my_t*)calloc(1, sizeof(xcbimage_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeXcbimageMy(void* lib)
{
    //xcbimage_my_t *my = (xcbimage_my_t *)lib;
}

#define SUPER(F, P, ...)                                            \
    EXPORT void* my_##F P                                           \
    {                                                               \
        xcbimage_my_t *my = (xcbimage_my_t*)emu->context->libxcb->priv.w.p2;  \
        *ret = my->F(__VA_ARGS__);                                  \
        return ret;                                                 \
    }

//SUPER(xcb_change_gc, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t gc, uint32_t mask, void* list), c, gc, mask, list)

#undef SUPER


#define CUSTOM_INIT \
    box86->libxcbimage = lib;                \
    lib->priv.w.p2 = getXcbimageMy(lib);

#define CUSTOM_FINI \
    freeXcbimageMy(lib->priv.w.p2);  \
    free(lib->priv.w.p2);       \
    ((box86context_t*)(lib->context))->libxcbimage = NULL;

#include "wrappedlib_init.h"
