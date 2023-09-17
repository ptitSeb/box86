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

const char* ssl3Name = "libssl3.so";
#define LIBNAME ssl3

#include "generated/wrappedssl3types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// SSLBadCertHandler ...
#define GO(A)   \
static uintptr_t my_SSLBadCertHandler_fct_##A = 0;                              \
static int my_SSLBadCertHandler_##A(void* a, void* b)                           \
{                                                                               \
    return RunFunctionFmt(my_SSLBadCertHandler_fct_##A, "pp", a, b);\
}
SUPER()
#undef GO
static void* find_SSLBadCertHandler_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_SSLBadCertHandler_fct_##A == (uintptr_t)fct) return my_SSLBadCertHandler_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_SSLBadCertHandler_fct_##A == 0) {my_SSLBadCertHandler_fct_##A = (uintptr_t)fct; return my_SSLBadCertHandler_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for ssl3 SSLBadCertHandler callback\n");
    return NULL;
}

// SSLAuthCertificate ...
#define GO(A)   \
static uintptr_t my_SSLAuthCertificate_fct_##A = 0;                                         \
static int my_SSLAuthCertificate_##A(void* a, void* b, int c, int d)                        \
{                                                                                           \
    return RunFunctionFmt(my_SSLAuthCertificate_fct_##A, "ppii", a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_SSLAuthCertificate_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_SSLAuthCertificate_fct_##A == (uintptr_t)fct) return my_SSLAuthCertificate_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_SSLAuthCertificate_fct_##A == 0) {my_SSLAuthCertificate_fct_##A = (uintptr_t)fct; return my_SSLAuthCertificate_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for ssl3 SSLAuthCertificate callback\n");
    return NULL;
}

#undef SUPER

EXPORT int my_SSL_BadCertHook(x86emu_t* emu, void* fd, void* f, void* arg)
{
    (void)emu;
    return my->SSL_BadCertHook(fd, find_SSLBadCertHandler_Fct(f), arg);
}

EXPORT int my_SSL_AuthCertificateHook(x86emu_t* emu, void* fd, void* f, void* arg)
{
    (void)emu;
    return my->SSL_AuthCertificateHook(fd, find_SSLAuthCertificate_Fct(f), arg);
}

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
