#ifndef __LIBRARY_H_
#define __LIBRARY_H_
#include <stdint.h>

typedef struct library_s       library_t;
typedef struct kh_symbolmap_s  kh_symbolmap_t;
typedef struct box86context_s  box86context_t;
typedef struct x86emu_s        x86emu_t;

library_t *NewLibrary(const char* path, box86context_t* box86);
int FinalizeLibrary(library_t* lib, x86emu_t* emu);
void FreeLibrary(library_t **lib);

char* GetNameLib(library_t *lib);
int IsSameLib(library_t* lib, const char* path);    // check if lib is same (path -> name)
int GetLibSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end);
kh_symbolmap_t * fillGLProcWrapper();
void freeGLProcWrapper(kh_symbolmap_t** symbolmap);

int GetElfIndex(library_t* lib);    // -1 if no elf (i.e. native)

#endif //__LIBRARY_H_