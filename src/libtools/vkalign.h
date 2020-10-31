#include <stdint.h>

void* VulkanFromx86(void* src, void** save);
void VulkanTox86(void* src, void* save);

int vkalignSize(const char* desc);
void* vkalignStruct(void* src, const char* desc, int cnt);
void vkunvkalignStruct(void* dst, void* src, const char* desc, int cnt);