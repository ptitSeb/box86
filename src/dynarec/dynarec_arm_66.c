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

#include "dynarec_arm_helper.h"
#include "dynarec_arm_functions.h"


uintptr_t dynarec66(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint32_t u32;
    int32_t i32, j32;
    int16_t i16;
    uint16_t u16;
    uint8_t gd, ed;
    uint8_t wback, wb1;
    int lock;
    int fixedaddress;
    MAYUSE(u16);
    MAYUSE(u8);
    MAYUSE(j32);
    MAYUSE(lock);
    while(opcode==0x66) opcode = F8;    // "unlimited" 0x66 as prefix for variable sized NOP
    if((opcode==0x26) || (opcode==0x2E) || (opcode==0x36) || (opcode==0x3E)) opcode = F8;       // segment prefix are ignored
    switch(opcode) {
        
        case 0x01:
            INST_NAME("ADD Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1);
            emit_add16(dyn, ninst, x1, x2, x14, x3, (wb1 && (wback==x3))?1:0);
            EWBACK;
            break;
        case 0x03:
            INST_NAME("ADD Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2);
            emit_add16(dyn, ninst, x1, x2, x3, x14, 0);
            GWBACK;
            break;
        case 0x05:
            INST_NAME("ADD AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            UXTH(x1, xEAX, 0);
            emit_add16c(dyn, ninst, x1, i32, x3, x14);
            BFI(xEAX, x1, 0, 16);
            break;
        case 0x06:
            INST_NAME("PUSH ES");
            MOVW(x1, offsetof(x86emu_t, segs[_ES]));
            ADD_REG_LSL_IMM5(x1, xEmu, x1, 0);
            LDRH_IMM8(x2, x1, 0);
            SUB_IMM8(xESP, xESP, 2);
            STRH_IMM8(x2, xESP, 0);
            break;
        case 0x07:
            INST_NAME("POP ES");
            MOVW(x1, offsetof(x86emu_t, segs[_ES]));
            ADD_REG_LSL_IMM5(x1, xEmu, x1, 0);
            LDRH_IMM8(x2, xESP, 0);
            STRH_IMM8(x2, x1, 0);
            ADD_IMM8(xESP, xESP, 2);
            MOVW(x1, 0);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, segs_serial[_ES]));
            break;

        case 0x09:
            INST_NAME("OR Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1);
            emit_or16(dyn, ninst, x1, x2, x14, x2);
            EWBACK;
            break;
        case 0x0B:
            INST_NAME("OR Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2);
            emit_or16(dyn, ninst, x1, x2, x14, x3);
            GWBACK;
            break;
        case 0x0D:
            INST_NAME("OR AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            UXTH(x1, xEAX, 0);
            emit_or16c(dyn, ninst, x1, i32, x3, x14);
            BFI(xEAX, x1, 0, 16);
            break;
                
        case 0x0F:
            addr = dynarec660F(dyn, addr, ip, ninst, ok, need_epilog);
            break;
        case 0x11:
            INST_NAME("ADC Ew, Gw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1);
            emit_adc16(dyn, ninst, x1, x2, x14, x3, (wb1 && (wback==x3))?1:0);
            EWBACK;
            break;
        case 0x13:
            INST_NAME("ADC Gw, Ew");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2);
            emit_adc16(dyn, ninst, x1, x2, x14, x3, 0);
            GWBACK;
            break;
        case 0x15:
            INST_NAME("ADC AX, Iw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            UXTH(x1, xEAX, 0);
            emit_adc16c(dyn, ninst, x1, i32, x3, x14);
            BFI(xEAX, x1, 0, 16);
            break;

        case 0x19:
            INST_NAME("SBB Ew, Gw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1);
            emit_sbb16(dyn, ninst, x1, x2, x14, x3, (wb1 && (wback==x3))?1:0);
            EWBACK;
            break;
        case 0x1B:
            INST_NAME("SBB Gw, Ew");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2);
            emit_sbb16(dyn, ninst, x1, x2, x14, x3, 0);
            GWBACK;
            break;
        case 0x1D:
            INST_NAME("SBB AX, Iw");
            READFLAGS(X_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            UXTH(x1, xEAX, 0);
            emit_sbb16c(dyn, ninst, x1, i32, x3, x14);
            BFI(xEAX, x1, 0, 16);
            break;
        case 0x1E:
            INST_NAME("PUSH DS");
            MOVW(x1, offsetof(x86emu_t, segs[_DS]));
            LDRH_REG(x2, xEmu, x1);
            PUSH16(x2, xESP);
            break;

        case 0x21:
            INST_NAME("AND Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1);
            emit_and16(dyn, ninst, x1, x2, x14, x2);
            EWBACK;
            break;
        case 0x23:
            INST_NAME("AND Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2);
            emit_and16(dyn, ninst, x1, x2, x3, x14);
            GWBACK;
            break;
        case 0x25:
            INST_NAME("AND AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            UXTH(x1, xEAX, 0);
            emit_and16c(dyn, ninst, x1, i32, x3, x14);
            BFI(xEAX, x1, 0, 16);
            break;
        case 0x26:
            INST_NAME("ES:");
            // ignored
            break;

        case 0x29:
            INST_NAME("SUB Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1);
            emit_sub16(dyn, ninst, x1, x2, x14, x3, (wb1 && (wback==x3))?1:0);
            EWBACK;
            break;
        case 0x2B:
            INST_NAME("SUB Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2);
            emit_sub16(dyn, ninst, x1, x2, x3, x14, 0);
            GWBACK;
            break;
        case 0x2D:
            INST_NAME("SUB AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            UXTH(x1, xEAX, 0);
            emit_sub16c(dyn, ninst, x1, i32, x3, x14);
            BFI(xEAX, x1, 0, 16);
            break;

        case 0x31:
            INST_NAME("XOR Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1);
            emit_xor16(dyn, ninst, x1, x2, x14, x2);
            EWBACK;
            break;
        case 0x33:
            INST_NAME("XOR Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2);
            emit_xor16(dyn, ninst, x1, x2, x3, x14);
            GWBACK;
            break;
        case 0x35:
            INST_NAME("XOR AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            UXTH(x1, xEAX, 0);
            emit_xor16c(dyn, ninst, x1, i32, x3, x14);
            BFI(xEAX, x1, 0, 16);
            break;

        case 0x39:
            INST_NAME("CMP Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x2);
            GETEW(x1);
            emit_cmp16(dyn, ninst, x1, x2, x3, x14);
            break;
        case 0x3B:
            INST_NAME("CMP Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2);
            emit_cmp16(dyn, ninst, x1, x2, x3, x14);
            break;
        case 0x3D:
            INST_NAME("CMP AX, Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            i32 = F16;
            MOVW(x2, i32);
            UXTH(x1, xEAX, 0);
            emit_cmp16(dyn, ninst, x1, x2, x3, x14);
            break;

        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
            INST_NAME("INC Reg16");
            SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
            gd = xEAX+(opcode&7);
            UXTH(x1, gd, 0);
            emit_inc16(dyn, ninst, x1, x3, x14);
            BFI(gd, x1, 0, 16);
            break;
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:
            INST_NAME("DEC Reg16");
            SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
            gd = xEAX+(opcode&7);
            UXTH(x1, gd, 0);
            emit_dec16(dyn, ninst, x1, x3, x14);
            BFI(gd, x1, 0, 16);
            break;
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
            INST_NAME("PUSH Reg16");
            PUSH16(xEAX+(opcode&7), xESP);
            break;
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:                      
        case 0x5D:
        case 0x5E:
        case 0x5F:
            INST_NAME("POP Reg16");
            POP16(x1, xESP);
            BFI(xEAX+(opcode&7), x1, 0, 16);
            break;
        case 0x60:
            INST_NAME("PUSHA");
            MOV_REG(x1, xESP);
            PUSH16(xEAX, xESP);
            PUSH16(xECX, xESP);
            PUSH16(xEDX, xESP);
            PUSH16(xEBX, xESP);
            PUSH16(x1, xESP);
            PUSH16(xEBP, xESP);
            PUSH16(xESI, xESP);
            PUSH16(xEDI, xESP);
            break;
        case 0x61:
            INST_NAME("POPA");
            POP16(x1, xESP);
            BFI(xEDI, x1, 0, 16);
            POP16(x1, xESP);
            BFI(xESI, x1, 0, 16);
            POP16(x1, xESP);
            BFI(xEBP, x1, 0, 16);
            ADD_IMM8(xESP, xESP, 2);    //POP16(xESP, xESP);
            POP16(x1, xESP);
            BFI(xEBX, x1, 0, 16);
            POP16(x1, xESP);
            BFI(xEDX, x1, 0, 16);
            POP16(x1, xESP);
            BFI(xECX, x1, 0, 16);
            POP16(x1, xESP);
            BFI(xEAX, x1, 0, 16);
            break;

        case 0x68:
            INST_NAME("PUSH Iw");
            u16 = F16;
            MOVW(x2, u16);
            PUSH16(x2, xESP);
            break;
        case 0x69:
            INST_NAME("IMUL Gw,Ew,Iw");
            SETFLAGS(X_ALL, SF_PENDING);
            nextop = F8;
            GETEW(x1);
            i16 = F16S;
            MOVW(x2, i16);
            SMULBB(x2, ed, x2);
            UFLAG_RES(x2);
            BFI(xEAX+((nextop&0x38)>>3), x2, 0, 16);
            UFLAG_DF(x1, d_imul16);
            break;
        case 0x6A:
            INST_NAME("PUSH Ib");
            i16 = F8S;
            MOVW(x2, i16);
            PUSH16(x2, xESP);
            break;
        case 0x6B:
            INST_NAME("IMUL Gw,Ew,Ib");
            SETFLAGS(X_ALL, SF_PENDING);
            nextop = F8;
            GETEW(x1);
            i16 = F8S;
            MOVW(x2, i16);
            SMULBB(x2, ed, x2);
            UFLAG_RES(x2);
            BFI(xEAX+((nextop&0x38)>>3), x2, 0, 16);
            UFLAG_DF(x1, d_imul16);
            break;

        case 0x81:
        case 0x83:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: //ADD
                    if(opcode==0x81) {
                        INST_NAME("ADD Ew, Iw");
                    } else {
                        INST_NAME("ADD Ew, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    emit_add16c(dyn, ninst, ed, i16, x2, x14);
                    EWBACK;
                    break;
                case 1: //OR
                    if(opcode==0x81) {INST_NAME("OR Ew, Iw");} else {INST_NAME("OR Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    emit_or16c(dyn, ninst, x1, i16, x2, x14);
                    EWBACK;
                    break;
                case 2: //ADC
                    if(opcode==0x81) {INST_NAME("ADC Ew, Iw");} else {INST_NAME("ADC Ew, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    emit_adc16c(dyn, ninst, x1, i16, x2, x14);
                    EWBACK;
                    break;
                case 3: //SBB
                    if(opcode==0x81) {INST_NAME("SBB Ew, Iw");} else {INST_NAME("SBB Ew, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    emit_sbb16c(dyn, ninst, x1, i16, x2, x14);
                    EWBACK;
                    break;
                case 4: //AND
                    if(opcode==0x81) {INST_NAME("AND Ew, Iw");} else {INST_NAME("AND Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    emit_and16c(dyn, ninst, x1, i16, x2, x14);
                    EWBACK;
                    break;
                case 5: //SUB
                    if(opcode==0x81) {INST_NAME("SUB Ew, Iw");} else {INST_NAME("SUB Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    emit_sub16c(dyn, ninst, x1, i16, x2, x14);
                    EWBACK;
                    break;
                case 6: //XOR
                    if(opcode==0x81) {INST_NAME("XOR Ew, Iw");} else {INST_NAME("XOR Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    emit_xor16c(dyn, ninst, x1, i16, x2, x14);
                    EWBACK;
                    break;
                case 7: //CMP
                    if(opcode==0x81) {INST_NAME("CMP Ew, Iw");} else {INST_NAME("CMP Ew, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1);
                    if(opcode==0x81) i16 = F16S; else i16 = F8S;
                    if(i16) {
                        MOVW(x2, i16);
                        emit_cmp16(dyn, ninst, x1, x2, x3, x14);
                    } else
                        emit_cmp16_0(dyn, ninst, x1, x3, x14);
                    break;
            }
            break;
                
        case 0x85:
            INST_NAME("TEST Ew, Gw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEW(x1);
            GETGW(x2);
            emit_test16(dyn, ninst, x1, x2, x3, x14);
            break;

        case 0x87:
            INST_NAME("(LOCK) XCHG Ew, Gw");
            nextop = F8;
            if(MODREG) {
                GETGD;
                UXTH(x14, gd, 0);
                ed = xEAX + (nextop&7);
                UXTH(x1, ed, 0);
                // do the swap 14 -> ed, 1 -> gd
                BFI(gd, x1, 0, 16);
                BFI(ed, x14, 0, 16);
            } else {
                GETGD;
                UXTH(x14, gd, 0);
                SMDMB();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                #if 0
                MARKLOCK;
                // do the swap with exclusive locking
                LDREXH(x1, ed);
                // do the swap 14 -> strb(ed), 1 -> gd
                STREXH(x3, x14, ed);
                CMPS_IMM8(x3, 0);
                B_MARKLOCK(cNE);
                #else
                LDRH_IMM8(x1, ed, fixedaddress);
                STRH_IMM8(x14, ed, fixedaddress);
                #endif
                SMDMB();
                BFI(gd, x1, 0, 16);
            }
            break;

        case 0x89:
            INST_NAME("MOV Ew, Gw");
            nextop = F8;
            GETGD;  // don't need GETGW here
            if(MODREG) {
                ed = xEAX+(nextop&7);
                if(ed!=gd) {
                    BFI(ed, gd, 0, 16);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 255, 0, 0, &lock);
                STRH_IMM8(gd, ed, fixedaddress);
                SMWRITELOCK(lock);
            }
            break;
        case 0x8B:
            INST_NAME("MOV Gw, Ew");
            nextop = F8;
            GETGD;  // don't need GETGW neither
            if(MODREG) {
                ed = xEAX+(nextop&7);
                if(ed!=gd) {
                    BFI(gd, ed, 0, 16);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 255, 0, 0, &lock);
                SMREADLOCK(lock);
                LDRH_IMM8(x1, ed, fixedaddress);
                BFI(gd, x1, 0, 16);
            }
            break;
        case 0x8C:
            INST_NAME("MOV Ew,Seg");
            nextop = F8;
            MOV32(x2, offsetof(x86emu_t, segs[(nextop&0x38)>>3]));
            LDRH_REG(x1, xEmu, x2);
            if(MODREG) {
                ed = xEAX+(nextop&7);
                BFI(ed, x1, 0, 16);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 255, 0, 0, NULL);
                STRH_IMM8(x1, ed, fixedaddress);
                SMWRITE2();
            }
            break;

        case 0x8E:
            INST_NAME("MOV Seg,Ew");
            nextop = F8;
            if(MODREG) {
                ed = xEAX+(nextop&7);
                MOV32(x2, offsetof(x86emu_t, segs[(nextop&0x38)>>3]));
                STRH_REG(ed, xEmu, x2);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 255, 0, 0, NULL);
                LDRH_IMM8(x1, ed, fixedaddress);
                MOV32(x2, offsetof(x86emu_t, segs[(nextop&0x38)>>3]));
                STRH_REG(x1, xEmu, x2);
            }
            MOVW(x1, 0);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, segs_serial[(nextop&0x38)>>3]));
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
            INST_NAME("XCHG AX, Reg");
            gd = xEAX+(opcode&0x07);
            MOV_REG(x2, xEAX);
            BFI(xEAX, gd, 0, 16);
            BFI(gd, x2, 0, 16);
            break;
        case 0x98:
            INST_NAME("CBW");
            SXTB(x1, xEAX, 0);
            BFI(xEAX, x1, 0, 16);
            break;
        case 0x99:
            INST_NAME("CWD");
            SXTH(x1, xEAX, 0);
            MOV_REG_LSR_IMM5(x1, x1, 16);
            BFI(xEDX, x1, 0, 16);
            break;

        case 0x9C:
            INST_NAME("PUSHF (16b)");
            READFLAGS(X_ALL);
            STRHB_IMM8(xFlags, xESP, -2);
            break;
        case 0x9D:
            INST_NAME("POPF (16b)");
            SETFLAGS(X_ALL, SF_SET_NODF);    // lower 16bits is all flags handled in dynarec
            LDRHA_IMM8(x2, xESP, 2);
            MOV32(x1, 0x7FD7);
            AND_REG_LSL_IMM5(x2, x2, x1, 0);
            ORR_IMM8(x2, x2, 2, 0);
            BFI(xFlags, x2, 0, 16);
            SET_DFNONE(x1);
            break;

        case 0xA1:
            INST_NAME("MOV, AX, Od");
            u32 = F32;
            MOV32(x2, u32);
            if(isLockAddress(u32)) lock=1; else lock = 0;
            SMREADLOCK(lock);
            LDRH_IMM8(x2, x2, 0);
            BFI(xEAX, x2, 0, 16);
            break;
        case 0xA3:
            INST_NAME("MOV Od, AX");
            u32 = F32;
            MOV32(x2, u32);
            if(isLockAddress(u32)) lock=1; else lock = 0;
            STRH_IMM8(xEAX, x2, 0);
            SMWRITELOCK(lock);
            break;
        case 0xA5:
            INST_NAME("MOVSW");
            GETDIR(x3, 2);
            LDRHAI_REG_LSL_IMM5(x1, xESI, x3);
            STRHAI_REG_LSL_IMM5(x1, xEDI, x3);
            break;
        case 0xA7:
            INST_NAME("CMPSW");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            GETDIR(x3, 2);
            LDRHAI_REG_LSL_IMM5(x1, xESI, x3);
            LDRHAI_REG_LSL_IMM5(x2, xEDI, x3);
            emit_cmp16(dyn, ninst, x1, x2, x3, x14);
            break;

        case 0xA9:
            INST_NAME("TEST AX,Iw");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            u16 = F16;
            MOVW(x2, u16);
            UBFX(x1, xEAX, 0, 16);
            emit_test16(dyn, ninst, x1, x2, x3, x14);
            break;

        case 0xAB:
            INST_NAME("STOSW");
            GETDIR(x3, 2);
            STRHAI_REG_LSL_IMM5(xEAX, xEDI, x3);
            break;
        case 0xAD:
            INST_NAME("LODSW");
            GETDIR(x3, 2);
            LDRHAI_REG_LSL_IMM5(x1, xESI, x3);
            BFI(xEAX, x1, 0, 16);
            break;
        case 0xAF:
            INST_NAME("SCASW");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            GETDIR(x3, 2);
            UXTH(x1, xEAX, 0);
            LDRHAI_REG_LSL_IMM5(x2, xEDI, x3);
            emit_cmp16(dyn, ninst, x1, x2, x3, x14);
            break;

        case 0xB8:
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0xBD:
        case 0xBE:
        case 0xBF:
            INST_NAME("MOV Reg16, Iw");
            u16 = F16;
            MOVW(x1, u16);
            gd = xEAX+(opcode&7);
            BFI(gd, x1, 0, 16);
            break;
        case 0xC1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ew, Ib");
                    if(geted_ib(dyn, addr, ninst, nextop)&31) {
                        SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                        GETEW(x1);
                        u8 = F8;
                        emit_rol16c(dyn, ninst, x1, u8&15, x2, x14);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 1:
                    INST_NAME("ROR Ew, Ib");
                    if(geted_ib(dyn, addr, ninst, nextop)&31) {
                        SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                        GETEW(x1);
                        u8 = F8;
                        emit_ror16c(dyn, ninst, x1, u8&15, x2, x14);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 2:
                    INST_NAME("RCL Ew, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    GETEW(x1);
                    u8 = F8;
                    MOVW(x2, u8);
                    CALL_(rcl16, x1, (1<<x3));
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ew, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    GETEW(x1);
                    u8 = F8;
                    MOVW(x2, u8);
                    CALL_(rcr16, x1, (1<<x3));
                    EWBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ew, Ib");
                    if(geted_ib(dyn, addr, ninst, nextop)&31) {
                        SETFLAGS(X_ALL, SF_PENDING);
                        GETEW(x1);
                        u8 = F8;
                        emit_shl16c(dyn, ninst, x1, u8&31, x2, x14);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 5:
                    INST_NAME("SHR Ed, Ib");
                    if(geted_ib(dyn, addr, ninst, nextop)&31) {
                        SETFLAGS(X_ALL, SF_PENDING);
                        GETEW(x1);
                        u8 = F8;
                        emit_shr16c(dyn, ninst, x1, u8&31, x2, x14);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
                case 7:
                    INST_NAME("SAR Ed, Ib");
                    if(geted_ib(dyn, addr, ninst, nextop)&31) {
                        SETFLAGS(X_ALL, SF_PENDING);
                        GETSEW(x1);
                        u8 = F8;
                        emit_sar16c(dyn, ninst, x1, u8&31, x2, x14);
                        EWBACK;
                    } else {
                        FAKEED;
                        F8;
                    }
                    break;
            }
            break;

        case 0xC7:
            INST_NAME("MOV Ew, Iw");
            nextop = F8;
            if(MODREG) {
                ed = xEAX+(nextop&7);
                u16 = F16;
                MOVW(x1, u16);
                BFI(ed, x1, 0, 16);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 255, 0, 0, NULL);
                u16 = F16;
                MOVW(x1, u16);
                STRH_IMM8(x1, ed, fixedaddress);
            }
            break;

        case 0xD1:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ew, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETEW(x1);
                    emit_rol16c(dyn, ninst, x1, 1, x2, x14);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("ROR Ew, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETEW(x1);
                    emit_ror16c(dyn, ninst, x1, 1, x2, x14);
                    EWBACK;
                    break;
                case 2:
                    INST_NAME("RCL Ew, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    MOVW(x2, 1);
                    GETEW(x1);
                    CALL_(rcl16, x1, (1<<x3));
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ew, 1");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    MOVW(x2, 1);
                    GETEW(x1);
                    CALL_(rcr16, x1, (1<<x3));
                    EWBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ew, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETEW(x1);
                    emit_shl16c(dyn, ninst, x1, 1, x2, x14);
                    EWBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ew, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETEW(x1);
                    emit_shr16c(dyn, ninst, x1, 1, x2, x14);
                    EWBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ew, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETSEW(x1);
                    emit_sar16c(dyn, ninst, x1, 1, x2, x14);
                    EWBACK;
                    break;
            }
            break;
            
        case 0xD3:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ew, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    if(box86_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        TSTS_IMM8(xECX, 0x1f);
                        B_NEXT(cEQ);
                    }
                    AND_IMM8(x2, xECX, 0x0f);
                    RSB_IMM8(x2, x2, 16);
                    GETEW(x1);
                    ORR_REG_LSL_IMM5(ed, ed, ed, 16);
                    MOV_REG_LSR_REG(ed, ed, x2);
                    EWBACK;
                    UFLAG_IF {  // calculate flags directly
                        CMPS_IMM8(x2, 15);
                            ADD_REG_LSR_IMM5_COND(cEQ, x3, ed, ed, 15);
                            BFI_COND(cEQ, xFlags, x3, F_OF, 1);
                        BFI(xFlags, ed, F_CF, 1);
                        UFLAG_DF(x2, d_none);
                    }
                    break;
                case 1:
                    INST_NAME("ROR Ew, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    if(box86_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        TSTS_IMM8(xECX, 0x1f);
                        B_NEXT(cEQ);
                    }
                    AND_IMM8(x2, xECX, 0x0f);
                    GETEW(x1);
                    ORR_REG_LSL_IMM5(ed, ed, ed, 16);
                    MOV_REG_LSR_REG(ed, ed, x2);
                    EWBACK;
                    UFLAG_IF {  // calculate flags directly
                        CMPS_IMM8(x2, 1);
                            MOV_REG_LSR_IMM5_COND(cEQ, x2, ed, 14); // x2 = d>>14
                            XOR_REG_LSR_IMM8_COND(cEQ, x2, x2, x2, 1); // x2 = ((d>>14) ^ ((d>>14)>>1))
                            BFI_COND(cEQ, xFlags, x2, F_OF, 1);
                        MOV_REG_LSR_IMM5(x2, ed, 15);
                        BFI(xFlags, x2, F_CF, 1);
                        UFLAG_DF(x2, d_none);
                    }
                    break;
                case 2:
                    INST_NAME("RCL Ew, CL");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    AND_IMM8(x2, xECX, 0x1f);
                    GETEW(x1);
                    CALL_(rcl16, x1, (1<<x3));
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("RCR Ew, CL");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    AND_IMM8(x2, xECX, 0x1f);
                    GETEW(x1);
                    CALL_(rcr16, x1, (1<<x3));
                    EWBACK;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ew, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    if(box86_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        ANDS_IMM8(x2, xECX, 0x1f);
                        B_NEXT(cEQ);
                    } else {
                        AND_IMM8(x2, xECX, 0x1f);
                    }
                    GETEW(x1);
                    emit_shl16(dyn, ninst, x1, x2, x2, x14);
                    EWBACK;
                    break;
                case 5:
                    INST_NAME("SHR Ew, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    if(box86_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        ANDS_IMM8(x2, xECX, 0x1f);
                        B_NEXT(cEQ);
                    } else {
                        AND_IMM8(x2, xECX, 0x1f);
                    }
                    GETEW(x1);
                    emit_shr16(dyn, ninst, x1, x2, x2, x14);
                    EWBACK;
                    break;
                case 7:
                    INST_NAME("SAR Ew, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    if(box86_dynarec_safeflags>1)
                        MAYSETFLAGS();
                    UFLAG_IF {
                        ANDS_IMM8(x2, xECX, 0x1f);
                        B_NEXT(cEQ);
                    } else {
                        AND_IMM8(x2, xECX, 0x1f);
                    }
                    GETSEW(x1);
                    emit_sar16(dyn, ninst, x1, x2, x2, x14);
                    EWBACK;
                    break;
            }
            break;

        case 0xF0:
            return dynarec66F0(dyn, addr, ip, ninst, ok, need_epilog);
            break;

        case 0xF2:
        case 0xF3:
            nextop = F8;
            switch(nextop) {
                case 0xA4:
                    INST_NAME("REP MOVSB");
                    TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                    B_NEXT(cEQ);    // end of loop
                    GETDIR(x3,1);
                    MARK;
                    LDRBAI_REG_LSL_IMM5(x1, xESI, x3, 0);
                    STRBAI_REG_LSL_IMM5(x1, xEDI, x3, 0);
                    SUBS_IMM8(xECX, xECX, 1);
                    B_MARK(cNE);
                    // done
                    break;
                case 0xA5:
                    INST_NAME("REP MOVSW");
                    TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                    B_NEXT(cEQ);    // end of loop
                    GETDIR(x3, 2);
                    MARK;
                    LDRHAI_REG_LSL_IMM5(x1, xESI, x3);
                    STRHAI_REG_LSL_IMM5(x1, xEDI, x3);
                    SUBS_IMM8(xECX, xECX, 1);
                    B_MARK(cNE);
                    break;

                case 0xA7:
                    if(opcode==0xF2) {INST_NAME("REPNZ CMPSW");} else {INST_NAME("REPZ CMPSW");}
                    MAYSETFLAGS();
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                    B_NEXT(cEQ);    // end of loop
                    GETDIR(x3, 2);
                    MARK;
                    LDRBAI_REG_LSL_IMM5(x1, xESI, x3, 0);
                    LDRBAI_REG_LSL_IMM5(x2, xEDI, x3, 0);
                    CMPS_REG_LSL_IMM5(x1, x2, 0);
                    if(opcode==0xF2) {
                        B_MARK2(cEQ);
                    } else {
                        B_MARK2(cNE);
                    }
                    SUBS_IMM8(xECX, xECX, 1);
                    B_MARK(cNE);
                    B_MARK3(c__);
                    // done, finish with cmp test
                    MARK2;
                    SUB_IMM8(xECX, xECX, 1);
                    MARK3;
                    emit_cmp16(dyn, ninst, x1, x2, x3, x14);
                    break;

                case 0xAB:
                    INST_NAME("REP STOSW");
                    TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                    B_NEXT(cEQ);    // end of loop
                    GETDIR(x3, 2);
                    MARK;
                    STRHAI_REG_LSL_IMM5(xEAX, xEDI, x3);
                    SUBS_IMM8(xECX, xECX, 1);
                    B_MARK(cNE);
                    break;

                case 0xAF:
                    if(opcode==0xF2) {INST_NAME("REPNZ SCASW");} else {INST_NAME("REPZ SCASW");}
                    MAYSETFLAGS();
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    TSTS_REG_LSL_IMM5(xECX, xECX, 0);
                    B_NEXT(cEQ);    // end of loop
                    GETDIR(x3, 2);
                    UXTH(x1, xEAX, 0); // get lower 16bits
                    MARK;
                    LDRHAI_REG_LSL_IMM5(x2, xEDI, x3);
                    CMPS_REG_LSL_IMM5(x1, x2, 0);
                    if(opcode==0xF2) {
                        B_MARK2(cEQ);
                    } else {
                        B_MARK2(cNE);
                    }
                    SUBS_IMM8(xECX, xECX, 1);
                    B_MARK(cNE);
                    B_MARK3(c__);
                    // done, finish with cmp test
                    MARK2;
                    SUB_IMM8(xECX, xECX, 1);
                    MARK3;
                    emit_cmp16(dyn, ninst, x1, x2, x3, x14);
                    break;
                default:
                    DEFAULT;
            }
            break;

        case 0xF5:
            INST_NAME("CMC");
            READFLAGS(X_CF);
            SETFLAGS(X_CF, SF_SUBSET);
            XOR_IMM8(xFlags, xFlags, 1<<F_CF);
            break;

        case 0xF7:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    INST_NAME("TEST Ew, Iw");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1);
                    u16 = F16;
                    MOVW(x2, u16);
                    emit_test16(dyn, ninst, x1, x2, x3, x14);
                    break;
                case 2:
                    INST_NAME("NOT Ew");
                    GETEW(x1);
                    MVN_REG_LSL_IMM5(ed, ed, 0);
                    EWBACK;
                    break;
                case 3:
                    INST_NAME("NEG Ew");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEW(x1);
                    emit_neg16(dyn, ninst, ed, x2, x14);
                    EWBACK;
                    break;
                case 4:
                    INST_NAME("MUL AX, Ew");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEW(x1);
                    STM(xEmu, (1<<xEAX) | (1<<xECX) | (1<<xEDX));
                    CALL(mul16, -1, 0);
                    LDM(xEmu, (1<<xEAX) | (1<<xECX) | (1<<xEDX));
                    break;
                case 5:
                    INST_NAME("IMUL AX, Ew");
                    SETFLAGS(X_ALL, SF_PENDING);
                    GETEW(x1);
                    SMULBB(x2, xEAX, ed);
                    UFLAG_RES(x2);
                    BFI(xEAX, x2, 0, 16);
                    MOV_REG_LSR_IMM5(x2, x2, 16);
                    BFI(xEDX, x2, 0, 16);
                    UFLAG_DF(x1, d_imul16);
                    break;
                case 6:
                    INST_NAME("DIV Ew");
                    if(arm_div) {
                        GETEW(x1);
                        SETFLAGS(X_ALL, SF_SET_NODF);
                        SET_DFNONE(x2);
                        UXTH(x2, xEAX, 0);
                        ORR_REG_LSL_IMM5(x2, x2, xEDX, 16);
                        UDIV(x3, x2, ed);
                        MLS(x14, x3, ed, x2);  // x14 = x2 mod ed (i.e. x2 - x3*ed)
                        BFI(xEAX, x3, 0, 16);
                        BFI(xEDX, x14, 0, 16);
                    } else {
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        SETFLAGS(X_ALL, SF_SET_DF);
                        GETEW(x1);
                        STM(xEmu, (1<<xEAX) | (1<<xECX) | (1<<xEDX));
                        CALL(div16, -1, 0);
                        LDM(xEmu, (1<<xEAX) | (1<<xECX) | (1<<xEDX));
                    }
                    break;
                case 7:
                    INST_NAME("IDIV Ew");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    SETFLAGS(X_ALL, SF_SET_DF);
                    GETEW(x1);
                    STM(xEmu, (1<<xEAX) | (1<<xECX) | (1<<xEDX));
                    CALL(idiv16, -1, 0);
                    LDM(xEmu, (1<<xEAX) | (1<<xECX) | (1<<xEDX));
                    break;
            }
            break;
        case 0xF8:
            INST_NAME("CLC");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE(x1);
            BFC(xFlags, F_CF, 1);
            break;
        case 0xF9:
            INST_NAME("STC");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE(x1);
            MOVW(x1, 1);
            BFI(xFlags, x1, F_CF, 1);
            break;

        case 0xFF:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("INC Ew");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    GETEW(x1);
                    emit_inc16(dyn, ninst, x1, x2, x14);
                    EWBACK;
                    break;
                case 1:
                    INST_NAME("DEC Ew");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    GETEW(x1);
                    emit_dec16(dyn, ninst, x1, x2, x14);
                    EWBACK;
                    break;
                case 6:
                    INST_NAME("PUSH Ew");
                    GETEW(x1);
                    PUSH16(x1, xESP);
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

