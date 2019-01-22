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
#include "box86context.h"

int Run(x86emu_t *emu)
{
    //ref opcode: http://ref.x86asm.net/geek32.html#xA1
    printf_log(LOG_DEBUG, "Run X86, EIP=%p, Stack=%p\n", (void*)R_EIP, emu->context->stack);
x86emurun:
    emu->quit = 0;
    while (!emu->quit)
    {
        emu->old_ip = R_EIP;
        if(emu->dec && (
                (emu->trace_end == 0) 
             || ((R_EIP >= emu->trace_start) && (R_EIP < emu->trace_end))) ) {
            printf_log(LOG_NONE, "%s", DumpCPURegs(emu));
            if(Peek(emu, 0)==0xcc && Peek(emu, 1)=='S' && Peek(emu, 2)=='C') {
                uint32_t a = *(uint32_t*)(R_EIP+3);
                if(a==0) {
                    printf_log(LOG_NONE, "0x%p: Exit x86emu\n", (void*)R_EIP);
                } else {
                    printf_log(LOG_NONE, "0x%p: Native call to %p => %s\n", (void*)R_EIP, (void*)a, GetNativeName(*(void**)(R_EIP+7)));
                }
            } else {
                printf_log(LOG_NONE, "%s", DecodeX86Trace(emu->dec, R_EIP));
                uint8_t peek = Peek(emu, 0);
                if(peek==0xC3 || peek==0xC2) {
                    printf_log(LOG_NONE, " => %p", *(void**)(R_ESP));
                } else if(peek==0x55) {
                    printf_log(LOG_NONE, " => STACK_TOP: %p", *(void**)(R_ESP));
                }
                printf_log(LOG_NONE, "\n");
            }
        }
        uint8_t opcode = Fetch8(emu);
        uint8_t nextop;
        reg32_t *op1, *op2, *op3, *op4;
        reg32_t ea1, ea2, ea3, ea4;
        uint8_t tmp8u, tmp8u2;
        int8_t tmp8s;
        uint16_t tmp16u;
        int16_t tmp16s;
        uint32_t tmp32u, tmp32u2, tmp32u3;
        int32_t tmp32s;
        uint64_t tmp64u;
        int64_t tmp64s;
        switch(opcode) {
            #define GO(B, OP)                       \
            case B+0:                               \
                nextop = Fetch8(emu);               \
                GetEb(emu, &op1, nextop);     \
                GetGb(emu, &op2, nextop);           \
                op1->byte[0] = OP##8(emu, op1->byte[0], op2->byte[0]);  \
                break;                              \
            case B+1:                               \
                nextop = Fetch8(emu);               \
                GetEd(emu, &op1, nextop);     \
                GetG(emu, &op2, nextop);            \
                op1->dword[0] = OP##32(emu, op1->dword[0], op2->dword[0]); \
                break;                              \
            case B+2:                               \
                nextop = Fetch8(emu);               \
                GetEb(emu, &op2, nextop);     \
                GetGb(emu, &op1, nextop);           \
                op1->byte[0] = OP##8(emu, op1->byte[0], op2->byte[0]); \
                break;                              \
            case B+3:                               \
                nextop = Fetch8(emu);               \
                GetEd(emu, &op2, nextop);     \
                GetG(emu, &op1, nextop);            \
                op1->dword[0] = OP##32(emu, op1->dword[0], op2->dword[0]); \
                break;                              \
            case B+4:                               \
                R_AL = OP##8(emu, R_AL, Fetch8(emu)); \
                break;                              \
            case B+5:                               \
                R_EAX = OP##32(emu, R_EAX, Fetch32(emu)); \
                break;

            GO(0x00, add)                   /* ADD 0x00 -> 0x05 */
            GO(0x08, or)                    /*  OR 0x08 -> 0x0D */
            GO(0x10, adc)                   /* ADC 0x10 -> 0x15 */
            GO(0x18, sbb)                   /* SBB 0x18 -> 0x1D */
            GO(0x20, and)                   /* AND 0x20 -> 0x25 */
            GO(0x28, sub)                   /* SUB 0x28 -> 0x2D */
            GO(0x30, xor)                   /* XOR 0x30 -> 0x35 */
            //GO(0x38, cmp)                   /* CMP 0x38 -> 0x3D */    avoid affectation

            #undef GO
            case 0x38:
                nextop = Fetch8(emu);
                GetEb(emu, &op1, nextop);
                GetGb(emu, &op2, nextop);
                cmp8(emu, op1->byte[0], op2->byte[0]);
                break;
            case 0x39:
                nextop = Fetch8(emu);
                GetEd(emu, &op1, nextop);
                GetG(emu, &op2, nextop);
                cmp32(emu, op1->dword[0], op2->dword[0]);
                break;
            case 0x3A:
                nextop = Fetch8(emu);
                GetEb(emu, &op2, nextop);
                GetGb(emu, &op1, nextop);
                cmp8(emu, op1->byte[0], op2->byte[0]);
                break;
            case 0x3B:
                nextop = Fetch8(emu);
                GetEd(emu, &op2, nextop);
                GetG(emu, &op1, nextop);
                cmp32(emu, op1->dword[0], op2->dword[0]);
                break;
            case 0x3C:
                cmp8(emu, R_AL, Fetch8(emu));
                break;
            case 0x3D:
                cmp32(emu, R_EAX, Fetch32(emu));
                break;

            case 0x0F:                      /* More instructions */
                Run0F(emu);
                break;

            case 0x27:                      /* DAA */
                R_AL = daa8(emu, R_AL);
                break;
            case 0x2F:                      /* DAS */
                R_AL = das8(emu, R_AL);
                break;
            case 0x37:                      /* AAA */
                R_AX = aaa16(emu, R_AX);
                break;
            case 0x3F:                      /* AAS */
                R_AX = aas16(emu, R_AX);
                break;


            case 0x40:
            case 0x41:
            case 0x42:
            case 0x43:
            case 0x44:
            case 0x45:
            case 0x46:
            case 0x47:                      /* INC Reg */
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
            case 0x4F:                      /* DEC Reg */
                tmp8u = opcode&7;
                emu->regs[tmp8u].dword[0] = dec32(emu, emu->regs[tmp8u].dword[0]);
                break;
            case 0x54:                      /* PUSH ESP */
                tmp32u = R_ESP;
                Push(emu, tmp32u);
                break;
            case 0x50:
            case 0x51:
            case 0x52:
            case 0x53:
            case 0x55:
            case 0x56:
            case 0x57:                      /* PUSH Reg */
                tmp8u = opcode&7;
                Push(emu, emu->regs[tmp8u].dword[0]);
                break;
            case 0x5C:                      /* POP ESP */
                R_ESP += 4;
                break;
            case 0x58:
            case 0x59:
            case 0x5A:
            case 0x5B:
            case 0x5D:
            case 0x5E:
            case 0x5F:                      /* POP Reg */
                tmp8u = opcode&7;
                emu->regs[tmp8u].dword[0] = Pop(emu);
                break;

            case 0x65:                      /* GS: */
                // TODO: set a new decoder function?
                opcode = Fetch8(emu);
                switch(opcode) {
                    case 0x33:              /* XOR Gd,Ed */
                        nextop = Fetch8(emu);
                        GetEd(emu, &op2, nextop);
                        op2 = (reg32_t*)(((char*)op2) + (uintptr_t)emu->globals);
                        GetG(emu, &op1, nextop);
                        op1->dword[0] = xor32(emu, op1->dword[0], op2->dword[0]);
                        break;
                    case 0x8B:              /* MOV Gd,Ed */
                        nextop = Fetch8(emu);
                        GetEd(emu, &op2, nextop);
                        op2 = (reg32_t*)(((char*)op2) + (uintptr_t)emu->globals);
                        GetG(emu, &op1, nextop);
                        op1->dword[0] = op2->dword[0];
                        break;
                    case 0xA1:              /* MOV EAX,Ov */
                        tmp32u = Fetch32(emu);
                        R_EAX = *(uint32_t*)(((uintptr_t)emu->globals) + tmp32u);
                        break;

                    default:
                        UnimpOpcode(emu);
                }
                break;
            case 0x66:                      /* Prefix to change width of intructions, so here, down to 16bits */
                nextop = Peek(emu, 0);
                if(nextop==0x0F)
                    Run660F(emu);
                else if(nextop==0xD9)
                    Run66D9(emu);
                else if(nextop==0xDD)
                    Run66DD(emu);
                else
                    Run66(emu);
                break;
            case 0x67:                      /* Prefix to change width of intructions */
                Run67(emu); // implemented in Run66.c
                break;

            case 0x68:                      /* Push Id */
                Push(emu, Fetch32(emu));
                break;
            case 0x69:                      /* IMUL Gd,Ed,Id */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, nextop);
                GetG(emu, &op2, nextop);
                tmp32u = Fetch32(emu);
                op2->dword[0] = imul32(emu, op1->dword[0], tmp32u);
                break;
            case 0x6A:                      /* Push Ib */
                tmp32s = Fetch8s(emu);
                Push(emu, (uint32_t)tmp32s);
                break;
            case 0x6B:                      /* IMUL Gd,Ed,Ib */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, nextop);
                GetG(emu, &op2, nextop);
                tmp32s = Fetch8s(emu);
                op2->dword[0] = imul32(emu, op1->dword[0], (uint32_t)tmp32s);
                break;

            #define GOCOND(BASE, PREFIX, CONDITIONAL) \
            case BASE+0x0:                          \
                PREFIX                              \
                if(ACCESS_FLAG(F_OF))               \
                    CONDITIONAL                     \
                break;                              \
            case BASE+0x1:                          \
                PREFIX                              \
                if(!ACCESS_FLAG(F_OF))              \
                    CONDITIONAL                     \
                break;                              \
            case BASE+0x2:                          \
                PREFIX                              \
                if(ACCESS_FLAG(F_CF))               \
                    CONDITIONAL                     \
                break;                              \
            case BASE+0x3:                          \
                PREFIX                              \
                if(!ACCESS_FLAG(F_CF))              \
                    CONDITIONAL                     \
                break;                              \
            case BASE+0x4:                          \
                PREFIX                              \
                if(ACCESS_FLAG(F_ZF))               \
                    CONDITIONAL                     \
                break;                              \
            case BASE+0x5:                          \
                PREFIX                              \
                if(!ACCESS_FLAG(F_ZF))              \
                    CONDITIONAL                     \
                break;                              \
            case BASE+0x6:                          \
                PREFIX                              \
                if((ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF)))  \
                    CONDITIONAL                     \
                break;                              \
            case BASE+0x7:                          \
                PREFIX                              \
                if(!(ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF))) \
                    CONDITIONAL                     \
                break;                              \
            case BASE+0x8:                          \
                PREFIX                              \
                if(ACCESS_FLAG(F_SF))               \
                    CONDITIONAL                     \
                break;                              \
            case BASE+0x9:                          \
                PREFIX                              \
                if(!ACCESS_FLAG(F_SF))              \
                    CONDITIONAL                     \
                break;                              \
            case BASE+0xA:                          \
                PREFIX                              \
                if(ACCESS_FLAG(F_PF))               \
                    CONDITIONAL                     \
                break;                              \
            case BASE+0xB:                          \
                PREFIX                              \
                if(!ACCESS_FLAG(F_PF))              \
                    CONDITIONAL                     \
                break;                              \
            case BASE+0xC:                          \
                PREFIX                              \
                if(ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))  \
                    CONDITIONAL                     \
                break;                              \
            case BASE+0xD:                          \
                PREFIX                              \
                if(ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF)) \
                    CONDITIONAL                     \
                break;                              \
            case BASE+0xE:                          \
                PREFIX                              \
                if(ACCESS_FLAG(F_ZF) || (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))) \
                    CONDITIONAL                     \
                break;                              \
            case BASE+0xF:                          \
                PREFIX                              \
                if(!ACCESS_FLAG(F_ZF) && (ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF))) \
                    CONDITIONAL                     \
                break;
            GOCOND(0x70
                ,   tmp8s = Fetch8s(emu);
                ,   R_EIP += tmp8s;
                )                           /* Jxx Ib */
            #undef GOCOND

            
            case 0x80:                      /* GRP Eb,Ib */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, nextop);
                tmp8u = Fetch8(emu);
                switch((nextop>>3)&7) {
                    case 0: op1->byte[0] = add8(emu, op1->byte[0], tmp8u); break;
                    case 1: op1->byte[0] =  or8(emu, op1->byte[0], tmp8u); break;
                    case 2: op1->byte[0] = adc8(emu, op1->byte[0], tmp8u); break;
                    case 3: op1->byte[0] = sbb8(emu, op1->byte[0], tmp8u); break;
                    case 4: op1->byte[0] = and8(emu, op1->byte[0], tmp8u); break;
                    case 5: op1->byte[0] = sub8(emu, op1->byte[0], tmp8u); break;
                    case 6: op1->byte[0] = xor8(emu, op1->byte[0], tmp8u); break;
                    case 7:                cmp8(emu, op1->byte[0], tmp8u); break;
                }
                break;
            case 0x81:                      /* GRP Ed,Id */
            case 0x83:                      /* GRP Ed,Ib */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, nextop);
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
                    case 7:                 cmp32(emu, op1->dword[0], tmp32u); break;
                }
                break;
            case 0x84:                      /* TEST Eb,Gb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, nextop);
                GetGb(emu, &op2, nextop);
                test8(emu, op1->byte[0], op2->byte[0]);
                break;
            case 0x85:                      /* TEST Ed,Gd */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, nextop);
                GetG(emu, &op2, nextop);
                test32(emu, op1->dword[0], op2->dword[0]);
                break;
            case 0x86:                      /* XCHG Eb,Gb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, nextop);
                GetGb(emu, &op2, nextop);
                tmp8u = op1->byte[0];
                op1->byte[0] = op2->byte[0];
                op2->byte[0] = tmp8u;
                break;
            case 0x87:                      /* XCHG Ed,Gd */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, nextop);
                GetG(emu, &op2, nextop);
                tmp32u = op1->dword[0];
                op1->dword[0] = op2->dword[0];
                op2->dword[0] = tmp32u;
                break;
            case 0x88:                      /* MOV Eb,Gb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, nextop);
                GetGb(emu, &op2, nextop);
                op1->byte[0] = op2->byte[0];
                break;
            case 0x89:                      /* MOV Ed,Gd */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, nextop);
                GetG(emu, &op2, nextop);
                op1->dword[0] = op2->dword[0];
                break;
            case 0x8A:                      /* MOV Gb,Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op2, nextop);
                GetGb(emu, &op1, nextop);
                op1->byte[0] = op2->byte[0];
                break;
            case 0x8B:                      /* MOV Gd,Ed */
                nextop = Fetch8(emu);
                GetEd(emu, &op2, nextop);
                GetG(emu, &op1, nextop);
                op1->dword[0] = op2->dword[0];
                break;

            case 0x8D:                      /* LEA Gd,M */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, nextop);
                GetG(emu, &op2, nextop);
                op2->dword[0] = (uint32_t)&op1->dword[0];
                break;

            case 0x8F:                      /* POP Ed */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, nextop);
                op1->dword[0] = Pop(emu);
                break;
            case 0x90:                      /* NOP */
                break;
            case 0x91:                      
            case 0x92:                      
            case 0x93:                      
            case 0x94:                      
            case 0x95:                      
            case 0x96:                      
            case 0x97:                      /* XCHG reg,EAX */
                tmp32u = R_EAX;
                R_EAX = emu->regs[opcode&7].dword[0];
                emu->regs[opcode&7].dword[0] = tmp32u;
                break;

            case 0x98:                      /* CWDE */
                *(int32_t*)&R_EAX = (int16_t)R_AX;
                break;
            case 0x99:                      /* CDQ */
                if(R_EAX & 0x80000000)
                    R_EDX=0xFFFFFFFF;
                else
                    R_EDX=0x00000000;
                break;

            case 0x9B:                      /* FWAIT */
                break;
            case 0x9C:                      /* PUSHF */
                Push(emu, emu->eflags.x32);
                break;
            case 0x9D:                      /* POPF */
                emu->eflags.x32 = ((Pop(emu) & 0x3F7FD7) & (0xffff-40) ) | 0x2; // mask off res2 and res3 and on res1
                break;
            case 0x9E:                      /* SAHF */
                emu->regs[_AX].byte[1] = emu->eflags.x32&0xff;
                break;

            case 0xA0:                      /* MOV AL,Ob */
                R_AL = *(uint8_t*)Fetch32(emu);
                break;
            case 0xA1:                      /* MOV EAX,Od */
                R_EAX = *(uint32_t*)Fetch32(emu);
                break;
            case 0xA2:                      /* MOV Ob,AL */
                *(uint8_t*)Fetch32(emu) = R_AL;
                break;
            case 0xA3:                      /* MOV Od,EAX */
                *(uint32_t*)Fetch32(emu) = R_EAX;
                break;
            case 0xA4:                      /* MOVSB */
                tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
                *(uint8_t*)R_EDI = *(uint8_t*)R_ESI;
                R_EDI += tmp8s;
                R_ESI += tmp8s;
                break;
            case 0xA5:                      /* MOVSD */
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
                *(uint32_t*)R_EDI = *(uint32_t*)R_ESI;
                R_EDI += tmp8s;
                R_ESI += tmp8s;
                break;
            case 0xA6:                      /* CMPSB */
                tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
                tmp8u  = *(uint8_t*)R_EDI;
                tmp8u2 = *(uint8_t*)R_ESI;
                R_EDI += tmp8s;
                R_ESI += tmp8s;
                cmp8(emu, tmp8u2, tmp8u);
                break;
            case 0xA7:                      /* CMPSD */
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
                tmp32u  = *(uint32_t*)R_EDI;
                tmp32u2 = *(uint32_t*)R_ESI;
                R_EDI += tmp8s;
                R_ESI += tmp8s;
                cmp32(emu, tmp32u2, tmp32u);
                break;
            case 0xA8:                      /* TEST AL, Ib */
                test8(emu, R_AL, Fetch8(emu));
                break;
            case 0xA9:                      /* TEST EAX, Id */
                test32(emu, R_EAX, Fetch32(emu));
                break;
            case 0xAA:                      /* STOSB */
                tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
                *(uint8_t*)R_EDI = R_AL;
                R_EDI += tmp8s;
                break;
            case 0xAB:                      /* STOSD */
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
                *(uint32_t*)R_EDI = R_EAX;
                R_EDI += tmp8s;
                break;
            case 0xAC:                      /* LODSB */
                tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
                R_AL = *(uint8_t*)R_ESI;
                R_ESI += tmp8s;
                break;
            case 0xAD:                      /* LODSD */
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
                R_EAX = *(uint32_t*)R_ESI;
                R_ESI += tmp8s;
                break;
            case 0xAE:                      /* SCASB */
                tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
                cmp8(emu, R_AL, *(uint8_t*)R_EDI);
                R_EDI += tmp8s;
                break;
            case 0xAF:                      /* SCASD */
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
                cmp32(emu, R_EAX, *(uint32_t*)R_EDI);
                R_EDI += tmp8s;
                break;
            
            case 0xB0:                      /* MOV AL,Ib */
            case 0xB1:                      /* MOV CL,Ib */
            case 0xB2:                      /* MOV DL,Ib */
            case 0xB3:                      /* MOV BL,Ib */
            case 0xB4:                      /* MOV AH,Ib */
            case 0xB5:                      /*    ...    */
            case 0xB6:
            case 0xB7:
                tmp8u = opcode - 0xB0;
                emu->regs[tmp8u%4].byte[tmp8u/4] = Fetch8(emu);
                break;
            case 0xB8:                      /* MOV EAX,Id */
            case 0xB9:                      /* MOV ECX,Id */
            case 0xBA:                      /* MOV EDX,Id */
            case 0xBB:                      /* MOV EBX,Id */
            case 0xBC:                      /*    ...     */
            case 0xBD:
            case 0xBE:
            case 0xBF:
                emu->regs[opcode-0xB8].dword[0] = Fetch32(emu);
                break;

            case 0xC0:                      /* GRP2 Eb,Ib */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, nextop);
                tmp8u = Fetch8(emu)/* & 0x1f*/; // masking done in each functions
                switch((nextop>>3)&7) {
                    case 0: op1->byte[0] = rol8(emu, op1->byte[0], tmp8u); break;
                    case 1: op1->byte[0] = ror8(emu, op1->byte[0], tmp8u); break;
                    case 2: op1->byte[0] = rcl8(emu, op1->byte[0], tmp8u); break;
                    case 3: op1->byte[0] = rcr8(emu, op1->byte[0], tmp8u); break;
                    case 4:
                    case 6: op1->byte[0] = shl8(emu, op1->byte[0], tmp8u); break;
                    case 5: op1->byte[0] = shr8(emu, op1->byte[0], tmp8u); break;
                    case 7: op1->byte[0] = sar8(emu, op1->byte[0], tmp8u); break;
                }
                break;
            case 0xC1:                      /* GRP2 Ed,Ib */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, nextop);
                tmp8u = Fetch8(emu)/* & 0x1f*/; // masking done in each functions
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
            case 0xC2:                      /* RETN Iw */
                tmp16u = Fetch16(emu);
                R_EIP = Pop(emu);
                R_ESP += tmp16u;
                break;
            case 0xC3:                      /* RET */
                R_EIP = Pop(emu);
                break;

            case 0xC6:                      /* MOV Eb,Ib */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, nextop);
                op1->byte[0] = Fetch8(emu);
                break;
            case 0xC7:                      /* MOV Ed,Id */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, nextop);
                op1->dword[0] = Fetch32(emu);
                break;

            case 0xC9:                      /* LEAVE */
                R_ESP = R_EBP;
                R_EBP = Pop(emu);
                break;

            case 0xCC:                      /* INT 3 */
                x86Int3(emu);
                break;
            case 0xCD:                      /* INT Ib */
                nextop = Fetch8(emu);
                if(nextop == 0x80) 
                    x86Syscall(emu);
                else {
                    printf_log(LOG_NONE, "Unsupported Int %02Xh\n", nextop);
                    emu->quit = 1;
                    emu->error |= ERR_UNIMPL;
                }
                break;

            case 0xD1:                      /* GRP2 Ed,1 */
            case 0xD3:                      /* GRP2 Ed,CL */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, nextop);
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
            case 0xD0:                      /* GRP2 Eb,1 */
            case 0xD2:                      /* GRP2 Eb,CL */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, nextop);
                tmp8u = (opcode==0xD0)?1:R_CL;
                switch((nextop>>3)&7) {
                    case 0: op1->byte[0] = rol8(emu, op1->byte[0], tmp8u); break;
                    case 1: op1->byte[0] = ror8(emu, op1->byte[0], tmp8u); break;
                    case 2: op1->byte[0] = rcl8(emu, op1->byte[0], tmp8u); break;
                    case 3: op1->byte[0] = rcr8(emu, op1->byte[0], tmp8u); break;
                    case 4: 
                    case 6: op1->byte[0] = shl8(emu, op1->byte[0], tmp8u); break;
                    case 5: op1->byte[0] = shr8(emu, op1->byte[0], tmp8u); break;
                    case 7: op1->byte[0] = sar8(emu, op1->byte[0], tmp8u); break;
                }
                break;
            case 0xD4:                      /* AAM Ib */
                tmp8u = Fetch8(emu);
                R_AX = aam16(emu, R_AL, tmp8u);
                break;
            case 0xD5:                      /* AAD Ib */
                tmp8u = Fetch8(emu);
                R_AX = aad16(emu, R_AX, tmp8u);
                break;
            
            case 0xD7:                      /* XLAT */
                R_AL = *(uint8_t*)(R_EBX + R_AL);
                break;
            
            case 0xD8:                      /* x87 */
                RunD8(emu);
                break;
            case 0xD9:                      /* x87 */
                RunD9(emu);
                break;
            case 0xDA:                      /* x87 */
                RunDA(emu);
                break;
            case 0xDB:                      /* x87 */
                RunDB(emu);
                break;
            case 0xDC:                      /* x87 */
                RunDC(emu);
                break;
            case 0xDD:                      /* x87 */
                RunDD(emu);
                break;
            case 0xDE:                      /* x87 */
                RunDE(emu);
                break;
            case 0xDF:                      /* x87 */
                RunDF(emu);
                break;

            case 0xE0:                      /* LOOPNZ */
                tmp8s = Fetch8s(emu);
                --R_ECX; // don't update flags
                if(R_ECX && !ACCESS_FLAG(F_ZF))
                    R_EIP += tmp8s;
                break;
            case 0xE1:                      /* LOOPZ */
                tmp8s = Fetch8s(emu);
                --R_ECX; // don't update flags
                if(R_ECX && ACCESS_FLAG(F_ZF))
                    R_EIP += tmp8s;
                break;
            case 0xE2:                      /* LOOP */
                tmp8s = Fetch8s(emu);
                --R_ECX; // don't update flags
                if(R_ECX)
                    R_EIP += tmp8s;
                break;
            case 0xE3:                      /* JECXZ */
                tmp8s = Fetch8s(emu);
                if(!R_ECX)
                    R_EIP += tmp8s;
                break;

            case 0xE8:                      /* CALL Id */
                tmp32s = Fetch32s(emu); // call is relative
                Push(emu, R_EIP);
                R_EIP += tmp32s;
                break;
            case 0xE9:                      /* JMP Id */
                tmp32s = Fetch32s(emu); // jmp is relative
                R_EIP += tmp32s;
                break;

            case 0xEB:                      /* JMP Ib */
                tmp32s = Fetch8s(emu); // jump is relative
                R_EIP += tmp32s;
                break;
            case 0xF0:                      /* LOCK */
                break;

            case 0xF2:                      /* REPNZ prefix */
            case 0xF3:                      /* REPZ prefix */
                nextop = Fetch8(emu);
                tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
                tmp32u = R_ECX;
                switch(nextop) {
                    case 0x0F:
                        if(opcode==0xF3)
                            RunF30F(emu);   // defined is run660f.c
                        else
                            RunF20F(emu);
                        break;
                    case 0xC3:              /* REPZ RET... yup */
                        R_EIP = Pop(emu);
                        break;
                    case 0xA4:              /* REP MOVSB */
                        while(tmp32u) {
                            --tmp32u;
                            *(uint8_t*)R_EDI = *(uint8_t*)R_ESI;
                            R_EDI += tmp8s;
                            R_ESI += tmp8s;
                        }
                        break;
                    case 0xA5:              /* REP MOVSD */
                        tmp8s *= 4;
                        while(tmp32u) {
                            --tmp32u;
                            *(uint32_t*)R_EDI = *(uint32_t*)R_ESI;
                            R_EDI += tmp8s;
                            R_ESI += tmp8s;
                        }
                        break;
                    case 0xA6:              /* REP(N)Z CMPSB */
                        if(opcode==0xF2) {
                            while(tmp32u) {
                                --tmp32u;
                                tmp8u  = *(uint8_t*)R_EDI;
                                tmp8u2 = *(uint8_t*)R_ESI;
                                R_EDI += tmp8s;
                                R_ESI += tmp8s;
                                if(tmp8u==tmp8u2)
                                    break;
                            }
                        } else {
                            while(tmp32u) {
                                --tmp32u;
                                tmp8u  = *(uint8_t*)R_EDI;
                                tmp8u2 = *(uint8_t*)R_ESI;
                                R_EDI += tmp8s;
                                R_ESI += tmp8s;
                                if(tmp8u!=tmp8u2)
                                    break;
                            }
                        }
                        cmp8(emu, tmp8u2, tmp8u);
                        break;
                    case 0xA7:              /* REP(N)Z CMPSD */
                        tmp8s *= 4;
                        if(opcode==0xF2) {
                            while(tmp32u) {
                                --tmp32u;
                                tmp32u3 = *(uint32_t*)R_EDI;
                                tmp32u2 = *(uint32_t*)R_ESI;
                                R_EDI += tmp8s;
                                R_ESI += tmp8s;
                                if(tmp32u3==tmp32u2)
                                    break;
                            }
                        } else {
                            while(tmp32u) {
                                --tmp32u;
                                tmp32u3 = *(uint32_t*)R_EDI;
                                tmp32u2 = *(uint32_t*)R_ESI;
                                R_EDI += tmp8s;
                                R_ESI += tmp8s;
                                if(tmp32u3!=tmp32u2)
                                    break;
                            }
                        }
                        cmp32(emu, tmp32u2, tmp32u3);
                        break;
                    case 0xAA:              /* REP STOSB */
                        while(tmp32u) {
                            --tmp32u;
                            *(uint8_t*)R_EDI = R_AL;
                            R_EDI += tmp8s;
                        }
                        break;
                    case 0xAB:              /* REP STOSD */
                        tmp8s *= 4;
                        while(tmp32u) {
                            --tmp32u;
                            *(uint32_t*)R_EDI = R_EAX;
                            R_EDI += tmp8s;
                        }
                        break;
                    case 0xAC:              /* REP LODSB */
                        while(tmp32u) {
                            --tmp32u;
                            R_AL = *(uint8_t*)R_ESI;
                            R_ESI += tmp8s;
                        }
                        break;
                    case 0xAD:              /* REP LODSD */
                        tmp8s *= 4;
                        while(tmp32u) {
                            --tmp32u;
                            R_EAX = *(uint32_t*)R_ESI;
                            R_ESI += tmp8s;
                        }
                        break;
                    case 0xAE:              /* REP(N)Z SCASB */
                        if(opcode==0xF2) {
                            while(tmp32u) {
                                --tmp32u;
                                tmp8u = *(uint8_t*)R_EDI;
                                R_EDI += tmp8s;
                                if(R_AL==tmp8u)
                                    break;
                            }
                        } else {
                            while(tmp32u) {
                                --tmp32u;
                                tmp8u = *(uint8_t*)R_EDI;
                                R_EDI += tmp8s;
                                if(R_AL!=tmp8u)
                                    break;
                            }
                        }
                        cmp8(emu, R_AL, tmp8u);
                        break;
                    case 0xAF:              /* REP(N)Z SCASD */
                        tmp8s *= 4;
                        if(opcode==0xF2) {
                            while(tmp32u) {
                                --tmp32u;
                                tmp32u2 = *(uint32_t*)R_EDI;
                                R_EDI += tmp8s;
                                if(R_EAX==tmp32u2)
                                    break;
                            }
                        } else {
                            while(tmp32u) {
                                --tmp32u;
                                tmp32u2 = *(uint32_t*)R_EDI;
                                R_EDI += tmp8s;
                                if(R_EAX!=tmp32u2)
                                    break;
                            }
                        }
                        cmp32(emu, R_EAX, tmp32u2);
                        break;
                    default:
                        UnimpOpcode(emu);
                }
                R_ECX = tmp32u;
                break;
            
            case 0xF6:                      /* GRP3 Eb(,Ib) */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, nextop);
                switch((nextop>>3)&7) {
                    case 0: 
                    case 1:                 /* TEST Eb,Ib */
                        test8(emu, op1->byte[0], Fetch8(emu));
                        break;
                    case 2:                 /* NOT Eb */
                        op1->byte[0] = not8(emu, op1->byte[0]);
                        break;
                    case 3:                 /* NEG Eb */
                        op1->byte[0] = neg8(emu, op1->byte[0]);
                        break;
                    case 4:                 /* MUL EAX,Eb */
                        mul8(emu, op1->byte[0]);
                        break;
                    case 5:                 /* IMUL EAX,Eb */
                        imul8(emu, op1->byte[0]);
                        break;
                    case 6:                 /* DIV Eb */
                        div8(emu, op1->byte[0]);
                        break;
                    case 7:                 /* IDIV Eb */
                        idiv8(emu, op1->byte[0]);
                        break;
                }
                break;
            case 0xF7:                      /* GRP3 Ed(,Id) */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, nextop);
                switch((nextop>>3)&7) {
                    case 0: 
                    case 1:                 /* TEST Ed,Id */
                        test32(emu, op1->dword[0], Fetch32(emu));
                        break;
                    case 2:                 /* NOT Ed */
                        op1->dword[0] = not32(emu, op1->dword[0]);
                        break;
                    case 3:                 /* NEG Ed */
                        op1->dword[0] = neg32(emu, op1->dword[0]);
                        break;
                    case 4:                 /* MUL EAX,Ed */
                        mul32_eax(emu, op1->dword[0]);
                        break;
                    case 5:                 /* IMUL EAX,Ed */
                        imul32_eax(emu, op1->dword[0]);
                        break;
                    case 6:                 /* DIV Ed */
                        div32(emu, op1->dword[0]);
                        break;
                    case 7:                 /* IDIV Ed */
                        idiv32(emu, op1->dword[0]);
                        break;
                }
                break;

            case 0xF8:                      /* CLC */
                CLEAR_FLAG(F_CF);
                break;
            case 0xF9:                      /* STC */
                SET_FLAG(F_CF);
                break;

            case 0xFC:                      /* CLD */
                CLEAR_FLAG(F_DF);
                break;
            case 0xFD:                      /* STD */
                SET_FLAG(F_DF);
                break;
            case 0xFE:                      /* GRP 5 Eb */
                nextop = Fetch8(emu);
                GetEb(emu, &op1, nextop);
                switch((nextop>>3)&7) {
                    case 0:                 /* INC Eb */
                        op1->byte[0] = inc8(emu, op1->byte[0]);
                        break;
                    case 1:                 /* DEC Ed */
                        op1->byte[0] = dec8(emu, op1->byte[0]);
                        break;
                    default:
                        printf_log(LOG_NONE, "Illegal Opcode %02X %02X\n", opcode, nextop);
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                }
                break;
            case 0xFF:                      /* GRP 5 Ed */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, nextop);
                switch((nextop>>3)&7) {
                    case 0:                 /* INC Ed */
                        op1->dword[0] = inc32(emu, op1->dword[0]);
                        break;
                    case 1:                 /* DEC Ed */
                        op1->dword[0] = dec32(emu, op1->dword[0]);
                        break;
                    case 2:                 /* CALL NEAR Ed */
                        Push(emu, R_EIP);
                        R_EIP = op1->dword[0];
                        break;
                    case 3:                 /* CALL FAR Ed */
                        if(nextop>0xc0) {
                            printf_log(LOG_NONE, "Illegal Opcode %02X %02X\n", opcode, nextop);
                            emu->quit=1;
                            emu->error |= ERR_ILLEGAL;
                        } else {
                            Push16(emu, R_CS);
                            Push(emu, R_EIP);
                            R_EIP = op1->dword[0];
                            R_CS = (op1+1)->word[0];
                        }
                        break;
                    case 4:                 /* JMP NEAR Ed */
                        R_EIP = op1->dword[0];
                        break;
                    case 5:                 /* JMP FAR Ed */
                        if(nextop>0xc0) {
                            printf_log(LOG_NONE, "Illegal Opcode 0x%02X 0x%02X\n", opcode, nextop);
                            emu->quit=1;
                            emu->error |= ERR_ILLEGAL;
                        } else {
                            R_EIP = op1->dword[0];
                            R_CS = (op1+1)->word[0];
                        }
                        break;
                    case 6:                 /* Push Ed */
                        Push(emu, op1->dword[0]);
                        break;
                    default:
                        printf_log(LOG_NONE, "Illegal Opcode 0x%02X 0x%02X\n", opcode, nextop);
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                }
                break;

            default:
                UnimpOpcode(emu);
        }
    }
    if(emu->fork) {
        emu->fork = 0;
        emu = x86emu_fork(emu);
        goto x86emurun;
    }
}
