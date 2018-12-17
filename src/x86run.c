#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86emu_private.h"
#include "x86primop.h"
#include "x86trace.h"

uint8_t Fetch8(x86emu_t *emu)
{
    uint8_t val = *(uint8_t*)R_EIP;
    R_EIP++;
    return val;
}
int8_t Fetch8s(x86emu_t *emu)
{
    int8_t val = *(int8_t*)R_EIP;
    R_EIP++;
    return val;
}
uint16_t Fetch16(x86emu_t *emu)
{
    uint16_t val = *(uint16_t*)R_EIP;
    R_EIP+=2;
    return val;
}
uint32_t Fetch32(x86emu_t *emu)
{
    uint32_t val = *(uint32_t*)R_EIP;
    R_EIP+=4;
    return val;
}
int32_t Fetch32s(x86emu_t *emu)
{
    int32_t val = *(int32_t*)R_EIP;
    R_EIP+=4;
    return val;
}
// the op code definition can be found here: http://ref.x86asm.net/geek32.html

void GetEb(x86emu_t *emu, reg32_t **op, reg32_t *ea, uint32_t v)
{
    uint32_t m = v&0xC7;    // filter Eb
    if(m>=0xC0) {
        int lowhigh = (m&04)>>3;
         *op = (reg32_t *)&emu->regs[_AX+(m&0x03)].byte[lowhigh];  //?
        return;
    } else if (m<=7) {
        if(m==0x4) {
            uint8_t sib = Fetch8(emu);
            uintptr_t base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            if((sib&0x7)==5)
                base = Fetch32(emu);
            base += emu->sbiidx[(sib>>3)&7]->dword[0] << (sib>>6);
            *op = (reg32_t*)base;
            return;
        } else if (m==0x5) { //disp32
            *op = ea;
            ea->dword[0] = Fetch32(emu);
            return;
        }
        *op = &emu->regs[_AX+m];
        return;
    } else if(m>=0x40 && m<=0x47) {
        uintptr_t base;
        if(m==0x44) {
            uint8_t sib = Fetch8(emu);
            base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            if((sib&0x7)==5)
                base = Fetch32(emu);
            uint32_t idx = emu->sbiidx[(sib>>3)&7]->dword[0];
            /*if(((v>>3)&7)==4)
                idx += Fetch8(emu);*/
            base += idx << (sib>>6);
        } else {
            base = emu->regs[_AX+(m&0x7)].dword[0];
        }
        base+=Fetch8(emu);
        *op = (reg32_t*)base;
        return;
    } else if(m>=0x80 && m<0x87) {
        uintptr_t base;
        if(m==0x84) {
            uint8_t sib = Fetch32(emu);
            base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            if((sib&0x7)==5)
                base = Fetch32(emu);
            uint32_t idx = emu->sbiidx[(sib>>3)&7]->dword[0];
            /*if(((v>>3)&7)==4)
                idx += Fetch32(emu);*/
            base += idx << (sib>>6);
        } else {
            base = emu->regs[_AX+(m&0x7)].dword[0];
        }
        base+=Fetch32(emu);
        *op = (reg32_t*)base;
        return;
    } else {
        ea->word[0] = 0;
        *op = ea;
        return;
    }
}

void GetEd(x86emu_t *emu, reg32_t **op, reg32_t *ea, uint32_t v)
{
    uint32_t m = v&0xC7;    // filter Ed
    if(m>=0xC0) {
         *op = &emu->regs[_AX+(m&0x07)];
        return;
    } else if (m<=7) {
        if(m==0x4) {
            uint8_t sib = Fetch8(emu);
            uintptr_t base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            if((sib&0x7)==5)
                base = Fetch32(emu);
            base += emu->sbiidx[(sib>>3)&7]->dword[0] << (sib>>6);
            *op = (reg32_t*)base;
            return;
        } else if (m==0x5) { //disp32
            *op = ea;
            ea->dword[0] = Fetch32(emu);
            return;
        }
        *op = &emu->regs[_AX+m];
        return;
    } else if(m>=0x40 && m<=0x47) {
        uintptr_t base;
        if(m==0x44) {
            uint8_t sib = Fetch8(emu);
            base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            if((sib&0x7)==5)
                base = Fetch32(emu);
            uint32_t idx = emu->sbiidx[(sib>>3)&7]->dword[0];
            /*if(((sib>>3)&7)==4)
                idx += Fetch8(emu);*/
            base += idx << (sib>>6);
        } else {
            base = emu->regs[_AX+(m&0x7)].dword[0];
        }
        base+=Fetch8s(emu);
        *op = (reg32_t*)base;
        return;
    } else if(m>=0x80 && m<0x87) {
        uintptr_t base;
        if(m==0x84) {
            uint8_t sib = Fetch32(emu);
            base = emu->regs[_AX+(sib&0x7)].dword[0]; // base
            if((sib&0x7)==5)
                base = Fetch32(emu);
            uint32_t idx = emu->sbiidx[(sib>>3)&7]->dword[0];
            /*if(((sib>>3)&7)==4)
                idx += Fetch32(emu);*/
            base += idx << (sib>>6);
        } else {
            base = emu->regs[_AX+(m&0x7)].dword[0];
        }
        base+=Fetch32s(emu);
        *op = (reg32_t*)base;
        return;
    } else {
        ea->word[0] = 0;
        *op = ea;
        return;
    }
}

void GetG(x86emu_t *emu, reg32_t **op, uint32_t v)
{
    *op = &emu->regs[_AX+((v&0x38)>>3)];
}

int Run(x86emu_t *emu)
{
    emu->quit = 0;
    printf("Starting\nCPU Regs: %s\n", DumpCPURegs(emu));
    while (!emu->quit)
    {
        if(emu->dec) {
            printf_debug(DEBUG_NONE, "%08p: %s\n", R_EIP, DecodeX86Trace(emu->dec, R_EIP));
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
                GetEb(emu, &op1, &ea2, nextop);
                GetG(emu, &op2, nextop);
                op2->byte[0] = add8(emu, op1->byte[0], op2->byte[1]);
                break;
            case 0x04: /* ADD AL, Ib */
                tmp8u = Fetch8(emu);
                R_AL = add8(emu, R_AL, tmp8u);
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
                tmp8u = opcode&7;
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
                tmp8u = opcode&7;
                emu->regs[tmp8u].dword[0] = Pop(emu);
                break;

            case 0x83:  /* Grpl Ed,Ix */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea2, nextop);
                tmp32s = Fetch8s(emu);
                switch((nextop>>3)&7) {
                    case 0: op1->dword[0] = add32(emu, op1->dword[0], tmp32s); break;
                    case 1: op1->dword[0] = or32(emu, op1->dword[0], tmp32s); break;
                    case 2: op1->dword[0] = adc32(emu, op1->dword[0], tmp32s); break;
                    case 3: op1->dword[0] = sbb32(emu, op1->dword[0], tmp32s); break;
                    case 4: op1->dword[0] = and32(emu, op1->dword[0], tmp32s); break;
                    case 5: op1->dword[0] = sub32(emu, op1->dword[0], tmp32s); break;
                    case 6: op1->dword[0] = xor32(emu, op1->dword[0], tmp32s); break;
                    case 7: op1->dword[0] = cmp32(emu, op1->dword[0], tmp32s); break;
                }
                break;

            case 0x65:  /* GS: */
                // set a new decoder...
                opcode = Fetch8(emu);
                switch(opcode) {
                    case 0xA1: /* MOV EAX, Ov */
                        tmp32u = Fetch32(emu);
                        R_EAX = *(uint32_t*)(((uintptr_t)emu->globals) + tmp32u);
                        break;

                    default:
                        printf("Unimplemented Opcode 0xFF 0x%02X\n", opcode);
                        emu->quit=1;
                }
                break;
            
            case 0x87:  /* XCHG Ed,Gd */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea2, nextop);
                GetG(emu, &op2, nextop);
                tmp32u = op1->dword[0];
                op1->dword[0] = op2->dword[0];
                op2->dword[0] = tmp32u;
                break;

            case 0x89:  /* MOV Ed, Gv */
                nextop = Fetch8(emu);
                GetEd(emu, &op1, &ea2, nextop);
                GetG(emu, &op2, nextop);
                op1->dword[0] = op2->dword[0];
                break;

            case 0x8B: /* MOV Gv, Ed */
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

            case 0xA1: /* MOV EAX, Ov */
                tmp32u = Fetch32(emu);
                R_EAX = *(uint32_t*)tmp32u;
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
                            printf("Illegal Opcode 0x%02X 0x%02X\n", opcode, nextop);
                            emu->quit=1;
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
                            printf("Illegal Opcode 0x%02X 0x%02X\n", opcode, nextop);
                            emu->quit=1;
                        } else {
                            R_EIP = op1->dword[0];
                            R_CS = (op1+1)->word[0];
                        }
                        break;
                    case 6: /* Push Ed */
                        Push(emu, op1->dword[0]);
                        break;
                    default:
                        printf("Illegal Opcode 0x%02X 0x%02X\n", opcode, nextop);
                        emu->quit=1;
                }
                break;

            default:
                printf("Unimplemented Opcode 0x%02X\n", opcode);
                emu->quit=1;
        }
        printf("CPU Regs: %s\n", DumpCPURegs(emu));


    }
}