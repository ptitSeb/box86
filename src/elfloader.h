#ifndef __ELF_LOADER_H_
#define __ELF_LOADER_H_
#include <stdio.h>

void* LoadAndCheckElfHeader(FILE* f, int exec); // exec : 0 = lib, 1 = exec

#endif //__ELF_LOADER_H_