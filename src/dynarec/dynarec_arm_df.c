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

static const void* round_map[4] = {arm_fist64_0, arm_fist64_1, arm_fist64_2, arm_fist64_3};
uintptr_t dynarecDF(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop = F8;
    uint8_t u8;
    int32_t j32;
    uint8_t wback;
    uint8_t ed;
    int /*v0, */v1, v2;
    int s0;
    int fixedaddress;
    int parity;

    MAYUSE(s0);
    MAYUSE(v2);
    MAYUSE(v1);
    //MAYUSE(v0);
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
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;

        case 0xE0:
            INST_NAME("FNSTSW AX");
            LDR_IMM9(x2, xEmu, offsetof(x86emu_t, top));
            LDRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            if(dyn->n.x87stack) {
                // update top
                if(dyn->n.x87stack>0) {
                    SUB_IMM8(x2, x2, dyn->n.x87stack);
                } else {
                    ADD_IMM8(x2, x2, -dyn->n.x87stack);
                }
                AND_IMM8(x2, x2, 7);
            }
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
            SETFLAGS(X_ALL, SF_SET_NODF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0))
            {
                VCMP_F32(v1, v2);
            } else {
                VCMP_F64(v1, v2);
            }
            FCOMI(x1, x2);
            X87_POP_OR_FAIL(dyn, ninst, x3);
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
            SETFLAGS(X_ALL, SF_SET_NODF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0))
            {
                VCMP_F32(v1, v2);
            } else {
                VCMP_F64(v1, v2);
            }
            FCOMI(x1, x2);
            X87_POP_OR_FAIL(dyn, ninst, x3);
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
                    X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, box86_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0, 0, NULL);
                    LDRSH_IMM8(x1, wback, fixedaddress);
                    if(ST_IS_F(0)) {
                        VMOVtoV(v1, x1);
                        VCVT_F32_S32(v1, v1);
                    } else {
                        s0 = fpu_get_scratch_single(dyn);
                        VMOVtoV(s0, x1);
                        VCVT_F64_S32(v1, s0);
                    }
                    break;
                case 1:
                    INST_NAME("FISTTP Ew, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, box86_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 255, 0, 0, NULL);
                    ed = x1;
                    s0 = fpu_get_scratch_single(dyn);
                    MSR_nzcvq_0();
                    VMRS(x14);               // get fpscr
                    // x1 already have FPCSR reg to clear exceptions flags
                    ORR_IMM8(x3, x14, 0b010, 9); // enable exceptions
                    BIC_IMM8(x3, x3, 0b10011111, 0);
                    VMSR(x3);
                    if(ST_IS_F(0)) {
                        VCVT_S32_F32(s0, v1);
                    } else {
                        VCVT_S32_F64(s0, v1);
                    }
                    VMRS(x3);   // get the FPCSR reg and test FPU execption (invalid operation only)
                    VMOVfrV(ed, s0);
                    TSTS_IMM8_ROR(x3, 0b00000001, 0);
                    SSAT_REG_LSL_IMM5_COND(cEQ, x3, 16, ed, 0);
                    CMPS_REG_LSL_IMM5_COND(cEQ, ed, x3, 0);
                    MOVW_COND(cNE, x3, 0x8000); // saturated
                    STRH_IMM8(x3, wback, fixedaddress);
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    VMSR(x14);
                    break;
                case 2:
                    INST_NAME("FIST Ew, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, box86_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
                    u8 = x87_setround_reset(dyn, ninst, x1, x2, x14);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 255, 0, 0, NULL);
                    ed = x1;
                    s0 = fpu_get_scratch_single(dyn);
                    MSR_nzcvq_0();
                    if(ST_IS_F(0)) {
                        VCVTR_S32_F32(s0, v1);
                    } else {
                        VCVTR_S32_F64(s0, v1);
                    }
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
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, box86_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
                    u8 = x87_setround_reset(dyn, ninst, x1, x2, x14);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 255, 0, 0, NULL);
                    ed = x1;
                    s0 = fpu_get_scratch_single(dyn);
                    MSR_nzcvq_0();
                    if(ST_IS_F(0)) {
                        VCVTR_S32_F32(s0, v1);
                    } else {
                        VCVTR_S32_F64(s0, v1);
                    }
                    VMRS(x3);   // get the FPCSR reg and test FPU execption (invalid operation only)
                    VMOVfrV(ed, s0);
                    TSTS_IMM8_ROR(x3, 0b00000001, 0);
                    SSAT_REG_LSL_IMM5_COND(cEQ, x3, 16, ed, 0);
                    CMPS_REG_LSL_IMM5_COND(cEQ, ed, x3, 0);
                    MOVW_COND(cNE, x3, 0x8000); // saturated
                    STRH_IMM8(x3, wback, fixedaddress);
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    x87_restoreround(dyn, ninst, u8);
                    break;
                case 4:
                    INST_NAME("FBLD ST0, tbytes");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, x1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                    if(ed!=x1) {MOV_REG(x1, ed);}
                    s0 = x87_stackcount(dyn, ninst, x3);
                    CALL(fpu_fbld, -1, 0);
                    x87_unstackcount(dyn, ninst, x3, s0);
                    break;
                case 5:
                    INST_NAME("FILD ST0, i64");
                    X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, NEON_CACHE_ST_D);
                    v2 = fpu_get_scratch_double(dyn);
                    s0 = fpu_get_scratch_single(dyn);
                    parity = getedparity(dyn, ninst, addr, nextop, 3);
                    addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, 0, 0, 0, NULL);
                    if(parity) {
                        LDRD_IMM8(x2, wback, 0);    // x2/x3 is 64bits
                    } else {
                        LDR_IMM9(x2, wback, 0);
                        LDR_IMM9(x3, wback, 4);
                    }
                    if(ninst<dyn->size
                      && dyn->insts[ninst+1].x86.addr
                      && *(uint8_t*)dyn->insts[ninst+1].x86.addr==0xDF
                      && (((*(uint8_t*)(dyn->insts[ninst+1].x86.addr+1))>>3)&7)==7)
                    {
                        MESSAGE(LOG_DUMP, "Hack for FILD/FISTP i64");
                    } else {
                        // set STll(0).ll=i64 and ref=ST(0).q later (emu->fpu_ll[emu->top].ref == emu->mmx87[emu->top])
                        //  get TOP
                        LDR_IMM9(x1, xEmu, offsetof(x86emu_t, top));
                        int a = 0 - dyn->n.x87stack;
                        if(a<0) {
                            SUB_IMM8(x1, x1, -a);
                            AND_IMM8(x1, x1, 7);
                        } else if(a>0) {
                            ADD_IMM8(x1, x1, a);
                            AND_IMM8(x1, x1, 7);
                        }
                        // ed (and so x1) is not use anymore.
                        ADD_REG_LSL_IMM5(x1, xEmu, x1, 4);  // each fpu_ll is 2 int64: ref than ll
                        ADD_IMM8_ROR(x1, x1, offsetof(x86emu_t, fpu_ll)>>2, 15);
                        STRD_IMM8(x2, x1, 8);  // save ll
                        // continue with conversion
                        MOVS_REG_LSR_IMM5(x14, x3, 31);    // x14 is sign bit
                        B_MARK(cEQ);    // no NEG is no sign bit
                        RSBS_IMM8(x2, x2, 0);
                        RSC_IMM8(x3, x3, 0);
                        MARK;
                        VMOVtoV(s0, x3);    // last time x3 is used
                        VCVT_F64_U32(v1, s0);
                        VEOR(v2, v2, v2);
                        MOVW(x3, 0x41F0);
                        VMOVtoDx_16(v2, 3, x3);
                        VMUL_F64(v1, v1, v2); // v1 = double high part of i64
                        VMOVtoV(s0, x2);
                        VCVT_F64_U32(v2, s0);
                        VADD_F64(v1, v1, v2);
                        TSTS_IMM8(x14, 1);
                        VNEG_F64_cond(cNE, v1, v1);
                        VST1_64(v1, x1);    // Store STll(0).ref
                    }
                    break;
                case 6:
                    INST_NAME("FBSTP tbytes, ST0");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    x87_forget(dyn, ninst, x1, x2, 0);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                    if(ed!=x1) {MOV_REG(x1, ed);}
                    s0 = x87_stackcount(dyn, ninst, x3);
                    CALL(fpu_fbst, -1, 0);
                    x87_unstackcount(dyn, ninst, x3, s0);
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    break;
                case 7: // could be inlined for most thing, but is it usefull?
                    INST_NAME("FISTP i64, ST0");
                    parity = getedparity(dyn, ninst, addr, nextop, 3);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                    if(ninst
                      && dyn->insts[ninst-1].x86.addr
                      && *(uint8_t*)dyn->insts[ninst-1].x86.addr==0xDF
                      && (((*(uint8_t*)(dyn->insts[ninst-1].x86.addr+1))>>3)&7)==5)
                    {
                        if(parity) {
                            STRD_IMM8(x2, ed, 0);    // x2/x3 is 64bits
                        } else {
                            STR_IMM9(x2, ed, 0);
                            STR_IMM9(x3, ed, 4);
                        }
                    } else {
                        v1 = x87_get_st(dyn, ninst, x2, x3, 0, NEON_CACHE_ST_D);
                        //addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        fpu_get_scratch_double(dyn); // to alocate v0
                        v2 = fpu_get_scratch_double(dyn);
                        //  get TOP
                        LDR_IMM9(x14, xEmu, offsetof(x86emu_t, top));
                        int a = 0 - dyn->n.x87stack;
                        if(a<0) {
                            SUB_IMM8(x14, x14, -a);
                            AND_IMM8(x14, x14, 7);    // (emu->top + i)&7
                        } else if(a>0) {
                            ADD_IMM8(x14, x14, a);
                            AND_IMM8(x14, x14, 7);    // (emu->top + i)&7
                        }
                        ADD_REG_LSL_IMM5(x14, xEmu, x14, 4);  // each fpu_ll is 2 int64: ref than ll
                        MOVW(x2, offsetof(x86emu_t, fpu_ll));   //can be optimized?
                        ADD_REG_LSL_IMM5(x14, x14, x2, offsetof(fpu_ll_t, sref));
                        VLDR_64(v2, x14, 0);
                        VCEQ_32(v2, v2, v1);    // compare
                        VMOVfrV_D(x2, x3, v2);
                        ANDS_REG_LSL_IMM5(x2, x2, x3, 0);   // if NE then values are the same!
                        B_MARK(cEQ);    // do the i64 conversion
                        // memcpy(ed, &STll(0).ll, sizeof(int64_t));
                        LDRD_IMM8(x2, x14, offsetof(fpu_ll_t, sq));  // load ll
                        B_MARK3(c__);
                        MARK;
                        // load round mode
                        LDRH_IMM8(x14, xEmu, offsetof(x86emu_t, cw));    // hopefully cw is not too far for an imm8
                        UBFX(x2, x14, 10, 2);    // extract round...
                        MOV32(x14, round_map);
                        LDR_REG_LSL_IMM5(x2, x14, x2, 2);
                        VMOV_64(0, v1);    // prepare call to log2
                        CALL_1DR_U64(x2, x2, x3, x14, (1<<x1));
                        MARK3;
                        STR_IMM9(x2, ed, 0);
                        STR_IMM9(x3, ed, 4);
                    }
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    break;
                default:
                    DEFAULT;
            }
    }
    return addr;
}

