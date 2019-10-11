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

/* setup r2 to address pointed by */
uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint) 
{
    uint8_t ret = 2;
    uint8_t scratch = 2;
    if(hint>0) ret = hint;
    if(hint>0 && hint<xEAX) scratch = hint;
    if(hint==xEIP) scratch = hint;  // allow this one as a scratch and return register
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = (sib>>3)&7;
            if((sib&0x7)==5) {
                uint32_t tmp = F32;
                MOV32(((sib_reg!=4)?scratch:ret), tmp);
                if (sib_reg!=4) {
                    if(tmp) {
                        MOV32(scratch, tmp);
                        ADD_REG_LSL_IMM8(ret, scratch, xEAX+sib_reg, (sib>>6));
                    } else {
                        MOV_REG_LSL_IMM5(ret, xEAX+sib_reg, (sib>>6));
                    }
                } else {
                    MOV32(ret, tmp);
                }
            } else {
                if (sib_reg!=4) {
                    ADD_REG_LSL_IMM8(ret, xEAX+(sib&0x7), xEAX+sib_reg, (sib>>6));
                } else {
                    ret = xEAX+(sib&0x7);
                }
            }
        } else if((nextop&7)==5) {
            uint32_t tmp = F32;
            MOV32(ret, tmp);
        } else {
            ret = xEAX+(nextop&7);
        }
    } else {
        int i32;
        uint8_t sib = 0;
        int sib_reg = 0;
        if((nextop&7)==4) {
            sib = F8;
            sib_reg = (sib>>3)&7;
        }
        if(nextop&0x80)
            i32 = F32S;
        else 
            i32 = F8S;
        if(i32==0) {
            if((nextop&7)==4) {
                if (sib_reg!=4) {
                    ADD_REG_LSL_IMM8(ret, xEAX+(sib&0x07), xEAX+sib_reg, (sib>>6));
                } else {
                    ret = xEAX+(sib&0x07);
                }
            } else
                ret = xEAX+(nextop&0x07);
        } else {
            int sub = (i32<0)?1:0;
            if(sub) i32 = -i32;
            if(i32<256) {
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        ADD_REG_LSL_IMM8(scratch, xEAX+(sib&0x07), xEAX+sib_reg, (sib>>6));
                    } else {
                        scratch = xEAX+(sib&0x07);
                    }
                } else
                    scratch = xEAX+(nextop&0x07);
                if(sub) {
                    SUB_IMM8(ret, scratch, i32);
                } else {
                    ADD_IMM8(ret, scratch, i32);
                }
            } else {
                MOV32(scratch, i32);
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        if(sub) {
                            SUB_REG_LSL_IMM8(scratch, xEAX+(sib&0x07), scratch ,0);
                        } else {
                            ADD_REG_LSL_IMM8(scratch, scratch, xEAX+(sib&0x07), 0);
                        }
                        ADD_REG_LSL_IMM8(ret, scratch, xEAX+sib_reg, (sib>>6));
                    } else {
                        int tmp = xEAX+(sib&0x07);
                        if(sub) {
                            SUB_REG_LSL_IMM8(ret, tmp, scratch, 0);
                        } else {
                            ADD_REG_LSL_IMM8(ret, tmp, scratch, 0);
                        }
                    }
                } else {
                    int tmp = xEAX+(nextop&0x07);
                    if(sub) {
                        SUB_REG_LSL_IMM8(ret, tmp, scratch, 0);
                    } else {
                        ADD_REG_LSL_IMM8(ret, tmp, scratch, 0);
                    }
                }
            }
        }
    }
    *ed = ret;
    return addr;
}

// Do the GETED, but don't emit anything...
uintptr_t fakeed(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop) 
{
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = (sib>>3)&7;
            if((sib&0x7)==5) {
                addr+=4;
            }
        } else if((nextop&7)==5) {
            addr+=4;
        }
    } else {
        if((nextop&7)==4) {
            ++addr;
        }
        if(nextop&0x80) {
            addr+=4;
        } else {
            ++addr;
        }
    }
    return addr;
}

void jump_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MESSAGE(LOG_DUMP, "Jump to epilog\n");
    if(reg) {
        if(reg!=xEIP) {
            MOV_REG(xEIP, reg);
        }
    } else {
        MOV32(xEIP, ip);
    }
    void* epilog = arm_epilog;
    MOV32(2, (uintptr_t)epilog);
    BX(2);
}

void jump_to_linker(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MESSAGE(LOG_DUMP, "Jump to linker (#%d)\n", dyn->tablei);
    if(reg) {
        if(reg!=xEIP) {
            MOV_REG(xEIP, reg);
        }
    } else {
        MOV32(xEIP, ip);
    }
    uintptr_t* table = 0;
    if(dyn->tablesz) {
        table = &dyn->table[dyn->tablei];
        *table = (uintptr_t)arm_linker;
    }
    ++dyn->tablei;
    MOV32_(x1, (uintptr_t)table);
    LDR_IMM9(x2, x1, 0);
    BX(x2);
}

void ret_to_epilog(dynarec_arm_t* dyn, int ninst)
{
    MESSAGE(LOG_DUMP, "Ret epilog\n");
    POP(xESP, 1<<xEIP);
    void* epilog = arm_epilog;
    MOV32(x2, (uintptr_t)epilog);
    BX(x2);
}

void retn_to_epilog(dynarec_arm_t* dyn, int ninst, int n)
{
    MESSAGE(LOG_DUMP, "Retn epilog\n");
    POP(xESP, 1<<xEIP);
    ADD_IMM8(xESP, xESP, n);
    void* epilog = arm_epilog;
    MOV32(x2, (uintptr_t)epilog);
    BX(x2);
}

void call_c(dynarec_arm_t* dyn, int ninst, void* fnc, int reg, int ret, uint32_t mask)
{
    MOV32(reg, (uintptr_t)fnc);
    PUSH(xSP, (1<<xEmu) | mask);
    BLX(reg);
    if(ret>=0) {
        MOV_REG(ret, 0);
    }
    POP(xSP, (1<<xEmu) | mask);
}

void grab_tlsdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg)
{
    MESSAGE(LOG_DUMP, "Get TLSData\n");
    call_c(dyn, ninst, GetGSBaseEmu, 12, reg, 0);
}

int isNativeCall(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t* calladdress, int* retn)
{
    if(PK(0)==0xff && PK(1)==0x25) {  // absolute jump, maybe the GOT
        uintptr_t a1 = (PK32(2));   // need to add a check to see if the address is from the GOT !
        addr = *(uint32_t*)a1; 
    }
    if(PK(0)==0xCC && PK(1)=='S' && PK(2)=='C' && PK32(3)!=0) {
        // found !
        if(retn) *retn = (PK(3+4+4+1)==0xc2)?(PK16(3+4+4+2)):0;
        if(calladdress) *calladdress = addr+1;
        return 1;
    }
    return 0;
}
