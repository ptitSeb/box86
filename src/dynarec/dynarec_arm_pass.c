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
#include "dynablock.h"
#include "dynablock_private.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"
#include "arm_printer.h"
#include "dynarec_arm_helper.h"

#ifndef STEP
#error No STEP defined
#endif

#include "dynarec_arm_helper.h"

void arm_pass(dynarec_arm_t* dyn, uintptr_t addr)
{
    uint8_t nextop, opcode;
    int ok = 1;
    int ninst = 0;
    uintptr_t ip = addr;
    uintptr_t natcall;
    int retn;
    uint8_t gd, ed;
    int8_t i8;
    int32_t i32, tmp;
    uint8_t u8;
    uint8_t gb1, gb2, eb1, eb2;
    uint32_t u32;
    int need_epilog = 1;
    dyn->tablei = 0;
    uint8_t wback;
    // Clean up (because there are multiple passes)
    dyn->cleanflags = 0;
    // ok, go now
    INIT;
    while(ok) {
        ip = addr;
        opcode = F8;
        NEW_INST;
#ifdef HAVE_TRACE
        if(dyn->emu->dec && box86_dynarec_trace) {
            MESSAGE(LOG_DUMP, "TRACE ----\n");
            STM(0, (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<11));
            MOV32(1, ip);
            STR_IMM9(1, 0, offsetof(x86emu_t, ip));
            MOV32(2, 1);
            CALL(PrintTrace, -1, 0);
            MESSAGE(LOG_DUMP, "----------\n");
        }
#endif
        if(dyn->insts && dyn->insts[ninst].x86.barrier) {
            if(dyn->insts[ninst].x86.barrier!=2)
                dyn->cleanflags = 0;
        }
        switch(opcode) {

            case 0x00:
                INST_NAME("ADD Eb, Gb");
                nextop = F8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    SXTB(x1, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x12);
                    LDRSB_IMM8(x1, ed, 0);
                }
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                SXTB(x2, gb1, gb2);
                UFLAG_OP12(x1, x2);
                ADD_REG_LSL_IMM8(x1, x1, x2, 0);
                UFLAG_RES(x1);
                if((nextop&0xC0)==0xC0) {
                    BFI(eb1, x1, eb2*8, 8);
                } else {
                    STRB_IMM9(x1, ed, 0);
                }
                UFLAG_DF(x3, d_add8);
                UFLAGS(0);
                break;
            case 0x01:
                INST_NAME("ADD Ed, Gd");
                nextop = F8;
                GETGD;
                GETED;
                UFLAG_OP12(ed, gd);
                ADD_REG_LSL_IMM8(ed, ed, gd, 0);
                WBACK;
                UFLAG_RES(ed);
                UFLAG_DF(x1, d_add32);
                UFLAGS(0);
                break;
            case 0x02:
                INST_NAME("ADD Gb, Eb");
                nextop = F8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    SXTB(x2, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x12);
                    LDRSB_IMM8(x2, ed, 0);
                }
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                SXTB(x1, gb1, gb2);
                UFLAG_OP12(x1, x2);
                ADD_REG_LSL_IMM8(x1, x1, x2, 0);
                UFLAG_RES(x1);
                BFI(gb1, x1, gb2*8, 8);
                UFLAG_DF(x3, d_add8);
                UFLAGS(0);
                break;
            case 0x03:
                INST_NAME("ADD Gd, Ed");
                nextop = F8;
                GETGD;
                GETED;
                UFLAG_OP12(gd, ed);
                ADD_REG_LSL_IMM8(gd, gd, ed, 0);
                UFLAG_RES(gd);
                UFLAG_DF(x1, d_add32);
                UFLAGS(0);
                break;
            case 0x04:
                INST_NAME("ADD Gb, Ib");
                u8 = F8;
                SXTB(x1, xEAX, 0);
                MOV32(x2, u8);
                UFLAG_OP12(x1, x2);
                ADD_REG_LSL_IMM8(x1, x1, x2, 0);
                UFLAG_RES(x1);
                BFI(xEAX, x1, 0, 8);
                UFLAG_DF(x3, d_add8);
                UFLAGS(0);
                break;
            case 0x05:
                INST_NAME("ADD EAX, Id");
                i32 = F32S;
                MOV32(x1, i32);
                UFLAG_OP12(xEAX, x1);
                ADD_REG_LSL_IMM8(xEAX, xEAX, x1, 0);
                UFLAG_RES(xEAX);
                UFLAG_DF(x1, d_add32);
                UFLAGS(0);
                break;

            case 0x08:
                INST_NAME("OR Eb, Gb");
                nextop = F8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    UXTB(x1, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x12);
                    LDRB_IMM9(x1, ed, 0);
                }
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                UXTB(x2, gb1, gb2);
                ORR_REG_LSL_IMM8(x1, x1, x2, 0);
                UFLAG_RES(x1);
                if((nextop&0xC0)==0xC0) {
                    BFI(eb1, x1, eb2*8, 8);
                } else {
                    STRB_IMM9(x1, ed, 0);
                }
                UFLAG_DF(x3, d_or8);
                UFLAGS(0);
                break;
            case 0x09:
                INST_NAME("OR Ed, Gd");
                nextop = F8;
                GETGD;
                GETED;
                ORR_REG_LSL_IMM8(ed, ed, gd, 0);
                WBACK;
                UFLAG_RES(ed);
                UFLAG_DF(x1, d_or32);
                UFLAGS(0);
                break;
            case 0x0A:
                INST_NAME("OR Gb, Eb");
                nextop = F8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    UXTB(x2, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x12);
                    LDRB_IMM9(x2, ed, 0);
                }
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                UXTB(x1, gb1, gb2);
                ORR_REG_LSL_IMM8(x1, x1, x2, 0);
                UFLAG_RES(x1);
                BFI(gb1, x1, gb2*8, 8);
                UFLAG_DF(x3, d_or8);
                UFLAGS(0);
                break;
            case 0x0B:
                INST_NAME("OR Gd, Ed");
                nextop = F8;
                GETGD;
                GETED;
                ORR_REG_LSL_IMM8(gd, gd, ed, 0);
                UFLAG_RES(gd);
                UFLAG_DF(x1, d_or32);
                UFLAGS(0);
                break;
            case 0x0C:
                INST_NAME("OR Gb, Ib");
                u8 = F8;
                UXTB(x1, xEAX, 0);
                ORR_IMM8(x1, x1, u8, 0);
                UFLAG_RES(x1);
                BFI(xEAX, x1, 0, 8);
                UFLAG_DF(x3, d_or8);
                UFLAGS(0);
                break;
            case 0x0D:
                INST_NAME("OR EAX, Id");
                i32 = F32S;
                MOV32(x1, i32);
                ORR_REG_LSL_IMM8(xEAX, xEAX, x1, 0);
                UFLAG_RES(xEAX);
                UFLAG_DF(x1, d_or32);
                UFLAGS(0);
                break;

            case 0x0F:
                addr = dynarec0f(dyn, addr, ninst, &ok, &need_epilog);
                break;

            case 0x10:
                INST_NAME("ADC Eb, Gb");
                nextop = F8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    UXTB(x1, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x12);
                    LDRB_IMM9(x1, ed, 0);
                }
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                UXTB(x2, gb1, gb2);
                CALL_(adc8, x1, (1<<x12));
                if((nextop&0xC0)==0xC0) {
                    BFI(eb1, x1, eb2*8, 8);
                } else {
                    STRB_IMM9(x1, ed, 0);
                }
                UFLAGS(1);
                break;
            case 0x11:
                INST_NAME("ADC Ed, Gd");
                nextop = F8;
                GETGD;
                GETEDW(x12, x1);
                MOV_REG(x2, gd);
                CALL_(adc32, x1, (1<<x12));
                SBACK(x1);
                UFLAGS(1);
                break;
            case 0x12:
                INST_NAME("ADC Gb, Eb");
                nextop = F8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    UXTB(x2, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x12);
                    LDRB_IMM9(x2, ed, 0);
                }
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                UXTB(x1, gb1, gb2);
                CALL_(adc8, x1, 0);
                BFI(gb1, x1, gb2*8, 8);
                UFLAGS(1);
                break;
            case 0x13:
                INST_NAME("ADC Gd, Ed");
                nextop = F8;
                GETGD;
                GETEDW(x12, x2);
                MOV_REG(x1, gd);
                CALL_(adc32, x1, 0);
                MOV_REG(gd, x1);
                UFLAGS(1);
                break;
            case 0x14:
                INST_NAME("ADC Gb, Ib");
                u8 = F8;
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                UXTB(x1, gb1, gb2);
                MOVW(x2, u8);
                CALL_(adc8, x1, 0);
                BFI(gb1, x1, gb2*8, 8);
                UFLAGS(1);
                break;
            case 0x15:
                INST_NAME("ADC Gd, Ed");
                i32 = F32S;
                GETGD;
                MOV_REG(x1, gd);
                MOV32(x2, i32);
                CALL_(adc32, x1, 0);
                MOV_REG(gd, x1);
                UFLAGS(1);
                break;

            case 0x18:
                INST_NAME("SBB Eb, Gb");
                nextop = F8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    UXTB(x1, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x12);
                    LDRB_IMM9(x1, ed, 0);
                }
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                UXTB(x2, gb1, gb2);
                CALL_(sbb8, x1, (1<<x12));
                if((nextop&0xC0)==0xC0) {
                    BFI(eb1, x1, eb2*8, 8);
                } else {
                    STRB_IMM9(x1, ed, 0);
                }
                UFLAGS(1);
                break;
            case 0x19:
                INST_NAME("SBB Ed, Gd");
                nextop = F8;
                GETGD;
                GETEDW(x12, x1);
                MOV_REG(x2, gd);
                CALL_(sbb32, x1, (1<<x12));
                SBACK(x1);
                UFLAGS(1);
                break;

            case 0x20:
                INST_NAME("AND Eb, Gb");
                nextop = F8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    UXTB(x1, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x12);
                    LDRB_IMM9(x1, ed, 0);
                }
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                UXTB(x2, gb1, gb2);
                AND_REG_LSL_IMM8(x1, x1, x2, 0);
                UFLAG_RES(x1);
                if((nextop&0xC0)==0xC0) {
                    BFI(eb1, x1, eb2*8, 8);
                } else {
                    STRB_IMM9(x1, ed, 0);
                }
                UFLAG_DF(x3, d_and8);
                UFLAGS(0);
                break;
            case 0x21:
                INST_NAME("AND Ed, Gd");
                nextop = F8;
                GETGD;
                GETED;
                AND_REG_LSL_IMM8(ed, ed, gd, 0);
                WBACK;
                UFLAG_RES(ed);
                UFLAG_DF(x1, d_and32);
                UFLAGS(0);
                break;
            case 0x22:
                INST_NAME("AND Gb, Eb");
                nextop = F8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    UXTB(x2, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x12);
                    LDRB_IMM9(x2, ed, 0);
                }
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                UXTB(x1, gb1, gb2);
                AND_REG_LSL_IMM8(x1, x1, x2, 0);
                UFLAG_RES(x1);
                BFI(gb1, x1, gb2*8, 8);
                UFLAG_DF(x3, d_and8);
                UFLAGS(0);
                break;
            case 0x23:
                INST_NAME("AND Gd, Ed");
                nextop = F8;
                GETGD;
                GETED;
                AND_REG_LSL_IMM8(gd, gd, ed, 0);
                UFLAG_RES(gd);
                UFLAG_DF(x1, d_and32);
                UFLAGS(0);
                break;
            case 0x24:
                INST_NAME("AND Gb, Ib");
                u8 = F8;
                UXTB(x1, xEAX, 0);
                AND_IMM8(x1, x1, u8);
                UFLAG_RES(x1);
                BFI(xEAX, x1, 0, 8);
                UFLAG_DF(x3, d_and8);
                UFLAGS(0);
                break;
            case 0x25:
                INST_NAME("AND EAX, Id");
                i32 = F32S;
                MOV32(x1, i32);
                AND_REG_LSL_IMM8(xEAX, xEAX, x1, 0);
                UFLAG_RES(xEAX);
                UFLAG_DF(x1, d_and32);
                UFLAGS(0);
                break;

            case 0x28:
                INST_NAME("SUB Eb, Gb");
                nextop = F8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    SXTB(x1, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x12);
                    LDRSB_IMM8(x1, ed, 0);
                }
                UFLAG_OP1(x1);
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                SXTB(x2, gb1, gb2);
                UFLAG_OP2(x2);
                SUB_REG_LSL_IMM8(x1, x1, x2, 0);
                UFLAG_RES(x1);
                if((nextop&0xC0)==0xC0) {
                    BFI(eb1, x1, eb2*8, 8);
                } else {
                    STRB_IMM9(x1, ed, 0);
                }
                UFLAG_DF(x3, d_sub8);
                UFLAGS(0);
                break;
            case 0x29:
                INST_NAME("SUB Ed, Gd");
                nextop = F8;
                GETGD;
                GETED;
                UFLAG_OP12(ed, gd);
                SUB_REG_LSL_IMM8(ed, ed, gd, 0);
                WBACK;
                UFLAG_RES(ed);
                UFLAG_DF(x1, d_sub32);
                UFLAGS(0);
                break;
            case 0x2A:
                INST_NAME("SUB Gb, Eb");
                nextop = F8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    SXTB(x2, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x12);
                    LDRSB_IMM8(x2, ed, 0);
                }
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                SXTB(x1, gb1, gb2);
                UFLAG_OP12(x1, x2);
                SUB_REG_LSL_IMM8(x1, x1, x2, 0);
                UFLAG_RES(x1);
                BFI(gb1, x1, gb2*8, 8);
                UFLAG_DF(x3, d_sub8);
                UFLAGS(0);
                break;
            case 0x2B:
                INST_NAME("SUB Gd, Ed");
                nextop = F8;
                GETGD;
                GETED;
                UFLAG_OP12(gd, ed);
                SUB_REG_LSL_IMM8(gd, gd, ed, 0);
                UFLAG_RES(gd);
                UFLAG_DF(x1, d_sub32);
                UFLAGS(0);
                break;
            case 0x2C:
                INST_NAME("SUB Gb, Ib");
                i32 = F8S;
                SXTB(x1, xEAX, 0);
                MOV32(x2, i32)
                UFLAG_OP12(x1, x2);
                SUB_REG_LSL_IMM8(x1, x1, x2, 0);
                UFLAG_RES(x1);
                BFI(xEAX, x1, 0, 8);
                UFLAG_DF(x3, d_sub8);
                UFLAGS(0);
                break;
            case 0x2D:
                INST_NAME("SUB EAX, Id");
                i32 = F32S;
                MOV32(x1, i32);
                UFLAG_OP12(xEAX, x1);
                SUB_REG_LSL_IMM8(xEAX, xEAX, x1, 0);
                UFLAG_RES(xEAX);
                UFLAG_DF(x1, d_sub32);
                UFLAGS(0);
                break;

            case 0x2E:
                INST_NAME("CS:");
                // ignored
                break;

            case 0x30:
                INST_NAME("XOR Eb, Gb");
                nextop = F8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    UXTB(x1, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x12);
                    LDRB_IMM9(x1, ed, 0);
                }
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                UXTB(x2, gb1, gb2);
                XOR_REG_LSL_IMM8(x1, x1, x2, 0);
                UFLAG_RES(x1);
                if((nextop&0xC0)==0xC0) {
                    BFI(eb1, x1, eb2*8, 8);
                } else {
                    STRB_IMM9(x1, ed, 0);
                }
                UFLAG_DF(x3, d_xor8);
                UFLAGS(0);
                break;
            case 0x31:
                INST_NAME("XOR Ed, Gd");
                nextop = F8;
                GETGD;
                GETED;
                XOR_REG_LSL_IMM8(ed, ed, gd, 0);
                WBACK;
                UFLAG_RES(ed);
                UFLAG_DF(x1, d_xor32);
                UFLAGS(0);
                break;
            case 0x32:
                INST_NAME("XOR Gb, Eb");
                nextop = F8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    UXTB(x2, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x12);
                    LDRB_IMM9(x2, ed, 0);
                }
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                UXTB(x1, gb1, gb2);
                XOR_REG_LSL_IMM8(x1, x1, x2, 0);
                UFLAG_RES(x1);
                BFI(gb1, x1, gb2*8, 8);
                UFLAG_DF(x3, d_xor8);
                UFLAGS(0);
                break;
            case 0x33:
                INST_NAME("XOR Gd, Ed");
                nextop = F8;
                GETGD;
                GETED;
                XOR_REG_LSL_IMM8(gd, gd, ed, 0);
                UFLAG_RES(gd);
                UFLAG_DF(x1, d_xor32);
                UFLAGS(0);
                break;
            case 0x34:
                INST_NAME("XOR Gb, Ib");
                u8 = F8;
                UXTB(x1, xEAX, 0);
                XOR_IMM8(x1, x1, u8);
                UFLAG_RES(x1);
                BFI(xEAX, x1, 0, 8);
                UFLAG_DF(x3, d_xor8);
                UFLAGS(0);
                break;
            case 0x35:
                INST_NAME("XOR EAX, Id");
                i32 = F32S;
                MOV32(x1, i32);
                XOR_REG_LSL_IMM8(xEAX, xEAX, x1, 0);
                UFLAG_RES(xEAX);
                UFLAG_DF(x1, d_xor32);
                UFLAGS(0);
                break;

            case 0x38:
                INST_NAME("CMP Eb, Gb");
                nextop = F8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    UXTB(x1, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x12);
                    LDRB_IMM9(x1, ed, 0);
                }
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                UXTB(x2, gb1, gb2);
                CALL_(cmp8, -1, 0);
                UFLAGS(1);
                break;
            case 0x39:
                INST_NAME("CMP Ed, Gd");
                nextop = F8;
                GETGD;
                GETEDH(x1);
                if(ed!=x1) {MOV_REG(x1, ed);};
                MOV_REG(x2, gd);
                CALL(cmp32, -1, 0);
                UFLAGS(1);
                break;
            case 0x3A:
                INST_NAME("CMP Gb, Eb");
                nextop = F8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    UXTB(x2, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x12);
                    LDRB_IMM9(x2, ed, 0);
                }
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                UXTB(x1, gb1, gb2);
                CALL(cmp8, -1, 0);
                UFLAGS(1);
                break;
            case 0x3B:
                INST_NAME("CMP Gd, Ed");
                nextop = F8;
                GETGD;
                GETEDH(x2);
                if(ed!=x2) {MOV_REG(x2,ed);}
                MOV_REG(x1, gd);
                CALL(cmp32, -1, 0);
                UFLAGS(1);
                break;
            case 0x3C:
                INST_NAME("CMP AL, Ib");
                u32 = F8;
                MOV32(x2, u32);
                UXTB(x1, xEAX, 0);
                CALL(cmp8, -1, 0);
                UFLAGS(1);
                break;
            case 0x3D:
                INST_NAME("CMP EAX, Id");
                i32 = F32S;
                MOV32(x2, i32);
                MOV_REG(x1, xEAX);
                CALL(cmp32, -1, 0);
                UFLAGS(1);
                break;

            case 0x40:
            case 0x41:
            case 0x42:
            case 0x43:
            case 0x44:
            case 0x45:
            case 0x46:
            case 0x47:
                INST_NAME("INC reg");
                gd = xEAX+(opcode&0x07);
                UFLAG_OP1(gd);
                ADD_IMM8(gd, gd, 1);
                UFLAG_RES(gd);
                UFLAG_DF(x1, d_inc32);
                UFLAGS(0);
                break;
            case 0x48:
            case 0x49:
            case 0x4A:
            case 0x4B:
            case 0x4C:
            case 0x4D:
            case 0x4E:
            case 0x4F:
                INST_NAME("DEC reg");
                gd = xEAX+(opcode&0x07);
                UFLAG_OP1(gd);
                SUB_IMM8(gd, gd, 1);
                UFLAG_RES(gd);
                UFLAG_DF(x1, d_dec32);
                UFLAGS(0);
                break;
            case 0x50:
            case 0x51:
            case 0x52:
            case 0x53:
            case 0x54:
            case 0x55:
            case 0x56:
            case 0x57:
                INST_NAME("PUSH reg");
                gd = xEAX+(opcode&0x07);
                PUSH(xESP, 1<<gd);
                break;
            case 0x58:
            case 0x59:
            case 0x5A:
            case 0x5B:
            case 0x5C:
            case 0x5D:
            case 0x5E:
            case 0x5F:
                INST_NAME("POP reg");
                gd = xEAX+(opcode&0x07);
                POP(xESP, 1<<gd);
                break;

            case 0x65:
                addr = dynarecGS(dyn, addr, ninst, &ok, &need_epilog);
                break;
            case 0x66:
                addr = dynarec66(dyn, addr, ninst, &ok, &need_epilog);
                break;

            case 0x68:
                INST_NAME("PUSH Id");
                i32 = F32S;
                MOV32(x3, i32);
                PUSH(xESP, 1<<x3);
                break;
            case 0x69:
                INST_NAME("IMUL Gd, Ed, Id");
                nextop = F8;
                GETGD;
                GETED;
                i32 = F32S;
                MOV32(x12, i32);
                UFLAG_IF {
                    SMULL(x3, gd, x12, ed);   //RdHi, RdLo, Rm must be different
                    UFLAG_OP1(x3);
                    UFLAG_RES(gd);
                    UFLAG_DF(x3, d_imul32);
                } else {
                    MUL(gd, ed, x12);
                }
                UFLAGS(0);
                break;
            case 0x6A:
                INST_NAME("PUSH Ib");
                i32 = F8S;
                MOV32(x3, i32);
                PUSH(xESP, 1<<x3);
                break;

            #define GO(GETFLAGS, NO, YES)   \
                i8 = F8S;   \
                USEFLAG;    \
                JUMP(addr+i8);\
                GETFLAGS;   \
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

            case 0x70:
                INST_NAME("JO ib");
                GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_OF]));
                    CMPS_IMM8(x2, 1)
                    , cNE, cEQ)
                break;
            case 0x71:
                INST_NAME("JNO ib");
                GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_OF]));
                    CMPS_IMM8(x2, 1)
                    , cEQ, cNE)
                break;
            case 0x72:
                INST_NAME("JC ib");
                GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                    CMPS_IMM8(x2, 1)
                    , cNE, cEQ)
                break;
            case 0x73:
                INST_NAME("JNC ib");
                GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                    CMPS_IMM8(x2, 1)
                    , cEQ, cNE)
                break;
            case 0x74:
                INST_NAME("JZ ib");
                GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                    CMPS_IMM8(x2, 1)
                    , cNE, cEQ)
                break;
            case 0x75:
                INST_NAME("JNZ ib");
                GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                    CMPS_IMM8(x2, 1)
                    , cEQ, cNE)
                break;
            case 0x76:
                INST_NAME("JBE ib");
                GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                    LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                    ORRS_REG_LSL_IMM8(x2, x2, x3, 0);
                    , cEQ, cNE)
                break;
            case 0x77:
                INST_NAME("JNBE ib");
                GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                    LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                    ORRS_REG_LSL_IMM8(x2, x2, x3, 0);
                    , cNE, cEQ)
                break;
            case 0x78:
                INST_NAME("JS ib");
                GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                    CMPS_IMM8(x2, 1)
                    , cNE, cEQ)
                break;
            case 0x79:
                INST_NAME("JNS ib");
                GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                    CMPS_IMM8(x2, 1)
                    , cEQ, cNE)
                break;
            case 0x7A:
                INST_NAME("JP ib");
                GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_PF]));
                    CMPS_IMM8(x2, 1)
                    , cNE, cEQ)
                break;
            case 0x7B:
                INST_NAME("JNP ib");
                GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_PF]));
                    CMPS_IMM8(x2, 1)
                    , cEQ, cNE)
                break;
            case 0x7C:
                INST_NAME("JL ib");
                GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                    LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                    CMPS_REG_LSL_IMM8(x1, x2, 0)
                    , cEQ, cNE)
                break;
            case 0x7D:
                INST_NAME("JGE ib");
                GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                    LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                    CMPS_REG_LSL_IMM8(x1, x2, 0)
                    , cNE, cEQ)
                break;
            case 0x7E:
                INST_NAME("JLE ib");
                GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                    LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                    XOR_REG_LSL_IMM8(x1, x1, x2, 0);
                    LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                    ORRS_REG_LSL_IMM8(x2, x1, x2, 0);
                    , cEQ, cNE)
                break;
            case 0x7F:
                INST_NAME("JG ib");
                GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                    LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                    XOR_REG_LSL_IMM8(x1, x1, x2, 0);
                    LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                    ORRS_REG_LSL_IMM8(x2, x1, x2, 0);
                    , cNE, cEQ)
                break;
            #undef GO
            
            case 0x80:
                nextop = F8;
                switch((nextop>>3)&7) {
                    case 0: //ADD
                        INST_NAME("ADD Eb, Ib");
                        if((nextop&0xC0)==0xC0) {
                            ed = (nextop&7);
                            eb1 = xEAX+(ed&3);
                            eb2 = ((ed&4)>>2);
                            UXTB(x1, eb1, eb2);
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                            LDRB_IMM9(x1, ed, 0);
                        }
                        UFLAG_OP1(x1);
                        u8 = F8;
                        UFLAG_IF{MOV32(x3, u8); UFLAG_OP2(x3); UFLAG_DF(x3, d_add8);}
                        ADD_IMM8(x1, x1, u8);
                        UFLAG_RES(x1);
                        if((nextop&0xC0)==0xC0) {
                            BFI(eb1, x1, eb2*8, 8);
                        } else {
                            STRB_IMM9(x1, ed, 0);
                        }
                        UFLAGS(0);
                        break;
                    case 1: //OR
                        INST_NAME("OR Eb, Ib");
                        if((nextop&0xC0)==0xC0) {
                            ed = (nextop&7);
                            eb1 = xEAX+(ed&3);
                            eb2 = ((ed&4)>>2);
                            UXTB(x1, eb1, eb2);
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                            LDRB_IMM9(x1, ed, 0);
                        }
                        u8 = F8;
                        ORR_IMM8(x1, x1, u8, 0);
                        UFLAG_RES(1);
                        if((nextop&0xC0)==0xC0) {
                            BFI(eb1, x1, eb2*8, 8);
                        } else {
                            STRB_IMM9(x1, ed, 0);
                        }
                        UFLAG_DF(x3, d_or8);
                        UFLAGS(0);
                        break;
                    case 4: //AND
                        INST_NAME("AND Eb, Ib");
                        if((nextop&0xC0)==0xC0) {
                            ed = (nextop&7);
                            eb1 = xEAX+(ed&3);
                            eb2 = ((ed&4)>>2);
                            UXTB(x1, eb1, eb2);
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                            LDRB_IMM9(x1, ed, 0);
                        }
                        u8 = F8;
                        AND_IMM8(x1, x1, u8);
                        UFLAG_RES(1);
                        if((nextop&0xC0)==0xC0) {
                            BFI(eb1, x1, eb2*8, 8);
                        } else {
                            STRB_IMM9(x1, ed, 0);
                        }
                        UFLAG_DF(x3, d_and8);
                        UFLAGS(0);
                        break;
                    case 5: //SUB
                        INST_NAME("SUB Eb, Ib");
                        if((nextop&0xC0)==0xC0) {
                            ed = (nextop&7);
                            eb1 = xEAX+(ed&3);
                            eb2 = ((ed&4)>>2);
                            UXTB(x1, eb1, eb2);
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                            LDRB_IMM9(x1, ed, 0);
                        }
                        UFLAG_OP1(x1);
                        u8 = F8;
                        UFLAG_IF{MOV32(x3, u8); UFLAG_OP2(x3); UFLAG_DF(x3, d_sub8);}
                        SUB_IMM8(x1, x1, u8);
                        UFLAG_RES(x1);
                        if((nextop&0xC0)==0xC0) {
                            BFI(eb1, x1, eb2*8, 8);
                        } else {
                            STRB_IMM9(x1, ed, 0);
                        }
                        UFLAGS(0);
                        break;
                    case 6: //XOR
                        INST_NAME("XOR Eb, Ib");
                        if((nextop&0xC0)==0xC0) {
                            ed = (nextop&7);
                            eb1 = xEAX+(ed&3);
                            eb2 = ((ed&4)>>2);
                            UXTB(x1, eb1, eb2);
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                            LDRB_IMM9(x1, ed, 0);
                        }
                        u8 = F8;
                        XOR_IMM8(x1, x1, u8);
                        UFLAG_RES(x1);
                        if((nextop&0xC0)==0xC0) {
                            BFI(eb1, x1, eb2*8, 8);
                        } else {
                            STRB_IMM9(x1, ed, 0);
                        }
                        UFLAG_DF(x3, d_xor8);
                        UFLAGS(0);
                        break;
                    case 7: //CMP
                        INST_NAME("CMP Eb, Ib");
                        if((nextop&0xC0)==0xC0) {
                            ed = (nextop&7);
                            eb1 = xEAX+(ed&3);
                            eb2 = ((ed&4)>>2);
                            UXTB(x1, eb1, eb2);
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                            LDRB_IMM9(x1, ed, 0);
                        }
                        u8 = F8;
                        MOV32(x2, u8);
                        CALL(cmp8, -1, 0);
                        UFLAGS(1);
                        break;
                    default:
                        INST_NAME("GRP1 Eb, Ib");
                        ok = 0;
                        DEFAULT;
                }
                break;
            case 0x81:
            case 0x83:
                nextop = F8;
                switch((nextop>>3)&7) {
                    case 0: //ADD
                        if(opcode==0x81) {
                            INST_NAME("ADD Ed, Id");
                        } else {
                            INST_NAME("ADD Ed, Ib");
                        }
                        GETED;
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        UFLAG_OP1(ed);
                        if(i32>=0 && i32<256) {
                            UFLAG_IF{
                                MOV32(x3, i32); UFLAG_OP2(x3);
                            };
                            ADD_IMM8(ed, ed, i32);
                        } else {
                            MOV32(x3, i32);
                            UFLAG_OP2(3);
                            ADD_REG_LSL_IMM8(ed, ed, x3, 0);
                        }
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_add32);
                        UFLAGS(0);
                        break;
                    case 1: //OR
                        if(opcode==0x81) {INST_NAME("OR Ed, Id");} else {INST_NAME("OR Ed, Ib");}
                        GETED;
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        if(i32>0 && i32<256) {
                            ORR_IMM8(ed, ed, i32, 0);
                        } else {
                            MOV32(x3, i32);
                            ORR_REG_LSL_IMM8(ed, ed, x3, 0);
                        }
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_or32);
                        UFLAGS(0);
                        break;
                    case 2: //ADC
                        if(opcode==0x81) {INST_NAME("ADC Ed, Id");} else {INST_NAME("ADC Ed, Ib");}
                        GETEDH(x1);
                        if(ed!=x1) {MOV_REG(x1, ed);}
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        MOV32(x2, i32);
                        CALL(adc32, ed, (wback?(1<<wback):0));
                        WBACK;
                        UFLAGS(1);
                        break;
                    case 3: //SBB
                        if(opcode==0x81) {INST_NAME("SBB Ed, Id");} else {INST_NAME("SBB Ed, Ib");}
                        GETEDH(x1);
                        if(ed!=x1) {MOV_REG(x1, ed);}
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        MOV32(x2, i32);
                        CALL(sbb32, ed, (wback?(1<<wback):0));
                        WBACK;
                        UFLAGS(1);
                        break;
                    case 4: //AND
                        if(opcode==0x81) {INST_NAME("AND Ed, Id");} else {INST_NAME("AND Ed, Ib");}
                        GETED;
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        if(i32>0 && i32<256) {
                            AND_IMM8(ed, ed, i32);
                        } else {
                            MOV32(x3, i32);
                            AND_REG_LSL_IMM8(ed, ed, x3, 0);
                        }
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_and32);
                        UFLAGS(0);
                        break;
                    case 5: //SUB
                        if(opcode==0x81) {INST_NAME("SUB Ed, Id");} else {INST_NAME("SUB Ed, Ib");}
                        GETED;
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        UFLAG_OP1(ed);
                        if(i32>0 && i32<256) {
                            UFLAG_IF{
                                MOV32(x3, i32); UFLAG_OP2(x3);
                            }
                            SUB_IMM8(ed, ed, i32);
                        } else {
                            MOV32(x3, i32);
                            UFLAG_OP2(x3);
                            SUB_REG_LSL_IMM8(ed, ed, x3, 0);
                        }
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_sub32);
                        UFLAGS(0);
                        break;
                    case 6: //XOR
                        if(opcode==0x81) {INST_NAME("XOR Ed, Id");} else {INST_NAME("XOR Ed, Ib");}
                        GETED;
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        if(i32>0 && i32<256) {
                            XOR_IMM8(ed, ed, i32);
                        } else {
                            MOV32(x3, i32);
                            XOR_REG_LSL_IMM8(ed, ed, x3, 0);
                        }
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_xor32);
                        UFLAGS(0);
                        break;
                    case 7: //CMP
                        if(opcode==0x81) {INST_NAME("CMP Ed, Id");} else {INST_NAME("CMP Ed, Ib");}
                        GETEDH(x1);
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        if(ed!=x1) {MOV_REG(x1,ed);}
                        MOV32(x2, i32);
                        CALL(cmp32, -1, 0);
                        UFLAGS(1);
                        break;
                }
                break;
            case 0x84:
                INST_NAME("TEST Eb, Gb");
                nextop=F8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    UXTB(x1, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                    LDRB_IMM9(x1, ed, 0);
                }
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                UXTB(x2, gb1, gb2);
                CALL(test8, -1, 0);
                UFLAGS(1);
                break;
            case 0x85:
                INST_NAME("TEST Ed, Gd");
                nextop=F8;
                GETGD;
                GETEDH(x1);
                if(ed!=x1) {MOV_REG(x1, ed);};
                MOV_REG(x2, gd);
                CALL(test32, -1, 0);
                UFLAGS(1);
                break;
            case 0x86:
                INST_NAME("(LOCK)XCHG Eb, Gb");
                // Lock
                PUSH(xSP, (1<<xEmu));   // save Emu
                LDR_IMM9(xEmu, xEmu, offsetof(x86emu_t, context));
                MOV32(x1, offsetof(box86context_t, mutex_lock));   // offset is way to big for imm8
                ADD_REG_LSL_IMM8(xEmu, xEmu, x1, 0);
                CALL(pthread_mutex_lock, -1, 0);
                POP(xSP, (1<<xEmu));
                // Do the swap
                nextop = F8;
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2);
                UXTB(x12, gb1, gb2);
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);
                    eb2 = ((ed&4)>>2);
                    UXTB(x1, eb1, eb2);
                    // do the swap 12 -> ed, 1 -> gd
                    BFI(gb1, x1, gb2*8, 8);
                    BFI(eb1, x12, eb2*8, 8);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                    LDRB_IMM9(x1, ed, 0);    // 1 gets eb
                    // do the swap 12 -> strb(ed), 1 -> gd
                    BFI(gb1, x1, gb2*8, 8);
                    STRB_IMM9(x12, ed, 0);
                }
                // Unlock
                PUSH(xSP, (1<<xEmu));   // save Emu
                LDR_IMM9(xEmu, xEmu, offsetof(x86emu_t, context));
                MOV32(x1, offsetof(box86context_t, mutex_lock));
                ADD_REG_LSL_IMM8(xEmu, xEmu, x1, 0);
                CALL(pthread_mutex_unlock, -1, 0);
                POP(xSP, (1<<xEmu));
                break;
            case 0x87:
                INST_NAME("(LOCK)XCHG Ed, Gd");
                // Lock
                PUSH(xSP, (1<<xEmu));   // save Emu
                LDR_IMM9(xEmu, xEmu, offsetof(x86emu_t, context));
                MOV32(x1, offsetof(box86context_t, mutex_lock));   // offset is way to big for imm8
                ADD_REG_LSL_IMM8(xEmu, xEmu, x1, 0);
                CALL(pthread_mutex_lock, -1, 0);
                POP(xSP, (1<<xEmu));
                // Do the swap
                nextop = F8;
                GETGD;
                GETED;
                // xor swap to avoid one more tmp reg
                XOR_REG_LSL_IMM8(gd, gd, ed, 0);
                XOR_REG_LSL_IMM8(ed, gd, ed, 0);
                XOR_REG_LSL_IMM8(gd, gd, ed, 0);
                WBACK;
                // Unlock
                PUSH(xSP, (1<<xEmu));   // save Emu
                LDR_IMM9(xEmu, xEmu, offsetof(x86emu_t, context));
                MOV32(x1, offsetof(box86context_t, mutex_lock));
                ADD_REG_LSL_IMM8(xEmu, xEmu, x1, 0);
                CALL(pthread_mutex_unlock, -1, 0);
                POP(xSP, (1<<xEmu));
                break;
            case 0x88:
                INST_NAME("MOV Eb, Gb");
                nextop = F8;
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = (gd&4)>>2;
                UXTB(12, gb1, gb2);
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);  // Ax, Cx, Dx or Bx
                    eb2 = ((ed&4)>>2)*8;    // L or H
                    BFI(eb1, x12, eb2, 8);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                    STRB_IMM9(x12, ed, 0);
                }
                break;
            case 0x89:
                INST_NAME("MOV Ed, Gd");
                nextop=F8;
                GETGD;
                if((nextop&0xC0)==0xC0) {   // reg <= reg
                    MOV_REG(xEAX+(nextop&7), gd);
                } else {                    // mem <= reg
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                    STR_IMM9(gd, ed, 0);
                }
                break;
            case 0x8A:
                INST_NAME("MOV Gb, Eb");
                nextop = F8;
                gd = (nextop&0x38)>>3;
                gb1 = xEAX+(gd&3);
                gb2 = ((gd&4)>>2)*8;
                if((nextop&0xC0)==0xC0) {
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);  // Ax, Cx, Dx or Bx
                    eb2 = (ed&4)>>2;    // L or H
                    UXTB(x12, eb1, eb2);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                    LDRB_IMM9(x12, ed, 0);
                }
                BFI(gb1, x12, gb2, 8);
                break;
            case 0x8B:
                INST_NAME("MOV Gd, Ed");
                nextop=F8;
                GETGD;
                if((nextop&0xC0)==0xC0) {   // reg <= reg
                    MOV_REG(gd, xEAX+(nextop&7));
                } else {                    // mem <= reg
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                    LDR_IMM9(gd, ed, 0);
                }
                break;

            case 0x8D:
                INST_NAME("LEA Gd, Ed");
                nextop=F8;
                GETGD;
                if((nextop&0xC0)==0xC0) {   // reg <= reg? that's an invalid operation
                    ok=0;
                    DEFAULT;
                } else {                    // mem <= reg
                    addr = geted(dyn, addr, ninst, nextop, &ed, gd);
                    if(gd!=ed) {    // it's sometimes used as a 3 bytes NOP
                        MOV_REG(gd, ed);
                    }
                }
                break;

            case 0x90:
                INST_NAME("NOP");
                break;
            case 0x91:
            case 0x92:
            case 0x93:
            case 0x94:
            case 0x95:
            case 0x96:
            case 0x97:
                INST_NAME("XCHG EAX, Reg");
                gd = xEAX+(opcode&0x07);
                MOV_REG(x2, xEAX);
                MOV_REG(xEAX, gd);
                MOV_REG(gd, x2);
                break;
            case 0x98:
                INST_NAME("CWDE");
                SXTH(xEAX, xEAX, 0);
                break;
            case 0x99:
                INST_NAME("CDQ");
                MOV_REG_ASR_IMM5(xEDX, xEAX, 0);    // 0 in ASR means only bit #31 everywhere
                break;

            case 0x9B:
                INST_NAME("FWAIT");
                break;

            case 0xA0:
                INST_NAME("MOV AL, Ob");
                u32 = F32;
                MOV32(x2, u32);
                LDRB_IMM9(x2, x2, 0);
                BFI(xEAX, x2, 0, 8);
                break;
            case 0xA1:
                INST_NAME("MOV, EAX, Od");
                u32 = F32;
                MOV32(x2, u32);
                LDR_IMM9(xEAX, x2, 0);
                break;
            case 0xA2:
                INST_NAME("MOV Ob, AL");
                u32 = F32;
                MOV32(x2, u32);
                STRB_IMM9(xEAX, x2, 0);
                break;
            case 0xA3:
                INST_NAME("MOV Od, EAX");
                u32 = F32;
                MOV32(x2, u32);
                STR_IMM9(xEAX, x2, 0);
                break;
            
            case 0xA8:
                INST_NAME("TEST AL, Ib");
                UXTB(x1, xEAX, 0);
                u8 = F8;
                MOV32(x2, u8);
                CALL(test8, -1, 0);
                UFLAGS(1);
                break;
            case 0xA9:
                INST_NAME("TEST EAX, Id");
                MOV_REG(x1, xEAX);
                i32 = F32S;
                MOV32(x2, i32);
                CALL(test32, -1, 0);
                UFLAGS(1);
                break;

            case 0xB0:
            case 0xB1:
            case 0xB2:
            case 0xB3:
                INST_NAME("MOV xL, Ib");
                u8 = F8;
                MOVW(x1, u8);
                gb1 = xEAX+(opcode&3);
                BFI(gb1, x1, 0, 8);
                break;
            case 0xB4:
            case 0xB5:
            case 0xB6:
            case 0xB7:
                INST_NAME("MOV xH, Ib");
                u8 = F8;
                MOVW(x1, u8);
                gb1 = xEAX+(opcode&3);
                BFI(gb1, x1, 8, 8);
                break;
            case 0xB8:
            case 0xB9:
            case 0xBA:
            case 0xBB:
            case 0xBC:
            case 0xBD:
            case 0xBE:
            case 0xBF:
                INST_NAME("MOV Reg, Id");
                gd = xEAX+(opcode&7);
                i32 = F32S;
                MOV32(gd, i32);
                break;

            case 0xC1:
                nextop = F8;
                switch((nextop>>3)&7) {
                    case 0:
                        INST_NAME("ROL Ed, Ib");
                        USEFLAG;
                        GETEDH(x12);
                        u8 = (F8)&0x1f;
                        if(u8) {
                            MOV_REG_ROR_IMM5(ed, ed, (32-u8));
                            WBACK;
                        }
                        UFLAG_IF {  // calculate flags directly
                            if(u8==1) {
                                MOV_REG_LSR_IMM5(x2, ed, 31);
                                ADD_REG_LSL_IMM8(x2, x2, ed, 0);
                                AND_IMM8(x2, x2, 1);
                                STR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_OF]));
                            }
                            if(u8) {
                                AND_IMM8(x2, ed, 0x1);
                                STR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                            }
                        }
                        UFLAGS(1);
                        break;
                    case 1:
                        INST_NAME("ROR Ed, Ib");
                        USEFLAG;
                        GETEDH(x12);
                        u8 = (F8)&0x1f;
                        UFLAG_OP1(ed);
                        if(u8) {
                            MOV_REG_ROR_IMM5(ed, ed, u8);
                            WBACK;
                        }
                        UFLAG_IF {  // calculate flags directly
                            if(u8==1) {
                                MOV_REG_LSR_IMM5(x2, ed, 30); // x2 = d>>30
                                XOR_REG_LSR_IMM8(x2, x2, x2, 1); // x2 = ((d>>30) ^ ((d>>30)>>1))
                                AND_IMM8(x2, x2, 1);    // x2 = (((d>>30) ^ ((d>>30)>>1)) &0x1)
                                STR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_OF]));
                            }
                            if(u8) {
                                MOV_REG_LSR_IMM5(x2, ed, 31);
                                STR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                            }
                        }
                        UFLAGS(1);
                        break;
                    case 2:
                        INST_NAME("RCL Ed, Ib");
                        GETEDW(x12, x1);
                        u8 = F8;
                        MOV_REG(x2, gd);
                        CALL_(rcl32, x1, (1<<x12));
                        SBACK(x1);
                        UFLAGS(1);
                        break;
                    case 3:
                        INST_NAME("RCR Ed, Ib");
                        GETEDW(x12, x1);
                        u8 = F8;
                        MOV_REG(x2, gd);
                        CALL_(rcl32, x1, (1<<x12));
                        SBACK(x1);
                        UFLAGS(1);
                        break;
                    case 4:
                    case 6:
                        INST_NAME("SHL Ed, Ib");
                        GETED;
                        u8 = (F8)&0x1f;
                        UFLAG_IF{
                            MOV32(x12, u8); UFLAG_OP2(x12)
                        };
                        UFLAG_OP1(ed);
                        MOV_REG_LSL_IMM5(ed, ed, u8);
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_shl32);
                        UFLAGS(0);
                        break;
                    case 5:
                        INST_NAME("SHR Ed, Ib");
                        GETED;
                        u8 = (F8)&0x1f;
                        UFLAG_IF{
                            MOV32(x12, u8); UFLAG_OP2(x12)
                        };
                        UFLAG_OP1(ed);
                        if(u8) {
                            MOV_REG_LSR_IMM5(ed, ed, u8);
                            WBACK;
                        }
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_shr32);
                        UFLAGS(0);
                        break;
                    case 7:
                        INST_NAME("SAR Ed, Ib");
                        GETED;
                        u8 = (F8)&0x1f;
                        UFLAG_IF{
                            MOV32(x12, u8); UFLAG_OP2(x12)
                        };
                        UFLAG_OP1(ed);
                        if(u8) {
                            MOV_REG_ASR_IMM5(ed, ed, u8);
                            WBACK;
                        }
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_sar32);
                        UFLAGS(0);
                        break;
                    default:
                        INST_NAME("GRP3 Ed, Ib");
                        ok = 0;
                        DEFAULT;
                }
                break;

            case 0xC2:
                INST_NAME("RETN");
                i32 = F16;
                retn_to_epilog(dyn, ninst, i32);
                need_epilog = 0;
                ok = 0;
                break;
            case 0xC3:
                INST_NAME("RET");
                ret_to_epilog(dyn, ninst);
                need_epilog = 0;
                ok = 0;
                break;

            case 0xC6:
                INST_NAME("MOV Eb, Ib");
                nextop=F8;
                if((nextop&0xC0)==0xC0) {   // reg <= u8
                    u8 = F8;
                    ed = (nextop&7);
                    eb1 = xEAX+(ed&3);  // Ax, Cx, Dx or Bx
                    eb2 = (ed&4)>>2;    // L or H
                    BIC_IMM8(eb1, eb1, 0xff, (eb2)?12:0);   // BFI needs a reg, so I keep the BIC/OR method here
                    ORR_IMM8(eb1, eb1, u8, (eb2)?12:0);
                } else {                    // mem <= u8
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1);
                    u8 = F8;
                    MOV32(x3, u8);
                    STRB_IMM9(x3, ed, 0);
                }
                break;
            case 0xC7:
                INST_NAME("MOV Ed, Id");
                nextop=F8;
                if((nextop&0xC0)==0xC0) {   // reg <= i32
                    i32 = F32S;
                    ed = xEAX+(nextop&7);
                    MOV32(ed, i32);
                } else {                    // mem <= i32
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                    i32 = F32S;
                    MOV32(x3, i32);
                    STR_IMM9(x3, ed, 0);
                }
                break;

            case 0xC9:
                INST_NAME("LEAVE");
                MOV_REG(xESP, xEBP);
                POP(xESP, (1<<xEBP));
                break;

            case 0xCC:
                if(PK(0)=='S' && PK(1)=='C') {
                    addr+=2;
                    BARRIER(2);
                    UFLAGS(1);  // cheating...
                    INST_NAME("Special Box86 instruction");
                    if((PK(0)==0) && (PK(1)==0) && (PK(2)==0) && (PK(3)==0))
                    {
                        addr+=4;
                        MESSAGE(LOG_DEBUG, "Exit x86 Emu\n");
                        MOV32(x12, ip+1+2);
                        STM(xEmu, (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12));
                        MOV32(x1, 1);
                        STR_IMM9(x1, xEmu, offsetof(x86emu_t, quit));
                        ok = 0;
                        need_epilog = 1;
                    } else {
                        MOV32(x12, ip+1); // read the 0xCC
                        addr+=4+4;
                        STM(xEmu, (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12));
                        CALL_(x86Int3, -1, 0);
                        LDM(xEmu, (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12));
                        MOV32(x3, ip+1+2+4+4); // expected return address
                        CMPS_REG_LSL_IMM8(xEIP, x3, 0);
                        i32 = dyn->insts[ninst].mark-(dyn->arm_size+8);
                        Bcond(cNE, i32);
                        LDR_IMM9(x1, xEmu, offsetof(x86emu_t, quit));
                        CMPS_IMM8(x1, 1);
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                        Bcond(cNE, i32);
                        MARK;
                        jump_to_epilog(dyn, 0, 12, ninst);
                    }
                } else {
                    INST_NAME("INT 3");
                    ok = 0;
                    DEFAULT;
                }
                break;
            case 0xCD:
                if(PK(0)==0x80) {
                    u8 = F8;
                    UFLAGS(1);  // cheating...
                    INST_NAME("Syscall");
                    MOV32(12, ip+2);
                    STM(xEmu, (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12));
                    CALL_(x86Syscall, -1, 0);
                    LDM(xEmu, (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12));
                    MOVW(x2, addr);
                    CMPS_REG_LSL_IMM8(x12, x2, 0);
                    i32 = 4*4-8;    // 4 instructions to epilog, if IP is not what is expected
                    Bcond(cNE, i32);
                    LDR_IMM9(x1, xEmu, offsetof(x86emu_t, quit));
                    CMPS_IMM8(x1, 1);
                    i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                    Bcond(cNE, i32);
                    jump_to_epilog(dyn, 0, 12, ninst);
                } else {
                    INST_NAME("INT Ib");
                    ok = 0;
                    DEFAULT;
                }
                break;

            case 0xD1:
            case 0xD3:
                nextop = F8;
                switch((nextop>>3)&7) {
                    case 0:
                        if(opcode==0xD1) {
                            INST_NAME("ROL Ed, 1");
                            MOVW(x3, 31);
                        } else {
                            INST_NAME("ROL Ed, CL");
                            AND_IMM8(x3, xECX, 0x1f);
                            RSB_IMM8(x3, x3, 0x20);
                            AND_IMM8(x3, x3, 0x1f); // usefull?
                        }
                        USEFLAG;
                        GETEDH(x12);
                        MOV_REG_ROR_REG(ed, ed, x3);
                        WBACK;
                        UFLAG_IF {  // calculate flags directly
                            if(u8==1) {
                                MOV_REG_LSR_IMM5(x2, ed, 31);
                                ADD_REG_LSL_IMM8(x2, x2, ed, 0);
                                AND_IMM8(x2, x2, 1);
                                STR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_OF]));
                            }
                            if(u8) {
                                AND_IMM8(x2, ed, 0x1);
                                STR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                            }
                        }
                        UFLAGS(1);
                        break;
                    case 1:
                        if(opcode==0xD1) {
                            INST_NAME("ROR Ed, 1");
                            MOVW(x3, 1);
                        } else {
                            INST_NAME("ROR Ed, CL");
                            AND_IMM8(x3, xECX, 0x1f);
                        }
                        USEFLAG;
                        GETEDH(x12);
                        u8 = (F8)&0x1f;
                        UFLAG_OP1(ed);
                        if(u8) {
                            MOV_REG_ROR_REG(ed, ed, x3);
                            WBACK;
                        }
                        UFLAG_IF {  // calculate flags directly
                            if(u8==1) {
                                MOV_REG_LSR_IMM5(x2, ed, 30); // x2 = d>>30
                                XOR_REG_LSR_IMM8(x2, x2, x2, 1); // x2 = ((d>>30) ^ ((d>>30)>>1))
                                AND_IMM8(x2, x2, 1);    // x2 = (((d>>30) ^ ((d>>30)>>1)) &0x1)
                                STR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_OF]));
                            }
                            if(u8) {
                                MOV_REG_LSR_IMM5(x2, ed, 31);
                                STR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                            }
                        }
                        UFLAGS(1);
                        break;
                    case 2:
                        if(opcode==0xD1) {
                            INST_NAME("RCL Ed, 1");
                            MOVW(x2, 1);
                        } else {
                            INST_NAME("RCL Ed, CL");
                            AND_IMM8(x2, xECX, 0x1f);
                        }
                        GETEDW(x12, x1);
                        CALL_(rcl32, x1, (1<<x12));
                        SBACK(x1);
                        UFLAGS(1);
                        break;
                    case 3:
                        if(opcode==0xD1) {
                            INST_NAME("RCR Ed, 1");
                            MOVW(x2, 1);
                        } else {
                            INST_NAME("RCR Ed, CL");
                            AND_IMM8(x2, xECX, 0x1f);
                        }
                        GETEDW(x12, x1);
                        CALL_(rcl32, x1, (1<<x12));
                        SBACK(x1);
                        UFLAGS(1);
                        break;
                    case 4:
                    case 6:
                        if(opcode==0xD1) {
                            INST_NAME("SHL Ed, 1");
                            MOVW(x12, 1);
                        } else {
                            INST_NAME("SHL Ed, CL");
                            AND_IMM8(x12, xECX, 0x1f);
                        }
                        GETED;
                        UFLAG_OP2(x12)
                        UFLAG_OP1(ed);
                        MOV_REG_LSL_REG(ed, ed, x12);
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_shl32);
                        UFLAGS(0);
                        break;
                    case 5:
                        if(opcode==0xD1) {
                            INST_NAME("SHR Ed, 1");
                            MOVW(x12, 1);
                        } else {
                            INST_NAME("SHR Ed, CL");
                            AND_IMM8(x12, xECX, 0x1f);
                        }
                        GETED;
                        UFLAG_OP2(x12)
                        UFLAG_OP1(ed);
                        MOV_REG_LSR_REG(ed, ed, 12);
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_shr32);
                        UFLAGS(0);
                        break;
                    case 7:
                        if(opcode==0xD1) {
                            INST_NAME("SAR Ed, 1");
                            MOVW(x12, 1);
                        } else {
                            INST_NAME("SAR Ed, CL");
                            AND_IMM8(x12, xECX, 0x1f);
                        }
                        GETED;
                        UFLAG_OP2(x12)
                        UFLAG_OP1(ed);
                        MOV_REG_ASR_REG(ed, ed, x12);
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(x3, d_sar32);
                        UFLAGS(0);
                        break;
                    default:
                        if(opcode==0xD1) {INST_NAME("GRP3 Ed, 1");} else {INST_NAME("GRP3 Ed, CL");}
                        ok = 0;
                        DEFAULT;
                }
                break;

            case 0xE8:
                INST_NAME("CALL Id");
                i32 = F32S;
                if(isNativeCall(dyn, addr+i32, &natcall, &retn)) {
                    BARRIER(2);
                    MOV32(x2, addr);
                    PUSH(xESP, 1<<x2);
                    MESSAGE(LOG_DUMP, "Native Call (retn=%d)\n", retn);
                    UFLAGS(1);  // cheating...
                    // calling a native function
                    MOV32(x12, natcall); // read the 0xCC
                    STM(xEmu, (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12));
                    CALL_(x86Int3, -1, 0);
                    LDM(xEmu, (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12));
                    MOV32(x3, natcall+2+4+4);
                    CMPS_REG_LSL_IMM8(xEIP, x3, 0);
                    i32 = dyn->insts[ninst].mark-(dyn->arm_size+8);
                    Bcond(cNE, i32);    // Not the expected address, exit dynarec block
                    POP(xESP, (1<<xEIP));   // pop the return address
                    if(retn) {
                        ADD_IMM8(xESP, xESP, retn);
                    }
                    MOV32(x3, addr);
                    CMPS_REG_LSL_IMM8(xEIP, x3, 0);
                    i32 = dyn->insts[ninst].mark-(dyn->arm_size+8);
                    Bcond(cNE, i32);    // Not the expected address again
                    LDR_IMM9(x1, xEmu, offsetof(x86emu_t, quit));
                    CMPS_IMM8(x1, 1);
                    i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                    Bcond(cNE, i32);    // not quitting, so lets continue
                    MARK;
                    jump_to_epilog(dyn, 0, xEIP, ninst);
                } else if ((i32==0) && ((PK(0)>=0x58) && (PK(0)<=0x5F))) {
                    MESSAGE(LOG_DUMP, "Hack for Call 0, Pop reg\n");
                    UFLAGS(1);
                    u8 = F8;
                    gd = xEAX+(u8&7);
                    MOV32(gd, addr);
                } else if ((PK(i32+0)==0x8B) && (((PK(i32+1))&0xC7)==0x04) && (PK(i32+2)==0x24) && (PK(i32+3)==0xC3)) {
                    MESSAGE(LOG_DUMP, "Hack for Call x86.get_pc_thunk.reg\n");
                    UFLAGS(1);
                    u8 = PK(i32+1);
                    gd = xEAX+((u8&0x38)>>3);
                    MOV32(gd, addr);
                } else {
                    MOV32(x2, addr);
                    PUSH(xESP, 1<<x2);
                    // regular call
                    jump_to_linker(dyn, addr+i32, 0, ninst);
                    need_epilog = 0;
                    ok = 0;
                }
                break;
            case 0xE9:
            case 0xEB:
                if(opcode==0xE9) {
                    INST_NAME("JMP Id");
                    i32 = F32S;
                } else {
                    INST_NAME("JMP Ib");
                    i32 = F8S;
                }
                JUMP(addr+i32);
                if(dyn->insts) {
                    if(dyn->insts[ninst].x86.jmp_insts==-1) {
                        // out of the block
                        jump_to_linker(dyn, addr+i32, 0, ninst);
                    } else {
                        // inside the block
                        tmp = dyn->insts[dyn->insts[ninst].x86.jmp_insts].address-(dyn->arm_size+8);
                        Bcond(c__, tmp);
                    }
                }
                need_epilog = 0;
                ok = 0;
                break;

        case 0xF2:                      /* REPNZ prefix */
        case 0xF3:                      /* REPZ prefix */
            nextop = F8;
            if(nextop==0x0F) {
                /*if(opcode==0xF3) {
                    #include "runf30f.h"
                } else {
                    #include "runf20f.h"
                }*/
                INST_NAME("F2/F3 0F ...");
                ok = 0;
                DEFAULT;
            } else if(nextop==0x66) {
                nextop = F8;
                #if 0
                tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
                tmp32u = R_ECX;
                switch(nextop) {
                    case 0xA5:              /* REP MOVSW */
                        tmp8s *= 2;
                        while(tmp32u) {
                            --tmp32u;
                            *(uint16_t*)R_EDI = *(uint16_t*)R_ESI;
                            R_EDI += tmp8s;
                            R_ESI += tmp8s;
                        }
                        break;
                    case 0xAB:              /* REP STOSW */
                        tmp8s *= 2;
                        while(tmp32u) {
                            --tmp32u;
                            *(uint16_t*)R_EDI = R_AX;
                            R_EDI += tmp8s;
                        }
                        break;
                    default:
                        goto _default;
                }
                R_ECX = tmp32u;
                #endif
                INST_NAME("F2/F3 66 ...");
                ok = 0;
                DEFAULT;
            } else {
                switch(nextop) {
                    case 0x90:
                        INST_NAME("PAUSE");
                        break;
                    case 0xC3:
                        INST_NAME("(REPZ) RET");
                        ret_to_epilog(dyn, 0);
                        ok = 0;
                        break;
                    case 0xA4:
                        INST_NAME("REP MOVSB");
                        LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_DF]));
                        CMPS_IMM8(x3, 1);
                        MOVW(x3, 1);
                        RSB_COND_IMM8(cEQ, x3, x3, 0);
                        // DF=0, increment addresses, DF=1 decrement addresses
                        TSTS_REG_LSL_IMM8(xECX, xECX, xECX, 0);
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                        Bcond(cEQ, i32);    // end of loop
                        MARK;
                        LDRBAI_REG_LSL_IMM5(x1, xESI, x3, 0);
                        STRBAI_REG_LSL_IMM5(x1, xEDI, x3, 0);
                        SUBS_IMM8(xECX, xECX, 1);
                        i32 = GETMARK-(dyn->arm_size+8);
                        Bcond(cNE, i32);
                        // done
                        break;
                    case 0xA5:
                        INST_NAME("REP MOVSD");
                        LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_DF]));
                        CMPS_IMM8(x3, 1);
                        MOVW(x3, 4);
                        RSB_COND_IMM8(cEQ, x3, x3, 0);
                        // DF=0, increment addresses, DF=1 decrement addresses
                        TSTS_REG_LSL_IMM8(xECX, xECX, xECX, 0);
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                        Bcond(cEQ, i32);    // end of loop
                        MARK;
                        LDRAI_REG_LSL_IMM5(x1, xESI, x3, 0);
                        STRAI_REG_LSL_IMM5(x1, xEDI, x3, 0);
                        SUBS_IMM8(xECX, xECX, 1);
                        i32 = GETMARK-(dyn->arm_size+8);
                        Bcond(cNE, i32);
                        // done
                        break;
                    case 0xA6:
                        if(opcode==0xF2) {INST_NAME("REPNZ CMPSB");} else {INST_NAME("REPZ CMPSB");}
                        LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_DF]));
                        CMPS_IMM8(x3, 1);
                        MOVW(x3, 1);
                        RSB_COND_IMM8(cEQ, x3, x3, 0);
                        // DF=0, increment addresses, DF=1 decrement addresses
                        TSTS_REG_LSL_IMM8(xECX, xECX, xECX, 0);
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                        Bcond(cEQ, i32);    // end of loop
                        MARK;
                        LDRBAI_REG_LSL_IMM5(x1, xESI, x3, 0);
                        LDRBAI_REG_LSL_IMM5(x2, xEDI, x3, 0);
                        CMPS_REG_LSL_IMM8(x1, x2, 0);
                        SUB_IMM8(xECX, xECX, 1);
                        i32 = GETMARK2-(dyn->arm_size+8);
                        if(opcode==0xF2) {
                            Bcond(cEQ, i32);
                        } else {
                            Bcond(cNE, i32);
                        }
                        TSTS_REG_LSL_IMM8(xECX, xECX, xECX, 0);
                        i32 = GETMARK-(dyn->arm_size+8);
                        Bcond(cNE, i32);
                        // done, finish cmp test
                        MARK2;
                        CALL(cmp8, -1, 0);
                        UFLAGS(0);  // in some case, there is no comp, so cannot use "1"
                        break;
                    case 0xA7:
                        if(opcode==0xF2) {INST_NAME("REPNZ CMPSD");} else {INST_NAME("REPZ CMPSD");}
                        LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_DF]));
                        CMPS_IMM8(x3, 1);
                        MOVW(x3, 4);
                        RSB_COND_IMM8(cEQ, x3, x3, 0);
                        // DF=0, increment addresses, DF=1 decrement addresses
                        TSTS_REG_LSL_IMM8(xECX, xECX, xECX, 0);
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                        Bcond(cEQ, i32);    // end of loop
                        MARK;
                        LDRAI_REG_LSL_IMM5(x1, xESI, x3, 0);
                        LDRAI_REG_LSL_IMM5(x2, xEDI, x3, 0);
                        CMPS_REG_LSL_IMM8(x1, x2, 0);
                        SUB_IMM8(xECX, xECX, 1);
                        i32 = GETMARK2-(dyn->arm_size+8);
                        if(opcode==0xF2) {
                            Bcond(cEQ, i32);
                        } else {
                            Bcond(cNE, i32);
                        }
                        TSTS_REG_LSL_IMM8(xECX, xECX, xECX, 0);
                        i32 = GETMARK-(dyn->arm_size+8);
                        Bcond(cNE, i32);
                        // done, finish with cmp test
                        MARK2;
                        CALL(cmp32, -1, 0);
                        UFLAGS(0);  // in some case, there is no comp, so cannot use "1"
                        break;
                    case 0xAA:
                        INST_NAME("REP STOSB");
                        LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_DF]));
                        CMPS_IMM8(x3, 1);
                        MOVW(x3, 1);
                        RSB_COND_IMM8(cEQ, x3, x3, 0);
                        // DF=0, increment addresses, DF=1 decrement addresses
                        TSTS_REG_LSL_IMM8(xECX, xECX, xECX, 0);
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                        Bcond(cEQ, i32);    // end of loop
                        MARK;
                        STRBAI_REG_LSL_IMM5(xEAX, xEDI, x3, 0);
                        SUBS_IMM8(xECX, xECX, 1);
                        i32 = GETMARK-(dyn->arm_size+8);
                        Bcond(cNE, i32);
                        break;
                    case 0xAB:
                        INST_NAME("REP STOSD");
                        LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_DF]));
                        CMPS_IMM8(x3, 1);
                        MOVW(x3, 4);
                        RSB_COND_IMM8(cEQ, x3, x3, 0);
                        // DF=0, increment addresses, DF=1 decrement addresses
                        TSTS_REG_LSL_IMM8(xECX, xECX, xECX, 0);
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                        Bcond(cEQ, i32);    // end of loop
                        MARK;
                        STRAI_REG_LSL_IMM5(xEAX, xEDI, x3, 0);
                        SUBS_IMM8(xECX, xECX, 1);
                        i32 = GETMARK-(dyn->arm_size+8);
                        Bcond(cNE, i32);
                        break;
                    case 0xAC:
                        INST_NAME("REP LODSB");
                        LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_DF]));
                        CMPS_IMM8(x3, 1);
                        MOV_REG_LSR_IMM5(x3, xECX, 0);
                        RSB_COND_IMM8(cEQ, x3, x3, 0);
                        // DF=0, increment addresses, DF=1 decrement addresses
                        TSTS_REG_LSL_IMM8(xECX, xECX, xECX, 0);
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                        Bcond(cEQ, i32);    // end of loop
                        LDRBAI_REG_LSL_IMM5(x1, xESI, x3, 0);
                        BFI(xEAX, x1, 0, 8);
                        XOR_REG_LSL_IMM8(xECX, xECX, xECX, 0);
                        break;
                    case 0xAD:
                        INST_NAME("REP LODSB");
                        LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_DF]));
                        CMPS_IMM8(x3, 1);
                        MOV_REG_LSR_IMM5(x3, xECX, 2);
                        RSB_COND_IMM8(cEQ, x3, x3, 0);
                        // DF=0, increment addresses, DF=1 decrement addresses
                        TSTS_REG_LSL_IMM8(xECX, xECX, xECX, 0);
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                        Bcond(cEQ, i32);    // end of loop
                        LDRAI_REG_LSL_IMM5(x1, xESI, x3, 0);
                        XOR_REG_LSL_IMM8(xECX, xECX, xECX, 0);
                        break;
                    case 0xAE:
                        if(opcode==0xF2) {INST_NAME("REPNZ SCASB");} else {INST_NAME("REPZ SCASB");}
                        LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_DF]));
                        CMPS_IMM8(x3, 1);
                        MOVW(x3, 1);
                        RSB_COND_IMM8(cEQ, x3, x3, 0);
                        UXTB(x1, xEAX, 0);
                        // DF=0, increment addresses, DF=1 decrement addresses
                        TSTS_REG_LSL_IMM8(xECX, xECX, xECX, 0);
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                        Bcond(cEQ, i32);    // end of loop
                        MARK;
                        LDRBAI_REG_LSL_IMM5(x2, xEDI, x3, 0);
                        CMPS_REG_LSL_IMM8(x1, x2, 0);
                        SUB_IMM8(xECX, xECX, 1);
                        i32 = GETMARK2-(dyn->arm_size+8);
                        if(opcode==0xF2) {
                            Bcond(cEQ, i32);
                        } else {
                            Bcond(cNE, i32);
                        }
                        TSTS_REG_LSL_IMM8(xECX, xECX, xECX, 0);
                        i32 = GETMARK-(dyn->arm_size+8);
                        Bcond(cNE, i32);
                        // done, finish with cmp test
                        MARK2;
                        CALL(cmp8, -1, 0);
                        UFLAGS(0);  // in some case, there is no comp, so cannot use "1"
                        break;
                    case 0xAF:
                        if(opcode==0xF2) {INST_NAME("REPNZ SCASD");} else {INST_NAME("REPZ SCASD");}
                        LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_DF]));
                        CMPS_IMM8(x3, 1);
                        MOVW(x3, 4);
                        RSB_COND_IMM8(cEQ, x3, x3, 0);
                        MOV_REG_LSR_IMM5(x1, xEAX, 0);
                        // DF=0, increment addresses, DF=1 decrement addresses
                        TSTS_REG_LSL_IMM8(xECX, xECX, xECX, 0);
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                        Bcond(cEQ, i32);    // end of loop
                        MARK;
                        LDRAI_REG_LSL_IMM5(x2, xEDI, x3, 0);
                        CMPS_REG_LSL_IMM8(x1, x2, 0);
                        SUB_IMM8(xECX, xECX, 1);
                        i32 = GETMARK2-(dyn->arm_size+8);
                        if(opcode==0xF2) {
                            Bcond(cEQ, i32);
                        } else {
                            Bcond(cNE, i32);
                        }
                        TSTS_REG_LSL_IMM8(xECX, xECX, xECX, 0);
                        i32 = GETMARK-(dyn->arm_size+8);
                        Bcond(cNE, i32);
                        // done, finish with cmp test
                        MARK2;
                        CALL(cmp32, -1, 0);
                        UFLAGS(0);  // in some case, there is no comp, so cannot use "1"
                        break;
                    default:
                        INST_NAME("F2/F3 ...");
                        ok = 0;
                        DEFAULT;
                }
            }
            break;
                
            case 0xF6:
                nextop = F8;
                switch((nextop>>3)&7) {
                    case 0:
                    case 1:
                        INST_NAME("TEST Eb, Ib");
                        if((nextop&0xC0)==0xC0) {
                            ed = (nextop&7);
                            eb1 = xEAX+(ed&3);
                            eb2 = ((ed&4)>>2);
                            UXTB(x1, eb1, eb2);
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                            LDRB_IMM9(x1, ed, 0);
                        }
                        u8 = F8;
                        MOV32(x2, u8);
                        CALL(test8, -1, 0);
                        UFLAGS(1);
                        break;
                    case 2:
                        INST_NAME("NOT Eb");
                        if((nextop&0xC0)==0xC0) {
                            ed = (nextop&7);
                            eb1 = xEAX+(ed&3);
                            eb2 = ((ed&4)>>2);
                            UXTB(x1, eb1, eb2);
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                            LDRB_IMM9(x1, ed, 0);
                        }
                        MVN_REG_LSL_IMM8(x1, x1, 0);
                        if((nextop&0xC0)==0xC0) {
                            BFI(eb1, x1, eb2*8, 8);
                        } else {
                            STRB_IMM9(x1, ed, 0);
                        }
                        break;
                    case 3:
                        INST_NAME("NEG Ed");
                        if((nextop&0xC0)==0xC0) {
                            ed = (nextop&7);
                            eb1 = xEAX+(ed&3);
                            eb2 = ((ed&4)>>2);
                            UXTB(x1, eb1, eb2);
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                            LDRB_IMM9(x1, ed, 0);
                        }
                        RSB_IMM8(x1, x1, 0);
                        UFLAG_RES(x1);
                        if((nextop&0xC0)==0xC0) {
                            BFI(eb1, x1, eb2*8, 8);
                        } else {
                            STRB_IMM9(x1, ed, 0);
                        }
                        UFLAG_DF(x2, d_neg8);
                        UFLAGS(0);
                        break;
                    case 4:
                        INST_NAME("MUL AL, Ed");
                        if((nextop&0xC0)==0xC0) {
                            ed = (nextop&7);
                            eb1 = xEAX+(ed&3);
                            eb2 = ((ed&4)>>2);
                            UXTB(x1, eb1, eb2);
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                            LDRB_IMM9(x1, ed, 0);
                        }
                        STM(xEmu, (1<<xEAX));
                        CALL(mul8, -1, 0);
                        LDM(xEmu, (1<<xEAX));
                        UFLAGS(0);
                        break;
                    case 5:
                        INST_NAME("IMUL AL, Eb");
                        if((nextop&0xC0)==0xC0) {
                            ed = (nextop&7);
                            eb1 = xEAX+(ed&3);
                            eb2 = ((ed&4)>>2);
                            UXTB(x1, eb1, eb2);
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                            LDRB_IMM9(x1, ed, 0);
                        }
                        STM(xEmu, (1<<xEAX));
                        CALL(imul8, -1, 0);
                        LDM(xEmu, (1<<xEAX));
                        UFLAGS(0);
                        break;
                    case 6:
                        INST_NAME("DIV Eb");
                        if((nextop&0xC0)==0xC0) {
                            ed = (nextop&7);
                            eb1 = xEAX+(ed&3);
                            eb2 = ((ed&4)>>2);
                            UXTB(x1, eb1, eb2);
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                            LDRB_IMM9(x1, ed, 0);
                        }
                        STM(xEmu, (1<<xEAX));
                        CALL(div8, -1, 0);
                        LDM(xEmu, (1<<xEAX));
                        UFLAGS(1);
                        break;
                    case 7:
                        INST_NAME("IDIV Eb");
                        if((nextop&0xC0)==0xC0) {
                            ed = (nextop&7);
                            eb1 = xEAX+(ed&3);
                            eb2 = ((ed&4)>>2);
                            UXTB(x1, eb1, eb2);
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                            LDRB_IMM9(x1, ed, 0);
                        }
                        STM(xEmu, (1<<xEAX));
                        CALL(idiv8, -1, 0);
                        LDM(xEmu, (1<<xEAX));
                        UFLAGS(1);
                        break;
                }
                break;
            case 0xF7:
                nextop = F8;
                switch((nextop>>3)&7) {
                    case 0:
                    case 1:
                        INST_NAME("TEST Ed, Id");
                        GETEDH(x1);
                        i32 = F32S;
                        if(ed!=x1) {
                            MOV_REG(x1, ed);
                        }
                        MOV32(x2, i32);
                        CALL(test32, -1, 0);
                        UFLAGS(1);
                        break;
                    case 2:
                        INST_NAME("NOT Ed");
                        GETED;
                        MVN_REG_LSL_IMM8(ed, ed, 0);
                        WBACK;
                        break;
                    case 3:
                        INST_NAME("NEG Ed");
                        GETED;
                        UFLAG_OP1(ed);
                        RSB_IMM8(ed, ed, 0);
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(x2, d_neg32);
                        UFLAGS(0);
                        break;
                    case 4:
                        INST_NAME("MUL EAX, Ed");
                        UFLAG_DF(2, d_mul32);
                        GETED;
                        UMULL(xEDX, xEAX, ed, xEAX);
                        UFLAG_RES(xEAX);
                        UFLAG_OP1(xEDX);
                        UFLAGS(0);
                        break;
                    case 5:
                        INST_NAME("IMUL EAX, Ed");
                        UFLAG_DF(x2, d_imul32);
                        GETED;
                        SMULL(xEDX, xEAX, ed, xEAX);
                        UFLAG_RES(xEAX);
                        UFLAG_OP1(xEDX);
                        UFLAGS(0);
                        break;
                    case 6:
                        INST_NAME("DIV Ed");
                        GETEDH(x1);
                        if(ed!=x1) {MOV_REG(x1, ed);}
                        // need to store/restore ECX too, or use 2 instructions?
                        STM(xEmu, (1<<xEAX) | (1<<xECX) | (1<<xEDX));
                        CALL(div32, -1, 0);
                        LDM(xEmu, (1<<xEAX) | (1<<xECX) | (1<<xEDX));
                        UFLAGS(1);
                        break;
                    case 7:
                        INST_NAME("IDIV Ed");
                        GETEDH(x1);
                        if(ed!=x1) {MOV_REG(x1, ed);}
                        STM(xEmu, (1<<xEAX) | (1<<xECX) | (1<<xEDX));
                        CALL(idiv32, -1, 0);
                        LDM(xEmu, (1<<xEAX) | (1<<xECX) | (1<<xEDX));
                        UFLAGS(1);
                        break;
                }
                break;
            case 0xF8:
                INST_NAME("CLC");
                USEFLAG;
                MOVW(x1, 0);
                STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_CF]));
                break;
            case 0xF9:
                INST_NAME("STC");
                USEFLAG;
                MOVW(x1, 1);
                STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_CF]));
                break;

            case 0xFC:
                INST_NAME("CLD");
                MOVW(x1, 0);
                STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_DF]));
                break;
            case 0xFD:
                INST_NAME("STD");
                MOVW(x1, 1);
                STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_DF]));
                break;

            case 0xFF:
                nextop = F8;
                switch((nextop>>3)&7) {
                    case 0: // INC Ed
                        INST_NAME("INC Ed");
                        GETED;
                        UFLAG_OP1(ed);
                        ADD_IMM8(ed, ed, 1);
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(x1, d_inc32);
                        UFLAGS(0);
                        break;
                    case 1: //DEC Ed
                        INST_NAME("DEC Ed");
                        GETED;
                        UFLAG_OP1(ed);
                        SUB_IMM8(ed, ed, 1);
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(x1, d_dec32);
                        UFLAGS(0);
                        break;
                    case 2: // CALL Ed
                        INST_NAME("CALL Ed");
                        GETEDH(xEIP);
                        MOV32(x3, addr);
                        PUSH(xESP, 1<<x3);
                        jump_to_epilog(dyn, 0, ed, ninst);  // it's variable, so no linker
                        need_epilog = 0;
                        ok = 0;
                        break;
                    case 4: // JMP Ed
                        INST_NAME("JMP Ed");
                        GETEDH(xEIP);
                        jump_to_epilog(dyn, 0, ed, ninst);     // it's variable, so no linker
                        need_epilog = 0;
                        ok = 0;
                        break;
                    case 6: // Push Ed
                        INST_NAME("PUSH Ed");
                        GETED;
                        PUSH(xESP, 1<<ed);
                        break;

                    default:
                        INST_NAME("Grp5 Ed");
                        ok = 0;
                        DEFAULT;
                }
                break;

            default:
                ok = 0;
                DEFAULT;
        }
        ++ninst;
    }
    if(need_epilog)
        jump_to_epilog(dyn, ip, 0, ninst);  // no linker here, it's an unknow instruction
    FINI;
}