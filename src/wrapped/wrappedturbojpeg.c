#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <setjmp.h>

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

const char* turbojpegName = "libturbojpeg.so.0";
#define LIBNAME turbojpeg

/*
static library_t* my_lib = NULL;

#define SUPER() \

typedef struct turbojpeg_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} turbojpeg_my_t;

void* getTurboJpegMy(library_t* lib)
{
    turbojpeg_my_t* my = (turbojpeg_my_t*)calloc(1, sizeof(turbojpeg_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeTurboJpegMy(void* lib)
{
    //turbojpeg_my_t *my = (turbojpeg_my_t *)lib;
}


#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)

static x86emu_t* myturbo_jpegcb_emu = NULL;\
// error_exit
#define GO(A)   \
static uintptr_t my_error_exit_fct_##A = 0;   \
static void my_error_exit_##A(turbojpeg_common_struct_t* cinfo)        \
{                                                                   \
    uintptr_t oldip = myturbo_jpegcb_emu->ip.dword[0];                 \
    wrapCommonStruct(cinfo, 0);                                     \
    RunFunctionWithEmu(myturbo_jpegcb_emu, 1, my_error_exit_fct_##A, 1, cinfo);   \
    if(oldip==myturbo_jpegcb_emu->ip.dword[0])                         \
        unwrapCommonStruct(cinfo, 0);                               \
    else                                                            \
        if(is_jmpbuf) longjmp(&jmpbuf, 1);                          \
}
SUPER()
#undef GO
static void* finderror_exitFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_error_exit_fct_##A == (uintptr_t)fct) return my_error_exit_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_error_exit_fct_##A == 0) {my_error_exit_fct_##A = (uintptr_t)fct; return my_error_exit_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Jpeg error_exit callback\n");
    return NULL;
}
#undef SUPER
*/


#define CUSTOM_INIT \
    lib->altmy = strdup("myturbo_");

//    lib->priv.w.p2 = getTurboJpegMy(lib);
/*
#define CUSTOM_FINI \
    freeTurboJpegMy(lib->priv.w.p2);   \
    free(lib->priv.w.p2);           \
    my_lib = NULL;
*/

#include "wrappedlib_init.h"
