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
    uint16_t tmp16u, tmp16u2;
    int16_t tmp16s;
    uint32_t tmp32u;
    int32_t tmp32s;
    uint64_t tmp64u;
    int64_t tmp64s;
    switch(opcode) {

    #define GO(B, OP)                       \
    case B+1:                               \
        nextop = Fetch8(emu);               \
        GetEw(emu, &op1, &ea1, nextop);     \
        GetG(emu, &op2, nextop);            \
        op1->word[0] = OP##16(emu, op1->word[0], op2->word[0]); \
        break;                              \
    case B+3:                               \
        nextop = Fetch8(emu);               \
        GetEw(emu, &op2, &ea2, nextop);     \
        GetG(emu, &op1, nextop);            \
        op1->word[0] = OP##16(emu, op1->word[0], op2->word[0]); \
        break;                              \
    case B+5:                               \
        R_AX = OP##16(emu, R_AX, Fetch16(emu)); \
        break;

    GO(0x00, add)                   /* ADD 0x01 ~> 0x05 */
    GO(0x08, or)                    /*  OR 0x09 ~> 0x0D */
    GO(0x10, adc)                   /* ADC 0x11 ~> 0x15 */
    GO(0x18, sbb)                   /* SBB 0x19 ~> 0x1D */
    GO(0x20, and)                   /* AND 0x21 ~> 0x25 */
    GO(0x28, sub)                   /* SUB 0x29 ~> 0x2D */
    GO(0x30, xor)                   /* XOR 0x31 ~> 0x35 */
    //GO(0x38, cmp)                   /* CMP 0x39 ~> 0x3D */
    #undef GO
    case 0x39:
        nextop = Fetch8(emu);
        GetEw(emu, &op1, &ea1, nextop);
        GetG(emu, &op2, nextop);
        cmp16(emu, op1->word[0], op2->word[0]);
        break;
    case 0x3B:
        nextop = Fetch8(emu);
        GetEw(emu, &op2, &ea2, nextop);
        GetG(emu, &op1, nextop);
        cmp16(emu, op1->word[0], op2->word[0]);
        break;
    case 0x3D:
        cmp16(emu, R_AX, Fetch16(emu));
        break;
    

    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:                              /* INC Reg */
        tmp8u = opcode&7;
        emu->regs[tmp8u].word[0] = inc16(emu, emu->regs[tmp8u].word[0]);
        break;
    case 0x48:
    case 0x49:
    case 0x4A:
    case 0x4B:
    case 0x4C:
    case 0x4D:
    case 0x4E:
    case 0x4F:                              /* DEC Reg */
        tmp8u = opcode&7;
        emu->regs[tmp8u].word[0] = dec16(emu, emu->regs[tmp8u].word[0]);
        break;

    case 0x69:                      /* IMUL Gw,Ew,Iw */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, &ea1, nextop);
        GetG(emu, &op2, nextop);
        tmp16u = Fetch16(emu);
        op2->word[0] = imul16(emu, op1->word[0], tmp16u);
        break;

    case 0x6B:                      /* IMUL Gw,Ew,Ib */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, &ea1, nextop);
        GetG(emu, &op2, nextop);
        tmp16s = Fetch8s(emu);
        op2->word[0] = imul16(emu, op1->word[0], (uint16_t)tmp16s);
        break;

    case 0x81:                              /* GRP3 Ew,Iw */
    case 0x83:                              /* GRP3 Ew,Ib */
        nextop = Fetch8(emu);
        GetEd(emu, &op1, &ea1, nextop);
        if(opcode==0x81) 
            tmp16u = Fetch16(emu);
        else {
            tmp16s = Fetch8s(emu);
            tmp16u = *(uint16_t*)&tmp16s;
        }
        switch((nextop>>3)&7) {
            case 0: op1->word[0] = add16(emu, op1->word[0], tmp16u); break;
            case 1: op1->word[0] =  or16(emu, op1->word[0], tmp16u); break;
            case 2: op1->word[0] = adc16(emu, op1->word[0], tmp16u); break;
            case 3: op1->word[0] = sbb16(emu, op1->word[0], tmp16u); break;
            case 4: op1->word[0] = and16(emu, op1->word[0], tmp16u); break;
            case 5: op1->word[0] = sub16(emu, op1->word[0], tmp16u); break;
            case 6: op1->word[0] = xor16(emu, op1->word[0], tmp16u); break;
            case 7:                cmp16(emu, op1->word[0], tmp16u); break;
        }
        break;

    case 0x85:                              /* TEST Ew,Gw */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, &ea1, nextop);
        GetG(emu, &op2, nextop);
        test16(emu, op1->word[0], op2->word[0]);
        break;

    case 0x87:                              /* XCHG Ew,Gw */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, &ea2, nextop);
        GetG(emu, &op2, nextop);
        tmp16u = op1->word[0];
        op1->word[0] = op2->word[0];
        op2->word[0] = tmp16u;
        break;

    case 0x89:                              /* MOV Ew,Gw */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, &ea2, nextop);
        GetG(emu, &op2, nextop);
        op1->word[0] = op2->word[0];
        break;

    case 0x8B:                              /* MOV Gw,Ew */
        nextop = Fetch8(emu);
        GetEw(emu, &op2, &ea2, nextop);
        GetG(emu, &op1, nextop);
        op1->word[0] = op2->word[0];
        break;
    
    case 0x90:                              /* NOP */
        break;

    case 0x92:                              /* XCHG DX,AX */
        tmp16u = R_AX;
        R_AX = R_DX;
        R_DX = tmp16u;
        break;

    case 0xA1:                              /* MOV AX,Ow */
        R_AX = *(uint16_t*)Fetch32(emu);
        break;
    case 0xA3:                              /* MOV Od,AX */
        *(uint16_t*)Fetch32(emu) = R_AX;
        break;
    case 0xA5:                              /* MOVSW */
        tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
        *(uint16_t*)R_EDI = *(uint16_t*)R_ESI;
        R_EDI += tmp8s;
        R_ESI += tmp8s;
        break;

    case 0xA7:                              /* CMPSW */
        tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
        tmp16u  = *(uint16_t*)R_EDI;
        tmp16u2 = *(uint16_t*)R_ESI;
        R_EDI += tmp8s;
        R_ESI += tmp8s;
        cmp16(emu, tmp16u2, tmp16u);
        break;
    case 0xAB:                              /* STOSW */
        tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
        *(uint16_t*)R_EDI = R_AX;
        R_EDI += tmp8s;
        break;
    case 0xAD:                              /* LODSW */
        tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
        R_AX = *(uint16_t*)R_ESI;
        R_ESI += tmp8s;
        break;
    case 0xAF:                              /* SCASW */
        tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
        cmp16(emu, R_AX, *(uint16_t*)R_EDI);
        R_EDI += tmp8s;
        break;

    case 0xB8:                              /* MOV AX,Iw */
    case 0xB9:                              /* MOV CX,Iw */
    case 0xBA:                              /* MOV DX,Iw */
    case 0xBB:                              /* MOV BX,Iw */
    case 0xBC:                              /*    ...     */
    case 0xBD:
    case 0xBE:
    case 0xBF:
        emu->regs[opcode-0xB8].word[0] = Fetch16(emu);
        break;

    case 0xC1:                              /* GRP2 Ew,Ib */
        nextop = Fetch8(emu);
        GetEd(emu, &op1, &ea2, nextop);
        tmp8u = Fetch8(emu) & 0x1f;
        switch((nextop>>3)&7) {
            case 0: op1->word[0] = rol16(emu, op1->word[0], tmp8u); break;
            case 1: op1->word[0] = ror16(emu, op1->word[0], tmp8u); break;
            case 2: op1->word[0] = rcl16(emu, op1->word[0], tmp8u); break;
            case 3: op1->word[0] = rcr16(emu, op1->word[0], tmp8u); break;
            case 4:
            case 6: op1->word[0] = shl16(emu, op1->word[0], tmp8u); break;
            case 5: op1->word[0] = shr16(emu, op1->word[0], tmp8u); break;
            case 7: op1->word[0] = sar16(emu, op1->word[0], tmp8u); break;
        }
        break;

    case 0xC7:                              /* MOV Ew,Iw */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, &ea2, nextop);
        op1->word[0] = Fetch16(emu);
        break;

    case 0xD1:                              /* GRP2 Ew,1  */
    case 0xD3:                              /* GRP2 Ew,CL */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, &ea2, nextop);
        tmp8u=(opcode==0xD3)?R_CL:1;
        switch((nextop>>3)&7) {
            case 0: op1->word[0] = rol16(emu, op1->word[0], tmp8u); break;
            case 1: op1->word[0] = ror16(emu, op1->word[0], tmp8u); break;
            case 2: op1->word[0] = rcl16(emu, op1->word[0], tmp8u); break;
            case 3: op1->word[0] = rcr16(emu, op1->word[0], tmp8u); break;
            case 4: 
            case 6: op1->word[0] = shl16(emu, op1->word[0], tmp8u); break;
            case 5: op1->word[0] = shr16(emu, op1->word[0], tmp8u); break;
            case 7: op1->word[0] = sar16(emu, op1->word[0], tmp8u); break;
        }
        break;

        case 0xF2:                      /* REPNZ prefix */
        case 0xF3:                      /* REPZ prefix */
            nextop = Fetch8(emu);
            tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
            tmp32u = R_ECX;
            switch(nextop) {
                case 0xA5:              /* REP MOVSW */
                    for(; tmp32u>0; --tmp32u) {
                        *(uint16_t*)R_EDI = *(uint16_t*)R_ESI;
                        R_EDI += tmp8s;
                        R_ESI += tmp8s;
                    }
                    break;
                case 0xA7:              /* REP(N)Z CMPSW */
                    while(tmp32u>0) {
                        --tmp32u;
                        tmp16u  = *(uint16_t*)R_EDI;
                        tmp16u2 = *(uint16_t*)R_ESI;
                        R_EDI += tmp8s;
                        R_ESI += tmp8s;
                        if((tmp16u==tmp16u2)==(opcode==0xF2))
                            break;
                    }
                    cmp16(emu, tmp16u2, tmp16u);
                    break;
                case 0xAB:              /* REP STOSW */
                    for(; tmp32u>0; --tmp32u) {
                        *(uint16_t*)R_EDI = R_AX;
                        R_EDI += tmp8s;
                    }
                    break;
                case 0xAD:              /* REP LODSW */
                    for(; tmp32u>0; --tmp32u) {
                        R_AX = *(uint16_t*)R_ESI;
                        R_ESI += tmp8s;
                    }
                    break;
                case 0xAF:              /* REP(N)Z SCASW */
                    while(tmp32u>0) {
                        --tmp32u;
                        tmp16u = *(uint16_t*)R_EDI;
                        R_EDI += tmp8s;
                        if((R_AX==tmp16u)==(opcode==0xF2))
                            break;
                    }
                    cmp16(emu, R_AX, tmp16u);
                    break;
                default:
                    UnimpOpcode(emu);
            }
            R_ECX = tmp32u;
            break;

    case 0xF7:                      /* GRP3 Ew(,Iw) */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, &ea1, nextop);
        switch((nextop>>3)&7) {
            case 0: 
            case 1:                 /* TEST Ed,Id */
                test16(emu, op1->word[0], Fetch16(emu));
                break;
            case 2:                 /* NOT Ed */
                op1->word[0] = not16(emu, op1->word[0]);
                break;
            case 3:                 /* NEG Ed */
                op1->word[0] = neg16(emu, op1->word[0]);
                break;
            case 4:                 /* MUL EAX,Ed */
                mul16(emu, op1->word[0]);
                break;
            case 5:                 /* IMUL EAX,Ed */
                imul16_eax(emu, op1->word[0]);
                break;
            case 6:                 /* DIV Ed */
                div16(emu, op1->word[0]);
                break;
            case 7:                 /* IDIV Ed */
                idiv16(emu, op1->word[0]);
                break;
        }
        break;

    case 0xFF:                      /* GRP 5 Ew */
        nextop = Fetch8(emu);
        GetEd(emu, &op1, &ea2, nextop);
        switch((nextop>>3)&7) {
            case 0:                 /* INC Ed */
                op1->word[0] = inc16(emu, op1->word[0]);
                break;
            case 1:                 /* DEC Ed */
                op1->word[0] = dec16(emu, op1->word[0]);
                break;
            default:
                printf_log(LOG_NONE, "Illegal Opcode 66 %02X %02X\n", opcode, nextop);
                emu->quit=1;
                emu->error |= ERR_ILLEGAL;
        }
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

    case 0xE0:                      /* LOOPNZ */
        tmp8s = Fetch8s(emu);
        --R_CX; // don't update flags
        if(R_CX && !ACCESS_FLAG(F_ZF))
            R_EIP += tmp8s;
        break;
    case 0xE1:                      /* LOOPZ */
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
