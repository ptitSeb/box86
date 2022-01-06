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

#include "dynarec_arm_functions.h"
#include "dynarec_arm_helper.h"


uintptr_t dynarec66F0(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop, opcode = F8;
    uint8_t wback, wb1, wb2, gb1, gb2;
    uint8_t ed, gd, u8;
    int fixedaddress;
    int32_t i32, j32;
    MAYUSE(i32);
    MAYUSE(j32);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(wb1);
    MAYUSE(wb2);
    switch(opcode) {
        
        case 0xFF:
            nextop = F8;
            DMB_ISH();
            switch((nextop>>3)&7)
            {
                case 0: // INC Ew
                    INST_NAME("INC Ew");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    if((nextop&0xC0)==0xC0) {
                        GETEW(x1);
                        emit_inc16(dyn, ninst, ed, x3, x14);
                        EWBACK(x1);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 1);
                        if(!fixedaddress) {
                            TSTS_IMM8(wback, 1);
                            B_MARK(cNE);    // unaligned
                        }
                        if(!fixedaddress || (fixedaddress && !(fixedaddress&1))) {
                            MARKLOCK;
                            LDREXH(x1, wback);
                            emit_inc16(dyn, ninst, x1, x3, x14);
                            STREXH(x3, x1, wback);
                            CMPS_IMM8(x3, 0);
                            B_MARKLOCK(cNE);
                        }
                        if(!fixedaddress) {
                            DMB_ISH();
                            B_NEXT(c__);
                        }
                        if(!fixedaddress || (fixedaddress && (fixedaddress&1))) {
                            MARK;
                            LDRH_IMM8(x1, wback, 0);
                            LDREXB(x3, wback);
                            BFI(x1, x3, 0, 8);
                            emit_inc16(dyn, ninst, x1, x3, x14);
                            STREXB(x3, x1, wback);
                            CMPS_IMM8(x3, 0);
                            B_MARK(cNE);
                            STRH_IMM8(x1, wback, 0);
                        }
                    }
                    break;
                case 1: //DEC Ew
                    INST_NAME("DEC Ew");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    if((nextop&0xC0)==0xC0) {
                        GETEW(x1);
                        emit_dec32(dyn, ninst, ed, x3, x14);
                        EWBACK(x1);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 1);
                        if(!fixedaddress) {
                            TSTS_IMM8(wback, 1);
                            B_MARK(cNE);    // unaligned
                        }
                        if(!fixedaddress || (fixedaddress && !(fixedaddress&1))) {
                            MARKLOCK;
                            LDREXH(x1, wback);
                            emit_dec16(dyn, ninst, x1, x3, x14);
                            STREXH(x3, x1, wback);
                            CMPS_IMM8(x3, 0);
                            B_MARKLOCK(cNE);
                        }
                        if(!fixedaddress) {
                            DMB_ISH();
                            B_NEXT(c__);
                        }
                        if(!fixedaddress || (fixedaddress && (fixedaddress&3))) {
                            MARK;
                            LDRH_IMM8(x1, wback, 0);
                            LDREXB(x3, wback);
                            BFI(x1, x3, 0, 8);
                            emit_dec16(dyn, ninst, x1, x3, x14);
                            STREXB(x3, x1, wback);
                            CMPS_IMM8(x3, 0);
                            B_MARK(cNE);
                            STRH_IMM8(x1, wback, 0);
                        }
                    }
                    break;
                default:
                    // default to NO LOCK
                    addr-=2;
            }
            DMB_ISH();
            break;
        default:
            DEFAULT;
    }

    return addr;
}
