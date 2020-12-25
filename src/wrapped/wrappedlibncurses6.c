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

const char* libncurses6Name = "libncurses.so.6";
#define LIBNAME libncurses6

static library_t* my_lib = NULL;

// this is a simple copy of libncursesw wrapper. TODO: check if ok

typedef int         (*iFppp_t)(void*, void*, void*);
typedef int         (*iFpiip_t)(void*, int32_t, int32_t, void*);

#define SUPER() \
    GO(mvwprintw, iFpiip_t) \
    GO(vwprintw, iFppp_t)   \
    GO(stdscr, void*)

typedef struct libncurses6_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} libncurses6_my_t;

void* getNCurses6My(library_t* lib)
{
    libncurses6_my_t* my = (libncurses6_my_t*)calloc(1, sizeof(libncurses6_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeNCurses6My(void* lib)
{
    //libncurses6_my_t *my = (libncurses6_my_t *)lib;
}

EXPORT int my6_mvwprintw(x86emu_t* emu, void* win, int32_t y, int32_t x, void* fmt, void* b)
{
    libncurses6_my_t *my = (libncurses6_my_t*)my_lib->priv.w.p2;

    char* buf = NULL;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, VARARGS);
    #else
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, b);
    #endif
    // pre-bake the fmt/vaarg, because there is no "va_list" version of this function
    int ret = my->mvwprintw(win, y, x, buf);
    free(buf);
    return ret;
}

EXPORT int my6_printw(x86emu_t* emu, void* fmt, void* b)
{
    libncurses6_my_t *my = (libncurses6_my_t*)my_lib->priv.w.p2;

    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    return my->vwprintw(my->stdscr, fmt, VARARGS);
    #else
    return my->vwprintw(my->stdscr, fmt, b);
    #endif
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getNCurses6My(lib);    \
    my_lib = lib;                           \
    lib->altmy = strdup("my6_");            \
    lib->priv.w.needed = 1;                 \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libtinfo.so.6");

#define CUSTOM_FINI \
    freeNCurses6My(lib->priv.w.p2); \
    free(lib->priv.w.p2);           \
    my_lib = NULL;

#include "wrappedlib_init.h"
