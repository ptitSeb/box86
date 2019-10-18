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

#include "dynarec_arm_helper.h"

static double maxint32 = (double)0x7fffffff;

uintptr_t dynarecDB(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int* ok, int* need_epilog)
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
    int v1, v2, v3;
    int s0, s1, s2;
    int d0;
    int fixedaddress;
    
    switch(nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:  /* FCMOVNB ST(0), ST(i) */
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:  /* FCMOVNE ST(0), ST(i) */
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:  /* FCMOVNBE ST(0), ST(i) */
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:  /* FCMOVNU ST(0), ST(i) */
        case 0xE2:      /* FNCLEX */
        case 0xE3:      /* FNINIT */
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:  /* FUCOMI ST0, STx */
        case 0xF0:  
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:  /* FCOMI ST0, STx */

        case 0xE0:
        case 0xE1:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
            *ok = 0;
            DEFAULT;
            break;

        default:
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("FILD ST0, Ed");
                    v1 = x87_do_push(dyn, ninst);
                    GETED;
                    s0 = x87_get_scratch_single(0);
                    VMOVtoV(s0, ed);
                    VCVT_F64_S32(v1, s0);
                    break;
                case 1:
                    INST_NAME("FISTTP Ed, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    u8 = x87_setround(dyn, ninst, x1, x2, x3);
                    if((nextop&0xC0)==0xC0) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress);
                        ed = x1;
                    }
                    s0 = x87_get_scratch_single(0);
                    d0 = x87_get_scratch_double(0);
                    VCVT_S32_F64(s0, v1);
                    VMOVfrV(ed, s0);
                    MOV32(x12, 0x7fffffff);
                    CMPS_REG_LSL_IMM8(ed, x12, 0);
                    i32 = GETMARK-(dyn->arm_size+8);
                    Bcond(cNE, i32);
                    VMOVtoV(s0, x12);
                    VCVT_F64_S32(d0, s0);
                    VCMP_F64(v1, d0);
                    VMRS_APSR();
                    i32 = GETMARK-(dyn->arm_size+8);
                    Bcond(cLS, i32);
                    MOV32(ed, 0x80000000);
                    MARK;
                    WBACK;
                    x87_do_pop(dyn, ninst);
                    x87_restoreround(dyn, ninst, u8);
                    break;
                case 2:
                    INST_NAME("FIST Ed, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    u8 = x87_setround(dyn, ninst, x1, x2, x3);
                    if((nextop&0xC0)==0xC0) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress);
                        ed = x1;
                    }
                    s0 = x87_get_scratch_single(0);
                    d0 = x87_get_scratch_double(0);
                    VCVTR_S32_F64(s0, v1);
                    VMOVfrV(ed, s0);
                    MOV32(x12, 0x7fffffff);
                    CMPS_REG_LSL_IMM8(ed, x12, 0);
                    i32 = GETMARK-(dyn->arm_size+8);
                    Bcond(cNE, i32);
                    VMOVtoV(s0, x12);
                    VCVT_F64_S32(d0, s0);
                    VCMP_F64(v1, d0);
                    VMRS_APSR();
                    i32 = GETMARK-(dyn->arm_size+8);
                    Bcond(cLS, i32);
                    MOV32(ed, 0x80000000);
                    MARK;
                    WBACK;
                    x87_restoreround(dyn, ninst, u8);
                    break;
                case 3:
                    INST_NAME("FISTP Ed, ST0");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    u8 = x87_setround(dyn, ninst, x1, x2, x3);
                    if((nextop&0xC0)==0xC0) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress);
                        ed = x1;
                    }
                    s0 = x87_get_scratch_single(0);
                    d0 = x87_get_scratch_double(0);
                    VCVTR_S32_F64(s0, v1);
                    VMOVfrV(ed, s0);
                    MOV32(x12, 0x7fffffff);
                    CMPS_REG_LSL_IMM8(ed, x12, 0);
                    i32 = GETMARK-(dyn->arm_size+8);
                    Bcond(cNE, i32);
                    VMOVtoV(s0, x12);
                    VCVT_F64_S32(d0, s0);
                    VCMP_F64(v1, d0);
                    VMRS_APSR();
                    i32 = GETMARK-(dyn->arm_size+8);
                    Bcond(cLS, i32);
                    MOV32(ed, 0x80000000);
                    MARK;
                    WBACK;
                    x87_do_pop(dyn, ninst);
                    x87_restoreround(dyn, ninst, u8);
                    break;
                case 7:
                    INST_NAME("FSTP tbyte");
                    x87_refresh(dyn, ninst, x1, x3, 0);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                    if(ed!=x1) {
                        MOV_REG(x1, ed);
                    }
                    CALL(arm_fstp, -1, 0);
                    x87_do_pop(dyn, ninst);
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
    }
    return addr;
}

