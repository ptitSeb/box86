#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x86emu.h"

const char* udev0Name = "libudev.so.0";
#define LIBNAME udev0

#define CUSTOM_INIT \
    setNeededLibs(lib, 1, "libudev.so.1");

#include "wrappedlib_init.h"

