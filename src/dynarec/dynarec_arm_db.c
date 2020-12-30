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
#include "emu/x87emu_private.h"
#include "x86run.h"
#include "x86emu.h"
#include "box86stack.h"
#include "callback.h"
#include "x86trace.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"
#include "arm_printer.h"

#include "dynarec_arm_functions.h"
#include "dynarec_arm_helper.h"

uintptr_t dynarecDB(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop = F8;
    uint8_t u8;
    int32_t j32;
    uint8_t ed;
    uint8_t wback;
    int v1, v2;
    int s0;
    int fixedaddress;
    int parity;

    MAYUSE(s0);
    MAYUSE(v1);
    MAYUSE(v2);
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
            INST_NAME("FCMOVNB ST0, STx");
            READFLAGS(X_CF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            TSTS_IMM8(xFlags, 1<<F_CF);
            VMOVcond_64(cEQ, v1, v2);   // F_CF==0
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            INST_NAME("FCMOVNE ST0, STx");
            READFLAGS(X_ZF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            TSTS_IMM8(xFlags, 1<<F_ZF);
            VMOVcond_64(cEQ, v1, v2);   // F_ZF==0
            break;
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
            INST_NAME("FCMOVNBE ST0, STx");
            READFLAGS(X_CF|X_ZF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            TSTS_IMM8(xFlags, (1<<F_CF)|(1<<F_ZF));
            VMOVcond_64(cEQ, v1, v2);   // F_CF==0 & F_ZF==0
            break;
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
            INST_NAME("FCMOVNU ST0, STx");
            READFLAGS(X_PF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            TSTS_IMM8(xFlags, 1<<F_PF);
            VMOVcond_64(cEQ, v1, v2);   // F_PF==0
            break;
        case 0xE1:
            INST_NAME("FDISI8087_NOP"); // so.. NOP?
            break;
        case 0xE2:
            INST_NAME("FNCLEX");
            LDRH_IMM8(x2, xEmu, offsetof(x86emu_t, sw));
            MOVW(x1, 0);
            BFI(x2, x1, 0, 8);  // IE .. PE, SF, ES
            BFI(x2, x1, 15, 1); // B
            STRH_IMM8(x2, xEmu, offsetof(x86emu_t, sw));
            break;
        case 0xE3:
            INST_NAME("FNINIT");
            fpu_purgecache(dyn, ninst, x1, x2, x3); // maybe only x87, not SSE?
            CALL(reset_fpu, -1, 0);
            break;
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
            INST_NAME("FUCOMI ST0, STx");
            SETFLAGS(X_ALL, SF_SET);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            VCMP_F64(v1, v2);
            FCOMI(x1, x2);
            break;
        case 0xF0:  
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
            INST_NAME("FCOMI ST0, STx");
            SETFLAGS(X_ALL, SF_SET);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            VCMP_F64(v1, v2);
            FCOMI(x1, x2);
            break;

        case 0xE0:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
            DEFAULT;
            break;

        default:
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("FILD ST0, Ed");
                    v1 = x87_do_push(dyn, ninst);
                    s0 = fpu_get_scratch_single(dyn);
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 1023, 0);
                        VLDR_32(s0, ed, fixedaddress);
                    } else {
                        GETED;
                        VMOVtoV(s0, ed);
                    }
                    VCVT_F64_S32(v1, s0);
                    break;
                case 1:
                    INST_NAME("FISTTP Ed, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    if((nextop&0xC0)==0xC0) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 4095, 0);
                        ed = x1;
                    }
                    s0 = fpu_get_scratch_single(dyn);
                    MSR_nzcvq_0();
                    VMRS(x14);   // Get FPCSR reg to clear exceptions flags
                    ORR_IMM8(x3, x14, 0b001, 6); // enable exceptions
                    BIC_IMM8(x3, x3, 0b10011111, 0);
                    VMSR(x3);
                    VCVT_S32_F64(s0, v1);
                    VMRS(x3);   // get the FPCSR reg and test FPU exception (IO only)
                    VMOVfrV(ed, s0);
                    TSTS_IMM8_ROR(x3, 0b00000001, 0);
                    MOV_IMM_COND(cNE, ed, 0b10, 1);   // 0x80000000
                    WBACK;
                    VMSR(x14);  // put back values
                    x87_do_pop(dyn, ninst);
                    break;
                case 2:
                    INST_NAME("FIST Ed, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    u8 = x87_setround(dyn, ninst, x1, x2, x14); // x1 have the modified RPSCR reg
                    if((nextop&0xC0)==0xC0) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 4095, 0);
                        ed = x1;
                    }
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
                    MOV_IMM_COND(cNE, ed, 0b10, 1);   // 0x80000000
                    WBACK;
                    x87_restoreround(dyn, ninst, u8);
                    break;
                case 3:
                    INST_NAME("FISTP Ed, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    u8 = x87_setround(dyn, ninst, x1, x2, x14); // x1 have the modified RPSCR reg
                    if((nextop&0xC0)==0xC0) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 4095, 0);
                        ed = x1;
                    }
                    s0 = fpu_get_scratch_single(dyn);
                    MSR_nzcvq_0();
                    // The FPCSR reg to clear exceptions flags
                    ORR_IMM8(x3, x1, 0b001, 6); // enable exceptions
                    BIC_IMM8(x3, x3, 0b10011111, 0);
                    VMSR(x3);
                    VCVTR_S32_F64(s0, v1);
                    VMRS(x3);   // get the FPCSR reg and test FPU execption (denormal and overflow only)
                    VMOVfrV(ed, s0);
                    TSTS_IMM8_ROR(x3, 0b00000001, 0);
                    MOV_IMM_COND(cNE, ed, 0b10, 1);   // 0x80000000
                    WBACK;
                    x87_do_pop(dyn, ninst);
                    x87_restoreround(dyn, ninst, u8);
                    break;
                case 5:
                    INST_NAME("FLD tbyte");
                    x87_do_push_empty(dyn, ninst, x1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0);
                    if(ed!=x1) {
                        MOV_REG(x1, ed);
                    }
                    CALL(arm_fld, -1, 0);
                    break;
                case 7:
                    INST_NAME("FSTP tbyte");
                    x87_forget(dyn, ninst, x1, x3, 0);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0);
                    if(ed!=x1) {
                        MOV_REG(x1, ed);
                    }
                    CALL(arm_fstp, -1, 0);
                    x87_do_pop(dyn, ninst);
                    break;
                default:
                    DEFAULT;
            }
    }
    return addr;
}

