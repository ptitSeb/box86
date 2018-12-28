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
#include "x87emu_private.h"

int Run(x86emu_t *emu)
{
    printf_log(LOG_DEBUG, "Run X86, EIP=%p\n", emu, R_EIP);
    emu->quit = 0;
    while (!emu->quit)
    {
        if(emu->dec && (
                (emu->trace_end == 0) 
             || ((R_EIP >= emu->trace_start) && (R_EIP < emu->trace_end))) ) {
            printf_log(LOG_NONE, "%s", DumpCPURegs(emu));
            if(Peek(emu, 0)==0xcc && Peek(emu, 1)=='S' && Peek(emu, 2)=='C') {
                uint32_t a = *(uint32_t*)(R_EIP+3);
                if(a==0) {
                    printf_log(LOG_NONE, "%08p: Exit x86emu\n", R_EIP);
                } else {
                    printf_log(LOG_NONE, "%08p: Native call to %p\n", R_EIP, a);
                }
            } else {
                printf_log(LOG_NONE, "%s\n", DecodeX86Trace(emu->dec, R_EIP));
            }
        }
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
            case 0x00:  /* ADD Eb,Gb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                GetGb(emu, &op2, nextop);
                op2->byte[0] = add8(emu, op1->byte[0], op2->byte[0]);
                break;
            case 0x01: /* ADD Ed,Gd */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea1, nextop);
                GetG(emu, &op2, nextop);
                op1->dword[0] = add32(emu, op1->dword[0], op2->dword[0]);
                break;
            case 0x02: /* ADD Gd,Ed */
                nextop = Fetch8(emu);
                GetEd(emu, &op2, &ea2, nextop);
                GetG(emu, &op1, nextop);
                op1->dword[0] = add32(emu, op1->dword[0], op2->dword[0]);
                break;
            case 0x04: /* ADD AL, Ib */
                tmp8u = Fetch8(emu);
                R_AL = add8(emu, R_AL, tmp8u);
                break;
            case 0x05: /* ADD EAX, Id */
                tmp32u = Fetch32(emu);
                R_EAX = add32(emu, R_EAX, tmp32u);
                break;

            case 0x08: /* OR Eb,Gb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                GetG(emu, &op2, nextop);
                op1->byte[0] = or8(emu, op1->byte[0], op2->byte[0]);
                break;
            case 0x09: /* OR Ed,Gd */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea1, nextop);
                GetG(emu, &op2, nextop);
                op1->dword[0] = or32(emu, op1->dword[0], op2->dword[0]);
                break;

            case 0x0F: /* More instructions */
                Run0F(emu);
                break;

            case 0x21: /* AND Ed,Gd */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea1, nextop);
                GetG(emu, &op2, nextop);
                op1->dword[0] = and32(emu, op1->dword[0], op2->dword[0]);
                break;
            case 0x23: /* AND Gd,Ed */
                nextop = Fetch8(emu);
                GetEd(emu, &op2, &ea2, nextop);
                GetG(emu, &op1, nextop);
                op1->dword[0] = and32(emu, op1->dword[0], op2->dword[0]);
                break;
            case 0x25: /* AND EAX, Id */
                R_EAX = and32(emu, R_EAX, Fetch32(emu));
                break;
            case 0x29: /* SUB Ed,Gd */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea1, nextop);
                GetG(emu, &op2, nextop);
                op1->dword[0] = sub32(emu, op1->dword[0], op2->dword[0]);
                break;
            case 0x2B: /* SUB Gd,Ed */
                nextop = Fetch8(emu);
                GetEd(emu, &op2, &ea2, nextop);
                GetG(emu, &op1, nextop);
                op1->dword[0] = sub32(emu, op1->dword[0], op2->dword[0]);
                break;
            case 0x2D: /* SUB EAX, Id */
                R_EAX = sub32(emu, R_EAX, Fetch32(emu));
                break;
            case 0x31: /* XOR Ed,Gd */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea1, nextop);
                GetG(emu, &op2, nextop);
                op1->dword[0] = xor32(emu, op1->dword[0], op2->dword[0]);
                break;
            case 0x33: /* XOR Gd,Ed */
                nextop = Fetch8(emu);
                GetEd(emu, &op2, &ea2, nextop);
                GetG(emu, &op1, nextop);
                op1->dword[0] = xor32(emu, op1->dword[0], op2->dword[0]);
                break;
            case 0x35: /* XOR EAX, Id */
                op1->dword[0] = xor32(emu, R_EAX, Fetch32(emu));
                break;
            case 0x39: /* CMP Ed,Gd */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea1, nextop);
                GetG(emu, &op2, nextop);
                cmp32(emu, op1->dword[0], op2->dword[0]);
                break;
            case 0x3B: /* CMP Gd,Ed */
                nextop = Fetch8(emu);
                GetEd(emu, &op2, &ea2, nextop);
                GetG(emu, &op1, nextop);
                cmp32(emu, op1->dword[0], op2->dword[0]);
                break;
            case 0x3D: /* CMP EAX, Id */
                cmp32(emu, R_EAX, Fetch32(emu));
                break;
            case 0x40:
            case 0x41:
            case 0x42:
            case 0x43:
            case 0x44:
            case 0x45:
            case 0x46:
            case 0x47:  /* INC Reg */
                tmp8u = opcode&7;
                emu->regs[tmp8u].dword[0] = inc32(emu, emu->regs[tmp8u].dword[0]);
                break;
            case 0x48:
            case 0x49:
            case 0x4A:
            case 0x4B:
            case 0x4C:
            case 0x4D:
            case 0x4E:
            case 0x4F:  /*DEC Reg */
                tmp8u = opcode&7;
                emu->regs[tmp8u].dword[0] = dec32(emu, emu->regs[tmp8u].dword[0]);
                break;
            case 0x50:
            case 0x51:
            case 0x52:
            case 0x53:
            case 0x54:
            case 0x55:
            case 0x56:
            case 0x57:  /* PUSH Reg */
                tmp8u = opcode-0x50;
                Push(emu, emu->regs[tmp8u].dword[0]);
                break;
            case 0x58:
            case 0x59:
            case 0x5A:
            case 0x5B:
            case 0x5C:
            case 0x5D:
            case 0x5E:
            case 0x5F:  /*POP Reg */
                tmp8u = opcode-0x58;
                emu->regs[tmp8u].dword[0] = Pop(emu);
                break;
            case 0x65:  /* GS: */
                // TODO: set a new decoder function?
                opcode = Fetch8(emu);
                switch(opcode) {
                    case 0x33: /* XOR Gd,Ed */
                        nextop = Fetch8(emu);
                        GetEd(emu, &op2, &ea2, nextop);
                        op2 = (reg32_t*)(((char*)op2) + (uintptr_t)emu->globals);
                        GetG(emu, &op1, nextop);
                        op1->dword[0] = xor32(emu, op1->dword[0], op2->dword[0]);
                        break;
                    case 0xA1: /* MOV EAX, Ov */
                        tmp32u = Fetch32(emu);
                        R_EAX = *(uint32_t*)(((uintptr_t)emu->globals) + tmp32u);
                        break;

                    default:
                        printf_log(LOG_NONE, "Unimplemented Opcode 66 %02X %02X %02X %02X\n", opcode, Peek(emu, 0), Peek(emu, 1), Peek(emu, 2));
                        emu->quit=1;
                        emu->error |= ERR_UNIMPL;
                }
                break;
            case 0x66: /* Prefix for changing width of intructions, so here, down to 16bits */
                Run66(emu);
                break;
            case 0x68: /* Push Id */
                Push(emu, Fetch32(emu));
                break;
            case 0x6A: /* Push Ib */
                Push(emu, Fetch8s(emu));
                break;

            case 0x72:  /* JB Ib */
                tmp8s = Fetch8s(emu);
                if(ACCESS_FLAG(F_CF))
                    R_EIP += tmp8s;
                break;
            case 0x73:  /* JNB Ib */
                tmp8s = Fetch8s(emu);
                if(!ACCESS_FLAG(F_CF))
                    R_EIP += tmp8s;
                break;
            case 0x74:  /* JZ Ib */
                tmp8s = Fetch8s(emu);
                if(ACCESS_FLAG(F_ZF))
                    R_EIP += tmp8s;
                break;
            case 0x75:  /* JNZ Ib */
                tmp8s = Fetch8s(emu);
                if(!ACCESS_FLAG(F_ZF))
                    R_EIP += tmp8s;
                break;
            case 0x76:  /* JBE Ib */
                tmp8s = Fetch8s(emu);
                if((ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF)))
                    R_EIP += tmp8s;
                break;
            case 0x77:  /* JNBE Ib */
                tmp8s = Fetch8s(emu);
                if(!(ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF)))
                    R_EIP += tmp8s;
                break;
            case 0x7C: /* JL Ib */
                tmp8s = Fetch8s(emu);
                if(ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))
                    R_EIP += tmp8s;
                break;
            case 0x7D: /* JNL Ib */
                tmp8s = Fetch8s(emu);
                if(ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF))
                    R_EIP += tmp8s;
                break;
            case 0x7E: /* JLE Ib */
                tmp8s = Fetch8s(emu);
                if(ACCESS_FLAG(F_ZF) || (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF)))
                    R_EIP += tmp8s;
                break;
            case 0x7F: /* JNLE Ib */
                tmp8s = Fetch8s(emu);
                if(!ACCESS_FLAG(F_ZF) && (ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF)))
                    R_EIP += tmp8s;
                break;
            
            case 0x81: /* Grp Ed, Id */
            case 0x83:  /* Grpl Ed,Ix */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea1, nextop);
                if(opcode==0x81) 
                    tmp32u = Fetch32(emu);
                else {
                    tmp32s = Fetch8s(emu);
                    tmp32u = *(uint32_t*)&tmp32s;
                }
                switch((nextop>>3)&7) {
                    case 0: op1->dword[0] = add32(emu, op1->dword[0], tmp32u); break;
                    case 1: op1->dword[0] =  or32(emu, op1->dword[0], tmp32u); break;
                    case 2: op1->dword[0] = adc32(emu, op1->dword[0], tmp32u); break;
                    case 3: op1->dword[0] = sbb32(emu, op1->dword[0], tmp32u); break;
                    case 4: op1->dword[0] = and32(emu, op1->dword[0], tmp32u); break;
                    case 5: op1->dword[0] = sub32(emu, op1->dword[0], tmp32u); break;
                    case 6: op1->dword[0] = xor32(emu, op1->dword[0], tmp32u); break;
                    case 7: op1->dword[0] = cmp32(emu, op1->dword[0], tmp32u); break;
                }
                break;

            case 0x85:  /* TEST Ed,Gd */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea2, nextop);
                GetG(emu, &op2, nextop);
                test32(emu, op1->dword[0], op2->dword[0]);
                break;
            case 0x87:  /* XCHG Ed,Gd */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea2, nextop);
                GetG(emu, &op2, nextop);
                tmp32u = op1->dword[0];
                op1->dword[0] = op2->dword[0];
                op2->dword[0] = tmp32u;
                break;

            case 0x88:  /* MOV Eb, Gv */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea2, nextop);
                GetG(emu, &op2, nextop);
                op1->byte[0] = op2->byte[0];
                break;
            case 0x89:  /* MOV Ed, Gv */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea2, nextop);
                GetG(emu, &op2, nextop);
                op1->dword[0] = op2->dword[0];
                break;

            case 0x8B: /* MOV Gd, Ed */
                nextop = Fetch8(emu);
                GetEd(emu, &op2, &ea2, nextop);
                GetG(emu, &op1, nextop);
                op1->dword[0] = op2->dword[0];
                break;

            case 0x8D: /* LEA Gd, M */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea2, nextop);
                GetG(emu, &op2, nextop);
                op2->dword[0] = (uint32_t)&op1->dword[0];
                break;
            
            case 0x90: /* NOP */
                break;

            case 0xA1: /* MOV EAX, Od */
                R_EAX = *(uint32_t*)Fetch32(emu);
                break;
            case 0xA3: /* MOV Od, EAX */
                *(uint32_t*)Fetch32(emu) = R_EAX;
                break;
            
            case 0xB8: /* MOV EAX,Id */
            case 0xB9: /* MOV ECX,Id */
            case 0xBA: /* MOV EDX,Id */
            case 0xBB: /* MOV EBX,Id */
            case 0xBC: /*    ...     */
            case 0xBD:
            case 0xBE:
            case 0xBF:
                emu->regs[opcode-0xB8].dword[0] = Fetch32(emu);
                break;

            case 0xC1: /* GRP2 Eb,Ib */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea2, nextop);
                tmp8u = Fetch8(emu) & 0x1f;
                switch((nextop>>3)&7) {
                    case 0: op1->dword[0] = rol32(emu, op1->dword[0], tmp8u); break;
                    case 1: op1->dword[0] = ror32(emu, op1->dword[0], tmp8u); break;
                    case 2: op1->dword[0] = rcl32(emu, op1->dword[0], tmp8u); break;
                    case 3: op1->dword[0] = rcr32(emu, op1->dword[0], tmp8u); break;
                    case 4:
                    case 6: op1->dword[0] = shl32(emu, op1->dword[0], tmp8u); break;
                    case 5: op1->dword[0] = shr32(emu, op1->dword[0], tmp8u); break;
                    case 7: op1->dword[0] = sar32(emu, op1->dword[0], tmp8u); break;
                }
                break;
            case 0xC3: /* RET */
                R_EIP = Pop(emu);
                break;

            case 0xC7: /* MOV Ed,Id */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea2, nextop);
                op1->dword[0] = Fetch32(emu);
                break;

            case 0xC9: /* LEAVE */
                R_ESP = R_EBP;
                R_EBP = Pop(emu);
                break;

            case 0xCC: /* INT 3 */
                x86Int3(emu);
                break;
            case 0xCD: /* INT Ib */
                nextop = Fetch8(emu);
                if(nextop == 0x80) 
                    x86Syscall(emu);
                else {
                    printf_log(LOG_NONE, "Unsupported Int %02Xh\n", nextop);
                    emu->quit = 1;
                    emu->error |= ERR_UNIMPL;
                }
                break;

            case 0xD1: /* GRP2 Eb,1 */
            case 0xD3: /* GRP2 Eb,CL */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea2, nextop);
                tmp8u = (opcode==0xD1)?1:R_CL;
                switch((nextop>>3)&7) {
                    case 0: op1->dword[0] = rol32(emu, op1->dword[0], tmp8u); break;
                    case 1: op1->dword[0] = ror32(emu, op1->dword[0], tmp8u); break;
                    case 2: op1->dword[0] = rcl32(emu, op1->dword[0], tmp8u); break;
                    case 3: op1->dword[0] = rcr32(emu, op1->dword[0], tmp8u); break;
                    case 4: 
                    case 6: op1->dword[0] = shl32(emu, op1->dword[0], tmp8u); break;
                    case 5: op1->dword[0] = shr32(emu, op1->dword[0], tmp8u); break;
                    case 7: op1->dword[0] = sar32(emu, op1->dword[0], tmp8u); break;
                }
                break;
            
            case 0xD9: /* x87 */
                RunD9(emu);
                break;

            case 0xDB: /* x87 */
                RunDB(emu);
                break;

            case 0xDD: /* x87 */
                RunDD(emu);
                break;
            case 0xDE: /* x87 */
                RunDE(emu);
                break;

            case 0xE8:  /* CALL Id */
                tmp32s = Fetch32s(emu); // call is relative
                Push(emu, R_EIP);
                R_EIP += tmp32s;
                break;
            case 0xE9:  /* JMP Id */
                tmp32s = Fetch32s(emu); // jmp is relative
                R_EIP += tmp32s;
                break;

            case 0xEB:  /* JMP Ib */
                tmp32s = Fetch8s(emu); // jump is relative
                R_EIP += tmp32s;
                break;
            case 0xF3:  /* REPZ prefix */
                nextop = Fetch8(emu);
                switch(nextop) {
                    case 0xC3:  /* repz ret... yup */
                        R_EIP = Pop(emu);
                        break;
                    default:
                        printf_log(LOG_NONE, "Unimplemented Opcode F3 %02X %02X \n", nextop, Peek(emu, 0));
                        emu->quit=1;
                        emu->error |= ERR_UNIMPL;
                }
                break;

            case 0xFF: /* GRP 5 Ed */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea2, nextop);
                switch((nextop>>3)&7) {
                    case 0: /* INC Ed */
                        op1->dword[0] = inc32(emu, op1->dword[0]);
                        break;
                    case 1: /* DEC Ed */
                        op1->dword[0] = dec32(emu, op1->dword[0]);
                        break;
                    case 2: /* CALL NEAR Ed */
                        Push(emu, R_EIP);
                        R_EIP = op1->dword[0];
                        break;
                    case 3: /* CALL FAR Ed */
                        if(nextop>0xc0) {
                            printf_log(LOG_NONE, "Illegal Opcode 0x%02X 0x%02X\n", opcode, nextop);
                            emu->quit=1;
                            emu->error |= ERR_ILLEGAL;
                        } else {
                            Push16(emu, R_CS);
                            Push(emu, R_EIP);
                            R_EIP = op1->dword[0];
                            R_CS = (op1+1)->word[0];
                        }
                        break;
                    case 4: /* JMP NEAR Ed */
                        R_EIP = op1->dword[0];
                        break;
                    case 5: /* JMP FAR Ed */
                        if(nextop>0xc0) {
                            printf_log(LOG_NONE, "Illegal Opcode 0x%02X 0x%02X\n", opcode, nextop);
                            emu->quit=1;
                            emu->error |= ERR_ILLEGAL;
                        } else {
                            R_EIP = op1->dword[0];
                            R_CS = (op1+1)->word[0];
                        }
                        break;
                    case 6: /* Push Ed */
                        Push(emu, op1->dword[0]);
                        break;
                    default:
                        printf_log(LOG_NONE, "Illegal Opcode 0x%02X 0x%02X\n", opcode, nextop);
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                }
                break;

            default:
                printf_log(LOG_NONE, "Unimplemented Opcode %02X %02X %02X %02X %02X\n", opcode, Peek(emu, 0), Peek(emu, 1), Peek(emu, 2), Peek(emu, 3));
                emu->quit=1;
                emu->error |= ERR_UNIMPL;
        }
    }
}