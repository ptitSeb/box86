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
uintptr_t Test6466(x86test_t *test, uintptr_t tlsdata, uintptr_t addr)
#else
uintptr_t Run6466(x86emu_t *emu, uintptr_t tlsdata, uintptr_t addr)
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

        case 0x03:                              /* ADD Gw, FS:Ew */
            nextop = F8;
            GET_EW_OFFS(tlsdata);
            GW.word[0] = add16(emu, GW.word[0], EW->word[0]);
            break;

        case 0x8B:                              /* MOV Gw,FS:Ew */
            nextop = F8;
            GET_EW_OFFS(tlsdata);
            GW.word[0] = EW->word[0];
            break;

        default:
            return 0;
    }
    return addr;
}
