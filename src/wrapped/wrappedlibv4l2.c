#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x86emu.h"

const char* libv4l2Name = "libv4l2.so.0";
#define LIBNAME libv4l2

#include "wrappedlib_init.h"

typedef int     (*iFpi_t)           (void*, int);
typedef int     (*iFi_t)            (int);
typedef int     (*iFiii_t)          (int, int, int);
typedef int     (*iFipu_t)          (int, void*, uint32_t);
typedef void*   (*pFpuiiiI_t)       (void*, uint32_t, int, int, int, int64_t);
typedef int     (*iFpu_t)           (void*, uint32_t);

static library_t* my_lib = NULL;

#define SUPER() \
    GO(v4l2_open, iFpi_t)       \
    GO(v4l2_close, iFi_t)       \
    GO(v4l2_ioctl, iFiii_t)     \
    GO(v4l2_read, iFipu_t)      \
    GO(v4l2_mmap, pFpuiiiI_t)   \
    GO(v4l2_munmap, iFpu_t)

typedef struct libv4l2_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} libv4l2_my_t;

void* getv4l2My(library_t* lib)
{
    libv4l2_my_t* my = (libv4l2_my_t*)calloc(1, sizeof(libv4l2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freev4l2My(void* lib)
{

}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getv4l2My(lib);    \
    my_lib = lib;

#define CUSTOM_FINI \
    freev4l2My(lib->priv.w.p2); \
    free(lib->priv.w.p2);       \
    my_lib = NULL;
