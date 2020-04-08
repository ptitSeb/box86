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

// emit ADD32 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch
void emit_add32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        MOVW(s3, d_add32);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
    } else IFX(X_ALL) {
        MOVW(s3, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, df));
    }
    IFX(X_AF) {
        ORR_REG_LSL_IMM8(s3, s1, s2, 0);    // s3 = op1 | op2
        AND_REG_LSL_IMM5(s4, s1, s2, 0);    // s4 = op1 & op2
    }
    IFX(X_ALL) {
        ADDS_REG_LSL_IMM5(s1, s1, s2, 0);
    } else {
        ADD_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_AF) {
        BIC_REG_LSL_IMM8(s3, s3, s1, 0);   // s3 = (op1 | op2) & ~ res
        ORR_REG_LSL_IMM8(s3, s3, s4, 0);   // s4 = (op1 & op2) | ((op1 | op2) & ~ res)
        UBFX(s4, s3, 3, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_AF]));    // AF: bc & 0x08
    }
    IFX(X_ZF) {
        MOVW_COND(cNE, s3, 0);
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_CF) {
        MOVW_COND(cCS, s4, 1);
        MOVW_COND(cCC, s4, 0);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        MOVW_COND(cVC, s4, 0);
        MOVW_COND(cVS, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_SF) {
        UBFX(s3, s1, 31, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_SF]));
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

// emit ADD32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_add32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        MOVW(s4, d_add32);
    } else IFX(X_ALL) {
        MOVW(s4, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df));
    }
    IFX(X_AF) {
        if(c>=0 && c<256) {
            ORR_IMM8(s3, s1, c, 0);             // s3 = op1 | op2
            AND_IMM8(s4, s1, c);                // s4 = op1 & op2
        } else {
            MOV32(s3, c);
            MOV_REG(s4, s3);
            ORR_REG_LSL_IMM8(s3, s1, s3, 0);
            AND_REG_LSL_IMM5(s4, s1, s4, 0);
            PUSH(xSP, 1<<s3);
        }
    }
    if(c>=0 && c<256) {
        IFX(X_ALL) {
            ADDS_IMM8(s1, s1, c);
        } else {
            ADD_IMM8(s1, s1, c);
        }
    } else {
        MOV32(s3, c);
        IFX(X_ALL) {
            ADDS_REG_LSL_IMM5(s1, s1, s3, 0);
        } else {
            ADD_REG_LSL_IMM5(s1, s1, s3, 0);
        }
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_AF) {
        if(c<0 || c>=256) {
            POP(xSP, 1<<s3);
        }
        BIC_REG_LSL_IMM8(s3, s3, s1, 0);   // s3 = (op1 | op2) & ~ res
        ORR_REG_LSL_IMM8(s3, s3, s4, 0);   // s4 = (op1 & op2) | ((op1 | op2) & ~ res)
        UBFX(s4, s3, 3, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_AF]));    // AF: bc & 0x08
    }
    IFX(X_ZF) {
        MOVW_COND(cNE, s3, 0);
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_CF) {
        MOVW_COND(cCS, s4, 1);
        MOVW_COND(cCC, s4, 0);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        MOVW_COND(cVC, s4, 0);
        MOVW_COND(cVS, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_SF) {
        UBFX(s3, s1, 31, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_SF]));
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

// emit SUB32 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch
void emit_sub32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        MOVW(s3, d_sub32);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
    } else IFX(X_ALL) {
        MOVW(s3, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, df));
    }
    IFX(X_AF) {
        MVN_REG_LSL_IMM8(s3, s1, 0);
        ORR_REG_LSL_IMM8(s3, s3, s2, 0);    // s3 = ~op1 | op2
        BIC_REG_LSL_IMM8(s4, s2, s1, 0);    // s4 = ~op1 & op2
    }
    IFX(X_ALL) {
        SUBS_REG_LSL_IMM8(s1, s1, s2, 0);
    } else {
        SUB_REG_LSL_IMM8(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_AF) {
        BIC_REG_LSL_IMM8(s3, s3, s1, 0);   // s3 = (~op1 | op2) & ~ res
        ORR_REG_LSL_IMM8(s3, s3, s4, 0);   // s4 = (~op1 & op2) | ((~op1 | op2) & ~ res)
        UBFX(s4, s3, 3, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_AF]));    // AF: bc & 0x08
    }
    IFX(X_ZF) {
        MOVW_COND(cNE, s3, 0);
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_CF) {
        MOVW_COND(cCS, s4, 0);
        MOVW_COND(cCC, s4, 1);  // Carry in inverted when substracting?
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        MOVW_COND(cVC, s4, 0);
        MOVW_COND(cVS, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_SF) {
        UBFX(s3, s1, 31, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_SF]));
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

// emit SUB32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_sub32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        MOVW(s4, d_sub32);
    } else IFX(X_ALL) {
        MOVW(s4, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df));
    }
    IFX(X_AF) {
        if(c>=0 && c<256) {
            MVN_REG_LSL_IMM8(s3, s1, 0);
            AND_IMM8(s4, s3, c);                // s4 = ~op1 & op2
            ORR_IMM8(s3, s3, c, 0);             // s3 = ~op1 | op2
        } else {
            MOV32(s3, c);
            MVN_REG_LSL_IMM8(s4, s1, 0);
            ORR_REG_LSL_IMM8(s3, s4, s3, 0);
            MOV32(s4, c);
            BIC_REG_LSL_IMM8(s4, s4, s1, 0);
            PUSH(xSP, 1<<s3);
        }
    }
    if(c>=0 && c<256) {
        IFX(X_ALL) {
            SUBS_IMM8(s1, s1, c);
        } else {
            SUB_IMM8(s1, s1, c);
        }
    } else {
        MOV32(s3, c);
        IFX(X_ALL) {
            SUBS_REG_LSL_IMM8(s1, s1, s3, 0);
        } else {
            SUB_REG_LSL_IMM8(s1, s1, s3, 0);
        }
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_AF) {
        if(c<0 || c>=256) {
            POP(xSP, 1<<s3);
        }
        BIC_REG_LSL_IMM8(s3, s3, s1, 0);   // s3 = (~op1 | op2) & ~ res
        ORR_REG_LSL_IMM8(s3, s3, s4, 0);   // s4 = (~op1 & op2) | ((~op1 | op2) & ~ res)
        UBFX(s4, s3, 3, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_AF]));    // AF: bc & 0x08
    }
    IFX(X_ZF) {
        MOVW_COND(cNE, s3, 0);
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_CF) {
        MOVW_COND(cCS, s4, 0);
        MOVW_COND(cCC, s4, 1);  // Carry is inverted with Sub?
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        MOVW_COND(cVC, s4, 0);
        MOVW_COND(cVS, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_SF) {
        UBFX(s3, s1, 31, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_SF]));
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
