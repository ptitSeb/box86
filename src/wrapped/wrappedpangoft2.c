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
#include "librarian.h"
#include "box86context.h"
#include "emu/x86emu_private.h"
#include "myalign.h"

const char* pangoft2Name = "libpangoft2-1.0.so.0";
#define LIBNAME pangoft2

#define PRE_INIT    \
    if(box86_nogtk) \
        return -1;

#define CUSTOM_INIT \
    setNeededLibs(lib, 2, "libfontconfig.so.1", "libfreetype.so.6");

#include "wrappedlib_init.h"
