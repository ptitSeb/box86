#ifndef __WRAPPED_LIBPTHREAD_H__
#define __WRAPPED_LIBPTHREAD_H__

#include "wrappedlibs.h"

int wrappedlibpthread_init(library_t* lib);
void wrappedlibpthread_fini(library_t* lib);
int wrappedlibpthread_get(library_t* lib, const char* name, uintptr_t *offs, uint32_t *sz);

#endif //__WRAPPED_LIBPTHREAD_H__