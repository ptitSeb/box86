#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <complex.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "library_private.h"
#include "x86emu.h"
#include "debug.h"

EXPORT void* my_clog(void* p, double real, double img)
{
    *(double complex*)p = clog(real+img*I);
    return p;
}

EXPORT void* my_csqrt(void* p, double real, double img)
{
    *(double complex*)p = csqrt(real+img*I);
    return p;
}

const char* libmName = "libm.so.6";
#define LIBNAME libm

#include "wrappedlib_init.h"

