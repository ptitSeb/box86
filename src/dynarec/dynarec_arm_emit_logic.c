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

// emit OR32 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch
void emit_or32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        MOVW(s4, d_or32);
    } else IFX(X_ALL) {
        MOVW(s4, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df));
    }
    IFX(X_ALL) {
        ORRS_REG_LSL_IMM5(s1, s1, s2, 0);
        IFX(X_CF|X_ZF|X_OF|X_AF) {
            MOVW(s3, 0);
        }
    } else {
        ORR_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
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

// emit OR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_or32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        MOVW(s4, d_or32);
    } else IFX(X_ALL) {
        MOVW(s4, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df));
    }
    if(c>=0 && c<256) {
        IFX(X_ALL) {
            ORRS_IMM8(s1, s1, c, 0);
        } else {
            ORR_IMM8(s1, s1, c, 0);
        }
    } else {
        IFX(X_PEND) {} else {MOV32(s3, c);}
        IFX(X_ALL) {
            ORRS_REG_LSL_IMM5(s1, s1, s3, 0);
        } else {
            ORR_REG_LSL_IMM5(s1, s1, s3, 0);
        }
    }
    IFX(X_CF|X_ZF|X_OF|X_AF) {
        MOVW(s3, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
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

// emit XOR32 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch
void emit_xor32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        MOVW(s4, d_xor32);
    } else IFX(X_ALL) {
        MOVW(s4, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df));
    }
    IFX(X_ALL) {
        XORS_REG_LSL_IMM5(s1, s1, s2, 0);
        IFX(X_CF|X_ZF|X_OF|X_AF) {
            MOVW(s3, 0);
        }
    } else {
        XOR_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
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

// emit XOR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_xor32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        MOVW(s4, d_xor32);
    } else IFX(X_ALL) {
        MOVW(s4, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df));
    }
    if(c>=0 && c<256) {
        IFX(X_ALL) {
            XORS_IMM8(s1, s1, c);
        } else {
            XOR_IMM8(s1, s1, c);
        }
    } else {
        IFX(X_PEND) {} else {MOV32(s3, c);}
        IFX(X_ALL) {
            XORS_REG_LSL_IMM5(s1, s1, s3, 0);
        } else {
            XOR_REG_LSL_IMM5(s1, s1, s3, 0);
        }
    }
    IFX(X_CF|X_ZF|X_OF|X_AF) {
        MOVW(s3, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
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

// emit AND32 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch
void emit_and32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        MOVW(s4, d_and32);
    } else IFX(X_ALL) {
        MOVW(s4, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df));
    }
    IFX(X_ALL) {
        ANDS_REG_LSL_IMM5(s1, s1, s2, 0);
        IFX(X_CF|X_ZF|X_OF|X_AF) {
            MOVW(s3, 0);
        }
    } else {
        AND_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
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

// emit AND32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_and32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        MOVW(s4, d_and32);
    } else IFX(X_ALL) {
        MOVW(s4, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df));
    }
    if(c>=0 && c<256) {
        IFX(X_ALL) {
            ANDS_IMM8(s1, s1, c);
        } else {
            AND_IMM8(s1, s1, c);
        }
    } else {
        IFX(X_PEND) {} else {MOV32(s3, c);}
        IFX(X_ALL) {
            ANDS_REG_LSL_IMM5(s1, s1, s3, 0);
        } else {
            AND_REG_LSL_IMM5(s1, s1, s3, 0);
        }
    }
    IFX(X_CF|X_ZF|X_OF|X_AF) {
        MOVW(s3, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
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

// emit OR8 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch
void emit_or8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int save_s4)
{
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        MOVW(s3, d_or8);
    } else IFX(X_ALL) {
        MOVW(s3, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, df));
    }
    IFX(X_ALL) {
        ORRS_REG_LSL_IMM5(s1, s1, s2, 0);
        IFX(X_CF|X_ZF|X_OF|X_AF) {
            IFX(X_PEND) {
                MOVW(s3, 0);
            }
        }
    } else {
        ORR_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s3, s1, 7, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_SF]));
    }
    IFX(X_PF) {
        if(save_s4) {PUSH(xSP, 1<<s4);}
        // PF: (((emu->x86emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
        AND_IMM8(s3, s1, 0xE0); // lsr 5 masking pre-applied
        LDR_IMM9(s4, xEmu, offsetof(x86emu_t, x86emu_parity_tab));
        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
        AND_IMM8(s3, s1, 31);
        MVN_REG_LSR_REG(s4, s4, s3);
        AND_IMM8(s4, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_PF]));
        if(save_s4) {POP(xSP, 1<<s4);}
    }
}

// emit OR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_or8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        MOVW(s4, d_or8);
    } else IFX(X_ALL) {
        MOVW(s4, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df));
    }
    IFX(X_ALL) {
        ORRS_IMM8(s1, s1, c, 0);
    } else {
        ORR_IMM8(s1, s1, c, 0);
    }
    IFX(X_CF|X_ZF|X_OF|X_AF) {
        MOVW(s3, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s3, s1, 7, 1);
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

// emit XOR8 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch
void emit_xor8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int save_s4)
{
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        MOVW(s3, d_xor8);
    } else IFX(X_ALL) {
        MOVW(s3, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, df));
    }
    IFX(X_ALL) {
        XORS_REG_LSL_IMM5(s1, s1, s2, 0);
        IFX(X_CF|X_ZF|X_OF|X_AF) {
            IFX(X_PEND) {
                MOVW(s3, 0);
            }
        }
    } else {
        XOR_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s3, s1, 7, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_SF]));
    }
    IFX(X_PF) {
        if(save_s4) {PUSH(xSP, 1<<s4);}
        // PF: (((emu->x86emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
        AND_IMM8(s3, s1, 0xE0); // lsr 5 masking pre-applied
        LDR_IMM9(s4, xEmu, offsetof(x86emu_t, x86emu_parity_tab));
        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
        AND_IMM8(s3, s1, 31);
        MVN_REG_LSR_REG(s4, s4, s3);
        AND_IMM8(s4, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_PF]));
        if(save_s4) {POP(xSP, 1<<s4);}
    }
}

// emit XOR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_xor8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        MOVW(s4, d_xor8);
    } else IFX(X_ALL) {
        MOVW(s4, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df));
    }
    IFX(X_ALL) {
        XORS_IMM8(s1, s1, c);
    } else {
        XOR_IMM8(s1, s1, c);
    }
    IFX(X_CF|X_ZF|X_OF|X_AF) {
        MOVW(s3, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s3, s1, 7, 1);
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

// emit AND8 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch
void emit_and8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int save_s4)
{
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        MOVW(s3, d_and8);
    } else IFX(X_ALL) {
        MOVW(s3, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, df));
    }
    IFX(X_ALL) {
        ANDS_REG_LSL_IMM5(s1, s1, s2, 0);
        IFX(X_CF|X_ZF|X_OF|X_AF) {
            IFX(X_PEND) {
                MOVW(s3, 0);
            }
        }
    } else {
        AND_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s3, s1, 7, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_SF]));
    }
    IFX(X_PF) {
        if(save_s4) {PUSH(xSP, 1<<s4);}
        // PF: (((emu->x86emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
        AND_IMM8(s3, s1, 0xE0); // lsr 5 masking pre-applied
        LDR_IMM9(s4, xEmu, offsetof(x86emu_t, x86emu_parity_tab));
        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
        AND_IMM8(s3, s1, 31);
        MVN_REG_LSR_REG(s4, s4, s3);
        AND_IMM8(s4, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_PF]));
        if(save_s4) {POP(xSP, 1<<s4);}
    }
}

// emit AND8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_and8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        MOVW(s4, d_and8);
    } else IFX(X_ALL) {
        MOVW(s4, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df));
    }
    IFX(X_ALL) {
        ANDS_IMM8(s1, s1, c);
    } else {
        AND_IMM8(s1, s1, c);
    }
    IFX(X_CF|X_ZF|X_OF|X_AF) {
        MOVW(s3, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s3, s1, 7, 1);
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


// emit OR16 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch
void emit_or16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int save_s4)
{
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        MOVW(s3, d_or16);
    } else IFX(X_ALL) {
        MOVW(s3, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, df));
    }
    IFX(X_ALL) {
        ORRS_REG_LSL_IMM5(s1, s1, s2, 0);
        IFX(X_CF|X_ZF|X_OF|X_AF) {
            IFX(X_PEND) {
                MOVW(s3, 0);
            }
        }
    } else {
        ORR_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s3, s1, 15, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_SF]));
    }
    IFX(X_PF) {
        if(save_s4) {PUSH(xSP, 1<<s4);}
        // PF: (((emu->x86emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
        AND_IMM8(s3, s1, 0xE0); // lsr 5 masking pre-applied
        LDR_IMM9(s4, xEmu, offsetof(x86emu_t, x86emu_parity_tab));
        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
        AND_IMM8(s3, s1, 31);
        MVN_REG_LSR_REG(s4, s4, s3);
        AND_IMM8(s4, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_PF]));
        if(save_s4) {POP(xSP, 1<<s4);}
    }
}

// emit OR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_or16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        MOVW(s4, d_or16);
    } else IFX(X_ALL) {
        MOVW(s4, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df));
    }
    if(c>=0 && c<256) {
        IFX(X_ALL) {
            ORRS_IMM8(s1, s1, c, 0);
        } else {
            ORR_IMM8(s1, s1, c, 0);
        }
    } else {
        IFX(X_PEND) {} else {MOV32(s3, c);}
        IFX(X_ALL) {
            ORRS_REG_LSL_IMM5(s1, s1, s3, 0);
        } else {
            ORR_REG_LSL_IMM5(s1, s1, s3, 0);
        }
    }
    IFX(X_CF|X_ZF|X_OF|X_AF) {
        MOVW(s3, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s3, s1, 15, 1);
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

// emit XOR16 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch
void emit_xor16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int save_s4)
{
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        MOVW(s3, d_xor16);
    } else IFX(X_ALL) {
        MOVW(s3, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, df));
    }
    IFX(X_ALL) {
        XORS_REG_LSL_IMM5(s1, s1, s2, 0);
        IFX(X_CF|X_ZF|X_OF|X_AF) {
            IFX(X_PEND) {
                MOVW(s3, 0);
            }
        }
    } else {
        XOR_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s3, s1, 15, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_SF]));
    }
    IFX(X_PF) {
        if(save_s4) {PUSH(xSP, 1<<s4);}
        // PF: (((emu->x86emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
        AND_IMM8(s3, s1, 0xE0); // lsr 5 masking pre-applied
        LDR_IMM9(s4, xEmu, offsetof(x86emu_t, x86emu_parity_tab));
        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
        AND_IMM8(s3, s1, 31);
        MVN_REG_LSR_REG(s4, s4, s3);
        AND_IMM8(s4, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_PF]));
        if(save_s4) {POP(xSP, 1<<s4);}
    }
}

// emit XOR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_xor16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        MOVW(s4, d_xor16);
    } else IFX(X_ALL) {
        MOVW(s4, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df));
    }
    if(c>=0 && c<256) {
        IFX(X_ALL) {
            XORS_IMM8(s1, s1, c);
        } else {
            XOR_IMM8(s1, s1, c);
        }
    } else {
        IFX(X_PEND) {} else {MOV32(s3, c);}
        IFX(X_ALL) {
            XORS_REG_LSL_IMM5(s1, s1, s3, 0);
        } else {
            XOR_REG_LSL_IMM5(s1, s1, s3, 0);
        }
    }
    IFX(X_CF|X_ZF|X_OF|X_AF) {
        MOVW(s3, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s3, s1, 15, 1);
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

// emit AND16 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch
void emit_and16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int save_s4)
{
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        MOVW(s3, d_and16);
    } else IFX(X_ALL) {
        MOVW(s3, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, df));
    }
    IFX(X_ALL) {
        ANDS_REG_LSL_IMM5(s1, s1, s2, 0);
        IFX(X_CF|X_ZF|X_OF|X_AF) {
            IFX(X_PEND) {
                MOVW(s3, 0);
            }
        }
    } else {
        AND_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s3, s1, 15, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_SF]));
    }
    IFX(X_PF) {
        if(save_s4) {PUSH(xSP, 1<<s4);}
        // PF: (((emu->x86emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
        AND_IMM8(s3, s1, 0xE0); // lsr 5 masking pre-applied
        LDR_IMM9(s4, xEmu, offsetof(x86emu_t, x86emu_parity_tab));
        LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
        AND_IMM8(s3, s1, 31);
        MVN_REG_LSR_REG(s4, s4, s3);
        AND_IMM8(s4, s4, 1);
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, flags[F_PF]));
        if(save_s4) {POP(xSP, 1<<s4);}
    }
}

// emit AND16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_and16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        MOVW(s4, d_and16);
    } else IFX(X_ALL) {
        MOVW(s4, d_none);
    }
    IFX(X_PEND|X_ALL) {
        STR_IMM9(s4, xEmu, offsetof(x86emu_t, df));
    }
    if(c>=0 && c<256) {
        IFX(X_ALL) {
            ANDS_IMM8(s1, s1, c);
        } else {
            AND_IMM8(s1, s1, c);
        }
    } else {
        IFX(X_ALL) {} else {MOVW(s3, c);}
        IFX(X_ALL) {
            ANDS_REG_LSL_IMM5(s1, s1, s3, 0);
        } else {
            AND_REG_LSL_IMM5(s1, s1, s3, 0);
        }
    }
    IFX(X_CF|X_ZF|X_OF|X_AF) {
        MOVW(s3, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_CF]));
    }
    IFX(X_OF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_OF]));
    }
    IFX(X_AF) {
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_AF]));
    }
    IFX(X_ZF) {
        MOVW_COND(cEQ, s3, 1);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
    }
    IFX(X_SF) {
        UBFX(s3, s1, 16, 1);
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