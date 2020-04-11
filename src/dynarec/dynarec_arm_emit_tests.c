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
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        MOVW(s4, d_cmp32);
    } else {
        MOVW(s4, 0);
    }
    STR_IMM9(s4, xEmu, offsetof(x86emu_t, df)); // reset flags
    SUBS_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 - s2
    IFX(X_PEND) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        IFX(X_PEND) {
            MOVW(s4, 0);
        }
        // first the easy flag, also found on ARM
        MOVW_COND(cEQ, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_OF) {
        IFX(X_ZF|X_PEND) {
            MOVW(s4, 0);
        }
        MOVW_COND(cVS, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_CF) {
        IFX(X_ZF|X_OF|X_PEND) {
            MOVW(s4, 0);
        }
        MOVW_COND(cCC, s4, 1);      // Carry flags on ARM is reversed compared to x86 one
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_SF) {
        UBFX(s4, s3, 31, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_SF]));
    }
    // and now the tricky ones (and mostly unused), PF and AF
    IFX(X_AF) {
        // bc = (res & (~d | s)) | (~d & s)
        MVN_REG_LSL_IMM5(s4, s1, 0);        // s4 = ~d
        ORR_REG_LSL_IMM5(s4, s4, s2, 0);    // s4 = ~d | s
        AND_REG_LSL_IMM5(s4, s4, s3, 0);    // s4 = res & (~d | s)
        BIC_REG_LSL_IMM5(s3, s2, s1, 0);    // loosing res... s3 = s & ~d
        ORR_REG_LSL_IMM5(s3, s4, s3, 0);    // s3 = (res & (~d | s)) | (s & ~d)
        IFX(X_AF) {
            UBFX(s4, s3, 3, 1);
            STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_AF]));    // AF: bc & 0x08
        }
    }
    IFX(X_PF) {
        // PF: (((emu->x86emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
        IFX(X_CF|X_AF) {
            SUB_REG_LSL_IMM5(s3, s1, s2, 0);
        }
        AND_IMM8(s3, s3, 0xE0); // lsr 5 masking pre-applied
        LDR_IMM9(s4, xEmu, offsetof(x86emu_t, x86emu_parity_tab));
        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
        SUB_REG_LSL_IMM5(s3, s1, s2, 0);
        AND_IMM8(s3, s3, 31);
        MVN_REG_LSR_REG(s4, s4, s3);
        AND_IMM8(s4, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_PF]));
    }
}

// emit CMP32 instruction, from cmp s1 , 0, using s3 and s4 as scratch
void emit_cmp32_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    MOVW(s4, 0);
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, op2));
        MOVW(s4, d_cmp32);
    }
    STR_IMM9(s4, xEmu, offsetof(x86emu_t, df)); // reset flags
    SUBS_IMM8(s3, s1, 0);   // res = s1 - 0
    IFX(X_PEND) {
        IFX(X_ZF|X_OF|X_CF|X_AF) {
            MOVW(s4, 0);
        }
    }
    // and now the tricky ones (and mostly unused), PF and AF
    // bc = (res & (~d | s)) | (~d & s) => is 0 here...
    IFX(X_CF) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_CF]));    // CF : bc & 0x80000000
    }
    IFX(X_AF) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_AF]));    // AF: bc & 0x08
    }
    IFX(X_OF) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_ZF) {
        // first the easy flags, also found on ARM
        MOVW_COND(cEQ, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s4, s3, 31, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_SF]));
    }
    IFX(X_PF) {
        // PF: (((emu->x86emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
        AND_IMM8(s3, s1, 0xE0); // lsr 5 masking pre-applied
        LDR_IMM9(s4, xEmu, offsetof(x86emu_t, x86emu_parity_tab));
        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
        AND_IMM8(s3, s1, 31);
        MVN_REG_LSR_REG(s4, s4, s3);
        AND_IMM8(s4, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_PF]));
    }
}

// emit CMP16 instruction, from cmp s1 , s2, using s3 and s4 as scratch
void emit_cmp16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        MOVW(s3, d_cmp16);
    } else {
        MOVW(s3, 0);
    }
    STR_IMM9(s3, xEmu, offsetof(x86emu_t, df)); // reset flags
    SUB_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 - s2
    IFX(X_PEND) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        MOVW(s4, 0xffff);
        TSTS_REG_LSL_IMM5(s3, s4, 0);
        MOVW(s4, 0);
        MOVW_COND(cEQ, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s4, s3, 15, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_SF]));
    }
    // bc = (res & (~d | s)) | (~d & s)
    IFX(X_CF|X_AF|X_OF) {
        MVN_REG_LSL_IMM5(s4, s1, 0);        // s4 = ~d
        ORR_REG_LSL_IMM5(s4, s4, s2, 0);    // s4 = ~d | s
        AND_REG_LSL_IMM5(s4, s4, s3, 0);    // s4 = res & (~d | s)
        BIC_REG_LSL_IMM5(s3, s2, s1, 0);    // loosing res... s3 = s & ~d
        ORR_REG_LSL_IMM5(s3, s4, s3, 0);    // s3 = (res & (~d | s)) | (s & ~d)
        IFX(X_CF) {
            UBFX(s4, s3, 15, 1);
            STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_CF]));    // CF : bc & 0x8000
        }
        IFX(X_AF) {
            TSTS_IMM8_ROR(s3, 0x08, 0);
            MOVW(s4, 0);
            MOVW_COND(cNE, s4, 1);
            STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_AF]));    // AF: bc & 0x08
        }
        IFX(X_OF) {
            MOV_REG_LSR_IMM5(s4, s3, 14);
            XOR_REG_LSR_IMM8(s4, s4, s4, 1);
            AND_IMM8(s4, s4, 1);
            STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_OF]));    // OF: ((bc >> 14) ^ ((bc>>14)>>1)) & 1
        }
    }
    // PF: (((emu->x86emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
    IFX(X_PF) {
        IFX(X_CF|X_AF|X_OF) {
            SUB_REG_LSL_IMM5(s3, s1, s2, 0);
        }
        AND_IMM8(s3, s3, 0xE0); // lsr 5 masking pre-applied
        LDR_IMM9(s4, xEmu, offsetof(x86emu_t, x86emu_parity_tab));
        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
        SUB_REG_LSL_IMM5(s3, s1, s2, 0);
        AND_IMM8(s3, s3, 31);
        MVN_REG_LSR_REG(s4, s4, s3);
        AND_IMM8(s4, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_PF]));
    }
}

// emit CMP16 instruction, from cmp s1 , #0, using s3 and s4 as scratch
void emit_cmp16_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    MOVW(s3, 0);
    IFX(X_PEND) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        MOVW(s3, d_cmp16);
    }
    STR_IMM9(s3, xEmu, offsetof(x86emu_t, df)); // reset flags
    IFX(X_OF|X_CF|X_AF) {
        MOVW(s4, 0);
    }
    // bc = (res & (~d | s)) | (~d & s) = 0
    IFX(X_CF) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_CF]));    // CF : bc & 0x8000
    }
    IFX(X_AF) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_AF]));    // AF: bc & 0x08
    }
    IFX(X_OF) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_OF]));    // OF: ((bc >> 14) ^ ((bc>>14)>>1)) & 1
    }
    IFX(X_ZF) {
        MOVW(s4, 0xffff);
        TSTS_REG_LSL_IMM5(s1, s4, 0);
        MOVW(s4, 0);
        MOVW_COND(cEQ, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s4, s1, 15, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_SF]));
    }
    // PF: (((emu->x86emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
    IFX(X_PF) {
        AND_IMM8(s3, s1, 0xE0); // lsr 5 masking pre-applied
        LDR_IMM9(s4, xEmu, offsetof(x86emu_t, x86emu_parity_tab));
        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
        AND_IMM8(s3, s1, 31);
        MVN_REG_LSR_REG(s4, s4, s3);
        AND_IMM8(s4, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_PF]));
    }
}
// emit CMP8 instruction, from cmp s1 , s2, using s3 and s4 as scratch
void emit_cmp8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        MOVW(s4, d_cmp8);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
    } else {
        MOVW(s4, 0);
    }
    STR_IMM9(s4, xEmu, offsetof(x86emu_t, df)); // reset flags
    SUB_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 - s2
    IFX(X_PEND) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        IFX(X_PEND) {
            MOVW(s4, 0);
        }
        TSTS_IMM8(s3, 0xff);
        MOVW_COND(cEQ, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s4, s3, 7, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_SF]));
    }
    // bc = (res & (~d | s)) | (~d & s)
    IFX(X_CF|X_AF|X_OF) {
        MVN_REG_LSL_IMM5(s4, s1, 0);        // s4 = ~d
        ORR_REG_LSL_IMM5(s4, s4, s2, 0);    // s4 = ~d | s
        AND_REG_LSL_IMM5(s4, s4, s3, 0);    // s4 = res & (~d | s)
        BIC_REG_LSL_IMM5(s3, s2, s1, 0);    // loosing res... s3 = s & ~d
        ORR_REG_LSL_IMM5(s3, s4, s3, 0);    // s3 = (res & (~d | s)) | (s & ~d)
        IFX(X_CF) {
            UBFX(s4, s3, 7, 1);
            STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_CF]));    // CF : bc & 0x80
        }
        IFX(X_AF) {
            UBFX(s4, s3, 3, 1);
            STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_AF]));    // AF: bc & 0x08
        }
        IFX(X_OF) {
            MOV_REG_LSR_IMM5(s4, s3, 6);
            XOR_REG_LSR_IMM8(s4, s4, s4, 1);
            AND_IMM8(s4, s4, 1);
            STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_OF]));    // OF: ((bc >> 6) ^ ((bc>>6)>>1)) & 1
        }
    }
    // PF: (((emu->x86emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
    IFX(X_PF) {
        IFX(X_CF|X_AF|X_OF) {
            SUB_REG_LSL_IMM5(s3, s1, s2, 0);
        }
        AND_IMM8(s3, s3, 0xE0); // lsr 5 masking pre-applied
        LDR_IMM9(s4, xEmu, offsetof(x86emu_t, x86emu_parity_tab));
        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
        SUB_REG_LSL_IMM5(s3, s1, s2, 0);
        AND_IMM8(s3, s3, 31);
        MVN_REG_LSR_REG(s4, s4, s3);
        AND_IMM8(s4, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_PF]));
    }
}
// emit CMP8 instruction, from cmp s1 , 0, using s3 and s4 as scratch
void emit_cmp8_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    MOVW(s4, 0);
    IFX(X_PEND) {
        MOVW(s3, d_cmp8);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, op2));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, df));
    } else {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df)); // reset flags
    }
    // bc = (res & (~d | s)) | (~d & s) = 0
    IFX(X_CF) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_CF]));    // CF : bc & 0x80
    }
    IFX(X_AF) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_AF]));    // AF: bc & 0x08
    }
    IFX(X_OF) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_OF]));    // OF: ((bc >> 6) ^ ((bc>>6)>>1)) & 1
    }
    IFX(X_ZF) {
        TSTS_IMM8(s1, 0xff);
        MOVW_COND(cEQ, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s4, s1, 7, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_SF]));
    }
    // PF: (((emu->x86emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
    IFX(X_PF) {
        AND_IMM8(s3, s1, 0xE0); // lsr 5 masking pre-applied
        LDR_IMM9(s4, xEmu, offsetof(x86emu_t, x86emu_parity_tab));
        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
        AND_IMM8(s3, s1, 31);
        MVN_REG_LSR_REG(s4, s4, s3);
        AND_IMM8(s4, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_PF]));
    }
}

// emit TEST32 instruction, from test s1 , s2, using s3 and s4 as scratch
void emit_test32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MOVW(s4, 0);
    IFX(X_PEND) {
        MOVW(s3, d_tst32);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, df));
    } else {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df)); // reset flags
    }
    IFX(X_OF) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_CF) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    ANDS_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 & s2
    IFX(X_PEND) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s4, s3, 31, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_SF]));
    }
    // PF: (((emu->x86emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
    IFX(X_PF) {
        AND_IMM8(s3, s3, 0xE0); // lsr 5 masking pre-applied
        LDR_IMM9(s4, xEmu, offsetof(x86emu_t, x86emu_parity_tab));
        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
        AND_REG_LSL_IMM5(s3, s1, s2, 0);
        AND_IMM8(s3, s3, 31);
        MVN_REG_LSR_REG(s4, s4, s3);
        AND_IMM8(s4, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_PF]));
    }
}

// emit TEST16 instruction, from test s1 , s2, using s3 and s4 as scratch
void emit_test16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MOVW(s4, 0);
    IFX(X_PEND) {
        MOVW(s3, d_tst16);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, df));
    } else {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df)); // reset flags
    }
    IFX(X_OF) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_CF) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    ANDS_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 & s2
    IFX(X_PEND) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s4, s3, 15, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_SF]));
    }
    // PF: (((emu->x86emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
    IFX(X_PF) {
        AND_IMM8(s3, s3, 0xE0); // lsr 5 masking pre-applied
        LDR_IMM9(s4, xEmu, offsetof(x86emu_t, x86emu_parity_tab));
        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
        AND_REG_LSL_IMM5(s3, s1, s2, 0);
        AND_IMM8(s3, s3, 31);
        MVN_REG_LSR_REG(s4, s4, s3);
        AND_IMM8(s4, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_PF]));
    }
}

// emit TEST8 instruction, from test s1 , s2, using s3 and s4 as scratch
void emit_test8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    MOVW(s4, 0);
    IFX(X_PEND) {
        MOVW(s3, d_tst8);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, df));
    } else {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df)); // reset flags
    }
    IFX(X_OF) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_CF) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    ANDS_REG_LSL_IMM5(s3, s1, s2, 0);   // res = s1 & s2
    IFX(X_PEND) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s4, s3, 7, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_SF]));
    }
    // PF: (((emu->x86emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
    IFX(X_PF) {
        AND_IMM8(s3, s3, 0xE0); // lsr 5 masking pre-applied
        LDR_IMM9(s4, xEmu, offsetof(x86emu_t, x86emu_parity_tab));
        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
        AND_REG_LSL_IMM5(s3, s1, s2, 0);
        AND_IMM8(s3, s3, 31);
        MVN_REG_LSR_REG(s4, s4, s3);
        AND_IMM8(s4, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_PF]));
    }
}
