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

uintptr_t dynarecFS(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop;
    int32_t i32, j32;
    uint32_t u32;
    uint8_t gd, ed;
    uint8_t wback;
    int fixedaddress;

    MAYUSE(j32);

    switch(opcode) {

        case 0x03:
            INST_NAME("ADD Gd, FS:Ed");
            SETFLAGS(X_ALL, SF_SET);
            grab_fsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO(x14);
            emit_add32(dyn, ninst, gd, ed, x3, x14);
            break;

        case 0x2B:
            INST_NAME("SUB Gd, FS:Ed");
            SETFLAGS(X_ALL, SF_SET);
            grab_fsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO(x14);
            emit_sub32(dyn, ninst, gd, ed, x3, x14);
            break;

        case 0x33:
            INST_NAME("XOR Gd, FS:Ed");
            SETFLAGS(X_ALL, SF_SET);
            grab_fsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO(x14);
            emit_xor32(dyn, ninst, gd, ed, x3, x14);
            break;

        case 0x3B:
            INST_NAME("CMP Gd, FS:Ed");
            SETFLAGS(X_ALL, SF_SET);
            grab_fsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO(x14);
            emit_cmp32(dyn, ninst, gd, ed, x3, x14);
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
                    if((nextop&0xC0)==0xC0) {
                        ed = xEAX+(nextop&7);
                        if(ed!=gd) {
                            BFI(ed, gd, 0, 16);
                        }
                    } else {
                        addr = geted16(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0);
                        STRH_REG(gd, x14, ed);
                    }
                    break;

                case 0x8B:
                    INST_NAME("MOV Gd, FS:Ew16");
                    nextop=F8;
                    grab_fsdata(dyn, addr, ninst, x14);
                    GETGD;
                    if((nextop&0xC0)==0xC0) {   // reg <= reg
                        MOV_REG(gd, xEAX+(nextop&7));
                    } else {                    // mem <= reg
                        addr = geted16(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0);
                        LDR_REG_LSL_IMM5(gd, ed, x14, 0);
                    }
                    break;

                case 0x8F:
                    INST_NAME("POP FS:Ew16");
                    nextop=F8;
                    grab_fsdata(dyn, addr, ninst, x14);
                    if((nextop&0xC0)==0xC0) {
                        POP1(xEAX+(nextop&7));  // 67 ignored
                    } else {
                        addr = geted16(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0);
                        POP1(x2);
                        STR_REG_LSL_IMM5(x2, x1, x14, 0);
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
                    break;

                case 0xFF:
                    nextop = F8;
                    grab_fsdata(dyn, addr, ninst, x14);
                    switch((nextop>>3)&7) {
                        case 6: // Push Ed
                            INST_NAME("PUSH FS:Ew");
                            if((nextop&0xC0)==0xC0) {   // reg
                                DEFAULT;
                            } else {                    // mem <= i32
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

        case 0x89:
            INST_NAME("MOV FS:Ed, Gd");
            grab_fsdata(dyn, addr, ninst, x14);
            nextop=F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {   // reg <= reg
                MOV_REG(xEAX+(nextop&7), gd);
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0);
                STR_REG_LSL_IMM5(gd, ed, x14, 0);
            }
            break;

        case 0x8B:
            INST_NAME("MOV Gd, FS:Ed");
            grab_fsdata(dyn, addr, ninst, x14);
            nextop=F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {   // reg <= reg
                MOV_REG(gd, xEAX+(nextop&7));
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0);
                LDR_REG_LSL_IMM5(gd, ed, x14, 0);
            }
            break;

        case 0x8F:
            INST_NAME("POP FS:Ed");
            grab_fsdata(dyn, addr, ninst, x14);
            nextop = F8;
            if((nextop&0xC0)==0xC0) {
                POP1(xEAX+(nextop&7));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0);
                POP1(x2);
                STR_REG_LSL_IMM5(x2, ed, x14, 0);
            }
            break;

        case 0xA0:
            INST_NAME("MOV AL, FS:Id");
            grab_fsdata(dyn, addr, ninst, x1);
            i32 = F32S;
            if(i32>-4096 && i32<4096) {
                LDRB_IMM9(xEAX, x1, i32);
            } else {
                MOV32(x2, i32);
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
            }
            break;
        case 0xA3:
            INST_NAME("MOV FS:Od, EAX");
            grab_fsdata(dyn, addr, ninst, x1);
            i32 = F32S;
            if(i32>-4096 && i32<4096) {
                STR_IMM9(xEAX, x1, i32);
            } else {
                MOV32(x2, i32);
                STR_REG_LSL_IMM5(xEAX, x2, x1, 0);
            }
            break;

        case 0xC7:
            INST_NAME("MOV FS:Ed, Id");
            grab_fsdata(dyn, addr, ninst, x14);
            nextop=F8;
            if((nextop&0xC0)==0xC0) {   // reg <= i32
                i32 = F32S;
                ed = xEAX+(nextop&7);
                MOV32(ed, i32);
            } else {                    // mem <= i32
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0);
                i32 = F32S;
                MOV32(x3, i32);
                STR_REG_LSL_IMM5(x3, ed, x14, 0);
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
                    PASS2IF(ninst && dyn->insts && dyn->insts[ninst-1].x86.set_flags, 1) {
                        READFLAGS(X_PEND);          // that's suspicious
                    } else {
                        SETFLAGS(X_ALL, SF_SET);    //Hack to put flag in "don't care" state
                    }
                    if((nextop&0xC0)==0xC0) {   // reg
                        MOV_REG(xEIP, xEAX+(nextop&7));
                    } else {                    // mem
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0);
                        LDR_REG_LSL_IMM5(xEIP, ed, x14, 0);
                    }
                    BARRIER(1);
                    BARRIER_NEXT(1);
                    if(!dyn->insts || ninst!=dyn->size-1) {
                    } else {
                        *need_epilog = 0;
                        *ok = 0;
                    }
                    MOV32(x2, addr);
                    PUSH1(x2);
                    jump_to_next(dyn, 0, xEIP, ninst);
                    break;
                case 6: // Push Ed
                    INST_NAME("PUSH FS:Ed");
                    if((nextop&0xC0)==0xC0) {   // reg
                        DEFAULT;
                    } else {                    // mem <= i32
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0);
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

