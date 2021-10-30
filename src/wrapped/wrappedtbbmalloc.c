#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x86emu.h"
#include "callback.h"

static library_t* my_lib = NULL;

const char* tbbmallocName = "libtbbmalloc.so.2";
#define LIBNAME tbbmalloc

#include "wrappedlib_init.h"

