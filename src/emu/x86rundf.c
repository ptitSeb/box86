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
#include "x86primop.h"
#include "x86trace.h"
#include "box86context.h"

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t TestDF(x86test_t *test, uintptr_t addr)
#else
uintptr_t RunDF(x86emu_t *emu, uintptr_t addr)
#endif
{
    uint8_t nextop;
    int16_t tmp16s;
    int64_t tmp64s;
    double d;
    reg32_t *oped;
    #ifdef TEST_INTERPRETER
    x86emu_t*emu = test->emu;
    #endif

    nextop = F8;
    switch (nextop) {
    case 0xC0:  /* FFREEP STx */
    case 0xC1:
    case 0xC2:
    case 0xC3:
    case 0xC4:
    case 0xC5:
    case 0xC6:
    case 0xC7:
        fpu_do_free(emu, nextop-0xC0);
        fpu_do_pop(emu);
        break;

    case 0xD0:  /* FSTP STx, ST0 */
    case 0xD1:
    case 0xD2:
    case 0xD3:
    case 0xD4:
    case 0xD5:
    case 0xD6:
    case 0xD7:
        ST(nextop&7).q = ST0.q;
        fpu_do_pop(emu);
        break;

    case 0xE0:  /* FNSTSW AX */
        {
            x87flags_t sw = emu->sw;
            sw.f.F87_TOP = emu->top&7;
            R_AX = sw.x16;
        }
        break;

    case 0xE8:  /* FUCOMIP ST0, STx */
    case 0xE9:
    case 0xEA:
    case 0xEB:
    case 0xEC:
    case 0xED:
    case 0xEE:
    case 0xEF:
        fpu_fcomi(emu, ST(nextop&7).d);   // bad, should handle QNaN and IA interrupt
        fpu_do_pop(emu);
        break;

    case 0xF0:  /* FCOMIP ST0, STx */
    case 0xF1:
    case 0xF2:
    case 0xF3:
    case 0xF4:
    case 0xF5:
    case 0xF6:
    case 0xF7:
        fpu_fcomi(emu, ST(nextop&7).d);
        fpu_do_pop(emu);
        break;

    case 0xC8:
    case 0xC9:
    case 0xCA:
    case 0xCB:
    case 0xCC:
    case 0xCD:
    case 0xCE:
    case 0xCF:
    case 0xD8:
    case 0xD9:
    case 0xDA:
    case 0xDB:
    case 0xDC:
    case 0xDD:
    case 0xDE:
    case 0xDF:
    case 0xE1:
    case 0xE2:
    case 0xE3:
    case 0xE4:
    case 0xE5:
    case 0xE6:
    case 0xE7:
    case 0xF8:
    case 0xF9:
    case 0xFA:
    case 0xFB:
    case 0xFC:
    case 0xFD:
    case 0xFE:
    case 0xFF:
        return 0;

    default:
        switch((nextop>>3)&7) {
        case 0: /* FILD ST0, Gw */
            GET_EW;
            tmp16s = EW->sword[0];
            fpu_do_push(emu);
            ST0.d = tmp16s;
            break;
        case 1: /* FISTTP Ew, ST0 */
            GET_EW;
            if(isgreater(ST0.d, (double)(int32_t)0x7fff) || isless(ST0.d, -(double)(int32_t)0x8000) || !isfinite(ST0.d))
                tmp16s = 0x8000;
            else
                tmp16s = ST0.d;
            EW->sword[0] = tmp16s;
            fpu_do_pop(emu);
            break;
        case 2: /* FIST Ew, ST0 */
            GET_EW;
            if(isgreater(ST0.d, (double)(int32_t)0x7fff) || isless(ST0.d, -(double)(int32_t)0x8000) || !isfinite(ST0.d))
                EW->sword[0] = 0x8000;
            else
                EW->sword[0] = fpu_round(emu, ST0.d);
            break;
        case 3: /* FISTP Ew, ST0 */
            GET_EW;
            if(isgreater(ST0.d, (double)(int32_t)0x7fff) || isless(ST0.d, -(double)(int32_t)0x8000) || !isfinite(ST0.d))
                EW->sword[0] = 0x8000;
            else
                EW->sword[0] = fpu_round(emu, ST0.d);
            fpu_do_pop(emu);
            break;
        case 4: /* FBLD ST0, tbytes */
            GET_EDT;
            fpu_do_push(emu);
            fpu_fbld(emu, (uint8_t*)ED);
            break;
        case 5: /* FILD ST0, Gq */
            GET_ED8;
            tmp64s = *(int64_t*)ED;
            fpu_do_push(emu);
            ST0.d = tmp64s;
            STll(0).sq = tmp64s;
            STll(0).sref = ST0.sq;
            break;
        case 6: /* FBSTP tbytes, ST0 */
            GET_EDT;
            fpu_fbst(emu, (uint8_t*)ED);
            fpu_do_pop(emu);
            break;
        case 7: /* FISTP i64 */
            GET_ED8;
            if((uintptr_t)ED & 0x7) {
                // un-aligned!
                if(STll(0).sref==ST(0).sq)
                    memcpy(ED, &STll(0).sq, sizeof(int64_t));
                else {
                    int64_t i64;
                    if(isgreater(ST0.d, (double)0x7fffffffffffffffLL) || isless(ST0.d, -(double)0x8000000000000000LL) || !isfinite(ST0.d))
                        i64 = 0x8000000000000000LL;
                    else
                        i64 = fpu_round(emu, ST0.d);
                    memcpy(ED, &i64, sizeof(int64_t));
                }
            } else {
                if(STll(0).sref==ST(0).sq)
                    *(int64_t*)ED = STll(0).sq;
                else {
                    if(isgreater(ST0.d, (double)0x7fffffffffffffffLL) || isless(ST0.d, -(double)0x8000000000000000LL) || !isfinite(ST0.d))
                        *(int64_t*)ED = 0x8000000000000000LL;
                    else
                        *(int64_t*)ED = fpu_round(emu, ST0.d);
                }
            }
            fpu_do_pop(emu);
            break;
        default:
            return 0;
        }
    }
    return addr;
}