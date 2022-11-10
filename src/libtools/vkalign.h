#include <stdint.h>

void* VulkanFromx86(void* src);
void* VulkanTox86(void* src);

int vkalignSize(const char* desc);
void* vkalignStruct(void* src, const char* desc, int cnt);
void* vkunalignStruct(void* src, const char* desc, int cnt);
void* vkStructUnalign(void* src, const char* desc, int cnt);
void* vkunalignNewStruct(void* dst, void* src, const char* desc, int cnt);
