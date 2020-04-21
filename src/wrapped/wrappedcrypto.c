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

typedef int32_t (*iFpiipp_t)(void*, int32_t, int32_t, void*, void*);

typedef struct crypto_my_s {
    // functions
    iFpiipp_t        ENGINE_ctrl;
} crypto_my_t;

void* getCryptoMy(library_t* lib)
{
    crypto_my_t* my = (crypto_my_t*)calloc(1, sizeof(crypto_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(ENGINE_ctrl, iFpiipp_t)
    #undef GO
    return my;
}

void freeCryptoMy(void* lib)
{
    //crypto_my_t *my = (crypto_my_t *)lib;
}

x86emu_t* engine_ctrl_emu = NULL;
static void my_ENGINE_ctrl_cb()
{
    if(engine_ctrl_emu)
        RunCallback(engine_ctrl_emu);
}
EXPORT int32_t my_ENGINE_ctrl(x86emu_t* emu, void* e, int32_t cmd, int32_t i, void* p, void* f)
{
    library_t * lib = GetLib(emu->context->maplib, cryptoName);
    crypto_my_t *my = (crypto_my_t*)lib->priv.w.p2;

    engine_ctrl_emu = f?AddSharedCallback(emu, (uintptr_t)f, 0, NULL, NULL, NULL, NULL):NULL;
    int32_t ret = my->ENGINE_ctrl(e, cmd, i, p, f?my_ENGINE_ctrl_cb:NULL);
    if(f) {
        FreeCallback(engine_ctrl_emu);
        engine_ctrl_emu = NULL;
    }
    return ret;
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getCryptoMy(lib);

#define CUSTOM_FINI \
    freeCryptoMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"
