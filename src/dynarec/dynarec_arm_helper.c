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
#include "../tools/bridge_private.h"

#include "dynarec_arm_helper.h"

/* setup r2 to address pointed by ED, also fixaddress is 1 if ED is a constant */
uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int* fixaddress)
{
    uint8_t ret = 2;
    uint8_t scratch = 2;
    *fixaddress = 0;
    if(hint>0) ret = hint;
    if(hint>0 && hint<xEAX) scratch = hint;
    if(hint==xEIP) scratch = hint;  // allow this one as a scratch and return register
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = (sib>>3)&7;
            if((sib&0x7)==5) {
                uint32_t tmp = F32;
                if (sib_reg!=4) {
                    if(tmp) {
                        MOV32(scratch, tmp);
                        ADD_REG_LSL_IMM8(ret, scratch, xEAX+sib_reg, (sib>>6));
                    } else {
                        MOV_REG_LSL_IMM5(ret, xEAX+sib_reg, (sib>>6));
                    }
                } else {
                    MOV32(ret, tmp);
                    *fixaddress = 1;
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
            *fixaddress = 1;
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
    int i32;
    if(!box86_dynarec_linker) {
        jump_to_epilog(dyn, ip, reg, ninst);
    } else {
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
            table[0] = (uintptr_t)arm_linker;
            if(!ip) {   // need the smart linker
                table[1] = ip;
            }
        }
        if(ip)
            dyn->tablei+=1; // fast linker, with a fixed address
        else
            dyn->tablei+=2; // smart linker
        MOV32_(x1, (uintptr_t)table);
        // TODO: This is not thread safe.
        if(!ip) {   // no IP, jump address in a reg, so need smart linker
            LDR_IMM9(x2, x1, 4);    // load planned IP
            CMPS_REG_LSL_IMM8(x12, x2, 0);
            i32 = GETMARK-(dyn->arm_size+8);
            Bcond(cEQ, i32);    // Ok, still going in the same place
            MOV32(x2, (uintptr_t)arm_linker);
            STM(x1, (1<<x2) | (1<<x12)); // nope, putting back linker & IP in place
            BX(x2); // go to linker
            MARK;
        }
        LDR_IMM9(x2, x1, 0);
        BX(x2); // jump
    }
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
    PUSH(xSP, (1<<xEmu) | mask);
    MOV32(reg, (uintptr_t)fnc);
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
    onebridge_t *b = (onebridge_t*)(addr);
    if(b->CC==0xCC && b->S=='S' && b->C=='C' && b->w!=(wrapper_t)0) {
        // found !
        if(retn) *retn = (b->C3==0xC2)?b->N:0;
        if(calladdress) *calladdress = addr+1;
        return 1;
    }
    return 0;
}

void x87_reset(dynarec_arm_t* dyn, int ninst)
{
    for (int i=0; i<8; ++i)
        dyn->x87cache[i] = -1;
}

int x87_do_push(dynarec_arm_t* dyn, int ninst, int scratch)
{
    // Add 1 to emu fpu_stack
    LDR_IMM9(scratch, xEmu, offsetof(x86emu_t, fpu_stack));
    ADD_IMM8(scratch, scratch, 1);  // no check...
    STR_IMM9(scratch, xEmu, offsetof(x86emu_t, fpu_stack));
    // Sub 1 to top, with and 7
    LDR_IMM9(scratch, xEmu, offsetof(x86emu_t, top));
    SUB_IMM8(scratch, scratch, 1);
    AND_IMM8(scratch, scratch, 7);
    STR_IMM9(scratch, xEmu, offsetof(x86emu_t, top));
    // move all regs in cache, and find a free one
    int ret = -1;
    for(int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1)
            ++dyn->x87cache[i];
        else if(ret==-1) {
            dyn->x87cache[i] = 0;
            ret=i;
        }
    return ret+8;
}
void x87_do_pop(dynarec_arm_t* dyn, int ninst, int scratch)
{
    // Sub 1 to emu fpu_stack
    LDR_IMM9(scratch, xEmu, offsetof(x86emu_t, fpu_stack));
    SUB_IMM8(scratch, scratch, 1);  // no check...
    STR_IMM9(scratch, xEmu, offsetof(x86emu_t, fpu_stack));
    // Add 1 to top, with and 7
    LDR_IMM9(scratch, xEmu, offsetof(x86emu_t, top));
    ADD_IMM8(scratch, scratch, 1);
    AND_IMM8(scratch, scratch, 7);
    STR_IMM9(scratch, xEmu, offsetof(x86emu_t, top));
    // move all regs in cache, poping ST0
    int ret = -1;
    for(int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1)
            --dyn->x87cache[i];
}

void x87_purgecache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    int ret = 0;
    for (int i=0; i<8 && !ret; ++i)
        if(dyn->x87cache[i] != -1)
            ret = 1;
    if(!ret)    // nothing to do
        return;
    // prepare offset to fpu => s1
    MOVW(s1, offsetof(x86emu_t, fpu));
    ADD_REG_LSL_IMM8(s1, xEmu, s1, 0);
    // Get top
    LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    // loop all cache entries
    for (int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1) {
            ADD_IMM8(s3, s2, i);
            AND_IMM8(s3, s3, 7);    // (emu->top + i)&7
            ADD_REG_LSL_IMM8(s3, s1, s3, 3);    // fpu[(emu->top+i)&7] lsl 3 because fpu are double, so 8 bytes
            VSTR_64(i+8, s3, 0);    // save the value
            dyn->x87cache[i] = -1;
        }
}

#ifdef HAVE_TRACE
void x87_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    int ret = 0;
    for (int i=0; i<8 && !ret; ++i)
        if(dyn->x87cache[i] != -1)
            ret = 1;
    if(!ret)    // nothing to do
        return;
    // prepare offset to fpu => s1
    MOVW(s1, offsetof(x86emu_t, fpu));
    ADD_REG_LSL_IMM8(s1, xEmu, s1, 0);
    // Get top
    LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    // loop all cache entries
    for (int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1) {
            ADD_IMM8(s3, s2, i);
            AND_IMM8(s3, s3, 7);    // (emu->top + i)&7
            ADD_REG_LSL_IMM8(s3, s1, s3, 3);    // fpu[(emu->top+i)&7] lsl 3 because fpu are double, so 8 bytes
            VSTR_64(i+8, s3, 0);    // save the value
        }
}
#endif