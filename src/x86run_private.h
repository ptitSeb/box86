#ifndef __X86RUN_PRIVATE_H_
#define __X86RUN_PRIVATE_H_

#include <stdint.h>
#include "regs.h"
#include "x86emu_private.h"
typedef struct x86emu_s x86emu_t;

void GetEb(x86emu_t *emu, reg32_t **op, reg32_t *ea, uint32_t v);
void GetEw(x86emu_t *emu, reg32_t **op, reg32_t *ea, uint32_t v);
void GetEd(x86emu_t *emu, reg32_t **op, reg32_t *ea, uint32_t v);
void GetEx(x86emu_t *emu, sse_regs_t **op, sse_regs_t *ea, uint32_t v);
void GetG(x86emu_t *emu, reg32_t **op, uint32_t v);
void GetGb(x86emu_t *emu, reg32_t **op, uint32_t v);
void GetGx(x86emu_t *emu, sse_regs_t **op, uint32_t v);

inline uint8_t Fetch8(x86emu_t *emu) {return *(uint8_t*)(R_EIP++);}
int8_t Fetch8s(x86emu_t *emu);
uint16_t Fetch16(x86emu_t *emu);
int16_t Fetch16s(x86emu_t *emu);
uint32_t Fetch32(x86emu_t *emu);
int32_t Fetch32s(x86emu_t *emu);


void Run66(x86emu_t *emu);
void Run67(x86emu_t *emu);
void Run0F(x86emu_t *emu);
void Run660F(x86emu_t *emu);
void RunF30F(x86emu_t *emu);

void x86Syscall(x86emu_t *emu);
void x86Int3(x86emu_t* emu);

const char* GetNativeName(void* p);


#endif //__X86RUN_PRIVATE_H_