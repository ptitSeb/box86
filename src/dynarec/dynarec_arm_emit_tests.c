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
#include "../tools/bridge_private.h"

#include "dynarec_arm_functions.h"
#include "dynarec_arm_helper.h"

// emit CMP32 instruction, from cmp s1 , s2, using s3 and s4 as scratch
void emit_cmp32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX_PENDOR0 {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_cmp32);
    } else {
        SET_DFNONE(s4);
    }
    SUBS_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 - s2
    IFX_PENDOR0 {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF|X_CF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_ZF)|(1<<F_CF), 0);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_OF) {
        ORR_IMM8_COND(cVS, xFlags, xFlags, 0b10, 0x0b);
        BIC_IMM8_COND(cVC, xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_CF) {
        // reversed carry
        ORR_IMM8_COND(cCC, xFlags, xFlags, 1<<F_CF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s3, 31);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s4);
    }
    // and now the tricky ones (and mostly unused), AF
    IFX(X_AF) {
        // bc = (res & (~d | s)) | (~d & s)
        MVN_REG_LSL_IMM5(s4, s1, 0);        // s4 = ~d
        ORR_REG_LSL_IMM5(s4, s4, s2, 0);    // s4 = ~d | s
        AND_REG_LSL_IMM5(s4, s4, s3, 0);    // s4 = res & (~d | s)
        BIC_REG_LSL_IMM5(s3, s2, s1, 0);    // loosing res... s3 = s & ~d
        ORR_REG_LSL_IMM5(s3, s4, s3, 0);    // s3 = (res & (~d | s)) | (s & ~d)
        IFX(X_AF) {
            MOV_REG_LSR_IMM5(s4, s3, 3);
            BFI(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
    }
}

// emit CMP32 instruction, from cmp s1 , 0, using s3 and s4 as scratch
void emit_cmp32_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    IFX_PENDOR0 {
        MOVW(s4, 0);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, op2));
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        SET_DF(s4, d_cmp32);
    } else {
        SET_DFNONE(s4);
    }
    SUBS_IMM8(s3, s1, 0);   // res = s1 - 0
    // and now the tricky ones (and mostly unused), PF and AF
    // bc = (res & (~d | s)) | (~d & s) => is 0 here...
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BFC(xFlags, F_OF, 1);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s3, 31);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit CMP16 instruction, from cmp s1 , s2, using s3 and s4 as scratch
void emit_cmp16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX_PENDOR0 {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s3, d_cmp16);
    } else {
        SET_DFNONE(s3);
    }
    SUB_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 - s2
    IFX_PENDOR0 {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        MOVW(s4, 0xffff);
        TSTS_REG_LSL_IMM5(s3, s4, 0);
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
        BIC_IMM8_COND(cNE, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s3, 15);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s4);
    }
    // bc = (res & (~d | s)) | (~d & s)
    IFX(X_CF|X_AF|X_OF) {
        MVN_REG_LSL_IMM5(s4, s1, 0);        // s4 = ~d
        ORR_REG_LSL_IMM5(s4, s4, s2, 0);    // s4 = ~d | s
        AND_REG_LSL_IMM5(s4, s4, s3, 0);    // s4 = res & (~d | s)
        BIC_REG_LSL_IMM5(s3, s2, s1, 0);    // loosing res... s3 = s & ~d
        ORR_REG_LSL_IMM5(s3, s4, s3, 0);    // s3 = (res & (~d | s)) | (s & ~d)
        IFX(X_CF) {
            MOV_REG_LSR_IMM5(s4, s3, 15);
            BFI(xFlags, s4, F_CF, 1);    // CF : bc & 0x8000
        }
        IFX(X_AF) {
            MOV_REG_LSR_IMM5(s4, s3, 3);
            BFI(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            MOV_REG_LSR_IMM5(s4, s3, 14);
            XOR_REG_LSR_IMM8(s4, s4, s4, 1);
            BFI(xFlags, s4, F_OF, 1);    // OF: ((bc >> 14) ^ ((bc>>14)>>1)) & 1
        }
    }
}

// emit CMP16 instruction, from cmp s1 , #0, using s3 and s4 as scratch
void emit_cmp16_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    IFX_PENDOR0 {
        MOVW(s3, 0);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        SET_DF(s3, d_cmp16);
    } else {
        SET_DFNONE(s3);
    }
    // bc = (res & (~d | s)) | (~d & s) = 0
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BFC(xFlags, F_OF, 1);
    }
    IFX(X_ZF) {
        MOVW(s4, 0xffff);
        TSTS_REG_LSL_IMM5(s1, s4, 0);
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s1, 15);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}
// emit CMP8 instruction, from cmp s1 , s2, using s3 and s4 as scratch
void emit_cmp8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX_PENDOR0 {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_cmp8);
    } else {
        SET_DFNONE(s4);
    }
    SUB_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 - s2
    IFX_PENDOR0 {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        TSTS_IMM8(s3, 0xff);
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
        BIC_IMM8_COND(cNE, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s3, 7);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s4);
    }
    // bc = (res & (~d | s)) | (~d & s)
    IFX(X_CF|X_AF|X_OF) {
        MVN_REG_LSL_IMM5(s4, s1, 0);        // s4 = ~d
        ORR_REG_LSL_IMM5(s4, s4, s2, 0);    // s4 = ~d | s
        AND_REG_LSL_IMM5(s4, s4, s3, 0);    // s4 = res & (~d | s)
        BIC_REG_LSL_IMM5(s3, s2, s1, 0);    // loosing res... s3 = s & ~d
        ORR_REG_LSL_IMM5(s3, s4, s3, 0);    // s3 = (res & (~d | s)) | (s & ~d)
        IFX(X_CF) {
            MOV_REG_LSR_IMM5(s4, s3, 7);
            BFI(xFlags, s4, F_CF, 1);    // CF : bc & 0x80
        }
        IFX(X_AF) {
            MOV_REG_LSR_IMM5(s4, s3, 3);
            BFI(xFlags, s4, F_AF, 1);    // AF: bc & 0x08
        }
        IFX(X_OF) {
            MOV_REG_LSR_IMM5(s4, s3, 6);
            XOR_REG_LSR_IMM8(s4, s4, s4, 1);
            BFI(xFlags, s4, F_OF, 1);    // OF: ((bc >> 6) ^ ((bc>>6)>>1)) & 1
        }
    }
}
// emit CMP8 instruction, from cmp s1 , 0, using s3 and s4 as scratch
void emit_cmp8_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    IFX_PENDOR0 {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        MOVW(s4, 0);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, op2));
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        SET_DF(s3, d_cmp8);
    } else {
        SET_DFNONE(s4);
    }
    // bc = (res & (~d | s)) | (~d & s) = 0
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BFC(xFlags, F_OF, 1);
    }
    IFX(X_ZF) {
        TSTS_IMM8(s1, 0xff);
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s1, 7);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit TEST32 instruction, from test s1 , s2, using s3 and s4 as scratch
void emit_test32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX_PENDOR0 {
        SET_DF(s3, d_tst32);
    } else {
        SET_DFNONE(s4);
    }
    IFX(X_OF) {
        BFC(xFlags, F_OF, 1);
    }
    IFX(X_ZF|X_CF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_ZF)|(1<<F_CF), 0);
    }
    ANDS_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 & s2
    IFX_PENDOR0 {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s3, 31);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s4);
    }
}

// emit TEST16 instruction, from test s1 , s2, using s3 and s4 as scratch
void emit_test16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX_PENDOR0 {
        SET_DF(s3, d_tst16);
    } else {
        SET_DFNONE(s4);
    }
    IFX(X_OF) {
        BFC(xFlags, F_OF, 1);
    }
    IFX(X_ZF|X_CF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_ZF)|(1<<F_CF), 0);
    }
    ANDS_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 & s2
    IFX_PENDOR0 {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s3, 15);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s4);
    }
}

// emit TEST8 instruction, from test s1 , s2, using s3 and s4 as scratch. May destroy s2 also
void emit_test8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX_PENDOR0 {
        SET_DF(s3, d_tst8);
    } else {
        SET_DFNONE(s4);
    }
    IFX(X_OF) {
        BFC(xFlags, F_OF, 1);
    }
    IFX(X_ZF|X_CF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_ZF)|(1<<F_CF), 0);
    }
    ANDS_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 & s2
    IFX_PENDOR0 {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s3, 7);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s3, s4);
    }
}
