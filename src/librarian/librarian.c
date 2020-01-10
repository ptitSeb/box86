#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include "debug.h"
#include "librarian.h"
#include "librarian_private.h"
#include "library.h"
#include "x86emu.h"
#include "box86context.h"
#include "elfloader.h"

#include "bridge.h"

KHASH_MAP_IMPL_STR(mapsymbols, onesymbol_t);
KHASH_MAP_IMPL_INT(mapoffsets, cstr_t);

lib_t *NewLibrarian(box86context_t* context)
{
    lib_t *maplib = (lib_t*)calloc(1, sizeof(lib_t));
    
    maplib->mapsymbols = kh_init(mapsymbols);
    maplib->weaksymbols = kh_init(mapsymbols);
    maplib->localsymbols = kh_init(mapsymbols);
    maplib->mapoffsets = kh_init(mapoffsets);
    maplib->bridge = NewBridge();

    maplib->context = context;

    return maplib;
}
void FreeLibrarian(lib_t **maplib)
{
    // should that be in reverse order?
    for (int i=0; i<(*maplib)->libsz; ++i) {
        Free1Library(&(*maplib)->libraries[i].lib);
    }
    free((*maplib)->libraries);
    (*maplib)->libraries = NULL;

    if((*maplib)->mapsymbols) {
        kh_destroy(mapsymbols, (*maplib)->mapsymbols);
    }
    if((*maplib)->weaksymbols) {
        kh_destroy(mapsymbols, (*maplib)->weaksymbols);
    }
    if((*maplib)->localsymbols) {
        kh_destroy(mapsymbols, (*maplib)->localsymbols);
    }
    if((*maplib)->mapoffsets) {
        kh_destroy(mapoffsets, (*maplib)->mapoffsets);
    }
    (*maplib)->libsz = (*maplib)->libcap = 0;

    if((*maplib)->bridge)
        FreeBridge(&(*maplib)->bridge);

    free(*maplib);
    *maplib = NULL;

}

box86context_t* GetLibrarianContext(lib_t* maplib)
{
    return maplib->context;
}

kh_mapsymbols_t* GetMapSymbol(lib_t* maplib)
{
    return maplib->mapsymbols;
}

kh_mapsymbols_t* GetWeakSymbol(lib_t* maplib)
{
    return maplib->weaksymbols;
}

kh_mapsymbols_t* GetLocalSymbol(lib_t* maplib)
{
    return maplib->localsymbols;
}

library_t* getLib(lib_t* maplib, const char* path)
{
    for(int i=0; i<maplib->libsz; ++i) {
        onelib_t *onelib = &maplib->libraries[i];
        if(IsSameLib(onelib->lib, path)) {
            return onelib->lib;
        }
    }
    return NULL;
}

int AddNeededLib(lib_t* maplib, library_t* parent, const char* path, box86context_t* box86, x86emu_t* emu)
{
    printf_log(LOG_DEBUG, "Trying to add \"%s\" to maplib\n", path);
    // first check if lib is already loaded
    library_t *lib = getLib(maplib, path);
    if(lib) {
        if(parent)
            LibAddNeededLib(parent, lib);
        printf_log(LOG_DEBUG, "Already present in maplib => success\n");
        return 0;
    }
    // load a new one
    lib = NewLibrary(path, box86);
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
    if(AddSymbolsLibrary(lib, emu)) {   // also add needed libs
        printf_log(LOG_DEBUG, "Failure to Add lib => fail\n");
        return 1;
    }

    if(parent)
        LibAddNeededLib(parent, lib);
    
    printf_log(LOG_DEBUG, "Created lib and added to maplib => success\n");
    
    return 0;
}
int FinalizeNeededLib(lib_t* maplib, const char* path, box86context_t* box86, x86emu_t* emu)
{
    if(FinalizeLibrary(GetLib(maplib, path), emu)) {
        printf_log(LOG_DEBUG, "Failure to finalizing lib => fail\n");
        return 1;
    }
    
    return 0;
}

library_t* GetLib(lib_t* maplib, const char* name)
{
    printf_log(LOG_DEBUG, "Trying to Get \"%s\" to maplib\n", name);
    return getLib(maplib, name);
}

uintptr_t FindGlobalSymbol(lib_t *maplib, const char* name)
{
    uintptr_t start = 0, end = 0;
    if(GetGlobalSymbolStartEnd(maplib, name, &start, &end))
        return start;
    return 0;
}

int GetNoSelfSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t* self)
{
    //excude self if defined
    if(maplib->context->elfs[0]!=self) {
        if(GetSymbolStartEnd(maplib->mapsymbols, name, start, end))
            if(*start)
                return 1;
        if(GetSymbolStartEnd(maplib->weaksymbols, name, start, end))
            if(*start)
                return 1;
    }
    for(int i=0; i<maplib->libsz; ++i) {
        if(GetElfIndex(maplib->libraries[i].lib)==-1 || (maplib->context->elfs[GetElfIndex(maplib->libraries[i].lib)]!=self))
            if(GetLibSymbolStartEnd(maplib->libraries[i].lib, name, start, end))
                if(*start)
                    return 1;
    }
    // if self defined, give it another chance with self...
    if(self) {
        if(maplib->context->elfs[0]==self) {
            if(GetSymbolStartEnd(maplib->mapsymbols, name, start, end))
                if(*start)
                    return 1;
            if(GetSymbolStartEnd(maplib->weaksymbols, name, start, end))
                if(*start)
                    return 1;
        }
        for(int i=0; i<maplib->libsz; ++i) {
            if(GetElfIndex(maplib->libraries[i].lib)!=-1 && (maplib->context->elfs[GetElfIndex(maplib->libraries[i].lib)]==self))
                if(GetLibSymbolStartEnd(maplib->libraries[i].lib, name, start, end))
                    if(*start)
                        return 1;
        }
    }
    // nope, not found
    return 0;
}
int GetGlobalSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end)
{
    // search non-weak symbol, from newer to older
    if(GetSymbolStartEnd(maplib->mapsymbols, name, start, end))
        if(*start)
            return 1;
    for(int i=maplib->libsz-1; i>=0; --i) {
        if(GetLibNoWeakSymbolStartEnd(maplib->libraries[i].lib, name, start, end))
            if(*start)
                return 1;
    }

    // and now weak symbol, from older to newer
    // library for newer to older, weak only now
    if(GetSymbolStartEnd(maplib->weaksymbols, name, start, end))
        if(*start)
            return 1;
    for(int i=0; i<maplib->libsz; ++i) {
        if(GetLibSymbolStartEnd(maplib->libraries[i].lib, name, start, end))    // only weak symbol haven't been found yet
            if(*start)
                return 1;
    }
    // nope, not found
    return 0;
}

elfheader_t* GetGlobalSymbolElf(lib_t *maplib, const char* name)
{
    uintptr_t start = 0;
    uintptr_t end = 0;
    if(GetSymbolStartEnd(maplib->mapsymbols, name, &start, &end))
        if(start)
            return maplib->context->elfs[0];
    if(GetSymbolStartEnd(maplib->weaksymbols, name, &start, &end))
        if(start)
            return maplib->context->elfs[0];
    for(int i=0; i<maplib->libsz; ++i) {
        if(GetLibSymbolStartEnd(maplib->libraries[i].lib, name, &start, &end))
            if(start) {
                int idx = GetElfIndex(maplib->libraries[i].lib);
                if(idx==-1) {
                    printf_log(LOG_NONE, "Warning, getting Elf info for a native symbol \"%s\" from lib \"%s\"\n", name, GetNameLib(maplib->libraries[i].lib));
                    return NULL;
                }
                return maplib->context->elfs[idx];
            }
    }
    // nope, not found
    return NULL;
}

int GetGlobalNoWeakSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end)
{
    if(GetSymbolStartEnd(maplib->mapsymbols, name, start, end))
        if(*start)
            return 1;
    for(int i=0; i<maplib->libsz; ++i)
        if(GetLibNoWeakSymbolStartEnd(maplib->libraries[i].lib, name, start, end))
            if(*start)
                return 1;
    // nope, not found
    return 0;
}

int GetLocalSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t *self)
{
    if(maplib->context->elfs[0]==self) {
        if(GetSymbolStartEnd(maplib->localsymbols, name, start, end))
            if(*start)
                return 1;
    } else {
        for(int i=0; i<maplib->libsz; ++i) {
            if(GetElfIndex(maplib->libraries[i].lib)!=-1 && (maplib->context->elfs[GetElfIndex(maplib->libraries[i].lib)]==self))
                if(GetLibLocalSymbolStartEnd(maplib->libraries[i].lib, name, start, end))
                    if(*start)
                        return 1;
        }
    }
    return 0;
}

int GetNoWeakSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t *self)
{
    if(maplib->context->elfs[0]==self) {
        if(GetSymbolStartEnd(maplib->mapsymbols, name, start, end))
            if(*start)
                return 1;
    } else {
        for(int i=0; i<maplib->libsz; ++i) {
            if(GetElfIndex(maplib->libraries[i].lib)!=-1 && (maplib->context->elfs[GetElfIndex(maplib->libraries[i].lib)]==self))
                if(GetLibNoWeakSymbolStartEnd(maplib->libraries[i].lib, name, start, end))
                    if(*start)
                        return 1;
        }
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
    if(k==kh_end(mapsymbols))
        return 0;
    return kh_val(mapsymbols, k).offs;
}
void AddWeakSymbol(kh_mapsymbols_t *mapsymbols, const char* name, uintptr_t addr, uint32_t sz)
{
    int ret;
    khint_t k = kh_put(mapsymbols, mapsymbols, name, &ret);
    if(ret==1)
        return; // Symbol already there, don't touch it
    kh_value(mapsymbols, k).offs = addr;
    kh_value(mapsymbols, k).sz = sz;
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

const char* GetSymbolName(kh_mapsymbols_t* mapsymbols, void* p, uintptr_t* start, uint32_t* sz)
{
    uintptr_t addr = (uintptr_t)p;
    onesymbol_t *one;
    kh_foreach_value_ref(mapsymbols, one, 
        if((one->offs >= addr) && (one->offs+one->sz<addr)) {
            *start  = one->offs;
            *sz = one->sz;
            return kh_key(mapsymbols, __i);
        }
    );
    return NULL;
}

const char* FindSymbolName(lib_t *maplib, void* p, void** start, uint32_t* sz, const char** libname, void** base)
{
    // first, search in self...
    const char* ret = NULL;
    uintptr_t offs = 0;
    uint32_t size = 0;
    uintptr_t elf_start = 0;
    uintptr_t elf_end = 0;
    // loop the elfs...
    for(int i=0; i<maplib->context->elfsize && !ret; ++i) {
        elf_start = (uintptr_t)GetBaseAddress(maplib->context->elfs[i]);
        elf_end = elf_start + GetBaseSize(maplib->context->elfs[i]);
        if((uintptr_t)p>=elf_start && (uintptr_t)p<elf_end)
            ret = FindNearestSymbolName(maplib->context->elfs[i], p, &offs, &size);
    }
    if(ret) {
        if(start)
            *start = (void*)offs;
        if(sz)
            *sz = size;
        if(libname)
            *libname = maplib->context->fullpath;
        if(base)
            *base = (void*)elf_start;
        return ret;
    }
    // TODO: then search in the other libs...
    return NULL;
}

void AddOffsetSymbol(lib_t *maplib, void* offs, const char* name)
{
    int ret;
    khint_t k = kh_put(mapoffsets, maplib->mapoffsets, (uintptr_t)offs, &ret);
    kh_value(maplib->mapoffsets, k) = (cstr_t)name;
}

const char* GetNameOffset(lib_t *maplib, void* offs)
{
    khint_t k = kh_get(mapoffsets, maplib->mapoffsets, (uintptr_t)offs);
    if (k!=kh_end(maplib->mapoffsets))
        return kh_value(maplib->mapoffsets, k);
    return NULL;
}
