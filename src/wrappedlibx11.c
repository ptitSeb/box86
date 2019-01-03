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

typedef int (*XErrorHandler)(void *, void *);
XErrorHandler my_XSetErrorHandler(x86emu_t* t, XErrorHandler handler);

typedef int(*EventHandler) (void*,void*,void*);
int32_t my_XIfEvent(x86emu_t* emu, void* d,void* ev, EventHandler h, void* arg);

const char* libx11Name = "libX11.so.6";
#define LIBNAME libx11

#include "wrappedlib_init.h"


XErrorHandler EXPORT my_XSetErrorHandler(x86emu_t* emu, XErrorHandler handler)
{
    printf_log(LOG_NONE, "Warning, ignoring XSetErrorHandler\n");
    return NULL;
}

int32_t EXPORT my_XIfEvent(x86emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    printf_log(LOG_NONE, "Error: call unimplemented XIfEvent(%p, %p, %p, %p)\n", d, ev, h, arg);
    emu->quit = 1;
}