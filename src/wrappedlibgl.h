#ifndef __WRAPPED_LIBGL_H__
#define __WRAPPED_LIBGL_H__

#include "wrappedlibs.h"

int wrappedlibgl_init(library_t* lib);
void wrappedlibgl_fini(library_t* lib);
int wrappedlibgl_get(library_t* lib, const char* name, uintptr_t *offs, uint32_t *sz);

#endif //__WRAPPED_LIBGL_H__