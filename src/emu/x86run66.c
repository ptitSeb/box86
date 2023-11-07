#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "debug.h"
#include "box86stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"
#include "box86context.h"
#include "bridge.h"
#ifdef DYNAREC
#include "../dynarec/arm_lock_helper.h"
#endif

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t Test66(x86test_t *test, int rep, uintptr_t addr)
#else
uintptr_t Run66(x86emu_t *emu, int rep, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    int8_t tmp8s;
    uint8_t tmp8u, tmp8u2;
    int16_t tmp16s;
    uint16_t tmp16u, tmp16u2;
    int32_t tmp32s;
    uint32_t tmp32u;
    int64_t tmp64s;
    uint64_t tmp64u, tmp64u2, tmp64u3;
    reg32_t *oped, *opgd;
    uintptr_t tlsdata;
    #ifdef TEST_INTERPRETER
    x86emu_t* emu = test->emu;
    #endif

    opcode = F8;

    while((opcode==0x2E) || (opcode==0x26) || (opcode==0x36) || (opcode==0x3E) || (opcode==0x66))   // ignoring Seg: or multiple 0x66
        opcode = F8;

    while((opcode==0xF2) || (opcode==0xF3)) {
        rep = opcode-0xF1;
        opcode = F8;
    }

    switch(opcode) {

        #define GO(B, OP)                               \
        case B+0:                                       \
            nextop = F8;                                \
            GET_EB;                                     \
            EB->byte[0] = OP##8(emu, EB->byte[0], GB);  \
            break;                                      \
        case B+1:                                       \
            nextop = F8;                                \
            GET_EW;                                     \
            EW->word[0] = OP##16(emu, EW->word[0], GW.word[0]); \
            break;                                      \
        case B+2:                                       \
            nextop = F8;                                \
            GET_EB;                                     \
            GB = OP##8(emu, GB, EB->byte[0]);           \
            break;                                      \
        case B+3:                                       \
            nextop = F8;                                \
            GET_EW;                                     \
            GW.word[0] = OP##16(emu, GW.word[0], EW->word[0]); \
            break;                                      \
        case B+4:                                       \
            R_AL = OP##8(emu, R_AL, F8);                \
            break;                                      \
        case B+5:                                       \
            R_AX = OP##16(emu, R_AX, F16);              \
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
        case 0x06:                      /* PUSH ES */
            Push16(emu, emu->segs[_ES]);
            break;
        case 0x07:                      /* POP ES */
            emu->segs[_ES] = Pop16(emu);    // no check, no use....
            emu->segs_serial[_ES] = 0;
            break;

        case 0x0F:                      /* 66 0f prefix */
            switch(rep) {
                case 2: return 0;
                case 1:
                    #ifdef TEST_INTERPRETER
                    return Test66F20F(test, addr);
                    #else
                    return Run66F20F(emu, addr);
                    #endif
                default:
                    #ifdef TEST_INTERPRETER
                    return Test660F(test, addr);
                    #else
                    return Run660F(emu, addr);
                    #endif
            }

        case 0x1E:                      /* PUSH DS */
            Push16(emu, emu->segs[_DS]);
            break;
        case 0x1F:                      /* POP DS */
            emu->segs[_DS] = Pop16(emu);    // no check, no use....
            emu->segs_serial[_DS] = 0;
            break;
            
        case 0x39:
            nextop = F8;
            GET_EW;
            cmp16(emu, EW->word[0], GW.word[0]);
            break;
        case 0x3B:
            nextop = F8;
            GET_EW;
            cmp16(emu, GW.word[0], EW->word[0]);
            break;
        case 0x3D:
            cmp16(emu, R_AX, F16);
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
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:                              /* PUSH Reg */
            tmp8u = opcode&7;
            Push16(emu, emu->regs[tmp8u].word[0]);
            break;
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:                      
        case 0x5D:
        case 0x5E:
        case 0x5F:                              /* POP Reg */
            tmp8u = opcode&7;
            emu->regs[tmp8u].word[0] = Pop16(emu);
            break;
        case 0x60:                              /* PUSHA */
            tmp16u = R_SP;
            Push16(emu, R_AX);
            Push16(emu, R_CX);
            Push16(emu, R_DX);
            Push16(emu, R_BX);
            Push16(emu, tmp16u);
            Push16(emu, R_BP);
            Push16(emu, R_SI);
            Push16(emu, R_DI);
            break;
        case 0x61:                              /* POPA */
            R_DI = Pop16(emu);
            R_SI = Pop16(emu);
            R_BP = Pop16(emu);
            R_ESP+=2;   // POP ESP
            R_BX = Pop16(emu);
            R_DX = Pop16(emu);
            R_CX = Pop16(emu);
            R_AX = Pop16(emu);
            break;

        case 0x64:
            tlsdata = GetFSBaseEmu(emu);
            #ifdef TEST_INTERPRETER
            return Test6466(test, tlsdata, addr);
            #else
            return Run6466(emu, tlsdata, addr);
            #endif

        case 0x68:                       /* PUSH u16 */
            tmp16u = F16;
            Push16(emu, tmp16u);
            break;
        case 0x69:                      /* IMUL Gw,Ew,Iw */
            nextop = F8;
            GET_EW;
            tmp16u = F16;
            GW.word[0] = imul16(emu, EW->word[0], tmp16u);
            break;
        case 0x6A:                      /* PUSH Ib (as signed word) */
            tmp16s = F8S;
            Push16(emu, (uint16_t)tmp16s);
            break;
        case 0x6B:                      /* IMUL Gw,Ew,Ib */
            nextop = F8;
            GET_EW;
            tmp16s = F8S;
            GW.word[0] = imul16(emu, EW->word[0], (uint16_t)tmp16s);
            break;

        case 0x81:                              /* GRP3 Ew,Iw */
        case 0x83:                              /* GRP3 Ew,Ib */
            nextop = F8;
            GET_EW;
            if(opcode==0x81) 
                tmp16u = F16;
            else {
                tmp16s = F8S;
                tmp16u = (uint16_t)tmp16s;
            }
            switch((nextop>>3)&7) {
                case 0: EW->word[0] = add16(emu, EW->word[0], tmp16u); break;
                case 1: EW->word[0] =  or16(emu, EW->word[0], tmp16u); break;
                case 2: EW->word[0] = adc16(emu, EW->word[0], tmp16u); break;
                case 3: EW->word[0] = sbb16(emu, EW->word[0], tmp16u); break;
                case 4: EW->word[0] = and16(emu, EW->word[0], tmp16u); break;
                case 5: EW->word[0] = sub16(emu, EW->word[0], tmp16u); break;
                case 6: EW->word[0] = xor16(emu, EW->word[0], tmp16u); break;
                case 7:               cmp16(emu, EW->word[0], tmp16u); break;
            }
            break;

        case 0x85:                              /* TEST Ew,Gw */
            nextop = F8;
            GET_EW;
            test16(emu, EW->word[0], GW.word[0]);
            break;

        case 0x87:                              /* XCHG Ew,Gw */
            nextop = F8;
            GET_EW;
            tmp16u = GW.word[0];
            GW.word[0] = EW->word[0];
            EW->word[0] = tmp16u;
            break;

        case 0x89:                              /* MOV Ew,Gw */
            nextop = F8;
            GET_EW;
            EW->word[0] = GW.word[0];
            break;

        case 0x8B:                              /* MOV Gw,Ew */
            nextop = F8;
            GET_EW;
            GW.word[0] = EW->word[0];
            break;
        case 0x8C:                              /* MOV Ew,Seg */
            nextop = F8;
            GET_EW;
            EW->word[0] = emu->segs[(nextop&0x38)>>3];
            break;
        
        case 0x8E:                               /* MOV Seg,Ew */
            nextop = F8;
            GET_EW;
            emu->segs[((nextop&0x38)>>3)] = EW->word[0];
            emu->segs_serial[((nextop&0x38)>>3)] = 0;
            break;
        case 0x8F:                              /* POP Ew */
            nextop = F8;
            GET_EW;
            EW->word[0] = Pop16(emu);
            break;
        case 0x90:                              /* NOP */
            break;

        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:                      /* XCHG reg,EAX */
            tmp16u = R_AX;
            R_AX = emu->regs[opcode&7].word[0];
            emu->regs[opcode&7].word[0] = tmp16u;
            break;

        case 0x98:                               /* CBW */
            emu->regs[_AX].sword[0] = emu->regs[_AX].sbyte[0];
            break;
        case 0x99:                              /* CWD */
            R_DX=((R_AX & 0x8000)?0xFFFF:0x0000);
            break;

        case 0x9C:                              /* PUSHFW */
            CHECK_FLAGS(emu);
            Push16(emu, (uint16_t)emu->eflags.x32);
            break;
        case 0x9D:                              /* POPFW */
            CHECK_FLAGS(emu);
            emu->eflags.x32 &=0xffff0000;
            emu->eflags.x32 |= (Pop16(emu) & 0x3F7FD7) | 0x2;
            break;

        case 0xA1:                              /* MOV AX,Ow */
            R_AX = *(uint16_t*)F32;
            break;
        case 0xA3:                              /* MOV Ow,AX */
            *(uint16_t*)F32 = R_AX;
            break;
        case 0xA4:                              /* MOVSB */
            tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
            tmp32u = rep?R_ECX:1;
            while(tmp32u--) {
                #ifndef TEST_INTERPRETER
                *(uint8_t*)R_EDI = *(uint8_t*)R_ESI;
                #endif
                R_EDI += tmp8s;
                R_ESI += tmp8s;
            }
            if(rep) R_ECX = 0;
            break;
        case 0xA5:                              /* MOVSW */
            tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
            tmp32u = rep?R_ECX:1;
            while(tmp32u--) {
                #ifndef TEST_INTERPRETER
                *(uint16_t*)R_EDI = *(uint16_t*)R_ESI;
                #endif
                R_EDI += tmp8s;
                R_ESI += tmp8s;
            }
            if(rep) R_ECX = 0;
            break;

        case 0xA7:                              /* CMPSW */
            tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
            switch(rep) {
                case 0:
                    tmp16u  = *(uint16_t*)R_EDI;
                    tmp16u2 = *(uint16_t*)R_ESI;
                    R_EDI += tmp8s;
                    R_ESI += tmp8s;
                    cmp16(emu, tmp16u2, tmp16u);
                    break;
                case 1:
                case 2:
                    tmp16u = 0;
                    tmp16u2 = 0;
                    tmp32u = R_ECX;
                    while(tmp32u) {
                        --tmp32u;
                        tmp16u  = *(uint16_t*)R_EDI;
                        tmp16u2 = *(uint16_t*)R_ESI;
                        R_EDI += tmp8s;
                        R_ESI += tmp8s;
                        if((tmp16u==tmp16u2)==(rep==1))
                            break;
                    }
                    if(R_ECX) cmp16(emu, tmp16u2, tmp16u);
                    R_ECX = tmp32u;
                    break;
            }
            break;

        case 0xA9:                             /* TEST AX,Iw */
            test16(emu, R_AX, F16);
            break;

        case 0xAB:                              /* STOSW */
            tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
            tmp32u = rep?R_ECX:1;
            while(tmp32u--) {
                #ifndef TEST_INTERPRETER
                *(uint16_t*)R_EDI = R_AX;
                #endif
                R_EDI += tmp8s;
            }
            if(rep) R_ECX = 0;
            break;
        case 0xAD:                              /* LODSW */
            tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
            tmp32u = rep?R_ECX:1;
            while(tmp32u--) {
                R_AX = *(uint16_t*)R_ESI;
                R_ESI += tmp8s;
            }
            if(rep) R_ECX = 0;
            break;
        case 0xAF:                              /* SCASW */
            tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
            switch(rep) {
                case 0:
                    cmp16(emu, R_AX, *(uint16_t*)R_EDI);
                    R_EDI += tmp8s;
                    break;
                case 1:
                case 2:
                    tmp16u = 0;
                    tmp32u = R_ECX;
                    while(tmp32u) {
                        --tmp32u;
                        tmp16u = *(uint16_t*)R_EDI;
                        R_EDI += tmp8s;
                        if((R_AX==tmp16u)==(rep==1))
                            break;
                    }
                    if(R_ECX) cmp16(emu, R_AX, tmp16u);
                    R_ECX = tmp32u;
                    break;
            }
            break;

        case 0xB8:                              /* MOV AX,Iw */
        case 0xB9:                              /* MOV CX,Iw */
        case 0xBA:                              /* MOV DX,Iw */
        case 0xBB:                              /* MOV BX,Iw */
        case 0xBC:                              /*    ...     */
        case 0xBD:
        case 0xBE:
        case 0xBF:
            emu->regs[opcode&7].word[0] = F16;
            break;

        case 0xC1:                              /* GRP2 Ew,Ib */
            nextop = F8;
            GET_EW;
            tmp8u = F8 /*& 0x1f*/;
            switch((nextop>>3)&7) {
                case 0: EW->word[0] = rol16(emu, EW->word[0], tmp8u); break;
                case 1: EW->word[0] = ror16(emu, EW->word[0], tmp8u); break;
                case 2: EW->word[0] = rcl16(emu, EW->word[0], tmp8u); break;
                case 3: EW->word[0] = rcr16(emu, EW->word[0], tmp8u); break;
                case 4:
                case 6: EW->word[0] = shl16(emu, EW->word[0], tmp8u); break;
                case 5: EW->word[0] = shr16(emu, EW->word[0], tmp8u); break;
                case 7: EW->word[0] = sar16(emu, EW->word[0], tmp8u); break;
            }
            break;

        case 0xC7:                              /* MOV Ew,Iw */
            nextop = F8;
            GET_EW;
            EW->word[0] = F16;
            break;

        case 0xCB:                               /* FAR RET */
            addr = Pop(emu);
            emu->segs[_CS] = Pop(emu);    // no check, no use....
            emu->segs_serial[_CS] = 0;
            // need to check status of CS register!
            break;
        case 0xCC:                              /* INT3 */
            #ifndef TEST_INTERPRETER
            if(my_context->signals[SIGTRAP])
                raise(SIGTRAP);
            #endif
            break;
        case 0xD1:                              /* GRP2 Ew,1  */
        case 0xD3:                              /* GRP2 Ew,CL */
            nextop = F8;
            GET_EW;
            tmp8u=(opcode==0xD3)?R_CL:1;
            switch((nextop>>3)&7) {
                case 0: EW->word[0] = rol16(emu, EW->word[0], tmp8u); break;
                case 1: EW->word[0] = ror16(emu, EW->word[0], tmp8u); break;
                case 2: EW->word[0] = rcl16(emu, EW->word[0], tmp8u); break;
                case 3: EW->word[0] = rcr16(emu, EW->word[0], tmp8u); break;
                case 4: 
                case 6: EW->word[0] = shl16(emu, EW->word[0], tmp8u); break;
                case 5: EW->word[0] = shr16(emu, EW->word[0], tmp8u); break;
                case 7: EW->word[0] = sar16(emu, EW->word[0], tmp8u); break;
            }
            break;
        
        case 0xD9:
            #ifdef TEST_INTERPRETER
            return Test66D9(test, addr);
            #else
            return Run66D9(emu, addr);
            #endif

        case 0xDD:
            #ifdef TEST_INTERPRETER
            return Test66DD(test, addr);
            #else
            return Run66DD(emu, addr);
            #endif

        case 0xF0:                      /* LOCK prefix */
            #ifdef TEST_INTERPRETER
            return TestF066(test, addr);
            #else
            return RunF066(emu, addr);
            #endif

        case 0xF5:                      /* CMC */
            CHECK_FLAGS(emu);
            CONDITIONAL_SET_FLAG(!ACCESS_FLAG(F_CF), F_CF);
            break;

        case 0xF7:                      /* GRP3 Ew(,Iw) */
            nextop = F8;
            GET_EW;
            switch((nextop>>3)&7) {
                case 0: 
                case 1:                 /* TEST Ew,Iw */
                    test16(emu, EW->word[0], F16);
                    break;
                case 2:                 /* NOT Ew */
                    EW->word[0] = not16(emu, EW->word[0]);
                    break;
                case 3:                 /* NEG Ew */
                    EW->word[0] = neg16(emu, EW->word[0]);
                    break;
                case 4:                 /* MUL AX,Ew */
                    mul16(emu, EW->word[0]);
                    break;
                case 5:                 /* IMUL AX,Ew */
                    imul16_eax(emu, EW->word[0]);
                    break;
                case 6:                 /* DIV Ew */
                    div16(emu, EW->word[0]);
                    break;
                case 7:                 /* IDIV Ew */
                    idiv16(emu, EW->word[0]);
                    break;
            }
            break;
        case 0xF8:                       /* CLC */
            CHECK_FLAGS(emu);
            CLEAR_FLAG(F_CF);
            break;
        case 0xF9:                       /* STC */
            CHECK_FLAGS(emu);
            SET_FLAG(F_CF);
            break;

        case 0xFF:                      /* GRP 5 Ew */
            nextop = F8;
            GET_EW;
            switch((nextop>>3)&7) {
                case 0:                 /* INC Ed */
                    EW->word[0] = inc16(emu, EW->word[0]);
                    break;
                case 1:                 /* DEC Ed */
                    EW->word[0] = dec16(emu, EW->word[0]);
                    break;
                case 6:
                    Push16(emu, EW->word[0]);
                    break;
                default:
                    return 0;
            }
            break;

        default:
            return 0;
    }
    return addr;
}