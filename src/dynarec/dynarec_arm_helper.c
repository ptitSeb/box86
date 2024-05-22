#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

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
uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int* fixaddress, uint32_t absmax, uint32_t mask, int getfixonly, int* l)
{
    int lock = l?((l==LOCK_LOCK)?1:2):0;
    if(lock==2)
        *l = (box86_dynarec_strongmem==2)?1:0;
    uint8_t ret = 2;
    uint8_t scratch = 2;
    *fixaddress = 0;
    if(hint>0) ret = hint;
    if(hint>0 && hint<xEAX) scratch = hint;
    if(hint==xEIP) scratch = hint;  // allow this one as a scratch and return register
    MAYUSE(scratch);

    if(l==LOCK_LOCK) { /*SMDMB();*/DMB_ISH(); }

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
                    switch(lock) {
                        case 1: addLockAddress(tmp); break;
                        case 2: if(isLockAddress(tmp)) *l=1; break;
                    }
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
            if(getfixonly)
                *fixaddress = tmp;
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
    if(!n && (m&7)==6) {
        offset = F16S;
        MOVW(ret, offset);
    } else {
        switch(n) {
            case 0: offset = 0; break;
            case 1: offset = F8S; break;
            case 2: offset = F16S; break;
        }
        if(offset && (abs(offset)<=absmax && !(offset&mask))) {
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
    SMEND();
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
    SMEND();
    BX(x3);
}

void ret_to_epilog(dynarec_arm_t* dyn, int ninst)
{
    MESSAGE(LOG_DUMP, "Ret next\n");
    POP1(xEIP);
    SMEND();
    if(box86_dynarec_callret) {
        // pop the actual return address for ARM stack
        POP(xSP, (1<<x2)|(1<<x3));
        CMPS_REG_LSL_IMM5(x2, xEIP, 0); // is it the right address?
        BXcond(cEQ, x3);
        // not the correct return address, regular jump, but purge the stack first, it's unsync now...
        CMPS_IMM8(x3, 0);   // that was already the top of the stack...
        LDR_IMM9_COND(cNE, xSP, xEmu, offsetof(x86emu_t, xSPSave)); // load pointer only if not already on top
        SUB_COND_IMM8(cEQ, xSP, xSP, 8);   // unpop
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
    SMEND();
    if(box86_dynarec_callret) {
        // pop the actual return address for ARM stack
        POP(xSP, (1<<x2)|(1<<x3));
        CMPS_REG_LSL_IMM5(x2, xEIP, 0); // is it the right address?
        BXcond(cEQ, x3);
        // not the correct return address, regular jump, but purge the stack first, it's unsync now...
        CMPS_IMM8(x3, 0);   // that was already the top of the stack...
        LDR_IMM9_COND(cNE, xSP, xEmu, offsetof(x86emu_t, xSPSave));
        SUB_COND_IMM8(cEQ, xSP, xSP, 8);   // unpop
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
    SET_DFNONE(x1);
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
    SMEND();
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
}

#if defined(__ARM_PCS) && !defined(__ARM_PCS_VFP)
// if __ARM_PCS then AARCCH32 is target
// if __ARM_PCS_VFP then -mfloat-abi=hard is used
#define ARM_SOFTFP
#endif
// call a function with n double args (taking care of the SOFTFP / HARD call) that return a double too
void call_dr(dynarec_arm_t* dyn, int ninst, int reg, int n, int s1, int ret, int ret2, uint32_t mask, int saveflags)
{
    if(ret!=-2 && !mask) {
        // ARM ABI require the stack to be 8-bytes aligned!
        // so, if no mask asked, add one to stay 8-bytes aligned
        if(ret!=xFlags) mask=1<<xFlags; else mask=1<<x3;
    }
    if(ret!=-2) {
        PUSH(xSP, (1<<xEmu) | mask);
    }
    fpu_pushcache(dyn, ninst, s1);
    if(saveflags) {
        STR_IMM9(xFlags, xEmu, offsetof(x86emu_t, eflags));
    }
    #ifdef ARM_SOFTFP
    VMOVfrV_64(0, 1, 0);// D0 -> r0:r1
    if(n!=1) { // n == 2, nothing else!
        PUSH(xSP, (1<<2) | (1<<3));
        VMOVfrV_64(2, 3, 1);
    }
    #endif
    BLX(reg);
    #ifdef ARM_SOFTFP
    if(n!=1) {
        POP(xSP, (1<<2) | (1<<3));
    }
    if(ret==-1) {
        VMOVtoV_64(0, 0, 1);    // load r0:r1 to D0 to simulate hardfo
    }
    #endif
    fpu_popcache(dyn, ninst, s1);
    if(ret>=0) {
        MOV_REG(ret, 0);
    }
    if(ret2>=0) {
        MOV_REG(ret2, 1);
    }
    if(ret!=-2) {
        POP(xSP, (1<<xEmu) | mask);
    }
    if(saveflags) {
        LDR_IMM9(xFlags, xEmu, offsetof(x86emu_t, eflags));
    }
}
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
    VMOVfrV_64(0, 1, 0);// D0 -> r0:r1
    if(n!=1) { // n == 2, nothing else!
        PUSH(xSP, (1<<2) | (1<<3));
        VMOVfrV_64(2, 3, 1);
    }
    #endif
    MOV32(reg, (uintptr_t)fnc);
    BLX(reg);
    if(fnc2) {
        #ifdef ARM_SOFTFP
        // result are already in r0:r1 for next call
        #endif
        MOV32(reg, (uintptr_t)fnc2);
        BLX(reg);
    }
    #ifdef ARM_SOFTFP
    if(n!=1) {
        POP(xSP, (1<<2) | (1<<3));
    }
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
}
// call a function with 1 double arg (taking care of the SOFTFP / HARD call) and 1 non-float arg that return a double
void call_ddr(dynarec_arm_t* dyn, int ninst, void* fnc, void* fnc2, int arg, int reg, int ret, uint32_t mask, int saveflags)
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
    VMOVfrV_64(0, 1, 0);// D0 -> r0:r1
    MOV_REG(2, arg);
    #else
    MOV_REG(0, arg);
    #endif
    MOV32(reg, (uintptr_t)fnc);
    BLX(reg);
    if(fnc2) {
        #ifdef ARM_SOFTFP
        // result are already in r0:r1 for next call
        #endif
        MOV32(reg, (uintptr_t)fnc2);
        BLX(reg);
    }
    #ifdef ARM_SOFTFP
    POP(xSP, (1<<2) | (1<<3));
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
}

// call a function with 1 arg, (taking care of the SOFTFP / HARD call) that return a double, using s1 as scratch
void call_rd(dynarec_arm_t* dyn, int ninst, void* fnc, int reg, int s1, uint32_t mask, int saveflags)
{
    if(!mask) {
        // ARM ABI require the stack to be 8-bytes aligned!
        // so, if no mask asked, add one to stay 8-bytes aligned
        if(s1!=xFlags) mask=1<<xFlags; else mask=1<<x3;
    }
    PUSH(xSP, (1<<xEmu) | mask);
    fpu_pushcache(dyn, ninst, s1);
    if(saveflags) {
        STR_IMM9(xFlags, xEmu, offsetof(x86emu_t, eflags));
    }
    if(reg!=0) {
        MOV_REG(0, reg);
    }
    MOV32(s1, (uintptr_t)fnc);
    BLX(s1);
    #ifdef ARM_SOFTFP
    VMOVtoV_64(0, 0, 1);    // load r0:r1 to D0 to simulate hardfo
    #endif
    fpu_popcache(dyn, ninst, s1);
    POP(xSP, (1<<xEmu) | mask);
    if(saveflags) {
        LDR_IMM9(xFlags, xEmu, offsetof(x86emu_t, eflags));
    }
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
int x87_stackcount(dynarec_arm_t* dyn, int ninst, int scratch)
{
    if(!dyn->n.x87stack)
        return 0;
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, scratch);
    MESSAGE(LOG_DUMP, "\tSynch x87 Stackcount (%d)\n", dyn->n.x87stack);
    int a = dyn->n.x87stack;
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
    // reset x87stack, but not the stack count of neoncache
    dyn->n.x87stack = 0;
    dyn->n.stack_next -= dyn->n.stack;
    dyn->n.stack = 0;
    MESSAGE(LOG_DUMP, "\t------x87 Stackcount\n");
    return a;
}

void x87_unstackcount(dynarec_arm_t* dyn, int ninst, int scratch, int count)
{
    if(!count)
        return;
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, scratch);
    MESSAGE(LOG_DUMP, "\tSynch x87 Unstackcount (%d)\n", dyn->n.x87stack);
    int a = -count;
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
    // reset x87stack, but not the stack count of neoncache
    dyn->n.x87stack = count;
    dyn->n.stack = count;
    dyn->n.stack_next += dyn->n.stack;
    MESSAGE(LOG_DUMP, "\t------x87 Unstackcount\n");
}

int neoncache_st_coherency(dynarec_arm_t* dyn, int ninst, int a, int b)
{
    int i1 = neoncache_get_st(dyn, ninst, a);
    int i2 = neoncache_get_st(dyn, ninst, b);
    if(i1!=i2) {
        MESSAGE(LOG_DUMP, "Warning, ST cache incoherent between ST%d(%d) and ST%d(%d)\n", a, i1, b, i2);
    }

    return i1;
}

// On step 1, Float/Double for ST is actualy computed and back-propagated
// On step 2-3, the value is just read for inst[...].n.neocache[..]
// the reg returned is *2 for FLOAT
int x87_do_push(dynarec_arm_t* dyn, int ninst, int s1, int t)
{
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    dyn->n.x87stack+=1;
    dyn->n.stack+=1;
    dyn->n.stack_next+=1;
    dyn->n.stack_push+=1;
    ++dyn->n.pushed;
    if(dyn->n.poped)
        --dyn->n.poped;
    // move all regs in cache, and find a free one
    for(int j=0; j<24; ++j)
        if((dyn->n.neoncache[j].t == NEON_CACHE_ST_D) || (dyn->n.neoncache[j].t == NEON_CACHE_ST_F))
            ++dyn->n.neoncache[j].n;
    int ret = -1;
    dyn->n.tags<<=2;
    for(int i=0; i<8; ++i)
        if(dyn->n.x87cache[i]!=-1)
            ++dyn->n.x87cache[i];
        else if(ret==-1) {
            dyn->n.x87cache[i] = 0;
            ret=dyn->n.x87reg[i]=fpu_get_reg_double(dyn, t, 0);
            #if STEP == 1
            // need to check if reg is compatible with float
            if((ret>15) && (t == NEON_CACHE_ST_F))
                dyn->n.neoncache[ret-FPUFIRST].t = NEON_CACHE_ST_D;
            #else
            dyn->n.neoncache[ret-FPUFIRST].t = X87_ST0;
            #endif
        }
    return ret * dyn->n.neoncache[ret-FPUFIRST].t;
}
void x87_do_push_empty(dynarec_arm_t* dyn, int ninst, int s1)
{
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    dyn->n.x87stack+=1;
    dyn->n.stack+=1;
    dyn->n.stack_next+=1;
    dyn->n.stack_push+=1;
    ++dyn->n.pushed;
    if(dyn->n.poped)
        --dyn->n.poped;
    // move all regs in cache
    for(int j=0; j<24; ++j)
        if((dyn->n.neoncache[j].t == NEON_CACHE_ST_D) || (dyn->n.neoncache[j].t == NEON_CACHE_ST_F))
            ++dyn->n.neoncache[j].n;
    for(int i=0; i<8; ++i)
        if(dyn->n.x87cache[i]!=-1)
            ++dyn->n.x87cache[i];
    dyn->n.tags<<=2;
    if(s1)
        x87_stackcount(dyn, ninst, s1);
}
void static internal_x87_dopop(dynarec_arm_t* dyn)
{
    for(int i=0; i<8; ++i)
        if(dyn->n.x87cache[i]!=-1) {
            --dyn->n.x87cache[i];
            if(dyn->n.x87cache[i]==-1) {
                fpu_free_reg_double(dyn, dyn->n.x87reg[i]);
                dyn->n.x87reg[i] = -1;
            }
        }
}
int static internal_x87_dofree(dynarec_arm_t* dyn)
{
    if(dyn->n.tags&0b11) {
        MESSAGE(LOG_DUMP, "\t--------x87 FREED ST0, poping 1 more\n");
        return 1;
    }
    return 0;
}
void x87_do_pop(dynarec_arm_t* dyn, int ninst, int s1)
{
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    do {
        dyn->n.x87stack-=1;
        dyn->n.stack_next-=1;
        dyn->n.stack_pop+=1;
        dyn->n.tags>>=2;
        ++dyn->n.poped;
        if(dyn->n.pushed)
            --dyn->n.pushed;
        // move all regs in cache, poping ST0
        internal_x87_dopop(dyn);
    } while(internal_x87_dofree(dyn));
}

static void x87_purgecache_full(dynarec_arm_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    MESSAGE(LOG_DUMP, "\tPurge Full %sx87 Cache and Synch Stackcount (%+d)---\n", next?"locally ":"", dyn->n.x87stack);
    int a = dyn->n.x87stack;
    if(a!=0) {
        // reset x87stack
        if(!next)
            dyn->n.x87stack = 0;
        // Add x87stack to emu fpu_stack
        LDR_IMM9(s2, xEmu, offsetof(x86emu_t, fpu_stack));
        if(a>0) {
            ADD_IMM8(s2, s2, a);
        } else {
            SUB_IMM8(s2, s2, -a);
        }
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, fpu_stack));
    }
    // Setting all 8 xp_regs to "FULL"
    MOVW(s3, 0);
    STR_IMM9(s3, xEmu, offsetof(x86emu_t, fpu_tags));
    // setup TOP => full stack so = 0
    STR_IMM9(s3, xEmu, offsetof(x86emu_t, top));
    uintptr_t offs = offsetof(x86emu_t, x87);
    if(!(offs&3) && (offs>>2)<256) {
        ADD_IMM8_ROR(s1, xEmu, offs>>2, 15);
    } else {
        MOVW(s1, offs);
        ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
    }
    for(int i=0; i<8; ++i) {
        //Stack is full, so STi is just x87reg[i]
        int j=0; 
        while(dyn->n.x87cache[j]!=i) ++j; // look for STi
        int st = dyn->n.x87cache[j]+dyn->n.stack_pop;
        #if STEP == 1
        if(!next) {  // don't force promotion here
            // pre-apply pop, because purge happens in-between
            neoncache_promote_double(dyn, ninst, st);
        }
        #endif
        #if STEP == 3
        if(!next && neoncache_get_current_st(dyn, ninst, st)!=NEON_CACHE_ST_D) {
            MESSAGE(LOG_DUMP, "Warning, incoherency with purged ST%d cache\n", st);
        }
        #endif
        switch(neoncache_get_current_st(dyn, ninst, st)) {
            case NEON_CACHE_ST_D:
                VSTM_64_W(dyn->n.x87reg[j], s1);    // save the value
                break;
            case NEON_CACHE_ST_F:
                VCVT_F64_F32(0, dyn->n.x87reg[j]*2);
                VSTM_64_W(0, s1);    // save the value
                break;
        }
        if(!next) {
            fpu_free_reg_double(dyn, dyn->n.x87reg[j]);
            dyn->n.x87reg[j] = -1;
            dyn->n.x87cache[j] = -1;
            //dyn->n.stack_pop+=1;
        }
    }
    if(!next) {
        dyn->n.stack_next = 0;
        dyn->n.tags = 0;
        #if STEP < 2
        // refresh the cached valued, in case it's a purge outside a instruction
        dyn->insts[ninst].n.barrier = 1;
        dyn->n.pushed = 0;
        dyn->n.poped = 0;

        #endif
    }
    MESSAGE(LOG_DUMP, "\t---Purge Full x87 Cache and Synch Stackcount\n");
}

void x87_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    int ret = 0;
    int full = 1;
    for (int i=0; i<8; ++i)
        if(dyn->n.x87cache[i] != -1)
            ret = 1;
        else
            full = 0;
    if(!ret && !dyn->n.x87stack)    // nothing to do
        return;
    if(full) {
        x87_purgecache_full(dyn, ninst, next, s1, s2, s3);
        return;
    }
    MESSAGE(LOG_DUMP, "\tPurge %sx87 Cache and Synch Stackcount (%+d)---\n", next?"locally ":"", dyn->n.x87stack);
    int a = dyn->n.x87stack;
    if(a!=0) {
        // reset x87stack
        if(!next)
            dyn->n.x87stack = 0;
        // Add x87stack to emu fpu_stack
        LDR_IMM9(s2, xEmu, offsetof(x86emu_t, fpu_stack));
        if(a>0) {
            ADD_IMM8(s2, s2, a);
        } else {
            SUB_IMM8(s2, s2, -a);
        }
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, fpu_stack));
        // update tags
        LDRH_IMM8(s1, xEmu, offsetof(x86emu_t, fpu_tags));
        if(a>0) {
            MOV_REG_LSL_IMM5(s1, s1, a*2);
        } else {
            // empty tags
            MOV32(s2, 0xffff);
            ORR_REG_LSL_IMM5(s1, s1, s2, 16);
            MOV_REG_LSR_IMM5(s1, s1, -a*2);
        }
        STRH_IMM8(s1, xEmu, offsetof(x86emu_t, fpu_tags));
        // Sub x87stack to top, with and 7
        LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
        if(a>0) {
            SUB_IMM8(s2, s2, a);
        } else {
            ADD_IMM8(s2, s2, -a);
        }
        AND_IMM8(s2, s2, 7);
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    } else {
        LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    }
    // check if free is used
    if(dyn->n.tags) {
        LDRH_IMM8(s1, xEmu, offsetof(x86emu_t, fpu_tags));
        MOV32(s3, dyn->n.tags);
        ORR_REG_LSL_IMM5(s1, s1, s3, 0);
        STRH_IMM8(s1, xEmu, offsetof(x86emu_t, fpu_tags));
    }

    if(ret!=0) {
        // --- set values
        // Get top
        // loop all cache entries
        for (int i=0; i<8; ++i)
            if(dyn->n.x87cache[i]!=-1) {
                #if STEP == 1
                if(!next) {   // don't force promotion here
                    // pre-apply pop, because purge happens in-between
                    neoncache_promote_double(dyn, ninst, dyn->n.x87cache[i]+dyn->n.stack_pop);
                }
                #endif
                #if STEP == 3
                if(!next && neoncache_get_st_f(dyn, ninst, dyn->n.x87cache[i])>=0) {
                    MESSAGE(LOG_DUMP, "Warning, incoherency with purged ST%d cache\n", dyn->n.x87cache[i]);
                }
                #endif
                if(dyn->n.x87cache[i]) {
                    ADD_IMM8(s3, s2, dyn->n.x87cache[i]);
                    AND_IMM8(s3, s3, 7);    // (emu->top + st)&7
                    ADD_REG_LSL_IMM5(s3, xEmu, s3, 3);    // fpu[(emu->top+i)&7] lsl 3 because fpu are double, so 8 bytes
                } else {
                    ADD_REG_LSL_IMM5(s3, xEmu, s2, 3);    // fpu[(emu->top+i)&7] lsl 3 because fpu are double, so 8 bytes
                }
                if(next) {
                    // need to check if a ST_F need local promotion
                    if(neoncache_get_st_f(dyn, ninst, dyn->n.x87cache[i])>=0) {
                        VCVT_F64_F32(0, dyn->n.x87reg[i]*2);
                        VSTR_64(0, s3, offsetof(x86emu_t, x87));    // save the value
                    } else {
                        VSTR_64(dyn->n.x87reg[i], s3, offsetof(x86emu_t, x87));    // save the value
                    }
                } else {
                    VSTR_64(dyn->n.x87reg[i], s3, offsetof(x86emu_t, x87));    // save the value
                    fpu_free_reg_double(dyn, dyn->n.x87reg[i]);
                    dyn->n.x87reg[i] = -1;
                    dyn->n.x87cache[i] = -1;
                    //dyn->n.stack_pop+=1; //no pop, but the purge because of barrier will have the n.barrier flags set
                }
            }
    }
    if(!next) {
        dyn->n.stack_next = 0;
        dyn->n.tags = 0;
        #if STEP < 2
        // refresh the cached valued, in case it's a purge outside a instruction
        dyn->insts[ninst].n.barrier = 1;
        dyn->n.pushed = 0;
        dyn->n.poped = 0;

        #endif
    }
    MESSAGE(LOG_DUMP, "\t---Purge x87 Cache and Synch Stackcount\n");
}

static void x87_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    // Synch top & stack counter
    int a = dyn->n.x87stack;
    if(a) {
    // Add x87stack to emu fpu_stack
        LDR_IMM9(s2, xEmu, offsetof(x86emu_t, fpu_stack));
        if(a>0) {
            ADD_IMM8(s2, s2, a);
        } else {
            SUB_IMM8(s2, s2, -a);
        }
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, fpu_stack));
        // update tags
        LDRH_IMM8(s1, xEmu, offsetof(x86emu_t, fpu_tags));
        if(a>0) {
            MOV_REG_LSL_IMM5(s1, s1, a*2);
        } else {
            MOV32(s2, 0xffff);
            ORR_REG_LSL_IMM5(s1, s1, s2, 16);
            MOV_REG_LSR_IMM5(s1, s1, -a*2);
        }
        STRH_IMM8(s1, xEmu, offsetof(x86emu_t, fpu_tags));
        // Sub x87stack to top, with and 7
        LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
        if(a>0) {
            SUB_IMM8(s2, s2, a);
        } else {
            ADD_IMM8(s2, s2, -a);
        }
        AND_IMM8(s2, s2, 7);
        STR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    }
    int ret = 0;
    for (int i=0; (i<8) && (!ret); ++i)
        if(dyn->n.x87cache[i] != -1)
            ret = 1;
    if(!ret)    // nothing to do
        return;
    // Get top
    if(!a) {
        // already there
        LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    }
    // loop all cache entries
    for (int i=0; i<8; ++i)
        if(dyn->n.x87cache[i]!=-1) {
            if(dyn->n.x87cache[i]) {
                ADD_IMM8(s3, s2, dyn->n.x87cache[i]);
                AND_IMM8(s3, s3, 7);    // (emu->top + i)&7
                ADD_REG_LSL_IMM5(s3, xEmu, s3, 3);    // fpu[(emu->top+i)&7] lsl 3 because fpu are double, so 8 bytes
            } else {
                ADD_REG_LSL_IMM5(s3, xEmu, s2, 3);
            }
            if(ST_IS_F(dyn->n.x87cache[i])) {
                VCVT_F64_F32(0, dyn->n.x87reg[i]*2);    // use D0 as scratch...
                VSTR_64(0, s3, offsetof(x86emu_t, x87));
            } else {
                VSTR_64(dyn->n.x87reg[i], s3, offsetof(x86emu_t, x87));    // save the value
            }
        }
}

static void x87_unreflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    // go back with the top & stack counter
    int a = dyn->n.x87stack;
    if(a) {
    // Add x87stack to emu fpu_stack
        LDR_IMM9(s1, xEmu, offsetof(x86emu_t, fpu_stack));
        if(a>0) {
            SUB_IMM8(s1, s1, a);
        } else {
            ADD_IMM8(s1, s1, -a);
        }
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, fpu_stack));
        // update tags
        LDRH_IMM8(s1, xEmu, offsetof(x86emu_t, fpu_tags));
        if(a>0) {
            MOV32(s2, 0xffff);
            ORR_REG_LSL_IMM5(s1, s1, s2, 16);
            MOV_REG_LSR_IMM5(s1, s1, a*2);
        } else {
            MOV_REG_LSL_IMM5(s1, s1, -a*2);
        }
        STRH_IMM8(s1, xEmu, offsetof(x86emu_t, fpu_tags));
        // Sub x87stack to top, with and 7
        LDR_IMM9(s1, xEmu, offsetof(x86emu_t, top));
        if(a>0) {
            ADD_IMM8(s1, s1, a);
        } else {
            SUB_IMM8(s1, s1, -a);
        }
        AND_IMM8(s1, s1, 7);
        STR_IMM9(s1, xEmu, offsetof(x86emu_t, top));
    }
}

int x87_get_current_cache(dynarec_arm_t* dyn, int ninst, int st, int t)
{
    // search in cache first
    for (int i=0; i<8; ++i) {
        if(dyn->n.x87cache[i]==st) {
            #if STEP == 1
            if(t==NEON_CACHE_ST_D && (dyn->n.neoncache[dyn->n.x87reg[i]-FPUFIRST].t==NEON_CACHE_ST_F))
                neoncache_promote_double(dyn, ninst, st);
            #endif
            return i;
        }
        assert(dyn->n.x87cache[i]<8);
    }
    return -1;
}

int x87_get_cache(dynarec_arm_t* dyn, int ninst, int populate, int s1, int s2, int st, int t)
{
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    int ret = x87_get_current_cache(dyn, ninst, st, t);
    if(ret!=-1)
        return ret;
    MESSAGE(LOG_DUMP, "\tCreate %sx87 Cache for ST%d\n", populate?"and populate ":"", st);
    // get a free spot
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->n.x87cache[i]==-1)
            ret = i;
    // found, setup and grab the value
    dyn->n.x87cache[ret] = st;
    dyn->n.x87reg[ret] = fpu_get_reg_double(dyn, NEON_CACHE_ST_D, st);
    if(populate) {
        LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
        int a = st - dyn->n.x87stack;
        if(a) {
            if(a<0) {
                SUB_IMM8(s2, s2, -a);
            } else {
                ADD_IMM8(s2, s2, a);
            }
            AND_IMM8(s2, s2, 7);    // (emu->top + i)&7
        }
        ADD_REG_LSL_IMM5(s2, xEmu, s2, 3);
        VLDR_64(dyn->n.x87reg[ret], s2, offsetof(x86emu_t, x87));
    }
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);

    return ret;
}
int x87_get_neoncache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
    for(int ii=0; ii<24; ++ii)
        if((dyn->n.neoncache[ii].t == NEON_CACHE_ST_F || dyn->n.neoncache[ii].t == NEON_CACHE_ST_D)
         && dyn->n.neoncache[ii].n==st)
            return ii;
    assert(0);
    return -1;
}
int x87_get_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a, int t)
{
    int ret = dyn->n.x87reg[x87_get_cache(dyn, ninst, 1, s1, s2, a, t)];
    return ret * dyn->n.neoncache[ret-FPUFIRST].t;
}
int x87_get_st_empty(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a, int t)
{
    int ret = dyn->n.x87reg[x87_get_cache(dyn, ninst, 0, s1, s2, a, t)];
    return ret * dyn->n.neoncache[ret-FPUFIRST].t;
}


void x87_refresh(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
    x87_stackcount(dyn, ninst, s1);
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->n.x87cache[i] == st)
            ret = i;
    if(ret==-1)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tRefresh x87 Cache for ST%d\n", st);
    // prepare offset to fpu => s1
    ADD_IMM8(s1, xEmu, offsetof(x86emu_t, x87));
    // Get top
    LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    // Update
    if(st) {
        ADD_IMM8(s2, s2, st);
        AND_IMM8(s2, s2, 7);    // (emu->top + i)&7
    }
    ADD_REG_LSL_IMM5(s2, s1, s2, 3);    // fpu[(emu->top+i)&7] lsl 3 because fpu are double, so 8 bytes
    if(ST_IS_F(dyn->n.x87cache[ret])) {
        VCVT_F64_F32(0, dyn->n.x87reg[ret]*2);    // use D0 as scratch...
        VSTR_64(0, s2, 0);
    } else {
        VSTR_64(dyn->n.x87reg[ret], s2, 0);    // save the value
    }
    MESSAGE(LOG_DUMP, "\t--------x87 Cache for ST%d\n", st);
}

void x87_forget(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
    x87_stackcount(dyn, ninst, s1);
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->n.x87cache[i] == st)
            ret = i;
    if(ret==-1)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "\tForget x87 Cache for ST%d\n", st);
    int reg = dyn->n.x87reg[ret];
    #if STEP == 1
    if(dyn->n.neoncache[reg].t==NEON_CACHE_ST_F)
        neoncache_promote_double(dyn, ninst, st);
    #endif
    // prepare offset to fpu => s1
    ADD_IMM8(s1, xEmu, offsetof(x86emu_t, x87));
    // Get top
    LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    // Update
    if(st) {
        ADD_IMM8(s2, s2, st);
        AND_IMM8(s2, s2, 7);    // (emu->top + i)&7
    }
    ADD_REG_LSL_IMM5(s2, s1, s2, 3);    // fpu[(emu->top+i)&7] lsl 3 because fpu are double, so 8 bytes
    if(dyn->n.neoncache[reg].t==NEON_CACHE_ST_F) {
        // this should not happens
        VCVT_F64_F32(0, reg*2);    // use D0 as scratch...
        VSTR_64(0, s2, 0);
    } else {
        VSTR_64(reg, s2, 0);    // save the value
    }
    MESSAGE(LOG_DUMP, "\t--------Forget x87 Cache for ST%d\n", st);
    // and forget that cache
    fpu_free_reg_double(dyn, reg);
    dyn->n.neoncache[reg].v = 0;
    dyn->n.x87cache[ret] = -1;
    dyn->n.x87reg[ret] = -1;
}

void x87_reget_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    // search in cache first
    for (int i=0; i<8; ++i)
        if(dyn->n.x87cache[i]==st) {
            #if STEP == 1
            neoncache_promote_double(dyn, ninst, st);
            #endif
            // refresh the value
            MESSAGE(LOG_DUMP, "\tRefresh x87 Cache for ST%d\n", st);
            ADD_IMM8(s1, xEmu, offsetof(x86emu_t, x87));
            LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
            int a = st - dyn->n.x87stack;
            if(a<0) {
                SUB_IMM8(s2, s2, -a);
            } else {
                ADD_IMM8(s2, s2, a);
            }
            AND_IMM8(s2, s2, 7);    // (emu->top + i)&7
            ADD_REG_LSL_IMM5(s2, s1, s2, 3);
            VLDR_64(dyn->n.x87reg[i], s2, 0);
            MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
            // ok
            return;
        }
    // Was not in the cache? creating it....
    MESSAGE(LOG_DUMP, "\tCreate x87 Cache for ST%d\n", st);
    // get a free spot
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->n.x87cache[i]==-1)
            ret = i;
    // found, setup and grab the value
    dyn->n.x87cache[ret] = st;
    dyn->n.x87reg[ret] = fpu_get_reg_double(dyn, NEON_CACHE_ST_D, st);
    ADD_IMM8(s1, xEmu, offsetof(x86emu_t, x87));
    LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    int a = st - dyn->n.x87stack;
    if(a<0) {
        SUB_IMM8(s2, s2, -a);
    } else {
        ADD_IMM8(s2, s2, a);
    }
    AND_IMM8(s2, s2, 7);    // (emu->top + i)&7
    ADD_REG_LSL_IMM5(s2, s1, s2, 3);
    VLDR_64(dyn->n.x87reg[ret], s2, 0);
    MESSAGE(LOG_DUMP, "\t-------x87 Cache for ST%d\n", st);
}

void x87_free(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int st)
{
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->n.x87cache[i] == st)
            ret = i;
    MESSAGE(LOG_DUMP, "\tFFREE%s x87 Cache for ST%d\n", (ret!=-1)?" (and Forget)":"", st);
    if(ret!=-1) {
        const int reg = dyn->n.x87reg[ret];
        #if STEP == 1
        if(dyn->n.neoncache[reg].t==NEON_CACHE_ST_F)
            neoncache_promote_double(dyn, ninst, st);
        #endif
        // prepare offset to fpu => s1
        ADD_IMM8(s1, xEmu, offsetof(x86emu_t, x87));
        // Get top
        LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
        // Update
        int ast = st - dyn->n.x87stack;
        if(ast) {
            if(ast>0) {
                ADD_IMM8(s2, s2, ast);
            } else {
                SUB_IMM8(s2, s2, -ast);
            }
            AND_IMM8(s2, s2, 7); // (emu->top + i)&7
        }
        ADD_REG_LSL_IMM5(s2, s1, s2, 3);    // fpu[(emu->top+i)&7] lsl 3 because fpu are double, so 8 bytes
        if(dyn->n.neoncache[reg].t==NEON_CACHE_ST_F) {
            // this should not happens
            VCVT_F64_F32(0, reg*2);    // use D0 as scratch...
            VSTR_64(0, s2, 0);
        } else {
            VSTR_64(reg, s2, 0);    // save the value
        }
        // and forget that cache
        fpu_free_reg_double(dyn, reg);
        dyn->n.neoncache[reg].v = 0;
        dyn->n.x87cache[ret] = -1;
        dyn->n.x87reg[ret] = -1;
    } else {
        // Get top
        LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
        // Update
        int ast = st - dyn->n.x87stack;
        if(ast) {
            if(ast>0) {
                ADD_IMM8(s2, s2, ast);
            } else {
                SUB_IMM8(s2, s2, -ast);
            }
            AND_IMM8(s2, s2, 7); // (emu->top + i)&7
        }
    }
    // add mark in the freed array
    dyn->n.tags |= 0b11<<(st*2);
    MESSAGE(LOG_DUMP, "\t--------x87 FFREE for ST%d\n", st);
}

// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    LDRH_IMM8(s1, xEmu, offsetof(x86emu_t, cw));    // hopefully cw is not too far for an imm8
    UBFX(s1, s1, 10, 2);    // extract round...
    UBFX(s2, s1, 1, 1);     // swap bits 0 and 1
    BFI(s2, s1, 1, 1);
    VMRS(s1);               // get fpscr
    if (s3 >= 0)
        MOV_REG(s3, s1);
    BFI(s1, s2, 22, 2);     // inject new round
    VMSR(s1);               // put new fpscr
    return s3;
}

// Set rounding according to cw flags, also enable exception and reset counter, return reg to restore flags
int x87_setround_reset(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    LDRH_IMM8(s1, xEmu, offsetof(x86emu_t, cw));    // hopefully cw is not too far for an imm8
    UBFX(s1, s1, 10, 2);    // extract round...
    UBFX(s2, s1, 1, 1);     // swap bits 0 and 1
    BFI(s2, s1, 1, 1);
    VMRS(s3);               // get fpscr
    ORR_IMM8(s1, s3, 0b010, 9); // enable exceptions
    BIC_IMM8(s1, s1, 0b10011111, 0);    // reset counters
    BFI(s1, s2, 22, 2);     // inject new round
    VMSR(s1);               // put new fpscr
    return s3;
}

void x87_swapreg(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a, int b)
{
    int i1, i2, i3;
    i1 = x87_get_cache(dyn, ninst, 1, s1, s2, b, X87_ST(b));
    i2 = x87_get_cache(dyn, ninst, 1, s1, s2, a, X87_ST(a));
    i3 = dyn->n.x87cache[i1];
    dyn->n.x87cache[i1] = dyn->n.x87cache[i2];
    dyn->n.x87cache[i2] = i3;
    // swap those too
    int j1, j2, j3;
    j1 = x87_get_neoncache(dyn, ninst, s1, s2, b);
    j2 = x87_get_neoncache(dyn, ninst, s1, s2, a);
    j3 = dyn->n.neoncache[j1].n;
    dyn->n.neoncache[j1].n = dyn->n.neoncache[j2].n;
    dyn->n.neoncache[j2].n = j3;
    // mark as swapped
    dyn->n.swapped = 1;
    dyn->n.combined1= a; dyn->n.combined2=b;
}

// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    LDR_IMM9(s1, xEmu, offsetof(x86emu_t, mxcsr));
    UBFX(s1, s1, 13, 2);    // extract round...
    UBFX(s2, s1, 1, 1);     // swap bits 0 and 1
    BFI(s2, s1, 1, 1);
    VMRS(s1);               // get fpscr
    MOV_REG(s3, s1);
    BFI(s1, s2, 22, 2);     // inject new round
    VMSR(s1);               // put new fpscr
    return s3;
}

// Set rounding according to mxcsr flags, also enable exception and reset counter, return reg to restore flags
int sse_setround_reset(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    LDR_IMM9(s1, xEmu, offsetof(x86emu_t, mxcsr));
    UBFX(s1, s1, 13, 2);    // extract round...
    UBFX(s2, s1, 1, 1);     // swap bits 0 and 1
    BFI(s2, s1, 1, 1);
    VMRS(s3);               // get fpscr
    ORR_IMM8(s1, s3, 0b010, 9); // enable exceptions
    BIC_IMM8(s1, s1, 0b10011111, 0);    // reset counters
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
static int isx87Empty(dynarec_arm_t* dyn)
{
    for (int i=0; i<8; ++i)
        if(dyn->n.x87cache[i] != -1)
            return 0;
    return 1;
}
// get neon register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    if(!dyn->n.x87stack && isx87Empty(dyn))
        x87_purgecache(dyn, ninst, 0, s1, s2, s3);
    if(dyn->n.mmxcache[a]!=-1)
        return dyn->n.mmxcache[a];
    ++dyn->n.mmxcount;
    int ret = dyn->n.mmxcache[a] = fpu_get_reg_double(dyn, NEON_CACHE_MM, a);
    ADD_IMM8(s1, xEmu, offsetof(x86emu_t, mmx[a]));
    VLD1_32(ret, s1);
    return ret;
}
// get neon register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int a)
{
    if(!dyn->n.x87stack && isx87Empty(dyn))
        x87_purgecache(dyn, ninst, 0, s1, s2, s3);
    if(dyn->n.mmxcache[a]!=-1)
        return dyn->n.mmxcache[a];
    ++dyn->n.mmxcount;
    int ret = dyn->n.mmxcache[a] = fpu_get_reg_double(dyn, NEON_CACHE_MM, a);
    return ret;
}
// purge the MMX cache only(needs 3 scratch registers)
void mmx_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1)
{
    if(!dyn->n.mmxcount)
        return;
    if(!next)
        dyn->n.mmxcount = 0;
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->n.mmxcache[i]!=-1) {
            if (old==-1) {
                MESSAGE(LOG_DUMP, "\tPurge %sMMX Cache ------\n", next?"locally ":"");
                ADD_IMM8(s1, xEmu, offsetof(x86emu_t, mmx[i]));
                old = i+1;  //+1 because VST1 with write back
            } else {
                if(old!=i) {
                    ADD_IMM8(s1, s1, (i-old)*8);
                }
                old = i+1;
            }
            VST1_32_W(dyn->n.mmxcache[i], s1);
            if(!next) {
                fpu_free_reg_double(dyn, dyn->n.mmxcache[i]);
                dyn->n.mmxcache[i] = -1;
            }
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge MMX Cache\n");
    }
}
static void mmx_reflectcache(dynarec_arm_t* dyn, int ninst, int s1)
{
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->n.mmxcache[i]!=-1) {
            if (old==-1) {
                ADD_IMM8(s1, xEmu, offsetof(x86emu_t, mmx[i]));
                old = i+1;
            } else {
                if(old!=i) {
                    ADD_IMM8(s1, s1, (i-old)*8);
                }
                old = i+1;
            }
            VST1_32_W(dyn->n.mmxcache[i], s1);
        }
}


// SSE / SSE2 helpers
// get neon register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a, int forwrite)
{
    if(dyn->n.ssecache[a].v!=-1) {
        if(forwrite) {
            dyn->n.ssecache[a].write = 1;    // update only if forwrite
            dyn->n.neoncache[dyn->n.ssecache[a].reg+0-FPUFIRST].t = NEON_CACHE_XMMW;
            dyn->n.neoncache[dyn->n.ssecache[a].reg+1-FPUFIRST].t = NEON_CACHE_XMMW;
        }
        return dyn->n.ssecache[a].reg;
    }
    dyn->n.ssecache[a].reg = fpu_get_reg_quad(dyn, forwrite?NEON_CACHE_XMMW:NEON_CACHE_XMMR, a);
    int ret =  dyn->n.ssecache[a].reg;
    dyn->n.ssecache[a].write = forwrite;
    int offs = offsetof(x86emu_t, xmm[a]);
    if(!(offs&3) && (offs>>2)<256) {
        ADD_IMM8_ROR(s1, xEmu, (offs>>2), 0xf);
    } else {
        MOV32(s1, offs);
        ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
    }
    VLD1Q_32(ret, s1);
    return ret;
}
// get neon register for a SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a)
{
    if(dyn->n.ssecache[a].v!=-1) {
        dyn->n.ssecache[a].write = 1;
        dyn->n.neoncache[dyn->n.ssecache[a].reg+0-FPUFIRST].t = NEON_CACHE_XMMW;
        dyn->n.neoncache[dyn->n.ssecache[a].reg+1-FPUFIRST].t = NEON_CACHE_XMMW;
        return dyn->n.ssecache[a].reg;
    }
    dyn->n.ssecache[a].reg = fpu_get_reg_quad(dyn, NEON_CACHE_XMMW, a);
    dyn->n.ssecache[a].write = 1; // it will be write...
    return dyn->n.ssecache[a].reg;
}
// forget neon register for a SSE reg
void sse_forget_reg(dynarec_arm_t* dyn, int ninst, int a, int s1)
{
    if(dyn->n.ssecache[a].v==-1)
        return;
    if(dyn->n.neoncache[dyn->n.ssecache[a].reg+0-FPUFIRST].t == NEON_CACHE_XMMW) {
        int offs = offsetof(x86emu_t, xmm[a]);
        if(!(offs&3) && (offs>>2)<256) {
            ADD_IMM8_ROR(s1, xEmu, offs>>2, 15);
        } else {
            MOV32(s1, offs);
            ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
        }
        VST1Q_64(dyn->n.ssecache[a].reg, s1);
    }
    fpu_free_reg_quad(dyn, dyn->n.ssecache[a].reg);
    dyn->n.ssecache[a].v = -1;
    return;
}

// purge the SSE cache only(needs 1 scratch registers)
static void sse_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1)
{
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->n.ssecache[i].v!=-1) {
            if(dyn->n.ssecache[i].write) {
                if (old==-1) {
                    MESSAGE(LOG_DUMP, "\tPurge %sSSE Cache ------\n", next?"locally ":"");
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
                VST1Q_32_W(dyn->n.ssecache[i].reg, s1);
            }
            if(!next) {
                fpu_free_reg_quad(dyn, dyn->n.ssecache[i].reg);
                dyn->n.ssecache[i].v = -1;
            }
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "\t------ Purge SSE Cache\n");
    }
}
static void sse_reflectcache(dynarec_arm_t* dyn, int ninst, int s1)
{
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->n.ssecache[i].v!=-1 && dyn->n.ssecache[i].write) {
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
            VST1Q_32_W(dyn->n.ssecache[i].reg, s1);
        }
}

int sse_reflect_reg(dynarec_arm_t* dyn, int ninst, int a, int s1)
{
    if(dyn->n.ssecache[a].v==-1)
        return 0;
    if(dyn->n.ssecache[a].write) {
        int offs = offsetof(x86emu_t, xmm[a]);
        if(!(offs&3) && (offs>>2)<256) {
            ADD_IMM8_ROR(s1, xEmu, offs>>2, 15);
        } else {
            MOV32(s1, offs);
            ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
        }
        VST1Q_32(dyn->n.ssecache[a].reg, s1);
        return 1;
    }
    return 0;
}

void fpu_pushcache(dynarec_arm_t* dyn, int ninst, int s1)
{
    // only need to push 16-31...
    int n=0;
    for (int i=16; i<32; i++)
        if(dyn->n.fpuused[i-8])
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
        if(dyn->n.fpuused[i-8]) {
            VST1_64_W(i, s1);
        }
    }
    MESSAGE(LOG_DUMP, "\t------- Push FPU Cache (%d)\n", n);
}

void fpu_popcache(dynarec_arm_t* dyn, int ninst, int s1)
{
    // only need to push 16-31...
    int n=0;
    for (int i=16; i<32; i++)
        if(dyn->n.fpuused[i-8])
            ++n;
    if(!n)
        return;
    MESSAGE(LOG_DUMP, "\tPop FPU Cache (%d)------\n", n);
    MOV_REG(s1, xSP);
    for (int i=16; i<32; ++i) {
        if(dyn->n.fpuused[i-8]) {
            VLD1_64_W(i, s1);
        }
    }
    if(n>=8) {
        MOVW(s1, n*8);
        ADD_REG_LSL_IMM5(xSP, xSP, s1, 0);
    } else {
        ADD_IMM8(xSP, xSP, n*8);
    }
    MESSAGE(LOG_DUMP, "\t------- Pop FPU Cache (%d)\n", n);
}

void fpu_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    x87_purgecache(dyn, ninst, next, s1, s2, s3);
    mmx_purgecache(dyn, ninst, next, s1);
    sse_purgecache(dyn, ninst, next, s1);
    if(!next)
        fpu_reset_reg(dyn);
}

static int findCacheSlot(dynarec_arm_t* dyn, int ninst, int t, int n, neoncache_t* cache)
{
    neon_cache_t f;
    f.n = n; f.t = t;
    for(int i=0; i<24; ++i) {
        if(cache->neoncache[i].v == f.v)
            return i;
        if(cache->neoncache[i].n == n) {
            switch(cache->neoncache[i].t) {
                case NEON_CACHE_ST_F:
                    if (t==NEON_CACHE_ST_D)
                        return i;
                    break;
                case NEON_CACHE_ST_D:
                    if (t==NEON_CACHE_ST_F)
                        return i;
                    break;
                case NEON_CACHE_XMMR:
                    if(t==NEON_CACHE_XMMW)
                        return i;
                    break;
                case NEON_CACHE_XMMW:
                    if(t==NEON_CACHE_XMMR)
                        return i;
                    break;
            }
        }
    }
    return -1;
}

static void swapCache(dynarec_arm_t* dyn, int ninst, int i, int j, neoncache_t *cache)
{
    if (i==j)
        return;
    int quad = 0;
    if(cache->neoncache[i].t==NEON_CACHE_XMMR || cache->neoncache[i].t==NEON_CACHE_XMMW)
        quad =1;
    if(cache->neoncache[j].t==NEON_CACHE_XMMR || cache->neoncache[j].t==NEON_CACHE_XMMW)
        quad =1;
    if(quad) {    // if quad, we need to swap the whole quad!
        i&=~1;
        j&=~1;
    }
    if(!cache->neoncache[i].v && ((quad && !cache->neoncache[i+1].v) || !quad)) {
        // a mov is enough, no need to swap
        MESSAGE(LOG_DUMP, "\t  - Moving %d <- %d\n", i, j);
        if(quad) {
            VMOVQ(i+FPUFIRST, j+FPUFIRST);
            cache->neoncache[i+1].v = cache->neoncache[j+1].v;
            cache->neoncache[j+1].v = 0;
        } else {
            VMOV_64(i+FPUFIRST, j+FPUFIRST);
        }
        cache->neoncache[i].v = cache->neoncache[j].v;
        cache->neoncache[j].v = 0;
        return;
    }
    // SWAP
    neon_cache_t tmp;
    MESSAGE(LOG_DUMP, "\t  - Swaping %d <-> %d\n", i, j);
    if(quad) {
        VSWPQ(i+FPUFIRST, j+FPUFIRST);
        tmp.v = cache->neoncache[i+1].v;
        cache->neoncache[i+1].v = cache->neoncache[j+1].v;
        cache->neoncache[j+1].v = tmp.v;
    } else {
        VSWP(i+FPUFIRST, j+FPUFIRST);
    }
    tmp.v = cache->neoncache[i].v;
    cache->neoncache[i].v = cache->neoncache[j].v;
    cache->neoncache[j].v = tmp.v;
}

static void loadCache(dynarec_arm_t* dyn, int ninst, int stack_cnt, int s1, int s2, int s3, int* s1_val, int* s2_val, int* s3_top, neoncache_t *cache, int i, int t, int n)
{
    if(cache->neoncache[i].v) {
        int quad = 0;
        if(t==NEON_CACHE_XMMR || t==NEON_CACHE_XMMW)
            quad = 1;
        if(cache->neoncache[i].t==NEON_CACHE_XMMR || cache->neoncache[i].t==NEON_CACHE_XMMW)
            quad = 1;
        if(quad)    // if quad, we need to move away the whole quad!
            i&=~1;
        int j = i+1+quad;
        if(quad) {
            while(cache->neoncache[j].v && cache->neoncache[j+1].v)
                j+=2;
            MESSAGE(LOG_DUMP, "\t  - Moving away %d/%d\n", i, i+1);
            VMOVQ(j+FPUFIRST, i+FPUFIRST);
            cache->neoncache[j+1].v = cache->neoncache[i+1].v;
            cache->neoncache[i+1].v = 0;    // in case a Q was moved for a D
        } else {
            while(cache->neoncache[j].v)
                ++j;
            MESSAGE(LOG_DUMP, "\t  - Moving away %d\n", i);
            VMOVD(j+FPUFIRST, i+FPUFIRST);
        }
        cache->neoncache[j].v = cache->neoncache[i].v;
    }
    switch(t) {
        case NEON_CACHE_XMMR:
        case NEON_CACHE_XMMW:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));
            int offs = offsetof(x86emu_t, xmm[n]);
            if(*s1_val != offs) {
                if(!(offs&3) && (offs>>2)<256) {
                    ADD_IMM8_ROR(s1, xEmu, (offs>>2), 0xf);
                } else {
                    MOV32(s1, offs);
                    ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
                }
            }
            VLD1Q_32_W(i+FPUFIRST, s1);
            *s1_val = offs + sizeof(sse_regs_t);
            cache->neoncache[i+1].n = n;    // Quad...
            cache->neoncache[i+1].t = t;
            break;
        case NEON_CACHE_MM:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));                    
            if(*s2_val!=offsetof(x86emu_t, mmx[n])) {
                ADD_IMM8(s2, xEmu, offsetof(x86emu_t, mmx[n]));
            }
            VLD1_32_W(i+FPUFIRST, s2);
            *s2_val = offsetof(x86emu_t, mmx[n]) + sizeof(mmx87_regs_t);
            break;
        case NEON_CACHE_ST_D:
        case NEON_CACHE_ST_F:
            MESSAGE(LOG_DUMP, "\t  - Loading %s\n", getCacheName(t, n));                    
            if((*s3_top) == 0xffff) {
                LDR_IMM9(s3, xEmu, offsetof(x86emu_t, top));
                *s3_top = 0;
            }
            int a = n  - (*s3_top) - stack_cnt;
            if(a) {
                if(a<0) {
                    SUB_IMM8(s3, s3, -a);
                } else {
                    ADD_IMM8(s3, s3, a);
                }
                AND_IMM8(s3, s3, 7);    // (emu->top + i)&7
            }
            *s3_top += a;
            *s2_val = 0;
            ADD_REG_LSL_IMM5(s2, xEmu, s3, 3);
            VLDR_64(i+FPUFIRST, s2, offsetof(x86emu_t, x87));
            if(t==NEON_CACHE_ST_F) {
                VCVT_F32_F64((i+FPUFIRST)*2, i+FPUFIRST);
            }
            break;                    
        case NEON_CACHE_NONE:
        case NEON_CACHE_SCR:
        default:    /* nothing done */
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break; 
    }
    cache->neoncache[i].n = n;
    cache->neoncache[i].t = t;
}

static void unloadCache(dynarec_arm_t* dyn, int ninst, int stack_cnt, int s1, int s2, int s3, int* s1_val, int* s2_val, int* s3_top, neoncache_t *cache, int i, int t, int n)
{
    switch(t) {
        case NEON_CACHE_XMMR:
            i&=~1;
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            cache->neoncache[i+1].v = 0;    // Quad...
            break;
        case NEON_CACHE_XMMW:
            i&=~1;
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));
            int offs = offsetof(x86emu_t, xmm[n]);
            if(*s1_val!=offs) {
                if(!(offs&3) && (offs>>2)<256) {
                    ADD_IMM8_ROR(s1, xEmu, (offs>>2), 0xf);
                } else {
                    MOV32(s1, offs);
                    ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
                }
            }
            VST1Q_32_W(i+FPUFIRST, s1);
            *s1_val = offs+sizeof(sse_regs_t);
            cache->neoncache[i+1].v = 0;    // Quad...
            break;
        case NEON_CACHE_MM:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));                    
            if(*s2_val != offsetof(x86emu_t, mmx[n])) {
                ADD_IMM8(s2, xEmu, offsetof(x86emu_t, mmx[n]));
            }
            VST1_32_W(i+FPUFIRST, s2);
            *s2_val = offsetof(x86emu_t, mmx[n])+sizeof(mmx87_regs_t);
            break;
        case NEON_CACHE_ST_D:
        case NEON_CACHE_ST_F:
            MESSAGE(LOG_DUMP, "\t  - Unloading %s\n", getCacheName(t, n));                    
            if((*s3_top)==0xffff) {
                LDR_IMM9(s3, xEmu, offsetof(x86emu_t, top));
                *s3_top = 0;
            }
            int a = n - (*s3_top) - stack_cnt;
            if(a) {
                if(a<0) {
                    SUB_IMM8(s3, s3, -a);
                } else {
                    ADD_IMM8(s3, s3, a);
                }
                AND_IMM8(s3, s3, 7);    // (emu->top + i)&7
            }
            *s3_top += a;
            ADD_REG_LSL_IMM5(s2, xEmu, s3, 3);
            *s2_val = 0;
            if(t==NEON_CACHE_ST_F) {
                VCVT_F64_F32(i+FPUFIRST, (i+FPUFIRST)*2);
            }
            VSTR_64(i+FPUFIRST, s2, offsetof(x86emu_t, x87));
            break;                    
        case NEON_CACHE_NONE:
        case NEON_CACHE_SCR:
        default:    /* nothing done */
            MESSAGE(LOG_DUMP, "\t  - ignoring %s\n", getCacheName(t, n));
            break; 
    }
    cache->neoncache[i].v = 0;
}

static void fpuCacheTransform(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
#if STEP > 1
    int i2 = dyn->insts[ninst].x86.jmp_insts;
    if(i2<0)
        return;
    MESSAGE(LOG_DUMP, "\tCache Transform ---- ninst=%d -> %d\n", ninst, i2);
    if((!i2) || (dyn->insts[i2].x86.barrier&BARRIER_FLOAT)) {
        if(dyn->n.stack_next)  {
            fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
            MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
            return;
        }
        for(int i=0; i<24; ++i)
            if(dyn->n.neoncache[i].v) {       // there is something at ninst for i
                fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
                MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
                return;
            }
        MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
        return;
    }
    neoncache_t cache_i2 = dyn->insts[i2].n;
    neoncacheUnwind(&cache_i2);

    if(!cache_i2.stack) {
        int purge = 1;
        for (int i=0; i<24 && purge; ++i)
            if(cache_i2.neoncache[i].v)
                purge = 0;
        if(purge) {
            fpu_purgecache(dyn, ninst, 1, s1, s2, s3);
            MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
            return;
        }
    }
    int stack_cnt = dyn->n.stack_next;
    int s3_top = 0xffff;
    neoncache_t cache = dyn->n;
    // sanitize XMM regs alocation, cache_i2 has been sanitized in neoncacheUnwind already
    for(int i=0; i<24; i+=2) {
        if(cache.neoncache[i].t == NEON_CACHE_XMMR || cache.neoncache[i].t == NEON_CACHE_XMMW)
            cache.neoncache[i+1] = cache.neoncache[i];
    }
    int s1_val = 0;
    int s2_val = 0;
    // unload every uneeded cache
    // check SSE first, than MMX, in order, for optimisation issue
    for(int i=0; i<8; ++i) {
        int j=findCacheSlot(dyn, ninst, NEON_CACHE_XMMW, i, &cache);
        if(j>=0 && findCacheSlot(dyn, ninst, NEON_CACHE_XMMW, i, &cache_i2)==-1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.neoncache[j].t, cache.neoncache[j].n);
    }
    for(int i=0; i<8; ++i) {
        int j=findCacheSlot(dyn, ninst, NEON_CACHE_MM, i, &cache);
        if(j>=0 && findCacheSlot(dyn, ninst, NEON_CACHE_MM, i, &cache_i2)==-1)
            unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, j, cache.neoncache[j].t, cache.neoncache[j].n);
    }
    for(int i=0; i<24; ++i) {
        if(cache.neoncache[i].v)
            if(findCacheSlot(dyn, ninst, cache.neoncache[i].t, cache.neoncache[i].n, &cache_i2)==-1)
                unloadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, i, cache.neoncache[i].t, cache.neoncache[i].n);
    }
    // and now load/swap the missing one
    for(int i=0; i<24; ++i) {
        if(cache_i2.neoncache[i].v) {
            if(cache_i2.neoncache[i].v != cache.neoncache[i].v) {
                int j;
                if((j=findCacheSlot(dyn, ninst, cache_i2.neoncache[i].t, cache_i2.neoncache[i].n, &cache))==-1)
                    loadCache(dyn, ninst, stack_cnt, s1, s2, s3, &s1_val, &s2_val, &s3_top, &cache, i, cache_i2.neoncache[i].t, cache_i2.neoncache[i].n);
                else {
                    // it's here, lets swap if needed
                    if(j!=i)
                        swapCache(dyn, ninst, i, j, &cache);
                }
            }
            if(cache.neoncache[i].t != cache_i2.neoncache[i].t) {
                if(cache.neoncache[i].t == NEON_CACHE_ST_D && cache_i2.neoncache[i].t == NEON_CACHE_ST_F) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    VCVT_F32_F64((i+8)*2, (i+8));
                    cache.neoncache[i].t = NEON_CACHE_ST_F;
                } else if(cache.neoncache[i].t == NEON_CACHE_ST_F && cache_i2.neoncache[i].t == NEON_CACHE_ST_D) {
                    MESSAGE(LOG_DUMP, "\t  - Convert %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    VCVT_F64_F32((i+8), (i+8)*2);
                    cache.neoncache[i].t = NEON_CACHE_ST_D;
                } else if(cache.neoncache[i].t == NEON_CACHE_XMMR && cache_i2.neoncache[i].t == NEON_CACHE_XMMW)
                    { cache.neoncache[i].t = cache.neoncache[i+1].t = NEON_CACHE_XMMW; }
                else if(cache.neoncache[i].t == NEON_CACHE_XMMW && cache_i2.neoncache[i].t == NEON_CACHE_XMMR) {
                    // refresh cache...
                    MESSAGE(LOG_DUMP, "\t  - Refreh %s\n", getCacheName(cache.neoncache[i].t, cache.neoncache[i].n));
                    int offs = offsetof(x86emu_t, xmm[cache.neoncache[i].n]);
                    if(s1_val != offs) {
                        if(!(offs&3) && (offs>>2)<256) {
                            ADD_IMM8_ROR(s1, xEmu, (offs>>2), 0xf);
                        } else {
                            MOV32(s1, offs);
                            ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
                        }
                    }
                    VST1Q_32_W(i+FPUFIRST, s1);
                    s1_val = offs + sizeof(sse_regs_t);
                    cache.neoncache[i].t = cache.neoncache[i+1].t = NEON_CACHE_XMMR;
                }
            }
        }
    }
    if(stack_cnt != cache_i2.stack) {
        MESSAGE(LOG_DUMP, "\t    - adjust stack count %d -> %d -\n", stack_cnt, cache_i2.stack);
        int a = stack_cnt - cache_i2.stack;
        // Add x87stack to emu fpu_stack
        LDR_IMM9(s3, xEmu, offsetof(x86emu_t, fpu_stack));
        if(a>0) {
            ADD_IMM8(s3, s3, a);
        } else {
            SUB_IMM8(s3, s3, -a);
        }
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, fpu_stack));
        // update tags
        LDRH_IMM8(s1, xEmu, offsetof(x86emu_t, fpu_tags));
        if(a>0) {
            MOV_REG_LSL_IMM5(s1, s1, a*2);
        } else {
            // empty tags
            MOV32(s3, 0xffff);
            ORR_REG_LSL_IMM5(s1, s1, s3, 16);
            MOV_REG_LSR_IMM5(s1, s1, -a*2);
        }
        STRH_IMM8(s1, xEmu, offsetof(x86emu_t, fpu_tags));
        // Sub x87stack to top, with and 7
        LDR_IMM9(s3, xEmu, offsetof(x86emu_t, top));
        if(a>0) {
            SUB_IMM8(s3, s3, a);
        } else {
            ADD_IMM8(s3, s3, -a);
        }
        AND_IMM8(s3, s3, 7);
        STR_IMM9(s3, xEmu, offsetof(x86emu_t, top));
        s3_top = 0;
        stack_cnt = cache_i2.stack;
    }
    MESSAGE(LOG_DUMP, "\t---- Cache Transform\n");
#endif
}
static void flagsCacheTransform(dynarec_arm_t* dyn, int ninst, int s1)
{
#if STEP > 1
    int j32;
    int jmp = dyn->insts[ninst].x86.jmp_insts;
    if(jmp<0)
        return;
    if(dyn->f.dfnone)  // flags are fully known, nothing we can do more
        return;
    MESSAGE(LOG_DUMP, "\tFlags fetch ---- ninst=%d -> %d\n", ninst, jmp);
    int go = (dyn->insts[jmp].f_entry.dfnone && !dyn->f.dfnone)?1:0;
    switch (dyn->insts[jmp].f_entry.pending) {
        case SF_UNKNOWN: break;
        case SF_SET: 
            if(dyn->f.pending!=SF_SET && dyn->f.pending!=SF_SET_PENDING) 
                go = 1; 
            break;
        case SF_SET_PENDING:
            if(dyn->f.pending!=SF_SET 
            && dyn->f.pending!=SF_SET_PENDING
            && dyn->f.pending!=SF_PENDING) 
                go = 1; 
            break;
        case SF_PENDING:
            if(dyn->f.pending!=SF_SET 
            && dyn->f.pending!=SF_SET_PENDING
            && dyn->f.pending!=SF_PENDING)
                go = 1;
            else if(dyn->insts[jmp].f_entry.dfnone!=dyn->f.dfnone)
                go = 1;
            break;
    }
    if(go) {
        if(dyn->f.pending!=SF_PENDING) {
            LDR_IMM9(s1, xEmu, offsetof(x86emu_t, df));
            TSTS_REG_LSL_IMM5(s1, s1, 0);
            j32 = (GETMARKF2)-(dyn->arm_size+8);
            Bcond(cEQ, j32);
        }
        CALL_(UpdateFlags, -1, 0);
        MARKF2;
    }
#endif
}

void CacheTransform(dynarec_arm_t* dyn, int ninst, int cacheupd, int s1, int s2, int s3) {
    if(cacheupd&1)
        fpuCacheTransform(dyn, ninst, s1, s2, s3);
    if(cacheupd&2)
        flagsCacheTransform(dyn, ninst, s1);
}

void fpu_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    x87_reflectcache(dyn, ninst, s1, s2, s3);
    mmx_reflectcache(dyn, ninst, s1);
    sse_reflectcache(dyn, ninst, s1);
}

void fpu_unreflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    x87_unreflectcache(dyn, ninst, s1, s2, s3);
}

// get the single reg that from the double "reg" (so Dx[idx])
int fpu_get_single_reg(dynarec_arm_t* dyn, int ninst, int reg, int idx)
{
    if(reg<16)
        return reg*2+idx;
    int a = fpu_get_scratch_double(dyn);
    VMOV_64(a, reg);
    return a*2+idx;
}
// put back (if needed) the single reg in place
void fpu_putback_single_reg(dynarec_arm_t* dyn, int ninst, int reg, int idx, int s)
{
    if(reg>=16) {
        VMOV_64(reg, s/2);
    }
}

void emit_pf(dynarec_arm_t* dyn, int ninst, int s1, int s4)
{
    // by xor'ing all the bit 2 by two with a shift, pair of bits are removed, and only 1 is left if bit number if odd
    XOR_REG_LSR_IMM8(s4, s1, s1, 4);
    XOR_REG_LSR_IMM8(s4, s4, s4, 2);
    XOR_REG_LSR_IMM8(s4, s4, s4, 1);
    MVN_REG_LSL_IMM5(s4, s4, 0);
    BFI(xFlags, s4, F_PF, 1);
}

void fpu_reset_cache(dynarec_arm_t* dyn, int ninst, int reset_n)
{
    MESSAGE(LOG_DEBUG, "Reset Caches with %d\n",reset_n);
    #if STEP > 1
    // for STEP 2 & 3, just need to refrest with current, and undo the changes (push & swap)
    dyn->n = dyn->insts[ninst].n;
    #else
    dyn->n = dyn->insts[reset_n].n;
    #endif
    neoncacheUnwind(&dyn->n);
    #if STEP == 0
    if(box86_dynarec_dump) dynarec_log(LOG_NONE, "New x87stack=%d\n", dyn->n.x87stack);
        #endif
    #if defined(HAVE_TRACE) && (STEP>2)
    if(box86_dynarec_dump)
        if(memcmp(&dyn->n, &dyn->insts[reset_n].n, sizeof(neon_cache_t))) {
            MESSAGE(LOG_DEBUG, "Warning, difference in neoncache: reset=");
            for(int i=0; i<24; ++i)
                if(dyn->insts[reset_n].n.neoncache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->insts[reset_n].n.neoncache[i].t, dyn->insts[reset_n].n.neoncache[i].n));
            if(dyn->insts[reset_n].n.combined1 || dyn->insts[reset_n].n.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->insts[reset_n].n.swapped?"SWP":"CMB", dyn->insts[reset_n].n.combined1, dyn->insts[reset_n].n.combined2);
            if(dyn->insts[reset_n].n.stack_push || dyn->insts[reset_n].n.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->insts[reset_n].n.stack_push, -dyn->insts[reset_n].n.stack_pop);
            MESSAGE(LOG_DEBUG, " ==> ");
            for(int i=0; i<24; ++i)
                if(dyn->insts[ninst].n.neoncache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->insts[ninst].n.neoncache[i].t, dyn->insts[ninst].n.neoncache[i].n));
            if(dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->insts[ninst].n.swapped?"SWP":"CMB", dyn->insts[ninst].n.combined1, dyn->insts[ninst].n.combined2);
            if(dyn->insts[ninst].n.stack_push || dyn->insts[ninst].n.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->insts[ninst].n.stack_push, -dyn->insts[ninst].n.stack_pop);
            MESSAGE(LOG_DEBUG, " -> ");
            for(int i=0; i<24; ++i)
                if(dyn->n.neoncache[i].v)
                    MESSAGE(LOG_DEBUG, " %02d:%s", i, getCacheName(dyn->n.neoncache[i].t, dyn->n.neoncache[i].n));
            if(dyn->n.combined1 || dyn->n.combined2)
                MESSAGE(LOG_DEBUG, " %s:%02d/%02d", dyn->n.swapped?"SWP":"CMB", dyn->n.combined1, dyn->n.combined2);
            if(dyn->n.stack_push || dyn->n.stack_pop)
                MESSAGE(LOG_DEBUG, " (%d:%d)", dyn->n.stack_push, -dyn->n.stack_pop);
            MESSAGE(LOG_DEBUG, "\n");
        }
    #endif //HAVE_TRACE
}

// propagate ST stack state, especial stack pop that are deferred
void fpu_propagate_stack(dynarec_arm_t* dyn, int ninst)
{
    if(dyn->n.stack_pop) {
        for(int j=0; j<24; ++j)
            if((dyn->n.neoncache[j].t == NEON_CACHE_ST_D || dyn->n.neoncache[j].t == NEON_CACHE_ST_F)) {
                if(dyn->n.neoncache[j].n<dyn->n.stack_pop)
                    dyn->n.neoncache[j].v = 0;
                else
                    dyn->n.neoncache[j].n-=dyn->n.stack_pop;
            }
        dyn->n.stack_pop = 0;
    }
    dyn->n.stack = dyn->n.stack_next;
    dyn->n.news = 0;
    dyn->n.stack_push = 0;
    dyn->n.swapped = 0;
}