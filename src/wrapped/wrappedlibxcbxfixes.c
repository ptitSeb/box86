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

const char* libxcbxfixesName = "libxcb-xfixes.so.0";
#define LIBNAME libxcbxfixes


#define SUPER() \


typedef struct xcbxfixes_my_s {
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
    // functions
} xcbxfixes_my_t;

void* getXcbxfixesMy(library_t* lib)
{
    xcbxfixes_my_t* my = (xcbxfixes_my_t*)calloc(1, sizeof(xcbxfixes_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeXcbxfixesMy(void* lib)
{
    //xcbxfixes_my_t *my = (xcbxfixes_my_t *)lib;
}

#define SUPER(F, P, ...)                                            \
    EXPORT void* my_##F P                                           \
    {                                                               \
        xcbxfixes_my_t *my = (xcbxfixes_my_t*)emu->context->libxcbxfixes->priv.w.p2;  \
        *ret = my->F(__VA_ARGS__);                                  \
        return ret;                                                 \
    }

//SUPER(xcb_change_gc, (x86emu_t* emu, my_xcb_cookie_t* ret, void* c, uint32_t gc, uint32_t mask, void* list), c, gc, mask, list)
#undef SUPER


#define CUSTOM_INIT \
    box86->libxcbxfixes = lib;                \
    lib->priv.w.p2 = getXcbxfixesMy(lib);

#define CUSTOM_FINI \
    freeXcbxfixesMy(lib->priv.w.p2);  \
    free(lib->priv.w.p2);       \
    ((box86context_t*)(lib->context))->libxcbxfixes = NULL;

#include "wrappedlib_init.h"
