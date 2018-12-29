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
#include "x86emu_private.h"

void* my_dlopen(x86emu_t* emu, void *filename, int flag);
char* my_dlerror(x86emu_t* emu);
void* my_dlsym(x86emu_t* emu, void *handle, void *symbol);
int my_dlclose(x86emu_t* emu, void *handle);
int my_dladdr(x86emu_t* emu, void *addr, void *info);
void* my_dlvsym(x86emu_t* emu, void *handle, void *symbol, void *version);

int wrappedlibdl_init(library_t* lib)
{
    lib->priv.w.lib = dlopen("libdl.so.2", RTLD_NOW);
    if(!lib->priv.w.lib) {
        return -1;
    }
    lib->priv.w.bridge = NewBridge();
    return 0;
}
void wrappedlibdl_fini(library_t* lib)
{
    if(lib->priv.w.lib)
        dlclose(lib->priv.w.lib);
    lib->priv.w.lib = NULL;
    FreeBridge(&lib->priv.w.bridge);
}
int wrappedlibdl_get(library_t* lib, const char* name, uintptr_t *offs, uint32_t *sz)
{
    uintptr_t addr = 0;
    uint32_t size = 0;
    void* symbol = NULL;

#include "wrappedlib_defines.h"
#include "wrappedlibdl_private.h"
#include "wrappedlib_undefs.h"

    if(!addr)
        return 0;
    *offs = addr;
    *sz = size;
    return 1;
}

// Implementation
void* my_dlopen(x86emu_t* emu, void *filename, int flag)
{
    //void *dlopen(const char *filename, int flag);
    char* rfilename = (char*)filename;
    printf_log(LOG_INFO, "Error: unimplement call to dlopen(%s, %X)\n", rfilename, flag);
    emu->quit = 1;
    return NULL;

}
char* my_dlerror(x86emu_t* emu)
{
    //char *dlerror(void);
    printf_log(LOG_INFO, "Error: unimplement call to dlerror()\n");
    emu->quit = 1;
    return "";
}
void* my_dlsym(x86emu_t* emu, void *handle, void *symbol)
{
    //void *dlsym(void *handle, const char *symbol);
    char* rsymbol = (char*)symbol;
    printf_log(LOG_INFO, "Error: unimplement call to dlsym(%p, %s)\n", handle, rsymbol);
    emu->quit = 1;
    return NULL;
}
int my_dlclose(x86emu_t* emu, void *handle)
{
    //int dlclose(void *handle);
    printf_log(LOG_INFO, "Error: unimplement call to dlclose(%p)\n", handle);
    emu->quit = 1;
    return 0;
}
int my_dladdr(x86emu_t* emu, void *addr, void *info)
{
    //int dladdr(void *addr, Dl_info *info);
    printf_log(LOG_INFO, "Error: unimplement call to dladdr(%p, %p)\n", addr, info);
    emu->quit = 1;
    return 0;
}
void* my_dlvsym(x86emu_t* emu, void *handle, void *symbol, void *version)
{
    //void *dlvsym(void *handle, char *symbol, char *version);
    char* rsymbol = (char*)symbol;
    char* rversion = (char*)version;
    printf_log(LOG_INFO, "Error: unimplement call to dlvsym(%p, %s, %s)\n", handle, rsymbol, rversion);
    emu->quit = 1;
    return NULL;
}
