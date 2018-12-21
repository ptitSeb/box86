#ifndef __LIBRARIAN_PRIVATE_H_
#define __LIBRARIAN_PRIVATE_H_
#include <stdint.h>
#include "khash.h"

typedef struct {
    uintptr_t   offs;
    // need to track type of symbol?
    // need to track origin?
} onelib_t;

KHASH_MAP_DECLARE_STR(maplib_t, onelib_t)

typedef struct lib_s {
    khash_t(maplib_t)    *maplib;
} lib_t;

#endif //__LIBRARIAN_PRIVATE_H_
