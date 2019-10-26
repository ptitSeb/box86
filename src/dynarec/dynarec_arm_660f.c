#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>

#include "debug.h"
#include "box86context.h"
#include "dynarec.h"
#include "emu/x86emu_private.h"
#include "emu/x86run_private.h"
#include "x86run.h"
#include "x86emu.h"
#include "box86stack.h"
#include "callback.h"
#include "emu/x86run_private.h"
#include "x86trace.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"
#include "arm_printer.h"

#include "dynarec_arm_functions.h"
#include "dynarec_arm_helper.h"

uintptr_t dynarec660F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    int32_t i32, i32_;
    int16_t i16;
    uint16_t u16;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    uint8_t eb1, eb2;
    int v0, v1;
    int q0, q1;
    int d0, d1;
    int s0, s1;
    int fixedaddress;
    switch(opcode) {

        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;

        #define GO(GETFLAGS, NO, YES)   \
            USEFLAG(1); \
            GETFLAGS;   \
            nextop=F8;  \
            GETGD;      \
            if((nextop&0xC0)==0xC0) {   \
                ed = xEAX+(nextop&7);   \
            } else { \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress);    \
                LDRH_IMM8_COND(YES, x1, ed, 0); \
                ed = x1;                        \
            }   \
            BFI_COND(YES, gd, ed, 0 ,16);

        case 0x40:
            INST_NAME("CMOVO Gw, Ew");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_OF]));
                CMPS_IMM8(x2, 1)
                , cNE, cEQ)
            break;
        case 0x41:
            INST_NAME("CMOVNO Gw, Ew");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_OF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x42:
            INST_NAME("CMOVC Gw, Ew");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                CMPS_IMM8(x2, 1)
                , cNE, cEQ)
            break;
        case 0x43:
            INST_NAME("CMOVNC Gw, Ew");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                CMPS_IMM8(2, 1)
                , cEQ, cNE)
            break;
        case 0x44:
            INST_NAME("CMOVZ Gw, Ew");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                CMPS_IMM8(x2, 1)
                , cNE, cEQ)
            break;
        case 0x45:
            INST_NAME("CMOVNZ Gw, Ew");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x46:
            INST_NAME("CMOVBE Gw, Ew");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                ORRS_REG_LSL_IMM8(x2, x2, x3, 0);
                , cEQ, cNE)
            break;
        case 0x47:
            INST_NAME("CMOVNBE Gw, Ew");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                ORRS_REG_LSL_IMM8(x2, x2, x3, 0);
                , cNE, cEQ)
            break;
        case 0x48:
            INST_NAME("CMOVS Gw, Ew");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                CMPS_IMM8(x2, x1)
                , cNE, cEQ)
            break;
        case 0x49:
            INST_NAME("CMOVNS Gw, Ew");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x4A:
            INST_NAME("CMOVP Gw, Ew");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_PF]));
                CMPS_IMM8(x2, x1)
                , cNE, cEQ)
            break;
        case 0x4B:
            INST_NAME("CMOVNP Gw, Ew");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_PF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x4C:
            INST_NAME("CMOVL Gw, Ew");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                CMPS_REG_LSL_IMM8(x1, x2, 0)
                , cEQ, cNE)
            break;
        case 0x4D:
            INST_NAME("CMOVGE Gw, Ew");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                CMPS_REG_LSL_IMM8(x1, x2, 0)
                , cNE, cEQ)
            break;
        case 0x4E:
            INST_NAME("CMOVLE Gw, Ew");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                XOR_REG_LSL_IMM8(x1, x1, x2, 0);
                LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                ORRS_REG_LSL_IMM8(x2, x1, x2, 0);
                , cEQ, cNE)
            break;
        case 0x4F:
            INST_NAME("CMOVG Gw, Ew");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                XOR_REG_LSL_IMM8(x1, x1, x2, 0);
                LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                ORRS_REG_LSL_IMM8(x2, x1, x2, 0);
                , cNE, cEQ)
            break;
        #undef GO

        case 0x6C:
            INST_NAME("PUNPCKLQDQ Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            if((nextop&0xC0)==0xC0) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7);
                VMOV_64(v0+1, v1);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                VLDR_64(v0+1, ed, 0);
            }
            break;

        case 0x6E:
            INST_NAME("MOVD Gx, Ed");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            d0 = fpu_get_scratch_double(dyn);
            VEOR(d0, d0, d0); // d0 = U32{0, 0}
            GETED;
            VMOVtoV(d0*2, ed);// d0 = U32{ed, 0}
            VMOVL_U32(v0, d0);// U32/U32 -> U64/U64
            break;
        case 0x6F:
            INST_NAME("MOVDQA Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if((nextop&0xC0)==0xC0) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                VLD1Q_64(v0, ed);
            }
            break;
        case 0x70:
            INST_NAME("PSHUFD Gx,Ex,Ib");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            i32 = -1;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            if((nextop&0xC0)==0xC0) {
                u8 = F8;
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7);
                // use stack as tmporary storage
                SUB_IMM8(xSP, xSP, 4);
                if (u8) {
                    for (int i=0; i<4; ++i) {
                        int32_t idx = (u8>>(i*2))&3;
                        if(idx!=i32) {
                            VST1LANE_32(v1+(idx/2), xSP, idx&1);
                            i32 = idx;
                        }
                        VLD1LANE_32(v0+(i/2), xSP, i&1);
                    }
                } else {
                    VST1LANE_32(v1, xSP, 0);
                    VLD1QALL_32(v0, xSP);
                }
                ADD_IMM8(xSP, xSP, 4);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                u8 = F8;
                if (u8) {
                    for (int i=0; i<4; ++i) {
                        int32_t idx = (u8>>(i*2))&3;
                        if(idx!=i32) {
                            ADD_IMM8(x2, ed, idx*4);
                            i32 = idx;
                        }
                        VLD1LANE_32(v0+(i/2), x2, i&1);
                    }
                } else {
                    VLD1QALL_32(v0, ed);
                }
            }
            break;

        case 0xA3:
            INST_NAME("BT Ew, Gw");
            nextop = F8;
            USEFLAG(1);
            GETGD;  // there is an AND below, so 32bits is the same (no need for GETGW)
            GETEW(x1);
            AND_IMM8(x2, gd, 15);
            MOV_REG_LSR_REG(x1, ed, x2);
            AND_IMM8(x1, x1, 1);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_CF]));
            break;
        case 0xA4:
        case 0xA5:
            nextop = F8;
            if(opcode==0xA4) {
                INST_NAME("SHLD Ew, Gw, Ib");
            } else {
                INST_NAME("SHLD Ew, Gw, CL");
                UXTB(x3, xECX, 0);
            }
            GETEWW(x12, x1);
            GETGW(x2);
            if(opcode==0xA4) {
                u8 = F8;
                MOVW(x3, u8);
            }
            CALL(shld16, x1, (1<<wback));
            EWBACKW(x1);
            UFLAGS(1);
            break;

        case 0xAB:
            INST_NAME("BTS Ew, Gw");
            nextop = F8;
            USEFLAG(1);
            GETGD;  // there is an AND below, to 32bits is the same
            GETEW(x12);
            AND_IMM8(x2, gd, 15);
            MOV_REG_LSR_REG(x1, ed, x2);
            ANDS_IMM8(x1, x1, 1);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_CF]));
            B_NEXT(cNE);
            MOVW(x1, 1);
            ORR_REG_LSL_REG(ed, ed, x1, x2);
            EWBACK;
            break;
        case 0xAC:
        case 0xAD:
            nextop = F8;
            if(opcode==0xAC) {
                INST_NAME("SHRD Ew, Gw, Ib");
            } else {
                INST_NAME("SHRD Ew, Gw, CL");
                UXTB(x3, xECX, 0);
            }
            GETEWW(x12, x1);
            GETGW(x2);
            if(opcode==0xAC) {
                u8 = F8;
                MOVW(x3, u8);
            }
            CALL(shrd16, x1, (1<<wback));
            EWBACKW(x1);
            UFLAGS(1);
            break;

        case 0xB3:
            INST_NAME("BTR Ew, Gw");
            nextop = F8;
            USEFLAG(1);
            GETGD;  // there is an AND below, to 32bits is the same
            GETEW(x12);
            AND_IMM8(x2, gd, 15);
            MOV_REG_LSR_REG(x1, ed, x2);
            ANDS_IMM8(x1, x1, 1);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_CF]));
            B_NEXT(cEQ);
            MOVW(x1, 1);
            XOR_REG_LSL_REG(ed, ed, x1, x2);
            EWBACK;
            break;

        case 0xB6:
            INST_NAME("MOVZX Gw, Eb");
            nextop = F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = (nextop&7);
                eb1 = xEAX+(ed&3);  // Ax, Cx, Dx or Bx
                eb2 = (ed&4)>>2;    // L or H
                UXTB(x1, eb1, eb2);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress);
                LDRB_IMM9(x1, ed, 0);
            }
            BFI(gd, x1, 0, 16);
            break;

        case 0xBB:
            INST_NAME("BTC Ew, Gw");
            nextop = F8;
            USEFLAG(1);
            GETGD;  // there is an AND below, to 32bits is the same
            GETEW(x12);
            AND_IMM8(x2, gd, 15);
            MOV_REG_LSR_REG(x1, ed, x2);
            AND_IMM8(x1, x1, 1);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_CF]));
            MOVW(x1, 1);
            XOR_REG_LSL_REG(ed, ed, x1, x2);
            EWBACK;
            break;

        case 0xBE:
            INST_NAME("MOVSX Gw, Eb");
            nextop = F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = (nextop&7);
                eb1 = xEAX+(ed&3);  // Ax, Cx, Dx or Bx
                eb2 = (ed&4)>>2;    // L or H
                SXTB(x1, eb1, eb2);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress);
                LDRSB_IMM8(x1, ed, 0);
            }
            BFI(gd, x1, 0, 16);
            break;

        case 0xEF:
            INST_NAME("PXOR Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            if((nextop&0xC0)==0xC0) {
                q0 = sse_get_reg(dyn, ninst, x1, nextop&7);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                q0 = fpu_get_scratch_quad(dyn);
                VLD1Q_64(q0, ed);
            }
            VEORQ(v0, v0, q0);
            break;

        case 0xFE:
            INST_NAME("PADDD Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            if((nextop&0xC0)==0xC0) {
                q0 = sse_get_reg(dyn, ninst, x1, nextop&7);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                q0 = fpu_get_scratch_quad(dyn);
                VLD1Q_64(q0, ed);
            }
            VADDQ_32(v0, v0, q0);
            break;

        default:
            *ok = 0;
            DEFAULT;
    }
    return addr;
}

