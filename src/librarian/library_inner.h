#ifndef __LIBRARY_INNER_H__
#define __LIBRARY_INNER_H__

void NativeLib_CommonInit(library_t* lib);
void NativeLib_FinishFini(library_t* lib);
int NativeLib_defget(library_t* lib, const char* name, uintptr_t *offs, uint32_t *sz, int* weak, int version, const char* vername, int local);
int NativeLib_defgetnoweak(library_t* lib, const char* name, uintptr_t *offs, uint32_t *sz, int* weak, int version, const char* vername, int local);

#endif