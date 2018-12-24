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


void Run0F(x86emu_t *emu)
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
            case 0x49: /* CMOVNS Gd, Ed */ // conditional move, no sign
                nextop = Fetch8(emu);
                GetEd(emu, &op2, &ea2, nextop);
                GetG(emu, &op1, nextop);
                if(!ACCESS_FLAG(F_SF))
                    op1->dword[0] = op2->dword[0];
                break;
            case 0x4E: /* CMOVLE Gd, Ed */ // conditional move, no sign
                nextop = Fetch8(emu);
                GetEd(emu, &op2, &ea2, nextop);
                GetG(emu, &op1, nextop);
                if(ACCESS_FLAG(F_ZF) || (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF)))
                    op1->dword[0] = op2->dword[0];
                break;

            case 0x80: /* JO */
                tmp32s = Fetch32s(emu);
                if(ACCESS_FLAG(F_OF))
                    R_EIP += tmp32s;
                break;
            case 0x81: /* JNO */
                tmp32s = Fetch32s(emu);
                if(!ACCESS_FLAG(F_OF))
                    R_EIP += tmp32s;
                break;
            case 0x82: /* JB */
                tmp32s = Fetch32s(emu);
                if(ACCESS_FLAG(F_CF))
                    R_EIP += tmp32s;
                break;
            case 0x83: /* JNB */
                tmp32s = Fetch32s(emu);
                if(!ACCESS_FLAG(F_CF))
                    R_EIP += tmp32s;
                break;
            case 0x84: /* JZ */
                tmp32s = Fetch32s(emu);
                if(ACCESS_FLAG(F_ZF))
                    R_EIP += tmp32s;
                break;
            case 0x85: /* JNZ */
                tmp32s = Fetch32s(emu);
                if(!ACCESS_FLAG(F_ZF))
                    R_EIP += tmp32s;
                break;
            case 0x86: /* JBE */
                tmp32s = Fetch32s(emu);
                if((ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF)))
                    R_EIP += tmp32s;
                break;
            case 0x87: /* JNBE */
                tmp32s = Fetch32s(emu);
                if(!(ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF)))
                    R_EIP += tmp32s;
                break;
            case 0x88: /* JS */
                tmp32s = Fetch32s(emu);
                if(ACCESS_FLAG(F_SF))
                    R_EIP += tmp32s;
                break;
            case 0x89: /* JNZ */
                tmp32s = Fetch32s(emu);
                if(!ACCESS_FLAG(F_SF))
                    R_EIP += tmp32s;
                break;
            case 0x8A: /* JP */
                tmp32s = Fetch32s(emu);
                if(ACCESS_FLAG(F_PF))
                    R_EIP += tmp32s;
                break;
            case 0x8B: /* JNP */
                tmp32s = Fetch32s(emu);
                if(!ACCESS_FLAG(F_PF))
                    R_EIP += tmp32s;
                break;
            case 0x8C: /* JL */
                tmp32s = Fetch32s(emu);
                if(ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))
                    R_EIP += tmp32s;
                break;
            case 0x8D: /* JNL */
                tmp32s = Fetch32s(emu);
                if(ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF))
                    R_EIP += tmp32s;
                break;
            case 0x8E: /* JLE */
                tmp32s = Fetch32s(emu);
                if(ACCESS_FLAG(F_ZF) || (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF)))
                    R_EIP += tmp32s;
                break;
            case 0x8F: /* JNLE */
                tmp32s = Fetch32s(emu);
                if(!ACCESS_FLAG(F_ZF) && (ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF)))
                    R_EIP += tmp32s;
                break;
            
            case 0xA3: /* BT Ed, Gd */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea1, nextop);
                GetG(emu, &op2, nextop);
                if(op1->dword[0] & (1<<(op2->dword[0]&31)))
                    SET_FLAG(F_CF);
                else
                    CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
                CLEAR_FLAG(F_SF);
                CLEAR_FLAG(F_ZF);
                CLEAR_FLAG(F_AF);
                CLEAR_FLAG(F_PF);
                break;

            case 0xAF: /* IMUL Gd, Ed */
                nextop = Fetch8(emu);
                GetEd(emu, &op2, &ea2, nextop);
                GetG(emu, &op1, nextop);
                op1->dword[0] = imul32(emu, op1->dword[0], op2->dword[0]);
                break;

            case 0xB6: /* MOVZX Gd, Eb */ //Move with zero extend
                nextop = Fetch8(emu);
                GetEb(emu, &op2, &ea2, nextop);
                GetG(emu, &op1, nextop);
                op1->dword[0] = op2->byte[0];
                break;

            default:
                printf_log(LOG_NONE, "Unimplemented Opcode 0F %02X %02X %02X %02X %02X\n", opcode, Peek(emu, 0), Peek(emu, 1), Peek(emu, 2), Peek(emu, 3));
                emu->quit=1;
                emu->error |= ERR_UNIMPL;
        }
}