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
        emit_pf(dyn, ninst, s1, s4);
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
        emit_pf(dyn, ninst, s1, s4);
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
    IFX(X_CF) {
        SUB_IMM8(s4, s2, 1);
        MOV_REG_LSR_REG(s4, s1, s4);
        BFI(xFlags, s4, F_CF, 1);
    }
    IFX(X_OF) {
        CMPS_IMM8(s2, 1);   // if s2==1
            MOV_REG_LSR_IMM5_COND(cEQ, s4, s1, 31);
            BFI_COND(cEQ, xFlags, s4, F_OF, 1);
    }
    IFX(X_ZF) {
        MOVS_REG_LSR_REG(s1, s1, s2);
    } else {
        MOV_REG_LSR_REG(s1, s1, s2);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_ZF), 0);
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s1, 31);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
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
    IFX(X_CF) {
        if(c==1) {
            BFI(xFlags, s1, F_CF, 1);
        } else {
            MOV_REG_LSR_IMM5(s3, s1, c-1);
            BFI(xFlags, s3, F_CF, 1);
        }
    }
    IFX(X_OF) {
        if(c==1) {
            MOV_REG_LSR_IMM5(s4, s1, 31);
            BFI(xFlags, s4, F_OF, 1);
        }
    }
    IFX(X_ZF) {
        MOVS_REG_LSR_IMM5(s1, s1, c);
    } else {
        MOV_REG_LSR_IMM5(s1, s1, c);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_ZF), 0);
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
    IFX(X_OF)
        if(c==1) {
            BFC(xFlags, F_OF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHL8 instruction, from s1 , shift s2, store result in s1 using s3 and s4 as scratch. s3 can be same as s2
void emit_shl8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_shl8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF | X_OF) {
        RSB_IMM8(s4, s2, 8);
        MOV_REG_LSR_REG(s4, s1, s4);
        BFI(xFlags, s4, F_CF, 1);
    }
    MOV_REG_LSL_REG(s1, s1, s2);
    IFX(X_ZF) {
        ANDS_IMM8(s1, s1, 0xff);
    } else IFX(X_PEND) {
        AND_IMM8(s1, s1, 0xff);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        MOV32(s4, 1);
        BFI_COND(cEQ, xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s1, 7);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        CMPS_IMM8(s2, 1);   // if s2==1
            IFX(X_SF) {} else {MOV_REG_LSR_IMM5_COND(cEQ, s4, s1, 7);}
            XOR_REG_LSL_IMM5_COND(cEQ, s4, s4, xFlags, 0);  // CF is set if OF is asked
            BFI_COND(cEQ, xFlags, s4, F_OF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHL8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shl8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    if(!c)
        return;
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_shl8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c<8) {
        IFX(X_CF|X_OF) {
            MOV_REG_LSR_IMM5(s3, s1, 8-c);
            BFI(xFlags, s3, F_CF, 1);
        }
        MOV_REG_LSL_IMM5(s1, s1, c);

        IFX(X_PEND) {
            AND_IMM8(s1, s1, 0xff);
            STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        }
        IFX(X_ZF) {
            TSTS_IMM8(s1, 0xff);
            MOVW_COND(cNE, s4, 0);
            MOVW_COND(cEQ, s4, 1);
            BFI(xFlags, s4, F_ZF, 1);
        }
        IFX(X_SF) {
            MOV_REG_LSR_IMM5(s4, s1, 7);
            BFI(xFlags, s4, F_SF, 1);
        }
        IFX(X_OF) {
            if(c==1) {
                IFX(X_SF) {} else {MOV_REG_LSR_IMM5(s4, s1, 7);}
                XOR_REG_LSL_IMM5(s4, s4, xFlags, 0);  // CF is set if OF is asked
                BFI(xFlags, s4, F_OF, 1);
            } else {
                BFC(xFlags, F_OF, 1);
            }
        }
        IFX(X_PF) {
            emit_pf(dyn, ninst, s1, s4);
        }
    } else {
        IFX(X_CF) {
            MOV_REG_LSL_IMM5(s3, s1, c-1);
            MOV_REG_LSR_IMM5(s4, s3, 7);
            BFI(xFlags, s4, F_CF, 1);
        }
        MOVW(s1, 0);
        IFX(X_OF) {
            BFC(xFlags, F_OF, 1);
        }
        IFX(X_SF) {
            BFC(xFlags, F_SF, 1);
        }
        IFX(X_PF | X_ZF) {
            MOVW(s3, 1);
            IFX(X_ZF) {
                BFI(xFlags, s3, F_ZF, 1);
            }
            IFX(X_PF) {
                BFI(xFlags, s3, F_PF, 1);
            }
        }
    }
}

// emit SHR8 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch, s2 can be same as s3
void emit_shr8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_shr8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        SUB_IMM8(s4, s2, 1);
        MOV_REG_LSR_REG(s4, s1, s4);
        BFI(xFlags, s4, 0, 1);
    }
    IFX(X_OF) {
        CMPS_IMM8(s2, 1);   // if s2==1
            MOV_REG_LSR_IMM5_COND(cEQ, s4, s1, 7);
            BFI_COND(cEQ, xFlags, s4, F_OF, 1);
    }
    MOV_REG_LSR_REG(s1, s1, s2);
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        TSTS_IMM8(s1, 0xff);
        MOVW_COND(cNE, s4, 0);
        MOVW_COND(cEQ, s4, 1);
        BFI(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s1, 7);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shr8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    if(!c)
        return;
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_shr8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        if(c==1) {
            BFI(xFlags, s1, 0, 1);
        } else {
            MOV_REG_LSR_IMM5(s3, s1, c-1);
            BFI(xFlags, s3, 0, 1);
        }
    }
    IFX(X_OF) {
        if(c==1) {
            MOV_REG_LSR_IMM5(s4, s1, 7);
            BFI(xFlags, s4, F_OF, 1);
        }
    }
    MOV_REG_LSR_IMM5(s1, s1, c);
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        TSTS_IMM8(s1, 0xff);
        MOVW_COND(cNE, s4, 0);
        MOVW_COND(cEQ, s4, 1);
        BFI(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s1, 7);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SAR8 instruction, from s1 , shift s2, store result in s1 using s3 and s4 as scratch, s2 can be same as s3
void emit_sar8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_sar8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        SUB_IMM8(s4, s2, 1);
        MOV_REG_ASR_REG(s4, s1, s4);
        BFI(xFlags, s4, 0, 1);
    }
    MOV_REG_ASR_REG(s1, s1, s2);
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        TSTS_IMM8(s1, 0xff);
        MOVW_COND(cNE, s4, 0);
        MOVW_COND(cEQ, s4, 1);
        BFI(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s1, 7);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        CMPS_IMM8(s2, 1);
            BFC_COND(cEQ, xFlags, F_OF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SAR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_sar8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    if(!c)
        return;
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_sar8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        MOV_REG_ASR_IMM5(s3, s1, c-1);
        BFI(xFlags, s3, 0, 1);
    }
    MOV_REG_ASR_IMM5(s1, s1, c);
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        TSTS_IMM8(s1, 0xff);
        MOVW_COND(cNE, s4, 0);
        MOVW_COND(cEQ, s4, 1);
        BFI(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s1, 7);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF)
        if(c==1) {
            BFC(xFlags, F_OF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHL16 instruction, from s1 , shift s2, store result in s1 using s3 and s4 as scratch. s3 can be same as s2
void emit_shl16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_shl16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF | X_OF) {
        RSB_IMM8(s4, s2, 16);
        MOV_REG_LSR_REG(s4, s1, s4);
        BFI(xFlags, s4, F_CF, 1);
    }
    MOV_REG_LSL_REG(s1, s1, s2);
    UBFX(s1, s1, 0, 16);
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        TSTS_REG_LSL_IMM5(s1, s1, 0);
        MOV32(s4, 1);
        BFI_COND(cEQ, xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s1, 15);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        CMPS_IMM8(s2, 1);   // if s2==1
            IFX(X_SF) {} else {MOV_REG_LSR_IMM5_COND(cEQ, s4, s1, 15);}
            XOR_REG_LSL_IMM5_COND(cEQ, s4, s4, xFlags, 0);  // CF is set if OF is asked
            BFI_COND(cEQ, xFlags, s4, F_OF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHL16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shl16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    if(!c)
        return;
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_shl16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(c<16) {
        IFX(X_CF|X_OF) {
            MOV_REG_LSR_IMM5(s3, s1, 16-c);
            BFI(xFlags, s3, F_CF, 1);
        }
        MOV_REG_LSL_IMM5(s1, s1, c);

        UBFX(s1, s1, 0, 16);
        IFX(X_PEND) {
            STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        }
        IFX(X_ZF) {
            TSTS_REG_LSL_IMM5(s1, s1, 0);
            MOVW_COND(cNE, s4, 0);
            MOVW_COND(cEQ, s4, 1);
            BFI(xFlags, s4, F_ZF, 1);
        }
        IFX(X_SF) {
            MOV_REG_LSR_IMM5(s4, s1, 15);
            BFI(xFlags, s4, F_SF, 1);
        }
        IFX(X_OF) {
            if(c==1) {
                IFX(X_SF) {} else {MOV_REG_LSR_IMM5(s4, s1, 15);}
                XOR_REG_LSL_IMM5(s4, s4, xFlags, 0);  // CF is set if OF is asked
                BFI(xFlags, s4, F_OF, 1);
            } else {
                BFC(xFlags, F_OF, 1);
            }
        }
        IFX(X_PF) {
            emit_pf(dyn, ninst, s1, s4);
        }
    } else {
        IFX(X_CF) {
            MOV_REG_LSL_IMM5(s3, s1, c-1);
            MOV_REG_LSR_IMM5(s4, s3, 15);
            BFI(xFlags, s4, F_CF, 1);
        }
        MOVW(s1, 0);
        IFX(X_OF) {
            BFC(xFlags, F_OF, 1);
        }
        IFX(X_SF) {
            BFC(xFlags, F_SF, 1);
        }
        IFX(X_PF | X_ZF) {
            MOVW(s3, 1);
            IFX(X_ZF) {
                BFI(xFlags, s3, F_ZF, 1);
            }
            IFX(X_PF) {
                BFI(xFlags, s3, F_PF, 1);
            }
        }
    }
}

// emit SHR16 instruction, from s1 , s2, store result in s1 using s3 and s4 as scratch, s2 can be same as s3
void emit_shr16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_shr16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        SUB_IMM8(s4, s2, 1);
        MOV_REG_LSR_REG(s4, s1, s4);
        BFI(xFlags, s4, 0, 1);
    }
    IFX(X_OF) {
        CMPS_IMM8(s2, 1);   // if s2==1
            MOV_REG_LSR_IMM5_COND(cEQ, s4, s1, 16);
            BFI_COND(cEQ, xFlags, s4, F_OF, 1);
    }
    MOV_REG_LSR_REG(s1, s1, s2);
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        TSTS_REG_LSL_IMM5(s1, s1, 0);
        MOVW_COND(cNE, s4, 0);
        MOVW_COND(cEQ, s4, 1);
        BFI(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s1, 15);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_shr16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    if(!c)
        return;
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_shr16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        if(c==1) {
            BFI(xFlags, s1, 0, 1);
        } else {
            MOV_REG_LSR_IMM5(s3, s1, c-1);
            BFI(xFlags, s3, 0, 1);
        }
    }
    IFX(X_OF) {
        if(c==1) {
            MOV_REG_LSR_IMM5(s4, s1, 15);
            BFI(xFlags, s4, F_OF, 1);
        }
    }
    MOV_REG_LSR_IMM5(s1, s1, c);
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        TSTS_REG_LSL_IMM5(s1, s1, 0);
        MOVW_COND(cNE, s4, 0);
        MOVW_COND(cEQ, s4, 1);
        BFI(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s1, 15);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SAR16 instruction, from s1 , shift s2, store result in s1 using s3 and s4 as scratch, s2 can be same as s3
void emit_sar16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_sar16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        SUB_IMM8(s4, s2, 1);
        MOV_REG_ASR_REG(s4, s1, s4);
        BFI(xFlags, s4, 0, 1);
    }
    MOV_REG_ASR_REG(s1, s1, s2);
    UBFX(s1, s1, 0, 16);
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        TSTS_REG_LSL_IMM5(s1, s1, 0);
        MOVW_COND(cNE, s4, 0);
        MOVW_COND(cEQ, s4, 1);
        BFI(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s1, 15);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        CMPS_IMM8(s2, 1);
            BFC_COND(cEQ, xFlags, F_OF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SAR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_sar16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    if(!c)
        return;
    IFX(X_PEND) {
        MOV32(s3, c);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_sar16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    IFX(X_CF) {
        MOV_REG_ASR_IMM5(s3, s1, c-1);
        BFI(xFlags, s3, 0, 1);
    }
    MOV_REG_ASR_IMM5(s1, s1, c);
    IFX(X_PEND | X_ZF) {
        UXTH(s1, s1, 0);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        TSTS_REG_LSL_IMM5(s1, s1, 0);
        MOVW_COND(cNE, s4, 0);
        MOVW_COND(cEQ, s4, 1);
        BFI(xFlags, s4, F_ZF, 1);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s1, 15);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF)
        if(c==1) {
            BFC(xFlags, F_OF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit ROL32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rol32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    if(!c)
        return;
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
            BFI(xFlags, s3, F_OF, 1);
        }
    }
}

// emit ROR32 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_ror32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    if(!c)
        return;
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
    IFX(X_OF) {
        if(c==1) {
            MOV_REG_LSR_IMM5(s4, s1, 31);
            BFI(xFlags, s4, F_OF, 1);   // store sign for later use
        }
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
            MOV_REG_LSR_IMM5(s4, s1, 31);
            XOR_REG_LSR_IMM8(s4, s4, xFlags, F_OF); // set if sign changed
            BFI(xFlags, s4, F_OF, 1);
        } else {
            BFC(xFlags, F_OF, 1);
        }
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
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
    IFX(X_OF) {
        if(c==1) {
            MOV_REG_LSR_IMM5(s4, s1, 31);
            BFI(xFlags, s4, F_OF, 1);   // store sign for later use
        }
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
            MOV_REG_LSR_IMM5(s4, s1, 31);
            XOR_REG_LSR_IMM8(s4, s4, xFlags, F_OF); // set if sign changed
            BFI(xFlags, s4, F_OF, 1);
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
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHRD32 instruction, from s1, fill s2 , shift s3, store result in s1 using s4 and s3 as scratch
void emit_shrd32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    int j32;
    MAYUSE(j32);
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        // same flags calc as shr32
        SET_DF(s4, d_shrd32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    CMPS_IMM8(s3, 0);
        IFX(X_PEND) {
            STR_IMM9_COND(cEQ, s1, xEmu, offsetof(x86emu_t, res));
        }
        B_NEXT(cEQ);

    IFX(X_OF) {
        MOV_REG_LSR_IMM5(s4, s1, 31);
        BFI(xFlags, s4, F_OF, 1);   // store sign for later use
    }
    IFX(X_CF) {
        MOVS_REG_LSR_REG(s1, s1, s3);
    } else {
        MOV_REG_LSR_REG(s1, s1, s3);
    }
    IFX(X_ZF|X_CF) {
        BIC_IMM8(xFlags, xFlags, (1<<F_ZF)|(1<<F_CF), 0);
    }
    IFX(X_CF) {
        ORR_IMM8_COND(cCS, xFlags, xFlags, 1<<F_CF, 0);
    }
    RSB_IMM8(s3, s3, 32);
    IFX(X_ZF) {
        ORRS_REG_LSL_REG(s1, s1, s2, s3);
    } else {
        ORR_REG_LSL_REG(s1, s1, s2, s3);
    }
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_ZF) {
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 1<<F_ZF, 0);
    }
    IFX(X_SF) {
        MOV_REG_LSR_IMM5(s4, s1, 31);
        BFI(xFlags, s4, F_SF, 1);
    }
    IFX(X_OF) {
        CMPS_IMM8(s3, 31);  // 32-c
            MOV_REG_LSR_IMM5_COND(cEQ, s4, s1, 31);
            XOR_REG_LSR_IMM8_COND(cEQ, s4, s4, xFlags, F_OF);
            BFI_COND(cEQ, xFlags, s4, F_OF, 1);
    }
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit SHLD32 instruction, from s1, fill s2 , shift s3, store result in s1 using s4 and s3 as scratch
void emit_shld32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4)
{
    int j32;
    MAYUSE(j32);
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, op1));
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        // same flags calc as shl32
        SET_DF(s4, d_shld32);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    CMPS_IMM8(s3, 0);
        IFX(F_OF) {
            BFC_COND(cEQ, xFlags, F_OF, 1);
        }
        IFX(X_PEND) {
            STR_IMM9_COND(cEQ, s1, xEmu, offsetof(x86emu_t, res));
        }
        B_NEXT(cEQ);

    IFX(X_OF) {
        MOV_REG_LSR_IMM5(s4, s1, 31);
        BFI(xFlags, s4, F_OF, 1);   // store sign for later use
    }
    IFX(X_CF) {
        RSB_IMM8(s4, s3, 32);
        MOV_REG_LSR_REG(s4, s1, s4);
        BFI(xFlags, s4, F_CF, 1);
    }
    IFX(X_OF) {
        MOVS_REG_LSL_REG(s1, s1, s3);
    } else {
        MOV_REG_LSL_REG(s1, s1, s3);
    }
    IFX(X_OF) {
        CMPS_IMM8(s3, 31);  // 32-c
            MOV_REG_LSR_IMM5_COND(cEQ, s4, s1, 31);
            XOR_REG_LSR_IMM8_COND(cEQ, s4, s4, xFlags, F_OF);
            BFI_COND(cEQ, xFlags, s4, F_OF, 1);
    }
    RSB_IMM8(s3, s3, 32);
    IFX(X_ZF) {
        ORRS_REG_LSR_REG(s1, s1, s2, s3);
    } else {
        ORR_REG_LSR_REG(s1, s1, s2, s3);
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
    IFX(X_PF) {
        emit_pf(dyn, ninst, s1, s4);
    }
}

// emit ROL8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rol8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOVW(s3, c);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_rol8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        }
        return;
    }
    if(c&7) {
        BFI(s1, s1, 8, 8);
        MOV_REG_LSR_IMM5(s1, s1, 8-(c&7));
    }
    //AND_IMM8(s1, s1, 0xff);
    IFX(X_PEND) {
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        BFI(xFlags, s1, F_CF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            XOR_REG_LSR_IMM8(s3, s1, s1, 7);
            BFI(xFlags, s3, F_OF, 1);
        }
    }
}

// emit ROR8 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_ror8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOVW(s3, c);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_ror8);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        }
        return;
    }
    if(c&7) {
        BFI(s1, s1, 8, 8);  // duplicate the value for Rotate right
        MOV_REG_LSR_IMM5(s1, s1, c&7);
    }
    IFX(X_PEND) {
        AND_IMM8(s1, s1, 0xff);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        MOV_REG_LSR_IMM5(s3, s1, 7);
        BFI(xFlags, s3, F_CF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            MOV_REG_LSR_IMM5(s4, s1, 6);
            XOR_REG_LSR_IMM8(s4, s4, s4, 1);
            BFI(xFlags, s4, F_OF, 1);
        }
    }
}

// emit ROL16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_rol16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOVW(s3, c);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_rol16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        }
        return;
    }
    if(c&15) {
        BFI(s1, s1, 16, 16);
        MOV_REG_LSR_IMM5(s1, s1, 16-(c&15));
    }
    IFX(X_PEND) {
        UBFX(s1, s1, 0, 16);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        BFI(xFlags, s1, F_CF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            XOR_REG_LSR_IMM8(s3, s1, s1, 15);
            BFI(xFlags, s3, F_OF, 1);
        }
    }
}

// emit ROR16 instruction, from s1 , constant c, store result in s1 using s3 and s4 as scratch
void emit_ror16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4)
{
    IFX(X_PEND) {
        MOVW(s3, c);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, op2));
        SET_DF(s4, d_ror16);
    } else IFX(X_ALL) {
        SET_DFNONE(s4);
    }
    if(!c) {
        IFX(X_PEND) {
            STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
        }
        return;
    }
    if(c&15) {
        BFI(s1, s1, 16, 16);  // duplicate the value for Rotate right
        MOV_REG_LSR_IMM5(s1, s1, c&15);
    }
    IFX(X_PEND) {
        UBFX(s1, s1, 0, 16);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, res));
    }
    IFX(X_CF) {
        MOV_REG_LSR_IMM5(s3, s1, 15);
        BFI(xFlags, s3, F_CF, 1);
    }
    IFX(X_OF) {
        if(c==1) {
            MOV_REG_LSR_IMM5(s4, s1, 14);
            XOR_REG_LSR_IMM8(s4, s4, s4, 1);
            BFI(xFlags, s4, F_OF, 1);
        }
    }
}
