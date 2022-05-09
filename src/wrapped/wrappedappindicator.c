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

const char* appindicatorName = "libappindicator.so.1";
#define LIBNAME appindicator

#define PRE_INIT    \
    if(box86_nogtk) \
        return -1;

#define CUSTOM_INIT \
    setNeededLibs(lib, 2, "libgtk-x11-2.0.so.0", "libglib-2.0.so.0");

#include "wrappedlib_init.h"
