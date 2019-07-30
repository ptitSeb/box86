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
#include "librarian.h"
#include "x86emu.h"
#include "callback.h"
#include "box86context.h"
#include "sdl2rwops.h"

const char* smpeg2Name = "libsmpeg2-2.0.so.0";
#define LIBNAME smpeg2

typedef void (*vFpppp_t)(void*, void*, void*, void*);
typedef void* (*pFppii_t)(void*, void*, int32_t, int32_t);
static x86emu_t *dspemu = NULL;

static void smpeg_dispcallback(void* dst, int32_t x, int32_t y, unsigned int w, unsigned int h)
{
    x86emu_t *emu = dspemu;

    if(!emu)
        return;
    SetCallbackArg(emu, 0, dst);
    SetCallbackArg(emu, 1, (void*)x);
    SetCallbackArg(emu, 2, (void*)y);
    SetCallbackArg(emu, 3, (void*)w);
    SetCallbackArg(emu, 4, (void*)h);
    RunCallback(emu);
}

EXPORT void my_SMPEG_setdisplay(x86emu_t* emu, void* mpeg, void* cb, void* data, void* lock)
{
    library_t* lib = GetLib(GetEmuContext(emu)->maplib, smpeg2Name);
    vFpppp_t fnc = (vFpppp_t)lib->priv.w.priv;
    x86emu_t *old = dspemu;
    dspemu = NULL;
    if(cb)
        dspemu = AddCallback(emu, (uintptr_t)cb, 5, NULL, NULL, NULL, NULL);
    fnc(mpeg, dspemu?smpeg_dispcallback:NULL, data, lock);
    if(old)
        FreeCallback(old);
}

EXPORT void* my_SMPEG_new_rwops(x86emu_t* emu, void* src, void* info, int32_t f, int32_t audio)
{
    library_t* lib = GetLib(GetEmuContext(emu)->maplib, smpeg2Name);
    pFppii_t fnc = (pFppii_t)lib->priv.w.p2;
    SDL2_RWops_t *rw = RWNativeStart2(emu, (SDL2_RWops_t*)src);
    void* ret = fnc(rw, info, f, audio);
    if(!f) {
        RWNativeEnd2(rw);
        printf_log(LOG_INFO, "Warning, SMPEG_new_rwops called without freesrc set\n");
    }
    return ret;
}

#define CUSTOM_INIT \
    lib->priv.w.priv = dlsym(lib->priv.w.lib, "SMPEG_setdisplay"); \
    lib->priv.w.p2 = dlsym(lib->priv.w.lib, "SMPEG_new_rwops");

#include "wrappedlib_init.h"

