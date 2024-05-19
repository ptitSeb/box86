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


uintptr_t dynarecDA(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop = F8;
    int32_t j32;
    uint8_t ed;
    uint8_t wback;
    int v1, v2;
    int d0;
    int s0;
    int fixedaddress;
    uint8_t u8;

    MAYUSE(s0);
    MAYUSE(d0);
    MAYUSE(v2);
    MAYUSE(v1);
    MAYUSE(ed);
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
            INST_NAME("FCMOVB ST0, STx");
            READFLAGS(X_CF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            TSTS_IMM8(xFlags, 1<<F_CF);
            if(ST_IS_F(0)) {
                VMOVcond_32(cNE, v1, v2);   // F_CF==1
            } else {
                VMOVcond_64(cNE, v1, v2);   // F_CF==1
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
            INST_NAME("FCMOVE ST0, STx");
            READFLAGS(X_ZF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            TSTS_IMM8(xFlags, 1<<F_ZF);
            if(ST_IS_F(0)) {
                VMOVcond_32(cNE, v1, v2);   // F_ZF==1
            } else {
                VMOVcond_64(cNE, v1, v2);   // F_ZF==1
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
            INST_NAME("FCMOVBE ST0, STx");
            READFLAGS(X_CF|X_ZF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            TSTS_IMM8(xFlags, (1<<F_CF)|(1<<F_ZF));
            if(ST_IS_F(0)) {
                VMOVcond_32(cNE, v1, v2);   // F_CF==1 | F_ZF==1
            } else {
                VMOVcond_64(cNE, v1, v2);   // F_CF==1 | F_ZF==1
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
            INST_NAME("FCMOVU ST0, STx");
            READFLAGS(X_PF);
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            TSTS_IMM8(xFlags, 1<<F_PF);
            if(ST_IS_F(0)) {
                VMOVcond_32(cNE, v1, v2);   // F_PF==1
            } else {
                VMOVcond_64(cNE, v1, v2);   // F_PF==1
            }
            break;       
        case 0xE9:
            INST_NAME("FUCOMPP ST0, ST1");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, 1, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                VCMP_F32(v1, v2);
            } else {
                VCMP_F64(v1, v2);
            }
            FCOM(x1, x2);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;

        case 0xE4:
        case 0xF0:
        case 0xF1:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
        case 0xF8:
        case 0xF9:
        case 0xFD:
            DEFAULT;
            break;

        default:
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("FIADD ST0, Ed");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    GETED;
                    d0 = fpu_get_scratch_double(dyn);
                    s0 = fpu_get_scratch_single(dyn);
                    VMOVtoV(s0, ed);
                    VCVT_F64_S32(d0, s0);
                    if(!box86_dynarec_fastround)
                        u8 = x87_setround(dyn, ninst, x1, x2, x14);
                    VADD_F64(v1, v1, d0);
                    if(!box86_dynarec_fastround)
                        x87_restoreround(dyn, ninst, u8);
                    break;
                case 1:
                    INST_NAME("FIMUL ST0, Ed");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    GETED;
                    d0 = fpu_get_scratch_double(dyn);
                    s0 = fpu_get_scratch_single(dyn);
                    VMOVtoV(s0, ed);
                    VCVT_F64_S32(d0, s0);
                    if(!box86_dynarec_fastround)
                        u8 = x87_setround(dyn, ninst, x1, x2, x14);
                    VMUL_F64(v1, v1, d0);
                    if(!box86_dynarec_fastround)
                        x87_restoreround(dyn, ninst, u8);
                    break;
                case 2:
                    INST_NAME("FICOM ST0, Ed");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    GETED;
                    d0 = fpu_get_scratch_double(dyn);
                    s0 = fpu_get_scratch_single(dyn);
                    VMOVtoV(s0, ed);
                    VCVT_F64_S32(d0, s0);
                    VCMP_F64(v1, d0);
                    FCOM(x1, x2);
                    break;
                case 3:
                    INST_NAME("FICOMP ST0, Ed");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    GETED;
                    d0 = fpu_get_scratch_double(dyn);
                    s0 = fpu_get_scratch_single(dyn);
                    VMOVtoV(s0, ed);
                    VCVT_F64_S32(d0, s0);
                    VCMP_F64(v1, d0);
                    FCOM(x1, x2);
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    break;
                case 4:
                    INST_NAME("FISUB ST0, Ed");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    GETED;
                    d0 = fpu_get_scratch_double(dyn);
                    s0 = fpu_get_scratch_single(dyn);
                    VMOVtoV(s0, ed);
                    VCVT_F64_S32(d0, s0);
                    if(!box86_dynarec_fastround)
                        u8 = x87_setround(dyn, ninst, x1, x2, x14);
                    VSUB_F64(v1, v1, d0);
                    if(!box86_dynarec_fastround)
                        x87_restoreround(dyn, ninst, u8);
                    break;
                case 5:
                    INST_NAME("FISUBR ST0, Ed");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    GETED;
                    d0 = fpu_get_scratch_double(dyn);
                    s0 = fpu_get_scratch_single(dyn);
                    VMOVtoV(s0, ed);
                    VCVT_F64_S32(d0, s0);
                    if(!box86_dynarec_fastround)
                        u8 = x87_setround(dyn, ninst, x1, x2, x14);
                    VSUB_F64(v1, d0, v1);
                    if(!box86_dynarec_fastround)
                        x87_restoreround(dyn, ninst, u8);
                    break;
                case 6:
                    INST_NAME("FIDIV ST0, Ed");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    GETED;
                    d0 = fpu_get_scratch_double(dyn);
                    s0 = fpu_get_scratch_single(dyn);
                    VMOVtoV(s0, ed);
                    VCVT_F64_S32(d0, s0);
                    if(!box86_dynarec_fastround)
                        u8 = x87_setround(dyn, ninst, x1, x2, x14);
                    VDIV_F64(v1, v1, d0);
                    if(!box86_dynarec_fastround)
                        x87_restoreround(dyn, ninst, u8);
                    break;
                case 7:
                    INST_NAME("FIDIVR ST0, Ed");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    GETED;
                    d0 = fpu_get_scratch_double(dyn);
                    s0 = fpu_get_scratch_single(dyn);
                    VMOVtoV(s0, ed);
                    VCVT_F64_S32(d0, s0);
                    if(!box86_dynarec_fastround)
                        u8 = x87_setround(dyn, ninst, x1, x2, x14);
                    VDIV_F64(v1, d0, v1);
                    if(!box86_dynarec_fastround)
                        x87_restoreround(dyn, ninst, u8);
                    break;
            }
    }
    return addr;
}

