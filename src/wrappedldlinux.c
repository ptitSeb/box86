#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "library_private.h"
#include "x86emu.h"

const char* ldlinuxName = "ld-linux.so.3";
#define LIBNAME ldlinux
#define ALTNAME "ld-linux.so.2"

// define all standard library functions
#include "wrappedlib_init.h"

