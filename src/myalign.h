#include <stdint.h>

void myStackAlign(const char* fmt, uint32_t* st, uint32_t* mystack);
void myStackAlignW(const char* fmt, uint32_t* st, uint32_t* mystack);

void UnalignStat64(void* source, void* dest);
