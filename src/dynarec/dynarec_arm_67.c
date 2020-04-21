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

#include "dynarec_arm_helper.h"


uintptr_t dynarec67(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop;
    int8_t  i8;
    int32_t i32;
    MAYUSE(i32);
    nextop = F8;
    switch(nextop) {
       

        #define GO(NO, YES)   \
            BARRIER(2); \
            JUMP(addr+i8);\
            if(dyn->insts) {    \
                if(dyn->insts[ninst].x86.jmp_insts==-1) {   \
                    /* out of the block */                  \
                    i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8); \
                    Bcond(NO, i32);     \
                    jump_to_linker(dyn, addr+i8, 0, ninst); \
                } else {    \
                    /* inside the block */  \
                    i32 = dyn->insts[dyn->insts[ninst].x86.jmp_insts].address-(dyn->arm_size+8);    \
                    Bcond(YES, i32);    \
                }   \
            }
        case 0xE0:
            INST_NAME("LOOPNZ (16bits)");
            READFLAGS(X_ZF);
            i8 = F8S;
            UXTH(x1, xECX, 0);
            SUBS_IMM8(x1, x1, 1);
            BFI(xECX, x1, 0, 16);
            B_NEXT(cEQ);    // CX is 0, no LOOP
            LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_ZF]));
            TSTS_REG_LSL_IMM5(x1, x1, 0);
            GO(cNE, cEQ);
            break;
        case 0xE1:
            INST_NAME("LOOPZ (16bits)");
            READFLAGS(X_ZF);
            i8 = F8S;
            UXTH(x1, xECX, 0);
            SUBS_IMM8(x1, x1, 1);
            BFI(xECX, x1, 0, 16);
            B_NEXT(cEQ);    // CX is 0, no LOOP
            LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_ZF]));
            TSTS_REG_LSL_IMM5(x1, x1, 0);
            GO(cEQ, cNE);
            break;
        case 0xE2:
            INST_NAME("LOOP (16bits)");
            i8 = F8S;
            UXTH(x1, xECX, 0);
            SUBS_IMM8(x1, x1, 1);
            BFI(xECX, x1, 0, 16);
            GO(cEQ, cNE);
            break;
        case 0xE3:
            INST_NAME("JCXZ");
            i8 = F8S;
            UXTH(x1, xECX, 0);
            TSTS_REG_LSL_IMM5(x1, x1, 0);
            GO(cNE, cEQ);
            break;
        #undef GO

        default:
            *ok = 0;
            DEFAULT;
    }
    return addr;
}

