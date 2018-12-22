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


void Run66(x86emu_t *emu)
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
            case 0x89:  /* MOV Ew, Gw */
                nextop = Fetch8(emu);
                GetEw(emu, &op1, &ea2, nextop);
                GetG(emu, &op2, nextop);
                op1->word[0] = op2->word[0];
                break;
            case 0xC7: /* MOV Ew,Iw */
                nextop = Fetch8(emu);
                GetEw(emu, &op1, &ea2, nextop);
                op1->word[0] = Fetch16(emu);
                break;
            default:
                printf_log(LOG_NONE, "Unimplemented Opcode 66 %02X %02X %02X %02X %02X\n", opcode, Peek(emu, 0), Peek(emu, 1), Peek(emu, 2), Peek(emu, 3));
                emu->quit=1;
        }
}
