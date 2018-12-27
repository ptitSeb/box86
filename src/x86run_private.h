#ifndef __X86RUN_PRIVATE_H_
#define __X86RUN_PRIVATE_H_

#include <stdint.h>
#include "regs.h"
typedef struct x86emu_s x86emu_t;

void GetEb(x86emu_t *emu, reg32_t **op, reg32_t *ea, uint32_t v);
void GetEw(x86emu_t *emu, reg32_t **op, reg32_t *ea, uint32_t v);
void GetEd(x86emu_t *emu, reg32_t **op, reg32_t *ea, uint32_t v);
void GetG(x86emu_t *emu, reg32_t **op, uint32_t v);
void GetGb(x86emu_t *emu, reg32_t **op, uint32_t v);

void Run66(x86emu_t *emu);
void Run0F(x86emu_t *emu);

void x86Syscall(x86emu_t *emu);
void x86Int3(x86emu_t* emu);

// fpu stuffs


#endif //__X86RUN_PRIVATE_H_