#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include "wrappedlibs.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x86emu.h"
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include "debug.h"
const char* libcairogobjectName = "libcairo-gobject.so.2";
#define LIBNAME libcairogobject

#define PRE_INIT    \
    if(box86_nogtk) \
        return -1;

#include "wrappedlib_init.h"