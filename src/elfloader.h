#ifndef __ELF_LOADER_H_
#define __ELF_LOADER_H_
#include <stdio.h>

typedef struct elfheader_s elfheader_t;

void* LoadAndCheckElfHeader(FILE* f, const char* name, int exec); // exec : 0 = lib, 1 = exec
void FreeElfHeader(elfheader_t** head);

int CalcLoadAddr(elfheader_t* head);    // return 0 if OK

int AllocElfMemory(elfheader_t* head);    // return 0 if OK

#endif //__ELF_LOADER_H_