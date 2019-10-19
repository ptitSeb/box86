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

uintptr_t dynarecGS(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop;
    int32_t i32;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    int fixedaddress;
    switch(opcode) {
        case 0x33:
            grab_tlsdata(dyn, addr, ninst, x12);
            INST_NAME("GS:XOR Gd, Ed");
            nextop = F8;
            GETGD;
            GETEDO(x12);
            XOR_REG_LSL_IMM8(gd, gd, ed, 0);
            UFLAG_RES(gd);
            UFLAG_DF(1, d_xor32);
            UFLAGS(0);
            break;

        case 0x8B:
            grab_tlsdata(dyn, addr, ninst, x12);
            INST_NAME("MOV Gd, GS:Ed");
            nextop=F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {   // reg <= reg
                MOV_REG(gd, xEAX+(nextop&7));
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress);
                LDR_REG_LSL_IMM5(gd, ed, x12, 0);
            }
            break;

        case 0xA1:
            grab_tlsdata(dyn, addr, ninst, x1);
            INST_NAME("MOV EAX, GS:Id");
            i32 = F32S;
            if(i32>0 && i32<256) {
                LDR_IMM9(xEAX, x1, i32);
            } else {
                MOV32(x2, i32);
                ADD_REG_LSL_IMM8(x1, x1, x2, 0);
                LDR_IMM9(xEAX, x1, 0);
            }
            break;
        default:
            *ok = 0;
            DEFAULT;
    }
    return addr;
}

