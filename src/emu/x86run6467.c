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
uintptr_t Test6467(x86test_t *test, uintptr_t tlsdata, uintptr_t addr)
#else
uintptr_t Run6467(x86emu_t *emu, uintptr_t tlsdata, uintptr_t addr)
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

        case 0x3B:                              /* CMP GD, FS:Ed16 */
            nextop = F8;
            GET_EW16_OFFS(tlsdata);
            cmp32(emu, GD.dword[0], ED->dword[0]);
            break;

        case 0x89:                              /* MOV ED16,Gd */
            nextop = F8;
            GET_EW16_OFFS(tlsdata);
            ED->dword[0] = GD.dword[0];
            break;

        case 0x8B:                              /* MOV Gd,Ed16 */
            nextop = F8;
            GET_EW16_OFFS(tlsdata);
            GD.dword[0] = ED->dword[0];
            break;

        case 0x8F:                              /* POP FS:Ed */
            nextop = F8;
            GET_EW16_OFFS(tlsdata);
            ED->dword[0] = Pop(emu);
            break;

        case 0xA1:                              /* MOV EAX,Ov16 */
            tmp32u = F16;
            R_EAX = *(uint32_t*)(tlsdata + tmp32u);
            break;
        case 0xA3:                              /* MOV Ov16,EAX */
            tmp32u = F16;
            #ifdef TEST_INTERPRETER
            test->memaddr = tlsdata + tmp32u;
            test->memsize = 4;
            *(uint32_t*)(test->mem) = R_EAX;
            #else
            *(uint32_t*)(tlsdata + tmp32u) = R_EAX;
            #endif
            break;
        case 0xFF:                              /* GRP 5 Ed */
            nextop = F8;
            switch((nextop>>3)&7) {
                case 6:                         /* Push Ed */
                    GET_EW16_OFFS(tlsdata);
                    Push(emu, ED->dword[0]);
                    break;
                default:
                    return 0;
            }
            break;
        default:
            return 0;
    }
    return addr;
}
