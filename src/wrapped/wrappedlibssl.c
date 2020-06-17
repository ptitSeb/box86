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

const char* libsslName = "libssl.so.1.0.0";
#define LIBNAME libssl
#define ALTNAME "libssl.so.1.0.2"

static library_t* my_lib = NULL;

typedef void    (*vFpp_t)       (void*, void*);
typedef void    (*vFpip_t)      (void*, int, void*);
typedef long    (*lFpip_t)      (void*, int, void*);

#define SUPER() \
    GO(SSL_CTX_set_default_passwd_cb, vFpp_t)   \
    GO(SSL_CTX_callback_ctrl, lFpip_t)          \
    GO(SSL_callback_ctrl, lFpip_t)              \
    GO(SSL_CTX_set_verify, vFpip_t)             \
    GO(SSL_set_verify, vFpip_t)

typedef struct libssl_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} libssl_my_t;

void* getSllMy(library_t* lib)
{
    libssl_my_t* my = (libssl_my_t*)calloc(1, sizeof(libssl_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeSllMy(void* lib)
{
    //libssl_my_t *my = (libssl_my_t *)lib;
}

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// pem_passwd_cb
#define GO(A)   \
static uintptr_t my_pem_passwd_cb_fct_##A = 0;                                                      \
static int my_pem_passwd_cb_##A(void* buf, int size, int rwflag, void* password)                    \
{                                                                                                   \
    return (int)RunFunction(my_context, my_pem_passwd_cb_fct_##A, 4, buf, size, rwflag, password);  \
}
SUPER()
#undef GO
static void* find_pem_passwd_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_pem_passwd_cb_fct_##A == (uintptr_t)fct) return my_pem_passwd_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_pem_passwd_cb_fct_##A == 0) {my_pem_passwd_cb_fct_##A = (uintptr_t)fct; return my_pem_passwd_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSM pem_passwd_cb callback\n");
    return NULL;
}

// anonymous
#define GO(A)   \
static uintptr_t my_anonymous_fct_##A = 0;                                      \
static void* my_anonymous_##A(void* a, void* b, void* c, void *d)               \
{                                                                               \
    return (void*)RunFunction(my_context, my_anonymous_fct_##A, 4, a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_anonymous_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_anonymous_fct_##A == (uintptr_t)fct) return my_anonymous_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_anonymous_fct_##A == 0) {my_anonymous_fct_##A = (uintptr_t)fct; return my_anonymous_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSM anonymous callback\n");
    return NULL;
}


// verify
#define GO(A)   \
static uintptr_t my_verify_fct_##A = 0;                                 \
static int my_verify_##A(int a, void* b)                                \
{                                                                       \
    return (int)RunFunction(my_context, my_verify_fct_##A, 2, a, b);    \
}
SUPER()
#undef GO
static void* find_verify_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_verify_fct_##A == (uintptr_t)fct) return my_verify_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_verify_fct_##A == 0) {my_verify_fct_##A = (uintptr_t)fct; return my_verify_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libSM verify callback\n");
    return NULL;
}



#undef SUPER

EXPORT void my_SSL_CTX_set_default_passwd_cb(x86emu_t* emu, void* ctx, void* cb)
{
    libssl_my_t* my = (libssl_my_t*)my_lib->priv.w.p2;

    my->SSL_CTX_set_default_passwd_cb(ctx, find_pem_passwd_cb_Fct(cb));
}

EXPORT long my_SSL_CTX_callback_ctrl(x86emu_t* emu, void* ctx, int cmd, void* f)
{
    libssl_my_t* my = (libssl_my_t*)my_lib->priv.w.p2;

    return my->SSL_CTX_callback_ctrl(ctx, cmd, find_anonymous_Fct(f));
}

EXPORT long my_SSL_callback_ctrl(x86emu_t* emu, void* ctx, int cmd, void* f)
{
    libssl_my_t* my = (libssl_my_t*)my_lib->priv.w.p2;

    return my->SSL_callback_ctrl(ctx, cmd, find_anonymous_Fct(f));
}

EXPORT void my_SSL_CTX_set_verify(x86emu_t* emu, void* ctx, int mode, void* f)
{
    libssl_my_t* my = (libssl_my_t*)my_lib->priv.w.p2;

    my->SSL_CTX_set_verify(ctx, mode, find_verify_Fct(f));
}

EXPORT void my_SSL_set_verify(x86emu_t* emu, void* ctx, int mode, void* f)
{
    libssl_my_t* my = (libssl_my_t*)my_lib->priv.w.p2;

    my->SSL_set_verify(ctx, mode, find_verify_Fct(f));
}

#define CUSTOM_INIT \
    my_lib = lib;   \
    lib->priv.w.p2 = getSllMy(lib);

#define CUSTOM_FINI \
    my_lib = NULL;              \
    freeSllMy(lib->priv.w.p2);  \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"
