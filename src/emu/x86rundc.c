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
uintptr_t TestDC(x86test_t *test, uintptr_t addr)
#else
uintptr_t RunDC(x86emu_t *emu, uintptr_t addr)
#endif
{
    uint8_t nextop;
    int32_t tmp32s;
    int64_t ll;
    double d;
    reg32_t *oped;
    #ifdef TEST_INTERPRETER
    x86emu_t*emu = test->emu;
    #endif

    int oldround = fpu_setround(emu);
    nextop = F8;
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
            GET_ED8;
            switch((nextop>>3)&7) {
            case 0:         /* FADD ST0, double */
                if(!(((uintptr_t)ED)&7))
                    ST0.d += *(double*)ED;
                else {
                    memcpy(&d, ED, sizeof(double));
                    ST0.d += d;
                }
                break;
            case 1:         /* FMUL ST0, double */
                if(!(((uintptr_t)ED)&7))
                    ST0.d *= *(double*)ED;
                else {
                    memcpy(&d, ED, sizeof(double));
                    ST0.d *= d;
                }
                break;
            case 2:      /* FCOM ST0, double */
                if(!(((uintptr_t)ED)&7))
                    fpu_fcom(emu, *(double*)ED);
                else {
                    memcpy(&d, ED, sizeof(double));
                    fpu_fcom(emu, d);
                }
                break;
            case 3:     /* FCOMP ST0, double */
                if(!(((uintptr_t)ED)&7))
                    fpu_fcom(emu, *(double*)ED);
                else {
                    memcpy(&d, ED, sizeof(double));
                    fpu_fcom(emu, d);
                }
                fpu_do_pop(emu);
                break;
            case 4:         /* FSUB ST0, double */
                if(!(((uintptr_t)ED)&7))
                    ST0.d -= *(double*)ED;
                else {
                    memcpy(&d, ED, sizeof(double));
                    ST0.d -= d;
                }
                break;
            case 5:         /* FSUBR ST0, double */
                if(!(((uintptr_t)ED)&7))
                    ST0.d = *(double*)ED - ST0.d;
                else {
                    memcpy(&d, ED, sizeof(double));
                    ST0.d = d - ST0.d;
                }
                break;
            case 6:         /* FDIV ST0, double */
                if(!(((uintptr_t)ED)&7))
                    ST0.d /= *(double*)ED;
                else {
                    memcpy(&d, ED, sizeof(double));
                    ST0.d /= d;
                }
                break;
            case 7:         /* FDIVR ST0, double */
                if(!(((uintptr_t)ED)&7))
                    ST0.d = *(double*)ED / ST0.d;
                else {
                    memcpy(&d, ED, sizeof(double));
                    ST0.d = d / ST0.d;
                }
                break;
            default:
                fesetround(oldround);
                return 0;
        }
    }
    fesetround(oldround);
    return addr;
}