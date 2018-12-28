#ifndef __STACK_H_
#define __STACK_H_

#include <stdint.h>

typedef struct box86context_s box86context_t;
typedef struct x86emu_s x86emu_t;

int CalcStackSize(box86context_t *context);
void SetupInitialStack(box86context_t *context);

uint32_t Pop(x86emu_t *emu);
void Push(x86emu_t *emu, uint32_t v);
uint16_t Pop16(x86emu_t *emu);
void Push16(x86emu_t *emu, uint16_t v);

#endif //__STACK_H_