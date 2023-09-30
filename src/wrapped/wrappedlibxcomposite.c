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
#include "debug.h"

#ifdef ANDROID
	const char* libxcompositeName = "libXcomposite.so";
#else
	const char* libxcompositeName = "libXcomposite.so.1";
#endif

#define LIBNAME libxcomposite

#include "wrappedlib_init.h"

