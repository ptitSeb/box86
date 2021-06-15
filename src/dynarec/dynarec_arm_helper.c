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
#include "dynablock_private.h"
#include "arm_printer.h"
#include "../tools/bridge_private.h"
#include "custommem.h"

#include "dynarec_arm_functions.h"
#include "dynarec_arm_helper.h"

/* setup r2 to address pointed by ED, also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int* fixaddress, uint32_t absmax, uint32_t mask)
{
    uint8_t ret = 2;
    uint8_t scratch = 2;
    *fixaddress = 0;
    if(hint>0) ret = hint;
    if(hint>0 && hint<xEAX) scratch = hint;
    if(hint==xEIP) scratch = hint;  // allow this one as a scratch and return register
    MAYUSE(scratch);
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = (sib>>3)&7;
            if((sib&0x7)==5) {
                uint32_t tmp = F32;
                if (sib_reg!=4) {
                    if(tmp && (abs(tmp)>absmax || (tmp&mask))) {
                        MOV32(scratch, tmp);
                        ADD_REG_LSL_IMM5(ret, scratch, xEAX+sib_reg, (sib>>6));
                    } else {
                        MOV_REG_LSL_IMM5(ret, xEAX+sib_reg, (sib>>6));
                        *fixaddress = tmp;
                    }
                } else {
                    MOV32(ret, tmp);
                }
            } else {
                if (sib_reg!=4) {
                    ADD_REG_LSL_IMM5(ret, xEAX+(sib&0x7), xEAX+sib_reg, (sib>>6));
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
        int32_t i32;
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
        if(i32==(int32_t)0x80000000) {
            // special case...
            MOV32(scratch, 0x80000000);
            if((nextop&7)==4) {
                if (sib_reg!=4) {
                    SUB_REG_LSL_IMM5(scratch, xEAX+(sib&0x07), scratch ,0);
                    ADD_REG_LSL_IMM5(ret, scratch, xEAX+sib_reg, (sib>>6));
                } else {
                    PASS3(int tmp = xEAX+(sib&0x07));
                    SUB_REG_LSL_IMM5(ret, tmp, scratch, 0);
                }
            } else {
                PASS3(int tmp = xEAX+(nextop&0x07));
                SUB_REG_LSL_IMM5(ret, tmp, scratch, 0);
            }
        } else {
            if(i32==0 || (abs(i32)<=absmax && !(i32&mask))) {
                *fixaddress = i32;
                if((nextop&7)==4) {
                    if (sib_reg!=4) {
                        ADD_REG_LSL_IMM5(ret, xEAX+(sib&0x07), xEAX+sib_reg, (sib>>6));
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
                            ADD_REG_LSL_IMM5(scratch, xEAX+(sib&0x07), xEAX+sib_reg, (sib>>6));
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
                                SUB_REG_LSL_IMM5(scratch, xEAX+(sib&0x07), scratch ,0);
                            } else {
                                ADD_REG_LSL_IMM5(scratch, scratch, xEAX+(sib&0x07), 0);
                            }
                            ADD_REG_LSL_IMM5(ret, scratch, xEAX+sib_reg, (sib>>6));
                        } else {
                            PASS3(int tmp = xEAX+(sib&0x07));
                            if(sub) {
                                SUB_REG_LSL_IMM5(ret, tmp, scratch, 0);
                            } else {
                                ADD_REG_LSL_IMM5(ret, tmp, scratch, 0);
                            }
                        }
                    } else {
                        PASS3(int tmp = xEAX+(nextop&0x07));
                        if(sub) {
                            SUB_REG_LSL_IMM5(ret, tmp, scratch, 0);
                        } else {
                            ADD_REG_LSL_IMM5(ret, tmp, scratch, 0);
                        }
                    }
                }
            }
        }
    }
    *ed = ret;
    return addr;
}

/* setup r2 to address pointed by ED, r3 as scratch also fixaddress is an optionnal delta in the range [-absmax, +absmax], with delta&mask==0 to be added to ed for LDR/STR */
uintptr_t geted16(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int* fixaddress, uint32_t absmax, uint32_t mask)
{
    uint8_t ret = x2;
    uint8_t scratch = x3;
    *fixaddress = 0;
    if(hint>0) ret = hint;
    if(scratch==ret) scratch = x2;
    MAYUSE(scratch);
    uint32_t m = nextop&0xC7;
    uint32_t n = (m>>6)&3;
    int32_t offset = 0;
    if(!n && m==6) {
        offset = F16;
        MOVW(ret, offset);
    } else {
        switch(n) {
            case 0: offset = 0; break;
            case 1: offset = F8S; break;
            case 2: offset = F16S; break;
        }
        if(offset && (abs(offset)>absmax || (offset&mask))) {
            *fixaddress = offset;
            offset = 0;
        }
        switch(m&7) {
            case 0: //R_BX + R_SI
                UBFX(ret, xEBX, 0, 16);
                UBFX(scratch, xESI, 0, 16);
                ADD_REG_LSL_IMM5(ret, ret, scratch, 0);
                break;
            case 1: //R_BX + R_DI
                UBFX(ret, xEBX, 0, 16);
                UBFX(scratch, xEDI, 0, 16);
                ADD_REG_LSL_IMM5(ret, ret, scratch, 0);
                break;
            case 2: //R_BP + R_SI
                UBFX(ret, xEBP, 0, 16);
                UBFX(scratch, xESI, 0, 16);
                ADD_REG_LSL_IMM5(ret, ret, scratch, 0);
                break;
            case 3: //R_BP + R_DI
                UBFX(ret, xEBP, 0, 16);
                UBFX(scratch, xEDI, 0, 16);
                ADD_REG_LSL_IMM5(ret, ret, scratch, 0);
                break;
            case 4: //R_SI
                UBFX(ret, xESI, 0, 16);
                break;
            case 5: //R_DI
                UBFX(ret, xEDI, 0, 16);
                break;
            case 6: //R_BP
                UBFX(ret, xEBP, 0, 16);
                break;
            case 7: //R_BX
                UBFX(ret, xEBX, 0, 16);
                break;
        }
        if(offset) {
            if(offset<0 && offset>-256) {
                SUB_IMM8(ret, ret, -offset);
            } else if(offset>0 && offset<256) {
                ADD_IMM8(ret, ret, offset);
            } else {
                MOVW(scratch, offset);
                ADD_REG_LSL_IMM5(ret, ret, scratch, 0);
            }
        }
    }

    *ed = ret;
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
        MOV32_(xEIP, ip);
    }
    PASS3(void* epilog = arm_epilog);
    MOV32_(2, (uintptr_t)epilog);
    BX(2);
}

void jump_to_next(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MESSAGE(LOG_DUMP, "Jump to next\n");

    if(reg) {
        if(reg!=xEIP) {
            MOV_REG(xEIP, reg);
        }
        MOV32(x2, getJumpTable());
        MOV_REG_LSR_IMM5(x3, xEIP, JMPTABL_SHIFT);
        LDR_REG_LSL_IMM5(x2, x2, x3, 2);    // shiftsizeof(uintptr_t)
        UBFX(x3, xEIP, 0, JMPTABL_SHIFT);
        LDR_REG_LSL_IMM5(x3, x2, x3, 2);    // shiftsizeof(uintptr_t)
    } else {
        uintptr_t p = getJumpTableAddress(ip); 
        MOV32(x2, p);
        MOV32_(xEIP, ip);
        LDR_IMM9(x3, x2, 0);
    }
    MOV_REG(x1, xEIP);
    #ifdef HAVE_TRACE
    MOV_REG(x2, 15);    // move current PC to x2, for tracing
    #endif
    BX(x3);
}

void ret_to_epilog(dynarec_arm_t* dyn, int ninst)
{
        MESSAGE(LOG_DUMP, "Ret next\n");
        POP1(xEIP);
        MOV32(x2, getJumpTable());
        MOV_REG_LSR_IMM5(x3, xEIP, JMPTABL_SHIFT);
        LDR_REG_LSL_IMM5(x2, x2, x3, 2);    // shiftsizeof(uintptr_t)
        UBFX(x3, xEIP, 0, JMPTABL_SHIFT);
        LDR_REG_LSL_IMM5(x3, x2, x3, 2);    // shiftsizeof(uintptr_t)
        MOV_REG(x1, xEIP);
        #ifdef HAVE_TRACE
        MOV_REG(x2, 15);    // move current PC to x2, for tracing
        #endif
        BX(x3);
}

void retn_to_epilog(dynarec_arm_t* dyn, int ninst, int n)
{
        MESSAGE(LOG_DUMP, "Retn epilog\n");
        POP1(xEIP);
        if(n>0xff) {
            MOVW(x1, n);
            ADD_REG_LSL_IMM5(xESP, xESP, x1, 0);
        } else {
            ADD_IMM8(xESP, xESP, n);
        }
        MOV32(x2, getJumpTable());
        MOV_REG_LSR_IMM5(x3, xEIP, JMPTABL_SHIFT);
        LDR_REG_LSL_IMM5(x2, x2, x3, 2);    // shiftsizeof(uintptr_t)
        UBFX(x3, xEIP, 0, JMPTABL_SHIFT);
        LDR_REG_LSL_IMM5(x3, x2, x3, 2);    // shiftsizeof(uintptr_t)
        MOV_REG(x1, xEIP);
        #ifdef HAVE_TRACE
        MOV_REG(x2, 15);    // move current PC to x2, for tracing
        #endif
        BX(x3);
}

void iret_to_epilog(dynarec_arm_t* dyn, int ninst)
{
    MESSAGE(LOG_DUMP, "IRet epilog\n");
    // POP IP
    POP1(xEIP);
    // POP CS
    MOVW(x1, offsetof(x86emu_t, segs[_CS]));
    POP1(x2);
    STRH_REG(x2, xEmu, x1);
    MOVW(x1, 0);
    STR_IMM9(x1, xEmu, offsetof(x86emu_t, segs_serial[_CS]));
    // POP EFLAGS
    POP1(xFlags);
    MOV32(x1, 0x3F7FD7);
    AND_REG_LSL_IMM5(xFlags, xFlags, x1, 0);
    ORR_IMM8(xFlags, xFlags, 2, 0);
    SET_DFNONE(x1);
    // Ret....
    MOV32_(x2, (uintptr_t)arm_epilog);  // epilog on purpose, CS might have changed!
    BX(x2);
}

void call_c(dynarec_arm_t* dyn, int ninst, void* fnc, int reg, int ret, uint32_t mask, int saveflags)
{
    if(ret!=-2 && !mask) {
        // ARM ABI require the stack to be 8-bytes aligned!
        // so, if no mask asked, add one to stay 8-bytes aligned
        if(ret!=x3) mask=1<<x3; else mask=1<<x14;
    }
    if(ret!=-2) {
        PUSH(xSP, (1<<xEmu) | mask);
    }
    fpu_pushcache(dyn, ninst, reg);
    if(saveflags) {
        STR_IMM9(xFlags, xEmu, offsetof(x86emu_t, eflags));
    }
    MOV32(reg, (uintptr_t)fnc);
    BLX(reg);
    fpu_popcache(dyn, ninst, reg);
    if(ret>=0) {
        MOV_REG(ret, 0);
    }
    if(ret!=-2) {
        POP(xSP, (1<<xEmu) | mask);
    }
    if(saveflags) {
        LDR_IMM9(xFlags, xEmu, offsetof(x86emu_t, eflags));
    }
    SET_NODF();
}

#if defined(__ARM_PCS) && !defined(__ARM_PCS_VFP)
// if __ARM_PCS then AARCCH32 is target
// if __ARM_PCS_VFP then -mfloat-abi=hard is used
#define ARM_SOFTFP
#endif
// call a function with n double args (taking care of the SOFTFP / HARD call) that return a double too
void call_d(dynarec_arm_t* dyn, int ninst, void* fnc, void* fnc2, int n, int reg, int ret, uint32_t mask, int saveflags)
{
    if(ret!=-2 && !mask) {
        // ARM ABI require the stack to be 8-bytes aligned!
        // so, if no mask asked, add one to stay 8-bytes aligned
        if(ret!=xFlags) mask=1<<xFlags; else mask=1<<x3;
    }
    if(ret!=-2) {
        PUSH(xSP, (1<<xEmu) | mask);
    }
    fpu_pushcache(dyn, ninst, reg);
    if(saveflags) {
        STR_IMM9(xFlags, xEmu, offsetof(x86emu_t, eflags));
    }
    #ifdef ARM_SOFTFP
    if(n==1) {
        SUB_IMM8(xSP, xSP, 8);
        VST1_64(0, xSP);    //store args on the stack
    } else {    // n == 2, nothing else!
        SUB_IMM8(xSP, xSP, n*8);
        ADD_IMM8(reg, xSP, 8)
        VST1_64(1, reg);
        VST1_64(0, xSP);    //store args on the stack
    }
    #endif
    MOV32(reg, (uintptr_t)fnc);
    BLX(reg);
    if(fnc2) {
        #ifdef ARM_SOFTFP
        STM(xSP, (1<<0)|(1<<1));    // put r0:r1 result on the stack for next call
        #endif
        MOV32(reg, (uintptr_t)fnc2);
        BLX(reg);
    }
    #ifdef ARM_SOFTFP
    ADD_IMM8(xSP, xSP, n*8);
    VMOVtoV_64(0, 0, 1);    // load r0:r1 to D0 to simulate hardfo
    #endif
    fpu_popcache(dyn, ninst, reg);
    if(ret>=0) {
        MOV_REG(ret, 0);
    }
    if(ret!=-2) {
        POP(xSP, (1<<xEmu) | mask);
    }
    if(saveflags) {
        LDR_IMM9(xFlags, xEmu, offsetof(x86emu_t, eflags));
    }
    SET_NODF();
}

void grab_tlsdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg)
{
    MESSAGE(LOG_DUMP, "Get TLSData\n");
    int32_t j32;
    MAYUSE(j32);
    LDR_IMM9(x1, xEmu, offsetof(x86emu_t, context));
    LDR_IMM9(x14, xEmu, offsetof(x86emu_t, segs_serial[_GS]));  // complete check here
    LDR_IMM9(x1, x1, offsetof(box86context_t, sel_serial));
    CMPS_REG_LSL_IMM5(x14, x1, 0);
    LDR_IMM9_COND(cEQ, reg, xEmu, offsetof(x86emu_t, segs_offs[_GS]));
    B_MARKSEG(cEQ);
    MOVW(x1, _GS);
    call_c(dyn, ninst, GetSegmentBaseEmu, x14, reg, 0, 1);
    MARKSEG;
    MESSAGE(LOG_DUMP, "----TLSData\n");
}

void grab_fsdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg)
{
    int32_t j32;
    MAYUSE(j32);
    MESSAGE(LOG_DUMP, "Get FS: Offset\n");
    LDR_IMM9(x14, xEmu, offsetof(x86emu_t, segs_serial[_FS]));// fast check here
    CMPS_IMM8(x14, 0);
    LDR_IMM9_COND(cNE, reg, xEmu, offsetof(x86emu_t, segs_offs[_FS]));
    B_MARKSEG(cNE);
    MOVW(x1, _FS);
    call_c(dyn, ninst, GetSegmentBaseEmu, x14, reg, 0, 1);
    MARKSEG;
    MESSAGE(LOG_DUMP, "----FS: Offset\n");
}

// x87 stuffs
static void x87_reset(dynarec_arm_t* dyn, int ninst)
{
#if STEP > 1
    dyn->x87count = 0;
    for (int i=0; i<8; ++i)
        dyn->x87cache[i] = -1;
    dyn->x87stack = 0;
#endif
}

void x87_stackcount(dynarec_arm_t* dyn, int ninst, int scratch)
{
#if STEP > 1
    if(!dyn->x87stack)
        return;
    if(dyn->mmxcount)
        mmx_purgecache(dyn, ninst, scratch);
    MESSAGE(LOG_DUMP, "\tSynch x87 Stackcount (%d)\n", dyn->x87stack);
    int a = dyn->x87stack;
    // Add x87stack to emu fpu_stack
    LDR_IMM9(scratch, xEmu, offsetof(x86emu_t, fpu_stack));
    if(a>0) {
        ADD_IMM8(scratch, scratch, a);
    } else {
        SUB_IMM8(scratch, scratch, -a);
    }
    STR_IMM9(scratch, xEmu, offsetof(x86emu_t, fpu_stack));
    // Sub x87stack to top, with and 7
    LDR_IMM9(scratch, xEmu, offsetof(x86emu_t, top));
    if(a>0) {
        SUB_IMM8(scratch, scratch, a);
    } else {
        ADD_IMM8(scratch, scratch, -a);
    }
    AND_IMM8(scratch, scratch, 7);
    STR_IMM9(scratch, xEmu, offsetof(x86emu_t, top));
    // reset x87stack
    dyn->x87stack = 0;
    MESSAGE(LOG_DUMP, "\t------x87 Stackcount\n");
#endif
}

int x87_do_push(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    if(dyn->mmxcount)
        mmx_purgecache(dyn, ninst, s1);
    ++dyn->x87count;
    dyn->x87stack+=1;
    // move all regs in cache, and find a free one
    int ret = -1;
    for(int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1)
            ++dyn->x87cache[i];
        else if(ret==-1) {
            dyn->x87cache[i] = 0;
            ret=dyn->x87reg[i]=fpu_get_reg_double(dyn);
        }
    return ret;
#else
    return 0;
#endif
}
void x87_do_push_empty(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    if(dyn->mmxcount)
        mmx_purgecache(dyn, ninst, s1);
    ++dyn->x87count;
    dyn->x87stack+=1;
    // move all regs in cache
    for(int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1)
            ++dyn->x87cache[i];
    if(s1)
        x87_stackcount(dyn, ninst, s1);
#endif
}
void x87_do_pop(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    if(dyn->mmxcount)
        mmx_purgecache(dyn, ninst, s1);
    dyn->x87stack-=1;
    // move all regs in cache, poping ST0
    for(int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1) {
            --dyn->x87cache[i];
            if(dyn->x87cache[i]==-1) {
                fpu_free_reg_double(dyn, dyn->x87reg[i]);
                dyn->x87reg[i] = -1;
            }
        }
    --dyn->x87count;
#endif
}

void x87_purgecache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
#if STEP > 1
    dyn->x87count = 0;
    int ret = 0;
    for (int i=0; i<8 && !ret; ++i)
        if(dyn->x87cache[i] != -1)
            ret = 1;
    if(!ret && !dyn->x87stack)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tPurge x87 Cache and Synch Stackcount (%+d)\n", dyn->x87stack);
    int a = dyn->x87stack;
    if(a!=0) {
        // reset x87stack
        dyn->x87stack = 0;
        // Add x87stack to emu fpu_stack
        LDR_IMM9(s2, xEmu, offsetof(x86emu_t, fpu_stack));
        if(a>0) {
            ADD_IMM8(s2, s2, a);
        } else {
            SUB_IMM8(s2, s2, -a);
        }
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, fpu_stack));
        // Sub x87stack to top, with and 7
        LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
        // update tags (and top at the same time)
        if(a>0) {
            // new tag to fulls
            MOVW(s3, 0);
            int offs = offsetof(x86emu_t, p_regs);
            if(!(offs&3) && (offs>>2)<256) {
                ADD_IMM8_ROR(s1, xEmu, offs>>2, 15);
            } else {
                MOVW(s1, offs);
                ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
            }
            for (int i=0; i<a; ++i) {
                SUB_IMM8(s2, s2, 1);
                AND_IMM8(s2, s2, 7);    // (emu->top + st)&7
                STR_REG_LSL_IMM5(s3, s1, s2, 2);    // that slot is full
            }
        } else {
            // empty tags
            MOVW(s3, 0b11);
            int offs = offsetof(x86emu_t, p_regs);
            if(!(offs&3) && (offs>>2)<256) {
                ADD_IMM8_ROR(s1, xEmu, offs>>2, 15);
            } else {
                MOVW(s1, offsetof(x86emu_t, p_regs));
                ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
            }
            for (int i=0; i<-a; ++i) {
                STR_REG_LSL_IMM5(s3, s1, s2, 2);    // empty slot before leaving it
                ADD_IMM8(s2, s2, 1);
                AND_IMM8(s2, s2, 7);    // (emu->top + st)&7
            }
        }
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    } else {
        LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    }
    if(ret!=0) {
        // --- set values
        // prepare offset to fpu => s1
        if(offsetof(x86emu_t, mmx87)<256) {
            ADD_IMM8(s1, xEmu, offsetof(x86emu_t, mmx87));
        } else {
            MOVW(s1, offsetof(x86emu_t, mmx87));
            ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
        }
        // Get top
        // loop all cache entries
        for (int i=0; i<8; ++i)
            if(dyn->x87cache[i]!=-1) {
                ADD_IMM8(s3, s2, dyn->x87cache[i]);
                AND_IMM8(s3, s3, 7);    // (emu->top + st)&7
                ADD_REG_LSL_IMM5(s3, s1, s3, 3);    // fpu[(emu->top+i)&7] lsl 3 because fpu are double, so 8 bytes
                VSTR_64(dyn->x87reg[i], s3, 0);    // save the value
                fpu_free_reg_double(dyn, dyn->x87reg[i]);
                dyn->x87reg[i] = -1;
                dyn->x87cache[i] = -1;
            }
    }
#endif
}

#ifdef HAVE_TRACE
static void x87_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
#if STEP > 1
    x87_stackcount(dyn, ninst, s1);
    int ret = 0;
    for (int i=0; (i<8) && (!ret); ++i)
        if(dyn->x87cache[i] != -1)
            ret = 1;
    if(!ret)    // nothing to do
        return;
    // prepare offset to fpu => s1
    if(offsetof(x86emu_t, mmx87)<256) {
        ADD_IMM8(s1, xEmu, offsetof(x86emu_t, mmx87));
    } else {
        MOVW(s1, offsetof(x86emu_t, mmx87));
        ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
    }
    // Get top
    LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    // loop all cache entries
    for (int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1) {
            ADD_IMM8(s3, s2, dyn->x87cache[i]);
            AND_IMM8(s3, s3, 7);    // (emu->top + i)&7
            ADD_REG_LSL_IMM5(s3, s1, s3, 3);    // fpu[(emu->top+i)&7] lsl 3 because fpu are double, so 8 bytes
            VSTR_64(dyn->x87reg[i], s3, 0);    // save the value
        }
#endif
}
#endif

int x87_get_cache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
#if STEP > 1
    if(dyn->mmxcount)
        mmx_purgecache(dyn, ninst, s1);
    // search in cache first
    for (int i=0; i<8; ++i)
        if(dyn->x87cache[i]==st)
            return i;
    MESSAGE(LOG_DUMP, "\tCreate x87 Cache for ST%d\n", st);
    // get a free spot
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->x87cache[i]==-1)
            ret = i;
    // found, setup and grab the value
    dyn->x87cache[ret] = st;
    dyn->x87reg[ret] = fpu_get_reg_double(dyn);
    ++dyn->x87count;
    if(offsetof(x86emu_t, mmx87)<256) {
        ADD_IMM8(s1, xEmu, offsetof(x86emu_t, mmx87));
    } else {
        MOVW(s1, offsetof(x86emu_t, mmx87));
        ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
    }
    LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    int a = st - dyn->x87stack;
    if(a) {
        if(a<0) {
            SUB_IMM8(s2, s2, -a);
        } else {
            ADD_IMM8(s2, s2, a);
        }
        AND_IMM8(s2, s2, 7);    // (emu->top + i)&7
    }
    ADD_REG_LSL_IMM5(s2, s1, s2, 3);
    VLDR_64(dyn->x87reg[ret], s2, 0);
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);

    return ret;
#else
    return 0;
#endif
}

int x87_get_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a)
{
#if STEP > 1
    return dyn->x87reg[x87_get_cache(dyn, ninst, s1, s2, a)];
#else
    return 0;
#endif
}


void x87_refresh(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
#if STEP > 1
    x87_stackcount(dyn, ninst, s1);
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->x87cache[i] == st)
            ret = i;
    if(ret==-1)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tRefresh x87 Cache for ST%d\n", st);
    // prepare offset to fpu => s1
    if(offsetof(x86emu_t, mmx87)<256) {
        ADD_IMM8(s1, xEmu, offsetof(x86emu_t, mmx87));
    } else {
        MOVW(s1, offsetof(x86emu_t, mmx87));
        ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
    }
    // Get top
    LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    // Update
    if(st) {
        ADD_IMM8(s2, s2, st);
        AND_IMM8(s2, s2, 7);    // (emu->top + i)&7
    }
    ADD_REG_LSL_IMM5(s2, s1, s2, 3);    // fpu[(emu->top+i)&7] lsl 3 because fpu are double, so 8 bytes
    VSTR_64(dyn->x87reg[ret], s2, 0);    // save the value
    MESSAGE(LOG_DUMP, "\t--------x87 Cache for ST%d\n", st);
#endif
}

void x87_forget(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
#if STEP > 1
    x87_stackcount(dyn, ninst, s1);
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->x87cache[i] == st)
            ret = i;
    if(ret==-1)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tForget x87 Cache for ST%d\n", st);
    // prepare offset to fpu => s1
    if(offsetof(x86emu_t, mmx87)<256) {
        ADD_IMM8(s1, xEmu, offsetof(x86emu_t, mmx87));
    } else {
        MOVW(s1, offsetof(x86emu_t, mmx87));
        ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
    }
    // Get top
    LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    // Update
    if(st) {
        ADD_IMM8(s2, s2, st);
        AND_IMM8(s2, s2, 7);    // (emu->top + i)&7
    }
    ADD_REG_LSL_IMM5(s2, s1, s2, 3);    // fpu[(emu->top+i)&7] lsl 3 because fpu are double, so 8 bytes
    VSTR_64(dyn->x87reg[ret], s2, 0);    // save the value
    MESSAGE(LOG_DUMP, "\t--------x87 Cache for ST%d\n", st);
    // and forget that cache
    fpu_free_reg_double(dyn, dyn->x87reg[ret]);
    dyn->x87cache[ret] = -1;
    dyn->x87reg[ret] = -1;
    --dyn->x87count;
#endif
}

void x87_reget_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
#if STEP > 1
    if(dyn->mmxcount)
        mmx_purgecache(dyn, ninst, s1);
    // search in cache first
    for (int i=0; i<8; ++i)
        if(dyn->x87cache[i]==st) {
            // refresh the value
        MESSAGE(LOG_DUMP, "\tRefresh x87 Cache for ST%d\n", st);
            if(offsetof(x86emu_t, mmx87)<256) {
                ADD_IMM8(s1, xEmu, offsetof(x86emu_t, mmx87));
            } else {
                MOVW(s1, offsetof(x86emu_t, mmx87));
                ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
            }
            LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
            int a = st - dyn->x87stack;
            if(a<0) {
                SUB_IMM8(s2, s2, -a);
            } else {
                ADD_IMM8(s2, s2, a);
            }
            AND_IMM8(s2, s2, 7);    // (emu->top + i)&7
            ADD_REG_LSL_IMM5(s2, s1, s2, 3);
            VLDR_64(dyn->x87reg[i], s2, 0);
            MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
            // ok
            return;
        }
    // Was not in the cache? creating it....
    MESSAGE(LOG_DUMP, "\tCreate x87 Cache for ST%d\n", st);
    // get a free spot
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->x87cache[i]==-1)
            ret = i;
    // found, setup and grab the value
    dyn->x87cache[ret] = st;
    dyn->x87reg[ret] = fpu_get_reg_double(dyn);
    ++dyn->x87count;
    if(offsetof(x86emu_t, mmx87)<256) {
        ADD_IMM8(s1, xEmu, offsetof(x86emu_t, mmx87));
    } else {
        MOVW(s1, offsetof(x86emu_t, mmx87));
        ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
    }
    LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    int a = st - dyn->x87stack;
    if(a<0) {
        SUB_IMM8(s2, s2, -a);
    } else {
        ADD_IMM8(s2, s2, a);
    }
    AND_IMM8(s2, s2, 7);    // (emu->top + i)&7
    ADD_REG_LSL_IMM5(s2, s1, s2, 3);
    VLDR_64(dyn->x87reg[ret], s2, 0);
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
#endif
}

static int round_map[] = {0, 2, 1, 3};  // map x86 -> arm round flag

// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    LDRH_IMM8(s1, xEmu, offsetof(x86emu_t, cw));    // hopefully cw is not too far for an imm8
    UBFX(s2, s1, 10, 2);    // extract round...
    MOV32(s1, round_map);
    LDR_REG_LSL_IMM5(s2, s1, s2, 2);
    VMRS(s1);               // get fpscr
    MOV_REG(s3, s1);
    BFI(s1, s2, 22, 2);     // inject new round
    VMSR(s1);               // put new fpscr
    return s3;
}

// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    LDRH_IMM8(s1, xEmu, offsetof(x86emu_t, mxcsr));
    UBFX(s2, s1, 13, 2);    // extract round...
    MOV32(s1, round_map);
    LDR_REG_LSL_IMM5(s2, s1, s2, 2);
    VMRS(s1);               // get fpscr
    MOV_REG(s3, s1);
    BFI(s1, s2, 22, 2);     // inject new round
    VMSR(s1);               // put new fpscr
    return s3;
}

// Restore round flag
void x87_restoreround(dynarec_arm_t* dyn, int ninst, int s1)
{
    VMSR(s1);               // put back fpscr
}

// MMX helpers
static void mmx_reset(dynarec_arm_t* dyn, int ninst)
{
#if STEP > 1
    dyn->mmxcount = 0;
    for (int i=0; i<8; ++i)
        dyn->mmxcache[i] = -1;
#endif
}
// get neon register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
#if STEP > 1
    if(dyn->x87count)
        x87_purgecache(dyn, ninst, s1, s2, s3);
    if(dyn->mmxcache[a]!=-1)
        return dyn->mmxcache[a];
    ++dyn->mmxcount;
    int ret = dyn->mmxcache[a] = fpu_get_reg_double(dyn);
    int offs = offsetof(x86emu_t, mmx87[a]);
    if(!(offs&3) && (offs>>2)<256) {
        ADD_IMM8_ROR(s1, xEmu, offs>>2, 15);
    } else {
        MOV32(s1, offs);
        ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
    }
    VLD1_32(ret, s1);
    return ret;
#else
    return 0;
#endif
}
// get neon register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
#if STEP > 1
    if(dyn->x87count)
        x87_purgecache(dyn, ninst, s1, s2, s3);
    if(dyn->mmxcache[a]!=-1)
        return dyn->mmxcache[a];
    ++dyn->mmxcount;
    int ret = dyn->mmxcache[a] = fpu_get_reg_double(dyn);
    return ret;
#else
    return 0;
#endif
}
// purge the MMX cache only(needs 3 scratch registers)
void mmx_purgecache(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    if(!dyn->mmxcount)
        return;
    dyn->mmxcount = 0;
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->mmxcache[i]!=-1) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge MMX Cache ------\n");
                int offs = offsetof(x86emu_t, mmx87[i]);
                if(!(offs&3) && (offs>>2)<256) {
                    ADD_IMM8_ROR(s1, xEmu, offs>>2, 15);
                } else {
                    MOV32(s1, offs);
                    ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
                }
                old = i+1;  //+1 because VST1 with write back
            } else {
                if(old!=i) {
                    ADD_IMM8(s1, s1, (i-old)*8);
                }
                old = i+1;
            }
            VST1_32_W(dyn->mmxcache[i], s1);
            fpu_free_reg_double(dyn, dyn->mmxcache[i]);
            dyn->mmxcache[i] = -1;
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge MMX Cache\n");
    }
#endif
}
#ifdef HAVE_TRACE
static void mmx_reflectcache(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->mmxcache[i]!=-1) {
            if (old==-1) {
                int offs = offsetof(x86emu_t, mmx87[i]);
                if(!(offs&3) && (offs>>2)<256) {
                    ADD_IMM8_ROR(s1, xEmu, offs>>2, 15);
                } else {
                    MOV32(s1, offs);
                    ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
                }
                old = i+1;
            } else {
                if(old!=i) {
                    ADD_IMM8(s1, s1, (i-old)*8);
                }
                old = i+1;
            }
            VST1_32_W(dyn->mmxcache[i], s1);
        }
#endif
}
#endif


// SSE / SSE2 helpers
static void sse_reset(dynarec_arm_t* dyn, int ninst)
{
#if STEP > 1
    for (int i=0; i<8; ++i)
        dyn->ssecache[i] = -1;
#endif
}
// get neon register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a)
{
#if STEP > 1
    if(dyn->ssecache[a]!=-1)
        return dyn->ssecache[a];
    int ret = dyn->ssecache[a] = fpu_get_reg_quad(dyn);
    int offs = offsetof(x86emu_t, xmm[a]);
    if(!(offs&3) && (offs>>2)<256) {
        ADD_IMM8_ROR(s1, xEmu, (offs>>2), 0xf);
    } else {
        MOV32(s1, offs);
        ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
    }
    VLD1Q_32(ret, s1);
    return ret;
#else
    return 0;
#endif
}
// get neon register for a SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a)
{
#if STEP > 1
    if(dyn->ssecache[a]!=-1)
        return dyn->ssecache[a];
    int ret = dyn->ssecache[a] = fpu_get_reg_quad(dyn);
    return ret;
#else
    return 0;
#endif
}
// purge the SSE cache only(needs 3 scratch registers)
static void sse_purgecache(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->ssecache[i]!=-1) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge SSE Cache ------\n");
                int offs = offsetof(x86emu_t, xmm[i]);
                if(!(offs&3) && (offs>>2)<256) {
                    ADD_IMM8_ROR(s1, xEmu, offs>>2, 15);
                } else {
                    MOV32(s1, offs);
                    ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
                }
                old = i+1;  //+1 because VST1Q with write back
            } else {
                if(old!=i) {
                    ADD_IMM8(s1, s1, (i-old)*16);
                }
                old = i+1;
            }
            VST1Q_32_W(dyn->ssecache[i], s1);
            fpu_free_reg_quad(dyn, dyn->ssecache[i]);
            dyn->ssecache[i] = -1;
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge SSE Cache\n");
    }
#endif
}
#ifdef HAVE_TRACE
static void sse_reflectcache(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->ssecache[i]!=-1) {
            if (old==-1) {
                int offs = offsetof(x86emu_t, xmm[i]);
                if(!(offs&3) && (offs>>2)<256) {
                    ADD_IMM8_ROR(s1, xEmu, offs>>2, 15);
                } else {
                    MOV32(s1, offs);
                    ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
                }
                old = i+1;
            } else {
                if(old!=i) {
                    ADD_IMM8(s1, s1, (i-old)*16);
                }
                old = i+1;
            }
            VST1Q_32_W(dyn->ssecache[i], s1);
        }
#endif
}
#endif

void fpu_pushcache(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    // only need to push 16-31...
    int n=0;
    for (int i=16; i<32; i++)
        if(dyn->fpuused[i-8])
            ++n;
    if(!n)
        return;
    MESSAGE(LOG_DUMP, "\tPush FPU Cache (%d)------\n", n);
    if(n>=8) {
        MOVW(s1, n*8);
        SUB_REG_LSL_IMM5(xSP, xSP, s1, 0);
    } else {
        SUB_IMM8(xSP, xSP, n*8);
    }
    MOV_REG(s1, xSP);
    for (int i=16; i<32; ++i) {   // should use VSTM?
        if(dyn->fpuused[i-8]) {
            VST1_32_W(i, s1);
        }
    }
    MESSAGE(LOG_DUMP, "\t------- Push FPU Cache (%d)\n", n);
#endif
}

void fpu_popcache(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    // only need to push 16-31...
    int n=0;
    for (int i=16; i<32; i++)
        if(dyn->fpuused[i-8])
            ++n;
    if(!n)
        return;
    MESSAGE(LOG_DUMP, "\tPop FPU Cache (%d)------\n", n);
    MOV_REG(s1, xSP);
    for (int i=16; i<32; ++i) {
        if(dyn->fpuused[i-8]) {
            VLD1_32_W(i, s1);
        }
    }
    if(n>=8) {
        MOVW(s1, n*8);
        ADD_REG_LSL_IMM5(xSP, xSP, s1, 0);
    } else {
        ADD_IMM8(xSP, xSP, n*8);
    }
    MESSAGE(LOG_DUMP, "\t------- Pop FPU Cache (%d)\n", n);
#endif
}

void fpu_purgecache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    x87_purgecache(dyn, ninst, s1, s2, s3);
    mmx_purgecache(dyn, ninst, s1);
    sse_purgecache(dyn, ninst, s1);
    fpu_reset_reg(dyn);
}

#ifdef HAVE_TRACE
void fpu_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    x87_reflectcache(dyn, ninst, s1, s2, s3);
    if(trace_emm)
       mmx_reflectcache(dyn, ninst, s1);
    if(trace_xmm)
       sse_reflectcache(dyn, ninst, s1);
}
#endif

void fpu_reset(dynarec_arm_t* dyn, int ninst)
{
    x87_reset(dyn, ninst);
    mmx_reset(dyn, ninst);
    sse_reset(dyn, ninst);
    fpu_reset_reg(dyn);
}

// get the single reg that from the double "reg" (so Dx[idx])
int fpu_get_single_reg(dynarec_arm_t* dyn, int ninst, int reg, int idx)
{
#if STEP > 1
    if(reg<16)
        return reg*2+idx;
    int a = fpu_get_scratch_double(dyn);
    VMOV_64(a, reg);
    return a*2+idx;
#else
    return 0;
#endif
}
// put back (if needed) the single reg in place
void fpu_putback_single_reg(dynarec_arm_t* dyn, int ninst, int reg, int idx, int s)
{
    if(reg>=16) {
        VMOV_64(reg, s/2);
    }
}

void emit_pf(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4)
{
    // PF: (((emu->x86emu_parity_tab[(res) / 32] >> ((res) % 32)) & 1) == 0)
    AND_IMM8(s3, s1, 0xE0); // lsr 5 masking pre-applied
    MOV32(s4, GetParityTab());
    LDR_REG_LSR_IMM5(s4, s4, s3, 5-2);   // x/32 and then *4 because array is integer
    AND_IMM8(s3, s1, 31);
    MVN_REG_LSR_REG(s4, s4, s3);
    BFI(xFlags, s4, F_PF, 1);
}