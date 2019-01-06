#ifndef __X86RUN_H_
#define __X86RUN_H_
#include <stdint.h>

typedef struct x86emu_s x86emu_t;
int Run(x86emu_t *emu); // 0 if run was successfull, 1 if error in x86 world

uint8_t Peek(x86emu_t *emu, int offset);

uint32_t LibSyscall(x86emu_t *emu);


#endif //__X86RUN_H_