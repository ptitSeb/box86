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
#include "emu/x87emu_private.h"

#include "dynarec_arm_functions.h"
#include "dynarec_arm_helper.h"

static const double d_l2t = L2T;
static const double d_l2e = L2E;
static const double d_pi  = PI;
static const double d_lg2 = LG2;
static const double d_ln2 = LN2;
static const float  f_l2t = L2T;
static const float  f_l2e = L2E;
static const float  f_pi  = PI;
static const float  f_lg2 = LG2;
static const float  f_ln2 = LN2;

extern int arm_v8;

static const void* round_map[4] = {nearbyint, floor, ceil, trunc};
uintptr_t dynarecD9(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop = F8;
    uint8_t ed;
    uint8_t wback, wb1;
    uint8_t u8;
    int fixedaddress;
    int v1, v2;
    int s0;
    int i1, i2, i3;
    int j32;
    int parity;

    MAYUSE(s0);
    MAYUSE(v2);
    MAYUSE(v1);
    MAYUSE(j32);
    (void)u8;

    switch(nextop) {

        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
            INST_NAME("FLD STx");
            X87_PUSH_OR_FAIL(v2, dyn, ninst, x3, X87_ST(nextop&7));
            v1 = x87_get_st(dyn, ninst, x1, x2, (nextop&7)+1, X87_COMBINE(0, (nextop&7)+1));
            if(ST_IS_F(0)) {
                VMOV_32(v2, v1);
            } else {
                VMOV_64(v2, v1);
            }
            // should set C1 to 0
            break;

        case 0xC8:
            INST_NAME("FXCH ST0");
            break;
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            INST_NAME("FXCH STx");
            // swap the cache value, not the double value itself :p
            x87_swapreg(dyn, ninst, x1, x2, 0, nextop&7);
            // should set C1 to 0
            break;

        case 0xD0:
            INST_NAME("FNOP");
            break;

        case 0xE0:
            INST_NAME("FCHS");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
            if(ST_IS_F(0)) {
                VNEG_F32(v1, v1);
            } else {
                VNEG_F64(v1, v1);
            }
            // should set C1 to 0
            break;
        case 0xE1:
            INST_NAME("FABS");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
            if(ST_IS_F(0)) {
                VABS_F32(v1, v1);
            } else {
                VABS_F64(v1, v1);
            }
            // should set C1 to 0
            break;

        case 0xE4:
            INST_NAME("FTST");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
            if(ST_IS_F(0)) {
                VCMP_F32_0(v1);
            } else {
                VCMP_F64_0(v1);
            }
            FCOM(x1, x2);   // same flags...
            break;
        case 0xE5:
            INST_NAME("FXAM");
            #if 1
            i1 = x87_get_current_cache(dyn, ninst, 0, NEON_CACHE_ST_D);
            // value put in x14
            if(i1==-1) {
                if(fpu_is_st_freed(dyn, ninst, 0)) {
                    MOV32(x14, 0b100000100000000);
                    B_MARK3(c__);
                } else {                // not in cache, so check Empty status and load it
                    // load tag
                    LDRH_IMM8(x3, xEmu, offsetof(x86emu_t, fpu_tags));
                    if(i2) {
                        if(i2<0) {
                            MOV_REG_LSL_IMM5(x3, x3, -i2*2);
                        } else {
                            MOV32(x14, 0xffff);
                            ORR_REG_LSL_IMM5(x3, x3, x14, 16);
                            MOV_REG_LSR_IMM5(x3, x3, i2*2);
                        }
                    }
                    TSTS_IMM8(x3, 0b11);
                    MOVW(x14, 0b100000100000000);   // empty
                    B_MARK3(cNE);
                    // check stack  TODO: this test should not be needed, something is wrong with tag handling
                    LDR_IMM9(x3, xEmu, offsetof(x86emu_t, fpu_stack));
                    if(i2) {
                        if(i2<0) {
                            ADD_IMM8(x3, x3, -i2);
                        } else {
                            SUB_IMM8(x3, x3, i2);
                        }
                    }
                    CMPS_IMM8(x3, 0);
                    B_MARK3(cLE);
                    // x14 will be the actual top
                    LDR_IMM9(x14, xEmu, offsetof(x86emu_t, top));
                    i2 = -dyn->n.x87stack;
                    if(i2) {
                        if(i2<0) {
                            SUB_IMM8(x14, x14, -i2);
                        } else {
                            ADD_IMM8(x14, x14, i2);
                        }
                        AND_IMM8(x14, x14, 7);    // (emu->top + i)&7
                    }
                    ADD_REG_LSL_IMM5(x1, xEmu, x14, 3);
                    LDRD_IMM8(x2, x1, offsetof(x86emu_t, x87)); // load r2/r3 with ST0 anyway, for sign extraction
                }
            } else {
                // simply move from cache reg to r2/r3
                v1 = dyn->n.x87reg[i1];
                VMOVfrV_D(x2, x3, v1);
            }
            // get exponant in r1
            MOV_REG_LSR_IMM5(x1, x3, 20);
            MOVW(x14, 0x7ff);
            ANDS_REG_LSL_IMM5(x1, x1, x14, 0);
            B_MARK(cNE); // not zero or denormal
            BIC_IMM8_ROR(x1, x3, 0b10, 1); // remove sign bit
            ORRS_REG_LSL_IMM5(x1, x1, x2, 0);
            MOVW_COND(cEQ, x14, 0b100000000000000); // Zero: C3,C2,C0 = 100
            MOVW_COND(cNE, x14, 0b100010000000000); // Denormal: C3,C2,C0 = 110
            B_MARK3(c__);
            MARK;
            CMPS_REG_LSL_IMM5(x1, x14, 0);   // infinite/NaN?
            MOVW_COND(cNE, x14, 0b000010000000000); // normal: C3,C2,C0 = 010
            B_MARK3(cNE);
            ORR_IMM8(x1, x1, 0x08, 12);  //prepare mask, 0x7ff | 0x800 => 0xfff
            BIC_REG_LSL_IMM5(x1, x3, x1, 20);
            ORRS_REG_LSL_IMM5(x1, x1, x2, 0);
            MOVW_COND(cEQ, x14, 0b000010100000000); // infinity: C3,C2,C0 = 011
            MOVW_COND(cNE, x14, 0b000000100000000); // NaN: C3,C2,C0 = 001
            MARK3;
            // Extract signa & Update SW
            MOV_REG_LSR_IMM5(x1, x3, 31);
            BFI(x14, x1, 9, 1); //C1
            LDRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            BIC_IMM8(x1, x1, 0b01000111, 12);
            ORR_REG_LSL_IMM5(x14, x14, x1, 0);
            STRH_IMM8(x14, xEmu, offsetof(x86emu_t, sw));
            #else
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_refresh(dyn, ninst, x1, x2, 0);
            CALL(fpu_fxam, -1, 0);  // should be possible inline, but is it worth it?
            #endif
            break;

        case 0xE8:
            INST_NAME("FLD1");
            X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, box86_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
            if(ST_IS_F(0)) {
                VMOV_i_32(v1, 0b01110000);
            } else {
                VMOV_i_64(v1, 0b01110000);
            }
            // should set C1 to 0
            break;
        case 0xE9:
            INST_NAME("FLDL2T");
            X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, box86_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
            if(ST_IS_F(0)) {
                MOV32(x2, (&f_l2t));
                VLDR_32(v1, x2, 0);
            } else {
                MOV32(x2, (&d_l2t));
                VLDR_64(v1, x2, 0);
            }
            // should set C1 to 0
            break;
        case 0xEA:     
            INST_NAME("FLDL2E");
            X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, box86_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
            if(ST_IS_F(0)) {
                MOV32(x2, (&f_l2e));
                VLDR_32(v1, x2, 0);
            } else {
                MOV32(x2, (&d_l2e));
                VLDR_64(v1, x2, 0);
            }
            // should set C1 to 0
            break;
        case 0xEB:
            INST_NAME("FLDPI");
            X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, box86_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
            if(ST_IS_F(0)) {
                MOV32(x2, (&f_pi));
                VLDR_32(v1, x2, 0);
            } else {
                MOV32(x2, (&d_pi));
                VLDR_64(v1, x2, 0);
            }
            // should set C1 to 0
            break;
        case 0xEC:
            INST_NAME("FLDLG2");
            X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, box86_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
            if(ST_IS_F(0)) {
                MOV32(x2, (&f_lg2));
                VLDR_32(v1, x2, 0);
            } else {
                MOV32(x2, (&d_lg2));
                VLDR_64(v1, x2, 0);
            }
            // should set C1 to 0
            break;
        case 0xED:
            INST_NAME("FLDLN2");
            X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, box86_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
            if(ST_IS_F(0)) {
                MOV32(x2, (&f_ln2));
                VLDR_32(v1, x2, 0);
            } else {
                MOV32(x2, (&d_ln2));
                VLDR_64(v1, x2, 0);
            }
            // should set C1 to 0
            break;
        case 0xEE:
            INST_NAME("FLDZ");
            X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, box86_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
            if(ST_IS_F(0)) {
                VMOV_8(v1/2, 0);  // float is *2...
            } else {
                VMOV_8(v1, 0);  // cannot use VMOV_i_F64
            }
            // should set C1 to 0
            break;

        case 0xF0:
            INST_NAME("F2XM1");
            #if 0
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(arm_f2xm1, -1, 0);
            #else
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
            if((PK(0)==0xD9 && PK(1)==0xE8) && // next inst is FLD1
            (PK(2)==0xDE && PK(3)==0xC1)) {
                MESSAGE(LOG_DUMP, "Hack for fld1 / faddp st1, st0\n");
                VMOV_64(0, v1);
                CALL_1D(exp2, 0);   // return is d0
                VMOV_64(v1, 0);
                addr+=4;
            } else {
                //ST0.d = expm1(LN2 * ST0.d);
                MOV32(x2, (&d_ln2));
                VLDR_64(0, x2, 0);
                VMUL_F64(0, 0, v1);
                CALL_1D(expm1, 0);   // return is d0
                VMOV_64(v1, 0);
            }
            #endif
            // should set C1 to 0
            break;
        case 0xF1:
            INST_NAME("FYL2X");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
            v2 = x87_get_st(dyn, ninst, x1, x2, 1, NEON_CACHE_ST_D);

            VMOV_64(0, v1);    // prepare call to log2
            CALL_1D(log2, 0);
            VMUL_F64(v2, v2, 0);    //ST(1).d = log2(ST0.d)*ST(1).d
            X87_POP_OR_FAIL(dyn, ninst, x3);
            // should set C1 to 0
            break;
        case 0xF2:
            INST_NAME("FPTAN");
            X87_PUSH_OR_FAIL(v2, dyn, ninst, x1, box86_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
            v1 = x87_get_st(dyn, ninst, x1, x2, 1, NEON_CACHE_ST_D);
            // seems that tan of glib doesn't follow the rounding direction mode
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            VMOV_64(0, v1);    // prepare call to tan
            CALL_1D(tan, box86_dynarec_fastround ? 0 : (1 << u8));
            VMOV_64(v1, 0);
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
            //emu->sw.f.F87_C2 = 0; 
            //emu->sw.f.F87_C1 = 0; 
            LDRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            BFC(x1, 9, 2); //C1 C2 = 0 0
            STRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            if(PK(0)==0xdd && PK(1)==0xd8) {
                MESSAGE(LOG_DUMP, "Optimized next DD D8 fstp st0, st0, not emiting 1\n");
            } else {
                // so here: F64: Imm8 = abcd efgh that gives => aBbbbbbb bbcdefgh 0000000 00000000 00000000...
                // and want 1.0 = 0x3ff0000000000000
                // so 00111111 11110000 00000000 00000000....
                // a = 0, b = 1, c = 1, d = 1, efgh=0
                // 0b01110000
                if(ST_IS_F(0)) {
                    VMOV_i_32(v2, 0b01110000);
                } else {
                    VMOV_i_64(v2, 0b01110000);
                }
            }
            break;
        case 0xF3:
            INST_NAME("FPATAN");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
            v2 = x87_get_st(dyn, ninst, x1, x2, 1, NEON_CACHE_ST_D);
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            VMOV_64(0, v2);    // prepare call to atan2
            VMOV_64(1, v1);
            CALL_2D(atan2, box86_dynarec_fastround ? 0 : (1 << u8));
            VMOV_64(v2, 0);    //ST(1).d = atan2(ST1.d, ST0.d);
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            // should set C1 to 0
            break;
        case 0xF4:
            INST_NAME("FXTRACT");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_do_push_empty(dyn, ninst, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            s0 = x87_stackcount(dyn, ninst, x3);
            CALL(arm_fxtract, -1, 0);
            x87_unstackcount(dyn, ninst, x3, s0);
            // C1 set only if stack under/overflow occurs
            break;
        case 0xF5:
            INST_NAME("FPREM1");
            #if 0
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fprem1, -1, 0);
            #else
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
            v2 = x87_get_st(dyn, ninst, x1, x2, 1, NEON_CACHE_ST_D);
            s0 = fpu_get_scratch_double(dyn);
            VDIV_F64(s0, v1, v2);
            // set rounding to Nearest
                MOVW(x2, 1);    // Nearest
                VMRS(x1);               // get fpscr
                MOV_REG(x14, x1);
                BFI(x1, x2, 22, 2);     // inject new round
                VMSR(x1);               // put new fpscr
            VCVTR_S32_F64(s0*2, s0); // => Q
            // set back rounding
                VMSR(x14);
            // get Q
            VMOVfrV(x14, s0*2);
            // Put back Q in a double
            VCVT_F64_S32(s0, s0*2);
            VMUL_F64(s0, s0, v2);
            VSUB_F64(v1, v1, s0);
            LDR_IMM9(x1, xEmu, offsetof(x86emu_t, sw));
            // set C2 = 0
            BFC(x1, 10, 1);
            // set C1 = Q0
            BFI(x1, x14, 9, 1);
            // set C3 = Q1
            MOV_REG_LSR_IMM5(x14, x14, 1);
            BFI(x1, x14, 14, 1);
            // Set C0 = Q2
            MOV_REG_LSR_IMM5(x14, x14, 1);
            BFI(x1, x14, 8, 1);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, sw));
            #endif
            break;
        case 0xF6:
            INST_NAME("FDECSTP");
            fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
            LDR_IMM9(x2, xEmu, offsetof(x86emu_t, top));
            SUB_IMM8(x2, x2, 1);
            AND_IMM8(x2, x2, 7);
            STR_IMM9(x2, xEmu, offsetof(x86emu_t, top));
            // should set C1 to 0
            break;
        case 0xF7:
            INST_NAME("FINCSTP");
            fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
            LDR_IMM9(x2, xEmu, offsetof(x86emu_t, top));
            ADD_IMM8(x2, x2, 1);
            AND_IMM8(x2, x2, 7);
            STR_IMM9(x2, xEmu, offsetof(x86emu_t, top));
            // should set C1 to 0
            break;
        case 0xF8:
            INST_NAME("FPREM");
            #if 0
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fprem, -1, 0);
            #else
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
            v2 = x87_get_st(dyn, ninst, x1, x2, 1, NEON_CACHE_ST_D);
            s0 = fpu_get_scratch_double(dyn);
            VDIV_F64(s0, v1, v2);
            VCVT_S32_F64(s0*2, s0); // => Q
            // get Q
            VMOVfrV(x14, s0*2);
            // Put back Q in a double
            VCVT_F64_S32(s0, s0*2);
            VMUL_F64(s0, s0, v2);
            VSUB_F64(v1, v1, s0);
            LDR_IMM9(x1, xEmu, offsetof(x86emu_t, sw));
            // set C2 = 0
            BFC(x1, 10, 1);
            // set C1 = Q0
            BFI(x1, x14, 9, 1);
            // set C3 = Q1
            MOV_REG_LSR_IMM5(x14, x14, 1);
            BFI(x1, x14, 14, 1);
            // Set C0 = Q2
            MOV_REG_LSR_IMM5(x14, x14, 1);
            BFI(x1, x14, 8, 1);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, sw));
            #endif
            break;
        case 0xF9:
            INST_NAME("FYL2XP1");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
            v2 = x87_get_st(dyn, ninst, x1, x2, 1, NEON_CACHE_ST_D);

            //ST(1).d = (ST(1).d * log1p(ST0.d)) / M_LN2;
            VMOV_64(0, v1);    // prepare call to log1p
            CALL_1D(log1p, 0);
            VMUL_F64(v2, v2, 0);
            MOV32(x2, (&d_ln2));
            VLDR_64(0, x2, 0);
            VDIV_F64(v2, v2, 0);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            // should set C1 to 0
            break;
        case 0xFA:
            INST_NAME("FSQRT");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_ST0);
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            if(ST_IS_F(0)) {
                VSQRT_F32(v1, v1);
            } else {
                VSQRT_F64(v1, v1);
            }
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
            // should set C1 to 0
            break;
        case 0xFB:
            INST_NAME("FSINCOS");
            X87_PUSH_OR_FAIL(v2, dyn, ninst, x3, NEON_CACHE_ST_D);
            v1 = x87_get_st(dyn, ninst, x1, x2, 1, NEON_CACHE_ST_D);
            // seems that sin and cos function of glibc don't follow the rounding mode
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            VMOV_64(0, v1);
            CALL_1D(sin, box86_dynarec_fastround ? 0 : (1 << u8));
            VSWP(v1, 0);
            CALL_1D(cos, box86_dynarec_fastround ? 0 : (1 << u8));    // would it be faster to do sqrt(1-sin()²) ???
            VMOV_64(v2, 0);
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
            //emu->sw.f.F87_C2 = 0; C1 too
            LDRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            BFC(x1, 9, 2); //C2 C1 = 0 0
            STRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            break;
        case 0xFC:
            INST_NAME("FRNDINT");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
            if(!arm_v8) {
                // check if finite first
                VCMP_F64_0(v1);
                VMRS_APSR();
                B_NEXT(cVS);    // Unordered, skip
                B_NEXT(cEQ);    // Zero, skip
                // load round mode
                LDRH_IMM8(x1, xEmu, offsetof(x86emu_t, cw));    // hopefully cw is not too far for an imm8
                UBFX(x2, x1, 10, 2);    // extract round...
                MOV32(x1, round_map);
                LDR_REG_LSL_IMM5(x2, x1, x2, 2);
                VMOV_64(0, v1);    // prepare call to fpu_round
                CALL_1DR(x2, x3, 0);
                VMOV_64(v1, 0);
            } else {
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
                VRINTR_F64(v1, v1);
                x87_restoreround(dyn, ninst, u8);
            }
            // should set C1 to 0
            break;
        case 0xFD:
            INST_NAME("FSCALE");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
            v2 = x87_get_st(dyn, ninst, x1, x2, 1, NEON_CACHE_ST_D);
            //if(ST0.d!=0.0)
            //    ST0.d = ldexp(ST0.d, trunc(ST1.d));
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            s0 = fpu_get_scratch_single(dyn);
            // value of s0 =
            // 2^31-1 (ST1 >= 2^31), -2^31 (ST1 < -2^31) or int(ST1) (other situations)
            VCVT_S32_F64(s0 , v2);
            VMOVfrV(x2, s0);
            VMOV_64(0, v1);
            CALL_1DDR(ldexp, x2, x3, box86_dynarec_fastround ? 0 : (1 << u8));
            VMOV_64(v1, 0);
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
            // should set C1 to 0
            break;
        case 0xFE:
            INST_NAME("FSIN");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
            // seems that sin of glib doesn't follow the rounding direction mode
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            VMOV_64(0, v1);    // prepare call to sin
            CALL_1D(sin, box86_dynarec_fastround ? 0 : (1 << u8));
            VMOV_64(v1, 0);
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
            //emu->sw.f.F87_C2 = 0; C1 too
            LDRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            BFC(x1, 9, 2); //C2 C1 = 0 0
            STRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            break;
        case 0xFF:
            INST_NAME("FCOS");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
            // seems that cos of glib doesn't follow the rounding direction mode
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            VMOV_64(0, v1);    // prepare call to cos
            CALL_1D(cos, box86_dynarec_fastround ? 0 : (1 << u8));
            VMOV_64(v1, 0);
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
            //emu->sw.f.F87_C2 = 0; C1 too
            LDRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            BFC(x1, 9, 2); //C2 C1 = 0 0
            STRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            break;


        case 0xD1:
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
        case 0xE2:
        case 0xE3:
        case 0xE6:
        case 0xE7:
        case 0xEF:
            DEFAULT;
            break;
             
        default:
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("FLD ST0, float[ED]");
                    X87_PUSH_OR_FAIL(v1, dyn, ninst, x1, box86_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
                    if(ST_IS_F(0))
                        s0 = v1;
                    else
                        s0 = fpu_get_scratch_single(dyn);
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 1023, 0, 0, NULL);
                        VLDR_32(s0, ed, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0, 0, NULL);
                        LDR_IMM9(x2, ed, fixedaddress);
                        VMOVtoV(s0, x2);
                    }
                    if(!ST_IS_F(0)) {
                        VCVT_F64_F32(v1, s0);
                    }
                    // should set C1 to 0
                    break;
                case 2:
                    INST_NAME("FST float[ED], ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, box86_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
                    if(ST_IS_F(0))
                        s0 = v1;
                    else {
                        s0 = fpu_get_scratch_single(dyn);
                        if(!box86_dynarec_fastround)
                            u8 = x87_setround(dyn, ninst, x1, x2, x14);
                        VCVT_F32_F64(s0, v1);
                        if(!box86_dynarec_fastround)
                            x87_restoreround(dyn, ninst, u8);
                    }
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 1023, 0, 0, NULL);
                        VSTR_32(s0, ed, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0, 0, NULL);
                        VMOVfrV(x2, s0);
                        STR_IMM9(x2, ed, fixedaddress);
                    }
                    break;
                case 3:
                    INST_NAME("FSTP float[ED], ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, box86_dynarec_x87double?NEON_CACHE_ST_D:NEON_CACHE_ST_F);
                    if(ST_IS_F(0))
                        s0 = v1;
                    else {
                        s0 = fpu_get_scratch_single(dyn);
                        if(!box86_dynarec_fastround)
                            u8 = x87_setround(dyn, ninst, x1, x2, x14);
                        VCVT_F32_F64(s0, v1);
                        if(!box86_dynarec_fastround)
                            x87_restoreround(dyn, ninst, u8);
                    }
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 1023, 0, 0, NULL);
                        VSTR_32(s0, ed, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0, 0, NULL);
                        VMOVfrV(x2, s0);
                        STR_IMM9(x2, ed, fixedaddress);
                    }
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    break;
                case 4:
                    INST_NAME("FLDENV Ed");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    fpu_purgecache(dyn, ninst, 0, x1, x2, x3); // maybe only x87, not SSE?
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                    if(ed!=x1) {
                        MOV_REG(x1, ed);
                    }
                    MOVW(x2, 0);
                    CALL(fpu_loadenv, -1, 0);
                    break;
                case 5:
                    INST_NAME("FLDCW Ew");
                    GETEW(x1);
                    STRH_IMM8(x1, xEmu, offsetof(x86emu_t, cw));    // hopefully cw is not too far for an imm8
                    break;
                case 6:
                    INST_NAME("FNSTENV Ed");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    fpu_purgecache(dyn, ninst, 0, x1, x2, x3); // maybe only x87, not SSE?
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                    if(ed!=x1) {
                        MOV_REG(x1, ed);
                    }
                    MOVW(x2, 0);
                    CALL(fpu_savenv, -1, 0);
                    break;
                case 7:
                    INST_NAME("FNSTCW Ew");
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0, 0, NULL);
                    ed = x1;
                    wb1 = 1;
                    LDRH_IMM8(x1, xEmu, offsetof(x86emu_t, cw));
                    EWBACK;
                    break;
                default:
                    DEFAULT;
            }
    }
    return addr;
}

