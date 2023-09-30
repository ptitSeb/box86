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

#ifdef ANDROID
    const char* libxxf86vmName = "libXxf86vm.so";
#else
    const char* libxxf86vmName = "libXxf86vm.so.1";
#endif
#define LIBNAME libxxf86vm

#ifdef PANDORA
typedef struct my_XF86VidModeGamma_s {
    float red;
    float green;
    float blue;
} my_XF86VidModeGamma_t;

static my_XF86VidModeGamma_t current_gamma = {0};

EXPORT int my_XF86VidModeGetGamma(void* display, int screen, my_XF86VidModeGamma_t* gamma)
{
    memcpy(gamma, &current_gamma, sizeof(current_gamma));
    return 1;
}

EXPORT int my_XF86VidModeSetGamma(void* display, int screen, my_XF86VidModeGamma_t* gamma)
{
    memcpy(&current_gamma, gamma, sizeof(current_gamma));
    float mean = (current_gamma.red+current_gamma.green+current_gamma.blue)/3;
    char buf[50];
    if(mean==0.0f)
        sprintf(buf, "sudo /usr/pandora/scripts/op_gamma.sh 0");
    else
        sprintf(buf, "sudo /usr/pandora/scripts/op_gamma.sh %.2f", mean);
    system(buf);
    return 1;
}
#endif

#ifdef ANDROID
    #define CUSTOM_INIT \
        setNeededLibs(lib, 2, "libX11.so", "libXext.so");
#else
    #define CUSTOM_INIT \
        setNeededLibs(lib, 2, "libX11.so.6", "libXext.so.6");
#endif

#include "wrappedlib_init.h"
