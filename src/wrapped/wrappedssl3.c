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

const char* ssl3Name = "libssl3.so";
#define LIBNAME ssl3
static library_t *my_lib = NULL;

#include "generated/wrappedssl3types.h"

typedef struct ssl3_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} ssl3_my_t;

void* getSsl3My(library_t* lib)
{
    my_lib = lib;
    ssl3_my_t* my = (ssl3_my_t*)calloc(1, sizeof(ssl3_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}

void freeSsl3My(void* lib)
{
    //ssl3_my_t *my = (ssl3_my_t *)lib;
}

#undef SUPER

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// SSLBadCertHandler ...
#define GO(A)   \
static uintptr_t my_SSLBadCertHandler_fct_##A = 0;                          \
static int my_SSLBadCertHandler_##A(void* a, void* b)                       \
{                                                                           \
    return RunFunction(my_context, my_SSLBadCertHandler_fct_##A, 2, a, b);  \
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
static uintptr_t my_SSLAuthCertificate_fct_##A = 0;                                 \
static int my_SSLAuthCertificate_##A(void* a, void* b, int c, int d)                \
{                                                                                   \
    return RunFunction(my_context, my_SSLAuthCertificate_fct_##A, 4, a, b, c, d);   \
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
    ssl3_my_t* my = (ssl3_my_t*)my_lib->priv.w.p2;

    return my->SSL_BadCertHook(fd, find_SSLBadCertHandler_Fct(f), arg);
}

EXPORT int my_SSL_AuthCertificateHook(x86emu_t* emu, void* fd, void* f, void* arg)
{
    ssl3_my_t* my = (ssl3_my_t*)my_lib->priv.w.p2;

    return my->SSL_AuthCertificateHook(fd, find_SSLBadCertHandler_Fct(f), arg);
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getSsl3My(lib);

#define CUSTOM_FINI \
    freeSsl3My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

