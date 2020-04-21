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

#include "dynarec_arm_functions.h"
#include "dynarec_arm_helper.h"


uintptr_t dynarecD8(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop = F8;
    uint8_t ed;
    uint8_t wback;
    int v1, v2;
    int s0;
    int d1;
    int fixedaddress;

    MAYUSE(d1);
    MAYUSE(s0);
    MAYUSE(v1);
    MAYUSE(v2);
    MAYUSE(ed);

    switch(nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
            INST_NAME("FADD ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            VADD_F64(v1, v1, v2);
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            INST_NAME("FMUL ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            VMUL_F64(v1, v1, v2);
            break;
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
            INST_NAME("FCOM ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            VCMP_F64(v1, v2);
            FCOM(x1, x2);
            break;
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
            INST_NAME("FCOMP ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            VCMP_F64(v1, v2);
            FCOM(x1, x2);
            x87_do_pop(dyn, ninst);
            break;
        case 0xE0:
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
            INST_NAME("FSUB ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            VSUB_F64(v1, v1, v2);
            break;
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
            INST_NAME("FSUBR ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            VSUB_F64(v1, v2, v1);
            break;
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
            INST_NAME("FDIV ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            VDIV_F64(v1, v1, v2);
            break;
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            INST_NAME("FDIVR ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0);
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7);
            VDIV_F64(v1, v2, v1);
            break;
      
        default:
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("FADD ST0, float[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    GETED;
                    s0 = fpu_get_scratch_single(dyn);
                    d1 = fpu_get_scratch_double(dyn);
                    VMOVtoV(s0, ed);
                    VCVT_F64_F32(d1, s0);
                    VADD_F64(v1, v1, d1);
                    break;
                case 1:
                    INST_NAME("FMUL ST0, float[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    GETED;
                    s0 = fpu_get_scratch_single(dyn);
                    d1 = fpu_get_scratch_double(dyn);
                    VMOVtoV(s0, ed);
                    VCVT_F64_F32(d1, s0);
                    VMUL_F64(v1, v1, d1);
                    break;
                case 2:
                    INST_NAME("FCOM ST0, float[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    GETED;
                    s0 = fpu_get_scratch_single(dyn);
                    d1 = fpu_get_scratch_double(dyn);
                    VMOVtoV(s0, ed);
                    VCVT_F64_F32(d1, s0);
                    VCMP_F64(v1, d1);
                    FCOM(x1, x2);
                    break;
                case 3:
                    INST_NAME("FCOMP ST0, float[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    GETED;
                    s0 = fpu_get_scratch_single(dyn);
                    d1 = fpu_get_scratch_double(dyn);
                    VMOVtoV(s0, ed);
                    VCVT_F64_F32(d1, s0);
                    VCMP_F64(v1, d1);
                    FCOM(x1, x2);
                    x87_do_pop(dyn, ninst);
                    break;
                case 4:
                    INST_NAME("FSUB ST0, float[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    GETED;
                    s0 = fpu_get_scratch_single(dyn);
                    d1 = fpu_get_scratch_double(dyn);
                    VMOVtoV(s0, ed);
                    VCVT_F64_F32(d1, s0);
                    VSUB_F64(v1, v1, d1);
                    break;
                case 5:
                    INST_NAME("FSUBR ST0, float[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    GETED;
                    s0 = fpu_get_scratch_single(dyn);
                    d1 = fpu_get_scratch_double(dyn);
                    VMOVtoV(s0, ed);
                    VCVT_F64_F32(d1, s0);
                    VSUB_F64(v1, d1, v1);
                    break;
                case 6:
                    INST_NAME("FDIV ST0, float[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    GETED;
                    s0 = fpu_get_scratch_single(dyn);
                    d1 = fpu_get_scratch_double(dyn);
                    VMOVtoV(s0, ed);
                    VCVT_F64_F32(d1, s0);
                    VDIV_F64(v1, v1, d1);
                    break;
                case 7:
                    INST_NAME("FDIVR ST0, float[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0);
                    GETED;
                    s0 = fpu_get_scratch_single(dyn);
                    d1 = fpu_get_scratch_double(dyn);
                    VMOVtoV(s0, ed);
                    VCVT_F64_F32(d1, s0);
                    VDIV_F64(v1, d1, v1);
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
    }
    return addr;
}

