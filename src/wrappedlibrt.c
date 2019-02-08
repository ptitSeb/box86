#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "library_private.h"
#include "x86emu.h"

#undef clock_gettime

const char* librtName = "librt.so.1";
#define LIBNAME librt

#include "wrappedlib_init.h"
