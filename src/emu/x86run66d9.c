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
uintptr_t Test66D9(x86test_t *test, uintptr_t addr)
#else
uintptr_t Run66D9(x86emu_t *emu, uintptr_t addr)
#endif
{
    uint8_t nextop;
    reg32_t *oped;
    #ifdef TEST_INTERPRETER
    x86emu_t* emu = test->emu;
    #endif

    nextop = F8;
    switch(nextop) {
        
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
        case 0xD0:
        case 0xE0:
        case 0xE5:
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xFC:
        case 0xE1:
        case 0xE4:
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
        case 0xFD:
        case 0xFE:
        case 0xFF:
            return 0;

        default:
            switch((nextop>>3)&7) {
                case 4:     /* FLDENV m */
                    // warning, incomplete
                    GET_EW;
                    #ifndef TEST_INTERPRETER
                    fpu_loadenv(emu, (char*)ED, 1);
                    #endif
                    break;
                case 6:     /* FNSTENV m */
                    // warning, incomplete
                    GET_EW;
                    #ifndef TEST_INTERPRETER
                    fpu_savenv(emu, (char*)ED, 1);
                    #endif
                    break;
                default:
                    return 0;
            }
    }

    return addr;
}
