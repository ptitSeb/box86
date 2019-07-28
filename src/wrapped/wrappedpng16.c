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

const char* png16Name = "libpng16.so.16";
#define LIBNAME png16

#define CUSTOM_INIT     lib->priv.w.altprefix=strdup("yes");

#include "wrappedlib_init.h"
