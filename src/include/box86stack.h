#ifndef __BOX86_STACK_H_
#define __BOX86_STACK_H_

#include <stdint.h>

typedef struct box86context_s box86context_t;
typedef struct x86emu_s x86emu_t;

int CalcStackSize(box86context_t *context);
void SetupInitialStack(x86emu_t *emu);

uint16_t Pop16(x86emu_t *emu);
void Push16(x86emu_t *emu, uint16_t v);
uint32_t Pop32(x86emu_t *emu);
void Push32(x86emu_t *emu, uint32_t v);

#endif //__BOX86_STACK_H_