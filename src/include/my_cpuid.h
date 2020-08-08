#ifndef __MY_CPUID_H__
#define __MY_CPUID_H__
#include <stdint.h>
typedef struct x86emu_s x86emu_t;

void my_cpuid(x86emu_t* emu, uint32_t tmp32u);

#endif //__MY_CPUID_H__