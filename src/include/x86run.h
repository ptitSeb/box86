#ifndef __X86RUN_H_
#define __X86RUN_H_
#include <stdint.h>

typedef struct x86emu_s x86emu_t;
typedef struct x86test_s x86test_t;
int Run(x86emu_t *emu, int step); // 0 if run was successfull, 1 if error in x86 world
int RunTest(x86test_t *test);
int DynaRun(x86emu_t *emu);

uint32_t LibSyscall(x86emu_t *emu);
void PltResolver(x86emu_t* emu);
extern uintptr_t pltResolver;
int GetTID();

#endif //__X86RUN_H_