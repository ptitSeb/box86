#ifndef __WRAPPED_LDLINUX_H__
#define __WRAPPED_LDLINUX_H__

#include "wrappedlibs.h"

int wrappedldlinux_init(library_t* lib);
void wrappedldlinux_fini(library_t* lib);
int wrappedldlinux_get(library_t* lib, const char* name, uintptr_t *offs, uint32_t *sz);

#endif //__WRAPPED_LDLINUX_H__