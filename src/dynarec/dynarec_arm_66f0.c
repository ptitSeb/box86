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
    int16_t i16;
    MAYUSE(i32);
    MAYUSE(j32);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(wb1);
    MAYUSE(wb2);
    switch(opcode) {
        case 0x81:
        case 0x83:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: //ADD
                    if(opcode==0x81) {
                        INST_NAME("LOCK ADD Ew, Iw");
                    } else {
                        INST_NAME("LOCK ADD Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(MODREG) {
                        if(opcode==0x81) i16 = F16S; else i16 = F8S;
                        ed = xEAX+(nextop&7);
                        emit_add16c(dyn, ninst, ed, i16, x3, x14);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 1, LOCK_LOCK);
                        if(opcode==0x81) i16 = F16S; else i16 = F8S;
                        if(!fixedaddress) {
                            TSTS_IMM8(wback, 0x1);
                            B_MARK(cNE);
                        }
                        if(!fixedaddress || (fixedaddress && !(fixedaddress&1))) {
                            MARKLOCK;
                            LDREXH(x1, wback);
                            emit_add16c(dyn, ninst, x1, i16, x3, x14);
                            STREXH(x3, x1, wback);
                            CMPS_IMM8(x3, 0);
                            B_MARKLOCK(cNE);
                            SMDMB();
                        }
                        if(!fixedaddress) {
                            B_NEXT(c__);
                        }
                        if(!fixedaddress || (fixedaddress && (fixedaddress&1))) {
                            MARK;   // unaligned! also, not enough
                            LDRH_IMM8(x1, wback, 0);
                            LDREXB(x14, wback);
                            BFI(x1, x14, 0, 8); // re-inject
                            emit_add16c(dyn, ninst, x1, i16, x3, x14);
                            STREXB(x3, x1, wback);
                            CMPS_IMM8(x3, 0);
                            B_MARK(cNE);
                            STRH_IMM8(x1, wback, 0); // put the whole value
                            SMDMB();
                        }
                    }
                    break;
                default:
                  DEFAULT;
            }
            SMDMB();
            break;
        case 0xFF:
            nextop = F8;
            SMDMB();
            switch((nextop>>3)&7)
            {
                case 0: // INC Ew
                    INST_NAME("INC Ew");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    if(MODREG) {
                        GETEW(x1);
                        emit_inc16(dyn, ninst, ed, x3, x14);
                        EWBACK(x1);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 1, LOCK_LOCK);
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
                            SMDMB();
                        }
                        if(!fixedaddress) {
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
                            SMDMB();
                        }
                    }
                    break;
                case 1: //DEC Ew
                    INST_NAME("DEC Ew");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    if(MODREG) {
                        GETEW(x1);
                        emit_dec32(dyn, ninst, ed, x3, x14);
                        EWBACK(x1);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 1, LOCK_LOCK);
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
                            SMDMB();
                        }
                        if(!fixedaddress) {
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
                            SMDMB();
                        }
                    }
                    break;
                default:
                    // default to NO LOCK
                    addr-=2;
            }
            SMDMB();
            break;
        default:
            DEFAULT;
    }

    return addr;
}
