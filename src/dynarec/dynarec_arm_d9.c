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
    switch(nextop) {

        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:  /* FLD STx */

        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:  /* FXCH STx */
            *ok = 0;
            DEFAULT;
            break;

        case 0xD0:  /* FNOP */
            break;

        case 0xE8:  /* FLD1 */
            v1 = x87_do_push(dyn, ninst, x1);
            MOV32(x2, (&d_1));
            VSTR_64(v1, x2, 0);
            break;
        case 0xE9:  /* FLDL2T */
            v1 = x87_do_push(dyn, ninst, x1);
            MOV32(x2, (&d_l2t));
            VSTR_64(v1, x2, 0);
        case 0xEA:  /* FLDL2E */        
            v1 = x87_do_push(dyn, ninst, x1);
            MOV32(x2, (&d_l2e));
            VSTR_64(v1, x2, 0);
        case 0xEB:  /* FLDPI */
            v1 = x87_do_push(dyn, ninst, x1);
            MOV32(x2, (&d_pi));
            VSTR_64(v1, x2, 0);
        case 0xEC:  /* FLDLG2 */
            v1 = x87_do_push(dyn, ninst, x1);
            MOV32(x2, (&d_lg2));
            VSTR_64(v1, x2, 0);
        case 0xED:  /* FLDLN2 */
            v1 = x87_do_push(dyn, ninst, x1);
            MOV32(x2, (&d_ln2));
            VSTR_64(v1, x2, 0);
        case 0xEE:  /* FLDZ */
            v1 = x87_do_push(dyn, ninst, x1);
            MOV32(x2, (&d_0));
            VSTR_64(v1, x2, 0);
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

