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
#include "debug.h"
#include "x86emu_private.h"
#include "box86context.h"
#include "librarian.h"

void* my_alGetProcAddress(x86emu_t* emu, void* name);

const char* openalName = "libopenal.so.1";
#define LIBNAME openal
#define CUSTOM_INIT lib->priv.w.priv = dlsym(lib->priv.w.lib, "alGetProcAddress");

#include "wrappedlib_init.h"

kh_symbolmap_t * fillALProcWrapper()
{
    int cnt, ret;
    khint_t k;
    kh_symbolmap_t * symbolmap = kh_init(symbolmap);
    // populates maps...
    cnt = sizeof(openalsymbolmap)/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, openalsymbolmap[i].name, &ret);
        kh_value(symbolmap, k) = openalsymbolmap[i].w;
    }
    return symbolmap;
}

EXPORT void* my_alGetProcAddress(x86emu_t* emu, void* name) 
{
    const char* rname = (const char*)name;
    printf_log(LOG_DEBUG, "Calling alGetProcAddress(%s)\n", rname);
    if(!emu->context->alwrappers)
        emu->context->alwrappers = fillALProcWrapper();
    procaddess_t alprocaddress = GetLib(emu->context->maplib, openalName)->priv.w.priv;
    // get proc adress using actual alGetProcAddress
    void* symbol = alprocaddress(rname);
    if(!symbol)
        return NULL;    // easy
    // check if alread bridged
    uintptr_t ret = CheckBridged(emu->context->system, symbol);
    if(ret)
        return (void*)ret; // already bridged
    // get wrapper    
    khint_t k = kh_get(symbolmap, emu->context->alwrappers, rname);
    if(k==kh_end(emu->context->alwrappers)) {
        printf_log(LOG_INFO, "Warning, no wrapper for %s\n", rname);
        return NULL;
    }
    return (void*)AddBridge(emu->context->system, kh_value(emu->context->alwrappers, k), symbol);
}
