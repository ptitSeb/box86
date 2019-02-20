#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "library_private.h"
#include "x86emu.h"

const char* dbusName = "libdbus-1.so.3";
#define LIBNAME dbus


typedef int32_t(* DBusHandleMessageFunction) (void* connection, void* message, void* user_data);


#include "wrappedlib_init.h"

