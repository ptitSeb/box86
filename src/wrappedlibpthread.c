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

int my_pthread_create(x86emu_t *emu, void* t, void* attr, void* start_routine, void* arg); //implemented in thread.c
int my_pthread_key_create(x86emu_t* emu, void* key, void* dtor);

const char* libpthreadName = "libpthread.so.0";
#define LIBNAME libpthread

#include "wrappedlib_init.h"