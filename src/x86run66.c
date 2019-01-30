#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"


void Run6766(x86emu_t *emu)
{
    uint8_t opcode = Fetch8(emu);
    uint8_t nextop;
    reg32_t *op1, *op2, *op3, *op4;
    reg32_t ea1, ea2, ea3, ea4;
    uint8_t tmp8u;
    int8_t tmp8s;
    uint16_t tmp16u, tmp16u2;
    int16_t tmp16s;
    uint32_t tmp32u;
    int32_t tmp32s;
    uint64_t tmp64u;
    int64_t tmp64s;
    switch(opcode) {

    case 0x8D:                              /* LEA Gw,Ew */
        nextop = Fetch8(emu);
        GetEw16(emu, &op1, nextop);
        GetG(emu, &op2, nextop);
        op2->word[0] = (uint16_t)(uintptr_t)&op1->word[0];
        break;
    
                
    default:
        UnimpOpcode(emu);
    }
}

void Run67(x86emu_t *emu)
{
    uint8_t opcode = Fetch8(emu);
    uint8_t nextop;
    reg32_t *op1, *op2, *op3, *op4;
    reg32_t ea1, ea2, ea3, ea4;
    uint8_t tmp8u;
    int8_t tmp8s;
    uint16_t tmp16u;
    int16_t tmp16s;
    uint32_t tmp32u;
    int32_t tmp32s;
    uint64_t tmp64u;
    int64_t tmp64s;
    switch(opcode) {

    case 0x66:                      /* MOARE */
        Run6766(emu);
        break;

    case 0xE0:                      /* LOOPNZ */
        CHECK_FLAGS(emu);
        tmp8s = Fetch8s(emu);
        --R_CX; // don't update flags
        if(R_CX && !ACCESS_FLAG(F_ZF))
            R_EIP += tmp8s;
        break;
    case 0xE1:                      /* LOOPZ */
        CHECK_FLAGS(emu);
        tmp8s = Fetch8s(emu);
        --R_CX; // don't update flags
        if(R_CX && ACCESS_FLAG(F_ZF))
            R_EIP += tmp8s;
        break;
    case 0xE2:                      /* LOOP */
        tmp8s = Fetch8s(emu);
        --R_CX; // don't update flags
        if(R_CX)
            R_EIP += tmp8s;
        break;
    case 0xE3:                      /* JCXZ */
        tmp8s = Fetch8s(emu);
        if(!R_CX)
            R_EIP += tmp8s;
        break;

    default:
        UnimpOpcode(emu);
    }
}
