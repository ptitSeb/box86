#ifndef __ELF_LOADER_H_
#define __ELF_LOADER_H_
#include <stdio.h>

typedef struct elfheader_s elfheader_t;
typedef struct lib_s lib_t;
typedef struct kh_mapsymbols_s kh_mapsymbols_t;
typedef struct box86context_s box86context_t;

void* LoadAndCheckElfHeader(FILE* f, const char* name, int exec); // exec : 0 = lib, 1 = exec
void FreeElfHeader(elfheader_t** head);
const char* ElfName(elfheader_t* head);

// return 0 if OK
int CalcLoadAddr(elfheader_t* head);
int AllocElfMemory(elfheader_t* head);
void FreeElfMemory(elfheader_t* head);
int LoadElfMemory(FILE* f, elfheader_t* head);
int RelocateElf(lib_t *maplib, elfheader_t* head);
int RelocateElfPlt(lib_t *maplib, elfheader_t* head);
void CalcStack(elfheader_t* h, uint32_t* stacksz, int* stackalign);
uintptr_t GetEntryPoint(lib_t* maplib, elfheader_t* h);
uintptr_t GetLastByte(elfheader_t* h);
void AddGlobalsSymbols(kh_mapsymbols_t* mapsymbols, elfheader_t* h);
int LoadNeededLib(elfheader_t* h, lib_t *maplib, box86context_t* box86, int pltNow);

#endif //__ELF_LOADER_H_