#ifndef __DYNAREC_ARM_HELPER_H__
#define __DYNAREC_ARM_HELPER_H__

#if STEP == 0
#include "dynarec_arm_pass0.h"
#elif STEP == 1
#include "dynarec_arm_pass1.h"
#elif STEP == 2
#include "dynarec_arm_pass2.h"
#elif STEP == 3
#include "dynarec_arm_pass3.h"
#endif

#include "debug.h"
#include "arm_emitter.h"
#include "../emu/x86primop.h"
#include "dynarec_arm_functions.h"

#define F8      *(uint8_t*)(addr++)
#define F8S     *(int8_t*)(addr++)
#define F16     *(uint16_t*)(addr+=2, addr-2)
#define F16S    *(int16_t*)(addr+=2, addr-2)
#define F32     *(uint32_t*)(addr+=4, addr-4)
#define F32S    *(int32_t*)(addr+=4, addr-4)
#define PK(a)   *(uint8_t*)(addr+a)
#define PK32(a)   *(uint32_t*)(addr+a)
#define PK16(a)   *(uint16_t*)(addr+a)
#define PKip(a)   *(uint8_t*)(ip+a)

// GETGD    get x86 register in gd
#define GETGD   gd = xEAX+((nextop&0x38)>>3)
//GETED can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETED   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    wback = 0;              \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress); \
                    LDR_IMM9(x1, wback, 0); \
                    ed = x1;                \
                }
//GETEDH can use hint for ed, and r1 or r2 for wback (depending on hint). wback is 0 if ed is xEAX..xEDI
#define GETEDH(hint)   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    wback = 0;              \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, (hint==x2)?x1:x2, &fixedaddress); \
                    LDR_IMM9(hint, wback, 0); \
                    ed = hint;              \
                }
//GETEDW can use hint for wback and ret for ed. wback is 0 if ed is xEAX..xEDI
#define GETEDW(hint, ret)   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    MOV_REG(ret, ed);       \
                    wback = 0;              \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, hint, &fixedaddress); \
                    ed = ret;               \
                    LDR_IMM9(ed, wback, 0); \
                }
// Write back ed in wback (if wback not 0)
#define WBACK       if(wback) {STR_IMM9(ed, wback, 0);}
// Send back wb to either ed or wback
#define SBACK(wb)   if(wback) {STR_IMM9(wb, wback, 0);} else {MOV_REG(ed, wb);}
//GETEDO can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETEDO(O)   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    wback = 0;              \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress); \
                    LDR_REG_LSL_IMM5(x1, wback, O, 0);  \
                    ed = x1;                 \
                }
#define WBACKO(O)   if(wback) {STR_REG_LSL_IMM5(ed, wback, O, 0);}
//FAKEELike GETED, but doesn't get anything
#define FAKEED  if((nextop&0xC0)!=0xC0) {   \
                    addr = fakeed(dyn, addr, ninst, nextop); \
                }
// GETGW extract x86 register in gd, that is i
#define GETGW(i) gd = xEAX+((nextop&0x38)>>3); UXTH(i, gd, 0); gd = i;
//GETEWW will use i for ed, and can use w for wback.
#define GETEWW(w, i) if((nextop&0xC0)==0xC0) {  \
                    wback = xEAX+(nextop&7);\
                    UXTH(i, wback, 0);      \
                    ed = i;                 \
                    wb1 = 0;                \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, w, &fixedaddress); \
                    LDRH_IMM8(i, wback, 0); \
                    ed = i;                 \
                    wb1 = 1;                \
                }
//GETEW will use i for ed, and can use r3 for wback.
#define GETEW(i) if((nextop&0xC0)==0xC0) {  \
                    wback = xEAX+(nextop&7);\
                    UXTH(i, wback, 0);      \
                    ed = i;                 \
                    wb1 = 0;                \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress); \
                    LDRH_IMM8(i, wback, 0); \
                    ed = i;                 \
                    wb1 = 1;                \
                }
// Write ed back to original register / memory
#define EWBACK   if(wb1) {STRH_IMM8(ed, wback, 0);} else {BFI(wback, ed, 0, 16);}
// Write w back to original register / memory
#define EWBACKW(w)   if(wb1) {STRH_IMM8(w, wback, 0);} else {BFI(wback, w, 0, 16);}
// Write back gd in correct register
#define GWBACK       BFI((xEAX+((nextop&0x38)>>3)), gd, 0, 16);
//GETEB will use i for ed, and can use r3 for wback.
#define GETEB(i) if((nextop&0xC0)==0xC0) {  \
                    wback = (nextop&7);     \
                    wb2 = (wback>>2);       \
                    wback = xEAX+(wback&3); \
                    UXTB(i, wback, wb2);    \
                    wb1 = 0;                \
                    ed = i;                 \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress); \
                    LDRB_IMM9(i, wback, 0); \
                    wb1 = 1;                \
                    ed = i;                 \
                }
// Write eb (ed) back to original register / memory
#define EBBACK   if(wb1) {STRB_IMM9(ed, wback, 0);} else {BFI(wback, ed, wb2*8, 8);}
//GETGB will use i for gd
#define GETGB(i)    gd = (nextop&0x38)>>3;  \
                    gb2 = ((gd&4)>>2);      \
                    gb1 = xEAX+(gd&3);      \
                    gd = i;                 \
                    UXTB(gd, gb1, gb2);
// Write gb (gd) back to original register / memory
#define GBBACK   BFI(gb1, gd, gb2*8, 8);

// Get Direction with size Z and based of F_DF flag, on register r ready for LDR/STR fetching
#define GETDIR(r, A)    \
    LDR_IMM9(r, xEmu, offsetof(x86emu_t, flags[F_DF]));     \
    CMPS_IMM8(r, 1);                                        \
    MOVW(r, A);                                             \
    RSB_COND_IMM8(cEQ, r, r, 0)

// CALL will use x12 for the call address. Return value can be put in ret (unless ret is -1)
#define CALL(F, ret, M) call_c(dyn, ninst, F, x12, ret, M)
// CALL_ will use x3 for the call address. Return value can be put in ret (unless ret is -1)
#define CALL_(F, ret, M) call_c(dyn, ninst, F, x3, ret, M)
#define MARK    if(dyn->insts) {dyn->insts[ninst].mark = (uintptr_t)dyn->arm_size;}
#define GETMARK ((dyn->insts)?dyn->insts[ninst].mark:(dyn->arm_size+4))
#define MARK2   if(dyn->insts) {dyn->insts[ninst].mark2 = (uintptr_t)dyn->arm_size;}
#define GETMARK2 ((dyn->insts)?dyn->insts[ninst].mark2:(dyn->arm_size+4))
#define MARK3   if(dyn->insts) {dyn->insts[ninst].mark3 = (uintptr_t)dyn->arm_size;}
#define GETMARK3 ((dyn->insts)?dyn->insts[ninst].mark3:(dyn->arm_size+4))
#define MARKF   if(dyn->insts) {dyn->insts[ninst].markf = (uintptr_t)dyn->arm_size;}
#define GETMARKF ((dyn->insts)?dyn->insts[ninst].markf:(dyn->arm_size+4))

// Generate FCOM with s1 and s2 scratch regs (the VCMP is already done)
#define FCOM(s1, s2)    \
    LDRH_IMM8(s2, xEmu, offsetof(x86emu_t, sw));   /*offset is 8bits right?*/   \
    MOVW(s1, 0b0100011100000000);                                               \
    BIC_REG_LSL_IMM8(s2, s2, s1, 0);                                            \
    VMRS_APSR();    /* 0b0100011100000000 */                                    \
    MOVW_COND(cVS, s1, 0b0100010100000000); /* unordered */                     \
    MOVW_COND(cEQ, s1, 0b0100000000000000); /* zero */                          \
    MOVW_COND(cGT, s1, 0b0000000000000000); /* greater than */                  \
    MOVW_COND(cLO, s1, 0b0000000100000000); /* less than */                     \
    ORR_REG_LSL_IMM8(s2, s2, s1, 0);                                            \
    STRH_IMM8(s2, xEmu, offsetof(x86emu_t, sw))

// Generate FCOMI with s1 and s2 scratch regs (the VCMP is already done)
#define FCOMI(s1, s2)    \
    XOR_REG_LSL_IMM8(s2, s2, s2, 0);                        \
    VMRS_APSR();    /* 0b111 */                             \
    MOVW_COND(cVS, s1, 0b111); /* unordered */              \
    MOVW_COND(cEQ, s1, 0b100); /* zero */                   \
    MOVW_COND(cGT, s1, 0b000); /* greater than */           \
    MOVW_COND(cLO, s1, 0b001); /* less than */              \
    UBFX(s2, s1, 0, 1);                                     \
    STR_IMM9(s2, xEmu, offsetof(x86emu_t, flags[F_CF]));    \
    UBFX(s2, s1, 1, 1);                                     \
    STR_IMM9(s2, xEmu, offsetof(x86emu_t, flags[F_PF]));    \
    UBFX(s2, s1, 2, 1);                                     \
    STR_IMM9(s2, xEmu, offsetof(x86emu_t, flags[F_ZF]));    \
    UFLAG_DF(x2, d_none)


#ifndef UFLAGS
#define UFLAGS(A)  dyn->cleanflags=A
#endif
#ifndef USEFLAG
// USEFLAG will check status of defered flags the call update flags if needed. x3 will be used
#define USEFLAG(A)   \
    if(!dyn->cleanflags) {  \
        if(A) {             \
            LDR_IMM9(x3, xEmu, offsetof(x86emu_t, df)); \
            TSTS_REG_LSL_IMM8(x3, x3, 0);               \
            i32 = (GETMARKF)-(dyn->arm_size+8);         \
            Bcond(cEQ, i32);                            \
            CALL_(UpdateFlags, -1, 0);                  \
            MARKF;                                      \
            dyn->cleanflags=1;                          \
        }                   \
    }
#endif
#ifndef JUMP
#define JUMP(A) 
#endif
#define BARRIER(A) if (dyn->insts) dyn->insts[ninst].x86.barrier = A
#define UFLAG_OP1(A) if(dyn->insts && dyn->insts[ninst].x86.flags) {STR_IMM9(A, 0, offsetof(x86emu_t, op1));}
#define UFLAG_OP2(A) if(dyn->insts && dyn->insts[ninst].x86.flags) {STR_IMM9(A, 0, offsetof(x86emu_t, op2));}
#define UFLAG_OP12(A1, A2) if(dyn->insts && dyn->insts[ninst].x86.flags) {STR_IMM9(A1, 0, offsetof(x86emu_t, op1));STR_IMM9(A2, 0, offsetof(x86emu_t, op2));}
#define UFLAG_RES(A) if(dyn->insts && dyn->insts[ninst].x86.flags) {STR_IMM9(A, 0, offsetof(x86emu_t, res));}
#define UFLAG_DF(r, A) if(dyn->insts && dyn->insts[ninst].x86.flags) {MOVW(r, A); STR_IMM9(r, 0, offsetof(x86emu_t, df));}
#define UFLAG_IF if(dyn->insts && dyn->insts[ninst].x86.flags)
#ifndef DEFAULT
#define DEFAULT      BARRIER(2)
#endif
// Emit the LOCK mutex (x1, x2 and x3 are lost)
#define LOCK        emit_lock(dyn, addr, ninst)
// Emit the UNLOCK mutex (x1, x2 and x3 are lost)
#define UNLOCK      emit_unlock(dyn, addr, ninst)


void arm_epilog();
void* arm_linker(x86emu_t* emu, void** table, uintptr_t addr);

#ifndef STEPNAME
#define STEPNAME3(N,M) N##M
#define STEPNAME2(N,M) STEPNAME3(N,M)
#define STEPNAME(N) STEPNAME2(N, STEP)
#endif

#define arm_pass        STEPNAME(arm_pass)

#define dynarec00       STEPNAME(dynarec00)
#define dynarec0F       STEPNAME(dynarec0F)
#define dynarecGS       STEPNAME(dynarecGS)
#define dynarec66       STEPNAME(dynarec66)
#define dynarec67       STEPNAME(dynarec67)
#define dynarecD8       STEPNAME(dynarecD8)
#define dynarecD9       STEPNAME(dynarecD9)
#define dynarecDA       STEPNAME(dynarecDA)
#define dynarecDB       STEPNAME(dynarecDB)
#define dynarecDC       STEPNAME(dynarecDC)
#define dynarecDD       STEPNAME(dynarecDD)
#define dynarecDE       STEPNAME(dynarecDE)
#define dynarecDF       STEPNAME(dynarecDF)
#define dynarecF0       STEPNAME(dynarecF0)
#define dynarec660F     STEPNAME(dynarec660F)
#define dynarecF20F     STEPNAME(dynarecF20F)
#define dynarecF30F     STEPNAME(dynarecF30F)

#define geted           STEPNAME(geted_)
#define fakeed          STEPNAME(fakeed_)
#define jump_to_epilog  STEPNAME(jump_to_epilog_)
#define jump_to_linker  STEPNAME(jump_to_linker_)
#define ret_to_epilog   STEPNAME(ret_to_epilog_)
#define retn_to_epilog  STEPNAME(retn_to_epilog_)
#define call_c          STEPNAME(call_c_)
#define grab_tlsdata    STEPNAME(grab_tlsdata_)
#define isNativeCall    STEPNAME(isNativeCall_)
#define emit_lock       STEPNAME(emit_lock)
#define emit_unlock     STEPNAME(emit_unlock)

#define x87_do_push     STEPNAME(x87_do_push)
#define x87_do_pop      STEPNAME(x87_do_pop)
#define x87_reset       STEPNAME(x87_reset)
#define x87_purgecache  STEPNAME(x87_purgecache)
#define x87_reflectcache STEPNAME(x87_reflectcache)
#define x87_get_cache   STEPNAME(x87_get_cache)
#define x87_get_st      STEPNAME(x87_get_st)
#define x87_refresh     STEPNAME(x87_refresh)
#define x87_stackcount  STEPNAME(x87_stackcount)
#define x87_setround    STEPNAME(x87_setround)
#define x87_restoreround STEPNAME(x87_restoreround)
#define x87_get_scratch_single STEPNAME(x87_get_scratch_single)
#define x87_get_scratch_double STEPNAME(x87_get_scratch_double)

/* setup r2 to address pointed by */
uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int* fixedaddress);

// Do the GETED, but don't emit anything...
uintptr_t fakeed(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop);

// generic x86 helper
void jump_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst);
void jump_to_linker(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst);
void ret_to_epilog(dynarec_arm_t* dyn, int ninst);
void retn_to_epilog(dynarec_arm_t* dyn, int ninst, int n);
void call_c(dynarec_arm_t* dyn, int ninst, void* fnc, int reg, int ret, uint32_t mask);
void grab_tlsdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg);
int isNativeCall(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t* calladdress, int* retn);
void emit_lock(dynarec_arm_t* dyn, uintptr_t addr, int ninst);
void emit_unlock(dynarec_arm_t* dyn, uintptr_t addr, int ninst);

// x87 helper
// reset the cache
void x87_reset(dynarec_arm_t* dyn, int ninst);
// cache of the local stack counter, to avoid upadte at every call
void x87_stackcount(dynarec_arm_t* dyn, int ninst, int scratch);
// fpu push, needs 1 scratch register. Return the Dd value to be used
int x87_do_push(dynarec_arm_t* dyn, int ninst);
// fpu pop, needs 1 scratch register. All previous returned Dd should be considered invalid
void x87_do_pop(dynarec_arm_t* dyn, int ninst);
// purge the cache (needs 3 scratch registers)
void x87_purgecache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
#ifdef HAVE_TRACE
void x87_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
#endif
// get cache index for a x87 reg, create the entry if needed
int x87_get_cache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a);
// get vfpu register for a x7 reg, create the entry if needed
int x87_get_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a);
// refresh a value from the cache ->emu (nothing done if value is not cached)
void x87_refresh(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st);
// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
// Restore round flag
void x87_restoreround(dynarec_arm_t* dyn, int ninst, int s1);
// Get a FPU single scratch reg
int x87_get_scratch_single(int i);
// Geta FPU double scratch reg
int x87_get_scratch_double(int i);


uintptr_t dynarec00(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarec0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarecGS(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarec66(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarec67(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarecD8(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarecD9(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarecDA(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarecDB(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarecDC(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarecDD(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarecDE(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarecDF(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarecF0(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarec660F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarecF20F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarecF30F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);

#endif //__DYNAREC_ARM_HELPER_H__