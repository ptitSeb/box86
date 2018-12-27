#ifndef __WRAPPED_LIBRT_H__
#define __WRAPPED_LIBRT_H__

#include "wrappedlibs.h"

int wrappedlibrt_init(library_t* lib);
void wrappedlibrt_fini(library_t* lib);
int wrappedlibrt_get(library_t* lib, const char* name, uintptr_t *offs, uint32_t *sz);

#endif //__WRAPPED_LIBRT_H__