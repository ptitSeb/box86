#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include "debug.h"
#include "box86context.h"
#include "dynarec.h"
#include "emu/x86emu_private.h"
#include "emu/x86run_private.h"
#include "x86run.h"
#include "x86emu.h"
#include "box86stack.h"
#include "callback.h"
#include "bridge.h"
#include "emu/x86run_private.h"
#include "x86trace.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"
#include "arm_printer.h"

#include "dynarec_arm_functions.h"
#include "dynarec_arm_helper.h"

uintptr_t dynarec00(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop, opcode;
    uint8_t gd, ed;
    int8_t i8;
    int32_t i32, j32, tmp;
    uint8_t u8;
    uint8_t gb1, gb2, eb1, eb2;
    uint32_t u32;
    uint8_t wback, wb1, wb2;
    int fixedaddress;

    opcode = F8;
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(tmp);
    MAYUSE(j32);

    switch(opcode) {
        case 0x00:
            INST_NAME("ADD Eb, Gb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x1);
            GETGB(x2);
            emit_add8(dyn, ninst, x1, x2, x14, x3, (wb1 && (wback==x3))?1:0);
            EBBACK;
            break;
        case 0x01:
            INST_NAME("ADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED;
            emit_add32(dyn, ninst, ed, gd, x3, x14);
            WBACK;
            break;
        case 0x02:
            INST_NAME("ADD Gb, Eb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x2);
            GETGB(x1);
            emit_add8(dyn, ninst, x1, x2, x3, x14, 0);
            GBBACK;
            break;
        case 0x03:
            INST_NAME("ADD Gd, Ed");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED;
            emit_add32(dyn, ninst, gd, ed, x3, x14);
            break;
        case 0x04:
            INST_NAME("ADD AL, Ib");
            SETFLAGS(X_ALL, SF_SET);
            u8 = F8;
            UXTB(x1, xEAX, 0);
            emit_add8c(dyn, ninst, x1, u8, x3, x14);
            BFI(xEAX, x1, 0, 8);
            break;
        case 0x05:
            INST_NAME("ADD EAX, Id");
            SETFLAGS(X_ALL, SF_SET);
            i32 = F32S;
            emit_add32c(dyn, ninst, xEAX, i32, x3, x14);
            break;
        case 0x06:
            INST_NAME("PUSH ES");
            MOVW(x1, offsetof(x86emu_t, segs[_ES]));
            LDRH_REG(x2, xEmu, x1);
            PUSH1(x2);
            break;
        case 0x07:
            INST_NAME("POP ES");
            MOVW(x1, offsetof(x86emu_t, segs[_ES]));
            POP1(x2);
            STRH_REG(x2, xEmu, x1);
            MOVW(x1, 0);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, segs_serial[_ES]));
            break;
        case 0x08:
            INST_NAME("OR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x1);
            GETGB(x2);
            emit_or8(dyn, ninst, x1, x2, x14, x2);
            EBBACK;
            break;
        case 0x09:
            INST_NAME("OR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED;
            emit_or32(dyn, ninst, ed, gd, x3, x14);
            WBACK;
            break;
        case 0x0A:
            INST_NAME("OR Gb, Eb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x2);
            GETGB(x1);
            emit_or8(dyn, ninst, x1, x2, x3, x14);
            GBBACK;
            break;
        case 0x0B:
            INST_NAME("OR Gd, Ed");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED;
            emit_or32(dyn, ninst, gd, ed, x3, x14);
            break;
        case 0x0C:
            INST_NAME("OR AL, Ib");
            SETFLAGS(X_ALL, SF_SET);
            u8 = F8;
            UXTB(x1, xEAX, 0);
            emit_or8c(dyn, ninst, x1, u8, x3, x14);
            BFI(xEAX, x1, 0, 8);
            break;
        case 0x0D:
            INST_NAME("OR EAX, Id");
            SETFLAGS(X_ALL, SF_SET);
            i32 = F32S;
            emit_or32c(dyn, ninst, xEAX, i32, x3, x14);
            break;

        case 0x0F:
            addr = dynarec0F(dyn, addr, ip, ninst, ok, need_epilog);
            break;
        case 0x10:
            INST_NAME("ADC Eb, Gb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x1);
            GETGB(x2);
            emit_adc8(dyn, ninst, x1, x2, x14, x3, (wb1 && (wback==x3))?1:0);
            EBBACK;
            break;
        case 0x11:
            INST_NAME("ADC Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED;
            emit_adc32(dyn, ninst, ed, gd, x3, x14);
            WBACK;
            break;
        case 0x12:
            INST_NAME("ADC Gb, Eb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x2);
            GETGB(x1);
            emit_adc8(dyn, ninst, x1, x2, x14, x3, 0);
            GBBACK;
            break;
        case 0x13:
            INST_NAME("ADC Gd, Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED;
            emit_adc32(dyn, ninst, gd, ed, x3, x14);
            break;
        case 0x14:
            INST_NAME("ADC AL, Ib");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            u8 = F8;
            UXTB(x1, xEAX, 0);
            emit_adc8c(dyn, ninst, x1, u8, x3, x14);
            BFI(xEAX, x1, 0, 8);
            break;
        case 0x15:
            INST_NAME("ADC EAX, Id");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            i32 = F32S;
            emit_adc32c(dyn, ninst, xEAX, i32, x3, x14);
            break;

        case 0x18:
            INST_NAME("SBB Eb, Gb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x1);
            GETGB(x2);
            emit_sbb8(dyn, ninst, x1, x2, x14, x3, (wb1 && (wback==x3))?1:0);
            EBBACK;
            break;
        case 0x19:
            INST_NAME("SBB Ed, Gd");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED;
            emit_sbb32(dyn, ninst, ed, gd, x3, x14);
            WBACK;
            break;
        case 0x1A:
            INST_NAME("SBB Gb, Eb");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x2);
            GETGB(x1);
            emit_sbb8(dyn, ninst, x1, x2, x3, x14, 0);
            GBBACK;
            break;
        case 0x1B:
            INST_NAME("SBB Gd, Ed");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED;
            emit_sbb32(dyn, ninst, gd, ed, x3, x14);
            break;
        case 0x1C:
            INST_NAME("SBB AL, Ib");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            u8 = F8;
            UXTB(x1, xEAX, 0);
            emit_sbb8c(dyn, ninst, x1, u8, x3, x14);
            BFI(xEAX, x1, 0, 8);
            break;
        case 0x1D:
            INST_NAME("SBB EAX, Id");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET);
            i32 = F32S;
            emit_sbb32c(dyn, ninst, xEAX, i32, x3, x14);
            break;
        case 0x1E:
            INST_NAME("PUSH DS");
            MOVW(x1, offsetof(x86emu_t, segs[_DS]));
            LDRH_REG(x2, xEmu, x1);
            PUSH1(x2);
            break;
        case 0x1F:
            INST_NAME("POP DS");
            MOVW(x1, offsetof(x86emu_t, segs[_DS]));
            POP1(x2);
            STRH_REG(x2, xEmu, x1);
            MOVW(x1, 0);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, segs_serial[_DS]));
            break;
        case 0x20:
            INST_NAME("AND Eb, Gb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x1);
            GETGB(x2);
            emit_and8(dyn, ninst, x1, x2, x14, x2);
            EBBACK;
            break;
        case 0x21:
            INST_NAME("AND Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED;
            emit_and32(dyn, ninst, ed, gd, x3, x14);
            WBACK;
            break;
        case 0x22:
            INST_NAME("AND Gb, Eb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x2);
            GETGB(x1);
            emit_and8(dyn, ninst, x1, x2, x3, x14);
            GBBACK;
            break;
        case 0x23:
            INST_NAME("AND Gd, Ed");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED;
            emit_and32(dyn, ninst, gd, ed, x3, x14);
            break;
        case 0x24:
            INST_NAME("AND AL, Ib");
            SETFLAGS(X_ALL, SF_PENDING);
            u8 = F8;
            UXTB(x1, xEAX, 0);
            emit_and8c(dyn, ninst, x1, u8, x3, x14);
            BFI(xEAX, x1, 0, 8);
            break;
        case 0x25:
            INST_NAME("AND EAX, Id");
            SETFLAGS(X_ALL, SF_SET);
            i32 = F32S;
            emit_and32c(dyn, ninst, xEAX, i32, x3, x14);
            break;
        case 0x26:
            INST_NAME("ES:");
            // ignored
            break;
        case 0x27:
            INST_NAME("DAA");
            READFLAGS(X_AF|X_CF);
            SETFLAGS(X_AF|X_CF|X_PF|X_SF|X_ZF, SF_SET);
            UXTB(x1, xEAX, 0);
            CALL_(daa8, x1, 0);
            BFI(xEAX, x1, 0, 8);
            break;
        case 0x28:
            INST_NAME("SUB Eb, Gb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x1);
            GETGB(x2);
            emit_sub8(dyn, ninst, x1, x2, x14, x3, (wb1 && (wback==x3))?1:0);
            EBBACK;
            break;
        case 0x29:
            INST_NAME("SUB Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED;
            emit_sub32(dyn, ninst, ed, gd, x3, x14);
            WBACK;
            break;
        case 0x2A:
            INST_NAME("SUB Gb, Eb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x2);
            GETGB(x1);
            emit_sub8(dyn, ninst, x1, x2, x3, x14, 0);
            GBBACK;
            break;
        case 0x2B:
            INST_NAME("SUB Gd, Ed");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED;
            emit_sub32(dyn, ninst, gd, ed, x3, x14);
            break;
        case 0x2C:
            INST_NAME("SUB AL, Ib");
            SETFLAGS(X_ALL, SF_SET);
            u8 = F8;
            UXTB(x1, xEAX, 0);
            emit_sub8c(dyn, ninst, x1, u8, x3, x14);
            BFI(xEAX, x1, 0, 8);
            break;
        case 0x2D:
            INST_NAME("SUB EAX, Id");
            SETFLAGS(X_ALL, SF_SET);
            i32 = F32S;
            emit_sub32c(dyn, ninst, xEAX, i32, x3, x14);
            break;
        case 0x2E:
            INST_NAME("CS:");
            // ignored
            break;
        case 0x2F:
            INST_NAME("DAS");
            READFLAGS(X_AF|X_CF);
            SETFLAGS(X_AF|X_CF|X_PF|X_SF|X_ZF, SF_SET);
            UXTB(x1, xEAX, 0);
            CALL_(das8, x1, 0);
            BFI(xEAX, x1, 0, 8);
            break;
        case 0x30:
            INST_NAME("XOR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x1);
            GETGB(x2);
            emit_xor8(dyn, ninst, x1, x2, x14, x2);
            EBBACK;
            break;
        case 0x31:
            INST_NAME("XOR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED;
            emit_xor32(dyn, ninst, ed, gd, x3, x14);
            WBACK;
            break;
        case 0x32:
            INST_NAME("XOR Gb, Eb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x2);
            GETGB(x1);
            emit_xor8(dyn, ninst, x1, x2, x3, x14);
            GBBACK;
            break;
        case 0x33:
            INST_NAME("XOR Gd, Ed");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETED;
            emit_xor32(dyn, ninst, gd, ed, x3, x14);
            break;
        case 0x34:
            INST_NAME("XOR AL, Ib");
            SETFLAGS(X_ALL, SF_SET);
            u8 = F8;
            UXTB(x1, xEAX, 0);
            emit_xor8c(dyn, ninst, x1, u8, x3, x14);
            BFI(xEAX, x1, 0, 8);
            break;
        case 0x35:
            INST_NAME("XOR EAX, Id");
            SETFLAGS(X_ALL, SF_SET);
            i32 = F32S;
            emit_xor32c(dyn, ninst, xEAX, i32, x3, x14);
            break;
        case 0x36:
            INST_NAME("SS:");
            // ignored
            break;
        case 0x37:
            INST_NAME("AAA");
            READFLAGS(X_AF);
            SETFLAGS(X_AF|X_CF|X_PF|X_SF|X_ZF, SF_SET);
            UXTH(x1, xEAX, 0);
            CALL_(aaa16, x1, 0);
            BFI(xEAX, x1, 0, 16);
            break;
        case 0x38:
            INST_NAME("CMP Eb, Gb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x1);
            GETGB(x2);
            emit_cmp8(dyn, ninst, x1, x2, x3, x14);
            break;
        case 0x39:
            INST_NAME("CMP Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETEDH(x1);
            emit_cmp32(dyn, ninst, ed, gd, x3, x14);
            break;
        case 0x3A:
            INST_NAME("CMP Gb, Eb");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETEB(x2);
            GETGB(x1);
            emit_cmp8(dyn, ninst, x1, x2, x3, x14);
            break;
        case 0x3B:
            INST_NAME("CMP Gd, Ed");
            SETFLAGS(X_ALL, SF_SET);
            nextop = F8;
            GETGD;
            GETEDH(x2);
            emit_cmp32(dyn, ninst, gd, ed, x3, x14);
            break;
        case 0x3C:
            INST_NAME("CMP AL, Ib");
            SETFLAGS(X_ALL, SF_SET);
            u8 = F8;
            UXTB(x1, xEAX, 0);
            if(u8) {
                MOVW(x2, u8);
                emit_cmp8(dyn, ninst, x1, x2, x3, x14);
            } else {
                emit_cmp8_0(dyn, ninst, x1, x3, x14);
            }
            break;
        case 0x3D:
            INST_NAME("CMP EAX, Id");
            SETFLAGS(X_ALL, SF_SET);
            i32 = F32S;
            if(i32) {
                MOV32(x2, i32);
                emit_cmp32(dyn, ninst, xEAX, x2, x3, x14);
            } else {
                emit_cmp32_0(dyn, ninst, xEAX, x3, x14);
            }
            break;
        case 0x3E:
            INST_NAME("DS:");
            // ignored
            break;
        case 0x3F:
            INST_NAME("AAS");
            READFLAGS(X_AF);
            SETFLAGS(X_AF|X_CF|X_PF|X_SF|X_ZF, SF_SET);
            UXTH(x1, xEAX, 0);
            CALL_(aas16, x1, 0);
            BFI(xEAX, x1, 0, 16);
            break;
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
            INST_NAME("INC reg");
            SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
            gd = xEAX+(opcode&0x07);
            emit_inc32(dyn, ninst, gd, x3, x14);
            break;
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:
            INST_NAME("DEC reg");
            SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
            gd = xEAX+(opcode&0x07);
            emit_dec32(dyn, ninst, gd, x3, x14);
            break;
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
            INST_NAME("PUSH reg");
            gd = xEAX+(opcode&0x07);
            PUSH1(gd);
            break;
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
            INST_NAME("POP reg");
            gd = xEAX+(opcode&0x07);
            POP1(gd);
            break;
        case 0x60:
            INST_NAME("PUSHAD");
            MOV_REG(x1, xESP);
            // cannot use PUSH (STMdb!) because order of regs are reversed!
            //PUSH(x1, (1<<xEAX)|(1<<xECX)|(1<<xEDX)|(1<<xEBX)|(1<<xESP)|(1<<xEBP)|(1<<xESI)|(1<<xEDI));
            PUSH1(xEAX);
            PUSH1(xECX);
            PUSH1(xEDX);
            PUSH1(xEBX);
            PUSH1(x1);
            PUSH1(xEBP);
            PUSH1(xESI);
            PUSH1(xEDI);
            break;
        case 0x61:
            INST_NAME("POPAD");
            //MOV_REG(x1, xESP);
            //POP(x1, (1<<xEAX)|(1<<xECX)|(1<<xEDX)|(1<<xEBX)|(1<<xESP)|(1<<xEBP)|(1<<xESI)|(1<<xEDI));
            POP1(xEDI);
            POP1(xESI);
            POP1(xEBP);
            ADD_IMM8(xESP, xESP, 4);    //POP(xESP, (1<<xESP));
            POP1(xEBX);
            POP1(xEDX);
            POP1(xECX);
            POP1(xEAX);
            //MOV_REG(xESP, x1);
            break;
        case 0x62:
            INST_NAME("BOUND Gd, Ed");
            nextop = F8;
            GETED;
            // no bound test for now
            break;

        case 0x64:
            addr = dynarecFS(dyn, addr, ip, ninst, ok, need_epilog);
            break;
        case 0x65:
            addr = dynarecGS(dyn, addr, ip, ninst, ok, need_epilog);
            break;
        case 0x66:
            addr = dynarec66(dyn, addr, ip, ninst, ok, need_epilog);
            break;
        case 0x67:
            addr = dynarec67(dyn, addr, ip, ninst, ok, need_epilog);
            break;
        case 0x68:
            INST_NAME("PUSH Id");
            i32 = F32S;
            if(PK(0)==0xC3) {
                MESSAGE(LOG_DUMP, "PUSH then RET, using indirect\n");
                MOV32(x3, ip+1);
                LDR_IMM9(x1, x3, 0);
                PUSH1(x1);
            } else {
                MOV32(x3, i32);
                PUSH1(x3);
            }
            break;
        case 0x69:
            INST_NAME("IMUL Gd, Ed, Id");
            SETFLAGS(X_ALL, SF_PENDING);
            nextop = F8;
            GETGD;
            GETED;
            i32 = F32S;
            MOV32(x14, i32);
            UFLAG_IF {
                SMULL(x3, gd, x14, ed);
                UFLAG_OP1(x3);
                UFLAG_RES(gd);
                UFLAG_DF(x3, d_imul32);
            } else {
                MUL(gd, ed, x14);
            }
            break;
        case 0x6A:
            INST_NAME("PUSH Ib");
            i32 = F8S;
            MOV32(x3, i32);
            PUSH1(x3);
            break;
        case 0x6B:
            INST_NAME("IMUL Gd, Ed, Ib");
            SETFLAGS(X_ALL, SF_PENDING);
            nextop = F8;
            GETGD;
            GETED;
            i32 = F8S;
            MOV32(x14, i32);
            UFLAG_IF {
                SMULL(x3, gd, x14, ed);
                UFLAG_OP1(x3);
                UFLAG_RES(gd);
                UFLAG_DF(x3, d_imul32);
            } else {
                MUL(gd, ed, x14);
            }
            break;

        #define GO(GETFLAGS, NO, YES, F)    \
            READFLAGS(F);                   \
            i8 = F8S;   \
            BARRIER(2); \
            JUMP(addr+i8);\
            GETFLAGS;   \
            if(dyn->insts) {    \
                if(dyn->insts[ninst].x86.jmp_insts==-1) {   \
                    /* out of the block */                  \
                    i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8); \
                    Bcond(NO, i32);     \
                    jump_to_next(dyn, addr+i8, 0, ninst); \
                } else {    \
                    /* inside the block */  \
                    i32 = dyn->insts[dyn->insts[ninst].x86.jmp_insts].address-(dyn->arm_size+8);    \
                    Bcond(YES, i32);    \
                }   \
            }

        case 0x70:
            INST_NAME("JO ib");
            //F_OF is 1<<11, so 2048, so 0b10 ROL 10 so 0b01 ROR 11*2
            GO( TSTS_IMM8_ROR(xFlags, 0b10, 0x0b)
                , cEQ, cNE, X_OF)
            break;
        case 0x71:
            INST_NAME("JNO ib");
            GO( TSTS_IMM8_ROR(xFlags, 0b10, 0x0b)
                , cNE, cEQ, X_OF)
            break;
        case 0x72:
            INST_NAME("JC ib");
            GO( TSTS_IMM8(xFlags, 1<<F_CF)
                , cEQ, cNE, X_CF)
            break;
        case 0x73:
            INST_NAME("JNC ib");
            GO( TSTS_IMM8(xFlags, 1<<F_CF)
                , cNE, cEQ, X_CF)
            break;
        case 0x74:
            INST_NAME("JZ ib");
            GO( TSTS_IMM8(xFlags, 1<<F_ZF)
                , cEQ, cNE, X_ZF)
            break;
        case 0x75:
            INST_NAME("JNZ ib");
            GO( TSTS_IMM8(xFlags, 1<<F_ZF)
                , cNE, cEQ, X_ZF)
            break;
        case 0x76:
            INST_NAME("JBE ib");
            GO( TSTS_IMM8(xFlags, (1<<F_CF)|(1<<F_ZF))
                , cEQ, cNE, X_CF|X_ZF)
            break;
        case 0x77:
            INST_NAME("JNBE ib");
            GO( TSTS_IMM8(xFlags, (1<<F_CF)|(1<<F_ZF))
                , cNE, cEQ, X_CF|X_ZF)
            break;
        case 0x78:
            INST_NAME("JS ib");
            GO( TSTS_IMM8(xFlags, 1<<F_SF)
                , cEQ, cNE, X_SF)
            break;
        case 0x79:
            INST_NAME("JNS ib");
            GO( TSTS_IMM8(xFlags, 1<<F_SF)
                , cNE, cEQ, X_SF)
            break;
        case 0x7A:
            INST_NAME("JP ib");
            GO( TSTS_IMM8(xFlags, 1<<F_PF)
                , cEQ, cNE, X_PF)
            break;
        case 0x7B:
            INST_NAME("JNP ib");
            GO( TSTS_IMM8(xFlags, 1<<F_PF)
                , cNE, cEQ, X_PF)
            break;
        case 0x7C:
            INST_NAME("JL ib");
            GO( XOR_REG_LSL_IMM5(x1, xFlags, xFlags, F_OF-F_SF);
                TSTS_IMM8_ROR(x1, 0b10, 0x0b)
                , cEQ, cNE, X_SF|X_OF)
            break;
        case 0x7D:
            INST_NAME("JGE ib");
            GO( XOR_REG_LSL_IMM5(x1, xFlags, xFlags, F_OF-F_SF);
                TSTS_IMM8_ROR(x1, 0b10, 0x0b)
                , cNE, cEQ, X_SF|X_OF)
            break;
        case 0x7E:
            INST_NAME("JLE ib");
            GO( XOR_REG_LSL_IMM5(x1, xFlags, xFlags, F_OF-F_SF);
                ORR_REG_LSL_IMM5(x1, x1, xFlags, F_OF-F_ZF);
                TSTS_IMM8_ROR(x1, 0b10, 0x0b)
                , cEQ, cNE, X_SF|X_OF|X_ZF)
            break;
        case 0x7F:
            INST_NAME("JG ib");
            GO( XOR_REG_LSL_IMM5(x1, xFlags, xFlags, F_OF-F_SF);
                ORR_REG_LSL_IMM5(x1, x1, xFlags, F_OF-F_ZF);
                TSTS_IMM8_ROR(x1, 0b10, 0x0b)
                , cNE, cEQ, X_SF|X_OF|X_ZF)
            break;
        #undef GO
        case 0x80:
        case 0x82:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: //ADD
                    INST_NAME("ADD Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1);
                    u8 = F8;
                    emit_add8c(dyn, ninst, x1, u8, x2, x14);
                    EBBACK;
                    break;
                case 1: //OR
                    INST_NAME("OR Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1);
                    u8 = F8;
                    emit_or8c(dyn, ninst, x1, u8, x2, x14);
                    EBBACK;
                    break;
                case 2: //ADC
                    INST_NAME("ADC Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1);
                    u8 = F8;
                    emit_adc8c(dyn, ninst, x1, u8, x2, x14);
                    EBBACK;
                    break;
                case 3: //SBB
                    INST_NAME("SBB Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1);
                    u8 = F8;
                    emit_sbb8c(dyn, ninst, x1, u8, x2, x14);
                    EBBACK;
                    break;
                case 4: //AND
                    INST_NAME("AND Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1);
                    u8 = F8;
                    emit_and8c(dyn, ninst, x1, u8, x2, x14);
                    EBBACK;
                    break;
                case 5: //SUB
                    INST_NAME("SUB Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1);
                    u8 = F8;
                    emit_sub8c(dyn, ninst, x1, u8, x2, x14);
                    EBBACK;
                    break;
                case 6: //XOR
                    INST_NAME("XOR Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1);
                    u8 = F8;
                    emit_xor8c(dyn, ninst, x1, u8, x2, x14);
                    EBBACK;
                    break;
                case 7: //CMP
                    INST_NAME("CMP Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1);
                    u8 = F8;
                    if(u8) {
                        MOVW(x2, u8);
                        emit_cmp8(dyn, ninst, x1, x2, x3, x14);
                    } else {
                        emit_cmp8_0(dyn, ninst, x1, x3, x14);
                    }
                    break;
            }
            break;
        case 0x81:
        case 0x83:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: //ADD
                    if(opcode==0x81) {
                        INST_NAME("ADD Ed, Id");
                    } else {
                        INST_NAME("ADD Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET);
                    GETED;
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    emit_add32c(dyn, ninst, ed, i32, x3, x14);
                    WBACK;
                    break;
                case 1: //OR
                    if(opcode==0x81) {INST_NAME("OR Ed, Id");} else {INST_NAME("OR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET);
                    GETED;
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    emit_or32c(dyn, ninst, ed, i32, x3, x14);
                    WBACK;
                    break;
                case 2: //ADC
                    if(opcode==0x81) {INST_NAME("ADC Ed, Id");} else {INST_NAME("ADC Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET);
                    GETED;
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    emit_adc32c(dyn, ninst, ed, i32, x3, x14);
                    WBACK;
                    break;
                case 3: //SBB
                    if(opcode==0x81) {INST_NAME("SBB Ed, Id");} else {INST_NAME("SBB Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET);
                    GETED;
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    emit_sbb32c(dyn, ninst, ed, i32, x3, x14);
                    WBACK;
                    break;
                case 4: //AND
                    if(opcode==0x81) {INST_NAME("AND Ed, Id");} else {INST_NAME("AND Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET);
                    GETED;
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    emit_and32c(dyn, ninst, ed, i32, x3, x14);
                    WBACK;
                    break;
                case 5: //SUB
                    if(opcode==0x81) {INST_NAME("SUB Ed, Id");} else {INST_NAME("SUB Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET);
                    GETED;
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    emit_sub32c(dyn, ninst, ed, i32, x3, x14);
                    WBACK;
                    break;
                case 6: //XOR
                    if(opcode==0x81) {INST_NAME("XOR Ed, Id");} else {INST_NAME("XOR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET);
                    GETED;
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    emit_xor32c(dyn, ninst, ed, i32, x3, x14);
                    WBACK;
                    break;
                case 7: //CMP
                    if(opcode==0x81) {INST_NAME("CMP Ed, Id");} else {INST_NAME("CMP Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET);
                    GETEDH(x1);
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    if(i32) {
                        MOV32(x2, i32);
                        emit_cmp32(dyn, ninst, ed, x2, x3, x14);
                    } else {
                        emit_cmp32_0(dyn, ninst, ed, x3, x14);
                    }
                    break;
            }
            break;
        case 0x84:
            INST_NAME("TEST Eb, Gb");
            SETFLAGS(X_ALL, SF_SET);
            nextop=F8;
            GETEB(x1);
            GETGB(x2);
            emit_test8(dyn, ninst, x1, x2, x3, x14);
            break;
        case 0x85:
            INST_NAME("TEST Ed, Gd");
            SETFLAGS(X_ALL, SF_SET);
            nextop=F8;
            GETGD;
            GETEDH(x1);
            emit_test32(dyn, ninst, ed, gd, x3, x14);
            break;
        case 0x86:
            INST_NAME("(LOCK)XCHG Eb, Gb");
            // Do the swap
            nextop = F8;
            if((nextop&0xC0)==0xC0) {
                GETGB(x14);
                ed = (nextop&7);
                eb1 = xEAX+(ed&3);
                eb2 = ((ed&4)>>2);
                UXTB(x1, eb1, eb2);
                // do the swap 14 -> ed, 1 -> gd
                BFI(gb1, x1, gb2*8, 8);
                BFI(eb1, x14, eb2*8, 8);
            } else {
                GETGB(x14);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0);
                MARKLOCK;
                // do the swap with exclusive locking
                LDREXB(x1, ed);
                // do the swap 14 -> strb(ed), 1 -> gd
                STREXB(x3, x14, ed);
                CMPS_IMM8(x3, 0);
                B_MARKLOCK(cNE);
                BFI(gb1, x1, gb2*8, 8);
            }
            break;
        case 0x87:
            INST_NAME("(LOCK)XCHG Ed, Gd");
            nextop = F8;
            if((nextop&0xC0)==0xC0) {
                GETGD;
                GETED;
                if(gd!=ed) {
                    XOR_REG_LSL_IMM5(gd, gd, ed, 0);
                    XOR_REG_LSL_IMM5(ed, gd, ed, 0);
                    XOR_REG_LSL_IMM5(gd, gd, ed, 0);
                }
            } else {
                GETGD;
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0);
                TSTS_IMM8(ed, 3);
                B_MARK(cNE);
                MARKLOCK;
                LDREX(x1, ed);
                STREX(x3, gd, ed);
                CMPS_IMM8(x3, 0);
                B_MARKLOCK(cNE);
                B_MARK2(c__);
                MARK;
                LDR_IMM9(x1, ed, 0);
                STR_IMM9(gd, ed, 0);
                MARK2;
                MOV_REG(gd, x1);
            }
            break;
        case 0x88:
            INST_NAME("MOV Eb, Gb");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            gb2 = ((gd&4)>>2);
            gb1 = xEAX+(gd&3);
            if(gb2) {
                gd = x14;
                UXTB(gd, gb1, gb2);
            } else {
                gd = gb1;   // no need to extract
            }
            if((nextop&0xC0)==0xC0) {
                ed = (nextop&7);
                eb1 = xEAX+(ed&3);  // Ax, Cx, Dx or Bx
                eb2 = ((ed&4)>>2);    // L or H
                BFI(eb1, gd, eb2*8, 8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 4095, 0);
                STRB_IMM9(gd, ed, fixedaddress);
            }
            break;
        case 0x89:
            INST_NAME("MOV Ed, Gd");
            nextop=F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {   // reg <= reg
                MOV_REG(xEAX+(nextop&7), gd);
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 4095, 0);
                STR_IMM9(gd, ed, fixedaddress);
            }
            break;
        case 0x8A:
            INST_NAME("MOV Gb, Eb");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            gb1 = xEAX+(gd&3);
            gb2 = ((gd&4)>>2);
            if((nextop&0xC0)==0xC0) {
                    wback = (nextop&7);
                    wb2 = (wback>>2);
                    wback = xEAX+(wback&3);
                    if(wb2) {
                        UXTB(x14, wback, wb2);
                        ed = x14;
                    } else {
                        ed = wback;
                    }
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 4095, 0);
                    LDRB_IMM9(x14, wback, fixedaddress);
                    ed = x14;
                }
            BFI(gb1, ed, gb2*8, 8);
            break;
        case 0x8B:
            INST_NAME("MOV Gd, Ed");
            nextop=F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {   // reg <= reg
                MOV_REG(gd, xEAX+(nextop&7));
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 4095, 0);
                LDR_IMM9(gd, ed, fixedaddress);
            }
            break;
        case 0x8C:
            INST_NAME("MOV Ed, Seg");
            nextop=F8;
            MOV32(x3, offsetof(x86emu_t, segs[(nextop&0x38)>>3]));
            if((nextop&0xC0)==0xC0) {   // reg <= seg
                LDRH_REG(xEAX+(nextop&7), xEmu, x3);
            } else {                    // mem <= seg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0);
                LDRH_REG(x3, xEmu, x3);
                STRH_IMM8(x3, ed, fixedaddress);
            }
            break;
        case 0x8D:
            INST_NAME("LEA Gd, Ed");
            nextop=F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {   // reg <= reg? that's an invalid operation
                DEFAULT;
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, gd, &fixedaddress, 0, 0);
                if(gd!=ed) {    // it's sometimes used as a 3 bytes NOP
                    MOV_REG(gd, ed);
                }
            }
            break;
        case 0x8E:
            INST_NAME("MOV Seg,Ew");
            nextop = F8;
            if((nextop&0xC0)==0xC0) {
                ed = xEAX+(nextop&7);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 255, 0);
                LDRH_IMM8(x1, ed, fixedaddress);
                ed = x1;
            }
            MOV32(x2, offsetof(x86emu_t, segs[(nextop&0x38)>>3]));
            STRH_REG(ed, xEmu, x2);
            if(((nextop&0x38)>>3)==_FS) {
                // update default_fs, hack for wine
                MOV32(x2, &default_fs);
                STRH_IMM8(ed, x2, 0);
            }
            MOVW(x1, 0);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, segs_serial[(nextop&0x38)>>3]));
            break;
        case 0x8F:
            INST_NAME("POP Ed");
            nextop = F8;
            if((nextop&0xC0)==0xC0) {
                POP1(xEAX+(nextop&7));
            } else {
                POP1(x2); // so this can handle POP [ESP] and maybe some variant too
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0);
                if(ed==xESP) {
                    STR_IMM9(x2, ed, fixedaddress);
                } else {
                    // complicated to just allow a segfault that can be recovered correctly
                    SUB_IMM8(xESP, xESP, 4);
                    STR_IMM9(x2, ed, fixedaddress);
                    ADD_IMM8(xESP, xESP, 4);
                }
            }
            break;
        case 0x90:
            INST_NAME("NOP");
            break;
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:
            INST_NAME("XCHG EAX, Reg");
            gd = xEAX+(opcode&0x07);
            MOV_REG(x2, xEAX);
            MOV_REG(xEAX, gd);
            MOV_REG(gd, x2);
            break;
        case 0x98:
            INST_NAME("CWDE");
            SXTH(xEAX, xEAX, 0);
            break;
        case 0x99:
            INST_NAME("CDQ");
            MOV_REG_ASR_IMM5(xEDX, xEAX, 0);    // 0 in ASR means only bit #31 everywhere
            break;

        case 0x9B:
            INST_NAME("FWAIT");
            break;
        case 0x9C:
            INST_NAME("PUSHF");
            READFLAGS(X_ALL);
            PUSH1(xFlags);
            break;
        case 0x9D:
            INST_NAME("POPF");
            SETFLAGS(X_ALL, SF_SET);
            POP1(xFlags);
            MOV32(x1, 0x3F7FD7);
            AND_REG_LSL_IMM5(xFlags, xFlags, x1, 0);
            ORR_IMM8(xFlags, xFlags, 2, 0);
            SET_DFNONE(x1);
            break;
        case 0x9E:
            INST_NAME("SAHF");
            SETFLAGS(X_CF|X_PF|X_AF|X_ZF|X_SF, SF_SUBSET);
            BIC_IMM8(xFlags, xFlags, 0b11010101, 0);
            UXTB(x1, xEAX, 1);
            AND_IMM8(x1, x1, 0b11010101);
            ORR_REG_LSL_IMM5(xFlags, xFlags, x1, 0);
            SET_DFNONE(x1);	
            break;
        case 0x9F:
            INST_NAME("LAHF");
            READFLAGS(X_CF|X_PF|X_AF|X_ZF|X_SF);
            BFI(xEAX, xFlags, 8, 8);
            break;
        case 0xA0:
            INST_NAME("MOV AL, Ob");
            u32 = F32;
            MOV32(x2, u32);
            LDRB_IMM9(x2, x2, 0);
            BFI(xEAX, x2, 0, 8);
            break;
        case 0xA1:
            INST_NAME("MOV, EAX, Od");
            u32 = F32;
            MOV32(x2, u32);
            LDR_IMM9(xEAX, x2, 0);
            break;
        case 0xA2:
            INST_NAME("MOV Ob, AL");
            u32 = F32;
            MOV32(x2, u32);
            STRB_IMM9(xEAX, x2, 0);
            break;
        case 0xA3:
            INST_NAME("MOV Od, EAX");
            u32 = F32;
            MOV32(x2, u32);
            STR_IMM9(xEAX, x2, 0);
            break;
        case 0xA4:
            INST_NAME("MOVSB");
            GETDIR(x3, 1);
            LDRBAI_REG_LSL_IMM5(x1, xESI, x3, 0);
            STRBAI_REG_LSL_IMM5(x1, xEDI, x3, 0);
            break;
        case 0xA5:
            INST_NAME("MOVSD");
            GETDIR(x3, 4);
            LDRAI_REG_LSL_IMM5(x1, xESI, x3, 0);
            STRAI_REG_LSL_IMM5(x1, xEDI, x3, 0);
            break;
        case 0xA6:
            INST_NAME("CMPSB");
            SETFLAGS(X_ALL, SF_SET);
            GETDIR(x3, 1);
            LDRBAI_REG_LSL_IMM5(x1, xESI, x3, 0);
            LDRBAI_REG_LSL_IMM5(x2, xEDI, x3, 0);
            emit_cmp8(dyn, ninst, x1, x2, x3, x14);
            break;
        case 0xA7:
            INST_NAME("CMPSD");
            SETFLAGS(X_ALL, SF_SET);
            GETDIR(x3, 4);
            LDRAI_REG_LSL_IMM5(x1, xESI, x3, 0);
            LDRAI_REG_LSL_IMM5(x2, xEDI, x3, 0);
            emit_cmp32(dyn, ninst, x1, x2, x3, x14);
            break;
        case 0xA8:
            INST_NAME("TEST AL, Ib");
            SETFLAGS(X_ALL, SF_SET);
            UXTB(x1, xEAX, 0);
            u8 = F8;
            MOVW(x2, u8);
            emit_test8(dyn, ninst, x1, x2, x3, x14);
            break;
        case 0xA9:
            INST_NAME("TEST EAX, Id");
            SETFLAGS(X_ALL, SF_SET);
            i32 = F32S;
            MOV32(x2, i32);
            emit_test32(dyn, ninst, xEAX, x2, x3, x14);
            break;
        case 0xAA:
            INST_NAME("STOSB");
            GETDIR(x3, 1);
            STRBAI_REG_LSL_IMM5(xEAX, xEDI, x3, 0);
            break;
        case 0xAB:
            INST_NAME("STOSD");
            GETDIR(x3, 4);
            STRAI_REG_LSL_IMM5(xEAX, xEDI, x3, 0);
            break;
        case 0xAC:
            INST_NAME("LODSB");
            GETDIR(x3, 1);
            LDRBAI_REG_LSL_IMM5(x1, xESI, x3, 0);
            BFI(xEAX, x1, 0, 8);
            break;
        case 0xAD:
            INST_NAME("LODSD");
            GETDIR(x3, 4);
            LDRAI_REG_LSL_IMM5(xEAX, xESI, x3, 0);
            break;
        case 0xAE:
            INST_NAME("SCASB");
            SETFLAGS(X_ALL, SF_SET);
            GETDIR(x3, 1);
            UXTB(x1, xEAX, 0);
            LDRBAI_REG_LSL_IMM5(x2, xEDI, x3, 0);
            emit_cmp8(dyn, ninst, x1, x2, x3, x14);
            break;
        case 0xAF:
            INST_NAME("SCASD");
            SETFLAGS(X_ALL, SF_SET);
            GETDIR(x3, 4);
            LDRAI_REG_LSL_IMM5(x2, xEDI, x3, 0);
            emit_cmp32(dyn, ninst, xEAX, x2, x3, x14);
            break;
        case 0xB0:
        case 0xB1:
        case 0xB2:
        case 0xB3:
            INST_NAME("MOV xL, Ib");
            u8 = F8;
            MOVW(x1, u8);
            gb1 = xEAX+(opcode&3);
            BFI(gb1, x1, 0, 8);
            break;
        case 0xB4:
        case 0xB5:
        case 0xB6:
        case 0xB7:
            INST_NAME("MOV xH, Ib");
            u8 = F8;
            MOVW(x1, u8);
            gb1 = xEAX+(opcode&3);
            BFI(gb1, x1, 8, 8);
            break;
        case 0xB8:
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0xBD:
        case 0xBE:
        case 0xBF:
            INST_NAME("MOV Reg, Id");
            gd = xEAX+(opcode&7);
            i32 = F32S;
            MOV32(gd, i32);
            break;

        case 0xC0:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Eb, Ib");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETEB(x1);
                    u8 = F8;
                    MOVW(x2, u8);
                    CALL_(rol8, ed, (1<<x3));
                    EBBACK;
                    break;
                case 1:
                    INST_NAME("ROR Eb, Ib");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETEB(x1);
                    u8 = F8;
                    MOVW(x2, u8);
                    CALL_(ror8, ed, (1<<x3));
                    EBBACK;
                    break;
                case 2:
                    INST_NAME("RCL Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEB(x1);
                    u8 = F8;
                    MOVW(x2, u8);
                    CALL_(rcl8, ed, (1<<x3));
                    EBBACK;
                    break;
                case 3:
                    INST_NAME("RCR Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEB(x1);
                    u8 = F8;
                    MOVW(x2, u8);
                    CALL_(rcr8, ed, (1<<x3));
                    EBBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Eb, Ib");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEB(x1);
                    u8 = (F8)&0x1f;
                    UFLAG_IF{
                        MOV32(x14, u8); UFLAG_OP2(x14)
                    };
                    UFLAG_OP1(ed);
                    MOV_REG_LSL_IMM5(ed, ed, u8);
                    EBBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shl8);
                    break;
                case 5:
                    INST_NAME("SHR Eb, Ib");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEB(x1);
                    u8 = (F8)&0x1f;
                    UFLAG_IF{
                        MOV32(x14, u8); UFLAG_OP2(x14)
                    };
                    UFLAG_OP1(ed);
                    if(u8) {
                        MOV_REG_LSR_IMM5(ed, ed, u8);
                        EBBACK;
                    }
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shr8);
                    break;
                case 7:
                    INST_NAME("SAR Eb, Ib");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETSEB(x1);
                    u8 = (F8)&0x1f;
                    UFLAG_IF{
                        MOV32(x14, u8); UFLAG_OP2(x14)
                    };
                    UFLAG_OP1(ed);
                    if(u8) {
                        MOV_REG_ASR_IMM5(ed, ed, u8);
                        EBBACK;
                    }
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_sar8);
                    break;
            }
            break;
        case 0xC1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, Ib");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETED;
                    u8 = (F8)&0x1f;
                    emit_rol32c(dyn, ninst, ed, u8, x3, x14);
                    if(u8) { WBACK; }
                    break;
                case 1:
                    INST_NAME("ROR Ed, Ib");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETED;
                    u8 = (F8)&0x1f;
                    emit_ror32c(dyn, ninst, ed, u8, x3, x14);
                    if(u8) { WBACK; }
                    break;
                case 2:
                    INST_NAME("RCL Ed, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEDW(x14, x1);
                    u8 = F8;
                    MOVW(x2, u8);
                    CALL_(rcl32, ed, (1<<x14));
                    WBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ed, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    GETEDW(x14, x1);
                    u8 = F8;
                    MOVW(x2, u8);
                    CALL_(rcr32, ed, (1<<x14));
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET);    // some flags are left undefined
                    GETED;
                    u8 = (F8)&0x1f;
                    emit_shl32c(dyn, ninst, ed, u8, x3, x14);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET);    // some flags are left undefined
                    GETED;
                    u8 = (F8)&0x1f;
                    emit_shr32c(dyn, ninst, ed, u8, x3, x14);
                    if(u8) {
                        WBACK;
                    }
                    break;
                case 7:
                    INST_NAME("SAR Ed, Ib");
                    SETFLAGS(X_ALL, SF_SET);    // some flags are left undefined
                    GETED;
                    u8 = (F8)&0x1f;
                    emit_sar32c(dyn, ninst, ed, u8, x3, x14);
                    if(u8) {
                        WBACK;
                    }
                    break;
                default:
                    INST_NAME("GRP3 Ed, Ib");
                    DEFAULT;
            }
            break;

        case 0xC2:
            INST_NAME("RETN");
            //SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            READFLAGS(X_PEND);  // lets play safe here too
            BARRIER(2);
            i32 = F16;
            retn_to_epilog(dyn, ninst, i32);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xC3:
            INST_NAME("RET");
            // SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            // ^^^ that hack break PlantsVsZombies and GOG Setup under wine....
            READFLAGS(X_PEND);  // so instead, force the defered flags, so it's not too slow, and flags are not lost
            BARRIER(2);
            ret_to_epilog(dyn, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;

        case 0xC6:
            INST_NAME("MOV Eb, Ib");
            nextop=F8;
            if((nextop&0xC0)==0xC0) {   // reg <= u8
                u8 = F8;
                ed = (nextop&7);
                eb1 = xEAX+(ed&3);  // Ax, Cx, Dx or Bx
                eb2 = (ed&4)>>2;    // L or H
                MOVW(x3, u8);
                BFI(eb1, x3, eb2*8, 8);
            } else {                    // mem <= u8
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0);
                u8 = F8;
                MOVW(x3, u8);
                STRB_IMM9(x3, ed, fixedaddress);
            }
            break;
        case 0xC7:
            INST_NAME("MOV Ed, Id");
            nextop=F8;
            if((nextop&0xC0)==0xC0) {   // reg <= i32
                i32 = F32S;
                ed = xEAX+(nextop&7);
                MOV32(ed, i32);
            } else {                    // mem <= i32
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 4095, 0);
                i32 = F32S;
                MOV32(x3, i32);
                STR_IMM9(x3, ed, fixedaddress);
            }
            break;

        case 0xC9:
            INST_NAME("LEAVE");
            MOV_REG(xESP, xEBP);
            POP1(xEBP);
            break;

        case 0xCC:
            SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            if(PK(0)=='S' && PK(1)=='C') {
                addr+=2;
                BARRIER(2);
                INST_NAME("Special Box86 instruction");
                if((PK(0)==0) && (PK(1)==0) && (PK(2)==0) && (PK(3)==0))
                {
                    addr+=4;
                    MESSAGE(LOG_DEBUG, "Exit x86 Emu\n");
                    MOV32(x14, ip+1+2);
                    STM(xEmu, (1<<xEAX)|(1<<xEBX)|(1<<xECX)|(1<<xEDX)|(1<<xESI)|(1<<xEDI)|(1<<xESP)|(1<<xEBP)|(1<<xEIP)|(1<<xFlags));
                    MOVW(x1, 1);
                    STR_IMM9(x1, xEmu, offsetof(x86emu_t, quit));
                    *ok = 0;
                    *need_epilog = 1;
                } else {
                    MESSAGE(LOG_DUMP, "Native Call to %s\n", GetNativeName(GetNativeFnc(ip)));
                    x87_forget(dyn, ninst, x3, x14, 0);
                    MOV32(xEIP, ip+1); // read the 0xCC
                    addr+=4+4;
                    STM(xEmu, (1<<xEAX)|(1<<xEBX)|(1<<xECX)|(1<<xEDX)|(1<<xESI)|(1<<xEDI)|(1<<xESP)|(1<<xEBP)|(1<<xEIP)|(1<<xFlags));
                    CALL_S(x86Int3, -1, 0);
                    LDM(xEmu, (1<<xEAX)|(1<<xEBX)|(1<<xECX)|(1<<xEDX)|(1<<xESI)|(1<<xEDI)|(1<<xESP)|(1<<xEBP)|(1<<xEIP)|(1<<xFlags));
                    MOV32(x3, ip+1+2+4+4); // expected return address
                    CMPS_REG_LSL_IMM5(xEIP, x3, 0);
                    B_MARK(cNE);
                    LDR_IMM9(x1, xEmu, offsetof(x86emu_t, quit));
                    CMPS_IMM8(x1, 1);
                    B_NEXT(cNE);
                    MARK;
                    jump_to_epilog(dyn, 0, xEIP, ninst);
                }
            } else {
                INST_NAME("INT 3");
                // check if TRAP signal is handled
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, context));
                MOV32(x2, offsetof(box86context_t, signals[SIGTRAP]));
                LDR_REG_LSL_IMM5(x3, x1, x2, 0);
                CMPS_IMM8(x3, 0);
                B_NEXT(cNE);
                MOV32(x1, SIGTRAP);
                CALL_(raise, -1, 0);
                break;
            }
            break;
        case 0xCD:
            SETFLAGS(X_ALL, SF_SET);    // Hack, set all flags (to an unknown state...)
            if(PK(0)==0x80) {
                INST_NAME("Syscall");
                u8 = F8;
                BARRIER(2);
                MOV32(xEIP, ip+2);
                STM(xEmu, (1<<xEAX)|(1<<xEBX)|(1<<xECX)|(1<<xEDX)|(1<<xESI)|(1<<xEDI)|(1<<xESP)|(1<<xEBP)|(1<<xEIP)|(1<<xFlags));
                CALL_S(x86Syscall, -1, 0);
                LDM(xEmu, (1<<xEAX)|(1<<xEBX)|(1<<xECX)|(1<<xEDX)|(1<<xESI)|(1<<xEDI)|(1<<xESP)|(1<<xEBP)|(1<<xEIP)|(1<<xFlags));
                MOVW(x2, addr);
                CMPS_REG_LSL_IMM5(x14, x2, 0);
                B_MARK(cNE);    // jump to epilog, if IP is not what is expected
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, quit));
                CMPS_IMM8(x1, 1);
                B_NEXT(cNE);
                MARK;
                jump_to_epilog(dyn, 0, xEIP, ninst);
            } else {
                INST_NAME("INT Ib");
                DEFAULT;
            }
            break;

        case 0xCF:
            INST_NAME("IRET");
            SETFLAGS(X_ALL, SF_SET);    // Not a hack, EFLAGS are restored
            BARRIER(2);
            iret_to_epilog(dyn, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0xD0:
        case 0xD2:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    if(opcode==0xD0) {
                        INST_NAME("ROL Eb, 1");
                        MOVW(x2, 1);
                    } else {
                        INST_NAME("ROL Eb, CL");
                        AND_IMM8(x2, xECX, 0x1f);
                    }
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETEB(x1);
                    CALL_(rol8, x1, (1<<x3));
                    EBBACK;
                    break;
                case 1:
                    if(opcode==0xD0) {
                        INST_NAME("ROR Eb, 1");
                        MOVW(x2, 1);
                    } else {
                        INST_NAME("ROR Eb, CL");
                        AND_IMM8(x2, xECX, 0x1f);
                    }
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETEB(x1);
                    CALL_(ror8, x1, (1<<x3));
                    EBBACK;
                    break;
                case 2:
                    if(opcode==0xD0) {INST_NAME("RCL Eb, 1");} else {INST_NAME("RCL Eb, CL");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    if(opcode==0xD0) {MOVW(x2, 1);} else {AND_IMM8(x2, xECX, 0x1f);}
                    GETEB(x1);
                    CALL_(rcl8, x1, (1<<x3));
                    EBBACK;
                    break;
                case 3:
                    if(opcode==0xD0) {INST_NAME("RCR Eb, 1");} else {INST_NAME("RCR Eb, CL");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    if(opcode==0xD0) {MOVW(x2, 1);} else {AND_IMM8(x2, xECX, 0x1f);}
                    GETEB(x1);
                    CALL_(rcr8, x1, (1<<x3));
                    EBBACK;
                    break;
                case 4:
                case 6:
                    if(opcode==0xD0) {
                        INST_NAME("SHL Eb, 1");
                        MOVW(x2, 1);
                    } else {
                        INST_NAME("SHL Eb, CL");
                        AND_IMM8(x2, xECX, 0x1f);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEB(x1);
                    UFLAG_OP12(ed, x2)
                    MOV_REG_LSL_REG(ed, ed, x2);
                    EBBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shl8);
                    break;
                case 5:
                    if(opcode==0xD0) {
                        INST_NAME("SHR Eb, 1");
                        MOVW(x2, 1);
                    } else {
                        INST_NAME("SHR Eb, CL");
                        AND_IMM8(x2, xECX, 0x1f);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEB(x1);
                    UFLAG_OP12(ed, x2)
                    MOV_REG_LSR_REG(ed, ed, x2);
                    EBBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_shr8);
                    break;
                case 7:
                    if(opcode==0xD0) {
                        INST_NAME("SAR Eb, 1");
                        MOVW(x2, 1);
                    } else {
                        INST_NAME("SAR Eb, CL");
                        AND_IMM8(x2, xECX, 0x1f);
                    }
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETSEB(x1);
                    UFLAG_OP12(ed, x2)
                    MOV_REG_ASR_REG(ed, ed, x2);
                    EBBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_sar8);
                    break;
            }
            break;
        case 0xD1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETED;
                    emit_rol32c(dyn, ninst, ed, 1, x3, x14);
                    WBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ed, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    GETED;
                    emit_ror32c(dyn, ninst, ed, 1, x3, x14);
                    WBACK;
                    break;
                case 2:
                    INST_NAME("RCL Ed, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    MOVW(x2, 1);
                    GETEDW(x14, x1);
                    CALL_(rcl32, ed, (1<<x14));
                    WBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ed, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET);
                    MOVW(x2, 1);
                    GETEDW(x14, x1);
                    CALL_(rcr32, ed, (1<<x14));
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, 1");
                    SETFLAGS(X_ALL, SF_SET);    // some flags are left undefined
                    GETED;
                    emit_shl32c(dyn, ninst, ed, 1, x3, x14);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET);    // some flags are left undefined
                    GETED;
                    emit_shr32c(dyn, ninst, ed, 1, x3, x14);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET);    // some flags are left undefined
                    GETED;
                    emit_sar32c(dyn, ninst, ed, 1, x3, x14);
                    WBACK;
                    break;
            }
            break;
        case 0xD3:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    AND_IMM8(x3, xECX, 0x1f);
                    TSTS_REG_LSL_IMM5(x3, x3, 0);
                    B_MARK2(cEQ);
                    RSB_IMM8(x3, x3, 0x20);
                    GETEDW(x14, x2);
                    MOV_REG_ROR_REG(ed, ed, x3);
                    WBACK;
                    UFLAG_IF {  // calculate flags directly
                        CMPS_IMM8(x3, 31);
                        B_MARK(cNE);
                            MOV_REG_LSR_IMM5(x1, ed, 31);
                            ADD_REG_LSL_IMM5(x1, x1, ed, 0);
                            BFI(xFlags, x1, F_OF, 1);
                        MARK;
                        BFI(xFlags, ed, F_CF, 1);
                        UFLAG_DF(x2, d_none);
                    }
                    MARK2;
                    break;
                case 1:
                    INST_NAME("ROR Ed, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    AND_IMM8(x3, xECX, 0x1f);
                    TSTS_REG_LSL_IMM5(x3, x3, 0);
                    B_MARK2(cEQ);
                    GETEDW(x14, x2);
                    MOV_REG_ROR_REG(ed, ed, x3);
                    WBACK;
                    UFLAG_IF {  // calculate flags directly
                        CMPS_IMM8(x3, 1);
                        B_MARK(cNE);
                            MOV_REG_LSR_IMM5(x2, ed, 30); // x2 = d>>30
                            XOR_REG_LSR_IMM8(x2, x2, x2, 1); // x2 = ((d>>30) ^ ((d>>30)>>1))
                            BFI(xFlags, x2, F_OF, 1);
                        MARK;
                        MOV_REG_LSR_IMM5(x2, ed, 31);
                        BFI(xFlags, x2, F_CF, 1);
                        UFLAG_DF(x2, d_none);
                    }
                    MARK2;
                    break;
                case 2:
                    INST_NAME("RCL Ed, CL");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    AND_IMM8(x2, xECX, 0x1f);
                    GETEDW(x14, x1);
                    CALL_(rcl32, ed, (1<<x14));
                    WBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ed, CL");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    AND_IMM8(x2, xECX, 0x1f);
                    GETEDW(x14, x1);
                    CALL_(rcr32, ed, (1<<x14));
                    WBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, CL");
                    SETFLAGS(X_ALL, SF_SET);    // some flags are left undefined
                    AND_IMM8(x3, xECX, 0x1f);
                    GETED;
                    emit_shl32(dyn, ninst, ed, x3, x3, x14);
                    WBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ed, CL");
                    SETFLAGS(X_ALL, SF_SET);    // some flags are left undefined
                    AND_IMM8(x3, xECX, 0x1f);
                    GETED;
                    emit_shr32(dyn, ninst, ed, x3, x3, x14);
                    WBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ed, CL");
                    SETFLAGS(X_ALL, SF_PENDING);
                    AND_IMM8(x3, xECX, 0x1f);
                    GETED;
                    UFLAG_OP12(ed, x3);
                    MOV_REG_ASR_REG(ed, ed, x3);
                    WBACK;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_sar32);
                    break;
            }
            break;
        case 0xD4:
            INST_NAME("AAM Ib");
            SETFLAGS(X_ALL, SF_SET);
            UBFX(x1, xEAX, 0, 8);    // load AL
            u8 = F8;
            MOVW(x2, u8);
            CALL_(aam16, x1, 0);
            BFI(xEAX, x1, 0, 16);
            break;
        case 0xD5:
            INST_NAME("AAD Ib");
            SETFLAGS(X_ALL, SF_SET);
            UBFX(x1, xEAX, 0, 16);    // load AX
            u8 = F8;
            MOVW(x2, u8);
            CALL_(aad16, x1, 0);
            BFI(xEAX, x1, 0, 16);
            break;
        case 0xD6:
            INST_NAME("SALC");
            READFLAGS(X_CF);
            TSTS_IMM8(xFlags, 1<<F_CF);
            MOVW_COND(cEQ, x3, 0x00);
            MOVW_COND(cNE, x3, 0xFF);
            BFI(xEAX, x3, 0, 8);
            break;
        case 0xD7:
            INST_NAME("XLAT");
            UBFX(x1, xEAX, 0, 8);    // x1 = AL
            LDRB_REG_LSL_IMM5(x1, xEBX, x1, 0); //x1 = byte ptr[EBX+AL]
            BFI(xEAX, x1, 0, 8);
            break;
        case 0xD8:
            addr = dynarecD8(dyn, addr, ip, ninst, ok, need_epilog);
            break;
        case 0xD9:
            addr = dynarecD9(dyn, addr, ip, ninst, ok, need_epilog);
            break;
        case 0xDA:
            addr = dynarecDA(dyn, addr, ip, ninst, ok, need_epilog);
            break;
        case 0xDB:
            addr = dynarecDB(dyn, addr, ip, ninst, ok, need_epilog);
            break;
        case 0xDC:
            addr = dynarecDC(dyn, addr, ip, ninst, ok, need_epilog);
            break;
        case 0xDD:
            addr = dynarecDD(dyn, addr, ip, ninst, ok, need_epilog);
            break;
        case 0xDE:
            addr = dynarecDE(dyn, addr, ip, ninst, ok, need_epilog);
            break;
        case 0xDF:
            addr = dynarecDF(dyn, addr, ip, ninst, ok, need_epilog);
            break;
        #define GO(NO, YES)   \
            BARRIER(2); \
            JUMP(addr+i8);\
            if(dyn->insts) {    \
                if(dyn->insts[ninst].x86.jmp_insts==-1) {   \
                    /* out of the block */                  \
                    i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8); \
                    Bcond(NO, i32);     \
                    jump_to_next(dyn, addr+i8, 0, ninst); \
                } else {    \
                    /* inside the block */  \
                    i32 = dyn->insts[dyn->insts[ninst].x86.jmp_insts].address-(dyn->arm_size+8);    \
                    Bcond(YES, i32);    \
                }   \
            }
        case 0xE0:
            INST_NAME("LOOPNZ");
            READFLAGS(X_ZF);
            i8 = F8S;
            SUBS_IMM8(xECX, xECX, 1);
            B_NEXT(cEQ);    // CX is 0, no LOOP
            TSTS_IMM8(xFlags, 1<<F_ZF);
            GO(cNE, cEQ);
            break;
        case 0xE1:
            INST_NAME("LOOPZ");
            READFLAGS(X_ZF);
            i8 = F8S;
            SUBS_IMM8(xECX, xECX, 1);
            B_NEXT(cEQ);    // CX is 0, no LOOP
            TSTS_IMM8(xFlags, 1<<F_ZF);
            GO(cEQ, cNE);
            break;
        case 0xE2:
            INST_NAME("LOOP");
            i8 = F8S;
            SUBS_IMM8(xECX, xECX, 1);
            GO(cEQ, cNE);
            break;
        case 0xE3:
            INST_NAME("JECXZ");
            i8 = F8S;
            TSTS_REG_LSL_IMM5(xECX, xECX, 0);
            GO(cNE, cEQ);
            break;
        #undef GO

        case 0xE8:
            INST_NAME("CALL Id");
            i32 = F32S;
            if(addr+i32==0) {
                #if STEP == 3
                printf_log(LOG_INFO, "Warning, CALL to 0x0 at %p (%p)\n", (void*)addr, (void*)(addr-1));
                #endif
            }
            #if STEP == 0
            if ((i32==0) && ((PK(0)>=0x58) && (PK(0)<=0x5F)))
                tmp = 1;
            else if (((addr+i32)>0x10000) && (PK(i32+0)==0x8B) && (((PK(i32+1))&0xC7)==0x04) && (PK(i32+2)==0x24) && (PK(i32+3)==0xC3))
                tmp = 2;
            else if(isNativeCall(dyn, addr+i32, NULL, NULL))
                tmp = 3;
            else 
                tmp = 0;
            #elif STEP < 2
            if ((i32==0) && ((PK(0)>=0x58) && (PK(0)<=0x5F)))
                tmp = dyn->insts[ninst].pass2choice = 1;
            else if (((addr+i32)>0x10000) && (PK(i32+0)==0x8B) && (((PK(i32+1))&0xC7)==0x04) && (PK(i32+2)==0x24) && (PK(i32+3)==0xC3))
                tmp = dyn->insts[ninst].pass2choice = 2;
            else if(isNativeCall(dyn, addr+i32, &dyn->insts[ninst].natcall, &dyn->insts[ninst].retn))
                tmp = dyn->insts[ninst].pass2choice = 3;
            else 
                tmp = dyn->insts[ninst].pass2choice = 0;
            #else
                tmp = dyn->insts[ninst].pass2choice;
            #endif
            switch(tmp) {
                case 1:
                    //SETFLAGS(X_ALL, SF_SET);    // Hack to set flags to "dont'care" state
                    MESSAGE(LOG_DUMP, "Hack for Call 0, Pop reg\n");
                    u8 = F8;
                    gd = xEAX+(u8&7);
                    MOV32(gd, addr-1);
                    break;
                case 2:
                    SETFLAGS(X_ALL, SF_SET);    // Hack to set flags to "dont'care" state
                    MESSAGE(LOG_DUMP, "Hack for Call x86.get_pc_thunk.reg\n");
                    u8 = PK(i32+1);
                    gd = xEAX+((u8&0x38)>>3);
                    MOV32(gd, addr);
                    break;
                case 3:
                    SETFLAGS(X_ALL, SF_SET);    // Hack to set flags to "dont'care" state
                    BARRIER(1);
                    BARRIER_NEXT(1);
                    MOV32(x2, addr);
                    PUSH1(x2);
                    MESSAGE(LOG_DUMP, "Native Call to %s (retn=%d)\n", GetNativeName(GetNativeFnc(dyn->insts[ninst].natcall-1)), dyn->insts[ninst].retn);
                    // calling a native function
                    x87_forget(dyn, ninst, x3, x14, 0);
                    MOV32(x14, dyn->insts[ninst].natcall); // read the 0xCC already
                    STM(xEmu, (1<<xEAX)|(1<<xEBX)|(1<<xECX)|(1<<xEDX)|(1<<xESI)|(1<<xEDI)|(1<<xESP)|(1<<xEBP)|(1<<xEIP)|(1<<xFlags));
                    CALL_S(x86Int3, -1, 0);
                    LDM(xEmu, (1<<xEAX)|(1<<xEBX)|(1<<xECX)|(1<<xEDX)|(1<<xESI)|(1<<xEDI)|(1<<xESP)|(1<<xEBP)|(1<<xEIP)|(1<<xFlags));
                    MOV32(x3, dyn->insts[ninst].natcall+2+4+4);
                    CMPS_REG_LSL_IMM5(xEIP, x3, 0);
                    B_MARK(cNE);    // Not the expected address, exit dynarec block
                    POP1(xEIP);   // pop the return address
                    if(dyn->insts[ninst].retn) {
                        ADD_IMM8(xESP, xESP, dyn->insts[ninst].retn);
                    }
                    MOV32(x3, addr);
                    CMPS_REG_LSL_IMM5(xEIP, x3, 0);
                    B_MARK(cNE);    // Not the expected address again
                    LDR_IMM9(x1, xEmu, offsetof(x86emu_t, quit));
                    CMPS_IMM8(x1, 1);
                    B_NEXT(cNE);    // not quitting, so lets continue
                    MARK;
                    jump_to_epilog(dyn, 0, xEIP, ninst);
                    break;
                default:
                    if(ninst && dyn->insts && dyn->insts[ninst-1].x86.set_flags) {
                        READFLAGS(X_PEND);  // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET);    // Hack to set flags to "dont'care" state
                    }
                    // regular call
                    BARRIER(1);
                    BARRIER_NEXT(1);
                    if(!dyn->insts || ninst==dyn->size-1) {
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    MOV32(x2, addr);
                    PUSH1(x2);
                    if(addr+i32==0) {   // self modifying code maybe? so use indirect address fetching
                        MOV32(x14, addr-4);
                        LDR_IMM9(x14, x14, 0);
                        jump_to_next(dyn, 0, x14, ninst);
                    } else
                        jump_to_next(dyn, addr+i32, 0, ninst);
                    break;
            }
            break;
        case 0xE9:
        case 0xEB:
            BARRIER(1);
            if(opcode==0xE9) {
                INST_NAME("JMP Id");
                i32 = F32S;
            } else {
                INST_NAME("JMP Ib");
                i32 = F8S;
            }
            JUMP(addr+i32);
            if(dyn->insts) {
                PASS2IF(dyn->insts[ninst].x86.jmp_insts==-1, 1) {
                    // out of the block
                    jump_to_next(dyn, addr+i32, 0, ninst);
                } else {
                    // inside the block
                    tmp = dyn->insts[dyn->insts[ninst].x86.jmp_insts].address-(dyn->arm_size+8);
                    if(tmp==-4) {
                        NOP;
                    } else {
                        Bcond(c__, tmp);
                    }
                }
            }
            *need_epilog = 0;
            *ok = 0;
            break;
        
        case 0xEE:
            INST_NAME("OUT dx, al");
            //ignored!
            break;

        case 0xF0:
            addr = dynarecF0(dyn, addr, ip, ninst, ok, need_epilog);
            break;

        case 0xF2:                      /* REPNZ prefix */
        case 0xF3:                      /* REPZ prefix */
            nextop = F8;
            if(nextop==0x0F) {
                if(opcode==0xF3) {
                    addr = dynarecF30F(dyn, addr, ip, ninst, ok, need_epilog);
                } else {
                    addr = dynarecF20F(dyn, addr, ip, ninst, ok, need_epilog);
                }
            } else if(nextop==0x66) {
                nextop = F8;
                switch(nextop) {
                    case 0xA5:
                        INST_NAME("REP MOVSW");
                        TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                        B_NEXT(cEQ);    // end of loop
                        GETDIR(x3, 2);
                        MARK;
                        LDRHAI_REG_LSL_IMM5(x1, xESI, x3);
                        STRHAI_REG_LSL_IMM5(x1, xEDI, x3);
                        SUBS_IMM8(xECX, xECX, 1);
                        B_MARK(cNE);
                        break;
                    case 0xAB:
                        INST_NAME("REP STOSW");
                        TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                        B_NEXT(cEQ);    // end of loop
                        GETDIR(x3, 2);
                        MARK;
                        STRHAI_REG_LSL_IMM5(xEAX, xEDI, x3);
                        SUBS_IMM8(xECX, xECX, 1);
                        B_MARK(cNE);
                        break;
                    case 0xAF:
                        if(opcode==0xF2) {INST_NAME("REPNZ SCASW");} else {INST_NAME("REPZ SCASW");}
                        SETFLAGS(X_ALL, SF_MAYSET);
                        TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                        B_NEXT(cEQ);    // end of loop
                        GETDIR(x3, 2);
                        UXTH(x1, xEAX, 0); // get lower 16bits
                        MARK;
                        LDRHAI_REG_LSL_IMM5(x2, xEDI, x3);
                        CMPS_REG_LSL_IMM5(x1, x2, 0);
                        if(opcode==0xF2) {
                            B_MARK2(cEQ);
                        } else {
                            B_MARK2(cNE);
                        }
                        SUBS_IMM8(xECX, xECX, 1);
                        B_MARK(cNE);
                        B_MARK3(c__);
                        // done, finish with cmp test
                        MARK2;
                        SUB_IMM8(xECX, xECX, 1);
                        MARK3;
                        emit_cmp16(dyn, ninst, x1, x2, x3, x14);
                        break;
                    default:
                        INST_NAME("F2/F3 66 ...");
                        DEFAULT;
                }
            } else {
                // DF=0, increment addresses, DF=1 decrement addresses
                switch(nextop) {
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
                    case 0x9C:
                    case 0xE8:
                    case 0xE9:
                    case 0xEB:
                        if(opcode==0xF2) {INST_NAME("BND");} else {INST_NAME("F3");}
                        --addr; // put back opcode
                        break;
                    case 0x90:
                        INST_NAME("PAUSE");
                        break;
                    case 0xC3:
                        INST_NAME("(REPZ) RET");
                        SETFLAGS(X_ALL, SF_SET);    // Hack to set flags to "dont'care" state
                        BARRIER(2);
                        ret_to_epilog(dyn, ninst);
                        *need_epilog = 0;
                        *ok = 0;
                        break;
                    case 0xA4:
                        INST_NAME("REP MOVSB");
                        TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                        B_NEXT(cEQ);    // end of loop
                        GETDIR(x3,1);
                        MARK;
                        LDRBAI_REG_LSL_IMM5(x1, xESI, x3, 0);
                        STRBAI_REG_LSL_IMM5(x1, xEDI, x3, 0);
                        SUBS_IMM8(xECX, xECX, 1);
                        B_MARK(cNE);
                        // done
                        break;
                    case 0xA5:
                        INST_NAME("REP MOVSD");
                        TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                        B_NEXT(cEQ);    // end of loop
                        GETDIR(x3,4);
                        MARK;
                        LDRAI_REG_LSL_IMM5(x1, xESI, x3, 0);
                        STRAI_REG_LSL_IMM5(x1, xEDI, x3, 0);
                        SUBS_IMM8(xECX, xECX, 1);
                        B_MARK(cNE);
                        // done
                        break;
                    case 0xA6:
                        if(opcode==0xF2) {INST_NAME("REPNZ CMPSB");} else {INST_NAME("REPZ CMPSB");}
                        SETFLAGS(X_ALL, SF_MAYSET);
                        TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                        B_NEXT(cEQ);    // end of loop
                        GETDIR(x3,1);
                        MARK;
                        LDRBAI_REG_LSL_IMM5(x1, xESI, x3, 0);
                        LDRBAI_REG_LSL_IMM5(x2, xEDI, x3, 0);
                        CMPS_REG_LSL_IMM5(x1, x2, 0);
                        if(opcode==0xF2) {
                            B_MARK2(cEQ);
                        } else {
                            B_MARK2(cNE);
                        }
                        SUBS_IMM8(xECX, xECX, 1);
                        B_MARK(cNE);
                        B_MARK3(c__);   // go past sub ecx, 1
                        // done, finish with cmp test
                        MARK2;
                        SUB_IMM8(xECX, xECX, 1);
                        MARK3;
                        emit_cmp8(dyn, ninst, x1, x2, x3, x14);
                        break;
                    case 0xA7:
                        if(opcode==0xF2) {INST_NAME("REPNZ CMPSD");} else {INST_NAME("REPZ CMPSD");}
                        SETFLAGS(X_ALL, SF_MAYSET);
                        TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                        B_NEXT(cEQ);    // end of loop
                        GETDIR(x3,4);
                        MARK;
                        LDRAI_REG_LSL_IMM5(x1, xESI, x3, 0);
                        LDRAI_REG_LSL_IMM5(x2, xEDI, x3, 0);
                        CMPS_REG_LSL_IMM5(x1, x2, 0);
                        if(opcode==0xF2) {
                            B_MARK2(cEQ);
                        } else {
                            B_MARK2(cNE);
                        }
                        SUBS_IMM8(xECX, xECX, 1);
                        B_MARK(cNE);
                        B_MARK3(c__);   // go past sub ecx, 1
                        // done, finish with cmp test
                        MARK2;
                        SUB_IMM8(xECX, xECX, 1);
                        MARK3;
                        emit_cmp32(dyn, ninst, x1, x2, x3, x14);
                        break;
                    case 0xAA:
                        INST_NAME("REP STOSB");
                        TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                        B_NEXT(cEQ);    // end of loop
                        GETDIR(x3,1);
                        MARK;
                        STRBAI_REG_LSL_IMM5(xEAX, xEDI, x3, 0);
                        SUBS_IMM8(xECX, xECX, 1);
                        B_MARK(cNE);
                        break;
                    case 0xAB:
                        INST_NAME("REP STOSD");
                        TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                        B_NEXT(cEQ);    // end of loop
                        GETDIR(x3,4);
                        MARK;
                        STRAI_REG_LSL_IMM5(xEAX, xEDI, x3, 0);
                        SUBS_IMM8(xECX, xECX, 1);
                        B_MARK(cNE);
                        break;
                    case 0xAC:
                        INST_NAME("REP LODSB");
                        TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                        B_NEXT(cEQ);    // end of loop
                        GETDIR(x3,1);
                        MARK;
                        LDRBAI_REG_LSL_IMM5(x1, xESI, x3, 0);
                        SUBS_IMM8(xECX, xECX, 1);
                        B_MARK(cNE);
                        BFI(xEAX, x1, 0, 8);
                        break;
                    case 0xAD:
                        INST_NAME("REP LODSD");
                        TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                        B_NEXT(cEQ);    // end of loop
                        GETDIR(x3,4);
                        MARK;
                        LDRAI_REG_LSL_IMM5(xEAX, xESI, x3, 0);
                        SUBS_IMM8(xECX, xECX, 1);
                        B_MARK(cNE);
                        break;
                    case 0xAE:
                        if(opcode==0xF2) {INST_NAME("REPNZ SCASB");} else {INST_NAME("REPZ SCASB");}
                        SETFLAGS(X_ALL, SF_MAYSET);
                        TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                        B_NEXT(cEQ);    // end of loop
                        GETDIR(x3,1);
                        UXTB(x1, xEAX, 0);
                        MARK;
                        LDRBAI_REG_LSL_IMM5(x2, xEDI, x3, 0);
                        CMPS_REG_LSL_IMM5(x1, x2, 0);
                        if(opcode==0xF2) {
                            B_MARK2(cEQ);
                        } else {
                            B_MARK2(cNE);
                        }
                        SUBS_IMM8(xECX, xECX, 1);
                        B_MARK(cNE);
                        B_MARK3(c__);
                        // done, finish with cmp test
                        MARK2;
                        SUB_IMM8(xECX, xECX, 1);
                        MARK3;
                        emit_cmp8(dyn, ninst, x1, x2, x3, x14);
                        break;
                    case 0xAF:
                        if(opcode==0xF2) {INST_NAME("REPNZ SCASD");} else {INST_NAME("REPZ SCASD");}
                        SETFLAGS(X_ALL, SF_MAYSET);
                        TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                        B_NEXT(cEQ);    // end of loop
                        GETDIR(x3,4);
                        MARK;
                        LDRAI_REG_LSL_IMM5(x2, xEDI, x3, 0);
                        CMPS_REG_LSL_IMM5(xEAX, x2, 0);
                        if(opcode==0xF2) {
                            B_MARK2(cEQ);
                        } else {
                            B_MARK2(cNE);
                        }
                        SUBS_IMM8(xECX, xECX, 1);
                        B_MARK(cNE);
                        B_MARK3(c__);
                        // done, finish with cmp test
                        MARK2;
                        SUB_IMM8(xECX, xECX, 1);
                        MARK3;
                        emit_cmp32(dyn, ninst, xEAX, x2, x3, x14);
                        break;
                    default:
                        INST_NAME("F2/F3 ...");
                        DEFAULT;
                }
            }
            break;
            
        case 0xF5:
            INST_NAME("CMC");
            READFLAGS(X_CF);
            SETFLAGS(X_CF, SF_SET);
            XOR_IMM8(xFlags, xFlags, 1<<F_CF);
            break;
        case 0xF6:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("TEST Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1);
                    u8 = F8;
                    MOVW(x2, u8);
                    emit_test8(dyn, ninst, x1, x2, x3, x14);
                    break;
                case 2:
                    INST_NAME("NOT Eb");
                    GETEB(x1);
                    MVN_REG_LSL_IMM5(x1, x1, 0);
                    EBBACK;
                    break;
                case 3:
                    INST_NAME("NEG Eb");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEB(x1);
                    emit_neg8(dyn, ninst, x1, x2, x14);
                    EBBACK;
                    break;
                case 4:
                    INST_NAME("MUL AL, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEB(x1);
                    STM(xEmu, (1<<xEAX));
                    CALL(mul8, -1, 0);
                    LDM(xEmu, (1<<xEAX));
                    break;
                case 5:
                    INST_NAME("IMUL AL, Eb");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEB(x1);
                    STM(xEmu, (1<<xEAX));
                    CALL(imul8, -1, 0);
                    LDM(xEmu, (1<<xEAX));
                    break;
                case 6:
                    INST_NAME("DIV Eb");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1);
                    STM(xEmu, (1<<xEAX));
                    CALL(div8, -1, 0);
                    LDM(xEmu, (1<<xEAX));
                    break;
                case 7:
                    INST_NAME("IDIV Eb");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEB(x1);
                    STM(xEmu, (1<<xEAX));
                    CALL(idiv8, -1, 0);
                    LDM(xEmu, (1<<xEAX));
                    break;
            }
            break;
        case 0xF7:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("TEST Ed, Id");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEDH(x1);
                    i32 = F32S;
                    MOV32(x2, i32);
                    emit_test32(dyn, ninst, ed, x2, x3, x14);
                    break;
                case 2:
                    INST_NAME("NOT Ed");
                    GETED;
                    MVN_REG_LSL_IMM5(ed, ed, 0);
                    WBACK;
                    break;
                case 3:
                    INST_NAME("NEG Ed");
                    SETFLAGS(X_ALL, SF_SET);
                    GETED;
                    emit_neg32(dyn, ninst, ed, x3, x14);
                    WBACK;
                    break;
                case 4:
                    INST_NAME("MUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x2, d_mul32);
                    GETED;
                    UMULL(xEDX, xEAX, xEAX, ed);
                    UFLAG_RES(xEAX);
                    UFLAG_OP1(xEDX);
                    break;
                case 5:
                    INST_NAME("IMUL EAX, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    UFLAG_DF(x2, d_imul32);
                    GETED;
                    SMULL(xEDX, xEAX, xEAX, ed);
                    UFLAG_RES(xEAX);
                    UFLAG_OP1(xEDX);
                    break;
                case 6:
                    INST_NAME("DIV Ed");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEDH(x1);
                    if(ed!=x1) {MOV_REG(x1, ed);}
                    STM(xEmu, (1<<xEAX) | (1<<xECX) | (1<<xEDX));
                    CALL(div32, -1, 0);
                    LDM(xEmu, (1<<xEAX) | (1<<xECX) | (1<<xEDX));
                    break;
                case 7:
                    INST_NAME("IDIV Ed");
                    SETFLAGS(X_ALL, SF_SET);
                    GETEDH(x1);
                    if(ed!=x1) {MOV_REG(x1, ed);}
                    STM(xEmu, (1<<xEAX) | (1<<xECX) | (1<<xEDX));
                    CALL(idiv32, -1, 0);
                    LDM(xEmu, (1<<xEAX) | (1<<xECX) | (1<<xEDX));
                    break;
            }
            break;
        case 0xF8:
            INST_NAME("CLC");
            i32 = dyn->state_flags; //TODO: find a better way to do this
            SETFLAGS(X_CF, SF_SUBSET);
            if(i32!=SF_SET) {
                SET_DFNONE(x1);
            }
            BFC(xFlags, F_CF, 1);
            break;
        case 0xF9:
            INST_NAME("STC");
            i32 = dyn->state_flags; //TODO: find a better way to do this
            SETFLAGS(X_CF, SF_SUBSET);
            if(i32!=SF_SET) {
                SET_DFNONE(x1);
            }
            ORRS_IMM8(xFlags, xFlags, 1, 0);
            break;
        case 0xFA:
            INST_NAME("CLI");
            BFC(xFlags, F_IF, 1);
            break;
        case 0xFB:
            INST_NAME("STI");
            MOVW(x1, 1);
            BFI(xFlags, x1, F_IF, 1);
            break;
        case 0xFC:
            INST_NAME("CLD");
            BFC(xFlags, F_DF, 1);
            break;
        case 0xFD:
            INST_NAME("STD");
            MOVW(x1, 1);
            BFI(xFlags, x1, F_DF, 1);
            break;

        case 0xFE:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("INC Eb");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                    GETEB(x1);
                    emit_inc8(dyn, ninst, x1, x2, x14);
                    EBBACK;
                    break;
                case 1:
                    INST_NAME("DEC Eb");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                    GETEB(x1);
                    emit_dec8(dyn, ninst, x1, x2, x14);
                    EBBACK;
                    break;
                default:
                    INST_NAME("Grp5 Ed");
                    DEFAULT;
            }
            break;
        case 0xFF:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: // INC Ed
                    INST_NAME("INC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                    GETED;
                    emit_inc32(dyn, ninst, ed, x3, x14);
                    WBACK;
                    break;
                case 1: //DEC Ed
                    INST_NAME("DEC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET);
                    GETED;
                    emit_dec32(dyn, ninst, ed, x3, x14);
                    WBACK;
                    break;
                case 2: // CALL Ed
                    INST_NAME("CALL Ed");
                    PASS2IF(ninst && dyn->insts && dyn->insts[ninst-1].x86.set_flags, 1) {
                        READFLAGS(X_PEND);          // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET);    //Hack to put flag in "don't care" state
                    }
                    GETEDH(xEIP);
                    BARRIER(1);
                    BARRIER_NEXT(1);
                    if(!dyn->insts || ninst==dyn->size-1) {
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    MOV32(x2, addr);
                    PUSH1(x2);
                    jump_to_next(dyn, 0, ed, ninst);
                    break;
                case 4: // JMP Ed
                    INST_NAME("JMP Ed");
                    BARRIER(1);
                    GETEDH(xEIP);
                    jump_to_next(dyn, 0, ed, ninst);
                    *need_epilog = 0;
                    *ok = 0;
                    break;
                case 6: // Push Ed
                    INST_NAME("PUSH Ed");
                    GETED;
                    PUSH1(ed);
                    break;

                default:
                    INST_NAME("Grp5 Ed");
                    DEFAULT;
            }
            break;

        default:
            DEFAULT;
    }
 
     return addr;
}

