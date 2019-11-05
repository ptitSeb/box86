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
#include "sdl1rwops.h"

const char* smpegName = "libsmpeg-0.4.so.0";
#define LIBNAME smpeg

typedef void (*vFpppp_t)(void*, void*, void*, void*);
typedef void* (*pFppi_t)(void*, void*, int32_t);
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

EXPORT void my_SMPEG_setdisplay(x86emu_t* emu, void* mpeg, void* surf, void* lock, void* cb)
{
    library_t* lib = GetLib(GetEmuContext(emu)->maplib, smpegName);
    vFpppp_t fnc = (vFpppp_t)lib->priv.w.priv;
    x86emu_t *old = dspemu;
    dspemu = NULL;
    if(cb)
        dspemu = AddCallback(emu, (uintptr_t)cb, 5, NULL, NULL, NULL, NULL);
    fnc(mpeg, surf, lock, dspemu?smpeg_dispcallback:NULL);
    if(old)
        FreeCallback(old);
}

EXPORT void* my_SMPEG_new_rwops(x86emu_t* emu, void* src, void* info, int32_t f)
{
    library_t* lib = GetLib(GetEmuContext(emu)->maplib, smpegName);
    pFppi_t fnc = (pFppi_t)lib->priv.w.p2;
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)src);
    void* ret = fnc(rw, info, f);
    if(!f) {
        RWNativeEnd(rw);
        printf_log(LOG_INFO, "Warning, SMPEG_new_rwops called without freesrc set\n");
    }
    return ret;
}

#define CUSTOM_INIT \
    lib->priv.w.priv = dlsym(lib->priv.w.lib, "SMPEG_setdisplay"); \
    lib->priv.w.p2 = dlsym(lib->priv.w.lib, "SMPEG_new_rwops");

#include "wrappedlib_init.h"

