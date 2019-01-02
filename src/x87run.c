#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x87emu_private.h"
#include "x86primop.h"
#include "x86trace.h"

#define PI		3.14159265358979323846
#define L2E		1.4426950408889634
#define L2T		3.3219280948873623
#define LN2		0.69314718055994531
#define LG2		0.3010299956639812

void RunD8(x86emu_t *emu)
{
    uint8_t nextop;
    reg32_t *op1, *op2;
    reg32_t ea1, ea2;
    int32_t tmp32s;
    float f;
    double d;
    long double ld;
    nextop = Fetch8(emu);
    switch(nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:  /* FADD */
            ST0.d += ST(nextop&7).d;
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:  /* FMUL */
            ST0.d *= ST(nextop&7).d;
            break;
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:  /* FCOM */
            fpu_fcom(emu, ST(nextop&7).d);
            break;
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:  /* FCOMP */
            fpu_fcom(emu, ST(nextop&7).d);
            fpu_do_pop(emu);
            break;
        case 0xE0:
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:  /* FSUB */
            ST0.d -= ST(nextop&7).d;
            break;
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:  /* FSUBR */
            ST0.d = ST(nextop&7).d - ST0.d;
            break;
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:  /* FDIV */
            ST0.d /= ST(nextop&7).d;
            break;
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:  /* FDIVR */
            ST0.d = ST(nextop&7).d / ST0.d;
            break;
        default:
        switch((nextop>>3)&7) {
            case 0:         /* FADD */
                GetEd(emu, &op2, &ea2, nextop);
                *(uint32_t*)&f = op2->dword[0];
                ST0.d += f;
                break;
            case 1:         /* FMUL */
                GetEd(emu, &op2, &ea2, nextop);
                *(uint32_t*)&f = op2->dword[0];
                ST0.d *= f;
                break;
            case 2:      /* FCOM */
                GetEd(emu, &op2, &ea2, nextop);
                *(uint32_t*)&f = op2->dword[0];
                fpu_fcom(emu, f);
                break;
            case 3:     /* FCOMP */
                GetEd(emu, &op2, &ea2, nextop);
                *(uint32_t*)&f = op2->dword[0];
                fpu_fcom(emu, f);
                fpu_do_pop(emu);
                break;
            case 4:         /* FSUB */
                GetEd(emu, &op2, &ea2, nextop);
                *(uint32_t*)&f = op2->dword[0];
                ST0.d -= f;
                break;
            case 5:         /* FSUBR */
                GetEd(emu, &op2, &ea2, nextop);
                *(uint32_t*)&f = op2->dword[0];
                ST0.d = f - ST0.d;
                break;
            case 6:         /* FDIV */
                GetEd(emu, &op2, &ea2, nextop);
                *(uint32_t*)&f = op2->dword[0];
                ST0.d /= f;
                break;
            case 7:         /* FDIVR */
                GetEd(emu, &op2, &ea2, nextop);
                *(uint32_t*)&f = op2->dword[0];
                ST0.d = f / ST0.d;
                break;
            default:
                UnimpOpcode(emu);
        }
    }
}

void RunD9(x86emu_t *emu)
{
    uint8_t nextop;
    reg32_t *op1, *op2;
    reg32_t ea1, ea2;
    float f;
    double d;
    long double ld;
    int64_t ll;
    nextop = Fetch8(emu);
    switch (nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:  /* FLD */
            ll = ST(nextop&7).ll;
            fpu_do_push(emu);
            ST0.ll = ll;
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:  /* FXCH */
            ll = ST(nextop&7).ll;
            ST(nextop&7).ll = ST0.ll;
            ST0.ll = ll;
            break;

        case 0xD0:  /* FNOP */
            break;

        case 0xE0:  /* FCHS */
            ST0.d = -ST0.d;
            break;

        case 0xEE:  /* FLDZ */
            fpu_do_push(emu);
            ST0.d = 0.0;
            break;
        case 0xE8:  /* FLD1 */
            fpu_do_push(emu);
            ST0.d = 1.0;
            break;
        case 0xE9:  /* FLDL2T */
            fpu_do_push(emu);
            ST0.d = L2T;
            break;
        case 0xEA:  /* FLDL2E */
            fpu_do_push(emu);
            ST0.d = L2E;
            break;
        case 0xEB:  /* FLDPI */
            fpu_do_push(emu);
            ST0.d = PI;
            break;
        case 0xEC:  /* FLDLG2 */
            fpu_do_push(emu);
            ST0.d = LG2;
            break;
        case 0xED:  /* FLDLN2 */
            fpu_do_push(emu);
            ST0.d = LN2;
            break;
        default:
        switch((nextop>>3)&7) {
            case 0:     /* FLD ST0, Ed float */
                GetEd(emu, &op2, &ea2, nextop);
                *(uint32_t*)&f = op2->dword[0];
                fpu_do_push(emu);
                ST0.d = f;
                break;
            case 2:     /* FST Ed, ST0 */
                GetEd(emu, &op1, &ea1, nextop);
                f = ST0.d;
                op1->dword[0] = *(uint32_t*)&f;
                break;
            case 3:     /* FSTP Ed, ST0 float with partial alias on mod=3=>ST1 */
                GetEd(emu, &op1, &ea1, nextop);
                /*if((nextop>>6)==3)    => TODO: to check
                    f = ST1.d;
                else*/
                    f = ST0.d;
                fpu_do_pop(emu);
                op1->dword[0] = *(uint32_t*)&f;
                break;
            case 5:     /* FLDCW Ew */
                GetEw(emu, &op1, &ea1, nextop);
                emu->cw = op1->word[0];
                // do something with cw?
                emu->round = (fpu_round_t)((emu->cw >> 10) & 3);
                break;
            case 7: /* FNSTCW Ew */
                GetEw(emu, &op1, &ea1, nextop);
                op1->word[0] = emu->cw;
                break;
            default:
                UnimpOpcode(emu);
        }
    }
}

void RunDB(x86emu_t *emu)
{
    uint8_t nextop;
    reg32_t *op1, *op2;
    reg32_t ea1, ea2;
    int32_t tmp32s;
    float f;
    double d;
    long double ld;
    nextop = Fetch8(emu);
    switch(nextop) {
    case 0xE2:      /* FNCLEX */
        //Clears the floating-point exception flags (PE, UE, OE, ZE, DE, and IE), 
        // the exception summary status flag (ES), the stack fault flag (SF), and the busy flag (B) in the FPU status word
        emu->sw.f.F87_PE = 0;
        emu->sw.f.F87_UE = 0;
        emu->sw.f.F87_OE = 0;
        emu->sw.f.F87_ZE = 0;
        emu->sw.f.F87_DE = 0;
        emu->sw.f.F87_IE = 0;
        emu->sw.f.F87_ES = 0;
        emu->sw.f.F87_SF = 0;
        emu->sw.f.F87_B = 0;
        break;
    case 0xE3:      /* FNINIT */
        reset_fpu(emu);
        break;
    default:
        switch((nextop>>3)&7) {
            case 0: /* FILD ST0, Gd */
                GetEd(emu, &op2, &ea2, nextop);
                *(uint32_t*)&tmp32s = op2->dword[0];
                fpu_do_push(emu);
                ST0.d = tmp32s;
                break;
            case 7: /* FSTP float */
                GetEd(emu, &op1, &ea1, nextop);
                f = ST0.d;
                fpu_do_pop(emu);
                op1->dword[0] = *(uint32_t*)&f;
                break;
            default:
                UnimpOpcode(emu);
        }
    }
}

void RunDC(x86emu_t *emu)
{
    uint8_t nextop;
    reg32_t *op1, *op2;
    reg32_t ea1, ea2;
    int32_t tmp32s;
    float f;
    double d;
    long double ld;
    nextop = Fetch8(emu);
    switch(nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:  /* FADD */
            ST(nextop&7).d += ST0.d;
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:  /* FMUL */
            ST(nextop&7).d *= ST0.d;
            break;
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:  /* FCOM */
            fpu_fcom(emu, ST(nextop&7).d);  // TODO: is this ok?
            break;
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:  /* FCOMP */
            fpu_fcom(emu, ST(nextop&7).d);  // TODO: is this ok?
            fpu_do_pop(emu);
            break;
        case 0xE0:
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:  /* FSUBR */
            ST(nextop&7).d = ST0.d -ST(nextop&7).d;
            break;
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:  /* FSUB */
            ST(nextop&7).d -= ST0.d;
            break;
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:  /* FDIVR */
            ST(nextop&7).d = ST0.d / ST(nextop&7).d;
            break;
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:  /* FDIV */
            ST(nextop&7).d /=  ST0.d;
            break;
        default:
        switch((nextop>>3)&7) {
        case 0:         /* FADD */
            GetEd(emu, &op2, &ea2, nextop);
            *(int64_t*)&d = *(int64_t*)&op2->dword[0];
            ST0.d += d;
        case 1:         /* FMUL */
            GetEd(emu, &op2, &ea2, nextop);
            *(int64_t*)&d = *(int64_t*)&op2->dword[0];
            ST0.d *= d;
            break;
        case 2:      /* FCOM */
            GetEd(emu, &op2, &ea2, nextop);
            *(int64_t*)&d = *(int64_t*)&op2->dword[0];
            fpu_fcom(emu, d);
            break;
        case 3:     /* FCOMP */
            GetEd(emu, &op2, &ea2, nextop);
            *(int64_t*)&d = *(int64_t*)&op2->dword[0];
            fpu_fcom(emu, d);
            fpu_do_pop(emu);
            break;
        case 4:         /* FSUB */
            GetEd(emu, &op2, &ea2, nextop);
            *(int64_t*)&d = *(int64_t*)&op2->dword[0];
            ST0.d -= d;
            break;
        case 5:         /* FSUBR */
            GetEd(emu, &op2, &ea2, nextop);
            *(int64_t*)&d = *(int64_t*)&op2->dword[0];
            ST0.d = d - ST0.d;
            break;
        case 6:         /* FDIV */
            GetEd(emu, &op2, &ea2, nextop);
            *(int64_t*)&d = *(int64_t*)&op2->dword[0];
            ST0.d /= d;
            break;
        case 7:         /* FDIVR */
            GetEd(emu, &op2, &ea2, nextop);
            *(int64_t*)&d = *(int64_t*)&op2->dword[0];
            ST0.d = d / ST0.d;
            break;
        default:
            UnimpOpcode(emu);
        }
    }
}

void RunDD(x86emu_t *emu)
{
    uint8_t nextop;
    reg32_t *op1, *op2;
    reg32_t ea1, ea2;
    float f;
    double d;
    long double ld;
    nextop = Fetch8(emu);
    switch((nextop>>3)&7) {
        case 0: /* FLD double */
            GetEd(emu, &op1, &ea1, nextop);
            fpu_do_push(emu);
            ST0.ll = *(int64_t*)&op1->dword[0];
            break;
        case 2: /* FST double */
            GetEd(emu, &op1, &ea1, nextop);
            *(int64_t*)&op1->dword[0] = ST0.ll;
            break;
        case 3: /* FSTP double */
            GetEd(emu, &op1, &ea1, nextop);
            *(int64_t*)&op1->dword[0] = ST0.ll;
            fpu_do_pop(emu);
            break;
        default:
            UnimpOpcode(emu);
    }
}

void RunDE(x86emu_t *emu)
{
    uint8_t nextop;
    reg32_t *op1, *op2;
    reg32_t ea1, ea2;
    float f;
    double d;
    long double ld;
    nextop = Fetch8(emu);
    switch (nextop) {
    case 0xC1:  /* FADDP ST1, ST0 */
        d = ST0.d;
        fpu_do_pop(emu);
        ST0.d += d;
        break;
    case 0xC0:  /* FADDP STx, ST0 */
    case 0xC2:
    case 0xC3:
    case 0xC4:
    case 0xC5:
    case 0xC6:
    case 0xC7:
        ST(nextop&7).d += ST0.d;
        fpu_do_pop(emu);
        break;
    case 0xC9:  /* FMULP ST1, ST0 */
        d = ST0.d;
        fpu_do_pop(emu);
        ST0.d *= d;
        break;
    case 0xC8:  /* FMULP STx, ST0 */
    case 0xCA:
    case 0xCB:
    case 0xCC:
    case 0xCD:
    case 0xCE:
    case 0xCF:
        ST(nextop&7).d *= ST0.d;
        fpu_do_pop(emu);
        break;
    case 0xD9:  /* FCOMPP */
        fpu_fcom(emu, ST1.d);
        fpu_do_pop(emu);
        fpu_do_pop(emu);
        break;
    case 0xF1:  /* FDIVRP ST1, ST0 */
        d = ST0.d;
        fpu_do_pop(emu);
        ST0.d = d / ST0.d;
        break;
    case 0xF0:  /* FDIVRP STx, ST0 */
    case 0xF2:
    case 0xF3:
    case 0xF4:
    case 0xF5:
    case 0xF6:
    case 0xF7:
        ST(nextop&7).d = ST0.d / ST(nextop&7).d;
        fpu_do_pop(emu);
        break;
    case 0xF9:  /* FDIVP ST1, ST0 */
        d = ST0.d;
        fpu_do_pop(emu);
        ST0.d /= d;
        break;
    case 0xF8:  /* FDIVP STx, ST0 */
    case 0xFA:
    case 0xFB:
    case 0xFC:
    case 0xFD:
    case 0xFE:
    case 0xFF:
        ST(nextop&7).d /= ST0.d;
        fpu_do_pop(emu);
        break;
    default:
        switch((nextop>>3)&7) {
        default:
            UnimpOpcode(emu);
        }
    }
}

void RunDF(x86emu_t *emu)
{
    uint8_t nextop;
    reg32_t *op1, *op2;
    reg32_t ea1, ea2;
    float f;
    double d;
    long double ld;
    nextop = Fetch8(emu);
    switch (nextop) {

    case 0xE8:  /* FUCOMIP STx, ST0 */
    case 0xE9:
    case 0xEA:
    case 0xEB:
    case 0xEC:
    case 0xED:
    case 0xEE:
    case 0xEF:
        fpu_fcom(emu, ST(nextop&7).d);   // bad, should handle QNaN and IA interrupt
        fpu_do_pop(emu);
        break;

    case 0xF0:  /* FCOMIP STx, ST0 */
    case 0xF1:
    case 0xF2:
    case 0xF3:
    case 0xF4:
    case 0xF5:
    case 0xF6:
    case 0xF7:
        fpu_fcom(emu, ST(nextop&7).d);
        fpu_do_pop(emu);
        break;
    default:
        switch((nextop>>3)&7) {
        default:
            UnimpOpcode(emu);
        }
    }
}