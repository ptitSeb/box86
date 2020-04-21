#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
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
    GO(FcPatternAdd, iFppSi_t)

typedef struct fontconfig_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} fontconfig_my_t;

void* getFontconfigMy(library_t* lib)
{
    fontconfig_my_t* my = (fontconfig_my_t*)calloc(1, sizeof(fontconfig_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeFontconfigMy(void* lib)
{
    //fontconfig_my_t *my = (fontconfig_my_t *)lib;
}

EXPORT int my_FcPatternAdd(x86emu_t* emu, void* p, void* object, int type, uint32_t t1, uint32_t t2, int append)
{
    library_t* lib = GetLib(my_context->maplib, fontconfigName);
    fontconfig_my_t* my = (fontconfig_my_t*)lib->priv.w.p2;
    FcValue_t val;
    val.type = type;
    val.u.fake[0] = t1;
    val.u.fake[1] = t2;

    return my->FcPatternAdd(p, object, val, append);    // may need alignement, because of the double!
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getFontconfigMy(lib);

#define CUSTOM_FINI \
    freeFontconfigMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

