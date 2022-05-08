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
#include "box86context.h"
#include "sdl2rwops.h"


const char* sdl2netName = "libSDL2_net-2.0.so.0";
#define LIBNAME sdl2net

#define CUSTOM_INIT \
    SETALT(my2_);

#include "wrappedlib_init.h"

