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

// Get Ex as a single, not a quad, with b a intermediary double
#define GETEX(a, b) \
    if((nextop&0xC0)==0xC0) { \
        b = sse_get_reg(dyn, ninst, x1, nextop&7);  \
        if(b<16)                                    \
            a = b;                                  \
        else {                                      \
            a = fpu_get_scratch_double(dyn);        \
            VMOV_64(a, b);                          \
        }                                           \
        a *= 2;                                     \
    } else {    \
        b = fpu_get_scratch_double(dyn);            \
        a = b*2;                                    \
        parity = getedparity(dyn, ninst, addr, nextop, 2);  \
        if(parity) {                                \
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 0); \
            VLDR_32(a, ed, fixedaddress);           \
        } else {                                    \
            GETED;                                  \
            VMOVtoV(a, ed);                         \
        }                                           \
    }

#define GETGX(a) \
    gd = (nextop&0x38)>>3;                          \
    a = sse_get_reg(dyn, ninst, x1, gd)


uintptr_t dynarecF30F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    int32_t j32;
    uint32_t u32;
    uint8_t gd, ed;
    uint8_t wback;
    int fixedaddress;
    int v0, v1;
    int s0, s1, s2;
    int d0, d1;
    int q0, q1;
    int parity;

    MAYUSE(q1);
    MAYUSE(s1);
    MAYUSE(s2);
    MAYUSE(j32);

    switch(opcode) {

        case 0x10:
            INST_NAME("MOVSS Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if((nextop&0xC0)==0xC0) {
                v0 = sse_get_reg(dyn, ninst, x1, gd);
                q0 = sse_get_reg(dyn, ninst, x1, nextop&7);
                if(v0<16)
                    d0 = v0;
                else {
                    d0 = fpu_get_scratch_double(dyn);
                    VMOVD(d0, v0);
                }
                if(q0<16)
                    d1 = q0;
                else {
                    d1 = fpu_get_scratch_double(dyn);
                    VMOVD(d1, q0);
                }
                VMOV_32(d0*2, d1*2);
                if(v0!=d0) {
                    VMOVD(v0, d0);
                }
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0);
                LDR_IMM9(x2, ed, fixedaddress);   // to avoid bus errors
                VEORQ(v0, v0, v0);
                VMOVtoDx_32(v0, 0, x2);
            }
            break;
        case 0x11:
            INST_NAME("MOVSS Ex, Gx");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            if((nextop&0xC0)==0xC0) {
                q0 = sse_get_reg(dyn, ninst, x1, nextop&7);
                if(v0<16)
                    d0 = v0;
                else {
                    d0 = fpu_get_scratch_double(dyn);
                    VMOVD(d0, v0);
                }
                if(q0<16)
                    d1 = q0;
                else {
                    d1 = fpu_get_scratch_double(dyn);
                    VMOVD(d1, q0);
                }
                VMOV_32(d1*2, d0*2);
                if(q0!=d1) {
                    VMOVD(q0, d1);
                }
            } else {
                VMOVfrDx_32(x2, v0, 0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0);
                STR_IMM9(x2, ed, fixedaddress);
            }
            break;
        case 0x12:
            INST_NAME("MOVSLDUP Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if((nextop&0xC0)==0xC0) {
                q1 = sse_get_reg(dyn, ninst, x1, nextop&7);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0);
                q1 = fpu_get_scratch_quad(dyn);
                VLD1Q_64(q1, ed);
            }
            q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            VDUP_32(q0+0, q1+0, 0);
            VDUP_32(q0+1, q1+1, 0);
            break;
        
        case 0x16:
            INST_NAME("MOVSHDUP Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if((nextop&0xC0)==0xC0) {
                q1 = sse_get_reg(dyn, ninst, x1, nextop&7);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0);
                q1 = fpu_get_scratch_quad(dyn);
                VLD1Q_64(q1, ed);
            }
            q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            VDUP_32(q0+0, q1+0, 1);
            VDUP_32(q0+1, q1+1, 1);
            break;

        
        case 0x1E:
            INST_NAME("NOP / ENDBR32 / ENDBR64");
            nextop = F8;
            FAKEED;
            break;
        case 0x1F:
            INST_NAME("REP NOP");
            nextop = F8;
            FAKEED;
            break;

        case 0x2A:
            INST_NAME("CVTSI2SS Gx, Ed");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            GETED;
            if(v0<16)
                d0 = v0;
            else {
                d0 = fpu_get_scratch_double(dyn);
                VMOV_64(d0, v0);
            }
            s0 = d0*2;
            VMOVtoV(s0, ed);
            VCVT_F32_S32(s0, s0);
            if(d0!=v0) {
                VMOV_64(v0, d0);
            }
            break;

        case 0x2C:
            INST_NAME("CVTTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            s0 = fpu_get_scratch_single(dyn);
            if((nextop&0xC0)==0xC0) {
                v0 = sse_get_reg(dyn, ninst, x1, nextop&7);
                if(v0<16)
                    d0 = v0;
                else {
                    d0 = fpu_get_scratch_double(dyn);
                    VMOV_64(d0, v0);
                }
                VMOV_32(s0, d0*2);
            } else {
                parity = getedparity(dyn, ninst, addr, nextop, 3);
                if(parity) {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3);
                    VLDR_32(s0, ed, fixedaddress);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0);
                    LDR_IMM9(x2, ed, fixedaddress);
                    VMOVtoV(s0, x2);
                }
            }
            VCVT_S32_F32(s0, s0);
            VMOVfrV(gd, s0);
            break;
        case 0x2D:
            INST_NAME("CVTSS2SI Gd, Ex");
            u8 = x87_setround(dyn, ninst, x1, x2, x14);
            nextop = F8;
            GETGD;
            s0 = fpu_get_scratch_single(dyn);
            if((nextop&0xC0)==0xC0) {
                v0 = sse_get_reg(dyn, ninst, x1, nextop&7);
                if(v0<16)
                    d0 = v0;
                else {
                    d0 = fpu_get_scratch_double(dyn);
                    VMOV_64(d0, v0);
                }
                VMOV_32(s0, d0*2);
            } else {
                parity = getedparity(dyn, ninst, addr, nextop, 3);
                if(parity) {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3);
                    VLDR_32(s0, ed, fixedaddress);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0);
                    LDR_IMM9(x2, ed, fixedaddress);
                    VMOVtoV(s0, x2);
                }
            }
            VCVTR_S32_F32(s0, s0);
            VMOVfrV(gd, s0);
            x87_restoreround(dyn, ninst, u8);
            break;

        case 0x51:
            INST_NAME("SQRTSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(s1, v1);
            if(v0==v1)
                d0 = s1/2;
            else
                if(v0<16)
                    d0 = v0;
                else {
                    d0 = fpu_get_scratch_double(dyn);
                    VMOV_64(d0, v0);
                }
            s0 = d0*2;
            VSQRT_F32(s0, s1);
            if(v0!=d0) {
                VMOV_64(v0, d0);
            }
            break;
        case 0x52:
            INST_NAME("RSQRTSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(s1, v1);
            if(v0==v1)
                d0 = s1/2;
            else
                if(v0<16)
                    d0 = v0;
                else {
                    d0 = fpu_get_scratch_double(dyn);
                    VMOV_64(d0, v0);
                }
            s0 = d0*2;
            s2 = fpu_get_scratch_single(dyn);
            // so here: F32: Imm8 = abcd efgh that gives => aBbbbbbc defgh000 00000000 00000000
            // and want 1.0f = 0x3f800000
            // so 00111111 10000000 00000000 00000000
            // a = 0, b = 1, c = 1, d = 1, efgh=0
            // 0b01110000
            VMOV_i_32(s2, 0b01110000);
            VSQRT_F32(s0, s1);
            VDIV_F32(s0, s2, s0);
            if(v0!=d0) {
                VMOV_64(v0, d0);
            }
            break;
        case 0x53:
            INST_NAME("RCPSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(s1, v1);
            if(v0==v1)
                d0 = s1/2;
            else
                if(v0<16)
                    d0 = v0;
                else {
                    d0 = fpu_get_scratch_double(dyn);
                    VMOV_64(d0, v0);
                }
            s0 = d0*2;
            s2 = fpu_get_scratch_single(dyn);
            VMOV_i_32(s2, 0b01110000);
            VDIV_F32(s0, s2, s1);
            if(v0!=d0) {
                VMOV_64(v0, d0);
            }
            break;

        case 0x58:
            INST_NAME("ADDSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(s1, v1);
            if(v0==v1)
                d0 = s1/2;
            else
                if(v0<16)
                    d0 = v0;
                else {
                    d0 = fpu_get_scratch_double(dyn);
                    VMOV_64(d0, v0);
                }
            s0 = d0*2;
            VADD_F32(s0, s0, s1);
            if(v0!=d0) {
                VMOV_64(v0, d0);
            }
            break;
        case 0x59:
            INST_NAME("MULSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(s1, v1);
            if(v0==v1)
                d0 = s1/2;
            else
                if(v0<16)
                    d0 = v0;
                else {
                    d0 = fpu_get_scratch_double(dyn);
                    VMOV_64(d0, v0);
                }
            s0 = d0*2;
            VMUL_F32(s0, s0, s1);
            if(v0!=d0) {
                VMOV_64(v0, d0);
            }
            break;
        case 0x5A:
            INST_NAME("CVTSS2SD Gx, Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(s0, d0);
            VCVT_F64_F32(v0, s0);
            break;
        case 0x5B:
            INST_NAME("CVTTPS2DQ Gx, Ex");
            nextop = F8;
            if((nextop&0xC0)==0xC0) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0);
                v1 = fpu_get_scratch_quad(dyn);
                VLD1Q_64(v1, ed);
            }
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            VCVTQ_S32_F32(v0, v1);
            break;
        case 0x5C:
            INST_NAME("SUBSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(s1, v1);
            if(v0==v1)
                d0 = s1/2;
            else
                if(v0<16)
                    d0 = v0;
                else {
                    d0 = fpu_get_scratch_double(dyn);
                    VMOV_64(d0, v0);
                }
            s0 = d0*2;
            VSUB_F32(s0, s0, s1);
            if(v0!=d0) {
                VMOV_64(v0, d0);
            }
            break;
        case 0x5D:
            INST_NAME("MINSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(s1, v1);
            if(v0==v1)
                d0 = s1/2;
            else
                if(v0<16)
                    d0 = v0;
                else {
                    d0 = fpu_get_scratch_double(dyn);
                    VMOV_64(d0, v0);
                }
            s0 = d0*2;
            // MINSS: if any input is NaN, or Ex[0]<Gx[0], copy Ex[0] -> Gx[0]
            VCMP_F32(s0, s1);
            VMRS_APSR();
            VMOVcond_32(cPL, s0, s1);
            if(v0!=d0) {
                VMOV_64(v0, d0);
            }
            break;
        case 0x5E:
            INST_NAME("DIVSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(s1, v1);
            if(v0==v1)
                d0 = s1/2;
            else
                if(v0<16)
                    d0 = v0;
                else {
                    d0 = fpu_get_scratch_double(dyn);
                    VMOV_64(d0, v0);
                }
            s0 = d0*2;
            VDIV_F32(s0, s0, s1);
            if(v0!=d0) {
                VMOV_64(v0, d0);
            }
            break;
        case 0x5F:
            INST_NAME("MAXSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(s1, v1);
            if(v0==v1)
                d0 = s1/2;
            else
                if(v0<16)
                    d0 = v0;
                else {
                    d0 = fpu_get_scratch_double(dyn);
                    VMOV_64(d0, v0);
                }
            s0 = d0*2;
            // MAXSS: if any input is NaN, or Ex[0]>Gx[0], copy Ex[0] -> Gx[0]
            VCMP_F32(s1, s0);
            VMRS_APSR();
            VMOVcond_32(cPL, s0, s1);
            if(v0!=d0) {
                VMOV_64(v0, d0);
            }
            break;

        case 0x6F:
            INST_NAME("MOVDQU Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if((nextop&0xC0)==0xC0) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-12, 0);
                LDR_IMM9(x2, ed, fixedaddress+0);
                LDR_IMM9(x3, ed, fixedaddress+4);
                VMOVtoV_D(v0, x2, x3);
                LDR_IMM9(x2, ed, fixedaddress+8);
                LDR_IMM9(x3, ed, fixedaddress+12);
                VMOVtoV_D(v0+1, x2, x3);
            }
            break;
        case 0x70:
            INST_NAME("PSHUFHW Gx, Ex, Ib");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            if((nextop&0xC0)==0xC0) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0);
                v1 = fpu_get_scratch_quad(dyn);
                VLD1Q_64(v1, ed);
            }
            u8 = F8;
            // only high part need to be shuffled. VTBL only handle 8bits value, so the 16bits shuffles need to be changed in 8bits
            u32 = 0;
            for (int i=0; i<2; ++i) {
                u32 |= (((u8>>(i*2))&3)*2+0)<<(i*16+0);
                u32 |= (((u8>>(i*2))&3)*2+1)<<(i*16+8);
            }
            MOV32(x2, u32);
            u32 = 0;
            for (int i=2; i<4; ++i) {
                u32 |= (((u8>>(i*2))&3)*2+0)<<((i-2)*16+0);
                u32 |= (((u8>>(i*2))&3)*2+1)<<((i-2)*16+8);
            }
            MOV32(x3, u32);
            d0 = fpu_get_scratch_double(dyn);
            VMOVtoV_D(d0, x2, x3);
            VTBL1_8(v0+1, v1+1, d0);
            if(v0!=v1) {
                VMOVD(v0, v1);
            }
            break;


        case 0x7E:
            INST_NAME("MOVQ Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if((nextop&0xC0)==0xC0) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVD(v0, v1);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0);
                VLD1_64(v0, ed);
            }
            VEOR(v0+1, v0+1, v0+1);
            break;
        case 0x7F:
            INST_NAME("MOVDQU Ex,Gx");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if((nextop&0xC0)==0xC0) {
                v0 = sse_get_reg(dyn, ninst, x1, gd);
                v1 = sse_get_reg_empty(dyn, ninst, x1, nextop&7);
                VMOVQ(v1, v0);
            } else {
                v0 = sse_get_reg(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-12, 0);
                VMOVfrV_D(x2, x3, v0);
                STR_IMM9(x2, ed, fixedaddress+0);
                STR_IMM9(x3, ed, fixedaddress+4);
                VMOVfrV_D(x2, x3, v0+1);
                STR_IMM9(x2, ed, fixedaddress+8);
                STR_IMM9(x3, ed, fixedaddress+12);
            }
            break;

        case 0xBC:
            INST_NAME("TZCNT Gd, Ed");
            SETFLAGS(X_CF|X_ZF, SF_SET);
            nextop = F8;
            GETED;
            GETGD;
            TSTS_REG_LSL_IMM5(ed, ed, 0);
            RBIT(x1, ed);   // reverse
            CLZ(gd, x1);    // x2 gets leading 0 == TZCNT
            MOVW_COND(cEQ, x1, 1);
            MOVW_COND(cNE, x1, 0);
            BFI(xFlags, x1, F_CF, 1);
            RSB_IMM8(x1, x1, 1);
            BFI(xFlags, x1, F_ZF, 1);
            SET_DFNONE(x1);
            break;

        case 0xC2:
            INST_NAME("CMPSS Gx, Ex");
            nextop = F8;
            GETGX(v0);
            GETEX(s1, v1);
            if(v0==v1)
                d0 = s1/2;
            else
                if(v0<16)
                    d0 = v0;
                else {
                    d0 = fpu_get_scratch_double(dyn);
                    VMOV_64(d0, v0);
                }
            s0 = d0*2;
            VCMP_F32(s0, s1);
            VMRS_APSR();
            MOVW(x2, 0);
            u8 = F8;
            switch(u8&7) {
                case 0: MVN_COND_REG_LSL_IMM5(cEQ, x2, x2, 0); break;   // Equal
                case 1: MVN_COND_REG_LSL_IMM5(cCC, x2, x2, 0); break;   // Less than
                case 2: MVN_COND_REG_LSL_IMM5(cLS, x2, x2, 0); break;   // Less or equal
                case 3: MVN_COND_REG_LSL_IMM5(cVS, x2, x2, 0); break;   // NaN
                case 4: MVN_COND_REG_LSL_IMM5(cNE, x2, x2, 0); break;   // Not Equal (or unordered on ARM, not on X86...)
                case 5: MVN_COND_REG_LSL_IMM5(cPL, x2, x2, 0); break;   // Greater or equal or unordered
                case 6: MVN_COND_REG_LSL_IMM5(cHI, x2, x2, 0); break;   // Greater or unordered
                case 7: MVN_COND_REG_LSL_IMM5(cVC, x2, x2, 0); break;   // not NaN
            }
            VMOVtoDx_32(v0, 0, x2);
            break;
        
        case 0xE6:
            INST_NAME("CVTDQ2PD Gx, Ex");
            nextop = F8;
            if((nextop&0xC0)==0xC0) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7);
                if(v1<16)
                    v0 = v1;
                else {
                    v0 = fpu_get_scratch_double(dyn);
                    VMOV_64(v0, v1);    // vfpu opcode here
                }
            } else {
                v0 = fpu_get_scratch_double(dyn);
                parity = getedparity(dyn, ninst, addr, nextop, 3);
                if(parity) {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3);
                    VLDR_64(v0, ed, fixedaddress);    // vfpu opcode here
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-8, 0);
                    LDR_IMM9(x2, ed, fixedaddress);
                    LDR_IMM9(x3, ed, fixedaddress+4);
                    VMOVtoV_D(v0, x2, x3);
                }
            }
            gd = (nextop&0x38)>>3;
            q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            //v0 is a low reg now
            VCVT_F64_S32(q0+1, v0*2+1);
            VCVT_F64_S32(q0+0, v0*2+0);
            break;

        default:
            DEFAULT;
    }
    return addr;
}

