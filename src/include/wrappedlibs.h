#ifndef __WRAPPEDLIBS_H__
#define __WRAPPEDLIBS_H__
#include <stdint.h>

typedef struct library_s library_t;
typedef struct box86context_s  box86context_t;

typedef int (*wrappedlib_init_t)(library_t * lib, box86context_t* box86);  // 0 = success
typedef void (*wrappedlib_fini_t)(library_t * lib);
typedef int (*wrappedlib_get_t)(library_t* lib, const char* name, uintptr_t *offs, uint32_t *sz);

typedef struct wrappedlib_s {
    const char*         name;
    wrappedlib_init_t   init;
    wrappedlib_fini_t   fini;
    wrappedlib_get_t   get;
    wrappedlib_get_t    getnoweak;
    wrappedlib_get_t    getlocal;
} wrappedlib_t;

#endif //__WRAPPEDLIBS_H__