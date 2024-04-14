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
uintptr_t TestDA(x86test_t *test, uintptr_t addr)
#else
uintptr_t RunDA(x86emu_t *emu, uintptr_t addr)
#endif
{
    uint8_t nextop;
    int64_t ll;
    float f;
    reg32_t *oped;
    #ifdef TEST_INTERPRETER
    x86emu_t*emu = test->emu;
    #endif

    nextop = F8;
    switch(nextop) {

    case 0xC0:      /* FCMOVB ST(0), ST(i) */
    case 0xC1:
    case 0xC2:
    case 0xC3:
    case 0xC4:
    case 0xC5:
    case 0xC6:
    case 0xC7:
        CHECK_FLAGS(emu);
        if(ACCESS_FLAG(F_CF))
            ST0.q = ST(nextop&7).q;
        break;
    case 0xC8:      /* FCMOVE ST(0), ST(i) */
    case 0xC9:
    case 0xCA:
    case 0xCB:
    case 0xCC:
    case 0xCD:
    case 0xCE:
    case 0xCF:
        CHECK_FLAGS(emu);
        if(ACCESS_FLAG(F_ZF))
            ST0.q = ST(nextop&7).q;
        break;
    case 0xD0:      /* FCMOVBE ST(0), ST(i) */
    case 0xD1:
    case 0xD2:
    case 0xD3:
    case 0xD4:
    case 0xD5:
    case 0xD6:
    case 0xD7:
        CHECK_FLAGS(emu);
        if(ACCESS_FLAG(F_CF) || ACCESS_FLAG(F_ZF))
            ST0.q = ST(nextop&7).q;
        break;
    case 0xD8:      /* FCMOVU ST(0), ST(i) */
    case 0xD9:
    case 0xDA:
    case 0xDB:
    case 0xDC:
    case 0xDD:
    case 0xDE:
    case 0xDF:
        CHECK_FLAGS(emu);
        if(ACCESS_FLAG(F_PF))
            ST0.q = ST(nextop&7).q;
        break;
    
    case 0xE9:      /* FUCOMPP */
        fpu_fcom(emu, ST1.d);   // bad, should handle QNaN and IA interrupt
        fpu_do_pop(emu);
        fpu_do_pop(emu);
        break;

    case 0xE4:
    case 0xF0:
    case 0xF1:
    case 0xF4:
    case 0xF5:
    case 0xF6:
    case 0xF7:
    case 0xF8:
    case 0xF9:
    case 0xFD:
        return 0;
    default:;
        int oldround = fpu_setround(emu);
        switch((nextop>>3)&7) {
            case 0:     /* FIADD ST0, Ed int */
                GET_ED;
                ST0.d += ED->sdword[0];
                break;
            case 1:     /* FIMUL ST0, Ed int */
                GET_ED;
                ST0.d *= ED->sdword[0];
                break;
            case 2:     /* FICOM ST0, Ed int */
                GET_ED;
                fpu_fcom(emu, ED->sdword[0]);
                break;
            case 3:     /* FICOMP ST0, Ed int */
                GET_ED;
                fpu_fcom(emu, ED->sdword[0]);
                fpu_do_pop(emu);
                break;
            case 4:     /* FISUB ST0, Ed int */
                GET_ED;
                ST0.d -= ED->sdword[0];
                break;
            case 5:     /* FISUBR ST0, Ed int */
                GET_ED;
                ST0.d = (double)ED->sdword[0] - ST0.d;
                break;
            case 6:     /* FIDIV ST0, Ed int */
                GET_ED;
                ST0.d /= ED->sdword[0];
                break;
            case 7:     /* FIDIVR ST0, Ed int */
                GET_ED;
                ST0.d = (double)ED->sdword[0] / ST0.d;
                break;
        }
        fesetround(oldround);
    }
    return addr;
}