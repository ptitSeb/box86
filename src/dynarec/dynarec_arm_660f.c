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

// Get EX as a quad
#define GETEX(a)                \
    if((nextop&0xC0)==0xC0) {   \
        a = sse_get_reg(dyn, ninst, x1, nextop&7);  \
    } else {                    \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress); \
        VLD1Q_64(q0, ed);       \
    }
#define GETGX(a)    \
    gd = (nextop&0x38)>>3;  \
    a = sse_get_reg(dyn, ninst, x1, gd)

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

        case 0x2E:
            // no special check...
        case 0x2F:
            if(opcode==0x2F) {INST_NAME("COMISD Gx, Ex");} else {INST_NAME("UCOMISD Gx, Ex");}
            UFLAGS(0);
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            GETEX(q0);
            VCMP_F64(v0, q0);
            FCOMI(x1, x2);
            UFLAGS(1);
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

        case 0x54:
            INST_NAME("ANDPD Gx, Ex");
            nextop = F8;
            GETEX(q0);
            GETGX(v0);
            VANDQ(v0, v0, q0);
            break;
        case 0x55:
            INST_NAME("ANDNPD Gx, Ex");
            nextop = F8;
            GETEX(q0);
            GETGX(v0);
            VBICQ(v0, q0, v0);
            break;
        case 0x56:
            INST_NAME("ORPD Gx, Ex");
            nextop = F8;
            GETEX(q0);
            GETGX(v0);
            VORRQ(v0, v0, q0);
            break;
        case 0x57:
            INST_NAME("XORPD Gx, Ex");
            nextop = F8;
            GETEX(q0);
            GETGX(v0);
            VEORQ(v0, v0, q0);
            break;
        case 0x58:
            INST_NAME("ADDPD Gx, Ex");
            nextop = F8;
            GETEX(q0);
            GETGX(v0);
            VADD_F64(v0, v0, q0);
            VADD_F64(v0+1, v0+1, q0+1);
            break;
        case 0x59:
            INST_NAME("MULPD Gx, Ex");
            nextop = F8;
            GETEX(q0);
            GETGX(v0);
            VMUL_F64(v0, v0, q0);
            VMUL_F64(v0+1, v0+1, q0+1);
            break;

        case 0x62:
            INST_NAME("PUNPCKLDQ Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            GETEX(q0);
            if((nextop&0xC0)==0xC0) {
                q1 = fpu_get_scratch_quad(dyn);
                VMOVQ(q1, q0);
            } else q1 = q0;
            VZIPQ_32(v0, q1);
            break;

        case 0x66:
            INST_NAME("PCMPGTD Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(v1);
            VCGTQ_S32(v0, v0, v1);
            break;

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
                // use stack as temporary storage
                SUB_IMM8(xSP, xSP, 4);
                if(v1==v0) {
                    q0 = fpu_get_scratch_quad(dyn);
                    VMOVQ(q0, v1);
                } else q0 = v1;
                if (u8) {
                    for (int i=0; i<4; ++i) {
                        int32_t idx = (u8>>(i*2))&3;
                        if(idx!=i32) {
                            VST1LANE_32(q0+(idx/2), xSP, idx&1);
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

        case 0x72:  /* GRP */
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLD Ex, Ib");
                    if((nextop&0xC0)==0xC0) {
                        q0 = sse_get_reg(dyn, ninst, x1, nextop&7);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                        q0 = fpu_get_scratch_quad(dyn);
                        VLD1Q_32(q0, ed);
                    }
                    u8 = F8;
                    if (u8>31) {
                        VEORQ(q0, q0, q0);
                    } else {
                        VSHRQ_U32(q0, q0, u8);
                    }
                    if((nextop&0xC0)!=0xC0) {
                        VST1Q_32(q0, ed);
                    }
                    break;
                case 4:
                    INST_NAME("PSRAD Ex, Ib");
                    if((nextop&0xC0)==0xC0) {
                        q0 = sse_get_reg(dyn, ninst, x1, nextop&7);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                        q0 = fpu_get_scratch_quad(dyn);
                        VLD1Q_32(q0, ed);
                    }
                    u8 = F8;
                    VSHRQ_S32(q0, q0, u8&31);
                    if((nextop&0xC0)!=0xC0) {
                        VST1Q_32(q0, ed);
                    }
                    break;
                case 6:
                    INST_NAME("PSLLD Ex, Ib");
                    if((nextop&0xC0)==0xC0) {
                        q0 = sse_get_reg(dyn, ninst, x1, nextop&7);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                        q0 = fpu_get_scratch_quad(dyn);
                        VLD1Q_32(q0, ed);
                    }
                    u8 = F8;
                    if (u8>31) {
                        VEORQ(q0, q0, q0);
                    } else {
                        VSHLQ_32(q0, q0, u8);
                    }
                    if((nextop&0xC0)!=0xC0) {
                        VST1Q_32(q0, ed);
                    }
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
            break;

        case 0x76:
            INST_NAME("PCMPEQD Gx,Ex");
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
            VCEQQ_32(v0, v0, q0);
            break;

        case 0x7E:
            INST_NAME("MOVD Ed,Gx");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            d0 = fpu_get_scratch_double(dyn);
            VMOV_64(d0, v0);
            if((nextop&0xC0)==0xC0) {
                ed = xEAX + (nextop&7);
                VMOVfrV(ed, d0*2);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                VSTR_32(d0*2, ed, 0);
            }
            break;
        case 0x7F:
            INST_NAME("MOVDQA Ex,Gx");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            if((nextop&0xC0)==0xC0) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7);
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                VST1Q_32(v0, ed);
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
        
        case 0xC4:
            INST_NAME("PINSRW Gx,Ew,Ib");
            nextop = F8;
            GETGX(v0);
            if((nextop&0xC0)==0xC0) {
                wback = xEAX+(nextop&7);
                PUSH(xSP, wback);
                wback = xSP;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress);
                wb1=1;
            }
            u8 = (F8)&7;
            VST1LANE_16(v0+(u8/4), wback, u8&3);
            if(wback==xSP) {
                ADD_IMM8(xSP, xSP, 4);
            }
            break;

        case 0xC6:
            INST_NAME("SHUFPD Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0);
            GETEX(v1);
            u8 = F8;
            if(v0==v1 && u8==0) {
                VMOV_64(q0+1, q0);
            } else {
                q0 = fpu_get_scratch_quad(dyn);
                VMOV_64(q0, v0+(u8&1));
                VMOV_64(q0+1, v1+((u8>>1)&1));
                VMOVQ(v0, q0);
            }
            break;

        case 0xD4:
            INST_NAME("PADDQ Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0);
            VADDQ_64(v0, v0, q0);
            break;

        case 0xDB:
            INST_NAME("PAND Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0);
            VANDQ(v0, v0, q0);
            break;

        case 0xDF:
            INST_NAME("PANDN Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0);
            VBICQ(v0, q0, v0);
            break;

        case 0xEB:
            INST_NAME("POR Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0);
            VORRQ(v0, q0, v0);
            break;

        case 0xEF:
            INST_NAME("PXOR Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(q0);
            VEORQ(v0, v0, q0);
            break;

        case 0xF4:
            INST_NAME("PMULUDQ Gx,Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(v1);
            q0 = fpu_get_scratch_quad(dyn);
            VMOVQ(q0, v0);
            VTRN_32(q0, q0+1);  // transpose GX
            if((nextop&0xC0)==0xC0) {
                q1 = fpu_get_scratch_quad(dyn);
                VMOVQ(q1, v1);
            } else {
                q1 = v1;
            }
            VTRN_32(q1, q1+1);  // transpose EX
            VMULL_S64_S32(v0, q0, q1);
            break;

        case 0xF9:
            INST_NAME("PSUBW Gx,Ex");
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
            VSUBQ_16(v0, v0, q0);
            break;
        case 0xFA:
            INST_NAME("PSUBD Gx,Ex");
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
            VSUBQ_32(v0, v0, q0);
            break;
        case 0xFB:
            INST_NAME("PSUBQ Gx,Ex");
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
            VSUBQ_64(v0, v0, q0);
            break;
        case 0xFC:
            INST_NAME("PADDB Gx,Ex");
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
            VADDQ_8(v0, v0, q0);
            break;
        case 0xFD:
            INST_NAME("PADDW Gx,Ex");
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
            VADDQ_16(v0, v0, q0);
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

