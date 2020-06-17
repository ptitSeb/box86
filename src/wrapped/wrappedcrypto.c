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
#include "callback.h"

const char* cryptoName = "libcrypto.so.1.0.0";
#define LIBNAME crypto
#define ALTNAME "libcrypto.so.1.0.2"

static library_t* my_lib = NULL;

typedef void*       (*pFp_t)        (void*);
typedef void        (*vFpp_t)       (void*, void*);
typedef int32_t     (*iFpiipp_t)    (void*, int32_t, int32_t, void*, void*);
typedef int32_t     (*iFpplppi_t)   (void*, void*, long, void*, void*, int32_t);

#define SUPER() \
    GO(ENGINE_ctrl, iFpiipp_t)          \
    GO(ENGINE_ctrl_cmd, iFpplppi_t)     \
    GO(sk_new, pFp_t)                   \
    GO(sk_pop_free, vFpp_t)

typedef struct crypto_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} crypto_my_t;

void* getCryptoMy(library_t* lib)
{
    crypto_my_t* my = (crypto_my_t*)calloc(1, sizeof(crypto_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeCryptoMy(void* lib)
{
    //crypto_my_t *my = (crypto_my_t *)lib;
}

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// ENGINE_ctrl_cb
#define GO(A)   \
static uintptr_t my_ENGINE_ctrl_cb_fct_##A = 0;                                                      \
static void my_ENGINE_ctrl_cb_##A()                    \
{                                                                                                   \
    RunFunction(my_context, my_ENGINE_ctrl_cb_fct_##A, 0);  \
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
static uintptr_t my_cmp_fnc_fct_##A = 0;                                \
static int my_cmp_fnc_##A(void* a, void* b)                             \
{                                                                       \
    return (int)RunFunction(my_context, my_cmp_fnc_fct_##A, 2, a, b);   \
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
static uintptr_t my_free_fnc_fct_##A = 0;               \
static void my_free_fnc_##A(void* p)                    \
{                                                       \
    RunFunction(my_context, my_free_fnc_fct_##A, 1, p); \
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

#undef SUPER

EXPORT int32_t my_ENGINE_ctrl(x86emu_t* emu, void* e, int32_t cmd, int32_t i, void* p, void* f)
{
    crypto_my_t *my = (crypto_my_t*)my_lib->priv.w.p2;

    return my->ENGINE_ctrl(e, cmd, i, p, find_ENGINE_ctrl_cb_Fct(f));
}

EXPORT int32_t my_ENGINE_ctrl_cmd(x86emu_t* emu, void* e, void* cmd, long i, void* p, void* f, int optional)
{
    crypto_my_t *my = (crypto_my_t*)my_lib->priv.w.p2;

    return my->ENGINE_ctrl_cmd(e, cmd, i, p, find_ENGINE_ctrl_cb_Fct(f), optional);
}

EXPORT void* my_sk_new(x86emu_t* emu, void* f)
{
    crypto_my_t *my = (crypto_my_t*)my_lib->priv.w.p2;

    return my->sk_new(find_cmp_fnc_Fct(f));
}

EXPORT void my_sk_pop_free(x86emu_t* emu, void* st, void* f)
{
    crypto_my_t *my = (crypto_my_t*)my_lib->priv.w.p2;

    my->sk_pop_free(st, find_free_fnc_Fct(f));
}

#define CUSTOM_INIT \
    my_lib = lib;   \
    lib->priv.w.p2 = getCryptoMy(lib);

#define CUSTOM_FINI \
    my_lib = NULL;  \
    freeCryptoMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"
