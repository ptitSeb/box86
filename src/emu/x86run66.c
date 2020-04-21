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


#define F8      *(uint8_t*)(R_EIP++)
#define F8S     *(int8_t*)(R_EIP++)
#define F16     *(uint16_t*)(R_EIP+=2, R_EIP-2)
#define F32     *(uint32_t*)(R_EIP+=4, R_EIP-4)
#define F32S    *(int32_t*)(R_EIP+=4, R_EIP-4)
#define PK(a)   *(uint8_t*)(R_EIP+a)

#include "modrm.h"

void Run6766(x86emu_t *emu)
{
    uint8_t opcode = Fetch8(emu);
    uint8_t nextop;
    reg32_t *op1, *op2;
    switch(opcode) {

    case 0x8D:                              /* LEA Gw,Ew */
        nextop = Fetch8(emu);
        op1=GetEw16(emu, nextop);
        op2=GetG(emu, nextop);
        op2->word[0] = (uint16_t)(uintptr_t)op1;
        break;
    
                
    default:
        UnimpOpcode(emu);
    }
}

void Run67(x86emu_t *emu)
{
    uint8_t opcode = Fetch8(emu);
    int8_t tmp8s;
    switch(opcode) {

    case 0x66:                      /* MoooRE opcodes */
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

void RunLock(x86emu_t *emu)
{
    uint8_t opcode = Fetch8(emu);
    uint8_t nextop;
    reg32_t *oped;
    uint8_t tmp8u;
    uint32_t tmp32u, tmp32u2;
    int32_t tmp32s;
    pthread_mutex_lock(&emu->context->mutex_lock);
    switch(opcode) {
        #define GO(B, OP)                      \
        case B+0: \
            nextop = F8;               \
            GET_EB;             \
            EB->byte[0] = OP##8(emu, EB->byte[0], GB);  \
            break;                              \
        case B+1: \
            nextop = F8;               \
            GET_ED;             \
            ED->dword[0] = OP##32(emu, ED->dword[0], GD.dword[0]); \
            break;                              \
        case B+2: \
            nextop = F8;               \
            GET_EB;                   \
            GB = OP##8(emu, GB, EB->byte[0]); \
            break;                              \
        case B+3: \
            nextop = F8;               \
            GET_ED;         \
            GD.dword[0] = OP##32(emu, GD.dword[0], ED->dword[0]); \
            break;                              \
        case B+4: \
            R_AL = OP##8(emu, R_AL, F8); \
            break;                              \
        case B+5: \
            R_EAX = OP##32(emu, R_EAX, F32); \
            break;

        GO(0x00, add)                   /* ADD 0x00 -> 0x05 */
        GO(0x08, or)                    /*  OR 0x08 -> 0x0D */
        GO(0x10, adc)                   /* ADC 0x10 -> 0x15 */
        GO(0x18, sbb)                   /* SBB 0x18 -> 0x1D */
        GO(0x20, and)                   /* AND 0x20 -> 0x25 */
        GO(0x28, sub)                   /* SUB 0x28 -> 0x2D */
        GO(0x30, xor)                   /* XOR 0x30 -> 0x35 */
        #undef GO

        case 0x0f:
            opcode = F8;
            switch (opcode) { 
                case 0xB0:                      /* CMPXCHG Eb,Gb */
                    CHECK_FLAGS(emu);
                    nextop = F8;
                    GET_EB;
                    cmp8(emu, R_AL, EB->byte[0]);
                    if(ACCESS_FLAG(F_ZF)) {
                        EB->byte[0] = GB;
                    } else {
                        R_AL = EB->byte[0];
                    }
                    break;
                case 0xB1:                      /* CMPXCHG Ed,Gd */
                    nextop = F8;
                    GET_ED;
                    cmp32(emu, R_EAX, ED->dword[0]);
                    if(ACCESS_FLAG(F_ZF)) {
                        ED->dword[0] = GD.dword[0];
                    } else {
                        R_EAX = ED->dword[0];
                    }
                    break;
                case 0xB3:                      /* BTR Ed,Gd */
                    CHECK_FLAGS(emu);
                    nextop = F8;
                    GET_ED;
                    tmp8u = GD.byte[0];
                    if((nextop&0xC0)!=0xC0)
                    {
                        ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
                    }
                    tmp8u&=31;
                    if(ED->dword[0] & (1<<tmp8u)) {
                        SET_FLAG(F_CF);
                        ED->dword[0] ^= (1<<tmp8u);
                    } else
                        CLEAR_FLAG(F_CF);
                    break;
                case 0xBA:                      
                    nextop = F8;
                    switch((nextop>>3)&7) {
                        case 4:                 /* BT Ed,Ib */
                            CHECK_FLAGS(emu);
                            GET_ED;
                            tmp8u = F8;
                            if((nextop&0xC0)!=0xC0)
                            {
                                ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
                            }
                            tmp8u&=31;
                            if(ED->dword[0] & (1<<tmp8u))
                                SET_FLAG(F_CF);
                            else
                                CLEAR_FLAG(F_CF);
                            break;
                        case 6:             /* BTR Ed, Ib */
                            CHECK_FLAGS(emu);
                            GET_ED;
                            tmp8u = F8;
                            if((nextop&0xC0)!=0xC0)
                            {
                                ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
                            }
                            tmp8u&=31;
                            if(ED->dword[0] & (1<<tmp8u)) {
                                SET_FLAG(F_CF);
                                ED->dword[0] ^= (1<<tmp8u);
                            } else
                                CLEAR_FLAG(F_CF);
                            break;

                        default:
                            R_EIP -= 3; //unfetch
                            break;
                    }
                    break;
                case 0xBB:                      /* BTC Ed,Gd */
                    CHECK_FLAGS(emu);
                    nextop = F8;
                    GET_ED;
                    tmp8u = GD.byte[0];
                    if((nextop&0xC0)!=0xC0)
                    {
                        ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
                    }
                    tmp8u&=31;
                    if(ED->dword[0] & (1<<tmp8u))
                        SET_FLAG(F_CF);
                    else
                        CLEAR_FLAG(F_CF);
                    ED->dword[0] ^= (1<<tmp8u);
                    break;
                case 0xC0:                      /* XADD Gb,Eb */
                    nextop = F8;
                    GET_EB;
                    tmp8u = add8(emu, EB->byte[0], GB);
                    GB = EB->byte[0];
                    EB->byte[0] = tmp8u;
                    break;
                case 0xC1:                      /* XADD Gd,Ed */
                    nextop = F8;
                    GET_ED;
                    tmp32u = add32(emu, ED->dword[0], GD.dword[0]);
                    GD.dword[0] = ED->dword[0];
                    ED->dword[0] = tmp32u;
                    break;
                case 0xC7:                      /* CMPXCHG8B Gq */
                    CHECK_FLAGS(emu);
                    nextop = F8;
                    GET_ED;
                    tmp32u = ED->dword[0];
                    tmp32u2= ED->dword[1];
                    if(R_EAX == tmp32u && R_EDX == tmp32u2) {
                        SET_FLAG(F_ZF);
                        ED->dword[0] = R_EBX;
                        ED->dword[1] = R_ECX;
                    } else {
                        CLEAR_FLAG(F_ZF);
                        R_EAX = tmp32u;
                        R_EDX = tmp32u2;
                    }
                    break;
                default:
                    // trigger invalid lock?
                    R_EIP -= 2; // unfetch
                    break;
            }
            break;

        case 0x81:              /* GRP Ed,Id */
        case 0x83:              /* GRP Ed,Ib */
            nextop = F8;
            GET_ED;
            if(opcode==0x83) {
                tmp32s = F8S;
                tmp32u = (uint32_t)tmp32s;
            } else
                tmp32u = F32;
            switch((nextop>>3)&7) {
                case 0: ED->dword[0] = add32(emu, ED->dword[0], tmp32u); break;
                case 1: ED->dword[0] =  or32(emu, ED->dword[0], tmp32u); break;
                case 2: ED->dword[0] = adc32(emu, ED->dword[0], tmp32u); break;
                case 3: ED->dword[0] = sbb32(emu, ED->dword[0], tmp32u); break;
                case 4: ED->dword[0] = and32(emu, ED->dword[0], tmp32u); break;
                case 5: ED->dword[0] = sub32(emu, ED->dword[0], tmp32u); break;
                case 6: ED->dword[0] = xor32(emu, ED->dword[0], tmp32u); break;
                case 7:                cmp32(emu, ED->dword[0], tmp32u); break;
            }
            break;
        case 0x86:                      /* XCHG Eb,Gb */
            nextop = F8;
            GET_EB;
            tmp8u = GB;
            GB = EB->byte[0];
            EB->byte[0] = tmp8u;
            break;
        case 0x87:                      /* XCHG Ed,Gd */
            nextop = F8;
            GET_ED;
            tmp32u = GD.dword[0];
            GD.dword[0] = ED->dword[0];
            ED->dword[0] = tmp32u;
            break;
        case 0xFF:              /* GRP 5 Ed */
            nextop = F8;
            GET_ED;
            switch((nextop>>3)&7) {
                case 0:                 /* INC Ed */
                    ED->dword[0] = inc32(emu, ED->dword[0]);
                    break;
                case 1:                 /* DEC Ed */
                    ED->dword[0] = dec32(emu, ED->dword[0]);
                    break;
                default:
                    printf_log(LOG_NONE, "Illegal Opcode 0xF0 0x%02X 0x%02X\n", opcode, PK(0));
                    emu->quit=1;
                    emu->error |= ERR_ILLEGAL;
                    break;
            }
            break;
        default:
            //UnimpOpcode(emu);
            // should trigger invalid unlock ?
            R_EIP--;    // "unfetch" to use normal instruction
    }
    pthread_mutex_unlock(&emu->context->mutex_lock);
}

void RunGS(x86emu_t *emu)
{
    uint8_t opcode = Fetch8(emu);
    uint8_t nextop;
    reg32_t *oped;
    uint8_t tmp8u;
    uint32_t tmp32u;
    int32_t tmp32s;
    uintptr_t tlsdata = GetGSBaseEmu(emu);
    switch(opcode) {
        case 0x33:              /* XOR Gd,Ed */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            GD.dword[0] = xor32(emu, GD.dword[0], ED->dword[0]);
            break;
        case 0x69:              /* IMUL Gd,Ed,Id */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            tmp32u = F32;
            GD.dword[0] = imul32(emu, ED->dword[0], tmp32u);
            break;
        case 0x80:             /* GRP Eb,Ib */
        case 0x82:             // 0x82 and 0x80 are the same opcodes it seems?
            nextop = F8;
            GET_EB_OFFS(tlsdata);
            tmp8u = F8;
            switch((nextop>>3)&7) {
                case 0: EB->byte[0] = add8(emu, EB->byte[0], tmp8u); break;
                case 1: EB->byte[0] =  or8(emu, EB->byte[0], tmp8u); break;
                case 2: EB->byte[0] = adc8(emu, EB->byte[0], tmp8u); break;
                case 3: EB->byte[0] = sbb8(emu, EB->byte[0], tmp8u); break;
                case 4: EB->byte[0] = and8(emu, EB->byte[0], tmp8u); break;
                case 5: EB->byte[0] = sub8(emu, EB->byte[0], tmp8u); break;
                case 6: EB->byte[0] = xor8(emu, EB->byte[0], tmp8u); break;
                case 7:               cmp8(emu, EB->byte[0], tmp8u); break;
            }
            break;
        case 0x81:              /* GRP Ed,Id */
        case 0x83:              /* GRP Ed,Ib */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            if(opcode==0x83) {
                tmp32s = F8S;
                tmp32u = (uint32_t)tmp32s;
            } else
                tmp32u = F32;
            switch((nextop>>3)&7) {
                case 0: ED->dword[0] = add32(emu, ED->dword[0], tmp32u); break;
                case 1: ED->dword[0] =  or32(emu, ED->dword[0], tmp32u); break;
                case 2: ED->dword[0] = adc32(emu, ED->dword[0], tmp32u); break;
                case 3: ED->dword[0] = sbb32(emu, ED->dword[0], tmp32u); break;
                case 4: ED->dword[0] = and32(emu, ED->dword[0], tmp32u); break;
                case 5: ED->dword[0] = sub32(emu, ED->dword[0], tmp32u); break;
                case 6: ED->dword[0] = xor32(emu, ED->dword[0], tmp32u); break;
                case 7:                cmp32(emu, ED->dword[0], tmp32u); break;
            }
            break;
        case 0x89:              /* MOV Ed,Gd */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            ED->dword[0] = GD.dword[0];
            break;
        case 0x8B:              /* MOV Gd,Ed */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            GD.dword[0] = ED->dword[0];
            break;
        case 0xA1:              /* MOV EAX,Ov */
            tmp32s = F32S;
            R_EAX = *(uint32_t*)((tlsdata) + tmp32s);
            break;
        case 0xA2:              /* MOV Ob,AL */
            tmp32s = F32S;
            *(uint8_t*)((tlsdata) + tmp32s) = R_AL;
            break;
        case 0xA3:             /* MOV Od,EAX */
            tmp32s = F32S;
            *(uint32_t*)((tlsdata) + tmp32s) = R_EAX;
            break;

        case 0xC0:             /* GRP2 Eb,Ib */
            nextop = F8;
            GET_EB_OFFS(tlsdata);
            tmp8u = F8/* & 0x1f*/; // masking done in each functions
            switch((nextop>>3)&7) {
                case 0: EB->byte[0] = rol8(emu, EB->byte[0], tmp8u); break;
                case 1: EB->byte[0] = ror8(emu, EB->byte[0], tmp8u); break;
                case 2: EB->byte[0] = rcl8(emu, EB->byte[0], tmp8u); break;
                case 3: EB->byte[0] = rcr8(emu, EB->byte[0], tmp8u); break;
                case 4:
                case 6: EB->byte[0] = shl8(emu, EB->byte[0], tmp8u); break;
                case 5: EB->byte[0] = shr8(emu, EB->byte[0], tmp8u); break;
                case 7: EB->byte[0] = sar8(emu, EB->byte[0], tmp8u); break;
            }
            break;

        case 0xC6:              /* MOV Eb,Ib */
            nextop = F8;
            GET_EB_OFFS(tlsdata);
            EB->byte[0] = F8;
            break;
        case 0xC7:              /* MOV Ed,Id */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            ED->dword[0] = F32;
            break;

        case 0xFF:              /* GRP 5 Ed */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            switch((nextop>>3)&7) {
                case 0:                 /* INC Ed */
                    ED->dword[0] = inc32(emu, ED->dword[0]);
                    break;
                case 1:                 /* DEC Ed */
                    ED->dword[0] = dec32(emu, ED->dword[0]);
                    break;
                case 2:                 /* CALL NEAR Ed */
                    Push(emu, R_EIP);
                    R_EIP = ED->dword[0];  // should get value in temp var. in case ED use ESP?
                    break;
                case 3:                 /* CALL FAR Ed */
                    if(nextop>0xc0) {
                        printf_log(LOG_NONE, "Illegal Opcode %02X %02X\n", opcode, nextop);
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                    } else {
                        Push16(emu, R_CS);
                        Push(emu, R_EIP);
                        R_EIP = ED->dword[0];
                        R_CS = (ED+1)->word[0];
                    }
                    break;
                case 4:                 /* JMP NEAR Ed */
                    R_EIP = ED->dword[0];
                    break;
                case 5:                 /* JMP FAR Ed */
                    if(nextop>0xc0) {
                        printf_log(LOG_NONE, "Illegal Opcode 0x%02X 0x%02X\n", opcode, nextop);
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                    } else {
                        R_EIP = ED->dword[0];
                        R_CS = (ED+1)->word[0];
                    }
                    break;
                case 6:                 /* Push Ed */
                    Push(emu, ED->dword[0]);
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

void RunFS(x86emu_t *emu)
{
    uint8_t opcode = Fetch8(emu);
    uint8_t nextop;
    reg32_t *oped;
    uint8_t tmp8u;
    uint32_t tmp32u;
    int32_t tmp32s;
    uintptr_t tlsdata = 0;
    if(emu->segs[_FS]==0x33)
        tlsdata = GetGSBaseEmu(emu);
    else {
        printf_log(LOG_INFO, "Warning, using FS: with FS=0x%x unsupported\n", emu->segs[_FS]);
    }
    switch(opcode) {
        case 0x33:              /* XOR Gd,Ed */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            GD.dword[0] = xor32(emu, GD.dword[0], ED->dword[0]);
            break;
        case 0x69:              /* IMUL Gd,Ed,Id */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            tmp32u = F32;
            GD.dword[0] = imul32(emu, ED->dword[0], tmp32u);
            break;
        case 0x80:             /* GRP Eb,Ib */
        case 0x82:             // 0x82 and 0x80 are the same opcodes it seems?
            nextop = F8;
            GET_EB_OFFS(tlsdata);
            tmp8u = F8;
            switch((nextop>>3)&7) {
                case 0: EB->byte[0] = add8(emu, EB->byte[0], tmp8u); break;
                case 1: EB->byte[0] =  or8(emu, EB->byte[0], tmp8u); break;
                case 2: EB->byte[0] = adc8(emu, EB->byte[0], tmp8u); break;
                case 3: EB->byte[0] = sbb8(emu, EB->byte[0], tmp8u); break;
                case 4: EB->byte[0] = and8(emu, EB->byte[0], tmp8u); break;
                case 5: EB->byte[0] = sub8(emu, EB->byte[0], tmp8u); break;
                case 6: EB->byte[0] = xor8(emu, EB->byte[0], tmp8u); break;
                case 7:               cmp8(emu, EB->byte[0], tmp8u); break;
            }
            break;
        case 0x81:              /* GRP Ed,Id */
        case 0x83:              /* GRP Ed,Ib */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            if(opcode==0x83) {
                tmp32s = F8S;
                tmp32u = (uint32_t)tmp32s;
            } else
                tmp32u = F32;
            switch((nextop>>3)&7) {
                case 0: ED->dword[0] = add32(emu, ED->dword[0], tmp32u); break;
                case 1: ED->dword[0] =  or32(emu, ED->dword[0], tmp32u); break;
                case 2: ED->dword[0] = adc32(emu, ED->dword[0], tmp32u); break;
                case 3: ED->dword[0] = sbb32(emu, ED->dword[0], tmp32u); break;
                case 4: ED->dword[0] = and32(emu, ED->dword[0], tmp32u); break;
                case 5: ED->dword[0] = sub32(emu, ED->dword[0], tmp32u); break;
                case 6: ED->dword[0] = xor32(emu, ED->dword[0], tmp32u); break;
                case 7:                cmp32(emu, ED->dword[0], tmp32u); break;
            }
            break;
        case 0x89:              /* MOV Ed,Gd */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            ED->dword[0] = GD.dword[0];
            break;
        case 0x8B:              /* MOV Gd,Ed */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            GD.dword[0] = ED->dword[0];
            break;
        case 0xA1:              /* MOV EAX,Ov */
            tmp32s = F32S;
            R_EAX = *(uint32_t*)((tlsdata) + tmp32s);
            break;
        case 0xA2:              /* MOV Ob,AL */
            tmp32s = F32S;
            *(uint8_t*)((tlsdata) + tmp32s) = R_AL;
            break;
        case 0xA3:             /* MOV Od,EAX */
            tmp32s = F32S;
            *(uint32_t*)((tlsdata) + tmp32s) = R_EAX;
            break;

        case 0xC0:             /* GRP2 Eb,Ib */
            nextop = F8;
            GET_EB_OFFS(tlsdata);
            tmp8u = F8/* & 0x1f*/; // masking done in each functions
            switch((nextop>>3)&7) {
                case 0: EB->byte[0] = rol8(emu, EB->byte[0], tmp8u); break;
                case 1: EB->byte[0] = ror8(emu, EB->byte[0], tmp8u); break;
                case 2: EB->byte[0] = rcl8(emu, EB->byte[0], tmp8u); break;
                case 3: EB->byte[0] = rcr8(emu, EB->byte[0], tmp8u); break;
                case 4:
                case 6: EB->byte[0] = shl8(emu, EB->byte[0], tmp8u); break;
                case 5: EB->byte[0] = shr8(emu, EB->byte[0], tmp8u); break;
                case 7: EB->byte[0] = sar8(emu, EB->byte[0], tmp8u); break;
            }
            break;

        case 0xC6:              /* MOV Eb,Ib */
            nextop = F8;
            GET_EB_OFFS(tlsdata);
            EB->byte[0] = F8;
            break;
        case 0xC7:              /* MOV Ed,Id */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            ED->dword[0] = F32;
            break;

        case 0xFF:              /* GRP 5 Ed */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            switch((nextop>>3)&7) {
                case 0:                 /* INC Ed */
                    ED->dword[0] = inc32(emu, ED->dword[0]);
                    break;
                case 1:                 /* DEC Ed */
                    ED->dword[0] = dec32(emu, ED->dword[0]);
                    break;
                case 2:                 /* CALL NEAR Ed */
                    Push(emu, R_EIP);
                    R_EIP = ED->dword[0];  // should get value in temp var. in case ED use ESP?
                    break;
                case 3:                 /* CALL FAR Ed */
                    if(nextop>0xc0) {
                        printf_log(LOG_NONE, "Illegal Opcode %02X %02X\n", opcode, nextop);
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                    } else {
                        Push16(emu, R_CS);
                        Push(emu, R_EIP);
                        R_EIP = ED->dword[0];
                        R_CS = (ED+1)->word[0];
                    }
                    break;
                case 4:                 /* JMP NEAR Ed */
                    R_EIP = ED->dword[0];
                    break;
                case 5:                 /* JMP FAR Ed */
                    if(nextop>0xc0) {
                        printf_log(LOG_NONE, "Illegal Opcode 0x%02X 0x%02X\n", opcode, nextop);
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                    } else {
                        R_EIP = ED->dword[0];
                        R_CS = (ED+1)->word[0];
                    }
                    break;
                case 6:                 /* Push Ed */
                    Push(emu, ED->dword[0]);
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
