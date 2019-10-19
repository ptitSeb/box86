#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include "debug.h"
#include "box86context.h"
#include "dynarec.h"
#include "emu/x86emu_private.h"
#include "tools/bridge_private.h"
#include "x86run.h"
#include "x86emu.h"
#include "box86stack.h"
#include "callback.h"
#include "emu/x86run_private.h"
#include "emu/x87emu_private.h"
#include "x86trace.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"
#include "dynarec_arm_functions.h"

void arm_popf(x86emu_t* emu, uint32_t f)
{
    emu->packed_eflags.x32 = ((f & 0x3F7FD7) & (0xffff-40) ) | 0x2; // mask off res2 and res3 and on res1
    UnpackFlags(emu);
    RESET_FLAGS(emu);
}

void arm_fstp(x86emu_t* emu, void* p)
{
    if(ST0.ll!=STld(0).ref)
        D2LD(&ST0.d, p);
    else
        memcpy(p, &STld(0).ld, 10);
}

void arm_print_armreg(x86emu_t* emu, uintptr_t reg, uintptr_t n)
{
    dynarec_log(LOG_DEBUG, "R%d=0x%x (%d)\n", n, reg, reg);
}

void arm_f2xm1(x86emu_t* emu)
{
    ST0.d = exp2(ST0.d) - 1.0;
}
void arm_fyl2x(x86emu_t* emu)
{
    ST(1).d = log2(ST0.d)*ST(1).d;
    fpu_do_pop(emu);
}
void arm_ftan(x86emu_t* emu)
{
    ST0.d = tan(ST0.d);
    fpu_do_push(emu);
    ST0.d = 1.0;

}
void arm_fpatan(x86emu_t* emu)
{
    ST1.d = atan2(ST1.d, ST0.d);
    fpu_do_pop(emu);
}
void arm_fxtract(x86emu_t* emu)
{
    int32_t tmp32s = (ST0.ll&0x7ff0000000000000LL)>>52;
    tmp32s -= 1023;
    ST0.d /= exp2(tmp32s);
    fpu_do_push(emu);
    ST0.d = tmp32s;
}
void arm_fprem(x86emu_t* emu)
{
    int32_t tmp32s = ST0.d / ST1.d;
    ST0.d -= ST1.d * tmp32s;
    emu->sw.f.F87_C2 = 0;
    emu->sw.f.F87_C0 = (tmp32s&1);
    emu->sw.f.F87_C3 = ((tmp32s>>1)&1);
    emu->sw.f.F87_C1 = ((tmp32s>>2)&1);
}
void arm_fyl2xp1(x86emu_t* emu)
{
    ST(1).d = log2(ST0.d + 1.0)*ST(1).d;
    fpu_do_pop(emu);
}
void arm_fsincos(x86emu_t* emu)
{
    fpu_do_push(emu);
    sincos(ST1.d, &ST1.d, &ST0.d);
}
void arm_frndint(x86emu_t* emu)
{
    ST0.d = fpu_round(emu, ST0.d);
}
void arm_fscale(x86emu_t* emu)
{
    ST0.d *= exp2(trunc(ST1.d));
}
void arm_fsin(x86emu_t* emu)
{
    ST0.d = sin(ST0.d);
}
void arm_fcos(x86emu_t* emu)
{
    ST0.d = cos(ST0.d);
}

void arm_fbld(x86emu_t* emu, uint8_t* ed)
{
    fpu_do_push(emu);
    fpu_fbld(emu, ed);
}

void arm_fild64(x86emu_t* emu, int64_t* ed)
{
    fpu_do_push(emu);
    ST0.d = *ed;
    STll(0).ll = *ed;
    STll(0).ref = ST0.ll;
}

void arm_fbstp(x86emu_t* emu, uint8_t* ed)
{
    fpu_fbst(emu, ed);
    fpu_do_pop(emu);
}

void arm_fistp64(x86emu_t* emu, int64_t* ed)
{
    if(STll(0).ref==ST(0).ll) {
        *ed = STll(0).ll;
    } else {
        if(isgreater(ST0.d, (double)(int64_t)0x7fffffffffffffffLL) || isless(ST0.d, (double)(int64_t)0x8000000000000000LL))
            *ed = 0x8000000000000000LL;
        else
            *ed = (int64_t)ST0.d;
    }
    fpu_do_pop(emu);
}

void arm_fld(x86emu_t* emu, uint8_t* ed)
{
    fpu_do_push(emu);
    memcpy(&STld(0).ld, ed, 10);
    LD2D(&STld(0), &ST(0).d);
    STld(0).ref = ST0.ll;
}