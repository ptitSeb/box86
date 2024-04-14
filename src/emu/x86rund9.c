#define _GNU_SOURCE
#include <fenv.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "debug.h"
#include "box86stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x87emu_private.h"
#include "x87emu_setround.h"
#include "x86primop.h"
#include "x86trace.h"
#include "box86context.h"

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t TestD9(x86test_t *test, uintptr_t addr)
#else
uintptr_t RunD9(x86emu_t *emu, uintptr_t addr)
#endif
{
    uint8_t nextop;
    int32_t tmp32s;
    int64_t ll;
    float f;
    reg32_t *oped;
    #ifdef TEST_INTERPRETER
    x86emu_t*emu = test->emu;
    #endif

    int oldround;
    nextop = F8;
    switch (nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:  /* FLD STx */
            ll = ST(nextop&7).sq;
            fpu_do_push(emu);
            ST0.sq = ll;
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:  /* FXCH STx */
            ll = ST(nextop&7).sq;
            ST(nextop&7).sq = ST0.sq;
            ST0.sq = ll;
            emu->sw.f.F87_C1 = 0;
            break;

        case 0xD0:  /* FNOP */
            break;

        case 0xE0:  /* FCHS */
            ST0.d = -ST0.d;
            emu->sw.f.F87_C1 = 0;
            break;
        case 0xE1:  /* FABS */
            ST0.d = fabs(ST0.d);
            emu->sw.f.F87_C1 = 0;
            break;
        
        case 0xE4:  /* FTST */
            fpu_ftst(emu);
            break;
        case 0xE5:  /* FXAM */
            fpu_fxam(emu);
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
        case 0xEE:  /* FLDZ */
            fpu_do_push(emu);
            ST0.d = 0.0;
            break;

        case 0xF0:  /* F2XM1 */
            if (ST0.d == 0)
                break;
            // Using the expm1 instead of exp2(ST0)-1 can avoid losing precision much,
            // expecially when ST0 is close to zero (which loses the precise when -1).
            // printf("%a, %a\n", LN2 * ST0.d, expm1(LN2 * ST0.d));
            ST0.d = expm1(LN2 * ST0.d);
            //    = 2^ST0 - 1 + error. (in math)
            emu->sw.f.F87_C1 = 0;
            break;
        case 0xF1:  /* FYL2X */
            ST(1).d *= log2(ST0.d);
            fpu_do_pop(emu);
            emu->sw.f.F87_C1 = 0;
            break;
        case 0xF2:  /* FPTAN */
            oldround = fpu_setround(emu);
            ST0.d = tan(ST0.d);
            fesetround(oldround);
            fpu_do_push(emu);
            ST0.d = 1.0;
            emu->sw.f.F87_C2 = 0;
            emu->sw.f.F87_C1 = 0;
            break;
        case 0xF3:  /* FPATAN */
            oldround = fpu_setround(emu);
            ST1.d = atan2(ST1.d, ST0.d);
            fesetround(oldround);
            fpu_do_pop(emu);
            emu->sw.f.F87_C1 = 0;
            break;
        case 0xF4:  /* FXTRACT */
            ST0.d = frexp(ST0.d, &tmp32s);
            fpu_do_push(emu);
            ST0.d = tmp32s;
            // C1 set only if stack under/overflow occurs
            break;

        case 0xF8:  /* FPREM */
            {
                int e0, e1;
                frexp(ST0.d, &e0);
                frexp(ST1.d, &e1);
                tmp32s = e0 - e1;
            }
            if(tmp32s<64)
            {
                ll = (int64_t)floor(ST0.d/ST1.d);
                ST0.d = ST0.d - (ST1.d*ll);
                emu->sw.f.F87_C2 = 0;
                emu->sw.f.F87_C1 = (ll&1)?1:0;
                emu->sw.f.F87_C3 = (ll&2)?1:0;
                emu->sw.f.F87_C0 = (ll&4)?1:0;
            } else {
                ll = (int64_t)(floor((ST0.d/ST1.d))/exp2(tmp32s - 32));
                ST0.d = ST0.d - ST1.d*ll*exp2(tmp32s - 32);
                emu->sw.f.F87_C2 = 1;
            }
            break;
        case 0xF5:  /* FPREM1 */
            // get exponant(ST(0))-exponant(ST(1)) in temp32s
            {
                int e0, e1;
                frexp(ST0.d, &e0);
                frexp(ST1.d, &e1);
                tmp32s = e0 - e1;
            }
            if(tmp32s<64)
            {
                ll = (int64_t)round(ST0.d/ST1.d);
                ST0.d = ST0.d - (ST1.d*ll);
                emu->sw.f.F87_C2 = 0;
                emu->sw.f.F87_C1 = (ll&1)?1:0;
                emu->sw.f.F87_C3 = (ll&2)?1:0;
                emu->sw.f.F87_C0 = (ll&4)?1:0;
            } else {
                ll = (int64_t)(trunc((ST0.d/ST1.d))/exp2(tmp32s - 32));
                ST0.d = ST0.d - ST1.d*ll*exp2(tmp32s - 32);
                emu->sw.f.F87_C2 = 1;
            }
            break;
        case 0xF6:  /* FDECSTP */
            emu->top=(emu->top-1)&7;    // this will probably break a few things
            break;
        case 0xF7:  /* FINCSTP */
            emu->top=(emu->top+1)&7;    // this will probably break a few things
            break;
        case 0xF9:  /* FYL2XP1 */
            // Using the log1p instead of log2(ST0+1) can avoid losing precision much,
            // expecially when ST0 is close to zero (which loses the precise when +1).
            ST(1).d = (ST(1).d * log1p(ST0.d)) / M_LN2;
            //      = ST1 * log2(ST0 + 1) + error. (in math)
            fpu_do_pop(emu);
            emu->sw.f.F87_C1 = 0;
            break;
        case 0xFA:  /* FSQRT */
            oldround = fpu_setround(emu);
            ST0.d = sqrt(ST0.d);
            fesetround(oldround);
            emu->sw.f.F87_C1 = 0;
            break;
        case 0xFB:  /* FSINCOS */
            fpu_do_push(emu);
            oldround = fpu_setround(emu);
            sincos(ST1.d, &ST1.d, &ST0.d);
            fesetround(oldround);
            emu->sw.f.F87_C2 = 0;
            emu->sw.f.F87_C1 = 0;
            break;
        case 0xFC:  /* FRNDINT */
            ST0.d = fpu_round(emu, ST0.d);
            emu->sw.f.F87_C1 = 0;
            break;
        case 0xFD:  /* FSCALE */
            if (ST1.d > INT32_MAX)
                tmp32s = INT32_MAX;
            else if (ST1.d < INT32_MIN)
                tmp32s = INT32_MIN;
            else
                tmp32s = ST1.d;
            if(ST0.d!=0.0) {
                oldround = fpu_setround(emu);
                ST0.d = ldexp(ST0.d, tmp32s);
                fesetround(oldround);
            }
            emu->sw.f.F87_C1 = 0;
            break;
        case 0xFE:  /* FSIN */
            oldround = fpu_setround(emu);
            ST0.d = sin(ST0.d);
            fesetround(oldround);
            emu->sw.f.F87_C2 = 0;
            emu->sw.f.F87_C1 = 0;
            break;
        case 0xFF:  /* FCOS */
            oldround = fpu_setround(emu);
            ST0.d = cos(ST0.d);
            fesetround(oldround);
            emu->sw.f.F87_C2 = 0;
            emu->sw.f.F87_C1 = 0;
            break;


        case 0xD1:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
        case 0xE2:
        case 0xE3:
        case 0xE6:
        case 0xE7:
        case 0xEF:
            return 0;
        default:
        switch((nextop>>3)&7) {
            case 0:     /* FLD ST0, Ed float */
                GET_ED;
                fpu_do_push(emu);
                if(!(((uintptr_t)ED)&3))
                    ST0.d = *(float*)ED;
                else {
                    memcpy(&f, ED, sizeof(float));
                    ST0.d = f;
                }
                break;
            case 2:     /* FST Ed, ST0 */
                GET_ED;
                if(!(((uintptr_t)ED)&3)) {
                    oldround = fpu_setround(emu);
                    *(float*)ED = ST0.d;
                    fesetround(oldround);
                } else {
                    f = ST0.d;
                    memcpy(ED, &f, sizeof(float));
                }
                break;
            case 3:     /* FSTP Ed, ST0 */
                GET_ED;
                if(!(((uintptr_t)ED)&3)) {
                    oldround = fpu_setround(emu);
                    *(float*)ED = ST0.d;
                    fesetround(oldround);
                } else {
                    f = ST0.d;
                    memcpy(ED, &f, sizeof(float));
                }
                fpu_do_pop(emu);
                break;
            case 4:     /* FLDENV m */
                // warning, incomplete
                GET_ED;
                #ifndef TEST_INTERPRETER
                fpu_loadenv(emu, (char*)ED, 0);
                #endif
                break;
            case 5:     /* FLDCW Ew */
                GET_EW;
                emu->cw.x16 = EW->word[0];
                // do something with cw?
                break;
            case 6:     /* FNSTENV m */
                // warning, incomplete
                GET_ED;
                #ifndef TEST_INTERPRETER
                fpu_savenv(emu, (char*)ED, 0);
                #endif
                // intruction pointer: 48bits
                // data (operand) pointer: 48bits
                // last opcode: 11bits save: 16bits restaured (1st and 2nd opcode only)
                break;
            case 7: /* FNSTCW Ew */
                GET_EW;
                EW->word[0] = emu->cw.x16;
                break;
            default:
                return 0;
        }
    }
    return addr;
}
