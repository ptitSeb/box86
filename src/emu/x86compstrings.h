#ifndef __X86_CMPSTRINGS_H__
#define __X86_CMPSTRINGS_H__

#include <stdint.h>

#include "regs.h"

typedef struct x86emu_s x86emu_t;

uint32_t sse42_compare_string_explicit_len(x86emu_t* emu, sse_regs_t* a, int la, sse_regs_t* b, int lb, uint8_t imm8);
uint32_t sse42_compare_string_implicit_len(x86emu_t* emu, sse_regs_t* a, sse_regs_t* b, uint8_t imm8);

#endif //__X86_CMPSTRINGS_H__