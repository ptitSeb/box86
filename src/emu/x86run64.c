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
#include "bridge.h"
#include "signals.h"
#ifdef DYNAREC
#include "../dynarec/arm_lock_helper.h"
#endif

#include "modrm.h"


#ifdef TEST_INTERPRETER
uintptr_t Test64(x86test_t *test, int seg, uintptr_t addr)
#else
uintptr_t Run64(x86emu_t *emu, int seg, uintptr_t addr)
#endif
{
    uint8_t opcode = F8;
    uint8_t nextop;
    reg32_t *oped;
    uint8_t tmp8u;
    uint32_t tmp32u;
    int32_t tmp32s;
    #ifdef TEST_INTERPRETER
    x86emu_t* emu = test->emu;
    #endif
    uintptr_t tlsdata = (seg==_FS)?GetFSBaseEmu(emu):GetGSBaseEmu(emu);
    switch(opcode) {
        case 0x01:              /* ADD GS:Ed, Gd */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            ED->dword[0] = add32(emu, ED->dword[0], GD.dword[0]);
            break;

        case 0x03:              /* ADD Gd, GS:Ed */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            GD.dword[0] = add32(emu, GD.dword[0], ED->dword[0]);
            break;

        case 0x06:              /* PUSH ES */
            Push(emu, emu->segs[_ES]);
            break;
        case 0x07:             /* POP ES */
            emu->segs[_ES] = Pop(emu);
            emu->segs_serial[_ES] = 0;
            break;

        case 0x0f:
            #ifdef TEST_INTERPRETER
            return Test640F(test, tlsdata, addr);
            #else
            return Run640F(emu, tlsdata, addr);
            #endif

        case 0x11:              /* ADC GS:Ed, Gd */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            ED->dword[0] = adc32(emu, ED->dword[0], GD.dword[0]);
            break;
        
        case 0x13:              /* ADC Gd, GS:Ed */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            GD.dword[0] = adc32(emu, GD.dword[0], ED->dword[0]);
            break;

        case 0x20:              /* AND Seg:Eb,Gb */
            nextop = F8;
            GET_EB_OFFS(tlsdata);
            EB->byte[0] = and8(emu, EB->byte[0], GB);
            break;
        case 0x21:              /* AND Seg:Ed,Gd */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            ED->dword[0] = and32(emu, ED->dword[0], GD.dword[0]);
            break;

        case 0x23:              /* AND Gd,Ed */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            GD.dword[0] = and32(emu, GD.dword[0], ED->dword[0]);
            break;

        case 0x29:              /* SUB GS:Ed, Gd */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            ED->dword[0] = sub32(emu, ED->dword[0], GD.dword[0]);
            break;

        case 0x2B:              /* SUB Gd, GS:Ed */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            GD.dword[0] = sub32(emu, GD.dword[0], ED->dword[0]);
            break;

        case 0x31:              /* XOR Ed,Gd */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            ED->dword[0] = xor32(emu, ED->dword[0], GD.dword[0]);
            break;

        case 0x33:              /* XOR Gd,Ed */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            GD.dword[0] = xor32(emu, GD.dword[0], ED->dword[0]);
            break;

        case 0x39:              /* CMP Ed,Gd */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            cmp32(emu, ED->dword[0], GD.dword[0]);
            break;

        case 0x3B:              /* CMP Gd,Ed */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            cmp32(emu, GD.dword[0], ED->dword[0]);
            break;
        
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:             /* INC Reg */
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
        case 0x4F:            /* DEC Reg */
            tmp8u = opcode&7;
            emu->regs[tmp8u].dword[0] = dec32(emu, emu->regs[tmp8u].dword[0]);
            break;
        case 0x50:              /* PUSH Reg */
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:               /* PUSH ESP */
        case 0x55:
        case 0x56:
        case 0x57:              
        case 0x58:              /* POP Reg */
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:              /* POP ESP */
        case 0x5D:
        case 0x5E:
        case 0x5F:
            // Segment override if for memory loc, no stack segment 
            return addr-1;

        case 0x64:              /* FS: */
            #ifdef TEST_INTERPRETER
            return Test64(test, _FS, addr-1); // put FS back
            #else
            return Run64(emu, _FS, addr-1); // put FS back
            #endif
        case 0x65:                      /* GS: */
            #ifdef TEST_INTERPRETER
            return Test64(test, _GS, addr);
            #else
            return Run64(emu, _GS, addr);
            #endif
            break;
        case 0x66:
            #ifdef TEST_INTERPRETER
            return Test6466(test, tlsdata, addr);
            #else
            return Run6466(emu, tlsdata, addr);
            #endif
        case 0x67:
            #ifdef TEST_INTERPRETER
            return Test6467(test, tlsdata, addr);
            #else
            return Run6467(emu, tlsdata, addr);
            #endif

        case 0x69:              /* IMUL Gd,Ed,Id */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            tmp32u = F32;
            GD.dword[0] = imul32(emu, ED->dword[0], tmp32u);
            break;
        
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
        case 0x78:
        case 0x79:
        case 0x7A:
        case 0x7B:
        case 0x7C:
        case 0x7D:
        case 0x7E:
        case 0x7F:
            // just ignore the prefix
            return addr-1;
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
        case 0x87:
            nextop = F8;
#if defined(DYNAREC) && !defined(TEST_INTERPRETER)
            GET_ED_OFFS(tlsdata);
            if(MODREG) {
                tmp32u = GD.dword[0];
                GD.dword[0] = ED->dword[0];
                ED->dword[0] = tmp32u;
            } else {
                if(((uintptr_t)ED)&3)
                {
                    // not aligned, dont't try to "LOCK"
                    tmp32u = ED->dword[0];
                    ED->dword[0] = GD.dword[0];
                    GD.dword[0] = tmp32u;
                } else {
                    // XCHG is supposed to automaticaly LOCK memory bus
                    GD.dword[0] = arm_lock_xchg(ED, GD.dword[0]);
                }
            }
#else
            GET_ED_OFFS(tlsdata);
            if(!MODREG)
                pthread_mutex_lock(&emu->context->mutex_lock); // XCHG always LOCK (but when accessing memory only)
            tmp32u = GD.dword[0];
            GD.dword[0] = ED->dword[0];
            ED->dword[0] = tmp32u;
            if(!MODREG)
                pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
            break;
        case 0x88:              /* MOV Seg:Eb,Gb */
            nextop = F8;
            GET_EB_OFFS(tlsdata);
            EB->byte[0] = GB;
            break;
        case 0x89:              /* MOV Ed,Gd */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            ED->dword[0] = GD.dword[0];
            break;
        case 0x8A:              /* MOV Gb,GS:Eb */
            nextop = F8;
            GET_EB_OFFS(tlsdata);
            GB = EB->byte[0];
            break;
        case 0x8B:              /* MOV Gd,Ed */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            GD.dword[0] = ED->dword[0];
            break;
        case 0x8C:              /* MOV Seg:Ed, Seg */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            ED->dword[0] = emu->segs[((nextop&0x38)>>3)];
            break;
        case 0x8E:              /* MOV Seg,Seg:Ed */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            emu->segs[((nextop&0x38)>>3)] = ED->word[0];
            emu->segs_serial[((nextop&0x38)>>3)] = 0;
            break;
        case 0x8F:              /* POP Ed */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            ED->dword[0] = Pop(emu);
            break;
        case 0x90:              /* NOP */
            break;

        case 0x9C:              /* PUSHFD */
            // Segment override if for memory loc, no stack segment 
            return addr-1;   // so ignore prefix and continue

        case 0xA0:              /* MOV AL,Ob */
            tmp32s = F32S;
            R_AL = *(uint8_t*)((tlsdata) + tmp32s);
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

        case 0xA8:            /* TEST AL, Ib */
            test8(emu, R_AL, F8);
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

        case 0xD0:                      /* GRP2 Eb,1 */
        case 0xD2:                      /* GRP2 Eb,CL */
            nextop = F8;
            GET_EB_OFFS(tlsdata);
            tmp8u = (opcode==0xD0)?1:R_CL;
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
        case 0xD1:                      /* GRP2 Ed,1 */
        case 0xD3:                      /* GRP2 Ed,CL */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            tmp8u = (opcode==0xD1)?1:R_CL;
            switch((nextop>>3)&7) {
                case 0: ED->dword[0] = rol32(emu, ED->dword[0], tmp8u); break;
                case 1: ED->dword[0] = ror32(emu, ED->dword[0], tmp8u); break;
                case 2: ED->dword[0] = rcl32(emu, ED->dword[0], tmp8u); break;
                case 3: ED->dword[0] = rcr32(emu, ED->dword[0], tmp8u); break;
                case 4: 
                case 6: ED->dword[0] = shl32(emu, ED->dword[0], tmp8u); break;
                case 5: ED->dword[0] = shr32(emu, ED->dword[0], tmp8u); break;
                case 7: ED->dword[0] = sar32(emu, ED->dword[0], tmp8u); break;
            }
            break;

        case 0xE9:
        case 0xEB:
            return addr-1;       // ignore FS: to execute regular opcode
            break;

        case 0xF7:                      /* GRP3 Ed(,Id) */
            nextop = F8;
            GET_ED_OFFS(tlsdata);
            switch((nextop>>3)&7) {
                case 0: 
                case 1:                 /* TEST Ed,Id */
                    tmp32u = F32;
                    test32(emu, ED->dword[0], tmp32u);
                    break;
                case 2:                 /* NOT Ed */
                    ED->dword[0] = not32(emu, ED->dword[0]);
                    break;
                case 3:                 /* NEG Ed */
                    ED->dword[0] = neg32(emu, ED->dword[0]);
                    break;
                case 4:                 /* MUL EAX,Ed */
                    mul32_eax(emu, ED->dword[0]);
                    break;
                case 5:                 /* IMUL EAX,Ed */
                    imul32_eax(emu, ED->dword[0]);
                    break;
                case 6:                 /* DIV Ed */
                    if(!ED->dword[0])
                        emit_div0(emu, (void*)R_EIP, 0);
                    div32(emu, ED->dword[0]);
                    break;
                case 7:                 /* IDIV Ed */
                    if(!ED->dword[0])
                        emit_div0(emu, (void*)R_EIP, 0);
                    idiv32(emu, ED->dword[0]);
                    break;
            }
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
                    R_EIP = (uintptr_t)getAlternate((void*)ED->dword[0]);
                    Push(emu, addr);
                    addr = R_EIP;
                    break;
                case 3:                 /* CALL FAR Ed */
                    if(nextop>0xc0) {
                        printf_log(LOG_NONE, "Illegal Opcode %02X %02X\n", opcode, nextop);
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                        return 0;
                    } else {
                        Push16(emu, R_CS);
                        Push(emu, addr);
                        addr = (uintptr_t)getAlternate((void*)ED->dword[0]);
                        R_CS = (ED+1)->word[0];
                    }
                    break;
                case 4:                 /* JMP NEAR Ed */
                    addr = (uintptr_t)getAlternate((void*)ED->dword[0]);
                    break;
                case 5:                 /* JMP FAR Ed */
                    if(nextop>0xc0) {
                        printf_log(LOG_NONE, "Illegal Opcode 0x%02X 0x%02X\n", opcode, nextop);
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                        return 0;
                    } else {
                        addr = (uintptr_t)getAlternate((void*)ED->dword[0]);
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
                    return 0;
            }
            break;
        default:
            return 0;
    }
    return addr;
}
