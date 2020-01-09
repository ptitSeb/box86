#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <complex.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x86emu.h"
#include "debug.h"

EXPORT void* my_clog(void* p, void* c)
{
    *(double complex*)p = clog(*(double complex*)c);
    return p;
}

EXPORT void* my_csqrt(void* p, void* c)
{
    *(double complex*)p = csqrt(*(double complex*)c);
    return p;
}

EXPORT void* my_csqrtf(void* p, void* c)
{
    *(float complex*)p = csqrtf(*(float complex*)c);
    return p;
}

const char* libmName = "libm.so.6";
#define LIBNAME libm

#include "wrappedlib_init.h"

