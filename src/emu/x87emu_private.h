#ifndef __X87RUN_PRIVATE_H_
#define __X87RUN_PRIVATE_H_

#include <stdint.h>
#include <math.h>
#include "regs.h"
#include "x86run_private.h"
#include "debug.h"
typedef struct x86emu_s x86emu_t;

#define PI		3.14159265358979323846
#define L2E		1.4426950408889634
#define L2T		3.3219280948873623
#define LN2		0.69314718055994531
#define LG2		0.3010299956639812

void Run66D9(x86emu_t *emu);
void Run66DD(x86emu_t *emu);
void RunDF(x86emu_t *emu);

#define ST0 emu->x87[emu->top8]
#define ST1 emu->x87[(emu->top8+1)]
#define ST(a) emu->x87[(emu->top8+(a))]

#define STld(a)  emu->fpu_ld[(emu->top8+(a))]
#define STll(a)  emu->fpu_ll[(emu->top8+(a))]

static inline void fpu_do_push(x86emu_t* emu)
{
    int newtop = (emu->top8-1);
    if(emu->fpu_stack<8)
        ++emu->fpu_stack;
    else {
        emu->sw.f.F87_C1 = 1;
        return;
    }
    emu->sw.f.F87_C1 = 0;
    emu->p_regs[newtop].tag = 0;    // full
    emu->top8 = newtop;
}

static inline void fpu_do_pop(x86emu_t* emu)
{
    int curtop = emu->top8;
    if(emu->fpu_stack>0)
        --emu->fpu_stack;
    
    emu->p_regs[curtop].tag = 0b11;    // empty
    emu->top8 = emu->top8+1;
}

static inline void fpu_do_free(x86emu_t* emu, int i)
{
    emu->p_regs[emu->top8+i].tag = 0b11;    // empty
    // check if all empty
    for(int i=0; i<8; ++i)
        if(emu->p_regs[i].tag != 0b11)
            return;
    emu->fpu_stack = 0;
}

void reset_fpu(x86emu_t* emu);

static inline void fpu_fcom(x86emu_t* emu, double b)
{
    emu->sw.f.F87_C1 = 0;
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
    } 
    else {
        emu->sw.f.F87_C0 = 0;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C3 = 1;
    }
}

static inline void fpu_fcomi(x86emu_t* emu, double b)
{
    RESET_FLAGS(emu);
    CLEAR_FLAG(F_AF);
    CLEAR_FLAG(F_OF);
    CLEAR_FLAG(F_SF);
    emu->sw.f.F87_C1 = 0;
    if(isnan(ST0.d) || isnan(b)) {
        SET_FLAG(F_CF);
        SET_FLAG(F_PF);
        SET_FLAG(F_ZF);
    } else if (isgreater(ST0.d, b)) {
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_PF);
        CLEAR_FLAG(F_ZF);
    } else if (isless(ST0.d, b)) {
        SET_FLAG(F_CF);
        CLEAR_FLAG(F_PF);
        CLEAR_FLAG(F_ZF);
    } 
    else {
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_PF);
        SET_FLAG(F_ZF);
    }
}

static inline double fpu_round(x86emu_t* emu, double d) {
    if (!isfinite(d))
        return d;
    switch(emu->round) {
        case ROUND_Nearest:
            return nearbyint(d);
        case ROUND_Down:
            return floor(d);
        case ROUND_Up:
            return ceil(d);
        case ROUND_Chop:
        default:
            return trunc(d);
    }
}

void fpu_fxam(x86emu_t* emu);

static inline void fpu_ftst(x86emu_t* emu) {
    emu->sw.f.F87_C1 = 0;
    if(isnan(ST0.d)) 
    {  // TODO: Unsuported and denormal not analysed...
        emu->sw.f.F87_C3 = 1;
        emu->sw.f.F87_C2 = 1;
        emu->sw.f.F87_C0 = 1;
        return;
    }
    if(ST0.d==0.0)
    {
        emu->sw.f.F87_C3 = 1;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C0 = 0;
        return;
    }
    // normal...
    emu->sw.f.F87_C3 = 0;
    emu->sw.f.F87_C2 = 0;
    emu->sw.f.F87_C0 = (ST0.ud[1]&0x80000000)?1:0;
}

void fpu_fbst(x86emu_t* emu, uint8_t* d);
void fpu_fbld(x86emu_t* emu, uint8_t* s);

void fpu_loadenv(x86emu_t* emu, char* p, int b16);
void fpu_savenv(x86emu_t* emu, char* p, int b16);
void fpu_fxsave(x86emu_t* emu, void* ed);
void fpu_fxrstor(x86emu_t* emu, void* ed);

#endif //__X87RUN_PRIVATE_H_
