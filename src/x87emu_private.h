#ifndef __X87RUN_PRIVATE_H_
#define __X87RUN_PRIVATE_H_

#include <stdint.h>
#include <math.h>
#include "regs.h"
#include "x86run_private.h"
#include "debug.h"
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

static inline void fpu_do_push(x86emu_t* emu)
{
    ++emu->fpu_stack;
    if(emu->fpu_stack == 9) {// overflow
        printf_log(LOG_NONE, "Error: %p: FPU Stack overflow\n", (void*)emu->old_ip);    // probably better to raise something
        emu->quit = 1;
        return;
    }
    emu->top = (emu->top-1)&7;
}

static inline void fpu_do_pop(x86emu_t* emu)
{
    emu->top = (emu->top+1)&7;
    --emu->fpu_stack;
    if(emu->fpu_stack < 0) {// underflow
        printf_log(LOG_NONE, "Error: %p: FPU Stack underflow\n", (void*)emu->old_ip);    // probably better to raise something
        emu->quit = 1;
        return;
    }

}

void reset_fpu(x86emu_t* emu);

static inline void fpu_fcom(x86emu_t* emu, double b)
{
    if(isnan(ST0.d) || isnan(b)) {
        emu->sw.f.F87_C0 = 1;
        emu->sw.f.F87_C2 = 1;
        emu->sw.f.F87_C3 = 1;
    } else if (isgreater(ST0.d, b)) {
        emu->sw.f.F87_C0 = 0;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C3 = 0;
    } else if (isless(ST0.d, b)) {
        emu->sw.f.F87_C0 = 1;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C3 = 0;
    } else {
        emu->sw.f.F87_C0 = 0;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C3 = 1;
    }
}

static inline void fpu_fcomi(x86emu_t* emu, double b)
{
    RESET_FLAGS(emu);
    if(isnan(ST0.d) || isnan(b)) {
        emu->eflags.f.F_CF = 1;
        emu->eflags.f.F_PF = 1;
        emu->eflags.f.F_ZF = 1;
    } else if (isgreater(ST0.d, b)) {
        emu->eflags.f.F_CF = 0;
        emu->eflags.f.F_PF = 0;
        emu->eflags.f.F_ZF = 0;
    } else if (isless(ST0.d, b)) {
        emu->eflags.f.F_CF = 1;
        emu->eflags.f.F_PF = 0;
        emu->eflags.f.F_ZF = 0;
    } else {
        emu->eflags.f.F_CF = 0;
        emu->eflags.f.F_PF = 0;
        emu->eflags.f.F_ZF = 1;
    }
}

static inline double fpu_round(x86emu_t* emu, double d) {
    if (!isfinite(d))
        return d;
    //switch(emu->cw)   // TODO: implement Rounding...
    return round(d);
}

static inline void fpu_fxam(x86emu_t* emu) {
    emu->sw.f.F87_C1 = (ST0.l.upper<0)?1:0;
    if(!emu->fpu_stack) {
        emu->sw.f.F87_C3 = 1;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C0 = 1;
        return;
    }
    if(isinf(ST0.d)) {  // TODO: Unsuported and denormal not analysed...
        emu->sw.f.F87_C3 = 0;
        emu->sw.f.F87_C2 = 1;
        emu->sw.f.F87_C0 = 1;
        return;
    }
    if(isnan(ST0.d)) {  // TODO: Unsuported and denormal not analysed...
        emu->sw.f.F87_C3 = 0;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C0 = 1;
        return;
    }
    if(ST0.d==0.0) {
        emu->sw.f.F87_C3 = 1;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C0 = 0;
        return;
    }
    // normal...
    emu->sw.f.F87_C3 = 0;
    emu->sw.f.F87_C2 = 1;
    emu->sw.f.F87_C0 = 0;

}

void fpu_fbst(x86emu_t* emu, uint8_t* d);
void fpu_fbld(x86emu_t* emu, uint8_t* s);

void fpu_loadenv(x86emu_t* emu, char* p, int b16);
void fpu_savenv(x86emu_t* emu, char* p, int b16);

#endif //__X87RUN_PRIVATE_H_