#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "debug.h"
#include "library.h"
#include "elfloader.h"
#include "library_private.h"
#include "khash.h"

#include "wrappedlibs.h"
// create the native lib list
#define GO(P, N) int wrapped##N##_init(library_t* lib); \
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

library_t *NewLibrary(const char* path)
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
            if(wrappedlibs[i].init(lib)) {
                // error!
                printf_log(LOG_NONE, "Error initializing native %s (last dlerror is %s)\n", wrappedlibs[i].name, dlerror());
                FreeLibrary(&lib);
                return NULL;
            }
            printf_log(LOG_INFO, "Using native(wrapped) %s\n", wrappedlibs[i].name);
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
