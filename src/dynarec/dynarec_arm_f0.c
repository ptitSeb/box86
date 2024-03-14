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


uintptr_t dynarecF0(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
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
        case 0x00:
            INST_NAME("LOCK ADD Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGB(x2);
            if(MODREG) {
                wback = (nextop&7);    
                wb2 = (wback>>2);      
                wback = xEAX+(wback&3);
                UXTB(x1, wback, wb2);   
                emit_add8(dyn, ninst, x1, x2, x14, x3, 0);
                BFI(wback, ed, wb2*8, 8);
            } else {                   
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                MARKLOCK;
                LDREXB(x1, wback);
                emit_add8(dyn, ninst, x1, x2, x14, x3, (wback==x3)?1:0);
                STREXB(x14, x1, wback);
                CMPS_IMM8(x14, 0);
                B_MARKLOCK(cNE);    // write failed, try again
            }
            SMDMB();
            break;
        case 0x01:
            INST_NAME("LOCK ADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xEAX+(nextop&7);
                emit_add32(dyn, ninst, ed, gd, x3, x14);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                MARKLOCK;
                LDREX(x1, wback);
                emit_add32(dyn, ninst, x1, gd, x3, x14);
                STREX(x3, x1, wback);
                CMPS_IMM8(x3, 0);
                B_MARKLOCK(cNE);
            }
            SMDMB();
            break;
        case 0x08:
            INST_NAME("LOCK OR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            if(MODREG) {
                GETGB(x2);
                wback = (nextop&7);    
                wb2 = (wback>>2);      
                wback = xEAX+(wback&3);
                UXTB(x1, wback, wb2);   
                emit_or8(dyn, ninst, x1, x2, x14, x3);
                BFI(wback, ed, wb2*8, 8);
            } else {                   
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                MARKLOCK;
                GETGB(x2);
                LDREXB(x1, wback);
                emit_or8(dyn, ninst, x1, x2, x14, x2);
                STREXB(x14, x1, wback);
                CMPS_IMM8(x14, 0);
                B_MARKLOCK(cNE);    // write failed, try again
            }
            SMDMB();
            break;
        case 0x09:
            INST_NAME("LOCK OR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xEAX+(nextop&7);
                emit_or32(dyn, ninst, ed, gd, x3, x14);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                MARKLOCK;
                LDREX(x1, wback);
                emit_or32(dyn, ninst, x1, gd, x3, x14);
                STREX(x3, x1, wback);
                CMPS_IMM8(x3, 0);
                B_MARKLOCK(cNE);
            }
            SMDMB();
            break;
        case 0x10:
            INST_NAME("LOCK ADC Eb, Gb");
            READFLAGS(F_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGB(x2);
            if(MODREG) {
                wback = (nextop&7);    
                wb2 = (wback>>2);      
                wback = xEAX+(wback&3);
                UXTB(x1, wback, wb2);   
                emit_adc8(dyn, ninst, x1, x2, x14, x3, 0);
                BFI(wback, ed, wb2*8, 8);
            } else {                   
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                MARKLOCK;
                LDREXB(x1, wback);
                emit_adc8(dyn, ninst, x1, x2, x14, x3, (wback==x3)?1:0);
                STREXB(x14, x1, wback);
                CMPS_IMM8(x14, 0);
                B_MARKLOCK(cNE);    // write failed, try again
            }
            SMDMB();
            break;
        case 0x11:
            INST_NAME("LOCK ADC Ed, Gd");
            READFLAGS(F_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xEAX+(nextop&7);
                emit_adc32(dyn, ninst, ed, gd, x3, x14);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                MARKLOCK;
                LDREX(x1, wback);
                emit_adc32(dyn, ninst, x1, gd, x3, x14);
                STREX(x3, x1, wback);
                CMPS_IMM8(x3, 0);
                B_MARKLOCK(cNE);
            }
            SMDMB();
            break;
        case 0x18:
            INST_NAME("LOCK SBB Eb, Gb");
            READFLAGS(F_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGB(x2);
            if(MODREG) {
                wback = (nextop&7);    
                wb2 = (wback>>2);      
                wback = xEAX+(wback&3);
                UXTB(x1, wback, wb2);   
                emit_sbb8(dyn, ninst, x1, x2, x14, x3, 0);
                BFI(wback, ed, wb2*8, 8);
            } else {                   
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                MARKLOCK;
                LDREXB(x1, wback);
                emit_sbb8(dyn, ninst, x1, x2, x14, x3, (wback==x3)?1:0);
                STREXB(x14, x1, wback);
                CMPS_IMM8(x14, 0);
                B_MARKLOCK(cNE);    // write failed, try again
            }
            SMDMB();
            break;
        case 0x19:
            INST_NAME("LOCK SBB Ed, Gd");
            READFLAGS(F_CF);
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xEAX+(nextop&7);
                emit_sbb32(dyn, ninst, ed, gd, x3, x14);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                MARKLOCK;
                LDREX(x1, wback);
                emit_sbb32(dyn, ninst, x1, gd, x3, x14);
                STREX(x3, x1, wback);
                CMPS_IMM8(x3, 0);
                B_MARKLOCK(cNE);
            }
            SMDMB();
            break;
        case 0x20:
            INST_NAME("LOCK AND Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            if(MODREG) {
                GETGB(x2);
                wback = (nextop&7);    
                wb2 = (wback>>2);      
                wback = xEAX+(wback&3);
                UXTB(x1, wback, wb2);   
                emit_and8(dyn, ninst, x1, x2, x14, x3);
                BFI(wback, ed, wb2*8, 8);
            } else {                   
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                MARKLOCK;
                GETGB(x2);
                LDREXB(x1, wback);
                emit_and8(dyn, ninst, x1, x2, x14, x2);
                STREXB(x14, x1, wback);
                CMPS_IMM8(x14, 0);
                B_MARKLOCK(cNE);    // write failed, try again
            }
            SMDMB();
            break;
        case 0x21:
            INST_NAME("LOCK AND Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xEAX+(nextop&7);
                emit_and32(dyn, ninst, ed, gd, x3, x14);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                MARKLOCK;
                LDREX(x1, wback);
                emit_and32(dyn, ninst, x1, gd, x3, x14);
                STREX(x3, x1, wback);
                CMPS_IMM8(x3, 0);
                B_MARKLOCK(cNE);
            }
            SMDMB();
            break;
        case 0x28:
            INST_NAME("LOCK SUB Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGB(x2);
            if(MODREG) {
                wback = (nextop&7);    
                wb2 = (wback>>2);      
                wback = xEAX+(wback&3);
                UXTB(x1, wback, wb2);   
                emit_sub8(dyn, ninst, x1, x2, x14, x3, 0);
                BFI(wback, ed, wb2*8, 8);
            } else {                   
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                MARKLOCK;
                LDREXB(x1, wback);
                emit_sub8(dyn, ninst, x1, x2, x14, x3, (wback==x3)?1:0);
                STREXB(x14, x1, wback);
                CMPS_IMM8(x14, 0);
                B_MARKLOCK(cNE);    // write failed, try again
            }
            SMDMB();
            break;
        case 0x29:
            INST_NAME("LOCK SUB Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xEAX+(nextop&7);
                emit_sub32(dyn, ninst, ed, gd, x3, x14);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                MARKLOCK;
                LDREX(x1, wback);
                emit_sub32(dyn, ninst, x1, gd, x3, x14);
                STREX(x3, x1, wback);
                CMPS_IMM8(x3, 0);
                B_MARKLOCK(cNE);
            }
            SMDMB();
            break;
        case 0x38:
            INST_NAME("LOCK XOR Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            if(MODREG) {
                GETGB(x2);
                wback = (nextop&7);    
                wb2 = (wback>>2);      
                wback = xEAX+(wback&3);
                UXTB(x1, wback, wb2);   
                emit_xor8(dyn, ninst, x1, x2, x14, x3);
                BFI(wback, ed, wb2*8, 8);
            } else {                   
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                MARKLOCK;
                GETGB(x2);
                LDREXB(x1, wback);
                emit_xor8(dyn, ninst, x1, x2, x14, x2);
                STREXB(x14, x1, wback);
                CMPS_IMM8(x14, 0);
                B_MARKLOCK(cNE);    // write failed, try again
            }
            SMDMB();
            break;
        case 0x39:
            INST_NAME("LOCK XOR Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xEAX+(nextop&7);
                emit_xor32(dyn, ninst, ed, gd, x3, x14);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                MARKLOCK;
                LDREX(x1, wback);
                emit_xor32(dyn, ninst, x1, gd, x3, x14);
                STREX(x3, x1, wback);
                CMPS_IMM8(x3, 0);
                B_MARKLOCK(cNE);
            }
            SMDMB();
            break;
        
        case 0x66:
            return dynarec66F0(dyn, addr, ip, ninst, ok, need_epilog);
            break;

        case 0x81:
        case 0x83:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0: //ADD
                    if(opcode==0x81) {
                        INST_NAME("LOCK ADD Ed, Id");
                    } else {
                        INST_NAME("LOCK ADD Ed, Ib");
                    }
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(MODREG) {
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        ed = xEAX+(nextop&7);
                        emit_add32c(dyn, ninst, ed, i32, x3, x14);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 1, LOCK_LOCK);
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        if(!fixedaddress) {
                            TSTS_IMM8(wback, 0x3);
                            B_MARK(cNE);
                        }
                        if(!fixedaddress || (fixedaddress && !(fixedaddress&3))) {
                            MARKLOCK;
                            LDREX(x1, wback);
                            emit_add32c(dyn, ninst, x1, i32, x3, x14);
                            STREX(x3, x1, wback);
                            CMPS_IMM8(x3, 0);
                            B_MARKLOCK(cNE);
                        }
                        if(!fixedaddress) {
                            SMDMB();
                            B_NEXT(c__);
                        }
                        if(!fixedaddress || (fixedaddress && (fixedaddress&3))) {
                            MARK;   // unaligned! also, not enough 
                            LDR_IMM9(x1, wback, 0);
                            LDREXB(x14, wback);
                            BFI(x1, x14, 0, 8); // re-inject
                            emit_add32c(dyn, ninst, x1, i32, x3, x14);
                            STREXB(x3, x1, wback);
                            CMPS_IMM8(x3, 0);
                            B_MARK(cNE);
                            STR_IMM9(x1, wback, 0);    // put the whole value
                        }
                        SMDMB();
                    }
                    break;
                case 1: //OR
                    if(opcode==0x81) {INST_NAME("LOCK OR Ed, Id");} else {INST_NAME("LOCK OR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(MODREG) {
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        ed = xEAX+(nextop&7);
                        emit_or32c(dyn, ninst, ed, i32, x3, x14);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        MARKLOCK;
                        LDREX(x1, wback);
                        emit_or32c(dyn, ninst, x1, i32, x3, x14);
                        STREX(x3, x1, wback);
                        CMPS_IMM8(x3, 0);
                        B_MARKLOCK(cNE);
                    }
                    SMDMB();
                    break;
                case 2: //ADC
                    if(opcode==0x81) {INST_NAME("LOCK ADC Ed, Id");} else {INST_NAME("LOCK ADC Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(MODREG) {
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        ed = xEAX+(nextop&7);
                        emit_adc32c(dyn, ninst, ed, i32, x3, x14);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        MARKLOCK;
                        LDREX(x1, wback);
                        emit_adc32c(dyn, ninst, x1, i32, x3, x14);
                        STREX(x3, x1, wback);
                        CMPS_IMM8(x3, 0);
                        B_MARKLOCK(cNE);
                    }
                    SMDMB();
                    break;
                case 3: //SBB
                    if(opcode==0x81) {INST_NAME("LOCK SBB Ed, Id");} else {INST_NAME("LOCK SBB Ed, Ib");}
                    READFLAGS(X_CF);
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(MODREG) {
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        ed = xEAX+(nextop&7);
                        emit_sbb32c(dyn, ninst, ed, i32, x3, x14);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        MARKLOCK;
                        LDREX(x1, wback);
                        emit_sbb32c(dyn, ninst, x1, i32, x3, x14);
                        STREX(x3, x1, wback);
                        CMPS_IMM8(x3, 0);
                        B_MARKLOCK(cNE);
                    }
                    SMDMB();
                    break;
                case 4: //AND
                    if(opcode==0x81) {INST_NAME("LOCK AND Ed, Id");} else {INST_NAME("LOCK AND Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(MODREG) {
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        ed = xEAX+(nextop&7);
                        emit_and32c(dyn, ninst, ed, i32, x3, x14);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        MARKLOCK;
                        LDREX(x1, wback);
                        emit_and32c(dyn, ninst, x1, i32, x3, x14);
                        STREX(x3, x1, wback);
                        CMPS_IMM8(x3, 0);
                        B_MARKLOCK(cNE);
                    }
                    SMDMB();
                    break;
                case 5: //SUB
                    if(opcode==0x81) {INST_NAME("LOCK SUB Ed, Id");} else {INST_NAME("LOCK SUB Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(MODREG) {
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        ed = xEAX+(nextop&7);
                        emit_sub32c(dyn, ninst, ed, i32, x3, x14);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 1, LOCK_LOCK);
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        if(!fixedaddress) {
                            TSTS_IMM8(wback, 0x3);
                            B_MARK(cNE);
                        }
                        if(!fixedaddress || (fixedaddress && !(fixedaddress&3))) {
                            MARKLOCK;
                            LDREX(x1, wback);
                            emit_sub32c(dyn, ninst, x1, i32, x3, x14);
                            STREX(x3, x1, wback);
                            CMPS_IMM8(x3, 0);
                            B_MARKLOCK(cNE);
                        }
                        if(!fixedaddress) {
                            SMDMB();
                            B_NEXT(c__);
                        }
                        if(!fixedaddress || (fixedaddress && (fixedaddress&3))) {
                            MARK;   // unaligned! also, not enough 
                            LDR_IMM9(x1, wback, 0);
                            LDREXB(x14, wback);
                            BFI(x1, x14, 0, 8); // re-inject
                            emit_sub32c(dyn, ninst, x1, i32, x3, x14);
                            STREXB(x3, x1, wback);
                            CMPS_IMM8(x3, 0);
                            B_MARK(cNE);
                            STR_IMM9(x1, wback, 0);    // put the whole value
                        }
                        SMDMB();
                    }
                    break;
                case 6: //XOR
                    if(opcode==0x81) {INST_NAME("LOCK XOR Ed, Id");} else {INST_NAME("LOCK XOR Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    if(MODREG) {
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        ed = xEAX+(nextop&7);
                        emit_xor32c(dyn, ninst, ed, i32, x3, x14);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        MARKLOCK;
                        LDREX(x1, wback);
                        emit_xor32c(dyn, ninst, x1, i32, x3, x14);
                        STREX(x3, x1, wback);
                        CMPS_IMM8(x3, 0);
                        B_MARKLOCK(cNE);
                        SMDMB();
                    }
                    break;
                case 7: //CMP
                    if(opcode==0x81) {INST_NAME("(LOCK) CMP Ed, Id");} else {INST_NAME("(LOCK) CMP Ed, Ib");}
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    GETEDH(x1);
                    // No need to LOCK, this is readonly
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

        // generic case, no lock needed, the value is only read (note that on x86 locked read is always followed by a locked write)
        #define GO(A)   \
        case A+0x02:    \
        case A+0x03:    \
        case A+0x04:    \
        case A+0x05:
        GO(0x00);
        GO(0x08);
        GO(0x10);
        GO(0x18);
        GO(0x20);
        GO(0x28);
        GO(0x30);
        #undef GO
        case 0x86:  // for this two, the lock is already done by the opcode, so just ignoring it
        case 0x87:
            SMDMB();
            --addr;
            break;
    
        case 0x0F:
            nextop = F8;
            switch(nextop) {
                case 0xB0:
                    INST_NAME("LOCK CMPXCHG Eb, Gb");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    nextop = F8;
                    if(MODREG) {
                        wback = (nextop&7);    
                        wb2 = (wback>>2);      
                        wback = xEAX+(wback&3);
                        UXTB(x2, wback, wb2);   
                        ed = x2;
                        wb1 = 0;
                    } else {                   
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                        MARKLOCK;
                        LDREXB(x2, wback);
                        ed = x2;
                        wb1 = 1;
                    }
                    UXTB(x1, xEAX, 0);
                    CMPS_REG_LSL_IMM5(x1, ed, 0);
                    B_MARK(cNE);
                    // AL == Eb
                    GETGB(x1);
                    if(wb1) {
                        STREXB(x14, x1, wback);
                        CMPS_IMM8(x14, 0);
                        B_MARKLOCK(cNE);    // write failed, try again
                        if(!(gb1==xEAX && gb2==0)) {
                            UXTB(x1, xEAX, 0);  // readback AL if needed
                        }
                    } else {
                        BFI(wback, x1, wb2*8, 8);
                    }
                    // done
                    B_MARK3(c__);
                    MARK;
                    // AL != Eb
                    BFI(xEAX, ed, 0, 8);
                    MARK3;
                    SMDMB();
                    // done, do the cmp now
                    UFLAG_IF {emit_cmp8(dyn, ninst, x1, x2, x3, x14);}
                    break;
                case 0xB1:
                    INST_NAME("LOCK CMPXCHG Ed, Gd");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    nextop = F8;
                    GETGD;
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                        // same as no lock, all in regs...
                        CMPS_REG_LSL_IMM5(xEAX, ed, 0);
                        B_MARK(cNE);
                        // EAX == Ed
                        MOV_REG(x3, ed);
                        MOV_REG(ed, gd);
                        emit_cmp32(dyn, ninst, xEAX, x3, x1, x14);
                        B_NEXT(c__);
                        MARK;
                        // EAX != Ed
                        emit_cmp32(dyn, ninst, xEAX, ed, x3, x14);
                        MOV_REG(xEAX, ed);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 1, LOCK_LOCK);
                        ed = x1;
                        if(!fixedaddress || (fixedaddress && (fixedaddress&3))) {
                            if(!fixedaddress) {
                                // alignment is undetermined
                                TSTS_IMM8(wback, 3);
                                B_MARK2(cNE);
                                MARKLOCK;
                                // aligned
                                LDREX(ed, wback);
                                CMPS_REG_LSL_IMM5(xEAX, ed, 0);
                                B_MARK(cNE);
                                // EAX == Ed
                                STREX(x14, gd, wback);
                                SMDMB();
                                CMPS_IMM8(x14, 0);
                                B_MARKLOCK(cNE);
                                B_MARK(c__);
                            }
                            // unaligned
                            MARK2;
                            LDR_IMM9(x1, wback, 0);
                            LDREXB( x3, wback); // dummy read, to arm the write...
                            CMPS_REG_LSL_IMM5(xEAX, ed, 0);
                            B_MARK(cNE);
                            // EAX == Ed
                            STREXB(x14, gd, wback);
                            CMPS_IMM8(x14, 0);
                            B_MARK2(cNE);
                            STR_IMM9(gd, wback, 0);
                            SMDMB();
                        } else {
                            MARKLOCK;
                            // alignment is good
                            LDREX(ed, wback);
                            CMPS_REG_LSL_IMM5(xEAX, ed, 0);
                            B_MARK(cNE);
                            // EAX == Ed
                            STREX(x14, gd, wback);
                            CMPS_IMM8(x14, 0);
                            B_MARKLOCK(cNE);
                            SMDMB();
                        }
                        // end of EAX == Ed or EAX != Ed
                        MARK;
                        UFLAG_IF { MOV_REG(x2, xEAX); }
                        MOV_REG(xEAX, ed);    
                        UFLAG_IF { emit_cmp32(dyn, ninst, x2, ed, x3, x14); }
                    }
                    break;
                case 0xB3:
                    INST_NAME("LOCK BTR Ed, Gd");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE(x1);
                    nextop = F8;
                    GETGD;
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                        MOV_REG_ASR_IMM5(x1, gd, 5);    // r1 = (gd>>5);
                        ADD_REG_LSL_IMM5(x3, wback, x1, 2); //(&ed)+=r1*4;
                        wback = x3;
                        MARKLOCK;
                        LDREX(x1, wback);
                        ed = x1;
                    }
                    AND_IMM8(x2, gd, 0x1f);
                    MOV_REG_LSR_REG(x14, ed, x2);
                    BFI(xFlags, x14, F_CF, 1);
                    MOVW(x14, 1);
                    BIC_REG_LSL_REG(ed, ed, x14, x2);
                    if(wback) {
                        STREX(x14, ed, wback);
                        CMPS_IMM8(x14, 0);
                        B_MARKLOCK(cNE);
                    }
                    SMDMB();
                    break;

                case 0xBA:
                    nextop = F8;
                    switch((nextop>>3)&7) {
                        case 4:
                            INST_NAME("(LOCK) BT Ed, Ib");
                            SETFLAGS(X_CF, SF_SUBSET);
                            SET_DFNONE(x1);
                            gd = x2;
                            if(MODREG) {
                                ed = xEAX+(nextop&7);
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, 4095-32, 0, 0, LOCK_LOCK);
                                LDR_IMM9(x1, ed, fixedaddress);
                                ed = x1;
                            }
                            u8 = F8;
                            u8&=0x1f;
                            if(u8) {
                                MOV_REG_LSR_IMM5(x1, ed, u8);
                                ed = x1;
                            }
                            BFI(xFlags, ed, F_CF, 1);
                            break;
                        case 5:
                            INST_NAME("LOCK BTS Ed, Ib");
                            SETFLAGS(X_CF, SF_SUBSET);
                            SET_DFNONE(x1);
                            if(MODREG) {
                                ed = xEAX+(nextop&7);
                                u8 = F8&0x1f;
                                wback = 0;
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                                u8 = F8&0x1f;
                                ed = x1;
                                MARKLOCK;
                                LDREX(ed, wback);
                            }
                            MOV_REG_LSR_IMM5(x14, ed, u8);
                            BFI(xFlags, x14, F_CF, 1);
                            MOVW(x14, 1);
                            BFI(ed, x14, u8, 1);
                            if(wback) {
                                STREX(x14, ed, wback);
                                CMPS_IMM8(x14, 0);
                                B_MARKLOCK(cNE);
                            }
                            MARK3;
                            break;
                        case 6:
                            INST_NAME("LOCK BTR Ed, Ib");
                            SETFLAGS(X_CF, SF_SUBSET);
                            SET_DFNONE(x1);
                            if(MODREG) {
                                ed = xEAX+(nextop&7);
                                u8 = F8&0x1f;
                                wback = 0;
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                                u8 = F8&0x1f;
                                ed = x1;
                                MARKLOCK;
                                LDREX(ed, wback);
                            }
                            MOV_REG_LSR_IMM5(x14, ed, u8);
                            BFI(xFlags, x14, F_CF, 1);
                            BFC(ed, u8, 1);
                            if(wback) {
                                STREX(x14, ed, wback);
                                CMPS_IMM8(x14, 0);
                                B_MARKLOCK(cNE);
                            }
                            MARK3;
                            break;
                        case 7:
                            INST_NAME("LOCK BTC Ed, Ib");
                            SETFLAGS(X_CF, SF_SUBSET);
                            SET_DFNONE(x1);
                            if(MODREG) {
                                ed = xEAX+(nextop&7);
                                u8 = F8&0x1f;
                                wback = 0;
                            } else {
                                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                                u8 = F8&0x1f;
                                ed = x1;
                                MARKLOCK;
                                LDREX(ed, wback);
                            }
                            MOV_REG_LSR_IMM5(x14, ed, u8);
                            BFI(xFlags, x14, F_CF, 1);
                            MOV32(x14, 1);
                            XOR_REG_LSL_IMM5(ed, ed, x14, u8);
                            if(wback) {
                                STREX(x14, ed, wback);
                                CMPS_IMM8(x14, 0);
                                B_MARKLOCK(cNE);
                            }
                            MARK3;
                            break;
                        default:
                            DEFAULT;
                    }
                    SMDMB();
                    break;
                case 0xBB:
                    INST_NAME("LOCK BTC Ed, Gd");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE(x1);
                    nextop = F8;
                    GETGD;
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, LOCK_LOCK);
                        MOV_REG_ASR_IMM5(x1, gd, 5);    // r1 = (gd>>5);
                        ADD_REG_LSL_IMM5(x3, wback, x1, 2); //(&ed)+=r1*4;
                        MARKLOCK;
                        LDREX(x1, x3);
                        ed = x1;
                        wback = x3;
                    }
                    AND_IMM8(x2, gd, 0x1f);
                    MOV_REG_LSR_REG(x14, ed, x2);
                    BFI(xFlags, x14, F_CF, 1);
                    MOVW(x14, 1);
                    XOR_REG_LSL_REG(ed, ed, x14, x2);
                    if(wback) {
                        STREX(x14, ed, wback);
                        CMPS_IMM8(x14, 0);
                        B_MARKLOCK(cNE);
                    }
                    SMDMB();
                    break;
            
                case 0xC0:
                    INST_NAME("LOCK XADD Gb, Eb");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    nextop = F8;
                    SMDMB();
                    GETGB(x1);
                    if(MODREG) {
                        wback = (nextop&7); 
                        wb2 = (wback>>2); 
                        wback = xEAX+(wback&3); 
                        UXTB(x2, wback, wb2); 
                        wb1 = 0;
                        ed = x2;
                    } else { 
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, LOCK_LOCK); 
                        MARKLOCK;
                        LDREXB(x2, wback); 
                        wb1 = 1;
                        ed = x2;
                    }
                    BFI(gb1, ed, gb2*8, 8); // gb <- eb
                    emit_add8(dyn, ninst, ed, gd, x14, x3, 1);
                    if(wb1) {
                        STREXB(x14, ed, wback);
                        CMPS_IMM8(x14, 0);
                        B_MARKLOCK(cNE);
                    } else {
                        BFI(wback, ed, wb2*8, 8);
                    }
                    SMDMB();
                    break;
                case 0xC1:
                    INST_NAME("LOCK XADD Gd, Ed");
                    SETFLAGS(X_ALL, SF_SET_PENDING);
                    nextop = F8;
                    GETGD;
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                        if(gd!=ed) {
                            XOR_REG_LSL_IMM5(gd, gd, ed, 0);    // swap gd, ed
                            XOR_REG_LSL_IMM5(ed, gd, ed, 0);
                            XOR_REG_LSL_IMM5(gd, gd, ed, 0);
                        }
                        emit_add32(dyn, ninst, ed, gd, x3, x14);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 1, LOCK_LOCK);
                        if(!fixedaddress) {
                            TSTS_IMM8(wback, 3);
                            B_MARK(cNE);    // unaligned
                        }
                        if(!fixedaddress || (fixedaddress && !(fixedaddress&3))) {
                            MARKLOCK;
                            LDREX(x1, wback);
                            ADD_REG_LSL_IMM5(x14, x1, gd, 0);
                            STREX(x3, x14, wback);
                            CMPS_IMM8(x3, 0);
                            B_MARKLOCK(cNE);
                        }
                        if(!fixedaddress) {
                            B_MARK2(c__);
                        }
                        if(!fixedaddress || (fixedaddress && (fixedaddress&3))) {
                            MARK;
                            LDR_IMM9(x1, wback, 0);
                            LDREXB(x14, wback);
                            BFI(x1, x14, 0, 8);
                            ADD_REG_LSL_IMM5(x14, x1, gd, 0);
                            STREXB(x3, x14, wback);
                            CMPS_IMM8(x3, 0);
                            B_MARK(cNE);
                            STR_IMM9(x14, wback, 0);
                        }
                        if(!fixedaddress) {
                            MARK2;
                        }
                        IFX(X_ALL|X_PEND) {
                            MOV_REG(x2, x1);
                            emit_add32(dyn, ninst, x2, gd, x3, x14);
                        }
                        MOV_REG(gd, x1);
                    }
                    SMDMB();
                    break;

                case 0xC7:
                    nextop = F8;
                    switch((nextop>>3)&7) {
                        case 1:
                            INST_NAME("LOCK CMPXCHG8B Gq, Eq");
                            SETFLAGS(X_ZF, SF_SUBSET);
                            addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, 0, 0, 1, LOCK_LOCK);
                            MARKLOCK;
                            if(!fixedaddress || (fixedaddress && (fixedaddress&7))) {
                                TSTS_IMM8(wback, 7);
                                LDREXD_COND(cEQ, x2, wback);
                                LDR_IMM9_COND(cNE, x3, wback, 4);
                                LDREX_COND(cNE, x2, wback);
                            } else {
                                LDREXD(x2, wback);
                            }
                            CMPS_REG_LSL_IMM5(xEAX, x2, 0);
                            B_MARK(cNE);    // EAX != Ed[0]
                            CMPS_REG_LSL_IMM5(xEDX, x3, 0);
                            B_MARK(cNE);    // EDX != Ed[1]
                            MOV_REG(x2, xEBX);
                            MOV_REG(x3, xECX);
                            if(!fixedaddress || (fixedaddress && (fixedaddress&7))) {
                                TSTS_IMM8(wback, 7);
                                STREXD_COND(cEQ, x14, x2, wback);
                                STREX_COND(cNE, x14, x2, wback);
                            } else {
                                STREXD(x14, x2, wback);
                            }
                            CMPS_IMM8(x14, 0);
                            B_MARKLOCK(cNE);
                            if(!fixedaddress || (fixedaddress && (fixedaddress&7))) {
                                TSTS_IMM8(wback, 7);
                                STR_IMM9_COND(cNE, x3, wback, 4);
                            }
                            MOVW(x1, 1);
                            B_MARK3(c__);
                            MARK;
                            MOV_REG(xEAX, x2);
                            MOV_REG(xEDX, x3);
                            MOVW(x1, 0);
                            MARK3;
                            SMDMB();
                            BFI(xFlags, x1, F_ZF, 1);
                            break;
                        default:
                            DEFAULT;
                    }
                    break;

                default:
                    DEFAULT;
            }
            break;

        case 0xFE:
            nextop = F8;
            SMDMB();
            switch((nextop>>3)&7)
            {
                case 0: // INC Eb
                    INST_NAME("INC Eb");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    if(MODREG) {
                        GETEB(x1);
                        emit_inc8(dyn, ninst, x1, x2, x14);
                        EBBACK;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 1, LOCK_LOCK);
                        MARKLOCK;
                        LDREXB(x1, wback);
                        emit_inc8(dyn, ninst, x1, x3, x14);
                        STREXB(x3, x1, wback);
                        CMPS_IMM8(x3, 0);
                        B_MARKLOCK(cNE);
                        SMDMB();
                    }
                    break;
                case 1: //DEC Ed
                    INST_NAME("DEC Eb");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    if(MODREG) {
                        GETEB(x1);
                        emit_dec8(dyn, ninst, x1, x2, x14);
                        EBBACK;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 1, LOCK_LOCK);
                        MARKLOCK;
                        LDREXB(x1, wback);
                        emit_dec8(dyn, ninst, x1, x3, x14);
                        STREXB(x3, x1, wback);
                        CMPS_IMM8(x3, 0);
                        B_MARKLOCK(cNE);
                        SMDMB();
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
                case 0: // INC Ed
                    INST_NAME("INC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                        emit_inc32(dyn, ninst, ed, x3, x14);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 1, LOCK_LOCK);
                        if(!fixedaddress) {
                            TSTS_IMM8(wback, 3);
                            B_MARK(cNE);    // unaligned
                        }
                        if(!fixedaddress || (fixedaddress && !(fixedaddress&3))) {
                            MARKLOCK;
                            LDREX(x1, wback);
                            emit_inc32(dyn, ninst, x1, x3, x14);
                            STREX(x3, x1, wback);
                            CMPS_IMM8(x3, 0);
                            B_MARKLOCK(cNE);
                            SMDMB();
                        }
                        if(!fixedaddress) {
                            B_NEXT(c__);
                        }
                        if(!fixedaddress || (fixedaddress && (fixedaddress&3))) {
                            MARK;
                            LDR_IMM9(x1, wback, 0);
                            LDREXB(x3, wback);
                            BFI(x1, x3, 0, 8);
                            emit_inc32(dyn, ninst, x1, x3, x14);
                            STREXB(x3, x1, wback);
                            CMPS_IMM8(x3, 0);
                            B_MARK(cNE);
                            STR_IMM9(x1, wback, 0);
                            SMDMB();
                        }
                    }
                    break;
                case 1: //DEC Ed
                    INST_NAME("DEC Ed");
                    SETFLAGS(X_ALL&~X_CF, SF_SUBSET_PENDING);
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                        emit_dec32(dyn, ninst, ed, x3, x14);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 1, LOCK_LOCK);
                        if(!fixedaddress) {
                            TSTS_IMM8(wback, 3);
                            B_MARK(cNE);    // unaligned
                        }
                        if(!fixedaddress || (fixedaddress && !(fixedaddress&3))) {
                            MARKLOCK;
                            LDREX(x1, wback);
                            emit_dec32(dyn, ninst, x1, x3, x14);
                            STREX(x3, x1, wback);
                            CMPS_IMM8(x3, 0);
                            B_MARKLOCK(cNE);
                            SMDMB();
                        }
                        if(!fixedaddress) {
                            B_NEXT(c__);
                        }
                        if(!fixedaddress || (fixedaddress && (fixedaddress&3))) {
                            MARK;
                            LDR_IMM9(x1, wback, 0);
                            LDREXB(x3, wback);
                            BFI(x1, x3, 0, 8);
                            emit_dec32(dyn, ninst, x1, x3, x14);
                            STREXB(x3, x1, wback);
                            CMPS_IMM8(x3, 0);
                            B_MARK(cNE);
                            STR_IMM9(x1, wback, 0);
                            SMDMB();
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            SMDMB();
            break;
       
        default:
            DEFAULT;
    }

    return addr;
}

