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

const char* png12Name = "libpng12.so.0";
#define LIBNAME png12

typedef void (*vFpp_t)(void*, void*);
typedef void (*vFppp_t)(void*, void*, void*);

typedef struct png12_my_s {
    vFppp_t     png_set_write_fn;
    vFpp_t      png_destroy_write_struct;
    // functions
} png12_my_t;

void* getPng12My(library_t* lib)
{
    png12_my_t* my = (png12_my_t*)calloc(1, sizeof(png12_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(png_set_write_fn, vFppp_t)
    GO(png_destroy_write_struct, vFpp_t)
    #undef GO
    return my;
}

void freePng12My(void* lib)
{
    png12_my_t *my = (png12_my_t *)lib;
}

static x86emu_t *emu_userdatawrite = NULL;
static void* userwrite_pngptr = NULL;
static void my12_user_write_data(void* png_ptr, void* data, int32_t length)
{
    if(emu_userdatawrite) {
        SetCallbackArg(emu_userdatawrite, 0, png_ptr);
        SetCallbackArg(emu_userdatawrite, 1, data);
        SetCallbackArg(emu_userdatawrite, 2, (void*)length);
        RunCallback(emu_userdatawrite);
    }
}
static x86emu_t *emu_userdataflush = NULL;
static void* userflush_pngptr = NULL;
static void my12_user_flush_data(void* png_ptr)
{
    if(emu_userdataflush) {
        SetCallbackArg(emu_userdataflush, 0, png_ptr);
        RunCallback(emu_userdataflush);
    }
}

EXPORT void my12_png_set_write_fn(x86emu_t* emu, void* png_ptr, void* write_fn, void* flush_fn)
{
    library_t * lib = GetLib(emu->context->maplib, png12Name);
    png12_my_t *my = (png12_my_t*)lib->priv.w.p2;

    if(write_fn && emu_userdatawrite) {
        printf_log(LOG_NONE, "Warning, pn12 is using 2* png_set_write_fn without clearing\n");
        FreeCallback(emu_userdatawrite);
        emu_userdatawrite = NULL;
        userwrite_pngptr = NULL;
    }
    if(flush_fn && emu_userdataflush) {
        printf_log(LOG_NONE, "Warning, pn12 is using 2* png_set_write_fn without clearing\n");
        FreeCallback(emu_userdataflush);
        emu_userdataflush = NULL;
        userflush_pngptr = NULL;
    }

    if(write_fn) {
        userwrite_pngptr = png_ptr;
        emu_userdatawrite = AddCallback(emu, (uintptr_t)write_fn, 3, NULL, NULL, NULL, NULL);
    }
    if(flush_fn) {
        userflush_pngptr = png_ptr;
        emu_userdataflush = AddCallback(emu, (uintptr_t)flush_fn, 1, NULL, NULL, NULL, NULL);
    }

    my->png_set_write_fn(png_ptr, (write_fn)?my12_user_write_data:NULL, (flush_fn)?my12_user_flush_data:NULL);
}

EXPORT void my12_png_destroy_write_struct(x86emu_t* emu, void* png_ptr_ptr, void* info_ptr_ptr)
{
    if(!png_ptr_ptr)
        return;

    library_t * lib = GetLib(emu->context->maplib, png12Name);
    png12_my_t *my = (png12_my_t*)lib->priv.w.p2;
    // clean up box86 stuff first
    void* ptr = *(void**)png_ptr_ptr;
    if(userflush_pngptr == ptr) {
        userflush_pngptr = NULL;
        FreeCallback(emu_userdataflush);
        emu_userdataflush = NULL;
    }
    if(userwrite_pngptr == ptr) {
        userwrite_pngptr = NULL;
        FreeCallback(emu_userdatawrite);
        emu_userdatawrite = NULL;
    }
    // ok, clean other stuffs now
    my->png_destroy_write_struct(png_ptr_ptr, info_ptr_ptr);
}

// Maybe this is needed?
//#define CUSTOM_INIT     lib->priv.w.altprefix=strdup("yes");

#define CUSTOM_INIT \
    lib->priv.w.p2 = getPng12My(lib); \
    lib->altmy = strdup("my12_");

#define CUSTOM_FINI \
    freePng12My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"
