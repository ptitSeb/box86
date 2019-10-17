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
#include "dynablock.h"
#include "dynablock_private.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"
#include "arm_printer.h"
#include "emu/x87emu_private.h"

#include "dynarec_arm_helper.h"

static double d_1   = 1.0;
static double d_l2t = L2T;
static double d_l2e = L2E;
static double d_pi  = PI;
static double d_lg2 = LG2;
static double d_ln2 = LN2;
static double d_0   = 0.0;

uintptr_t dynarecD9(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int* ok, int* need_epilog)
{
    uintptr_t ip = addr-1;
    uint8_t nextop = F8;
    uint8_t u8;
    uint32_t u32;
    int32_t i32;
    int16_t i16;
    uint16_t u16;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    int fixedaddress;
    int v1, v2, v3;
    int s0, s1, s2;
    int i1, i2, i3;
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
            v2 = x87_do_push(dyn, ninst, x1);
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

        case 0xE8:
            INST_NAME("FLD1");
            v1 = x87_do_push(dyn, ninst, x1);
            MOV32(x2, (&d_1));
            VLDR_64(v1, x2, 0);
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
            MOV32(x2, (&d_0));
            VLDR_64(v1, x2, 0);
            break;
 
        case 0xE0:  /* FCHS */
        case 0xE1:  /* FABS */
        case 0xE4:  /* FTST */
        case 0xE5:  /* FXAM */
        case 0xF0:  /* F2XM1 */
        case 0xF1:  /* FYL2X */
        case 0xF2:  /* FTAN */
        case 0xF3:  /* FPATAN */
        case 0xF4:  /* FXTRACT */
        case 0xF8:  /* FPREM */
        case 0xF9:  /* FYL2XP1 */
        case 0xFA:  /* FSQRT */
        case 0xFB:  /* FSINCOS */
        case 0xFC:  /* FRNDINT */
        case 0xFD:  /* FSCALE */
        case 0xFE:  /* FSIN */
        case 0xFF:  /* FCOS */

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
        case 0xF5:
        case 0xF6:
        case 0xF7:
            *ok = 0;
            DEFAULT;
            break;
             
        default:
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("FLD ST0, float[ED]");
                    v1 = x87_do_push(dyn, ninst, x1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                    s0 = 0; // use S0 as scratch single reg
                    VLDR_32(s0, ed, 0);
                    VCVT_F64_F32(v1, s0);
                    break;
                case 2:
                    INST_NAME("FST float[ED], ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                    s0 = 0;
                    VCVT_F32_F64(s0, v1);
                    VSTR_32(s0, ed, 0);
                    break;
                case 3:
                    INST_NAME("FSTP float[ED], ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                    s0 = 0;
                    VCVT_F32_F64(s0, v1);
                    VSTR_32(s0, ed, 0);
                    x87_do_pop(dyn, ninst, x1);
                    break;
                case 7:
                    INST_NAME("FNSTCW Ew");
                    GETEW(x1);
                    LDR_IMM9(x1, xEmu, offsetof(x86emu_t, cw));
                    EWBACK;
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
    }
    return addr;
}

