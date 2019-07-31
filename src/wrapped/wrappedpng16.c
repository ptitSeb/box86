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

const char* png16Name = "libpng16.so.16";
#define LIBNAME png16

typedef void (*vFpp_t)(void*, void*);
typedef void (*vFppp_t)(void*, void*, void*);

typedef struct png16_my_s {
    vFppp_t     png_set_read_fn;
    vFpp_t      png_set_read_user_transform_fn;
    vFppp_t     png_destroy_read_struct;
    // functions
} png16_my_t;

void* getPng16My(library_t* lib)
{
    png16_my_t* my = (png16_my_t*)calloc(1, sizeof(png16_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(png_set_read_fn, vFppp_t)
    GO(png_set_read_user_transform_fn, vFpp_t)
    GO(png_destroy_read_struct, vFppp_t)
    #undef GO
    return my;
}

void freePng16My(void* lib)
{
    png16_my_t *my = (png16_my_t *)lib;
}

// TODO: remove those static and create a proprer map, png_ptr as key....
static x86emu_t *emu_userdataread = NULL;
static void* userread_pngptr = NULL;
static void my16_user_read_data(void* png_ptr, void* data, int32_t length)
{
    if(emu_userdataread) {
        SetCallbackArg(emu_userdataread, 0, png_ptr);
        SetCallbackArg(emu_userdataread, 1, data);
        SetCallbackArg(emu_userdataread, 2, (void*)length);
        RunCallback(emu_userdataread);
    }
}

EXPORT void my16_png_set_read_fn(x86emu_t *emu, void* png_ptr, void* io_ptr, void* read_data_fn)
{
    library_t * lib = GetLib(emu->context->maplib, png16Name);
    png16_my_t *my = (png16_my_t*)lib->priv.w.p2;

    if(emu_userdataread) {
        if(read_data_fn) {printf_log(LOG_NONE, "Warning, pn16 is using 2* png_set_read_fn without clearing\n");}
        FreeCallback(emu_userdataread);
        userread_pngptr = NULL;
        emu_userdataread = NULL;
    }

    if(read_data_fn) {
        userread_pngptr = png_ptr;
        emu_userdataread = AddCallback(emu, (uintptr_t)read_data_fn, 3, NULL, NULL, NULL, NULL);
        my->png_set_read_fn(png_ptr, io_ptr, my16_user_read_data);
    } else {
        my->png_set_read_fn(png_ptr, io_ptr, NULL);
    }
}

static x86emu_t *emu_userdatatransform = NULL;
static void* usertransform_pngptr = NULL;
static void my16_user_transform_fn(void* ptr, void* row_info, void* data)
{
    if(emu_userdatatransform) {
        SetCallbackArg(emu_userdatatransform, 0, ptr);
        SetCallbackArg(emu_userdatatransform, 1, row_info);
        SetCallbackArg(emu_userdatatransform, 2, data);
        RunCallback(emu_userdatatransform);
    }
}

EXPORT void my16_png_set_read_user_transform_fn(x86emu_t *emu, void* png_ptr, void* read_transform_fn)
{
    library_t * lib = GetLib(emu->context->maplib, png16Name);
    png16_my_t *my = (png16_my_t*)lib->priv.w.p2;

    if(emu_userdatatransform) {
        if(read_transform_fn) {printf_log(LOG_NONE, "Warning, pn16 is using 2* png_set_read_user_transform_fn without clearing\n");}
        FreeCallback(emu_userdatatransform);
        usertransform_pngptr = NULL;
        emu_userdatatransform = NULL;
    }

    if(read_transform_fn) {
        usertransform_pngptr = png_ptr;
        emu_userdatatransform = AddCallback(emu, (uintptr_t)read_transform_fn, 3, NULL, NULL, NULL, NULL);
        my->png_set_read_user_transform_fn(png_ptr, my16_user_transform_fn);
    } else {
        my->png_set_read_user_transform_fn(png_ptr, NULL);
    }
}

EXPORT void my16_png_destroy_read_struct(x86emu_t* emu, void* png_ptr_ptr, void* info_ptr_ptr, void* end_info_ptr_ptr)
{
    if(!png_ptr_ptr)
        return;

    library_t * lib = GetLib(emu->context->maplib, png16Name);
    png16_my_t *my = (png16_my_t*)lib->priv.w.p2;
    // clean up box86 stuff first
    void* ptr = *(void**)png_ptr_ptr;
    if(usertransform_pngptr == ptr) {
        usertransform_pngptr = NULL;
        FreeCallback(emu_userdatatransform);
        emu_userdatatransform = NULL;
    }
    if(userread_pngptr == ptr) {
        userread_pngptr = NULL;
        FreeCallback(emu_userdataread);
        emu_userdataread = NULL;
    }
    // ok, clean other stuffs now
    my->png_destroy_read_struct(png_ptr_ptr, info_ptr_ptr, end_info_ptr_ptr);
}

#define CUSTOM_INIT \
    lib->priv.w.altprefix=strdup("yes"); \
    lib->priv.w.p2 = getPng16My(lib); \
    lib->altmy = strdup("my16_");

#define CUSTOM_FINI \
    freePng16My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"
