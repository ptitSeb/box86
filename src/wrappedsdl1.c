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

#include "wrappedlib_init.h"

