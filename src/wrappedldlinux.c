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
#include "x86emu_private.h"

extern void* __tls_get_addr(void*);  // I just declare this here...
EXPORT void* my____tls_get_addr(x86emu_t* emu)
{
    // the GNU version (with 3 '_') use register for the parameter!
    return __tls_get_addr((void*)R_EAX);
}

const char* ldlinuxName = "ld-linux.so.3";
#define LIBNAME ldlinux
#define ALTNAME "ld-linux.so.2"
#define ALTNAME2 "ld-linux-armhf.so.3"

// define all standard library functions
#include "wrappedlib_init.h"

