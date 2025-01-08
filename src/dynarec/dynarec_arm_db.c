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
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            TSTS_IMM8(xFlags, 1<<F_CF);
            if(ST_IS_F(0)) {
                VMOVcond_32(cEQ, v1, v2);   // F_CF==0
            } else {
                VMOVcond_64(cEQ, v1, v2);   // F_CF==0
            }
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
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            TSTS_IMM8(xFlags, 1<<F_ZF);
            if(ST_IS_F(0)) {
                VMOVcond_32(cEQ, v1, v2);   // F_ZF==0
            } else {
                VMOVcond_64(cEQ, v1, v2);   // F_ZF==0
            }
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
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            TSTS_IMM8(xFlags, (1<<F_CF)|(1<<F_ZF));
            if(ST_IS_F(0)) {
                VMOVcond_32(cEQ, v1, v2);   // F_CF==0 & F_ZF==0
            } else {
                VMOVcond_64(cEQ, v1, v2);   // F_CF==0 & F_ZF==0
            }
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
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            TSTS_IMM8(xFlags, 1<<F_PF);
            if(ST_IS_F(0)) {
                VMOVcond_32(cEQ, v1, v2);   // F_PF==0
            } else {
                VMOVcond_64(cEQ, v1, v2);   // F_PF==0
            }
            break;
        case 0xE1:
            INST_NAME("FDISI8087_NOP"); // so.. NOP?
            break;
        case 0xE2:
            INST_NAME("FNCLEX");
            LDRH_IMM8(x2, xEmu, offsetof(x86emu_t, sw));
            BFC(x2, 0, 8);  // IE .. PE, SF, ES
            BFC(x2, 15, 1); // B
            STRH_IMM8(x2, xEmu, offsetof(x86emu_t, sw));
            break;
        case 0xE3:
            INST_NAME("FNINIT");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            fpu_purgecache(dyn, ninst, 0, x1, x2, x3); // maybe only x87, not SSE?
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
            SETFLAGS(X_ALL, SF_SET_NODF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                VCMP_F32(v1, v2);
            } else {
                VCMP_F64(v1, v2);
            }
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
            SETFLAGS(X_ALL, SF_SET_NODF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                VCMP_F32(v1, v2);
            } else {
                VCMP_F64(v1, v2);
            }
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
                    X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_D);
                    s0 = fpu_get_scratch_single(dyn);
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 1023, 0, 0, NULL);
                        VLDR_32(s0, ed, fixedaddress);
                    } else {
                        GETED;
                        VMOVtoV(s0, ed);
                    }
                    VCVT_F64_S32(v1, s0);
                    break;
                case 1:
                    INST_NAME("FISTTP Ed, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    if((nextop&0xC0)==0xC0) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 4095, 0, 0, NULL);
                        ed = x1;
                    }
                    s0 = fpu_get_scratch_single(dyn);
                    MSR_nzcvq_0();
                    VMRS(x14);   // Get FPCSR reg to clear exceptions flags
                    ORR_IMM8(x3, x14, 0b010, 9); // enable exceptions
                    BIC_IMM8(x3, x3, 0b10011111, 0);
                    VMSR(x3);
                    VCVT_S32_F64(s0, v1);
                    VMRS(x3);   // get the FPCSR reg and test FPU exception (IO only)
                    VMOVfrV(ed, s0);
                    TSTS_IMM8_ROR(x3, 0b00000001, 0);
                    MOV_IMM_COND(cNE, ed, 0b10, 1);   // 0x80000000
                    WBACK;
                    VMSR(x14);  // put back values
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    break;
                case 2:
                    INST_NAME("FIST Ed, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    u8 = x87_setround_reset(dyn, ninst, x1, x2, x14); // x1 have the modified RPSCR reg
                    if((nextop&0xC0)==0xC0) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 4095, 0, 0, NULL);
                        ed = x1; // will not be writen immediatly, so x1 is safe for now
                    }
                    s0 = fpu_get_scratch_single(dyn);
                    MSR_nzcvq_0();
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
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    u8 = x87_setround_reset(dyn, ninst, x1, x2, x14); // x1 have the modified RPSCR reg
                    if((nextop&0xC0)==0xC0) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 4095, 0, 0, NULL);
                        ed = x1;
                    }
                    s0 = fpu_get_scratch_single(dyn);
                    MSR_nzcvq_0();
                    VCVTR_S32_F64(s0, v1);
                    VMRS(x3);   // get the FPCSR reg and test FPU execption (denormal and overflow only)
                    VMOVfrV(ed, s0);
                    TSTS_IMM8_ROR(x3, 0b00000001, 0);
                    MOV_IMM_COND(cNE, ed, 0b10, 1);   // 0x80000000
                    WBACK;
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    x87_restoreround(dyn, ninst, u8);
                    break;
                case 5:
                    INST_NAME("FLD tbyte");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                    if(PK(0)==0xDB && ((PK(1)>>3)&7)==7) {
                        // the FLD is immediatly followed by an FSTP
                        LDM(ed, (1<<x2)|(1<<x3));
                        LDRH_IMM8(x14, ed, 8);
                        // no persistant scratch register, so unrool both instruction here...
                        MESSAGE(LOG_DUMP, "\tHack: FSTP tbyte\n");
                        nextop = F8;    //0xDB
                        nextop = F8;    //modrm
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        STM(ed, (1<<x2)|(1<<x3));
                        STRH_IMM8(x14, ed, 8);
                    } else {
                        if(box86_x87_no80bits) {
                            X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_D);
                            parity = getedparity(dyn, ninst, addr, nextop, 3);
                            if (parity) {
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL);
                                VLDR_64(v1, ed, fixedaddress);
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                                LDR_IMM9(x2, ed, fixedaddress);
                                LDR_IMM9(x3, ed, fixedaddress + 4);
                                VMOVtoV_D(v1, x2, x3);
                            }
                        } else {
                            #if 0
                            MESSAGE(LOG_DUMP, "Need Optimization\n");
                            if(ed!=x1) {
                                MOV_REG(x1, ed);
                            }
X87_PUSH_OR_FAIL_empty(                   , dyn, ninst, x3);
                            CALL(arm_fld, -1, 0);
                            #else
                            X87_PUSH_OR_FAIL(v1, dyn, ninst, x2, NEON_CACHE_ST_D);
                            // copy 10bytes of *ED to STld(0)
                            LDR_IMM9(x3, xEmu, offsetof(x86emu_t, top));
                            int a = -dyn->n.x87stack;
                            if(a) {
                                if(a<0) {
                                    SUB_IMM8(x3, x3, -a);
                                } else {
                                    ADD_IMM8(x3, x3, a);
                                }
                                AND_IMM8(x3, x3, 7);
                            }
                            ADD_IMM8(x2, xEmu, offsetof(x86emu_t, fpu_ld)+offsetof(fpu_ld_t, ld));
                            MOVW(x14, sizeof(fpu_ld_t));
                            MLA(x2, x3, x14, x2);
                            LDR_IMM9(x14, ed, 0);
                            STR_IMM9(x14, x2, 0);
                            LDR_IMM9(x14, ed, 4);
                            STR_IMM9(x14, x2, 4);
                            LDRH_IMM8(x14, ed, 8);
                            STRH_IMM8(x14, x2, 8);
                            // convert 10bytes -> double
                            CALL_1RD(FromLD, ed, x2, (1<<x3)|(1<<xFlags)|(1<<x2));
                            // store ref to STld(0).uref, but can be unaligned...
                            VMOVfrV_D(x3, x14, 0);
                            // also, store result to ST0
                            VMOVD(v1, 0);
                            STR_IMM9(x3, x2, offsetof(fpu_ld_t, uref)-offsetof(fpu_ld_t, ld));
                            STR_IMM9(x14, x2, offsetof(fpu_ld_t, uref)-offsetof(fpu_ld_t, ld)+4);
                            #endif
                        }
                    }
                    break;
                case 7:
                    INST_NAME("FSTP tbyte");
                        if(box86_x87_no80bits) {
                        v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                        parity = getedparity(dyn, ninst, addr, nextop, 3);
                        if(parity) {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL);
                            VSTR_64(v1, ed, fixedaddress);
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                            VMOVfrV_D(x2, x3, v1);
                            STR_IMM9(x2, ed, fixedaddress);
                            STR_IMM9(x3, ed, fixedaddress+4);
                        }
                    } else {
                        #if 0
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        x87_forget(dyn, ninst, x1, x3, 0);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        if(ed!=x1) {
                            MOV_REG(x1, ed);
                        }
                        CALL(arm_fstp, -1, 0);
                        #else
                        v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                        v2 = fpu_get_scratch_double(dyn);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        // get top in x3
                        LDR_IMM9(x3, xEmu, offsetof(x86emu_t, top));
                        int a = -dyn->n.x87stack;
                        if(a) {
                            if(a<0) {
                                SUB_IMM8(x3, x3, -a);
                            } else {
                                ADD_IMM8(x3, x3, a);
                            }
                            AND_IMM8(x3, x3, 7);
                        }
                        // get STld(0)
                        ADD_IMM8(x2, xEmu, offsetof(x86emu_t, fpu_ld)+offsetof(fpu_ld_t, ld));
                        MOVW(x14, sizeof(fpu_ld_t));
                        MLA(x2, x3, x14, x2);
                        // compare uref with actual value, top is not used anymore, so x3 is free
                        LDR_IMM9(x14, x2, offsetof(fpu_ld_t, uref)-offsetof(fpu_ld_t, ld));
                        LDR_IMM9(x3, x2, offsetof(fpu_ld_t, uref)-offsetof(fpu_ld_t, ld)+4);
                        VMOVtoV_D(v2, x14, x3);
                        VCEQ_32(v2, v2, v1);    // same?
                        VMOVfrV_D(x14, x3, v2);
                        TSTS_REG_LSL_IMM5(x14, x3, 0);
                        B_MARK(cEQ);    // different, do the conversion
                        // same... copy STld(0).ld
                        LDR_IMM9(x14, x2, 0);
                        LDR_IMM9(x3, x2, 4);
                        STR_IMM9(x14, ed, 0);
                        STR_IMM9(x3, ed, 4);
                        LDRH_IMM8(x14, x2, 8);
                        STRH_IMM8(x14, ed, 8);
                        B_MARK2(c__);
                        MARK;
                        // different, call D2LD
                        SUB_IMM8(xSP, xSP, 8);
                        VSTR_64(v1, xSP, 0);
                        PUSH(xSP, (1<<xEmu)|(1<<xFlags));
                        ADD_IMM8(0, xSP, 8);    //1st arg is &d
                        if(ed!=x1) {
                            MOV_REG_LSL_IMM5(x1, ed, 0);    //2nd arg is ed
                        }
                        CALL_S(D2LD, -2, 0);
                        POP(xSP, (1<<xEmu)|(1<<xFlags));
                        ADD_IMM8(xSP, xSP, 8);
                        MARK2;
                        #endif
                    }
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    break;
                default:
                    DEFAULT;
            }
    }
    return addr;
}

