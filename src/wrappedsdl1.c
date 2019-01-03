#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "library_private.h"
#include "x86emu.h"
#include "callback.h"
#include "librarian.h"
#include "x86emu_private.h"
#include "box86context.h"

const char* sdl1Name = "libSDL-1.2.so.0";
#define LIBNAME sdl1

int sdl_Yes() { return 1; }
int sdl_No() { return 0; }
int EXPORT my_SDL_Has3DNow() __attribute__((alias("sdl_No")));
int EXPORT my_SDL_Has3DNowExt() __attribute__((alias("sdl_No")));
int EXPORT my_SDL_HasAltiVec() __attribute__((alias("sdl_No")));
int EXPORT my_SDL_HasMMX() __attribute__((alias("sdl_Yes")));
int EXPORT my_SDL_HasMMXExt() __attribute__((alias("sdl_Yes")));
int EXPORT my_SDL_HasRDTSC() __attribute__((alias("sdl_Yes")));
int EXPORT my_SDL_HasSSE() __attribute__((alias("sdl_Yes")));
int EXPORT my_SDL_HasSSE2() __attribute__((alias("sdl_Yes")));

typedef struct {
  int32_t freq;
  uint16_t format;
  uint8_t channels;
  uint8_t silence;
  uint16_t samples;
  uint32_t size;
  void (*callback)(void *userdata, uint8_t *stream, int32_t len);
  void *userdata;
} SDL_AudioSpec;

typedef int (*PFNOPENAUDIO)(SDL_AudioSpec *desired, void *obtained);

void sdl1Callback(void *userdata, uint8_t *stream, int32_t len)
{
    x86emu_t *emu = (x86emu_t*) userdata;
    SetCallbackArg(emu, 1, stream);
    SetCallbackArg(emu, 2, (void*)len);
    RunCallback(emu);
}

// TODO: track the memory for those callback
int EXPORT my_SDL_OpenAudio(x86emu_t* emu, void* d, void* o)
{
    SDL_AudioSpec *desired = (SDL_AudioSpec*)d;
    library_t *sdllib = GetLib(emu->context->maplib, sdl1Name);
    PFNOPENAUDIO openaudio = (PFNOPENAUDIO)dlsym(sdllib->priv.w.lib, "SDL_OpenAudio");
    // create a callback
    void *fnc = (void*)desired->callback;
    x86emu_t *cbemu = (desired->callback)?NULL:AddCallback(emu, (uintptr_t)fnc, 3, desired->userdata, NULL, NULL, NULL);
    desired->userdata = cbemu;
    int ret = openaudio(desired, (SDL_AudioSpec*)o);
    if (ret!=0) {
        // error, clean the callback...
        desired->callback = fnc;
        FreeCallback(cbemu);
        return ret;
    }
    // put back stuff in place?
    desired->callback = fnc;
    ((SDL_AudioSpec*)o)->callback = fnc;

    return ret;
}

#include "wrappedlib_init.h"

