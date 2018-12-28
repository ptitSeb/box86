#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibgl.h"

#include "wrapper.h"
#include "bridge.h"
#include "library_private.h"
#include "x86emu.h"

int wrappedlibgl_init(library_t* lib)
{
    lib->priv.w.lib = dlopen("libGL.so.1", RTLD_NOW);
    if(!lib->priv.w.lib) {
        return -1;
    }
    lib->priv.w.bridge = NewBridge();
    return 0;
}
void wrappedlibgl_fini(library_t* lib)
{
    if(lib->priv.w.lib)
        dlclose(lib->priv.w.lib);
    lib->priv.w.lib = NULL;
    FreeBridge(&lib->priv.w.bridge);
}
int wrappedlibgl_get(library_t* lib, const char* name, uintptr_t *offs, uint32_t *sz)
{
    uintptr_t addr = 0;
    uint32_t size = 0;
    void* symbol = NULL;

#include "wrappedlib_defines.h"
#include "wrappedlibgl_private.h"
#include "wrappedlib_undefs.h"

    if(!addr)
        return 0;
    *offs = addr;
    *sz = size;
    return 1;
}
