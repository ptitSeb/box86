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

const char* gdkpixbuf2Name = "libgdk_pixbuf-2.0.so.0";
#define LIBNAME gdkpixbuf2

#include "wrappedlib_init.h"
