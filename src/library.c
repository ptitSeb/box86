#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "debug.h"
#include "library.h"
#include "elfloader.h"
#include "bridge.h"
#include "library_private.h"
#include "khash.h"

#include "wrappedlibs.h"
// create the native lib list
#define GO(P, N) int wrapped##N##_init(library_t* lib, box86context_t *box86); \
                 void wrapped##N##_fini(library_t* lib);\
                 int wrapped##N##_get(library_t* lib, const char* name, uintptr_t *offs, uint32_t *sz);
#include "library_list.h"
#undef GO

#define GO(P, N) {P, wrapped##N##_init, wrapped##N##_fini, wrapped##N##_get},
wrappedlib_t wrappedlibs[] = {
#include "library_list.h"
};
#undef GO

typedef struct bridged_s {
    char*       name;
    uintptr_t   start;
    uint32_t    end;
} bridged_t;

KHASH_MAP_INIT_STR(bridgemap, bridged_t)

KHASH_MAP_IMPL_STR(datamap, uint32_t)
KHASH_MAP_IMPL_STR(symbolmap, wrapper_t)
KHASH_MAP_IMPL_STR(symbol2map, symbol2_t)

char* Path2Name(const char* path)
{
    char* name = (char*)calloc(1, MAX_PATH);
    char* p = strrchr(path, '/');
    strcpy(name, (p)?(p+1):path);
    // name should be libXXX.so.A(.BB.CCCC)
    // so keep only 2 dot after ".so" (if any)
    p = strstr(name, ".so");
    if(p) {
        p=strchr(p+3, '.');   // this one is ok
        //if(p) p=strchr(p+1, '.');// this one is ok too
        if(p) p=strchr(p+1, '.');// this one is not
        if(p) *p = '\0';
    }
    return name;
}
int NbDot(const char* name)
{
    char *p = strstr(name, ".so");
    if(!p)
        return -1;
    int ret = 0;
    while(p) {
        p = strchr(p+1, '.');
        if(p) ++ret;
    }
    return ret;
}

library_t *NewLibrary(const char* path, box86context_t* box86)
{
    printf_log(LOG_DEBUG, "Trying to load \"%s\"\n", path);
    library_t *lib = (library_t*)calloc(1, sizeof(library_t));
    lib->name = Path2Name(path);
    lib->nbdot = NbDot(lib->name);
    lib->type = -1;
    printf_log(LOG_DEBUG, "Simplified name is \"%s\"\n", lib->name);
    // And now, actually loading a library
    // look for native(wrapped) libs first
    int nb = sizeof(wrappedlibs) / sizeof(wrappedlib_t);
    for (int i=0; i<nb; ++i) {
        if(strcmp(lib->name, wrappedlibs[i].name)==0) {
            if(wrappedlibs[i].init(lib, box86)) {
                // error!
                printf_log(LOG_NONE, "Error initializing native %s (last dlerror is %s)\n", wrappedlibs[i].name, dlerror());
                FreeLibrary(&lib);
                return NULL;
            }
            printf_log(LOG_INFO, "Using native(wrapped) %s\n", wrappedlibs[i].name);
            lib->priv.w.box86lib = box86;
            lib->fini = wrappedlibs[i].fini;
            lib->get = wrappedlibs[i].get;
            lib->type = 0;
            break;
        }
    }
    // then look for a native one
    // not implemented yet, so error...
    if(lib->type==-1)
    {
        FreeLibrary(&lib);
        return NULL;
    }

    lib->bridgemap = kh_init(bridgemap);

    return lib;
}
void FreeLibrary(library_t **lib)
{
    if(!(*lib)) return;

    if((*lib)->type!=-1 && (*lib)->fini) {
        (*lib)->fini(*lib);
    }
    free((*lib)->name);

    if((*lib)->bridgemap) {
        bridged_t *br;
        kh_foreach_value_ref((*lib)->bridgemap, br,
            free(br->name);
        );
        kh_destroy(bridgemap, (*lib)->bridgemap);
    }
    if((*lib)->symbolmap)
        kh_destroy(symbolmap, (*lib)->symbolmap);
    if((*lib)->datamap)
        kh_destroy(datamap, (*lib)->datamap);
    if((*lib)->mysymbolmap)
        kh_destroy(symbolmap, (*lib)->mysymbolmap);
    if((*lib)->symbol2map)
        kh_destroy(symbol2map, (*lib)->symbol2map);

    free(*lib);
    *lib = NULL;
}

char* GetNameLib(library_t *lib)
{
    return lib->name;
}
int IsSameLib(library_t* lib, const char* path)
{
    int ret = 0;
    char* name = Path2Name(path);
    if(strcmp(name, lib->name)==0)
        ret=1;
    else {
        int n = NbDot(name);
        if(n>=0 && n<lib->nbdot)
            if(strncmp(name, lib->name, strlen(name))==0)
                ret=1;
    }

    free(name);
    return ret;
}
int GetLibSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end)
{
    khint_t k;
    // check first if already in the map
    k = kh_get(bridgemap, lib->bridgemap, name);
    if(k!=kh_end(lib->bridgemap)) {
        *start = kh_value(lib->bridgemap, k).start;
        *end = kh_value(lib->bridgemap, k).end;
        return 1;
    }
    // get a new symbol
    if(lib->get(lib, name, start, end)) {
        *end += *start;     // lib->get(...) gives size, not end
        char* symbol = strdup(name);
        int ret;
        k = kh_put(bridgemap, lib->bridgemap, symbol, &ret);
        kh_value(lib->bridgemap, k).name = symbol;
        kh_value(lib->bridgemap, k).start = *start;
        kh_value(lib->bridgemap, k).end = *end;
        return 1;
    }
    // nope
    return 0;
}

int getSymbolInMaps(library_t*lib, const char* name, uintptr_t *addr, uint32_t *size)
{
    khint_t k;
    int ret;
    void* symbol;
    // check in datamap
    k = kh_get(datamap, lib->datamap, name);
    if (k!=kh_end(lib->datamap)) {
        symbol = dlsym(lib->priv.w.lib, kh_key(lib->datamap, k));
        if(symbol) {
            // found!
            *addr = (uintptr_t)symbol;
            *size = kh_value(lib->datamap, k);
            return 1;
        }
    }
    // check in mysymbolmap
    k = kh_get(symbolmap, lib->mysymbolmap, name);
    if (k!=kh_end(lib->mysymbolmap)) {
        char buff[200];
        strcpy(buff, "my_");
        strcat(buff, name);
        symbol = dlsym(lib->priv.w.box86lib, buff);
        if(!symbol)
            printf_log(LOG_NONE, "Warning, function %s not found\n", buff);
        *addr = AddBridge(lib->priv.w.bridge, kh_value(lib->mysymbolmap, k), symbol);
        *size = sizeof(void*);
        return 1;
    }
    // check in symbolmap
    k = kh_get(symbolmap, lib->symbolmap, name);
    if (k!=kh_end(lib->symbolmap)) {
        symbol = dlsym(lib->priv.w.lib, name);
        if(!symbol) {
            printf_log(LOG_INFO, "Warning, function %s not found in lib %s\n", name, lib->name);
            return 0;
        }
        *addr = AddBridge(lib->priv.w.bridge, kh_value(lib->symbolmap, k), symbol);
        *size = sizeof(void*);
        return 1;
    }
    // check in symbol2map
    k = kh_get(symbol2map, lib->symbol2map, name);
    if (k!=kh_end(lib->symbol2map)) {
        symbol = dlsym(lib->priv.w.lib, kh_value(lib->symbol2map, k).name);
        if(!symbol) {
            printf_log(LOG_INFO, "Warning, function %s not found in lib %s\n", kh_value(lib->symbol2map, k).name, lib->name);
            return 0;
        }
        *addr = AddBridge(lib->priv.w.bridge, kh_value(lib->symbol2map, k).w, symbol);
        *size = sizeof(void*);
        return 1;
    }

    return 0;
}