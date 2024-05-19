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

uintptr_t dynarecGS(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop;
    int32_t i32, j32;
    uint32_t u32;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2, wb, eb1, eb2;
    uint8_t u8;
    int fixedaddress;

    MAYUSE(j32);
    MAYUSE(wback);
    MAYUSE(wb2);
    MAYUSE(wb);
    MAYUSE(eb1);
    MAYUSE(eb2);
    
    switch(opcode) {

        case 0x01:
            INST_NAME("ADD GS:Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_tlsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO2(x14);
            emit_add32(dyn, ninst, ed, gd, x3, x14);
            WBACK2;
            break;

        case 0x03:
            INST_NAME("ADD Gd, GS:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_tlsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO(x14);
            emit_add32(dyn, ninst, gd, ed, x3, x14);
            break;

        case 0x0F:  // more opcodes
            opcode = F8;
            switch(opcode) {
                
                case 0xAF:
                    INST_NAME("IMUL Gd, Ed");
                    SETFLAGS(X_ALL, SF_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x14);
                    nextop = F8;
                    GETGD;
                    GETEDO(x14);
                    UFLAG_IF {
                        SMULL(x3, gd, gd, ed);
                        UFLAG_OP1(x3);
                        UFLAG_RES(gd);
                        UFLAG_DF(x3, d_imul32);
                    } else {
                        MUL(gd, gd, ed);
                    }
                    break;

                default:
                    DEFAULT;
            }
            break;

        case 0x21:
            INST_NAME("AND Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_tlsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO2(x14);
            emit_and32(dyn, ninst, ed, gd, x3, x14);
            WBACK2;
            break;

        case 0x23:
            INST_NAME("AND Gd, GS:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_tlsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO(x14);
            emit_and32(dyn, ninst, gd, ed, x3, x14);
            break;

        case 0x29:
            INST_NAME("SUB GS:Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_tlsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO2(x14);
            emit_sub32(dyn, ninst, ed, gd, x3, x14);
            WBACK2;
            break;

        case 0x2B:
            INST_NAME("SUB Gd, GS:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_tlsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO(x14);
            emit_sub32(dyn, ninst, gd, ed, x3, x14);
            break;

        case 0x31:
            INST_NAME("XOR GS:Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_tlsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO2(x14);
            emit_xor32(dyn, ninst, gd, ed, x3, x14);
            WBACK2;
            break;

        case 0x33:
            INST_NAME("XOR Gd, GS:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_tlsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO(x14);
            emit_xor32(dyn, ninst, gd, ed, x3, x14);
            break;

        case 0x39:
            INST_NAME("CMP GS:Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_tlsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO(x14);
            emit_cmp32(dyn, ninst, ed, gd, x3, x14);
            break;

        case 0x3B:
            INST_NAME("CMP Gd, GS:Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            grab_tlsdata(dyn, addr, ninst, x14);
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

        case 0x69:
            INST_NAME("IMUL Gd, GS:Ed, Id");
            SETFLAGS(X_ALL, SF_PENDING);
            grab_tlsdata(dyn, addr, ninst, x14);
            nextop = F8;
            GETGD;
            GETEDO(x14);
            i32 = F32S;
            MOV32(x14, i32);
            UFLAG_IF {
                SMULL(x3, gd, x14, ed);
                UFLAG_OP1(x3);
                UFLAG_RES(gd);
                UFLAG_DF(x3, d_imul32);
            } else {
                MUL(gd, ed, x14);
            }
            break;

        case 0x80:
        case 0x82:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: //ADD
                    INST_NAME("ADD GS:Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x1);
                    GETEBO(x1);
                    u8 = F8;
                    emit_add8c(dyn, ninst, x1, u8, x2, x14);
                    EBBACK2;
                    break;
                case 1: //OR
                    INST_NAME("OR GS:Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x1);
                    GETEBO(x1);
                    u8 = F8;
                    emit_or8c(dyn, ninst, x1, u8, x2, x14);
                    EBBACK2;
                    break;
                case 2: //ADC
                    INST_NAME("ADC GS:Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x1);
                    GETEBO(x1);
                    u8 = F8;
                    emit_adc8c(dyn, ninst, x1, u8, x2, x14);
                    EBBACK2;
                    break;
                case 3: //SBB
                    INST_NAME("SBB GS:Eb, Ib");
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x1);
                    GETEBO(x1);
                    u8 = F8;
                    emit_sbb8c(dyn, ninst, x1, u8, x2, x14);
                    EBBACK2;
                    break;
                case 4: //AND
                    INST_NAME("AND GS:Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x1);
                    GETEBO(x1);
                    u8 = F8;
                    emit_and8c(dyn, ninst, x1, u8, x2, x14);
                    EBBACK2;
                    break;
                case 5: //SUB
                    INST_NAME("SUB GS:Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x1);
                    GETEBO(x1);
                    u8 = F8;
                    emit_sub8c(dyn, ninst, x1, u8, x2, x14);
                    EBBACK2;
                    break;
                case 6: //XOR
                    INST_NAME("XOR GS:Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x1);
                    GETEBO(x1);
                    u8 = F8;
                    emit_xor8c(dyn, ninst, x1, u8, x2, x14);
                    EBBACK2;
                    break;
                case 7: //CMP
                    INST_NAME("CMP GS:Eb, Ib");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x1);
                    GETEBO(x1);
                    u8 = F8;
                    if(u8) {
                        MOVW(x2, u8);
                        emit_cmp8(dyn, ninst, x1, x2, x3, x14);
                    } else {
                        emit_cmp8_0(dyn, ninst, x1, x3, x14);
                    }
                    break;
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
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x14);
                    GETEDO2(x14);
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    emit_add32c(dyn, ninst, ed, i32, x3, x14);
                    WBACK2;
                    break;
                case 1: //OR
                    if(opcode==0x81) {INST_NAME("OR Ed, Id");} else {INST_NAME("OR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x14);
                    GETEDO2(x14);
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    emit_or32c(dyn, ninst, ed, i32, x3, x14);
                    WBACK2;
                    break;
                case 2: //ADC
                    if(opcode==0x81) {INST_NAME("ADC Ed, Id");} else {INST_NAME("ADC Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x14);
                    GETEDO2(x14);
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    emit_adc32c(dyn, ninst, ed, i32, x3, x14);
                    WBACK2;
                    break;
                case 3: //SBB
                    if(opcode==0x81) {INST_NAME("SBB Ed, Id");} else {INST_NAME("SBB Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x14);
                    GETEDO2(x14);
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    emit_sbb32c(dyn, ninst, ed, i32, x3, x14);
                    WBACK2;
                    break;
                case 4: //AND
                    if(opcode==0x81) {INST_NAME("AND Ed, Id");} else {INST_NAME("AND Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x14);
                    GETEDO2(x14);
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    emit_and32c(dyn, ninst, ed, i32, x3, x14);
                    WBACK2;
                    break;
                case 5: //SUB
                    if(opcode==0x81) {INST_NAME("SUB Ed, Id");} else {INST_NAME("SUB Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x14);
                    GETEDO2(x14);
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    emit_sub32c(dyn, ninst, ed, i32, x3, x14);
                    WBACK2;
                    break;
                case 6: //XOR
                    if(opcode==0x81) {INST_NAME("XOR Ed, Id");} else {INST_NAME("XOR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x14);
                    GETEDO2(x14);
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    emit_xor32c(dyn, ninst, ed, i32, x3, x14);
                    WBACK2;
                    break;
                case 7: //CMP
                    if(opcode==0x81) {INST_NAME("CMP Ed, Id");} else {INST_NAME("CMP Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    grab_tlsdata(dyn, addr, ninst, x14);
                    GETEDO2(x14);
                    if(opcode==0x81) i32 = F32S; else i32 = F8S;
                    if(i32) {
                        MOV32(x2, i32);
                        emit_cmp32(dyn, ninst, ed, x2, x3, x14);
                    } else {
                        emit_cmp32_0(dyn, ninst, ed, x3, x14);
                    }
                    break;
            }
            break;

        case 0x89:
            INST_NAME("MOV GS:Ed, Gd");
            grab_tlsdata(dyn, addr, ninst, x14);
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
            INST_NAME("MOV Gd, GS:Ed");
            grab_tlsdata(dyn, addr, ninst, x14);
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
            INST_NAME("POP GS:Ed");
            grab_tlsdata(dyn, addr, ninst, x14);
            nextop = F8;
            if(MODREG) {
                POP1((xEAX+(nextop&7)));
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

        case 0xA1:
            INST_NAME("MOV EAX, GS:Id");
            grab_tlsdata(dyn, addr, ninst, x1);
            i32 = F32S;
            SMREAD();
            if(i32>-4096 && i32<4096) {
                LDR_IMM9(xEAX, x1, i32);
            } else {
                MOV32(x2, i32);
                ADD_REG_LSL_IMM5(x1, x1, x2, 0);
                LDR_IMM9(xEAX, x1, 0);
            }
            break;

        case 0xA2:
            INST_NAME("MOV GS:Od, AL");
            grab_tlsdata(dyn, addr, ninst, x1);
            u32 = F32;
            MOV32(x2, u32);
            ADD_REG_LSL_IMM5(x2, x1, x2, 0);
            STRB_IMM9(xEAX, x2, 0);
            SMWRITE2();
            break;

        case 0xA3:
            INST_NAME("MOV GS:Od, EAX");
            grab_tlsdata(dyn, addr, ninst, x1);
            u32 = F32;
            MOV32(x2, u32);
            ADD_REG_LSL_IMM5(x2, x1, x2, 0);
            STR_IMM9(xEAX, x2, 0);
            SMWRITE2();
            break;

        case 0xC6:
            INST_NAME("MOV GS:Eb, Ib");
            grab_tlsdata(dyn, addr, ninst, x14);
            nextop=F8;
            if(MODREG) {   // reg <= u8
                u8 = F8;
                ed = (nextop&7);
                eb1 = xEAX+(ed&3);  // Ax, Cx, Dx or Bx
                eb2 = (ed&4)>>2;    // L or H
                MOVW(x3, u8);
                BFI(eb1, x3, eb2*8, 8);
            } else {                    // mem <= u8
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                u8 = F8;
                MOVW(x3, u8);
                STRB_REG_LSL_IMM5(x3, ed, x14, 0);
                SMWRITE2();
            }
            break;
        case 0xC7:
            INST_NAME("MOV GS:Ed, Id");
            grab_tlsdata(dyn, addr, ninst, x14);
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

        case 0xD1:
            nextop = F8;
            grab_tlsdata(dyn, addr, ninst, x14);
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETEDO2(x14);
                    emit_rol32c(dyn, ninst, ed, 1, x3, x14);
                    WBACK2;
                    break;
                case 1:
                    INST_NAME("ROR Ed, 1");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET_PENDING);
                    GETEDO2(x14);
                    emit_ror32c(dyn, ninst, ed, 1, x3, x14);
                    WBACK2;
                    break;
                case 2:
                    INST_NAME("RCL Ed, 1");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    MOVW(x2, 1);
                    GETEDO2(x14);
                    if(ed!=x1) {MOV_REG(x1, ed); wb = x1;}
                    CALL_(rcl32, ed, (1<<x2));
                    WBACK2;
                    break;
                case 3:
                    INST_NAME("RCR Ed, 1");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    MOVW(x2, 1);
                    if(ed!=x1) {MOV_REG(x1, ed); wb = x1;}
                    CALL_(rcr32, ed, (1<<x2));
                    WBACK2;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETEDO2(x14);
                    emit_shl32c(dyn, ninst, ed, 1, x3, x14);
                    WBACK2;
                    break;
                case 5:
                    INST_NAME("SHR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETEDO2(x14);
                    emit_shr32c(dyn, ninst, ed, 1, x3, x14);
                    WBACK2;
                    break;
                case 7:
                    INST_NAME("SAR Ed, 1");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    GETEDO2(x14);
                    emit_sar32c(dyn, ninst, ed, 1, x3, x14);
                    WBACK2;
                    break;
            }
            break;
        case 0xD3:
            nextop = F8;
            grab_tlsdata(dyn, addr, ninst, x14);
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("ROL Ed, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    AND_IMM8(x3, xECX, 0x1f);
                    TSTS_REG_LSL_IMM5(x3, x3, 0);
                    B_MARK2(cEQ);
                    RSB_IMM8(x3, x3, 0x20);
                    GETEDO2(x14);
                    MOV_REG_ROR_REG(ed, ed, x3);
                    WBACK2;
                    UFLAG_IF {  // calculate flags directly
                        CMPS_IMM8(x3, 31);
                        B_MARK(cNE);
                            MOV_REG_LSR_IMM5(x1, ed, 31);
                            ADD_REG_LSL_IMM5(x1, x1, ed, 0);
                            BFI(xFlags, x1, F_OF, 1);
                        MARK;
                        BFI(xFlags, ed, F_CF, 1);
                        UFLAG_DF(x2, d_none);
                    }
                    MARK2;
                    break;
                case 1:
                    INST_NAME("ROR Ed, CL");
                    SETFLAGS(X_OF|X_CF, SF_SUBSET);
                    AND_IMM8(x3, xECX, 0x1f);
                    TSTS_REG_LSL_IMM5(x3, x3, 0);
                    B_MARK2(cEQ);
                    GETEDO2(x14);
                    MOV_REG_ROR_REG(ed, ed, x3);
                    WBACK2;
                    UFLAG_IF {  // calculate flags directly
                        CMPS_IMM8(x3, 1);
                        B_MARK(cNE);
                            MOV_REG_LSR_IMM5(x2, ed, 30); // x2 = d>>30
                            XOR_REG_LSR_IMM8(x2, x2, x2, 1); // x2 = ((d>>30) ^ ((d>>30)>>1))
                            BFI(xFlags, x2, F_OF, 1);
                        MARK;
                        MOV_REG_LSR_IMM5(x2, ed, 31);
                        BFI(xFlags, x2, F_CF, 1);
                        UFLAG_DF(x2, d_none);
                    }
                    MARK2;
                    break;
                case 2:
                    INST_NAME("RCL Ed, CL");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    AND_IMM8(x2, xECX, 0x1f);
                    if(ed!=x1) {MOV_REG(x1, ed); wb = x1;}
                    CALL_(rcl32, ed, (1<<x2));
                    WBACK2;
                    break;
                case 3:
                    INST_NAME("RCR Ed, CL");
                    MESSAGE(LOG_DUMP, "Need Optimization\n");
                    READFLAGS(X_CF);
                    SETFLAGS(X_OF|X_CF, SF_SET_DF);
                    AND_IMM8(x2, xECX, 0x1f);
                    if(ed!=x1) {MOV_REG(x1, ed); wb = x1;}
                    CALL_(rcr32, ed, (1<<x14));
                    WBACK2;
                    break;
                case 4:
                case 6:
                    INST_NAME("SHL Ed, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    AND_IMM8(x3, xECX, 0x1f);
                    GETEDO2(x14);
                    emit_shl32(dyn, ninst, ed, x3, x3, x14);
                    WBACK2;
                    break;
                case 5:
                    INST_NAME("SHR Ed, CL");
                    SETFLAGS(X_ALL, SF_SET_PENDING);    // some flags are left undefined
                    AND_IMM8(x3, xECX, 0x1f);
                    GETEDO2(x14);
                    emit_shr32(dyn, ninst, ed, x3, x3, x14);
                    WBACK2;
                    break;
                case 7:
                    INST_NAME("SAR Ed, CL");
                    SETFLAGS(X_ALL, SF_PENDING);
                    AND_IMM8(x3, xECX, 0x1f);
                    GETEDO2(x14);
                    UFLAG_OP12(ed, x3);
                    MOV_REG_ASR_REG(ed, ed, x3);
                    WBACK2;
                    UFLAG_RES(ed);
                    UFLAG_DF(x3, d_sar32);
                    break;
            }
            break;

        case 0xE9:
        case 0xEB:
            INST_NAME("(ignored) GS:");
            --addr;
            break;

        case 0xFF:
            nextop = F8;
            grab_tlsdata(dyn, addr, ninst, x14);
            switch((nextop>>3)&7) {
                case 6: // Push Ed
                    INST_NAME("PUSH GS:Ed");
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

