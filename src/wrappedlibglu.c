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
#include "x86emu_private.h"

void EXPORT my_gluQuadricCallback(x86emu_t* emu, void* a, int32_t b, void* cb)
{
    printf_log(LOG_NONE, "Error, using Unimplemented SDL1 gluQuadricCallback\n");
    emu->quit = 1;
}
void EXPORT my_gluTessCallback(x86emu_t* emu, void* a, int32_t b, void* cb)
{
    printf_log(LOG_NONE, "Error, using Unimplemented SDL1 gluTessCallback\n");
    emu->quit = 1;
}
void EXPORT my_gluNurbsCallback(x86emu_t* emu, void* a, int32_t b, void* cb)
{
    printf_log(LOG_NONE, "Error, using Unimplemented SDL1 gluNurbsCallback\n");
    emu->quit = 1;
}

const char* libgluName = "libGLU.so.1";
#define LIBNAME libglu

#include "wrappedlib_init.h"
