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
#include "sdl1rwops.h"


const char* sdl1netName = "libSDL_net-1.2.so.0";
#define LIBNAME sdl1net

#define CUSTOM_INIT \
    SETALT(my_);


#include "wrappedlib_init.h"

