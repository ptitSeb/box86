#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

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
    emu->packed_eflags.x32 = (f & 0x3F7FD7) | 0x2; // mask off res2 and res3 and on res1
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
}
void arm_ftan(x86emu_t* emu)
{
    ST0.d = tan(ST0.d);
}
void arm_fpatan(x86emu_t* emu)
{
    ST1.d = atan2(ST1.d, ST0.d);
}
void arm_fxtract(x86emu_t* emu)
{
    int32_t tmp32s = (ST1.ll&0x7ff0000000000000LL)>>52;
    tmp32s -= 1023;
    ST1.d /= exp2(tmp32s);
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
}
void arm_fsincos(x86emu_t* emu)
{
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
    fpu_fbld(emu, ed);
}

void arm_fild64(x86emu_t* emu, int64_t* ed)
{
    ST0.d = *ed;
    STll(0).ll = *ed;
    STll(0).ref = ST0.ll;
}

void arm_fbstp(x86emu_t* emu, uint8_t* ed)
{
    fpu_fbst(emu, ed);
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
}

void arm_fld(x86emu_t* emu, uint8_t* ed)
{
    memcpy(&STld(0).ld, ed, 10);
    LD2D(&STld(0), &ST(0).d);
    STld(0).ref = ST0.ll;
}

void arm_cpuid(x86emu_t* emu, uint32_t tmp32u)
{
    switch(tmp32u) {
        case 0x0:
            // emulate a P4
            R_EAX = 0x80000004;
            // return GenuineIntel
            R_EBX = 0x756E6547;
            R_EDX = 0x49656E69;
            R_ECX = 0x6C65746E;
            break;
        case 0x1:
            R_EAX = 0x00000101; // familly and all
            R_EBX = 0;          // Brand indexe, CLFlush, Max APIC ID, Local APIC ID
            R_EDX =   1         // fpu 
                    | 1<<8      // cmpxchg8
                    | 1<<11     // sep (sysenter & sysexit)
                    | 1<<15     // cmov
                    | 1<<19     // clflush (seems to be with SSE2)
                    | 1<<23     // mmx
                    //| 1<<24     // fxsr (fxsave, fxrestore)
                    | 1<<25     // SSE
                    | 1<<26     // SSE2
                    ;
            R_ECX =   1<<0      // SSE3
                    //| 1<<9      // SSSE3
                    | 1<<12     // fma
                    | 1<<13     // cx16 (cmpxchg16)
                    ; 
            if(!box86_steam)
                R_ECX |= (1<<9);    // Disabling SSSE3 for steam for now
            break;
        case 0x2:   // TLB and Cache info. Sending 1st gen P4 info...
            R_EAX = 0x665B5001;
            R_EBX = 0x00000000;
            R_ECX = 0x00000000;
            R_EDX = 0x007A7000;
            break;
        
        case 0x4:   // Cache info
            switch (R_ECX) {
                case 0: // L1 data cache
                    R_EAX = (1 | (1<<5) | (1<<8));   //type
                    R_EBX = (63 | (7<<22)); // size
                    R_ECX = 63;
                    R_EDX = 1;
                    break;
                case 1: // L1 inst cache
                    R_EAX = (2 | (1<<5) | (1<<8)); //type
                    R_EBX = (63 | (7<<22)); // size
                    R_ECX = 63;
                    R_EDX = 1;
                    break;
                case 2: // L2 cache
                    R_EAX = (3 | (2<<5) | (1<<8)); //type
                    R_EBX = (63 | (15<<22));    // size
                    R_ECX = 4095;
                    R_EDX = 1;
                    break;

                default:
                    R_EAX = 0x00000000;
                    R_EBX = 0x00000000;
                    R_ECX = 0x00000000;
                    R_EDX = 0x00000000;
                    break;
            }
            break;
        case 0x5:   //mwait info
            R_EAX = 0;
            R_EBX = 0;
            R_ECX = 1 | 2;
            R_EDX = 0;
            break;
        case 0x6:   // thermal
        case 0x9:   // direct cache access
        case 0xA:   // Architecture performance monitor
            R_EAX = 0;
            R_EBX = 0;
            R_ECX = 0;
            R_EDX = 0;
            break;
        case 0x7:   // extended bits...
            /*if(R_ECX==0)    R_EAX = 0;
            else*/ R_EAX = R_ECX = R_EBX = R_EDX = 0;
            break;

        case 0x80000000:        // max extended
            break;              // no extended, so return same value as beeing the max value!
        default:
            printf_log(LOG_INFO, "Warning, CPUID command %X unsupported (ECX=%08x)\n", tmp32u, R_ECX);
            R_EAX = 0;
    }   
}

void arm_ud(x86emu_t* emu)
{
    kill(getpid(), SIGILL);
}

// Get a FPU single scratch reg
int fpu_get_scratch_single(dynarec_arm_t* dyn)
{
    return dyn->fpu_scratch++;  // return an Sx
}
// Get a FPU double scratch reg
int fpu_get_scratch_double(dynarec_arm_t* dyn)
{
    int i = (dyn->fpu_scratch+1)&(~1);
    dyn->fpu_scratch = i+2;
    return i/2; // return a Dx
}
// Get a FPU quad scratch reg
int fpu_get_scratch_quad(dynarec_arm_t* dyn)
{
    int i = (dyn->fpu_scratch+3)&(~3);
    dyn->fpu_scratch = i+4;
    return i/2; // Return a Dx, not a Qx
}
// Reset scratch regs counter
void fpu_reset_scratch(dynarec_arm_t* dyn)
{
    dyn->fpu_scratch = 0;
}
#define FPUFIRST    8
// Get a FPU double reg
int fpu_get_reg_double(dynarec_arm_t* dyn)
{
    // TODO: check upper limit?
    int i=0;
    while (dyn->fpuused[i]) ++i;
    dyn->fpuused[i] = 1;
    return i+FPUFIRST; // return a Dx
}
// Free a FPU double reg
void fpu_free_reg_double(dynarec_arm_t* dyn, int reg)
{
    // TODO: check upper limit?
    int i=reg-FPUFIRST;
    dyn->fpuused[i] = 0;
}
// Get a FPU quad reg
int fpu_get_reg_quad(dynarec_arm_t* dyn)
{
    int i=0;
    while (dyn->fpuused[i] || dyn->fpuused[i+1]) i+=2;
    dyn->fpuused[i] = dyn->fpuused[i+1] = 1;
    return i+FPUFIRST; // Return a Dx, not a Qx
}
// Free a FPU quad reg
void fpu_free_reg_quad(dynarec_arm_t* dyn, int reg)
{
    int i=reg-FPUFIRST;
    dyn->fpuused[i] = dyn->fpuused[i+1] = 0;
}
// Reset fpu regs counter
void fpu_reset_reg(dynarec_arm_t* dyn)
{
    dyn->fpu_reg = 0;
    for (int i=0; i<24; ++i)
        dyn->fpuused[i]=0;
}
