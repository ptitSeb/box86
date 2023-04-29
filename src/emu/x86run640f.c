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
uintptr_t Test640F(x86test_t *test, uintptr_t tlsdata, uintptr_t addr)
#else
uintptr_t Run640F(x86emu_t *emu, uintptr_t tlsdata, uintptr_t addr)
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
    switch(opcode) {

        case 0xAF:                      /* IMUL Gd,Ed */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            GD.dword[0] = imul32(emu, GD.dword[0], ED->dword[0]);
            break;

        case 0xB6:                              /* MOVZX Gd,GS:Eb */
            nextop = F8;
            GET_EB_OFFS(tlsdata);
            GD.dword[0] = EB->byte[0];
            break;

        default:
            return 0;
    }
    return addr;
}
