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

const char* appindicator3Name = "libappindicator3.so.1";
#define LIBNAME appindicator3

#define PRE_INIT    \
    if(box86_nogtk) \
        return -1;

#define CUSTOM_INIT \
    SETALT(my3_);                   \
    setNeededLibs(lib, 5,           \
        "libgtk-3.so.0",            \
        "libgdk-3.so.0",            \
        "libgio-2.0.so.0",          \
        "libgobject-2.0.so.0",      \
        "libglib-2.0.so.0");

#include "wrappedlib_init.h"
