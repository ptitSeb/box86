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

// emit SHL32 instruction, from s1 , shift s2, store result in s1 using s3 and s4 as scratch. s3 can be same as s2
void emit_shl32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    int32_t j32;
    MAYUSE(j32);

    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_shl32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(F_OF) {
        CMPS_IMM8(s2, 0);
        IFX(F_OF) {
            BIC_IMM8_COND(cEQ, xFlags, xFlags, 0b10, 0x0b);
        }
        IFX(X_PEND) {
            STR_IMM9_COND(cEQ, s1, xEmu, offsetof(x86emu_t, res));
        }
        B_NEXT(cEQ);
    }
    IFX(X_CF | X_OF) {
        RSB_IMM8(s4, s2, 32);
        MOV_REG_LSR_REG(s4, s1, s4);
        BFI(xFlags, s4, F_CF, 1);
    }
    IFX(X_ZF) {
        MOVS_REG_LSL_REG(s1, s1, s2);
    } else {
        MOV_REG_LSL_REG(s1, s1, s2);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
        BIC_IMM8_COND(cNE, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s1, 31);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        CMPS_IMM8(s2, 1);   // if s3==1
            IFX(X_SF) {} else {MOV_REG_LSR_IMM5_COND(cEQ, s4, s1, 31);}
            XOR_REG_LSL_IMM5_COND(cEQ, s4, s4, xFlags, 0);  // CF is set if OF is asked
            BFI_COND(cEQ, xFlags, s4, F_OF, 1);
        // else
            BFC_COND(cNE, xFlags, F_OF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHL32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shl32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOVW(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_shl32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c==0) {
        IFX(F_OF) {
            BIC_IMM8(xFlags, xFlags, 0b10, 0x0b);
        }
        IFX(X_PEND) {
            STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        }
        return;
    }
    IFX(X_CF) {
        MOV_REG_LSR_IMM5(s3, s1, 32-c);
        BFI(xFlags, s3, F_CF, 1);
    }
    IFX(X_ZF|X_OF) {
        MOVS_REG_LSL_IMM5(s1, s1, c);
    } else {
        MOV_REG_LSL_IMM5(s1, s1, c);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
        BIC_IMM8_COND(cNE, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 31);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            IFX(X_SF) {} else {MOV_REG_LSR_IMM5(s3, s1, 31);}
            XOR_IMM8_COND(cCS, s3, s3, 1);
            BFI(xFlags, s3, F_OF, 1);
        } else {
            BFC(xFlags, F_OF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHR32 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch, s2 can be same as s3
void emit_shr32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    int32_t j32;
    MAYUSE(j32);

    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_shr32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_ALL) {
        CMPS_IMM8(s2, 0); //if(!c)
            IFX(X_PEND) {
                STR_IMM9_COND(cEQ, s1, xEmu, offsetof(x86emu_t, res));
            }
            B_NEXT(cEQ);
    }
    IFX(X_ZF|X_CF) {
        MOVS_REG_LSR_REG(s1, s1, s2);
    } else {
        MOV_REG_LSR_REG(s1, s1, s2);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF|X_CF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_ZF)|(1<<F_CF), 0);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_CF) {
        ORR_IMM8_COND(cCS, xFlags, xFlags, 1<<F_CF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s1, 31);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        CMPS_IMM8(s2, 1);
            MOV_REG_LSR_IMM5_COND(cEQ, s4, s1, 30);
            XOR_REG_LSR_IMM8_COND(cEQ, s4, s4, s4, 1);
            BFI_COND(cEQ, xFlags, s4, F_OF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SHR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shr32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOVW(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_shr32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        }
        return;
    }
    IFX(X_ZF|X_CF) {
        MOVS_REG_LSR_IMM5(s1, s1, c);
    } else {
        MOV_REG_LSR_IMM5(s1, s1, c);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF|X_CF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_ZF)|(1<<F_CF), 0);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_CF) {
        ORR_IMM8_COND(cCS, xFlags, xFlags, 1<<F_CF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 31);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            MOV_REG_LSR_IMM5(s4, s1, 30);
            XOR_REG_LSR_IMM8(s4, s4, s4, 1);
            BFI(xFlags, s4, F_OF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit SAR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_sar32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOVW(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_sar32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        }
        return;
    }
    IFX(X_ZF|X_CF) {
        MOVS_REG_ASR_IMM5(s1, s1, c);
    } else {
        MOV_REG_ASR_IMM5(s1, s1, c);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF|X_CF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_ZF)|(1<<F_CF), 0);
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_CF) {
        ORR_IMM8_COND(cCS, xFlags, xFlags, 1<<F_CF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 31);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

// emit ROL32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rol32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOVW(s3, c);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_rol32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        }
        return;
    }
    MOV_REG_ROR_IMM5(s1, s1, 32-c);
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        BFI(xFlags, s1, F_CF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            ADD_REG_LSR_IMM5(s3, s1, s1, 31);
            AND_IMM8(s3, s3, 1);
            BFI(xFlags, s3, F_OF, 1);
        }
    }
}

// emit ROR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_ror32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOVW(s3, c);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_ror32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        }
        return;
    }
    MOV_REG_ROR_IMM5(s1, s1, c);
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        MOV_REG_LSR_IMM5(s3, s1, 31);
        BFI(xFlags, s3, F_CF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            MOV_REG_LSR_IMM5(s4, s1, 30);
            XOR_REG_LSR_IMM8(s4, s4, s4, 1);
            BFI(xFlags, s4, F_OF, 1);
        }
    }
}

// emit SHRD32 instruction, from s1, fill s2 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shrd32c(dynarec_arm_t* dyn, int ninst, int s1, int s2, int32_t c, int s3, int s4)
{
    c&=0x1f;
    IFX(X_PEND) {
        MOVW(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        // same flags calc as shr32
        SET_DF(s4, d_shr32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        }
        return;
    }
    IFX(X_CF) {
        MOVS_REG_LSR_IMM5(s1, s1, c);
    } else {
        MOV_REG_LSR_IMM5(s1, s1, c);
    }
    IFX(X_ZF|X_CF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_ZF)|(1<<F_CF), 0);
    }
    IFX(X_CF) {
        ORR_IMM8_COND(cCS, xFlags, xFlags, 1<<F_CF, 0);
    }
    IFX(X_ZF) {
        ORRS_REG_LSL_IMM5(s1, s1, s2, 32-c);
    } else {
        ORR_REG_LSL_IMM5(s1, s1, s2, 32-c);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 31);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            MOV_REG_LSR_IMM5(s4, s1, 30);
            XOR_REG_LSR_IMM8(s4, s4, s4, 1);
            BFI(xFlags, s4, F_OF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}

void emit_shld32c(dynarec_arm_t* dyn, int ninst, int s1, int s2, int32_t c, int s3, int s4)
{
    c&=0x1f;
    IFX(X_PEND) {
        MOVW(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        // same flags computation as with shl32
        SET_DF(s4, d_shl32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c==0) {
        IFX(F_OF) {
            BFC(xFlags, F_OF, 1);
        }
        IFX(X_PEND) {
            STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        }
        return;
    }
    IFX(X_CF) {
        MOV_REG_LSR_IMM5(s3, s1, 32-c);
        BFI(xFlags, s3, F_CF, 1);
    }
    IFX(X_OF) {
        MOVS_REG_LSL_IMM5(s1, s1, c);
    } else {
        MOV_REG_LSL_IMM5(s1, s1, c);
    }
    IFX(X_OF) {
        if(c==1) {
            UBFX(s3, s2, 0, 1);
            XOR_IMM8_COND(cCS, s3, s3, 1);
            BFI(xFlags, s3, F_OF, 1);
        } else {
            BFC(xFlags, F_OF, 1);
        }
    }
    IFX(X_ZF) {
        ORRS_REG_LSR_IMM5(s1, s1, s2, 32-c);
    } else {
        ORR_REG_LSR_IMM5(s1, s1, s2, 32-c);
    }

    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
        BIC_IMM8_COND(cNE, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s3, s1, 31);
        BFI(xFlags, s3, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s3, s4);
    }
}
