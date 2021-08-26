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

const char* libncurseswName = "libncursesw.so.5";
#define LIBNAME libncursesw

static library_t* my_lib = NULL;

#define ADDED_FUNCTIONS() GO(stdscr, void*)
#include "generated/wrappedlibncurseswtypes.h"

typedef struct libncursesw_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} libncursesw_my_t;

void* getNCurseswMy(library_t* lib)
{
    libncursesw_my_t* my = (libncursesw_my_t*)calloc(1, sizeof(libncursesw_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeNCurseswMy(void* lib)
{
    //libncursesw_my_t *my = (libncursesw_my_t *)lib;
}

EXPORT int myw_mvwprintw(x86emu_t* emu, void* win, int y, int x, void* fmt, void* b)
{
    libncursesw_my_t *my = (libncursesw_my_t*)my_lib->priv.w.p2;

    char* buf = NULL;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    vasprintf(&buf, fmt, VARARGS);
    #else
    vasprintf(&buf, fmt, b);
    #endif
    // pre-bake the fmt/vaarg, because there is no "va_list" version of this function
    int ret = my->mvwprintw(win, y, x, buf);
    free(buf);
    return ret;
}

EXPORT int myw_printw(x86emu_t* emu, void* fmt, void* b)
{
    libncursesw_my_t *my = (libncursesw_my_t*)my_lib->priv.w.p2;

    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    return my->vw_printw(my->stdscr, fmt, VARARGS);
    #else
    return my->vw_printw(my->stdscr, fmt, b);
    #endif
}

EXPORT int myw_mvprintw(x86emu_t* emu, int x, int y, void* fmt, void* b)
{
    libncursesw_my_t *my = (libncursesw_my_t*)my_lib->priv.w.p2;

    char* buf = NULL;
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    vasprintf(&buf, fmt, VARARGS);
    #else
    vasprintf(&buf, fmt, b);
    #endif
    // pre-bake the fmt/vaarg, because there is no "va_list" version of this function
    int ret = my->mvprintw(x, y, buf);
    free(buf);
    return ret;
}

EXPORT int myw_vw_printw(x86emu_t *emu, void* win, void* fmt, void* b) {
    libncursesw_my_t *my = (libncursesw_my_t*)my_lib->priv.w.p2;

    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    PREPARE_VALIST;
    return my->vw_printw(win, fmt, VARARGS);
    #else
    // other platform don't need that
    return my->vw_printw(win, fmt, b);
    #endif
}
EXPORT int myw_vwprintw(x86emu_t *emu, void* win, void* fmt, void* b) __attribute__((alias("myw_vw_printw")));

EXPORT void* myw_initscr()
{
    libncursesw_my_t *my = (libncursesw_my_t*)my_lib->priv.w.p2;
    void* ret = my->initscr();
    my_checkGlobalTInfo();
    return ret;
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getNCurseswMy(lib); \
    my_lib = lib;   \
    lib->altmy = strdup("myw_"); \
    lib->priv.w.needed = 1; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libtinfo.so.5");

#define CUSTOM_FINI \
    freeNCurseswMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);           \
    my_lib = NULL;

#include "wrappedlib_init.h"
