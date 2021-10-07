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

uintptr_t dynarecD9(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop = F8;
    uint8_t ed;
    uint8_t wback, wb1;
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
            v1 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            v2 = x87_do_push(dyn, ninst, x3);
            VMOV_64(v2, v1);
            break;

        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            INST_NAME("FXCH STx");
            // swap the cache value, not the double value itself :p
            i1 = x87_get_cache(dyn, ninst, x1, x2, nextop&7);
            i2 = x87_get_cache(dyn, ninst, x1, x2, 0);
            i3 = dyn->x87cache[i1];
            dyn->x87cache[i1] = dyn->x87cache[i2];
            dyn->x87cache[i2] = i3;
            // swap those too
            i1 = x87_get_neoncache(dyn, ninst, x1, x2, nextop&7);
            i2 = x87_get_neoncache(dyn, ninst, x1, x2, 0);
            i3 = dyn->n.neoncache[i1].v;
            dyn->n.neoncache[i1].v = dyn->n.neoncache[i2].v;
            dyn->n.neoncache[i2].v = i3;
            break;

        case 0xD0:
            INST_NAME("FNOP");
            break;

        case 0xE0:
            INST_NAME("FCHS");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            VNEG_F64(v1, v1);
            break;
        case 0xE1:
            INST_NAME("FABS");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            VABS_F64(v1, v1);
            break;

        case 0xE4:
            INST_NAME("FTST");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            VCMP_F64_0(v1);
            FCOM(x1, x2);   // same flags...
            break;
        case 0xE5:
            INST_NAME("FXAM");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_refresh(dyn, ninst, x1, x2, 0);
            CALL(fpu_fxam, -1, 0);  // should be possible inline, but is it worth it?
            break;

        case 0xE8:
            INST_NAME("FLD1");
            v1 = x87_do_push(dyn, ninst, x1);
            #if 0
            MOV32(x2, (&d_1));
            VLDR_64(v1, x2, 0);
            #else
            VMOV_i_64(v1, 0b01110000);
            #endif
            break;
        case 0xE9:
            INST_NAME("FLDL2T");
            v1 = x87_do_push(dyn, ninst, x1);
            MOV32(x2, (&d_l2t));
            VLDR_64(v1, x2, 0);
            break;
        case 0xEA:     
            INST_NAME("FLDL2E");
            v1 = x87_do_push(dyn, ninst, x1);
            MOV32(x2, (&d_l2e));
            VLDR_64(v1, x2, 0);
            break;
        case 0xEB:
            INST_NAME("FLDPI");
            v1 = x87_do_push(dyn, ninst, x1);
            MOV32(x2, (&d_pi));
            VLDR_64(v1, x2, 0);
            break;
        case 0xEC:
            INST_NAME("FLDLG2");
            v1 = x87_do_push(dyn, ninst, x1);
            MOV32(x2, (&d_lg2));
            VLDR_64(v1, x2, 0);
            break;
        case 0xED:
            INST_NAME("FLDLN2");
            v1 = x87_do_push(dyn, ninst, x1);
            MOV32(x2, (&d_ln2));
            VLDR_64(v1, x2, 0);
            break;
        case 0xEE:
            INST_NAME("FLDZ");
            v1 = x87_do_push(dyn, ninst, x1);
            #if 0
            MOV32(x2, (&d_0));
            VLDR_64(v1, x2, 0);
            #else
            VMOV_8(v1, 0);  // cannot use VMOV_i_F64
            #endif
            break;

        case 0xFA:
            INST_NAME("FSQRT");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            VSQRT_F64(v1, v1);
            break;

        case 0xFC:
            INST_NAME("FRNDINT");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            // use C helper for now, nothing staightforward is available
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(arm_frndint, -1, 0);
            /*
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            VCMP_F64_0(v1);
            VMRS_APSR();
            B_NEXT(cVS);    // Unordered, skip
            B_NEXT(cEQ);    // Zero, skip
            u8 = x87_setround(dyn, ninst, x1, x2, x3);
            VCVT_S32_F64(x1, v1);   // limit to 32bits....
            VCVT_F64_S32(v1, x1);
            x87_restoreround(dyn, ninst, u8);
            */
            break;
        case 0xF0:
            INST_NAME("F2XM1");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(arm_f2xm1, -1, 0);
            break;
        case 0xF1:
            INST_NAME("FYL2X");
            #if 0
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fyl2x, -1, 0);
            #else
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, 1);
            VMOV_64(0, v1);    // prepare call to log2
            CALL_1D(log2, 0);
            VMUL_F64(v2, v2, 0);    //ST(1).d = log2(ST0.d)*ST(1).d
            #endif
            x87_do_pop(dyn, ninst, x3);
            break;
        case 0xF2:
            INST_NAME("FTAN");
            #if 0
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(arm_ftan, -1, 0);
            #else
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            VMOV_64(0, v1);    // prepare call to tan
            CALL_1D(tan, 0);
            VMOV_64(v1, 0);
            //emu->sw.f.F87_C2 = 0;
            LDRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            BFC(x1, 10, 1); //C2 = 0
            STRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            #endif
            v2 = x87_do_push(dyn, ninst, x3);
            #if 0
            MOV32(x2, (&d_1));
            VLDR_64(v2, x2, 0);
            #else
            // so here: F64: Imm8 = abcd efgh that gives => aBbbbbbb bbcdefgh 0000000 00000000 00000000...
            // and want 1.0 = 0x3ff0000000000000
            // so 00111111 11110000 00000000 00000000....
            // a = 0, b = 1, c = 1, d = 1, efgh=0
            // 0b01110000
            VMOV_i_64(v2, 0b01110000);
            #endif
            break;
        case 0xF3:
            INST_NAME("FPATAN");
            #if 0
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fpatan, -1, 0);
            #else
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, 1);
            VMOV_64(0, v2);    // prepare call to atan2
            VMOV_64(1, v1);
            CALL_2D(atan2, 0);
            VMOV_64(v2, 0);    //ST(1).d = atan2(ST1.d, ST0.d);
            #endif
            x87_do_pop(dyn, ninst, x3);
            break;
        case 0xF4:
            INST_NAME("FXTRACT");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_do_push_empty(dyn, ninst, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fxtract, -1, 0);
            break;
        case 0xF5:
            INST_NAME("FPREM1");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fprem1, -1, 0);
            break;
        case 0xF6:
            INST_NAME("FDECSTP");
            fpu_purgecache(dyn, ninst, x1, x2, x3);
            LDR_IMM9(x2, xEmu, offsetof(x86emu_t, top));
            SUB_IMM8(x2, x2, 1);
            AND_IMM8(x2, x2, 7);
            STR_IMM9(x2, xEmu, offsetof(x86emu_t, top));
            break;
        case 0xF7:
            INST_NAME("FINCSTP");
            fpu_purgecache(dyn, ninst, x1, x2, x3);
            LDR_IMM9(x2, xEmu, offsetof(x86emu_t, top));
            ADD_IMM8(x2, x2, 1);
            AND_IMM8(x2, x2, 7);
            STR_IMM9(x2, xEmu, offsetof(x86emu_t, top));
            break;
        case 0xF8:
            INST_NAME("FPREM");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fprem, -1, 0);
            break;
        case 0xF9:
            INST_NAME("FYL2XP1");
            #if 0
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fyl2xp1, -1, 0);
            #else
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, 1);
            VMOV_i_64(0, 0b01110000);   // D0 = 1.0
            VADD_F64(0, 0, v1);    // prepare call to log2
            CALL_1D(log2, 0);
            VMUL_F64(v2, v2, 0);    //ST(1).d = log2(ST0.d + 1.0)*ST(1).d;
            #endif
            x87_do_pop(dyn, ninst, x3);
            break;
        case 0xFB:
            INST_NAME("FSINCOS");
            #if 0
            x87_do_push_empty(dyn, ninst, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fsincos, -1, 0);
            #else
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_do_push(dyn, ninst, x3);
            VMOV_64(0, v1);
            CALL_1D(sin, 0);
            VSWP(v1, 0);
            CALL_1D(cos, 0);
            VMOV_64(v2, 0);
            //emu->sw.f.F87_C2 = 0;
            LDRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            BFC(x1, 10, 1); //C2 = 0
            STRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            #endif
            break;
        case 0xFD:
            INST_NAME("FSCALE");
            #if 0
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fscale, -1, 0);
            #else
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, 1);
            //if(ST0.d!=0.0)
            //    ST0.d *= exp2(trunc(ST1.d));
            VCMP_F64_0(v1);
            VMRS_APSR();
            B_NEXT(cEQ);
            VMOV_64(0, v2);
            CALL_1DD(trunc, exp2, 0);
            VMUL_F64(v1, v1, 0);
            #endif
            break;
        case 0xFE:
            INST_NAME("FSIN");
            #if 0
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(arm_fsin, -1, 0);
            #else
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            VMOV_64(0, v1);    // prepare call to sin
            CALL_1D(sin, 0);
            VMOV_64(v1, 0);
            //emu->sw.f.F87_C2 = 0;
            LDRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            BFC(x1, 10, 1); //C2 = 0
            STRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            #endif
            break;
        case 0xFF:
            INST_NAME("FCOS");
            #if 0
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(arm_fcos, -1, 0);
            #else
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            VMOV_64(0, v1);    // prepare call to cos
            CALL_1D(cos, 0);
            VMOV_64(v1, 0);
            //emu->sw.f.F87_C2 = 0;
            LDRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            BFC(x1, 10, 1); //C2 = 0
            STRH_IMM8(x1, xEmu, offsetof(x86emu_t, sw));
            #endif
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
                    v1 = x87_do_push(dyn, ninst, x1);
                    s0 = fpu_get_scratch_single(dyn);
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 1023, 0, 0);
                        VLDR_32(s0, ed, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0, 0);
                        LDR_IMM9(x2, ed, fixedaddress);
                        VMOVtoV(s0, x2);
                    }
                    VCVT_F64_F32(v1, s0);
                    break;
                case 2:
                    INST_NAME("FST float[ED], ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    s0 = fpu_get_scratch_single(dyn);
                    VCVT_F32_F64(s0, v1);
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 1023, 0, 0);
                        VSTR_32(s0, ed, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0, 0);
                        VMOVfrV(x2, s0);
                        STR_IMM9(x2, ed, fixedaddress);
                    }
                    break;
                case 3:
                    INST_NAME("FSTP float[ED], ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    s0 = fpu_get_scratch_single(dyn);
                    VCVT_F32_F64(s0, v1);
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 1023, 0, 0);
                        VSTR_32(s0, ed, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0, 0);
                        VMOVfrV(x2, s0);
                        STR_IMM9(x2, ed, fixedaddress);
                    }
                    x87_do_pop(dyn, ninst, x3);
                    break;
                case 4:
                    INST_NAME("FLDENV Ed");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    fpu_purgecache(dyn, ninst, x1, x2, x3); // maybe only x87, not SSE?
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0);
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
                    UBFX(x1, x1, 10, 2);    // extract round
                    STR_IMM9(x1, xEmu, offsetof(x86emu_t, round));
                    break;
                case 6:
                    INST_NAME("FNSTENV Ed");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    fpu_purgecache(dyn, ninst, x1, x2, x3); // maybe only x87, not SSE?
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0);
                    if(ed!=x1) {
                        MOV_REG(x1, ed);
                    }
                    MOVW(x2, 0);
                    CALL(fpu_savenv, -1, 0);
                    break;
                case 7:
                    INST_NAME("FNSTCW Ew");
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0, 0);
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

