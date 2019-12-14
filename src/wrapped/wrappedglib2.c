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
#include "myalign.h"

const char* glib2Name = "libglib-2.0.so.0";
#define LIBNAME glib2

typedef void  (*vFpp_t)(void*, void*);
typedef void* (*pFpp_t)(void*, void*);

#define SUPER() \
    GO(g_list_free_full, vFpp_t)    \
    GO(g_markup_vprintf_escaped, pFpp_t)

typedef struct glib2_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} glib2_my_t;

void* getGlib2My(library_t* lib)
{
    glib2_my_t* my = (glib2_my_t*)calloc(1, sizeof(glib2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
}
#undef SUPER

void freeGlib2My(void* lib)
{
    glib2_my_t *my = (glib2_my_t *)lib;
}

x86emu_t* my_free_full_emu = NULL;
static void my_free_full_cb(void* data)
{
    if(!my_free_full_emu)
        return;
    SetCallbackArg(my_free_full_emu, 0, data);
    RunCallback(my_free_full_emu);
}
EXPORT void my_g_list_free_full(x86emu_t* emu, void* list, void* free_func)
{
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    x86emu_t *old = my_free_full_emu;
    my_free_full_emu = AddSharedCallback(emu, (uintptr_t)free_func, 1, NULL, NULL, NULL, NULL);
    my->g_list_free_full(list, my_free_full_cb);
    FreeCallback(my_free_full_emu);
    my_free_full_emu = old;
}

EXPORT void* my_g_markup_printf_escaped(x86emu_t *emu, void* fmt, void* b) {
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, b, emu->scratch);
    return my->g_markup_vprintf_escaped(fmt, emu->scratch);
    #else
    // other platform don't need that
    return my->g_markup_vprintf_escaped((const char*)fmt, b);
    #endif
}

EXPORT void* my_g_markup_vprintf_escaped(x86emu_t *emu, void* fmt, void* b) {
    library_t * lib = GetLib(emu->context->maplib, glib2Name);
    glib2_my_t *my = (glib2_my_t*)lib->priv.w.p2;
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, *(uint32_t**)b, emu->scratch);
    return my->g_markup_vprintf_escaped(fmt, emu->scratch);
    #else
    // other platform don't need that
    return my->g_markup_vprintf_escaped(fmt, *(uint32_t**)b);
    #endif
}


#define CUSTOM_INIT \
    lib->priv.w.p2 = getGlib2My(lib);

#define CUSTOM_FINI \
    freeGlib2My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

