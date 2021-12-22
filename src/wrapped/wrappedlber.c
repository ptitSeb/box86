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

const char* lberName =
#ifdef ANDROID
    "liblber-2.4.so"
#else
    "liblber-2.4.so.2"
#endif
    ;
#define LIBNAME lber

#include "wrappedlib_init.h"

