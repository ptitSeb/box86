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
#include "emu/x86emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box86context.h"
#include "emu/x86emu_private.h"
#include "myalign.h"

const char* libtiffName =
#ifdef ANDROID
    "libtiff.so"
#else
    "libtiff.so.5"
#endif
    ;
#define LIBNAME libtiff

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedlibtifftypes.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \


// TIFFReadWriteProc ...
#define GO(A)   \
static uintptr_t my_TIFFReadWriteProc_fct_##A = 0;                                          \
static int my_TIFFReadWriteProc_##A(void* a, void* b, int c)                                \
{                                                                                           \
    return (int)RunFunctionFmt(my_TIFFReadWriteProc_fct_##A, "ppi", a, b, c);   \
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
static uintptr_t my_TIFFSeekProc_fct_##A = 0;                                               \
static int64_t my_TIFFSeekProc_##A(void* a, int64_t b, int c)                               \
{                                                                                           \
    return (int64_t)RunFunctionFmt64(my_TIFFSeekProc_fct_##A, "pIi", a, b, c);  \
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
static uintptr_t my_TIFFCloseProc_fct_##A = 0;                                  \
static int my_TIFFCloseProc_##A(void* a)                                        \
{                                                                               \
    return (int)RunFunctionFmt(my_TIFFCloseProc_fct_##A, "p", a);   \
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
static uintptr_t my_TIFFSizeProc_fct_##A = 0;                                       \
static int64_t my_TIFFSizeProc_##A(void* a)                                         \
{                                                                                   \
    return (int64_t)RunFunctionFmt64(my_TIFFSizeProc_fct_##A, "p", a);  \
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
static uintptr_t my_TIFFMapFileProc_fct_##A = 0;                                        \
static int my_TIFFMapFileProc_##A(void* a, void* b, void* c)                            \
{                                                                                       \
    return (int)RunFunctionFmt(my_TIFFMapFileProc_fct_##A, "ppp", a, b, c); \
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
static uintptr_t my_TIFFUnmapFileProc_fct_##A = 0;                              \
static void my_TIFFUnmapFileProc_##A(void* a, void* b, int64_t c)               \
{                                                                               \
    RunFunctionFmt(my_TIFFUnmapFileProc_fct_##A, "ppI", a, b, c);   \
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
    return my->TIFFClientOpen(filename, mode, clientdata,
                            find_TIFFReadWriteProc_Fct(readproc), find_TIFFReadWriteProc_Fct(writeproc), find_TIFFSeekProc_Fct(seekproc),
                            find_TIFFCloseProc_Fct(closeproc), find_TIFFSizeProc_Fct(sizeproc), find_TIFFMapFileProc_Fct(mapproc),
                            find_TIFFUnmapFileProc_Fct(unmapproc) );
}

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"

