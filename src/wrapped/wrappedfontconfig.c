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

const char* fontconfigName = "libfontconfig.so.1";
#define LIBNAME fontconfig

typedef struct FcValue_s {
    int      type;
    union {
        char*           s;
        int             i;
        int             b;
        double          d;
        void*           m;
        void*           c;
        void*           f;
        void*           l;
        uint32_t        fake[2];    // to ease filing it
    } u;
} FcValue_t;

typedef int (*iFppSi_t)(void*, void*, FcValue_t, int);

#define SUPER() \
    GO(FcPatternAdd, iFppSi_t)              \
    GO(FcPatternAddWeak, iFppSi_t)          \

#include "wrappercallback.h"

EXPORT int my_FcPatternAdd(x86emu_t* emu, void* p, void* object, int type, uint32_t t1, uint32_t t2, int append)
{
    (void)emu;
    FcValue_t val;
    val.type = type;
    val.u.fake[0] = t1;
    val.u.fake[1] = t2;

    return my->FcPatternAdd(p, object, val, append);    // may need alignement, because of the double!
}

EXPORT int my_FcPatternAddWeak(x86emu_t* emu, void* p, void* object, int type, uint32_t t1, uint32_t t2, int append)
{
    (void)emu;
    FcValue_t val;
    val.type = type;
    val.u.fake[0] = t1;
    val.u.fake[1] = t2;

    return my->FcPatternAddWeak(p, object, val, append);    // may need alignement, because of the double!
}

#define CUSTOM_INIT \
    getMy(lib);                     \
    setNeededLibs(lib, 2,           \
        "libexpat.so.1",            \
        "libfreetype.so.6");

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"
