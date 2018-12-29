#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        sse_regs_t *opx1, *opx2;
        sse_regs_t eax1;
        switch(opcode) {
            case 0x10: /* MOVUPS Gd, Ed */
                nextop = Fetch8(emu);
                GetEx(emu, &opx2, &eax1, nextop);
                GetGx(emu, &opx1, nextop);
                memcpy(opx1, opx2, sizeof(sse_regs_t));
                break;
            case 0x11: /* MOVUPS Ed, Gd */
                nextop = Fetch8(emu);
                GetEx(emu, &opx1, &eax1, nextop);
                GetGx(emu, &opx2, nextop);
                memcpy(opx1, opx2, sizeof(sse_regs_t));
                break;

            case 0x28: /* MOVAPS Gd, Ed */
                nextop = Fetch8(emu);
                GetEx(emu, &opx2, &eax1, nextop);
                GetGx(emu, &opx1, nextop);
                memcpy(opx1, opx2, sizeof(sse_regs_t));
                break;

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
            case 0x90: /* SETO Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                op1->byte[0] = (ACCESS_FLAG(F_OF))?1:0;
                break;
            case 0x91: /* SETNO Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                op1->byte[0] = (!ACCESS_FLAG(F_OF))?1:0;
                break;
            case 0x92: /* SETB Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                op1->byte[0] = (ACCESS_FLAG(F_CF))?1:0;
                break;
            case 0x93: /* SETNB Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                op1->byte[0] = (!ACCESS_FLAG(F_CF))?1:0;
                break;
            case 0x94: /* SETZ Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                op1->byte[0] = (ACCESS_FLAG(F_ZF))?1:0;
                break;
            case 0x95: /* SETNZ Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                op1->byte[0] = (!ACCESS_FLAG(F_ZF))?1:0;
                break;
            case 0x96: /* SETBE Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                op1->byte[0] = ((ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF)))?1:0;
                break;
            case 0x97: /* SETNBE Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                op1->byte[0] = (!(ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF)))?1:0;
                break;
            case 0x98: /* SETS Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                op1->byte[0] = (ACCESS_FLAG(F_SF))?1:0;
                break;
            case 0x99: /* SETNZ Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                op1->byte[0] = (!ACCESS_FLAG(F_SF))?1:0;
                break;
            case 0x9A: /* SETP Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                op1->byte[0] = (ACCESS_FLAG(F_PF))?1:0;
                break;
            case 0x9B: /* SETNP Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                op1->byte[0] = (!ACCESS_FLAG(F_PF))?1:0;
                break;
            case 0x9C: /* SETL Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                op1->byte[0] = (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))?1:0;
                break;
            case 0x9D: /* SETNL Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                op1->byte[0] = (ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF))?1:0;
                break;
            case 0x9E: /* SETLE Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                op1->byte[0] = (ACCESS_FLAG(F_ZF) || (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF)))?1:0;
                break;
            case 0x9F: /* SETNLE Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, &ea1, nextop);
                op1->byte[0] = (!ACCESS_FLAG(F_ZF) && (ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF)))?1:0;
                break;
            case 0xA2: /* CPUID */
                tmp32u = R_EAX;
                switch(tmp32u) {
                    case 0x0:
                        // emulate a P4
                        R_EAX = 0x80000004;
                        // return GuenuineIntel
                        R_EBX = 0x756E6547;
                        R_EDX = 0x49656E69;
                        R_ECX = 0x6C65746E;
                        break;
                    case 0x1:
                        R_EAX = 0x00000101; // familly and all
                        R_EBX = 0;  // Brand indexe, CLFlush, Max APIC ID, Local APIC ID
                        R_EDX =   1     // fpu 
                                | 1<<8  // cmpxchg8
                                | 1<<11 // sep (sysenter & sysexit)
                                | 1<<15 // cmov
                                | 1<<19 // clflush (seems to be with SSE2)
                                | 1<<23 // mmx
                                | 1<<24 // fxsr (fxsave, fxrestore)
                                | 1<<25 // SSE
                                | 1<<26 // SSE2
                                ;
                        R_ECX =   1<<12 // fma
                                | 1<<13 // cx16 (cmpxchg16)
                                ;       // also 1<<0 is SSE3 and 1<<9 is SSSE3
                        break;
                    default:
                        printf_log(LOG_INFO, "Warning, CPUID command %X unsupported\n", tmp32u);
                        R_EAX = 0;
                }
                break;
            case 0xA3: /* BT Ed, Gd */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea1, nextop);
                GetG(emu, &op2, nextop);
                CLEAR_FLAG(F_CF);
                CLEAR_FLAG(F_OF);
                CLEAR_FLAG(F_SF);
                CLEAR_FLAG(F_ZF);
                CLEAR_FLAG(F_AF);
                CLEAR_FLAG(F_PF);
                if(op1->dword[0] & (1<<(op2->dword[0]&31)))
                    SET_FLAG(F_CF);
                break;

            case 0xAE: /* Grp Ed (SSE) */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea1, nextop);
                switch((nextop>>3)&7) {
                    case 2: /* LDMXCSR Md */
                        emu->mxcsr = op1->dword[0];
                        break;
                    case 3: /* SDMXCSR Md */
                        op1->dword[0] = emu->mxcsr;
                        break;
                    default:
                        printf_log(LOG_NONE, "Unimplemented Opcode 0F %02X %02X ...\n", opcode, nextop);
                        emu->quit=1;
                        emu->error |= ERR_UNIMPL;
                }
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
            case 0xB7: /* MOVZX Gd, Ew */ //Move with zero extend
                nextop = Fetch8(emu);
                GetEw(emu, &op2, &ea2, nextop);
                GetG(emu, &op1, nextop);
                op1->dword[0] = op2->word[0];
                break;

            default:
                printf_log(LOG_NONE, "Unimplemented Opcode 0F %02X %02X %02X %02X %02X\n", opcode, Peek(emu, 0), Peek(emu, 1), Peek(emu, 2), Peek(emu, 3));
                emu->quit=1;
                emu->error |= ERR_UNIMPL;
        }
}