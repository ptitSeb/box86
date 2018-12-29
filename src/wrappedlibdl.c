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
#include "library.h"
#include "librarian.h"
#include "box86context.h"


dlprivate_t *NewDLPrivate() {
    dlprivate_t* dl =  (dlprivate_t*)calloc(1, sizeof(dlprivate_t));
    return dl;
}
void FreeDLPrivate(dlprivate_t **lib) {
    free(*lib);
}

void* my_dlopen(x86emu_t* emu, void *filename, int flag) EXPORT;
char* my_dlerror(x86emu_t* emu) EXPORT;
void* my_dlsym(x86emu_t* emu, void *handle, void *symbol) EXPORT;
int my_dlclose(x86emu_t* emu, void *handle) EXPORT;
int my_dladdr(x86emu_t* emu, void *addr, void *info) EXPORT;
void* my_dlvsym(x86emu_t* emu, void *handle, void *symbol, void *version) EXPORT;

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
    lib->priv.w.priv = NULL;
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
    // TODO, handling special values for filename, like RTLD_SELF?
    // TODO, handling flags?
    char* rfilename = (char*)filename;
    printf_log(LOG_DEBUG, "Call to dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
    dlprivate_t *dl = emu->context->dlprivate;
    // check if alread dlopenned...
    for (int i=0; i<dl->lib_sz; ++i) {
        if(IsSameLib(dl->libs[i], rfilename))
            return (void*)(i+1);
    }
    // Then open the lib
    if(AddNeededLib(emu->context->maplib, rfilename)) {
        printf_log(LOG_INFO, "Warning: Cannot dlopen(\"%s\"/%p, %X)\n", rfilename, filename, flag);
        return NULL;
    }
    //get the lib and add it to the collection
    library_t *lib = GetLib(emu->context->maplib, rfilename);
    if(dl->lib_sz == dl->lib_cap) {
        dl->lib_cap += 4;
        dl->libs = (library_t**)realloc(dl->libs, sizeof(library_t*)*dl->lib_cap);
    }
    dl->libs[dl->lib_sz] = lib;
    return (void*)(++dl->lib_sz);
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
