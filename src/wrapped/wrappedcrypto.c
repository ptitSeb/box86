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
#include "callback.h"

const char* cryptoName = "libcrypto.so.1.0.0";
#define LIBNAME crypto
#define ALTNAME "libcrypto.so.1.0.2"
#define ALTNAME2 "libcrypto.so.1.1"

#include "generated/wrappedcryptotypes.h"

#include "wrappercallback.h"

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// ENGINE_ctrl_cb
#define GO(A)   \
static uintptr_t my_ENGINE_ctrl_cb_fct_##A = 0;                 \
static void my_ENGINE_ctrl_cb_##A()                             \
{                                                               \
    RunFunctionFmt(my_ENGINE_ctrl_cb_fct_##A, "");  \
}
SUPER()
#undef GO
static void* find_ENGINE_ctrl_cb_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_ENGINE_ctrl_cb_fct_##A == (uintptr_t)fct) return my_ENGINE_ctrl_cb_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ENGINE_ctrl_cb_fct_##A == 0) {my_ENGINE_ctrl_cb_fct_##A = (uintptr_t)fct; return my_ENGINE_ctrl_cb_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto ENGINE_ctrl_cb callback\n");
    return NULL;
}

// cmp_fnc
#define GO(A)   \
static uintptr_t my_cmp_fnc_fct_##A = 0;                                    \
static int my_cmp_fnc_##A(void* a, void* b)                                 \
{                                                                           \
    return (int)RunFunctionFmt(my_cmp_fnc_fct_##A, "pp", a, b); \
}
SUPER()
#undef GO
static void* find_cmp_fnc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_cmp_fnc_fct_##A == (uintptr_t)fct) return my_cmp_fnc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cmp_fnc_fct_##A == 0) {my_cmp_fnc_fct_##A = (uintptr_t)fct; return my_cmp_fnc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto cmp_fnc callback\n");
    return NULL;
}

// free_fnc
#define GO(A)   \
static uintptr_t my_free_fnc_fct_##A = 0;                       \
static void my_free_fnc_##A(void* p)                            \
{                                                               \
    RunFunctionFmt(my_free_fnc_fct_##A, "p", p);    \
}
SUPER()
#undef GO
static void* find_free_fnc_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_free_fnc_fct_##A == (uintptr_t)fct) return my_free_fnc_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_free_fnc_fct_##A == 0) {my_free_fnc_fct_##A = (uintptr_t)fct; return my_free_fnc_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto free_fnc callback\n");
    return NULL;
}

// id_func
#define GO(A)   \
static uintptr_t my_id_func_fct_##A = 0;                                        \
static unsigned long my_id_func_##A()                                           \
{                                                                               \
    return (unsigned long)RunFunctionFmt(my_id_func_fct_##A, "");   \
}
SUPER()
#undef GO
static void* find_id_func_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_id_func_fct_##A == (uintptr_t)fct) return my_id_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_id_func_fct_##A == 0) {my_id_func_fct_##A = (uintptr_t)fct; return my_id_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto id_func callback\n");
    return NULL;
}

// lock_func
#define GO(A)   \
static uintptr_t my_lock_func_fct_##A = 0;                                  \
static void my_lock_func_##A(int mode, int n, void* f, int l)               \
{                                                                           \
    RunFunctionFmt(my_lock_func_fct_##A, "iipi", mode, n, f, l);\
}
SUPER()
#undef GO
static void* find_lock_func_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_lock_func_fct_##A == (uintptr_t)fct) return my_lock_func_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_lock_func_fct_##A == 0) {my_lock_func_fct_##A = (uintptr_t)fct; return my_lock_func_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto lock_func callback\n");
    return NULL;
}

// passphrase
#define GO(A)   \
static uintptr_t my_passphrase_fct_##A = 0;                                                     \
static int my_passphrase_##A(void* buff, int size, int rw, void* u)                             \
{                                                                                               \
    return (int)RunFunctionFmt(my_passphrase_fct_##A, "piip", buff, size, rw, u);   \
}
SUPER()
#undef GO
static void* find_passphrase_Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_passphrase_fct_##A == (uintptr_t)fct) return my_passphrase_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_passphrase_fct_##A == 0) {my_passphrase_fct_##A = (uintptr_t)fct; return my_passphrase_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libcrypto passphrase callback\n");
    return NULL;
}

#undef SUPER

EXPORT int32_t my_ENGINE_ctrl(x86emu_t* emu, void* e, int32_t cmd, int32_t i, void* p, void* f)
{
    (void)emu;
    return my->ENGINE_ctrl(e, cmd, i, p, find_ENGINE_ctrl_cb_Fct(f));
}

EXPORT int32_t my_ENGINE_ctrl_cmd(x86emu_t* emu, void* e, void* cmd, long i, void* p, void* f, int optional)
{
    (void)emu;
    return my->ENGINE_ctrl_cmd(e, cmd, i, p, find_ENGINE_ctrl_cb_Fct(f), optional);
}

EXPORT void my_CRYPTO_set_id_callback(x86emu_t* emu, void* cb)
{
    (void)emu;
    my->CRYPTO_set_id_callback(find_id_func_Fct(cb));
}

EXPORT void my_CRYPTO_set_locking_callback(x86emu_t* emu, void* cb)
{
    (void)emu;
    my->CRYPTO_set_locking_callback(find_lock_func_Fct(cb));
}

EXPORT void my_PEM_read_bio_DSAPrivateKey(x86emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_DSAPrivateKey(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT void my_PEM_read_bio_DSA_PUBKEY(x86emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_DSA_PUBKEY(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT void my_PEM_read_bio_RSAPrivateKey(x86emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_RSAPrivateKey(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT void my_PEM_read_bio_RSA_PUBKEY(x86emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_RSA_PUBKEY(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT void my_PEM_read_bio_ECPrivateKey(x86emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_ECPrivateKey(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT void my_PEM_read_bio_EC_PUBKEY(x86emu_t* emu, void* bp, void* x, void* cb, void* u)
{
    (void)emu;
    my->PEM_read_bio_EC_PUBKEY(bp, x, find_passphrase_Fct(cb), u);
}

EXPORT int my_PEM_write_bio_DSAPrivateKey(x86emu_t* emu, void* bp, void* x, void* e, void* str, int len, void* cb, void* u)
{
    (void)emu;
    return my->PEM_write_bio_DSAPrivateKey(bp, x, e, str, len, find_passphrase_Fct(cb), u);
}

EXPORT int my_PEM_write_bio_RSAPrivateKey(x86emu_t* emu, void* bp, void* x, void* e, void* str, int len, void* cb, void* u)
{
    (void)emu;
    return my->PEM_write_bio_RSAPrivateKey(bp, x, e, str, len, find_passphrase_Fct(cb), u);
}

EXPORT int my_PEM_write_bio_ECPrivateKey(x86emu_t* emu, void* bp, void* x, void* e, void* str, int len, void* cb, void* u)
{
    (void)emu;
    return my->PEM_write_bio_ECPrivateKey(bp, x, e, str, len, find_passphrase_Fct(cb), u);
}

EXPORT void* my_sk_new(x86emu_t* emu, void* f)
{
    (void)emu;
    return my->sk_new(find_cmp_fnc_Fct(f));
}

EXPORT void my_sk_pop_free(x86emu_t* emu, void* st, void* f)
{
    (void)emu;
    my->sk_pop_free(st, find_free_fnc_Fct(f));
}

#define GO(A)           \
EXPORT void my_##A(void)\
{                       \
    if(my->A)           \
        my->A();        \
}
// Old deprecated function, not present (and useless) in 1.1 version
GO(OPENSSL_add_all_algorithms_conf)
GO(OPENSSL_add_all_algorithms_noconf)
GO(OpenSSL_add_all_ciphers)
GO(OpenSSL_add_all_digests)
#undef GO

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
