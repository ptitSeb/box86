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

#ifdef ANDROID
	const char* libxauName = "libXau.so";
#else
	const char* libxauName = "libXau.so.6";
#endif

#define LIBNAME libxau

#include "wrappedlib_init.h"
