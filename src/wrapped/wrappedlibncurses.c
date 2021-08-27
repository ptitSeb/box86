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
#include "globalsymbols.h"

const char* libncursesName = "libncurses.so.5";
#define LIBNAME libncurses

static library_t* my_lib = NULL;

typedef int32_t (*iFppp_t)(void*, void*, void*);

#define ADDED_FUNCTIONS() GO(stdscr, void*)
#include "generated/wrappedlibncursestypes.h"

typedef struct libncurses_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} libncurses_my_t;

void* getNCursesMy(library_t* lib)
{
    libncurses_my_t* my = (libncurses_my_t*)calloc(1, sizeof(libncurses_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeNCursesMy(void* lib)
{
    //libncurses_my_t *my = (libncurses_my_t *)lib;
}

EXPORT int my_mvwprintw(x86emu_t* emu, void* win, int y, int x, void* fmt, void* b)
{
    libncurses_my_t *my = (libncurses_my_t*)my_lib->priv.w.p2;

    char* buf = NULL;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, VARARGS);
    #else
    vasprintf(&buf, fmt, b);
    #endif
    // pre-bake the fmt/vaarg, because there is no "va_list" version of this function
    int ret = my->mvwprintw(win, y, x, buf);
    free(buf);
    return ret;
}

EXPORT int my_printw(x86emu_t* emu, void* fmt, void* b)
{
    libncurses_my_t *my = (libncurses_my_t*)my_lib->priv.w.p2;

    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    return my->vw_printw(my->stdscr, fmt, VARARGS);
    #else
    return my->vw_printw(my->stdscr, fmt, b);
    #endif
}

EXPORT int my_mvprintw(x86emu_t* emu, int x, int y, void* fmt, void* b)
{
    libncurses_my_t *my = (libncurses_my_t*)my_lib->priv.w.p2;

    char* buf = NULL;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    iFppp_t f = (iFppp_t)vasprintf;
    f(&buf, fmt, VARARGS);
    #else
    vasprintf(&buf, fmt, b);
    #endif
    // pre-bake the fmt/vaarg, because there is no "va_list" version of this function
    int ret = my->mvprintw(x, y, buf);
    free(buf);
    return ret;
}

EXPORT int my_vw_printw(x86emu_t *emu, void* win, void* fmt, void* b) {
    libncurses_my_t *my = (libncurses_my_t*)my_lib->priv.w.p2;

    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    return my->vw_printw(win, fmt, VARARGS);
    #else
    // other platform don't need that
    return my->vw_printw(win, fmt, b);
    #endif
}
EXPORT int my_vwprintw(x86emu_t *emu, void* win, void* fmt, void* b) __attribute__((alias("my_vw_printw")));

EXPORT void* my_initscr()
{
    libncurses_my_t *my = (libncurses_my_t*)my_lib->priv.w.p2;
    void* ret = my->initscr();
    my_checkGlobalTInfo();
    return ret;
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getNCursesMy(lib); \
    my_lib = lib; \
    lib->priv.w.needed = 1; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libtinfo.so.5");

#define CUSTOM_FINI \
    freeNCursesMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);          \
    my_lib = NULL;

#include "wrappedlib_init.h"
