#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "box86stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"
#include "box86context.h"
#include "bridge.h"
#ifdef DYNAREC
#include "../dynarec/arm_lock_helper.h"
#endif

#include "modrm.h"


#ifdef TEST_INTERPRETER
uintptr_t Test6766(x86test_t *test, int rep, uintptr_t addr)
#else
uintptr_t Run6766(x86emu_t *emu, int rep, uintptr_t addr)
#endif
{
    uint8_t nextop;
    reg32_t *oped;
    uint8_t tmp8u;
    uint32_t tmp32u;
    int32_t tmp32s;
    #ifdef TEST_INTERPRETER
    x86emu_t* emu = test->emu;
    #endif
    uint8_t opcode = F8;

    while((opcode==0x2E) || (opcode==0x66))   // ignoring CS: or multiple 0x66
        opcode = F8;

    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }

    switch(opcode) {

        case 0x8D:                              /* LEA Gw,Ew */
            nextop = F8;
            GET_EW16_;
            GW.word[0] = (uint16_t)(uintptr_t)ED;
            break;

        default:
            return 0;
    }
    return addr;
}
