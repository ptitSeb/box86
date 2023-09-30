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
#include "callback.h"
#include "box86context.h"
#include "librarian.h"
#include "myalign.h"

#ifdef ANDROID
    const char* pulsesimpleName = "libpulse-simple.so";
#else
    const char* pulsesimpleName = "libpulse-simple.so.0";
#endif

#define LIBNAME pulsesimple

#define PRE_INIT        \
    if(box86_nopulse)   \
        return -1;

#ifdef ANDROID
    #define CUSTOM_INIT \
        setNeededLibs(lib, 1, "libpulse.so");
#else
    #define CUSTOM_INIT \
        setNeededLibs(lib, 1, "libpulse.so.0");
#endif

#include "wrappedlib_init.h"

