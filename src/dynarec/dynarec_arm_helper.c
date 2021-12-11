#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>

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
uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int* fixaddress, uint32_t absmax, uint32_t mask, int getfixonly)
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
    SET_NODF();
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
static void x87_reset(dynarec_arm_t* dyn)
{
    for (int i=0; i<8; ++i)
        dyn->n.x87cache[i] = -1;
    dyn->n.x87stack = 0;
    dyn->n.stack = 0;
    dyn->n.stack_next = 0;
    dyn->n.stack_pop = 0;
    dyn->n.stack_push = 0;
    dyn->n.combined1 = dyn->n.combined2 = 0;
    dyn->n.swapped = 0;
    for(int i=0; i<24; ++i)
        if(dyn->n.neoncache[i].t == NEON_CACHE_ST_F || dyn->n.neoncache[i].t == NEON_CACHE_ST_D)
            dyn->n.neoncache[i].v = 0;
}

void x87_stackcount(dynarec_arm_t* dyn, int ninst, int scratch)
{
    if(!dyn->n.x87stack)
        return;
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
    MESSAGE(LOG_DUMP, "\t------x87 Stackcount\n");
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
    // move all regs in cache, and find a free one
    for(int j=0; j<24; ++j)
        if((dyn->n.neoncache[j].t == NEON_CACHE_ST_D) || (dyn->n.neoncache[j].t == NEON_CACHE_ST_F))
            ++dyn->n.neoncache[j].n;
    int ret = -1;
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
    // move all regs in cache
    for(int j=0; j<24; ++j)
        if((dyn->n.neoncache[j].t == NEON_CACHE_ST_D) || (dyn->n.neoncache[j].t == NEON_CACHE_ST_F))
            ++dyn->n.neoncache[j].n;
    for(int i=0; i<8; ++i)
        if(dyn->n.x87cache[i]!=-1)
            ++dyn->n.x87cache[i];
    if(s1)
        x87_stackcount(dyn, ninst, s1);
}
void x87_do_pop(dynarec_arm_t* dyn, int ninst, int s1)
{
    if(dyn->n.mmxcount)
        mmx_purgecache(dyn, ninst, 0, s1);
    dyn->n.x87stack-=1;
    dyn->n.stack_next-=1;
    dyn->n.stack_pop+=1;
    // move all regs in cache, poping ST0
    for(int i=0; i<8; ++i)
        if(dyn->n.x87cache[i]!=-1) {
            --dyn->n.x87cache[i];
            if(dyn->n.x87cache[i]==-1) {
                fpu_free_reg_double(dyn, dyn->n.x87reg[i]);
                dyn->n.x87reg[i] = -1;
            }
        }
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
    int offs = offsetof(x86emu_t, p_regs);
    if(!(offs&3) && (offs>>2)<256) {
        ADD_IMM8_ROR(s1, xEmu, offs>>2, 15);
    } else {
        MOVW(s1, offs);
        ADD_REG_LSL_IMM5(s1, xEmu, s1, 0);
    }
    MOVW(s3, 0);
    for (int i=0; i<8; ++i) {
        STRAI_IMM9_W(s3, s1, sizeof(fpu_p_reg_t));
    }
    // setup TOP => full stack so = 0
    STR_IMM9(s3, xEmu, offsetof(x86emu_t, top));
    offs = offsetof(x86emu_t, x87);
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
        if(next) {
            // need to check if a ST_F need local promotion
            if(neoncache_get_st_f(dyn, ninst, dyn->n.x87cache[j], dyn->n.stack_next)>=0) {
                VCVT_F64_F32(0, dyn->n.x87reg[j]*2);
                VSTM_64_W(0, s1);    // save the value
            } else {
                VSTM_64_W(dyn->n.x87reg[j], s1);    // save the value
            }
        } else {
            VSTM_64_W(dyn->n.x87reg[j], s1);    // save the value
            fpu_free_reg_double(dyn, dyn->n.x87reg[j]);
            dyn->n.x87reg[j] = -1;
            dyn->n.x87cache[j] = -1;
            dyn->n.stack_pop+=1;
        }
    }
    if(!next) {
        dyn->n.stack_next = 0;
        #if STEP == 1
        // refresh the cached valued, in case it's a purge outside a instruction
        dyn->insts[ninst].n = dyn->n;
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
        // Get top
        // loop all cache entries
        for (int i=0; i<8; ++i)
            if(dyn->n.x87cache[i]!=-1) {
                #if STEP == 1
                if(!next)   // don't force promotion here
                    neoncache_promote_double(dyn, ninst, dyn->n.x87cache[i], dyn->n.stack_next);
                #endif
                #if STEP == 3
                if(!next && neoncache_get_st_f(dyn, ninst, dyn->n.x87cache[i], dyn->n.stack_next)>=0) {
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
                    if(neoncache_get_st_f(dyn, ninst, dyn->n.x87cache[i], dyn->n.stack_next)>=0) {
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
                    dyn->n.stack_pop+=1;
                }
            }
    }
    if(!next) {
        dyn->n.stack_next = 0;
        #if STEP == 1
        // refresh the cached valued, in case it's a purge outside a instruction
        dyn->insts[ninst].n = dyn->n;
        #endif
    }
    MESSAGE(LOG_DUMP, "\t---Purge x87 Cache and Synch Stackcount\n");
}

#ifdef HAVE_TRACE
static void x87_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    x87_stackcount(dyn, ninst, s1);
    int ret = 0;
    for (int i=0; (i<8) && (!ret); ++i)
        if(dyn->n.x87cache[i] != -1)
            ret = 1;
    if(!ret)    // nothing to do
        return;
    // Get top
    LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    // loop all cache entries
    for (int i=0; i<8; ++i)
        if(dyn->n.x87cache[i]!=-1) {
            ADD_IMM8(s3, s2, dyn->n.x87cache[i]);
            AND_IMM8(s3, s3, 7);    // (emu->top + i)&7
            ADD_REG_LSL_IMM5(s3, xEmu, s3, 3);    // fpu[(emu->top+i)&7] lsl 3 because fpu are double, so 8 bytes
            if(ST_IS_F(dyn->n.x87cache[i])) {
                VCVT_F64_F32(0, dyn->n.x87reg[i]*2);    // use D0 as scratch...
                VSTR_64(0, s3, offsetof(x86emu_t, x87));
            } else {
                VSTR_64(dyn->n.x87reg[i], s3, offsetof(x86emu_t, x87));    // save the value
            }
        }
}
#endif

int x87_get_current_cache(dynarec_arm_t* dyn, int ninst, int st, int t)
{
    // search in cache first
    for (int i=0; i<8; ++i) {
        if(dyn->n.x87cache[i]==st) {
            #if STEP == 1
            if(t==NEON_CACHE_ST_D && (dyn->n.neoncache[dyn->n.x87reg[i]-FPUFIRST].t==NEON_CACHE_ST_F))
                neoncache_promote_double(dyn, ninst, st, dyn->n.stack);
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
    #if STEP == 1
    neoncache_promote_double(dyn, ninst, st, dyn->n.stack);
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
    VSTR_64(dyn->n.x87reg[ret], s2, 0);    // save the value
    MESSAGE(LOG_DUMP, "\t--------Forget x87 Cache for ST%d\n", st);
    // and forget that cache
    fpu_free_reg_double(dyn, dyn->n.x87reg[ret]);
    dyn->n.neoncache[dyn->n.x87reg[ret]].v = 0;
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
            neoncache_promote_double(dyn, ninst, st, dyn->n.stack);
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
static void mmx_reset(dynarec_arm_t* dyn)
{
    dyn->n.mmxcount = 0;
    for (int i=0; i<8; ++i)
        dyn->n.mmxcache[i] = -1;
}
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
#ifdef HAVE_TRACE
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
#endif


// SSE / SSE2 helpers
static void sse_reset(dynarec_arm_t* dyn)
{
    for (int i=0; i<8; ++i)
        dyn->n.ssecache[i].v = -1;
}
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
#ifdef HAVE_TRACE
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
#endif

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
            VST1_32_W(i, s1);
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
}

void fpu_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1, int s2, int s3)
{
    x87_purgecache(dyn, ninst, next, s1, s2, s3);
    mmx_purgecache(dyn, ninst, next, s1);
    sse_purgecache(dyn, ninst, next, s1);
    if(!next)
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

void fpu_reset(dynarec_arm_t* dyn)
{
    x87_reset(dyn);
    mmx_reset(dyn);
    sse_reset(dyn);
    fpu_reset_reg(dyn);
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
