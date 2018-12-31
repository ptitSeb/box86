#ifndef LIBNAME
#error Meh
#endif

#define FUNC3(M,N) wrapped##M##N
#define FUNC2(M,N) FUNC3(M,N)
#define FUNC(N) FUNC2(LIBNAME,N)
#define QUOTE(M) #M
#define PRIVATE2(P) QUOTE(wrapped##P##_private.h)
#define PRIVATE(P) PRIVATE2(P)
#define MAPNAME3(N,M) N##M
#define MAPNAME2(N,M) MAPNAME3(N,M)
#define MAPNAME(N) MAPNAME2(LIBNAME,N)
// prepare the maps
#define GO(N, W)
#define GOW(N, W)
#define GOM(N, W)
#define GO2(N, W, O)
#define DATA(N, S)
#define DATAV(N, S)
#define DATAB(N, S)

// #define the 4 maps first
#undef GO
#undef GOW
#define GO(N, W) {#N, W, 0},
#define GOW(N, W) {#N, W, 1},
static const onesymbol_t MAPNAME(symbolmap)[] = {
    #include PRIVATE(LIBNAME)
};
#undef GO
#undef GOW
#define GO(N, W)
#define GOW(N, W)
#undef GOM
#define GOM(N, W) {#N, W, 0},
static const onesymbol_t MAPNAME(mysymbolmap)[] = {
    #include PRIVATE(LIBNAME)
};
#undef GOM
#define GOM(N, W)
#undef GO2
#define GO2(N, W, O) {#N, W, 0, #O},
static const onesymbol2_t MAPNAME(symbol2map)[] = {
    #include PRIVATE(LIBNAME)
};
#undef GO2
#define GO2(N, W, O)
#undef DATA
#undef DATAV
#undef DATAB
#define DATA(N, S) {#N, S, 0},
#define DATAV(N, S) {#N, S, 1},
#define DATAB(N, S) {#N, S, 0},
static const onedata_t MAPNAME(datamap)[] = {
    #include PRIVATE(LIBNAME)
};
#include "wrappedlib_undefs.h"



int FUNC(_init)(library_t* lib)
{
// Init first
    lib->priv.w.lib = dlopen(MAPNAME(Name), RTLD_NOW);
    if(!lib->priv.w.lib) {
        return -1;
    }
    lib->priv.w.bridge = NewBridge();
// Create maps
    lib->symbolmap = kh_init(symbolmap);
    lib->mysymbolmap = kh_init(symbolmap);
    lib->symbol2map = kh_init(symbol2map);
    lib->datamap = kh_init(datamap);

    khint_t k;
    int ret;
    int cnt;

    // populates maps...
    cnt = sizeof(MAPNAME(symbolmap))/sizeof(onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, lib->symbolmap, MAPNAME(symbolmap)[i].name, &ret);
        kh_value(lib->symbolmap, k) = MAPNAME(symbolmap)[i].w;
    }
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, lib->mysymbolmap, MAPNAME(mysymbolmap)[i].name, &ret);
        kh_value(lib->mysymbolmap, k) = MAPNAME(mysymbolmap)[i].w;
    }
    cnt = sizeof(MAPNAME(symbol2map))/sizeof(onesymbol2_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbol2map, lib->symbol2map, MAPNAME(symbol2map)[i].name, &ret);
        kh_value(lib->symbol2map, k).name = MAPNAME(symbol2map)[i].name2;
        kh_value(lib->symbol2map, k).w = MAPNAME(symbol2map)[i].w;
    }
    cnt = sizeof(MAPNAME(datamap))/sizeof(onedata_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(datamap, lib->datamap, MAPNAME(datamap)[i].name, &ret);
        kh_value(lib->datamap, k) = MAPNAME(datamap)[i].sz;
    }
    
    return 0;
}

int FUNC(_fini)(library_t* lib)
{
    if(lib->priv.w.lib)
        dlclose(lib->priv.w.lib);
    lib->priv.w.lib = NULL;
    FreeBridge(&lib->priv.w.bridge);
}

int FUNC(_get)(library_t* lib, const char* name, uintptr_t *offs, uint32_t *sz)
{
    uintptr_t addr = 0;
    uint32_t size = 0;
    void* symbol = NULL;
    khint_t k;
//PRE
    if (!getSymbolInMaps(lib, name, &addr, &size)) {
//FAIL
        return 0;
    }
//POST
    if(!addr)
        return 0;
    *offs = addr;
    *sz = size;
    return 1;
}

