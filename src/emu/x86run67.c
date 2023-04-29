#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "box86stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"
#include "x87emu_private.h"
#include "box86context.h"
#include "my_cpuid.h"
#include "bridge.h"
#include "signals.h"
#ifdef DYNAREC
#include "../dynarec/arm_lock_helper.h"
#endif

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t Test67(x86test_t *test, int rep, uintptr_t addr)
#else
uintptr_t Run67(x86emu_t *emu, int rep, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    reg32_t *oped;
    uint8_t tmp8u, tmp8u2;
    int8_t tmp8s;
    uint16_t tmp16u, tmp16u2;
    int16_t tmp16s;
    uint32_t tmp32u, tmp32u2, tmp32u3;
    int32_t tmp32s, tmp32s2;
    uint64_t tmp64u;
    int64_t tmp64s;
    double d;
    float f;
    int64_t ll;
    sse_regs_t *opex, eax1;
    mmx87_regs_t *opem, eam1;
    uintptr_t tlsdata;

#ifdef TEST_INTERPRETER
    x86emu_t *emu = test->emu;
#endif
    opcode = F8;

    while(opcode==0x67)
        opcode = F8;

    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }

    switch(opcode) {

        case 0x64:
            tlsdata = GetFSBaseEmu(emu);
            #ifdef TEST_INTERPRETER
            return Test6467(test, tlsdata, addr);
            #else
            return Run6467(emu, tlsdata, addr);
            #endif

        case 0x66:                      /* MoooRE opcodes */
            #ifdef TEST_INTERPRETER
            return Test6766(test, rep, addr);
            #else
            return Run6766(emu, rep, addr);
            #endif

        case 0x6C:                      /* INSB */
            tmp32u = rep?R_CX:1;
            while(tmp32u--) {
                #ifdef TEST_INTERPRETER
                *(int8_t*)(R_DI+GetESBaseEmu(emu)) = 0;         // faking port read, using actual segment ES, just in case
                #endif
                if(ACCESS_FLAG(F_DF))
                    R_DI-=1;
                else
                    R_DI+=1;
            }
            if(rep) R_CX = 0;
            break;

        case 0xAC:                      /* LODSB */
            tmp32u = rep?R_CX:1;
            while(tmp32u--) {
                R_AL = *(int8_t*)(R_SI+GetDSBaseEmu(emu));
                if(ACCESS_FLAG(F_DF))
                    R_SI-=1;
                else
                    R_SI+=1;
            }
            if(rep) R_CX = 0;
            break;

        case 0xE0:                      /* LOOPNZ */
            CHECK_FLAGS(emu);
            tmp8s = F8S;
            --R_CX; // don't update flags
            if(R_CX && !ACCESS_FLAG(F_ZF))
                addr += tmp8s;
            break;
        case 0xE1:                      /* LOOPZ */
            CHECK_FLAGS(emu);
            tmp8s = F8S;
            --R_CX; // don't update flags
            if(R_CX && ACCESS_FLAG(F_ZF))
                addr += tmp8s;
            break;
        case 0xE2:                      /* LOOP */
            tmp8s = F8S;
            --R_CX; // don't update flags
            if(R_CX)
                addr += tmp8s;
            break;
        case 0xE3:                      /* JCXZ */
            tmp8s = F8S;
            if(!R_CX)
                addr += tmp8s;
            break;

        case 0xE8:                      /* CALL Id */
            tmp32s = F32S; // call is relative
            Push(emu, addr);
            addr += tmp32s;
            break;


        default:
            return 0;
    }
    return addr;
}
