#ifndef __WRAPPED_LIBC_H__
#define __WRAPPED_LIBC_H__

#include "wrappedlibs.h"

int wrappedlibc_init(library_t* lib);
void wrappedlibc_fini(library_t* lib);
int wrappedlibc_get(library_t* lib, const char* name, uintptr_t *offs, uint32_t *sz);

#endif