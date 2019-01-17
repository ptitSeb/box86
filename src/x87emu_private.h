#ifndef __X87RUN_PRIVATE_H_
#define __X87RUN_PRIVATE_H_

#include <stdint.h>
#include "regs.h"
typedef struct x86emu_s x86emu_t;

void RunD8(x86emu_t *emu);
void RunD9(x86emu_t *emu);
void Run66D9(x86emu_t *emu);
void RunDA(x86emu_t *emu);
void RunDB(x86emu_t *emu);
void RunDC(x86emu_t *emu);
void RunDD(x86emu_t *emu);
void Run66DD(x86emu_t *emu);
void RunDE(x86emu_t *emu);
void RunDF(x86emu_t *emu);

#define ST0 emu->fpu[emu->top]
#define ST1 emu->fpu[(emu->top+1)&7]
#define ST(a) emu->fpu[(emu->top+(a))&7]

#define STld(a)  emu->fpu_ld[(emu->top+(a))&7]
#define STll(a)  emu->fpu_ll[(emu->top+(a))&7]

void fpu_do_push(x86emu_t* emu);
void fpu_do_pop(x86emu_t* emu);

void reset_fpu(x86emu_t* emu);

void fpu_fcom(x86emu_t* emu, double b);
void fpu_fcomi(x86emu_t* emu, double b);

double fpu_round(x86emu_t* emu, double d);
void fpu_fxam(x86emu_t* emu);
void fpu_fbst(x86emu_t* emu, uint8_t* d);
void fpu_fbld(x86emu_t* emu, uint8_t* s);

void fpu_loadenv(x86emu_t* emu, char* p, int b16);
void fpu_savenv(x86emu_t* emu, char* p, int b16);

#endif //__X87RUN_PRIVATE_H_