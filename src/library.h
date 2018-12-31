#ifndef __LIBRARY_H_
#define __LIBRARY_H_
#include <stdint.h>

typedef struct library_s library_t;

library_t *NewLibrary(const char* path, void* box86);
void FreeLibrary(library_t **lib);

char* GetNameLib(library_t *lib);
int IsSameLib(library_t* lib, const char* path);    // check if lib is same (path -> name)
int GetLibSymbolStartEnd(library_t* lib, const char* name, uintptr_t* start, uintptr_t* end);

#endif //__LIBRARY_H_