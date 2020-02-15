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

#define ST0 emu->fpu[emu->top]
#define ST1 emu->fpu[(emu->top+1)&7]
#define ST(a) emu->fpu[(emu->top+(a))&7]

#define STld(a)  emu->fpu_ld[(emu->top+(a))&7]
#define STll(a)  emu->fpu_ll[(emu->top+(a))&7]

static inline void fpu_do_push(x86emu_t* emu)
{
    int newtop = (emu->top-1)&7;
    if(emu->p_regs[newtop].tag!=0b11) {// not empty, overflow!
        printf_log(LOG_NONE, "Warning: %p: FPU Stack overflow\n", (void*)emu->old_ip);    // probably better to raise something
        //emu->quit = 1;
        return;
    }
    if(emu->fpu_stack<8)
        ++emu->fpu_stack; 
    emu->p_regs[newtop].tag = 0;    // full
    emu->top = newtop;
}

static inline void fpu_do_pop(x86emu_t* emu)
{
    int curtop = (emu->top)&7;
    if(emu->p_regs[(emu->top)&7].tag==0b11) {// underflow
        printf_log(LOG_NONE, "Warning: %p: FPU Stack underflow\n", (void*)emu->old_ip);    // probably better to raise something
        //emu->quit = 1;
        return;
    }
    if(emu->fpu_stack>0)
        --emu->fpu_stack;
    
    emu->p_regs[curtop].tag = 0b11;    // empty
    emu->top = (emu->top+1)&7;
}

static inline void fpu_do_free(x86emu_t* emu, int i)
{
    emu->p_regs[(emu->top+i)&7].tag = 0b11;    // empty
    // check if all empty
    for(int i=0; i<8; ++i)
        if(emu->p_regs[i].tag != 0b11)
            return;
    emu->fpu_stack = 0;
}

void reset_fpu(x86emu_t* emu);

#ifdef USE_FLOAT
static inline void fpu_fcom(x86emu_t* emu, float b)
#else
static inline void fpu_fcom(x86emu_t* emu, double b)
#endif
{
    emu->sw.f.F87_C1 = 0;
    #ifdef USE_FLOAT
    if(isnan(ST0.f) || isnan(b)) {
        emu->sw.f.F87_C0 = 1;
        emu->sw.f.F87_C2 = 1;
        emu->sw.f.F87_C3 = 1;
    } else if (isgreater(ST0.f, b)) {
        emu->sw.f.F87_C0 = 0;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C3 = 0;
    } else if (isless(ST0.f, b)) {
        emu->sw.f.F87_C0 = 1;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C3 = 0;
    } 
    #else
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
    #endif
    else {
        emu->sw.f.F87_C0 = 0;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C3 = 1;
    }
}

#ifdef USE_FLOAT
static inline void fpu_fcomi(x86emu_t* emu, float b)
#else
static inline void fpu_fcomi(x86emu_t* emu, double b)
#endif
{
    RESET_FLAGS(emu);
    emu->sw.f.F87_C1 = 0;
    #ifdef USE_FLOAT
    if(isnan(ST0.f) || isnan(b)) {
        emu->flags[F_CF] = 1;
        emu->flags[F_PF] = 1;
        emu->flags[F_ZF] = 1;
    } else if (isgreater(ST0.f, b)) {
        emu->flags[F_CF] = 0;
        emu->flags[F_PF] = 0;
        emu->flags[F_ZF] = 0;
    } else if (isless(ST0.f, b)) {
        emu->flags[F_CF] = 1;
        emu->flags[F_PF] = 0;
        emu->flags[F_ZF] = 0;
    } 
    #else
    if(isnan(ST0.d) || isnan(b)) {
        emu->flags[F_CF] = 1;
        emu->flags[F_PF] = 1;
        emu->flags[F_ZF] = 1;
    } else if (isgreater(ST0.d, b)) {
        emu->flags[F_CF] = 0;
        emu->flags[F_PF] = 0;
        emu->flags[F_ZF] = 0;
    } else if (isless(ST0.d, b)) {
        emu->flags[F_CF] = 1;
        emu->flags[F_PF] = 0;
        emu->flags[F_ZF] = 0;
    } 
    #endif
    else {
        emu->flags[F_CF] = 0;
        emu->flags[F_PF] = 0;
        emu->flags[F_ZF] = 1;
    }
}

#ifdef USE_FLOAT
static inline float fpu_round(x86emu_t* emu, float d) {
    if (!isfinite(d))
        return d;
    switch(emu->round) {
        case ROUND_Nearest:
            return floorf(d+0.5);
        case ROUND_Down:
            return floorf(d);
        case ROUND_Up:
            return ceilf(d);
        case ROUND_Chop:
        default:
            return roundf(d);
    }
}
#else
static inline double fpu_round(x86emu_t* emu, double d) {
    if (!isfinite(d))
        return d;
    switch(emu->round) {
        case ROUND_Nearest:
            return floor(d+0.5);
        case ROUND_Down:
            return floor(d);
        case ROUND_Up:
            return ceil(d);
        case ROUND_Chop:
        default:
            return round(d);
    }
}
#endif

static inline void fpu_fxam(x86emu_t* emu) {
    #ifdef USE_FLOAT
    emu->sw.f.F87_C1 = (ST0.ll&0x8000)?1:0;
    #else
    emu->sw.f.F87_C1 = (ST0.l.upper&0x80000000)?1:0;
    #endif
    if(!emu->fpu_stack) {
        emu->sw.f.F87_C3 = 1;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C0 = 1;
        return;
    }
    #ifdef USE_FLOAT
    if(isinf(ST0.f))
    #else
    if(isinf(ST0.d)) 
    #endif
    {  // TODO: Unsuported and denormal not analysed...
        emu->sw.f.F87_C3 = 0;
        emu->sw.f.F87_C2 = 1;
        emu->sw.f.F87_C0 = 1;
        return;
    }
    #ifdef USE_FLOAT
    if(isnan(ST0.f))
    #else
    if(isnan(ST0.d))
    #endif
    {  // TODO: Unsuported and denormal not analysed...
        emu->sw.f.F87_C3 = 0;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C0 = 1;
        return;
    }
    #ifdef USE_FLOAT
    if(ST0.f==0.0f)
    #else
    if(ST0.d==0.0)
    #endif
    {
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

static inline void fpu_ftst(x86emu_t* emu) {
    emu->sw.f.F87_C1 = 0;
    #ifdef USE_FLOAT
    if(isinf(ST0.f) || isnan(ST0.f))
    #else
    if(isinf(ST0.d) || isnan(ST0.d)) 
    #endif
    {  // TODO: Unsuported and denormal not analysed...
        emu->sw.f.F87_C3 = 1;
        emu->sw.f.F87_C2 = 1;
        emu->sw.f.F87_C0 = 1;
        return;
    }
    #ifdef USE_FLOAT
    if(ST0.f==0.0f)
    #else
    if(ST0.d==0.0)
    #endif
    {
        emu->sw.f.F87_C3 = 1;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C0 = 0;
        return;
    }
    // normal...
    emu->sw.f.F87_C3 = 0;
    emu->sw.f.F87_C2 = 0;
    #ifdef USE_FLOAT
    emu->sw.f.F87_C0 = (ST0.ll&0x8000)?1:0;
    #else
    emu->sw.f.F87_C0 = (ST0.l.upper&0x80000000)?1:0;
    #endif
}

void fpu_fbst(x86emu_t* emu, uint8_t* d);
void fpu_fbld(x86emu_t* emu, uint8_t* s);

void fpu_loadenv(x86emu_t* emu, char* p, int b16);
void fpu_savenv(x86emu_t* emu, char* p, int b16);

#endif //__X87RUN_PRIVATE_H_
