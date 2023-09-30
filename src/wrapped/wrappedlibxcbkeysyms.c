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

#ifdef ANDROID
	const char* libxcbkeysymsName = "libxcb-keysyms.so";
#else
	const char* libxcbkeysymsName = "libxcb-keysyms.so.1";
#endif

#define LIBNAME libxcbkeysyms

#include "wrappedlib_init.h"
