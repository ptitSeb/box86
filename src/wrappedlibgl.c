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

void* my_glXGetProcAddress(x86emu_t* emu, void* name) EXPORT;
void* my_glXGetProcAddressARB(x86emu_t* emu, void* name) EXPORT;

const char* libglName = "libGL.so.1";
#define LIBNAME libgl

#include "wrappedlib_init.h"

void* my_glXGetProcAddress(x86emu_t* emu, void* name) 
{
    const char* rname = (const char*)name;
    printf_log(LOG_NONE, "Warning, Inimplemented glXGetProcAddress(%s)\n", name);
    return NULL;
}
void* my_glXGetProcAddressARB(x86emu_t* emu, void* name) __attribute__((alias("my_glXGetProcAddress")));
