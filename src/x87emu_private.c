#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "debug.h"
#include "x86emu_private.h"
#include "x87emu_private.h"

void fpu_do_push(x86emu_t* emu)
{
    ++emu->fpu_stack;
    if(emu->fpu_stack == 9) {// overflow
        printf_log(LOG_NONE, "Error: %p: FPU Stack overflow\n", emu->old_ip);    // probably better to raise something
        emu->quit = 1;
        return;
    }
    emu->top = (emu->top-1)&7;
}

void fpu_do_pop(x86emu_t* emu)
{
    emu->top = (emu->top+1)&7;
    --emu->fpu_stack;
    if(emu->fpu_stack < 0) {// underflow
        printf_log(LOG_NONE, "Error: %p: FPU Stack underflow\n", emu->old_ip);    // probably better to raise something
        emu->quit = 1;
        return;
    }

}

void reset_fpu(x86emu_t* emu)
{
    memset(emu->fpu, 0, sizeof(emu->fpu));
    memset(emu->fpu_ld, 0, sizeof(emu->fpu_ld));
    emu->cw = 0x37F;
    emu->sw.x16 = 0x0000;
    emu->top = 0;
    emu->fpu_stack = 0;
}

void fpu_fcom(x86emu_t* emu, double b)
{
    if(!isfinite(ST0.d) || !isfinite(b)) {
        emu->sw.f.F87_C0 = 1;
        emu->sw.f.F87_C2 = 1;
        emu->sw.f.F87_C3 = 1;
    } else if (ST0.d>b) {
        emu->sw.f.F87_C0 = 0;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C3 = 0;
    } else if (ST0.d<b) {
        emu->sw.f.F87_C0 = 1;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C3 = 0;
    } else {
        emu->sw.f.F87_C0 = 0;
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C3 = 1;
    }
}

void fpu_fcomi(x86emu_t* emu, double b)
{
    if(!isfinite(ST0.d) || !isfinite(b)) {
        emu->eflags.f.F_CF = 1;
        emu->eflags.f.F_PF = 1;
        emu->eflags.f.F_ZF = 1;
    } else if (ST0.d>b) {
        emu->eflags.f.F_CF = 0;
        emu->eflags.f.F_PF = 0;
        emu->eflags.f.F_ZF = 0;
    } else if (ST0.d<b) {
        emu->eflags.f.F_CF = 1;
        emu->eflags.f.F_PF = 0;
        emu->eflags.f.F_ZF = 0;
    } else {
        emu->eflags.f.F_CF = 0;
        emu->eflags.f.F_PF = 0;
        emu->eflags.f.F_ZF = 1;
    }
}