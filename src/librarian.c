#include <stdio.h>
#include <stdlib.h>

#include "librarian.h"
#include "librarian_private.h"

KHASH_MAP_IMPL_STR(maplib_t, onelib_t);

lib_t *NewLibrarian()
{
    lib_t *maplib = (lib_t*)calloc(1, sizeof(lib_t));
    maplib->maplib = kh_init(maplib_t);

    return maplib;
}
void FreeLibrarian(lib_t **maplib)
{
    kh_destroy(maplib_t, (*maplib)->maplib);
    free(*maplib);
    *maplib = NULL;
}

void AddSymbol(lib_t *maplib, const char* name, uintptr_t addr)
{
    int ret;
    khint_t k = kh_put(maplib_t, maplib->maplib, name, &ret);
    kh_value(maplib->maplib, k).offs = addr;
}
uintptr_t FindSymbol(lib_t *maplib, const char* name)
{
    khint_t k = kh_get(maplib_t, maplib->maplib, name);
    if(k==kh_end(maplib->maplib))
        return 0;
    return kh_val(maplib->maplib, k).offs;
}
