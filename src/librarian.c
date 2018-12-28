#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include "debug.h"
#include "librarian.h"
#include "librarian_private.h"
#include "library.h"
#include "x86emu.h"

#include "bridge.h"

KHASH_MAP_IMPL_STR(mapsymbols, onesymbol_t);

lib_t *NewLibrarian()
{
    lib_t *maplib = (lib_t*)calloc(1, sizeof(lib_t));
    
    maplib->mapsymbols = kh_init(mapsymbols);
    maplib->bridge = NewBridge();

    return maplib;
}
void FreeLibrarian(lib_t **maplib)
{
    kh_destroy(mapsymbols, (*maplib)->mapsymbols);
    // should that be in reverse order?
    for (int i=0; i<(*maplib)->libsz; ++i) {
        FreeLibrary(&(*maplib)->libraries[i].lib);
    }
    free((*maplib)->libraries);
    (*maplib)->libraries = NULL;
    (*maplib)->libsz = (*maplib)->libcap = 0;

    if((*maplib)->bridge)
        FreeBridge(&(*maplib)->bridge);

    free(*maplib);
    *maplib = NULL;

}

kh_mapsymbols_t* GetMapSymbol(lib_t* maplib)
{
    return maplib->mapsymbols;
}

int AddNeededLib(lib_t* maplib, const char* path)
{
    printf_log(LOG_DEBUG, "Trying to add \"%s\" to maplib\n", path);
    // first check if lib is already loaded
    for(int i=0; i<maplib->libsz; ++i) {
        onelib_t *onelib = &maplib->libraries[i];
        if(IsSameLib(onelib->lib, path)) {
            printf_log(LOG_DEBUG, "Already present in maplib => success\n");
            return 0;
        }
    }
    // load a new one
    library_t *lib = NewLibrary(path);
    if(!lib) {
        printf_log(LOG_DEBUG, "Faillure to create lib => fail\n");
        return 1;   //Error
    }
    
    // add lib now
    if (maplib->libsz == maplib->libcap) {
        maplib->libcap += 8;
        maplib->libraries = (onelib_t*)realloc(maplib->libraries, maplib->libcap*sizeof(onelib_t));
    }
    maplib->libraries[maplib->libsz].lib = lib;
    maplib->libraries[maplib->libsz].name = GetNameLib(lib);
    ++maplib->libsz;
    printf_log(LOG_DEBUG, "Created lib and added to maplib => success\n");
    
    return 0;
}

uintptr_t FindGlobalSymbol(lib_t *maplib, const char* name)
{
    uintptr_t start = 0, end = 0;
    for(int i=0; i<maplib->libsz; ++i) {
        if(GetLibSymbolStartEnd(maplib->libraries[i].lib, name, &start, &end))
            return start;
    }
    return 0;
}

int GetGlobalSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end)
{
    for(int i=0; i<maplib->libsz; ++i) {
        if(GetLibSymbolStartEnd(maplib->libraries[i].lib, name, start, end))
            return 1;
    }
    return 0;
}

void AddSymbol(kh_mapsymbols_t *mapsymbols, const char* name, uintptr_t addr, uint32_t sz)
{
    int ret;
    khint_t k = kh_put(mapsymbols, mapsymbols, name, &ret);
    kh_value(mapsymbols, k).offs = addr;
    kh_value(mapsymbols, k).sz = sz;
}
uintptr_t FindSymbol(kh_mapsymbols_t *mapsymbols, const char* name)
{
    khint_t k = kh_get(mapsymbols, mapsymbols, name);
    return kh_val(mapsymbols, k).offs;
}

int GetSymbolStartEnd(kh_mapsymbols_t* mapsymbols, const char* name, uintptr_t* start, uintptr_t* end)
{
    khint_t k = kh_get(mapsymbols, mapsymbols, name);
    if(k==kh_end(mapsymbols))
        return 0;
    *start = kh_val(mapsymbols, k).offs;
    *end = *start + kh_val(mapsymbols, k).sz;
    return 1;
}
