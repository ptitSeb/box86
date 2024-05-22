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
        SET_DF(s4, d_or32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_ALL) {
        ORRS_REG_LSL_IMM5(s1, s1, s2, 0);
    } else {
        ORR_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 31);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit OR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_or32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, d_or32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c>=0 && c<256) {
        IFX(X_ALL) {
            ORRS_IMM8(s1, s1, c, 0);
        } else {
            ORR_IMM8(s1, s1, c, 0);
        }
    } else {
        MOV32(s3, c);
        IFX(X_ALL) {
            ORRS_REG_LSL_IMM5(s1, s1, s3, 0);
        } else {
            ORR_REG_LSL_IMM5(s1, s1, s3, 0);
        }
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 31);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit XOR32 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch
void emit_xor32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, d_xor32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_ALL) {
        XORS_REG_LSL_IMM5(s1, s1, s2, 0);
    } else {
        XOR_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 31);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit XOR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_xor32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, d_xor32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c>=0 && c<256) {
        IFX(X_ALL) {
            XORS_IMM8(s1, s1, c);
        } else {
            XOR_IMM8(s1, s1, c);
        }
    } else {
        MOV32(s3, c);
        IFX(X_ALL) {
            XORS_REG_LSL_IMM5(s1, s1, s3, 0);
        } else {
            XOR_REG_LSL_IMM5(s1, s1, s3, 0);
        }
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 31);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit AND32 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch
void emit_and32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, d_and32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_ALL) {
        ANDS_REG_LSL_IMM5(s1, s1, s2, 0);
    } else {
        AND_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 31);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit AND32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_and32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, d_and32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c>=0 && c<256) {
        IFX(X_ALL) {
            ANDS_IMM8(s1, s1, c);
        } else {
            AND_IMM8(s1, s1, c);
        }
    } else {
        if(((~c)>=0 && (~c)<256)) {
            IFX(X_ALL) {
                BICS_IMM8_ROR(s1, s1, ~c, 0);
            } else {
                BIC_IMM8(s1, s1, ~c, 0);
            }
        } else {
            MOV32(s3, c);
            IFX(X_ALL) {
                ANDS_REG_LSL_IMM5(s1, s1, s3, 0);
            } else {
                AND_REG_LSL_IMM5(s1, s1, s3, 0);
            }
        }
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 31);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit OR8 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_or8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s3, d_or8);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_ALL) {
        ORRS_REG_LSL_IMM5(s1, s1, s2, 0);
    } else {
        ORR_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 7);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit OR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_or8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, d_or8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_ALL) {
        ORRS_IMM8(s1, s1, c, 0);
    } else {
        ORR_IMM8(s1, s1, c, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 7);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit XOR8 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_xor8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s3, d_xor8);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_ALL) {
        XORS_REG_LSL_IMM5(s1, s1, s2, 0);
    } else {
        XOR_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 7);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit XOR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_xor8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, d_xor8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_ALL) {
        XORS_IMM8(s1, s1, c);
    } else {
        XOR_IMM8(s1, s1, c);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 7);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit AND8 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_and8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s3, d_and8);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_ALL) {
        ANDS_REG_LSL_IMM5(s1, s1, s2, 0);
    } else {
        AND_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 7);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit AND8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_and8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, d_and8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_ALL) {
        ANDS_IMM8(s1, s1, c);
    } else {
        AND_IMM8(s1, s1, c);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 7);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}


// emit OR16 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_or16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s3, d_or16);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_ALL) {
        ORRS_REG_LSL_IMM5(s1, s1, s2, 0);
    } else {
        ORR_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 15);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit OR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_or16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, d_or16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c>=0 && c<256) {
        IFX(X_ALL) {
            ORRS_IMM8(s1, s1, c, 0);
        } else {
            ORR_IMM8(s1, s1, c, 0);
        }
    } else {
        MOVW(s3, c);
        IFX(X_ALL) {
            ORRS_REG_LSL_IMM5(s1, s1, s3, 0);
        } else {
            ORR_REG_LSL_IMM5(s1, s1, s3, 0);
        }
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 15);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit XOR16 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_xor16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s3, d_xor16);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_ALL) {
        XORS_REG_LSL_IMM5(s1, s1, s2, 0);
    } else {
        XOR_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 15);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit XOR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_xor16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, d_xor16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c>=0 && c<256) {
        IFX(X_ALL) {
            XORS_IMM8(s1, s1, c);
        } else {
            XOR_IMM8(s1, s1, c);
        }
    } else {
        MOVW(s3, c);
        IFX(X_ALL) {
            XORS_REG_LSL_IMM5(s1, s1, s3, 0);
        } else {
            XOR_REG_LSL_IMM5(s1, s1, s3, 0);
        }
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 15);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit AND16 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch, s4 can be same as s2 (and so s2 destroyed)
void emit_and16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s3, d_and16);
    } else IFX(X_ALL) {
        SET_DFNONE(s3);
    }
    IFX(X_ALL) {
        ANDS_REG_LSL_IMM5(s1, s1, s2, 0);
    } else {
        AND_REG_LSL_IMM5(s1, s1, s2, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 15);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit AND16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_and16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        SET_DF(s4, d_and16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c>=0 && c<256) {
        IFX(X_ALL) {
            ANDS_IMM8(s1, s1, c);
        } else {
            AND_IMM8(s1, s1, c);
        }
    } else {
        MOVW(s3, c);
        IFX(X_ALL) {
            ANDS_REG_LSL_IMM5(s1, s1, s3, 0);
        } else {
            AND_REG_LSL_IMM5(s1, s1, s3, 0);
        }
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF | X_AF | X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_CF)|(1<<F_AF)|(1<<F_ZF), 0);
    }
    IFX(X_OF) {
        BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 15);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}