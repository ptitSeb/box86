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

static const double d_1   = 1.0;
static const double d_l2t = L2T;
static const double d_l2e = L2E;
static const double d_pi  = PI;
static const double d_lg2 = LG2;
static const double d_ln2 = LN2;
static const double d_0   = 0.0;

uintptr_t dynarecD9(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop = F8;
    uint8_t ed;
    uint8_t wback, wb1;
    int fixedaddress;
    int v1, v2;
    int s0;
    int i1, i2, i3;
    int parity;

    MAYUSE(s0);
    MAYUSE(v2);
    MAYUSE(v1);

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
            v2 = x87_do_push(dyn, ninst);
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
            x87_refresh(dyn, ninst, x1, x2, 0);
            CALL(fpu_fxam, -1, 0);  // should be possible inline, but is it worth it?
            break;

        case 0xE8:
            INST_NAME("FLD1");
            v1 = x87_do_push(dyn, ninst);
            MOV32(x2, (&d_1));
            VLDR_64(v1, x2, 0);
            break;
        case 0xE9:
            INST_NAME("FLDL2T");
            v1 = x87_do_push(dyn, ninst);
            MOV32(x2, (&d_l2t));
            VLDR_64(v1, x2, 0);
            break;
        case 0xEA:     
            INST_NAME("FLDL2E");
            v1 = x87_do_push(dyn, ninst);
            MOV32(x2, (&d_l2e));
            VLDR_64(v1, x2, 0);
            break;
        case 0xEB:
            INST_NAME("FLDPI");
            v1 = x87_do_push(dyn, ninst);
            MOV32(x2, (&d_pi));
            VLDR_64(v1, x2, 0);
            break;
        case 0xEC:
            INST_NAME("FLDLG2");
            v1 = x87_do_push(dyn, ninst);
            MOV32(x2, (&d_lg2));
            VLDR_64(v1, x2, 0);
            break;
        case 0xED:
            INST_NAME("FLDLN2");
            v1 = x87_do_push(dyn, ninst);
            MOV32(x2, (&d_ln2));
            VLDR_64(v1, x2, 0);
            break;
        case 0xEE:
            INST_NAME("FLDZ");
            v1 = x87_do_push(dyn, ninst);
            MOV32(x2, (&d_0));
            VLDR_64(v1, x2, 0);
            break;

        case 0xFA:
            INST_NAME("FSQRT");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            VSQRT_F64(v1, v1);
            break;

        case 0xFC:
            INST_NAME("FRNDINT");
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
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(arm_f2xm1, -1, 0);
            break;
        case 0xF1:
            INST_NAME("FYL2X");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fyl2x, -1, 0);
            x87_do_pop(dyn, ninst);
            break;
        case 0xF2:
            INST_NAME("FTAN");
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(arm_ftan, -1, 0);
            v1 = x87_do_push(dyn, ninst);
            MOV32(x2, (&d_1));
            VLDR_64(v1, x2, 0);
            break;
        case 0xF3:
            INST_NAME("FPATAN");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fpatan, -1, 0);
            x87_do_pop(dyn, ninst);
            break;
        case 0xF4:
            INST_NAME("FXTRACT");
            x87_do_push_empty(dyn, ninst, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fxtract, -1, 0);
            break;
        case 0xF5:
            INST_NAME("FPREM1");
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
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fprem, -1, 0);
            break;
        case 0xF9:
            INST_NAME("FYL2XP1");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fyl2xp1, -1, 0);
            x87_do_pop(dyn, ninst);
            break;
        case 0xFB:
            INST_NAME("FSINCOS");
            x87_do_push_empty(dyn, ninst, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fsincos, -1, 0);
            break;
        case 0xFD:
            INST_NAME("FSCALE");
            x87_forget(dyn, ninst, x1, x2, 0);
            x87_forget(dyn, ninst, x1, x2, 1);
            CALL(arm_fscale, -1, 0);
            break;
        case 0xFE:
            INST_NAME("FSIN");
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(arm_fsin, -1, 0);
            break;
        case 0xFF:
            INST_NAME("FCOS");
            x87_forget(dyn, ninst, x1, x2, 0);
            CALL(arm_fcos, -1, 0);
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
                    v1 = x87_do_push(dyn, ninst);
                    s0 = fpu_get_scratch_single(dyn);
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 1023, 0);
                        VLDR_32(s0, ed, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0);
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
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 1023, 0);
                        VSTR_32(s0, ed, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0);
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
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 1023, 0);
                        VSTR_32(s0, ed, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0);
                        VMOVfrV(x2, s0);
                        STR_IMM9(x2, ed, fixedaddress);
                    }
                    x87_do_pop(dyn, ninst);
                    break;
                case 4:
                    INST_NAME("FLDENV Ed");
                    fpu_purgecache(dyn, ninst, x1, x2, x3); // maybe only x87, not SSE?
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0);
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
                    fpu_purgecache(dyn, ninst, x1, x2, x3); // maybe only x87, not SSE?
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0);
                    if(ed!=x1) {
                        MOV_REG(x1, ed);
                    }
                    MOVW(x2, 0);
                    CALL(fpu_savenv, -1, 0);
                    break;
                case 7:
                    INST_NAME("FNSTCW Ew");
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0);
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

