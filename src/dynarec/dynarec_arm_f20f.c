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

// Get Ex as a double, not a quad
#define GETEX(a) \
    if((nextop&0xC0)==0xC0) { \
        a = sse_get_reg(dyn, ninst, x1, nextop&7); \
    } else {    \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress); \
        a = fpu_get_scratch_double(dyn);            \
        VLDR_64(a, ed, 0);                          \
    }

uintptr_t dynarecF20F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop = F8;
    uint8_t u8;
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
    switch(nextop) {
        
        case 0x10:
            INST_NAME("MOVSD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            if((nextop&0xC0)==0xC0) {
                d0 = sse_get_reg(dyn, ninst, x1, nextop&7);
                VMOVD(v0, d0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                LDRD_IMM8(x2, ed, 0);   // to avoid bus errors
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
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                STRD_IMM8(x2, ed, 0);
            }
            break;
        case 0x12:
            INST_NAME("MOVDDUP Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            if((nextop&0xC0)==0xC0) {
                d0 = sse_get_reg(dyn, ninst, x1, nextop&7);
                VMOVD(v0, d0);
                VMOVD(v0+1, d0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                LDRD_IMM8(x2, ed, 0);   // to avoid bus errors
                VMOVtoV_D(v0, x2, x3);
                VMOVtoV_D(v0+1, x2, x3);
            }
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
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                d0 = fpu_get_scratch_double(dyn);
                VLDR_64(d0, ed, 0);
            }
            VCVT_S32_F64(s0, d0);
            VMOVfrV(gd, s0);
            break;
        case 0x2D:
            INST_NAME("CVTSD2SI Gd, Ex");
            u8 = x87_setround(dyn, ninst, x1, x2, x12);
            nextop = F8;
            GETGD;
            s0 = fpu_get_scratch_single(dyn);
            if((nextop&0xC0)==0xC0) {
                d0 = sse_get_reg(dyn, ninst, x1, nextop&7);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                d0 = fpu_get_scratch_double(dyn);
                VLDR_64(d0, ed, 0);
            }
            VCVTR_S32_F64(s0, d0);
            VMOVfrV(gd, s0);
            x87_restoreround(dyn, ninst, u8);
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
                case 0: MVN_COND_REG_LSL_IMM8(cEQ, x2, x2, 0); break;   // Equal
                case 1: MVN_COND_REG_LSL_IMM8(cCC, x2, x2, 0); break;   // Less than
                case 2: MVN_COND_REG_LSL_IMM8(cLS, x2, x2, 0); break;   // Less or equal
                case 3: MVN_COND_REG_LSL_IMM8(cVS, x2, x2, 0); break;   // NaN
                case 4: MVN_COND_REG_LSL_IMM8(cNE, x2, x2, 0); break;   // Not Equal (or unordered on ARM, not on X86...)
                case 5: MVN_COND_REG_LSL_IMM8(cPL, x2, x2, 0); break;   // Greater or equal or unordered
                case 6: MVN_COND_REG_LSL_IMM8(cHI, x2, x2, 0); break;   // Greater or unordered
                case 7: MVN_COND_REG_LSL_IMM8(cVC, x2, x2, 0); break;   // not NaN
            }
            VMOVtoV_D(v0, x2, x2);
            break;

        default:
            *ok = 0;
            DEFAULT;
    }
    return addr;
}

