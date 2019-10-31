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
        int32_t i32;
        uint32_t u32;
        uint8_t sib = 0;
        int sib_reg = 0;
        if((nextop&7)==4) {
            sib = F8;
            sib_reg = (sib>>3)&7;
        }
        if(nextop&0x80)
            u32 = F32;
        else 
            i32 = F8S;
        if(u32==0x80000000) {
            // special case...
            MOV32(scratch, u32);
            if((nextop&7)==4) {
                if (sib_reg!=4) {
                    SUB_REG_LSL_IMM8(scratch, xEAX+(sib&0x07), scratch ,0);
                    ADD_REG_LSL_IMM8(ret, scratch, xEAX+sib_reg, (sib>>6));
                } else {
                    int tmp = xEAX+(sib&0x07);
                    SUB_REG_LSL_IMM8(ret, tmp, scratch, 0);
                }
            } else {
                int tmp = xEAX+(nextop&0x07);
                SUB_REG_LSL_IMM8(ret, tmp, scratch, 0);
            }
        } else {
            if(nextop&0x80)
                i32=(int32_t)u32;
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
                if(sub) i32 = -i32;  // this value cannot be negated!
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
            LDM(x1, (1<<x2) | (1<<x3)); // load dest address in x2 and planned ip in x3
            CMPS_REG_LSL_IMM8(xEIP, x3, 0);
            BXcond(cEQ, x2);
            MOV32(x2, (uintptr_t)arm_linker);
            STM(x1, (1<<x2) | (1<<x12)); // nope, putting back linker & IP in place
            BX(x2); // go to linker
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
    sse_pushcache(dyn, ninst, reg);
    MOV32(reg, (uintptr_t)fnc);
    BLX(reg);
    sse_popcache(dyn, ninst, reg);
    if(ret>=0) {
        MOV_REG(ret, 0);
    }
    POP(xSP, (1<<xEmu) | mask);
}

void grab_tlsdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg)
{
    MESSAGE(LOG_DUMP, "Get TLSData\n");
    call_c(dyn, ninst, GetGSBaseEmu, 12, reg, 0);
    MESSAGE(LOG_DUMP, "----TLSData\n");
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

// emit "lock", x1, x2 and x3 are lost
void emit_lock(dynarec_arm_t* dyn, uintptr_t addr, int ninst)
{
    PUSH(xSP, (1<<xEmu));   // save Emu
    LDR_IMM9(xEmu, xEmu, offsetof(x86emu_t, context));
    MOV32(x1, offsetof(box86context_t, mutex_lock));   // offset is way to big for imm8
    ADD_REG_LSL_IMM8(xEmu, xEmu, x1, 0);
    CALL(pthread_mutex_lock, -1, 0);
    POP(xSP, (1<<xEmu));
}

// emit "unlock", x1, x2 and x3 are lost
void emit_unlock(dynarec_arm_t* dyn, uintptr_t addr, int ninst)
{
    PUSH(xSP, (1<<xEmu));   // save Emu
    LDR_IMM9(xEmu, xEmu, offsetof(x86emu_t, context));
    MOV32(x1, offsetof(box86context_t, mutex_lock));   // offset is way to big for imm8
    ADD_REG_LSL_IMM8(xEmu, xEmu, x1, 0);
    CALL(pthread_mutex_unlock, -1, 0);
    POP(xSP, (1<<xEmu));
}

// x87 stuffs
#define X87FIRST    8
static void x87_reset(dynarec_arm_t* dyn, int ninst)
{
    for (int i=0; i<8; ++i)
        dyn->x87cache[i] = -1;
    dyn->x87stack = 0;
}

void x87_stackcount(dynarec_arm_t* dyn, int ninst, int scratch)
{
    if(!dyn->x87stack)
        return;
    MESSAGE(LOG_DUMP, "Synch x87 Stackcount (%d)\n", dyn->x87stack);
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
    MESSAGE(LOG_DUMP, "------x87 Stackcount\n");
}

int x87_do_push(dynarec_arm_t* dyn, int ninst)
{
    dyn->x87stack+=1;
    // move all regs in cache, and find a free one
    int ret = -1;
    for(int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1)
            ++dyn->x87cache[i];
        else if(ret==-1) {
            dyn->x87cache[i] = 0;
            ret=i;
        }
    return ret+X87FIRST;
}
void x87_do_pop(dynarec_arm_t* dyn, int ninst)
{
    dyn->x87stack-=1;
    // move all regs in cache, poping ST0
    for(int i=0; i<8; ++i)
        if(dyn->x87cache[i]!=-1)
            --dyn->x87cache[i];
}

static void x87_purgecache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    int ret = 0;
    for (int i=0; i<8 && !ret; ++i)
        if(dyn->x87cache[i] != -1)
            ret = 1;
    if(!ret && !dyn->x87stack)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "Purge x87 Cache and Synch Stackcount (%d)\n", dyn->x87stack);
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
    if(ret!=0) {
        // prepare offset to fpu => s1
        MOVW(s1, offsetof(x86emu_t, fpu));
        ADD_REG_LSL_IMM8(s1, xEmu, s1, 0);
        // Get top
        // loop all cache entries
        for (int i=0; i<8; ++i)
            if(dyn->x87cache[i]!=-1) {
                ADD_IMM8(s3, s2, dyn->x87cache[i]);
                AND_IMM8(s3, s3, 7);    // (emu->top + st)&7
                ADD_REG_LSL_IMM8(s3, s1, s3, 3);    // fpu[(emu->top+i)&7] lsl 3 because fpu are double, so 8 bytes
                VSTR_64(i+X87FIRST, s3, 0);    // save the value
                dyn->x87cache[i] = -1;
            }
    }
}

#ifdef HAVE_TRACE
static void x87_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    x87_stackcount(dyn, ninst, s1);
    int ret = 0;
    for (int i=0; (i<8) && (!ret); ++i)
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
            ADD_IMM8(s3, s2, dyn->x87cache[i]);
            AND_IMM8(s3, s3, 7);    // (emu->top + i)&7
            ADD_REG_LSL_IMM8(s3, s1, s3, 3);    // fpu[(emu->top+i)&7] lsl 3 because fpu are double, so 8 bytes
            VSTR_64(i+X87FIRST, s3, 0);    // save the value
        }
}
#endif

int x87_get_cache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
    // search in cache first
    for (int i=0; i<8; ++i)
        if(dyn->x87cache[i]==st)
            return i;
    MESSAGE(LOG_DUMP, "Create x87 Cache for ST%d\n", st);
    // get a free spot
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->x87cache[i]==-1)
            ret = i;
    // found, setup and grab the value
    dyn->x87cache[ret] = st;
    MOVW(s1, offsetof(x86emu_t, fpu));
    ADD_REG_LSL_IMM8(s1, xEmu, s1, 0);
    LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    int a = st - dyn->x87stack;
    if(a<0) {
        SUB_IMM8(s2, s2, -a);
    } else {
        ADD_IMM8(s2, s2, a);
    }
    AND_IMM8(s2, s2, 7);    // (emu->top + i)&7
    ADD_REG_LSL_IMM8(s2, s1, s2, 3);
    VLDR_64((ret+X87FIRST), s2, 0);
    MESSAGE(LOG_DUMP, "-------x87 Cache for ST%d\n", st);

    return ret;
}

int x87_get_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a)
{
    return x87_get_cache(dyn, ninst, s1, s2, a) + X87FIRST;
}


void x87_refresh(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st)
{
    x87_stackcount(dyn, ninst, s1);
    int ret = -1;
    for (int i=0; (i<8) && (ret==-1); ++i)
        if(dyn->x87cache[i] == st)
            ret = i;
    if(ret==-1)    // nothing to do
        return;
    MESSAGE(LOG_DUMP, "Refresh x87 Cache for ST%d\n", st);
    // prepare offset to fpu => s1
    MOVW(s1, offsetof(x86emu_t, fpu));
    ADD_REG_LSL_IMM8(s1, xEmu, s1, 0);
    // Get top
    LDR_IMM9(s2, xEmu, offsetof(x86emu_t, top));
    // Update
    ADD_IMM8(s2, s2, st);
    AND_IMM8(s2, s2, 7);    // (emu->top + i)&7
    ADD_REG_LSL_IMM8(s2, s1, s2, 3);    // fpu[(emu->top+i)&7] lsl 3 because fpu are double, so 8 bytes
    VSTR_64(ret+X87FIRST, s2, 0);    // save the value
    MESSAGE(LOG_DUMP, "--------x87 Cache for ST%d\n", st);
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

// Restore round flag
void x87_restoreround(dynarec_arm_t* dyn, int ninst, int s1)
{
    VMSR(s1);               // put back fpscr
}

// first SSE cache is Q8 (so D16+D17)
#define FIRSTSSE    8
static void sse_reset(dynarec_arm_t* dyn, int ninst)
{
    for (int i=0; i<8; ++i)
        dyn->ssecache[i] = 0;
}
// get neon register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a)
{
    int ret = (FIRSTSSE + a)*2;
    if(dyn->ssecache[a])
        return ret;
    dyn->ssecache[a] = 1;
    MOV32(s1, offsetof(x86emu_t, xmm[a]));
    ADD_REG_LSL_IMM8(s1, xEmu, s1, 0);
    VLD1Q_32(ret, s1);
    return ret;
}
// get neon register for a SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a)
{
    int ret = (FIRSTSSE + a)*2;
    if(dyn->ssecache[a])
        return ret;
    dyn->ssecache[a] = 1;
    return ret;
}
// purge the SSE cache only(needs 3 scratch registers)
static void sse_purgecache(dynarec_arm_t* dyn, int ninst, int s1)
{
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->ssecache[i]) {
            int a = (FIRSTSSE + i)*2;
            if (old==-1) {
                MESSAGE(LOG_DUMP, "Purge SSE Cache ------\n");
                MOV32(s1, offsetof(x86emu_t, xmm[i]));
                ADD_REG_LSL_IMM8(s1, xEmu, s1, 0);
                old = i;
            } else {
                ADD_IMM8(s1, s1, (i-old)*16);
                old = i;
            }
            VST1Q_32(a, s1);
            dyn->ssecache[i] = 0;
        }
    if(old!=-1) {
        MESSAGE(LOG_DUMP, "------ Purge SSE Cache\n");
    }
}
#ifdef HAVE_TRACE
static void sse_reflectcache(dynarec_arm_t* dyn, int ninst, int s1)
{
    int old = -1;
    for (int i=0; i<8; ++i)
        if(dyn->ssecache[i]) {
            int a = (FIRSTSSE + i)*2;
            if (old==-1) {
                MOV32(s1, offsetof(x86emu_t, xmm[i]));
                ADD_REG_LSL_IMM8(s1, xEmu, s1, 0);
            } else {
                ADD_IMM8(s1, s1, (i-old)*16);
                old = i;
            }
            VST1Q_32(a, s1);
        }
}
#endif

void sse_pushcache(dynarec_arm_t* dyn, int ninst, int s1)
{
    int n=0;
    for (int i=0; i<8; ++i)
        if(dyn->ssecache[i])
            ++n;
    if(!n)
        return;
    MESSAGE(LOG_DUMP, "Push SSE Cache (%d)------\n", n);
    if(n==8) {
        SUB_IMM8(xSP, xSP, 7*16);
        SUB_IMM8(xSP, xSP, 16);
    } else {
        SUB_IMM8(xSP, xSP, n*16);
    }
    MOV_REG(s1, xSP);
    for (int i=0; i<8; ++i)
        if(dyn->ssecache[i]) {
            int a = (FIRSTSSE+i)*2;
            VST1Q_32_W(a, s1);
        }
    MESSAGE(LOG_DUMP, "------- Push SSE Cache (%d)\n", n);
}

void sse_popcache(dynarec_arm_t* dyn, int ninst, int s1)
{
    int n=0;
    for (int i=0; i<8; ++i)
        if(dyn->ssecache[i])
            ++n;
    if(!n)
        return;
    MESSAGE(LOG_DUMP, "Pop SSE Cache (%d)------\n", n);
    MOV_REG(s1, xSP);
    for (int i=0; i<8; ++i)
        if(dyn->ssecache[i]) {
            int a = (FIRSTSSE+i)*2;
            VLD1Q_32_W(a, s1);
        }
    if(n==8) {
        ADD_IMM8(xSP, xSP, 7*16);
        ADD_IMM8(xSP, xSP, 16);
    } else {
        ADD_IMM8(xSP, xSP, n*16);
    }
    MESSAGE(LOG_DUMP, "------- Pop SSE Cache (%d)\n", n);
}

void fpu_purgecache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    x87_purgecache(dyn, ninst, s1, s2, s3);
    sse_purgecache(dyn, ninst, s1);
}

#ifdef HAVE_TRACE
void fpu_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3)
{
    x87_reflectcache(dyn, ninst, s1, s2, s3);
    if(trace_xmm)
       sse_reflectcache(dyn, ninst, s1);
}
#endif

void fpu_reset(dynarec_arm_t* dyn, int ninst)
{
    x87_reset(dyn, ninst);
    sse_reset(dyn, ninst);
}