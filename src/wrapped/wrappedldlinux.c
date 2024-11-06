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
#include "elfloader.h"
#include "box86context.h"

typedef struct my_tls_s {
    int         i;
    uint32_t     o;
} my_tls_t;

EXPORT void* my___tls_get_addr(x86emu_t* emu, void* p)
{
    my_tls_t *t = (my_tls_t*)p;
    return GetDTatOffset(emu->context, t->i, t->o);
}

EXPORT void* my____tls_get_addr(x86emu_t* emu)
{
    // the GNU version (with 3 '_') use register for the parameter!
    my_tls_t *t = (my_tls_t*)R_EAX;
    return GetDTatOffset(emu->context, t->i, t->o);
}

EXPORT void* my___libc_stack_end;
void stSetup(box86context_t* context)
{
    my___libc_stack_end = context->stack + context->stacksz;
}

const char* ldlinuxName = "ld-linux.so.3";
#define LIBNAME ldlinux
#define ALTNAME "ld-linux.so.2"
#if !defined(POWERPCLE)
#define ALTNAME2 "ld-linux-armhf.so.3"
#else
#define ALTNAME2 "ld-2.32.so"
#endif

// fake (ignored) _r_data structure
EXPORT void* my__r_debug[5];

#define PRE_INIT\
    if(1)                                                           \
        lib->w.lib = dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL);    \
    else

#define CUSTOM_INIT         \
    stSetup(box86);         \

#include "wrappedlib_init.h"

