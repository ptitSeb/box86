#ifndef __ELF_LOADER_H_
#define __ELF_LOADER_H_
#include <stdio.h>

typedef struct elfheader_s elfheader_t;

void* LoadAndCheckElfHeader(FILE* f, int exec); // exec : 0 = lib, 1 = exec
void FreeElfHeader(elfheader_t** head);

#endif //__ELF_LOADER_H_