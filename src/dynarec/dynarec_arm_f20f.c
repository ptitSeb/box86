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
                VMOV_64(v0, d0);
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
                VMOV_64(d0, v0);
            } else {
                VMOVfrV_D(x2, x3, v0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                STRD_IMM8(x2, ed, 0);
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

        default:
            *ok = 0;
            DEFAULT;
    }
    return addr;
}

