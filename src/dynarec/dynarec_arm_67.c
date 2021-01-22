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
    uint8_t nextop, opcode;
    int8_t  i8;
    int32_t i32, j32;
    uint32_t u32;
    uint8_t gd, ed;
    int fixedaddress;

    MAYUSE(i32);
    MAYUSE(j32);
    
    nextop = F8;

    switch(nextop) {
       

        case 0x64:
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
                        ed = xEAX+(nextop&7);
                        if(ed!=gd) {
                            BFI(gd, ed, 0, 16);
                        }
                    } else {                    // mem <= reg
                        addr = geted16(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0);
                        LDR_REG_LSL_IMM5(gd, x14, ed, 0);
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
                        STR_REG_LSL_IMM5(x2, x14, ed, 0);
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
            
        #define GO(NO, YES)   \
            BARRIER(2); \
            JUMP(addr+i8);\
            if(dyn->insts) {    \
                if(dyn->insts[ninst].x86.jmp_insts==-1) {   \
                    /* out of the block */                  \
                    i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8); \
                    Bcond(NO, i32);     \
                    jump_to_next(dyn, addr+i8, 0, ninst); \
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
            TSTS_IMM8(xFlags, 1<<F_ZF);
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
            TSTS_IMM8(xFlags, 1<<F_ZF);
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
            DEFAULT;
    }
    return addr;
}

