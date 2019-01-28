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
#include "box86context.h"
#include "librarian.h"

void* my_glXGetProcAddress(x86emu_t* emu, void* name);
void* my_glXGetProcAddressARB(x86emu_t* emu, void* name);

const char* libglName = "libGL.so.1";
#define LIBNAME libgl
#define CUSTOM_INIT lib->priv.w.priv = dlsym(lib->priv.w.lib, "glXGetProcAddress");

#include "wrappedlib_init.h"

kh_symbolmap_t * fillGLProcWrapper()
{
    int cnt, ret;
    khint_t k;
    kh_symbolmap_t * symbolmap = kh_init(symbolmap);
    // populates maps...
    cnt = sizeof(libglsymbolmap)/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, libglsymbolmap[i].name, &ret);
        kh_value(symbolmap, k) = libglsymbolmap[i].w;
    }
    return symbolmap;
}
void freeGLProcWrapper(kh_symbolmap_t** symbolmap)
{
    if(!symbolmap)
        return;
    if(!*symbolmap)
        return
    kh_destroy(symbolmap, *symbolmap);
    *symbolmap = NULL;
}

EXPORT void* my_glXGetProcAddress(x86emu_t* emu, void* name) 
{
    const char* rname = (const char*)name;
    printf_log(LOG_DEBUG, "Calling glXGetProcAddress(%s)\n", rname);
    if(!emu->context->glwrappers)
        emu->context->glwrappers = fillGLProcWrapper();
    // check if glxprocaddress is filled, and search for lib and fill it if needed
    if(!emu->context->glxprocaddress) {
        library_t* lib = GetLib(emu->context->maplib, libglName);
        if(!lib) {
            printf_log(LOG_NONE, "Warning, libGL not found in librarian?!\n");
            return NULL;
        }
        emu->context->glxprocaddress = lib->priv.w.priv;
    }
    // get proc adress using actual glXGetProcAddress
    void* symbol = emu->context->glxprocaddress(rname);
    if(!symbol)
        return NULL;    // easy
    // check if alread bridged
    uintptr_t ret = CheckBridged(emu->context->system, symbol);
    if(ret)
        return (void*)ret; // already bridged
    // get wrapper    
    khint_t k = kh_get(symbolmap, emu->context->glwrappers, rname);
    if(k==kh_end(emu->context->glwrappers) && strstr(rname, "ARB")==NULL) {
        // try again, adding ARB at the end if not present
        char tmp[200];
        strcpy(tmp, rname);
        strcat(tmp, "ARB");
        k = kh_get(symbolmap, emu->context->glwrappers, tmp);
    }
    if(k==kh_end(emu->context->glwrappers) && strstr(rname, "EXT")==NULL) {
        // try again, adding EXT at the end if not present
        char tmp[200];
        strcpy(tmp, rname);
        strcat(tmp, "EXT");
        k = kh_get(symbolmap, emu->context->glwrappers, tmp);
    }
    if(k==kh_end(emu->context->glwrappers)) {
        printf_log(LOG_INFO, "Warning, no wrapper for %s\n", rname);
        return NULL;
    }
    return (void*)AddBridge(emu->context->system, kh_value(emu->context->glwrappers, k), symbol);
}
EXPORT void* my_glXGetProcAddressARB(x86emu_t* emu, void* name) __attribute__((alias("my_glXGetProcAddress")));
