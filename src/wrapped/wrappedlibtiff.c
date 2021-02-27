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

const char* libtiffName = "libtiff.so.5";
#define LIBNAME libtiff
static library_t* my_lib = NULL;

typedef void*(*pFpppppppppp_t)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);

#define SUPER() \
    GO(TIFFClientOpen, pFpppppppppp_t)  \

typedef struct tiff_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} tiff_my_t;

void* getTiffMy(library_t* lib)
{
    tiff_my_t* my = (tiff_my_t*)calloc(1, sizeof(tiff_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeTiffMy(void* lib)
{
    //tiff_my_t *my = (tiff_my_t *)lib;
}

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \


// TIFFReadWriteProc ...
#define GO(A)   \
static uintptr_t my_TIFFReadWriteProc_fct_##A = 0;                                  \
static int my_TIFFReadWriteProc_##A(void* a, void* b, int c)                        \
{                                                                                   \
    return (int)RunFunction(my_context, my_TIFFReadWriteProc_fct_##A, 3, a, b, c);  \
}
SUPER()
#undef GO
static void* find_TIFFReadWriteProc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_TIFFReadWriteProc_fct_##A == (uintptr_t)fct) return my_TIFFReadWriteProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TIFFReadWriteProc_fct_##A == 0) {my_TIFFReadWriteProc_fct_##A = (uintptr_t)fct; return my_TIFFReadWriteProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libtiff TIFFReadWriteProc callback\n");
    return NULL;
}
// TIFFSeekProc ...
#define GO(A)   \
static uintptr_t my_TIFFSeekProc_fct_##A = 0;                                                                       \
static int64_t my_TIFFSeekProc_##A(void* a, int64_t b, int c)                                                       \
{                                                                                                                   \
    return (int64_t)RunFunction64(my_context, my_TIFFSeekProc_fct_##A, 4, a, (uint32_t)(b&0xffffffff), (uint32_t)(b>>32), c);  \
}
SUPER()
#undef GO
static void* find_TIFFSeekProc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_TIFFSeekProc_fct_##A == (uintptr_t)fct) return my_TIFFSeekProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TIFFSeekProc_fct_##A == 0) {my_TIFFSeekProc_fct_##A = (uintptr_t)fct; return my_TIFFSeekProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libtiff TIFFSeekProc callback\n");
    return NULL;
}
// TIFFCloseProc ...
#define GO(A)   \
static uintptr_t my_TIFFCloseProc_fct_##A = 0;                              \
static int my_TIFFCloseProc_##A(void* a)                                    \
{                                                                           \
    return (int)RunFunction(my_context, my_TIFFCloseProc_fct_##A, 1, a);    \
}
SUPER()
#undef GO
static void* find_TIFFCloseProc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_TIFFCloseProc_fct_##A == (uintptr_t)fct) return my_TIFFCloseProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TIFFCloseProc_fct_##A == 0) {my_TIFFCloseProc_fct_##A = (uintptr_t)fct; return my_TIFFCloseProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libtiff TIFFCloseProc callback\n");
    return NULL;
}
// TIFFSizeProc ...
#define GO(A)   \
static uintptr_t my_TIFFSizeProc_fct_##A = 0;                                   \
static int64_t my_TIFFSizeProc_##A(void* a)                                     \
{                                                                               \
    return (int64_t)RunFunction64(my_context, my_TIFFSizeProc_fct_##A, 1, a);   \
}
SUPER()
#undef GO
static void* find_TIFFSizeProc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_TIFFSizeProc_fct_##A == (uintptr_t)fct) return my_TIFFSizeProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TIFFSizeProc_fct_##A == 0) {my_TIFFSizeProc_fct_##A = (uintptr_t)fct; return my_TIFFSizeProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libtiff TIFFSizeProc callback\n");
    return NULL;
}
// TIFFMapFileProc ...
#define GO(A)   \
static uintptr_t my_TIFFMapFileProc_fct_##A = 0;                                    \
static int my_TIFFMapFileProc_##A(void* a, void* b, void* c)                        \
{                                                                                   \
    return (int)RunFunction(my_context, my_TIFFMapFileProc_fct_##A, 3, a, b, c);    \
}
SUPER()
#undef GO
static void* find_TIFFMapFileProc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_TIFFMapFileProc_fct_##A == (uintptr_t)fct) return my_TIFFMapFileProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TIFFMapFileProc_fct_##A == 0) {my_TIFFMapFileProc_fct_##A = (uintptr_t)fct; return my_TIFFMapFileProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libtiff TIFFMapFileProc callback\n");
    return NULL;
}
// TIFFUnmapFileProc ...
#define GO(A)   \
static uintptr_t my_TIFFUnmapFileProc_fct_##A = 0;                                                                  \
static void my_TIFFUnmapFileProc_##A(void* a, void* b, int64_t c)                                                   \
{                                                                                                                   \
    RunFunction(my_context, my_TIFFUnmapFileProc_fct_##A, 4, a, b, (uint32_t)(c&0xffffffff), (uint32_t)(c>>32));    \
}
SUPER()
#undef GO
static void* find_TIFFUnmapFileProc_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_TIFFUnmapFileProc_fct_##A == (uintptr_t)fct) return my_TIFFUnmapFileProc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TIFFUnmapFileProc_fct_##A == 0) {my_TIFFUnmapFileProc_fct_##A = (uintptr_t)fct; return my_TIFFUnmapFileProc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libtiff TIFFUnmapFileProc callback\n");
    return NULL;
}
#undef SUPER

EXPORT void* my_TIFFClientOpen(void* filename, void* mode, void* clientdata, void* readproc, void* writeproc, void* seekproc, void* closeproc, void* sizeproc, void* mapproc, void* unmapproc)
{
    tiff_my_t *my = (tiff_my_t*)my_lib->priv.w.p2;
    return my->TIFFClientOpen(filename, mode, clientdata,
                            find_TIFFReadWriteProc_Fct(readproc), find_TIFFReadWriteProc_Fct(writeproc), find_TIFFSeekProc_Fct(seekproc),
                            find_TIFFCloseProc_Fct(closeproc), find_TIFFSizeProc_Fct(sizeproc), find_TIFFMapFileProc_Fct(mapproc),
                            find_TIFFUnmapFileProc_Fct(unmapproc) );
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getTiffMy(lib); \
    my_lib = lib;

#define CUSTOM_FINI \
    freeTiffMy(lib->priv.w.p2);  \
    free(lib->priv.w.p2);       \
    my_lib = NULL;

#include "wrappedlib_init.h"

