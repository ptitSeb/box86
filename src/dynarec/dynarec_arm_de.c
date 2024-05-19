#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>

#include "debug.h"
#include "box86context.h"
#include "dynarec.h"
#include "dynarec/arm_emitter.h"
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
#include "dynarec_arm_functions.h"
#include "arm_printer.h"

#include "dynarec_arm_helper.h"


uintptr_t dynarecDE(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop = F8;
    int v1, v2;
    uint8_t u8;

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
            INST_NAME("FADDP STx, ST0");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            if(ST_IS_F(0)) {
                VADD_F32(v2, v2, v1);
            } else {
                VADD_F64(v2, v2, v1);
            }
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
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
            INST_NAME("FMULP STx, ST0");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            if(ST_IS_F(0)) {
                VMUL_F32(v2, v2, v1);
            } else {
                VMUL_F64(v2, v2, v1);
            }
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
            INST_NAME("FCOMP ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                VCMP_F32(v1, v2);
            } else {
                VCMP_F64(v1, v2);
            }
            FCOM(x1, x2);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;

        case 0xD9: 
            INST_NAME("FCOMPP ST0, ST1");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, 1));
            v2 = x87_get_st(dyn, ninst, x1, x2, 1, X87_COMBINE(0, 1));
            if(ST_IS_F(0)) {
                VCMP_F32(v1, v2);
            } else {
                VCMP_F64(v1, v2);
            }
            FCOM(x1, x2);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xE0:
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
            INST_NAME("FSUBRP STx, ST0");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            if(ST_IS_F(0)) {
                VSUB_F32(v2, v1, v2);
            } else {
                VSUB_F64(v2, v1, v2);
            }
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
            INST_NAME("FSUBP STx, ST0");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            if(ST_IS_F(0)) {
                VSUB_F32(v2, v2, v1);
            } else {
                VSUB_F64(v2, v2, v1);
            }
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
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
            INST_NAME("FDIVRP STx, ST0");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            if(ST_IS_F(0)) {
                VDIV_F32(v2, v1, v2);
            } else {
                VDIV_F64(v2, v1, v2);
            }
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            INST_NAME("FDIVP STx, ST0");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(!box86_dynarec_fastround || !box86_dynarec_fastnan) {
                VMRS(x14);               // get fpscr
                if(!box86_dynarec_fastnan) {
                    ORR_IMM8(x3, x14, 0b010, 9); // enable exceptions
                    BIC_IMM8(x3, x3, 0b10011111, 0);
                } else if(!box86_dynarec_fastround)
                    MOV_REG(x3, x14);
                if(!box86_dynarec_fastround){
                    LDRH_IMM8(x1, xEmu, offsetof(x86emu_t, cw));    // hopefully cw is not too far for an imm8
                    UBFX(x1, x1, 10, 2);    // extract round...
                    UBFX(x2, x1, 1, 1);     // swap bits 0 and 1
                    BFI(x2, x1, 1, 1);
                    BFI(x3, x2, 22, 2);     // inject new round
                }
                VMSR(x3);               // put new fpscr
            }
            if(ST_IS_F(0)) {
                VDIV_F32(v2, v2, v1);
            } else {
                VDIV_F64(v2, v2, v1);
            }
            if(!box86_dynarec_fastnan) {
                VMRS(x3);   // get the FPCSR reg and test FPU execption (invalid operation only)
                TSTS_IMM8_ROR(x3, 0b00000001, 0);
                if(ST_IS_F(0)) {
                    VNEG_F32_cond(cNE, v2, v2);
                } else {
                    VNEG_F64_cond(cNE, v2, v2);
                }
            }
            if(!box86_dynarec_fastround || !box86_dynarec_fastnan)
                VMSR(x14);  // restore fpscr
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;

        case 0xD8:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
            DEFAULT;
            break;

       
        default:
            switch((nextop>>3)&7) {
                default:
                    DEFAULT;
            }
    }
    return addr;
}

