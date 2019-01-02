#ifndef __X87RUN_PRIVATE_H_
#define __X87RUN_PRIVATE_H_

#include <stdint.h>
#include "regs.h"
typedef struct x86emu_s x86emu_t;

void RunD8(x86emu_t *emu);
void RunD9(x86emu_t *emu);
void RunDB(x86emu_t *emu);
void RunDC(x86emu_t *emu);
void RunDD(x86emu_t *emu);
void RunDE(x86emu_t *emu);
void RunDF(x86emu_t *emu);

#define ST0 emu->fpu[emu->top]
#define ST1 emu->fpu[(emu->top+1)&7]
#define ST(a) emu->fpu[(emu->top+(a))&7]

void fpu_do_push(x86emu_t* emu);
void fpu_do_pop(x86emu_t* emu);

void reset_fpu(x86emu_t* emu);

void fpu_fcom(x86emu_t* emu, double b);

#endif //__X87RUN_PRIVATE_H_