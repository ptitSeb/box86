#ifndef __LIBRARIAN_H_
#define __LIBRARIAN_H_
#include <stdint.h>

typedef struct lib_s lib_t;

lib_t *NewLibrarian();
void FreeLibrarian(lib_t **maplib);

void AddSymbol(lib_t *maplib, const char* name, uintptr_t addr);
uintptr_t FindSymbol(lib_t *maplib, const char* name);

#endif //__LIBRARIAN_H_