#ifndef __X86RUN_H_
#define __X86RUN_H_
#include <stdint.h>

typedef struct x86emu_s x86emu_t;
int Run(x86emu_t *emu, int step); // 0 if run was successfull, 1 if error in x86 world
int DynaRun(x86emu_t *emu);

uint32_t LibSyscall(x86emu_t *emu);
void PltResolver(x86emu_t* emu, uint32_t id, uintptr_t ofs);
extern uintptr_t pltResolver;


#endif //__X86RUN_H_