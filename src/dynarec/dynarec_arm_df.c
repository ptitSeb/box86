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
#include "emu/x87emu_private.h"
#include "x86trace.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"
#include "arm_printer.h"

#include "dynarec_arm_functions.h"
#include "dynarec_arm_helper.h"


uintptr_t dynarecDF(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop = F8;
    uint8_t u8;
    int32_t j32;
    uint8_t wback;
    uint8_t ed;
    int v1, v2;
    int s0;
    int fixedaddress;

    MAYUSE(s0);
    MAYUSE(v2);
    MAYUSE(v1);
    MAYUSE(j32);

    switch(nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
            INST_NAME("FFREEP STx");
            // not handling Tag...
            x87_do_pop(dyn, ninst);
            break;

        case 0xE0:
            INST_NAME("FNSTSW AX");
            LDR_IMM9(x2, xEmu, offsetof(x86emu_t, top));
            LDRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            AND_IMM8(x2, x2, 7);
            BFI(x1, x2, 11, 3); // inject top
            BFI(xEAX, x1, 0, 16);
            break;
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
            INST_NAME("FUCOMIP ST0, STx");
            SETFLAGS(X_ALL, SF_SET);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            VCMP_F64(v1, v2);
            FCOMI(x1, x2);
            x87_do_pop(dyn, ninst);
            break;
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
            INST_NAME("FCOMIP ST0, STx");
            SETFLAGS(X_ALL, SF_SET);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            VCMP_F64(v1, v2);
            FCOMI(x1, x2);
            x87_do_pop(dyn, ninst);
            break;

        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            DEFAULT;
            break;

        default:
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("FILD ST0, Ew");
                    v1 = x87_do_push(dyn, ninst);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0);
                    LDRSH_IMM8(x1, wback, fixedaddress);
                    s0 = fpu_get_scratch_single(dyn);
                    VMOVtoV(s0, x1);
                    VCVT_F64_S32(v1, s0);
                    break;
                case 1:
                    INST_NAME("FISTTP Ew, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 255, 0);
                    ed = x1;
                    s0 = fpu_get_scratch_single(dyn);
                    MSR_nzcvq_0();
                    VMRS(x14);               // get fpscr
                    // x1 already have FPCSR reg to clear exceptions flags
                    ORR_IMM8(x3, x14, 0b001, 6); // enable exceptions
                    BIC_IMM8(x3, x3, 0b10011111, 0);
                    VMSR(x3);
                    VCVTR_S32_F64(s0, v1);
                    VMRS(x3);   // get the FPCSR reg and test FPU execption (invalid operation only)
                    VMOVfrV(ed, s0);
                    TSTS_IMM8_ROR(x3, 0b00000001, 0);
                    SSAT_REG_LSL_IMM5_COND(cEQ, x3, 16, ed, 0);
                    CMPS_REG_LSL_IMM5_COND(cEQ, ed, x3, 0);
                    MOVW_COND(cNE, x3, 0x8000); // saturated
                    STRH_IMM8(x3, wback, fixedaddress);
                    x87_do_pop(dyn, ninst);
                    VMSR(x14);
                    break;
                case 2:
                    INST_NAME("FIST Ew, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    u8 = x87_setround(dyn, ninst, x1, x2, x14);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 255, 0);
                    ed = x1;
                    s0 = fpu_get_scratch_single(dyn);
                    MSR_nzcvq_0();
                    // x1 already have FPCSR reg to clear exceptions flags
                    ORR_IMM8(x3, x1, 0b001, 6); // enable exceptions
                    BIC_IMM8(x3, x3, 0b10011111, 0);
                    VMSR(x3);
                    VCVTR_S32_F64(s0, v1);
                    VMRS(x3);   // get the FPCSR reg and test FPU execption (invalid operation only)
                    VMOVfrV(ed, s0);
                    TSTS_IMM8_ROR(x3, 0b00000001, 0);
                    SSAT_REG_LSL_IMM5_COND(cEQ, x3, 16, ed, 0);
                    CMPS_REG_LSL_IMM5_COND(cEQ, ed, x3, 0);
                    MOVW_COND(cNE, x3, 0x8000); // saturated
                    STRH_IMM8(x3, wback, fixedaddress);
                    x87_restoreround(dyn, ninst, u8);
                    break;
                case 3:
                    INST_NAME("FISTP Ew, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    u8 = x87_setround(dyn, ninst, x1, x2, x14);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 255, 0);
                    ed = x1;
                    s0 = fpu_get_scratch_single(dyn);
                    MSR_nzcvq_0();
                    // x1 already have FPCSR reg to clear exceptions flags
                    ORR_IMM8(x3, x1, 0b001, 6); // enable exceptions
                    BIC_IMM8(x3, x3, 0b10011111, 0);
                    VMSR(x3);
                    VCVTR_S32_F64(s0, v1);
                    VMRS(x3);   // get the FPCSR reg and test FPU execption (invalid operation only)
                    VMOVfrV(ed, s0);
                    TSTS_IMM8_ROR(x3, 0b00000001, 0);
                    SSAT_REG_LSL_IMM5_COND(cEQ, x3, 16, ed, 0);
                    CMPS_REG_LSL_IMM5_COND(cEQ, ed, x3, 0);
                    MOVW_COND(cNE, x3, 0x8000); // saturated
                    STRH_IMM8(x3, wback, fixedaddress);
                    x87_do_pop(dyn, ninst);
                    x87_restoreround(dyn, ninst, u8);
                    break;
                case 4:
                    INST_NAME("FBLD ST0, tbytes");
                    x87_do_push_empty(dyn, ninst, x1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0);
                    if(ed!=x1) {MOV_REG(x1, ed);}
                    CALL(fpu_fbld, -1, 0);
                    break;
                case 5: // could be inlined for most thing, but is it usefull?
                    INST_NAME("FILD ST0, i64");
                    x87_do_push_empty(dyn, ninst, x1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0);
                    if(ed!=x1) {MOV_REG(x1, ed);}
                    CALL(arm_fild64, -1, 0);
                    break;
                case 6:
                    INST_NAME("FBSTP tbytes, ST0");
                    x87_forget(dyn, ninst, x1, x2, 0);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0);
                    if(ed!=x1) {MOV_REG(x1, ed);}
                    CALL(fpu_fbst, -1, 0);
                    x87_do_pop(dyn, ninst);
                    break;
                case 7: // could be inlined for most thing, but is it usefull?
                    INST_NAME("FISTP i64, ST0");
                    x87_forget(dyn, ninst, x1, x2, 0);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0);
                    if(ed!=x1) {MOV_REG(x1, ed);}
                    CALL(arm_fistp64, -1, 0);
                    x87_do_pop(dyn, ninst);
                    break;
                default:
                    DEFAULT;
            }
    }
    return addr;
}

