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
#include "custommem.h"

#include "dynarec_arm_functions.h"
#include "dynarec_arm_helper.h"

uintptr_t dynarecFS(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop;
    int32_t i32, j32;
    uint32_t u32;
    uint8_t gd, ed;
    uint8_t wback, u8, wb1, wb2;
    uint8_t gb1, gb2, eb1, eb2;
    int fixedaddress;

    MAYUSE(j32);

    switch(opcode) {

        case 0x03:
            INST_NAME("ADD Gd, FS:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_fsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO(x14);
            emit_add32(dyn, ninst, gd, ed, x3, x14);
            break;

        case 0x20:
            INST_NAME("AND Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_fsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGB(x2);
            GETEBO(x14);
            emit_and8(dyn, ninst, x1, x2, x14, x2);
            EBBACK;
            break;

        case 0x2B:
            INST_NAME("SUB Gd, FS:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_fsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO(x14);
            emit_sub32(dyn, ninst, gd, ed, x3, x14);
            break;

        case 0x33:
            INST_NAME("XOR Gd, FS:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_fsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO(x14);
            emit_xor32(dyn, ninst, gd, ed, x3, x14);
            break;

        case 0x39:
            INST_NAME("CMP FS:Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_fsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO(x14);
            emit_cmp32(dyn, ninst, ed, gd, x3, x14);
            break;

        case 0x3B:
            INST_NAME("CMP Gd, FS:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_fsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO(x14);
            emit_cmp32(dyn, ninst, gd, ed, x3, x14);
            break;

        case 0x64:
            addr = dynarecFS(dyn, addr, ip, ninst, ok, need_epilog);
            break;
        case 0x65:
            addr = dynarecGS(dyn, addr, ip, ninst, ok, need_epilog);
            break;

        case 0x67:
            // reduced EA size...
            opcode = F8;
            switch(opcode) {
                case 0x89:
                    INST_NAME("MOV FS:Ew16, Gw");
                    nextop = F8;
                    grab_fsdata(dyn, addr, ninst, x14);
                    GETGD;  // don't need GETGW here
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                        if(ed!=gd) {
                            BFI(ed, gd, 0, 16);
                        }
                    } else {
                        addr = geted16(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0);
                        STRH_REG(gd, x14, ed);
                        SMWRITE2();
                    }
                    break;

                case 0x8B:
                    INST_NAME("MOV Gd, FS:Ew16");
                    nextop=F8;
                    grab_fsdata(dyn, addr, ninst, x14);
                    GETGD;
                    if(MODREG) {   // reg <= reg
                        MOV_REG(gd, xEAX+(nextop&7));
                    } else {                    // mem <= reg
                        SMREAD();
                        addr = geted16(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0);
                        LDR_REG_LSL_IMM5(gd, ed, x14, 0);
                    }
                    break;

                case 0x8F:
                    INST_NAME("POP FS:Ew16");
                    nextop=F8;
                    grab_fsdata(dyn, addr, ninst, x14);
                    if(MODREG) {
                        POP1(xEAX+(nextop&7));  // 67 ignored
                    } else {
                        addr = geted16(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0);
                        POP1(x2);
                        STR_REG_LSL_IMM5(x2, x1, x14, 0);
                        SMWRITE2();
                    }
                    break;

                case 0xA1:
                    INST_NAME("MOV EAX, FS:Ow");
                    grab_fsdata(dyn, addr, ninst, x1);
                    u32 = F16;
                    if(u32) {
                        MOV32(x2, u32);
                        ADD_REG_LSL_IMM5(x1, x1, x2, 0);
                    }
                    SMREAD();
                    LDR_IMM9(xEAX, x1, 0);
                    break;

                case 0xA3:
                    INST_NAME("MOV FS:Ow, EAX");
                    grab_fsdata(dyn, addr, ninst, x1);
                    u32 = F16;
                    if(u32) {
                        MOV32(x2, u32);
                        ADD_REG_LSL_IMM5(x1, x1, x2, 0);
                    }
                    STR_IMM9(xEAX, x1, 0);
                    SMWRITE2();
                    break;

                case 0xFF:
                    nextop = F8;
                    grab_fsdata(dyn, addr, ninst, x14);
                    switch((nextop>>3)&7) {
                        case 6: // Push Ed
                            INST_NAME("PUSH FS:Ew");
                            if(MODREG) {   // reg
                                DEFAULT;
                            } else {                    // mem <= i32
                                SMREAD();
                                addr = geted16(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0);
                                LDR_REG_LSL_IMM5(x3, ed, x14, 0);
                                PUSH1(x3);
                            }
                            break;
                        default:
                            DEFAULT;
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x88:
            INST_NAME("MOV Eb, Gb");
            grab_fsdata(dyn, addr, ninst, x14);
            nextop = F8;
            gd = (nextop&0x38)>>3;
            gb2 = ((gd&4)>>2);
            gb1 = xEAX+(gd&3);
            if(gb2) {
                gd = x3;
                UXTB(gd, gb1, gb2);
            } else {
                gd = gb1;   // no need to extract
            }
            if(MODREG) {
                ed = (nextop&7);
                eb1 = xEAX+(ed&3);  // Ax, Cx, Dx or Bx
                eb2 = ((ed&4)<<1);    // L or H
                BFI(eb1, gd, eb2, 8);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0, 0, NULL);
                STRB_REG_LSL_IMM5(gd, ed, x14, 0);
                SMWRITE2();
            }
            break;
        case 0x89:
            INST_NAME("MOV FS:Ed, Gd");
            grab_fsdata(dyn, addr, ninst, x14);
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MOV_REG(xEAX+(nextop&7), gd);
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0, 0, NULL);
                STR_REG_LSL_IMM5(gd, ed, x14, 0);
                SMWRITE2();
            }
            break;

        case 0x8B:
            INST_NAME("MOV Gd, FS:Ed");
            grab_fsdata(dyn, addr, ninst, x14);
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MOV_REG(gd, xEAX+(nextop&7));
            } else {                    // mem <= reg
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0, 0, NULL);
                LDR_REG_LSL_IMM5(gd, ed, x14, 0);
            }
            break;

        case 0x8F:
            INST_NAME("POP FS:Ed");
            grab_fsdata(dyn, addr, ninst, x14);
            nextop = F8;
            if(MODREG) {
                POP1(xEAX+(nextop&7));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                POP1(x2);
                STR_REG_LSL_IMM5(x2, ed, x14, 0);
                SMWRITE2();
            }
            break;
        case 0x90:
            INST_NAME("NOP");
            break;

        case 0xA0:
            INST_NAME("MOV AL, FS:Id");
            grab_fsdata(dyn, addr, ninst, x1);
            i32 = F32S;
            if(i32>-4096 && i32<4096) {
                LDRB_IMM9(xEAX, x1, i32);
            } else {
                MOV32(x2, i32);
                SMREAD();
                LDRB_REG_LSL_IMM5(xEAX, x1, x2, 0);
            }
            break;
        case 0xA1:
            INST_NAME("MOV EAX, FS:Id");
            grab_fsdata(dyn, addr, ninst, x1);
            i32 = F32S;
            if(i32>-4096 && i32<4096) {
                LDR_IMM9(xEAX, x1, i32);
            } else {
                MOV32(x2, i32);
                SMREAD();
                LDR_REG_LSL_IMM5(xEAX, x1, x2, 0);
            }
            break;
        case 0xA2:
            INST_NAME("MOV FS:Od, AL");
            grab_fsdata(dyn, addr, ninst, x1);
            i32 = F32S;
            if(i32>-4096 && i32<4096) {
                STRB_IMM9(xEAX, x1, i32);
            } else {
                MOV32(x2, i32);
                STRB_REG_LSL_IMM5(xEAX, x1, x2, 0);
                SMWRITE2();
            }
            break;
        case 0xA3:
            INST_NAME("MOV FS:Od, EAX");
            grab_fsdata(dyn, addr, ninst, x1);
            i32 = F32S;
            if(i32>-4096 && i32<4096) {
                STR_IMM9(xEAX, x1, i32);
                SMWRITE2();
            } else {
                MOV32(x2, i32);
                STR_REG_LSL_IMM5(xEAX, x2, x1, 0);
                SMWRITE2();
            }
            break;

        case 0xA8:
            INST_NAME("TEST AL, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            UXTB(x1, xEAX, 0);
            u8 = F8;
            MOVW(x2, u8);
            emit_test8(dyn, ninst, x1, x2, x3, x14);
            break;

        case 0xC7:
            INST_NAME("MOV FS:Ed, Id");
            grab_fsdata(dyn, addr, ninst, x14);
            nextop=F8;
            if(MODREG) {   // reg <= i32
                i32 = F32S;
                ed = xEAX+(nextop&7);
                MOV32(ed, i32);
            } else {                    // mem <= i32
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0, 0, NULL);
                i32 = F32S;
                MOV32(x3, i32);
                STR_REG_LSL_IMM5(x3, ed, x14, 0);
                SMWRITE2();
            }
            break;
            
        case 0xE9:
        case 0xEB:
            INST_NAME("(ignored) FS:");
            --addr;
            break;

        case 0xFF:
            nextop = F8;
            grab_fsdata(dyn, addr, ninst, x14);
            switch((nextop>>3)&7) {
                case 2: // CALL Ed
                    INST_NAME("CALL FS:Ed");
                    PASS2IF((box86_dynarec_safeflags>1) ||
                        ((ninst && dyn->insts[ninst-1].x86.set_flags)
                        || ((ninst>1) && dyn->insts[ninst-2].x86.set_flags)), 1)
                    {
                        READFLAGS(X_PEND);          // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET_NODF);    //Hack to put flag in "don't care" state
                    }
                    if(MODREG) {   // reg
                        MOV_REG(xEIP, xEAX+(nextop&7));
                    } else {                    // mem
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0, 0, NULL);
                        LDR_REG_LSL_IMM5(xEIP, ed, x14, 0);
                    }
                    if(box86_dynarec_callret && box86_dynarec_bigblock>1) {
                        BARRIER(BARRIER_FULL);
                        BARRIER_NEXT(BARRIER_FULL);
                    } else {
                        BARRIER(BARRIER_FLOAT);
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    MOV32(x2, addr);
                    if(box86_dynarec_callret) {
                        SET_HASCALLRET();
                        // Push actual return address
                        if(addr < (dyn->start+dyn->isize)) {
                            // there is a next...
                            j32 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->arm_size)-8):0;
                            MESSAGE(LOG_NONE, "\tCALLRET set return to +%di\n", j32>>2);
                            ADR(c__, x3, j32);
                        } else {
                            MESSAGE(LOG_NONE, "\tCALLRET set return to Jmptable(%p)\n", (void*)addr);
                            j32 = getJumpTableAddress(addr);
                            MOV32(x3, j32);
                            LDR_IMM9(x3, x3, 0);
                        }
                        PUSH(xSP, (1<<x2)|(1<<x3));
                    }
                    PUSH1(x2);
                    jump_to_next(dyn, 0, xEIP, ninst);
                    break;
                case 6: // Push Ed
                    INST_NAME("PUSH FS:Ed");
                    if(MODREG) {   // reg
                        DEFAULT;
                    } else {                    // mem <= i32
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0, 0, NULL);
                        LDR_REG_LSL_IMM5(x3, ed, x14, 0);
                        PUSH1(x3);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;

        default:
            DEFAULT;
    }
    return addr;
}

