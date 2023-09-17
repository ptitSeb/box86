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
uintptr_t TestDD(x86test_t *test, uintptr_t addr)
#else
uintptr_t RunDD(x86emu_t *emu, uintptr_t addr)
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

    nextop = F8;
    switch(nextop) {
    
    case 0xC0:  /* FFREE STx */
    case 0xC1:
    case 0xC2:
    case 0xC3:
    case 0xC4:
    case 0xC5:
    case 0xC6:
    case 0xC7:
        fpu_do_free(emu, nextop-0xC0);
        break;

    case 0xD0:  /* FST ST0, STx */
    case 0xD1:
    case 0xD2:
    case 0xD3:
    case 0xD4:
    case 0xD5:
    case 0xD6:
    case 0xD7:
        ST(nextop&7).q = ST0.q;
        break;
    case 0xD8:  /* FSTP ST0, STx */
    case 0xD9:
    case 0xDA:
    case 0xDB:
    case 0xDC:
    case 0xDD:
    case 0xDE:
    case 0xDF:
        ST(nextop&7).q = ST0.q;
        fpu_do_pop(emu);
        break;
    case 0xE0:  /* FUCOM ST0, STx */
    case 0xE1:
    case 0xE2:
    case 0xE3:
    case 0xE4:
    case 0xE5:
    case 0xE6:
    case 0xE7:
        fpu_fcom(emu, ST(nextop&7).d);   // bad, should handle QNaN and IA interrupt
        break;
    case 0xE8:  /* FUCOMP ST0, STx */
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

    case 0xC8:
    case 0xC9:
    case 0xCA:
    case 0xCB:
    case 0xCC:
    case 0xCD:
    case 0xCE:
    case 0xCF:
    case 0xF0:
    case 0xF1:
    case 0xF2:
    case 0xF3:
    case 0xF4:
    case 0xF5:
    case 0xF6:
    case 0xF7:
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
            case 0: /* FLD double */
                GET_ED8;
                fpu_do_push(emu);
                if(!(((uintptr_t)ED)&7))
                    ST0.d = *(double*)ED;
                else {
                    memcpy(&ST0.d, ED, sizeof(double));
                }
                break;
            case 1: /* FISTTP ED qword */
                GET_ED8;
                if(!(((uintptr_t)ED)&7)) {
                    if(STll(0).sref==ST(0).sq)
                        *(int64_t*)ED = STll(0).sq;
                    else {
                        if(isgreater(ST0.d, (double)0x7fffffffffffffffLL) || isless(ST0.d, -(double)0x8000000000000000LL) || !isfinite(ST0.d))
                            *(uint64_t*)ED = 0x8000000000000000LL;
                        else
                            *(int64_t*)ED = ST0.d;
                    }
                } else {
                    int64_t i64;
                    if(STll(0).sref==ST(0).sq)
                        i64 = STll(0).sq;
                    else {
                        if(isgreater(ST0.d, (double)0x7fffffffffffffffLL) || isless(ST0.d, -(double)0x8000000000000000LL) || !isfinite(ST0.d))
                            i64 = 0x8000000000000000LL;
                        else
                            i64 = ST0.d;
                    }
                    memcpy(ED, &i64, sizeof(int64_t));
                }
                fpu_do_pop(emu);
                break;
            case 2: /* FST double */
                GET_ED8;
                if(!(((uintptr_t)ED)&7))
                    *(double*)ED = ST0.d;
                else {
                    memcpy(ED, &ST0.d, sizeof(double));
                }
                break;
            case 3: /* FSTP double */
                GET_ED8;
                if(!(((uintptr_t)ED)&7))
                    *(double*)ED = ST0.d;
                else {
                    memcpy(ED, &ST0.d, sizeof(double));
                }
                fpu_do_pop(emu);
                break;
            case 4: /* FRSTOR m108byte */
                GET_ED_;
                #ifndef TEST_INTERPRETER
                fpu_loadenv(emu, (char*)ED, 0);
                #endif
                // get the STx
                {
                    char* p =(char*)ED;
                    p += 28;
                    for (int i=0; i<8; ++i) {
                        LD2D(p, &ST(i).d);
                        p+=10;
                    }
                }
                break;
            case 6: /* FNSAVE m108byte */
                GET_ED_;
                // ENV first...
                // warning, incomplete
                #ifndef TEST_INTERPRETER
                fpu_savenv(emu, (char*)ED, 0);
                // save the STx
                {
                    char* p =(char*)ED;
                    p += 28;
                    for (int i=0; i<8; ++i) {
                        D2LD(&ST(i).d, p);
                        p+=10;
                    }
                }
                #endif
                reset_fpu(emu);
                break;
            case 7: /* FNSTSW m2byte */
                GET_ED;
                emu->sw.f.F87_TOP = emu->top&7;
                *(uint16_t*)ED = emu->sw.x16;
                break;
            default:
                return 0;
        }
    }
    return addr;
}