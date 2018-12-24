#ifndef __LIBRARIAN_H_
#define __LIBRARIAN_H_
#include <stdint.h>

typedef struct lib_s lib_t;
typedef struct bridge_s bridge_t;

lib_t *NewLibrarian();
void FreeLibrarian(lib_t **maplib);

void AddSymbol(lib_t *maplib, const char* name, uintptr_t addr, uint32_t sz);
uintptr_t FindSymbol(lib_t *maplib, const char* name);
int GetSymbolStartEnd(lib_t* maplib, const char* name, uintptr_t* start, uintptr_t* end);

#endif //__LIBRARIAN_H_