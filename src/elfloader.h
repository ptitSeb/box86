#ifndef __ELF_LOADER_H_
#define __ELF_LOADER_H_
#include <stdio.h>

typedef struct elfheader_s elfheader_t;

void* LoadAndCheckElfHeader(FILE* f, const char* name, int exec); // exec : 0 = lib, 1 = exec
void FreeElfHeader(elfheader_t** head);
const char* ElfName(elfheader_t* head);

// return 0 if OK
int CalcLoadAddr(elfheader_t* head);
int AllocElfMemory(elfheader_t* head);
void FreeElfMemory(elfheader_t* head);
int LoadElfMemory(FILE* f, elfheader_t* head);
int RelocateElf(elfheader_t* head);
void CalcStack(elfheader_t* h, uint32_t* stacksz, int* stackalign);
uintptr_t GetEntryPoint(elfheader_t* h);
uintptr_t GetLastByte(elfheader_t* h);

#endif //__ELF_LOADER_H_