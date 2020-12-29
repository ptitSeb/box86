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

// Get Ex as a double, not a quad (warning, x2 and x3 may get used)
#define GETEX(a) \
    if((nextop&0xC0)==0xC0) { \
        a = sse_get_reg(dyn, ninst, x1, nextop&7); \
    } else {    \
        parity = getedparity(dyn, ninst, addr, nextop, 3);  \
        a = fpu_get_scratch_double(dyn);            \
        if(parity) {                                \
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3); \
            VLDR_64(a, ed, fixedaddress);           \
        } else {                                    \
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0);\
            LDR_IMM9(x2, ed, fixedaddress+0);       \
            LDR_IMM9(x3, ed, fixedaddress+4);       \
            VMOVtoV_D(a, x2, x3);                   \
        }                                           \
    }

uintptr_t dynarecF20F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop = F8;
    uint8_t u8;
    uint32_t u32;
    uint8_t gd, ed;
    uint8_t wback;
    int v0, v1;
    int d0, d1;
    int s0;
    int fixedaddress;
    int parity;

    MAYUSE(s0);
    MAYUSE(d1);
    MAYUSE(d0);

    switch(nextop) {

        case 0x10:
            INST_NAME("MOVSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if((nextop&0xC0)==0xC0) {
                v0 = sse_get_reg(dyn, ninst, x1, gd);
                d0 = sse_get_reg(dyn, ninst, x1, nextop&7);
                VMOVD(v0, d0);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0);
                LDR_IMM9(x2, ed, fixedaddress+0);
                LDR_IMM9(x3, ed, fixedaddress+4);
                VMOVtoV_D(v0, x2, x3);
                VEOR(v0+1, v0+1, v0+1); // upper 64bits set to 0
            }
            break;
        case 0x11:
            INST_NAME("MOVSD Ex, Gx");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            if((nextop&0xC0)==0xC0) {
                d0 = sse_get_reg(dyn, ninst, x1, nextop&7);
                VMOVD(d0, v0);
            } else {
                VMOVfrV_D(x2, x3, v0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0);
                STR_IMM9(x2, ed, fixedaddress+0);
                STR_IMM9(x3, ed, fixedaddress+4);
            }
            break;
        case 0x12:
            INST_NAME("MOVDDUP Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if((nextop&0xC0)==0xC0) {
                d0 = sse_get_reg(dyn, ninst, x1, nextop&7);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVD(v0, d0);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0);
                LDR_IMM9(x2, ed, fixedaddress+0);
                LDR_IMM9(x3, ed, fixedaddress+4);
                VMOVtoV_D(v0, x2, x3);
            }
            VMOVD(v0+1, v0);
            break;

        case 0x2A:
            INST_NAME("CVTSI2SD Gx, Ed");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            GETED;
            s0 = fpu_get_scratch_single(dyn);
            VMOVtoV(s0, ed);
            VCVT_F64_S32(v0, s0);
            break;

        case 0x2C:
            INST_NAME("CVTTSD2SI Gd, Ex");
            nextop = F8;
            GETGD;
            s0 = fpu_get_scratch_single(dyn);
            if((nextop&0xC0)==0xC0) {
                d0 = sse_get_reg(dyn, ninst, x1, nextop&7);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3);
                d0 = fpu_get_scratch_double(dyn);
                VLDR_64(d0, ed, fixedaddress);
            }
            VCVT_S32_F64(s0, d0);
            VMOVfrV(gd, s0);
            break;
        case 0x2D:
            INST_NAME("CVTSD2SI Gd, Ex");
            u8 = x87_setround(dyn, ninst, x1, x2, x14);
            nextop = F8;
            GETGD;
            s0 = fpu_get_scratch_single(dyn);
            if((nextop&0xC0)==0xC0) {
                d0 = sse_get_reg(dyn, ninst, x1, nextop&7);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3);
                d0 = fpu_get_scratch_double(dyn);
                VLDR_64(d0, ed, fixedaddress);
            }
            VCVTR_S32_F64(s0, d0);
            VMOVfrV(gd, s0);
            x87_restoreround(dyn, ninst, u8);
            break;

        case 0x51:
            INST_NAME("SQRTSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            GETEX(d0);
            VSQRT_F64(v0, d0);
            break;

        case 0x58:
            INST_NAME("ADDSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            GETEX(d0);
            VADD_F64(v0, v0, d0);
            break;
        case 0x59:
            INST_NAME("MULSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            GETEX(d0);
            VMUL_F64(v0, v0, d0);
            break;
        case 0x5A:
            INST_NAME("CVTSD2SS Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            GETEX(d0);
            d1 = fpu_get_scratch_double(dyn);
            s0 = fpu_get_scratch_single(dyn);
            VCVT_F32_F64(s0, d0);
            VMOV_64(d1, v0);
            VMOV_32(d1*2, s0);
            VMOV_64(v0, d1);
            break;

        case 0x5C:
            INST_NAME("SUBSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            GETEX(d0);
            VSUB_F64(v0, v0, d0);
            break;
        case 0x5D:
            INST_NAME("MINSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            GETEX(d0);
            // MINSD: if any input is NaN, or Ex[0]<Gx[0], copy Ex[0] -> Gx[0]
            VCMP_F64(v0, d0);
            VMRS_APSR();
            VMOVcond_64(cPL, v0, d0);
            break;
        case 0x5E:
            INST_NAME("DIVSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            GETEX(d0);
            VDIV_F64(v0, v0, d0);
            break;
        case 0x5F:
            INST_NAME("MAXSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            GETEX(d0);
            // MAXSD: if any input is NaN, or Ex[0]>Gx[0], copy Ex[0] -> Gx[0]
            VCMP_F64(d0, v0);
            VMRS_APSR();
            VMOVcond_64(cPL, v0, d0);
            break;

        case 0x70:
            INST_NAME("PSHUFLW Gx, Ex, Ib");
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
            // only low part need to be suffled. VTBL only handle 8bits value, so the 16bits suffles need to be changed in 8bits
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
            VTBL1_8(v0, v1, d0);
            if(v0!=v1) {
                VMOVD(v0+1, v1+1);
            }
            break;

        case 0x7C:
            INST_NAME("HADDPS Gx, Ex");
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
            VPADD_F32(v0, v0, v0+1);
            if(v0==v1) {
                VMOVD(v0+1, v0);
            } else {
                VPADD_F32(v0+1, v1, v1+1);
            }
            break;

        case 0xC2:
            INST_NAME("CMPSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            GETEX(d0);
            VCMP_F64(v0, d0);
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
            VMOVtoV_D(v0, x2, x2);
            break;

        case 0xF0:
            INST_NAME("LDDQ Gx,Ex");
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

        default:
            DEFAULT;
    }
    return addr;
}

