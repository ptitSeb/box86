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
#include "myalign.h"

const char* libdrmName = "libdrm.so.2";
#define LIBNAME libdrm

typedef int (*iFppp_t)(void*, void*, void*);

#define ADDED_FUNCTIONS()                   \

#include "generated/wrappedlibdrmtypes.h"

#include "wrappercallback.h"


EXPORT void my_drmMsg(x86emu_t* emu, void* fmt, void* b) {
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    char* buf = NULL;
    void* f = vasprintf;
    ((iFppp_t)f)(&buf, fmt, VARARGS);
    my->drmMsg(buf);
    free(buf);
}

#define CUSTOM_INIT \
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"

