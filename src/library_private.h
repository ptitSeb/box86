#ifndef __LIBRARY_PRIVATE_H_
#define __LIBRARY_PRIVATE_H_
#include <stdint.h>

#include "wrappedlibs.h"

typedef struct bridge_s bridge_t;

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif

typedef struct wlib_s {
    bridge_t    *bridge;
    void*       lib;    // dlopen result
} wlib_t;

typedef struct library_s {
    char*               name;   // <> path
    int                 nbdot;  // nombre of "." after .so
    int                 type;   // 0: native(wrapped) 1: emulated(elf) -1: undetermined
    wrappedlib_fini_t   fini;
    wrappedlib_get_t    get;
    union {
        wlib_t  w;     
    }                   priv;  // private lib data
    bridge_t            *brige;   
} library_t;


#endif //__LIBRARY_PRIVATE_H_