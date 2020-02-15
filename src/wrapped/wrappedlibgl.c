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
#include "box86context.h"
#include "librarian.h"
#include "callback.h"

char* libGL;

const char* libglName = "libGL.so.1";
#define LIBNAME libgl

void fillGLProcWrapper(box86context_t*);
void freeProcWrapper(kh_symbolmap_t** symbolmap);

EXPORT void* my_glXGetProcAddress(x86emu_t* emu, void* name) 
{
    khint_t k;
    const char* rname = (const char*)name;
    printf_log(LOG_DEBUG, "Calling glXGetProcAddress(\"%s\")\n", rname);
    if(!emu->context->glwrappers)
        fillGLProcWrapper(emu->context);
    // check if glxprocaddress is filled, and search for lib and fill it if needed
    // get proc adress using actual glXGetProcAddress
    k = kh_get(symbolmap, emu->context->glmymap, rname);
    int is_my = (k==kh_end(emu->context->glmymap))?0:1;
    void* symbol;
    if(is_my) {
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        symbol = dlsym(emu->context->box86lib, tmp);
    } else 
        symbol = emu->context->glxprocaddress(rname);
    if(!symbol)
        return NULL;    // easy
    // check if alread bridged
    uintptr_t ret = CheckBridged(emu->context->system, symbol);
    if(ret) {
        return (void*)ret; // already bridged
    }
    // get wrapper    
    k = kh_get(symbolmap, emu->context->glwrappers, rname);
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
    AddOffsetSymbol(emu->context->maplib, symbol, rname);
    return (void*)AddBridge(emu->context->system, kh_value(emu->context->glwrappers, k), symbol, 0);
}
EXPORT void* my_glXGetProcAddressARB(x86emu_t* emu, void* name) __attribute__((alias("my_glXGetProcAddress")));

typedef void (*vFpp_t)(void*, void*);
typedef void (*debugProc_t)(int32_t, int32_t, uint32_t, int32_t, int32_t, void*, void*);
static x86emu_t *debug_cb = NULL;
static void debug_callback(int32_t source, int32_t type, uint32_t id, int32_t severity, int32_t length, const char* message, const void* param)
{
    if(!debug_cb)
        return;
    SetCallbackArg(debug_cb, 0, (void*)source);
    SetCallbackArg(debug_cb, 1, (void*)type);
    SetCallbackArg(debug_cb, 2, (void*)id);
    SetCallbackArg(debug_cb, 3, (void*)severity);
    SetCallbackArg(debug_cb, 4, (void*)length);
    SetCallbackArg(debug_cb, 5, (void*)message);
    SetCallbackArg(debug_cb, 6, (void*)param);
    RunCallback(debug_cb);
}
EXPORT void my_glDebugMessageCallback(x86emu_t* emu, void* prod, void* param)
{
    static vFpp_t DebugMessageCallback = NULL;
    static int init = 1;
    if(init) {
        DebugMessageCallback = emu->context->glxprocaddress("glDebugMessageCallback");
        init = 0;
    }
    if(!DebugMessageCallback)
        return;
    if(debug_cb) {
        FreeCallback(debug_cb);
        debug_cb = NULL;
    }
    if(prod)
        debug_cb = AddCallback(emu, (uintptr_t)prod, 7, NULL, NULL, NULL, NULL);
    DebugMessageCallback(prod?debug_callback:NULL, param);
}

#define PRE_INIT if(libGL) lib->priv.w.lib = dlopen(libGL, RTLD_LAZY | RTLD_GLOBAL); else
#define CUSTOM_INIT \
    lib->priv.w.priv = dlsym(lib->priv.w.lib, "glXGetProcAddress"); \
    box86->glxprocaddress = lib->priv.w.priv;


#include "wrappedlib_init.h"

void fillGLProcWrapper(box86context_t* context)
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
    // and the my_ symbols map
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, libglmysymbolmap[i].name, &ret);
        kh_value(symbolmap, k) = libglmysymbolmap[i].w;
    }
    context->glwrappers = symbolmap;
    // my_* map
    symbolmap = kh_init(symbolmap);
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, libglmysymbolmap[i].name, &ret);
        kh_value(symbolmap, k) = libglmysymbolmap[i].w;
    }
    context->glmymap = symbolmap;
}
void freeGLProcWrapper(box86context_t* context)
{
    if(!context)
        return;
    if(context->glwrappers)
        kh_destroy(symbolmap, context->glwrappers);
    if(context->glmymap)
        kh_destroy(symbolmap, context->glmymap);
    context->glwrappers = NULL;
    context->glmymap = NULL;
}
