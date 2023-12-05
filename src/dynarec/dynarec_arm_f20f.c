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
#define GETEX(a, w)                                 \
    if(MODREG) {                       \
        a = sse_get_reg(dyn, ninst, x1, nextop&7,w);\
    } else {    \
        parity = getedparity(dyn, ninst, addr, nextop, 3);  \
        a = fpu_get_scratch_double(dyn);            \
        SMREAD();                                   \
        if(parity) {                                \
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL); \
            VLDR_64(a, ed, fixedaddress);           \
        } else {                                    \
            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0, 0, NULL);\
            LDR_IMM9(x2, ed, fixedaddress+0);       \
            LDR_IMM9(x3, ed, fixedaddress+4);       \
            VMOVtoV_D(a, x2, x3);                   \
        }                                           \
    }

#define GETGM(a)    \
    gd = (nextop&0x38)>>3;  \
    a = mmx_get_reg(dyn, ninst, x1, x2, x3, gd)

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
            if(MODREG) {
                v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                d0 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
                VMOVD(v0, d0);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0, 0, NULL);
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
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                d0 = sse_get_reg(dyn, ninst, x1, nextop&7, 1);
                VMOVD(d0, v0);
            } else {
                VMOVfrV_D(x2, x3, v0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                STR_IMM9(x2, ed, fixedaddress+0);
                STR_IMM9(x3, ed, fixedaddress+4);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("MOVDDUP Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if(MODREG) {
                d0 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVD(v0, d0);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0, 0, NULL);
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
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
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
            if(MODREG) {
                d0 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL);
                d0 = fpu_get_scratch_double(dyn);
                VLDR_64(d0, ed, fixedaddress);
            }
            if(!box86_dynarec_fastround) {
                VMRS(x14);   // Get FPCSR reg to clear exceptions flags
                ORR_IMM8(x3, x14, 0b010, 9); // enable exceptions
                BIC_IMM8(x3, x3, 0b10011111, 0);
                VMSR(x3);
            }
            VCVT_S32_F64(s0, d0);
            VMOVfrV(gd, s0);
            if(!box86_dynarec_fastround) {
                VMRS(x3);   // get the FPCSR reg and test FPU exception (IO only)
                TSTS_IMM8_ROR(x3, 0b00000001, 0);
                MOV_IMM_COND(cNE, gd, 0b10, 1);   // 0x80000000
                VMSR(x14);  // put back values
            }
            break;
        case 0x2D:
            INST_NAME("CVTSD2SI Gd, Ex");
            if(!box86_dynarec_fastround)
                u8 = sse_setround_reset(dyn, ninst, x1, x2, x14);
            else
                u8 = sse_setround(dyn, ninst, x1, x2, x14);
            nextop = F8;
            GETGD;
            s0 = fpu_get_scratch_single(dyn);
            if(MODREG) {
                d0 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL);
                d0 = fpu_get_scratch_double(dyn);
                VLDR_64(d0, ed, fixedaddress);
            }
            VCVTR_S32_F64(s0, d0);
            VMOVfrV(gd, s0);
            if(!box86_dynarec_fastround) {
                VMRS(x3);   // get the FPCSR reg and test FPU exception (IO only)
                TSTS_IMM8_ROR(x3, 0b00000001, 0);
                MOV_IMM_COND(cNE, gd, 0b10, 1);   // 0x80000000
            }
            x87_restoreround(dyn, ninst, u8);
            break;

        case 0x51:
            INST_NAME("SQRTSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            GETEX(d0, 0);
            if(!box86_dynarec_fastnan) {
                VMRS(x14);               // get fpscr
                ORR_IMM8(x3, x14, 0b010, 9); // enable exceptions
                BIC_IMM8(x3, x3, 0b10011111, 0);
                VMSR(x3);
            }
            VSQRT_F64(v0, d0);
            if(!box86_dynarec_fastnan) {
                VMRS(x3);   // get the FPCSR reg and test FPU execption (invalid operation only)
                TSTS_IMM8_ROR(x3, 0b00000001, 0);
                VNEG_F64_cond(cNE, v0, v0);
                VMSR(x14);  // restore FPSCR
            }
            break;

        case 0x58:
            INST_NAME("ADDSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            GETEX(d0, 0);
            if(!box86_dynarec_fastnan) {
                VMRS(x14);               // get fpscr
                ORR_IMM8(x3, x14, 0b010, 9); // enable exceptions
                BIC_IMM8(x3, x3, 0b10011111, 0);
                VMSR(x3);
            }
            VADD_F64(v0, v0, d0);
            if(!box86_dynarec_fastnan) {
                VMRS(x3);   // get the FPCSR reg and test FPU execption (invalid operation only)
                TSTS_IMM8_ROR(x3, 0b00000001, 0);
                VNEG_F64_cond(cNE, v0, v0);
                VMSR(x14);  // restore FPSCR
            }
            break;
        case 0x59:
            INST_NAME("MULSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            GETEX(d0, 0);
            if(!box86_dynarec_fastnan) {
                VMRS(x14);               // get fpscr
                ORR_IMM8(x3, x14, 0b010, 9); // enable exceptions
                BIC_IMM8(x3, x3, 0b10011111, 0);
                VMSR(x3);
            }
            VMUL_F64(v0, v0, d0);
            if(!box86_dynarec_fastnan) {
                VMRS(x3);   // get the FPCSR reg and test FPU execption (invalid operation only)
                TSTS_IMM8_ROR(x3, 0b00000001, 0);
                VNEG_F64_cond(cNE, v0, v0);
                VMSR(x14);  // restore FPSCR
            }
            break;
        case 0x5A:
            INST_NAME("CVTSD2SS Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            GETEX(d0, 0);
            if(v0<16) {
                VCVT_F32_F64(v0*2, d0);
            } else {
                d1 = fpu_get_scratch_double(dyn);
                VMOV_64(d1, v0);
                VCVT_F32_F64(d1*2, d0);
                VMOV_64(v0, d1);
            }
            break;

        case 0x5C:
            INST_NAME("SUBSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            GETEX(d0, 0);
            if(!box86_dynarec_fastnan) {
                VMRS(x14);               // get fpscr
                ORR_IMM8(x3, x14, 0b010, 9); // enable exceptions
                BIC_IMM8(x3, x3, 0b10011111, 0);
                VMSR(x3);
            }
            VSUB_F64(v0, v0, d0);
            if(!box86_dynarec_fastnan) {
                VMRS(x3);   // get the FPCSR reg and test FPU execption (invalid operation only)
                TSTS_IMM8_ROR(x3, 0b00000001, 0);
                VNEG_F64_cond(cNE, v0, v0);
                VMSR(x14);  // restore FPSCR
            }
            break;
        case 0x5D:
            INST_NAME("MINSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            GETEX(d0, 0);
            // MINSD: if any input is NaN, or Ex[0]<Gx[0], copy Ex[0] -> Gx[0]
            VCMP_F64(v0, d0);
            VMRS_APSR();
            VMOVcond_64(cCS, v0, d0); // move v0 <- d0 if v0 or d0 unordered or v0 >= d0
            break;
        case 0x5E:
            INST_NAME("DIVSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            GETEX(d0, 0);
            if(!box86_dynarec_fastnan) {
                VMRS(x14);               // get fpscr
                ORR_IMM8(x3, x14, 0b010, 9); // enable exceptions
                BIC_IMM8(x3, x3, 0b10011111, 0);
                VMSR(x3);
            }
            VDIV_F64(v0, v0, d0);
            if(!box86_dynarec_fastnan) {
                VMRS(x3);   // get the FPCSR reg and test FPU execption (invalid operation only)
                TSTS_IMM8_ROR(x3, 0b00000001, 0);
                VNEG_F64_cond(cNE, v0, v0);
                VMSR(x14);  // restore FPSCR
            }
            break;
        case 0x5F:
            INST_NAME("MAXSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            GETEX(d0, 0);
            // MAXSD: if any input is NaN, or Ex[0]>Gx[0], copy Ex[0] -> Gx[0]
            VCMP_F64(d0, v0);
            VMRS_APSR();
            VMOVcond_64(cCS, v0, d0); // move v0 <- d0 if v0 or d0 is unordered or d0 >= v0
            break;

        case 0x70:
            INST_NAME("PSHUFLW Gx, Ex, Ib");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
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
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
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

        case 0x80:
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x86:
        case 0x87:
        case 0x88:
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0x8C:
        case 0x8D:
        case 0x8E:
        case 0x8F:
            return dynarec0F(dyn, addr-1, ip, ninst, ok, need_epilog);

        case 0xC2:
            INST_NAME("CMPSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            GETEX(d0, 0);
            u8 = F8;
            VCMP_F64(v0, d0);
            VMRS_APSR();
            MOVW(x2, 0);
            switch(u8&7) {
                case 0: MVN_COND_REG_LSL_IMM5(cEQ, x2, x2, 0); break;   // Equal
                case 1: MVN_COND_REG_LSL_IMM5(cCC, x2, x2, 0); break;   // Less than
                case 2: MVN_COND_REG_LSL_IMM5(cLS, x2, x2, 0); break;   // Less or equal
                case 3: MVN_COND_REG_LSL_IMM5(cVS, x2, x2, 0); break;   // NaN
                case 4: MVN_COND_REG_LSL_IMM5(cNE, x2, x2, 0); break;   // Not Equal or unordered
                case 5: MVN_COND_REG_LSL_IMM5(cCS, x2, x2, 0); break;   // Greater or equal or unordered
                case 6: MVN_COND_REG_LSL_IMM5(cHI, x2, x2, 0); break;   // Greater or unordered, test inverted, N!=V so unordereded or less than (inverted)
                case 7: MVN_COND_REG_LSL_IMM5(cVC, x2, x2, 0); break;   // not NaN
            }
            VMOVtoV_D(v0, x2, x2);
            break;

        case 0xD6:
            INST_NAME("MOVDQ2Q Gm, Ex");
            nextop = F8;
            GETGM(v0);
            GETEX(v1, 0);
            VMOV_64(v0, v1);
            break;

        case 0xE6:
            INST_NAME("CVTPD2DQ Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                v1 = fpu_get_scratch_quad(dyn);
                VLD1Q_8(v1, ed);
            }
            u8 = sse_setround(dyn, ninst, x1, x2, x14);
            if(v0<16) {
                VCVTR_S32_F64(v0*2, v1);
                VCVTR_S32_F64(v0*2+1, v1+1);
            } else {
                d0 = fpu_get_scratch_double(dyn);
                VCVTR_S32_F64(d0*2, v1);
                VCVTR_S32_F64(d0*2+1, v1+1);
                VMOVD(v0, d0);
            }
            x87_restoreround(dyn, ninst, u8);
            VEOR(v0+1, v0+1, v0+1);
            break;

        case 0xF0:
            INST_NAME("LDDQ Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-12, 0, 0, NULL);
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

