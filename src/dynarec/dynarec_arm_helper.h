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
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 4095, 0); \
                    LDR_IMM9(x1, wback, fixedaddress); \
                    ed = x1;                \
                }
//GETEDH can use hint for ed, and r1 or r2 for wback (depending on hint). wback is 0 if ed is xEAX..xEDI
#define GETEDH(hint)   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    wback = 0;              \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, (hint==x2)?x1:x2, &fixedaddress, 4095, 0); \
                    LDR_IMM9(hint, wback, fixedaddress); \
                    ed = hint;              \
                }
//GETEDW can use hint for wback and ret for ed. wback is 0 if ed is xEAX..xEDI
#define GETEDW(hint, ret)   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    MOV_REG(ret, ed);       \
                    wback = 0;              \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, hint, &fixedaddress, 4095, 0); \
                    ed = ret;               \
                    LDR_IMM9(ed, wback, fixedaddress); \
                }
// Write back ed in wback (if wback not 0)
#define WBACK       if(wback) {STR_IMM9(ed, wback, fixedaddress);}
// Send back wb to either ed or wback
#define SBACK(wb)   if(wback) {STR_IMM9(wb, wback, fixedaddress);} else {MOV_REG(ed, wb);}
//GETEDO can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETEDO(O)   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    wback = 0;              \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0); \
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
                    addr = geted(dyn, addr, ninst, nextop, &wback, w, &fixedaddress, 255, 0); \
                    LDRH_IMM8(i, wback, fixedaddress); \
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
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0); \
                    LDRH_IMM8(i, wback, fixedaddress); \
                    ed = i;                 \
                    wb1 = 1;                \
                }
//GETSEW will use i for ed, and can use r3 for wback. This is the Signed version
#define GETSEW(i) if((nextop&0xC0)==0xC0) {  \
                    wback = xEAX+(nextop&7);\
                    SXTH(i, wback, 0);      \
                    ed = i;                 \
                    wb1 = 0;                \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0); \
                    LDRSH_IMM8(i, wback, fixedaddress);\
                    ed = i;                 \
                    wb1 = 1;                \
                }
// Write ed back to original register / memory
#define EWBACK   if(wb1) {STRH_IMM8(ed, wback, fixedaddress);} else {BFI(wback, ed, 0, 16);}
// Write w back to original register / memory
#define EWBACKW(w)   if(wb1) {STRH_IMM8(w, wback, fixedaddress);} else {BFI(wback, w, 0, 16);}
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
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 4095, 0); \
                    LDRB_IMM9(i, wback, fixedaddress); \
                    wb1 = 1;                \
                    ed = i;                 \
                }
//GETEBO will use i for ed, i is also Offset, and can use r3 for wback.
#define GETEBO(i) if((nextop&0xC0)==0xC0) {  \
                    wback = (nextop&7);     \
                    wb2 = (wback>>2);       \
                    wback = xEAX+(wback&3); \
                    UXTB(i, wback, wb2);    \
                    wb1 = 0;                \
                    ed = i;                 \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0); \
                    ADD_REG_LSL_IMM5(wback, wback, i, 0);   \
                    LDRB_IMM9(i, wback, fixedaddress);      \
                    wb1 = 1;                \
                    ed = i;                 \
                }
//GETSEB sign extend EB, will use i for ed, and can use r3 for wback.
#define GETSEB(i) if((nextop&0xC0)==0xC0) { \
                    wback = (nextop&7);     \
                    wb2 = (wback>>2);       \
                    wback = xEAX+(wback&3); \
                    SXTB(i, wback, wb2);    \
                    wb1 = 0;                \
                    ed = i;                 \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0); \
                    LDRSB_IMM8(i, wback, fixedaddress);\
                    wb1 = 1;                \
                    ed = i;                 \
                }
// Write eb (ed) back to original register / memory
#define EBBACK   if(wb1) {STRB_IMM9(ed, wback, fixedaddress);} else {BFI(wback, ed, wb2*8, 8);}
//GETGB will use i for gd
#define GETGB(i)    gd = (nextop&0x38)>>3;  \
                    gb2 = ((gd&4)>>2);      \
                    gb1 = xEAX+(gd&3);      \
                    gd = i;                 \
                    UXTB(gd, gb1, gb2);
//GETSGB signe extend GB, will use i for gd
#define GETSGB(i)    gd = (nextop&0x38)>>3; \
                    gb2 = ((gd&4)>>2);      \
                    gb1 = xEAX+(gd&3);      \
                    gd = i;                 \
                    SXTB(gd, gb1, gb2);
// Write gb (gd) back to original register / memory
#define GBBACK   BFI(gb1, gd, gb2*8, 8);

// Get Direction with size Z and based of F_DF flag, on register r ready for LDR/STR fetching
// F_DF is 1<<10, so 1 ROR 11*2 (so F_OF)
#define GETDIR(r, A)    \
    TSTS_IMM8_ROR(xFlags, 1, 0x0b);         \
    MOVW(r, A);                             \
    RSB_COND_IMM8(cNE, r, r, 0)

// CALL will use x14 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2
#define CALL(F, ret, M) call_c(dyn, ninst, F, x14, ret, M, 1)
// CALL_ will use x3 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2
#define CALL_(F, ret, M) call_c(dyn, ninst, F, x3, ret, M, 1)
// CALL_S will use x3 for the call address. Return value can be put in ret (unless ret is -1)
// R0 will not be pushed/popd if ret is -2. Flags are not save/restored
#define CALL_S(F, ret, M) call_c(dyn, ninst, F, x3, ret, M, 0)

#define MARK    if(dyn->insts) {dyn->insts[ninst].mark = (uintptr_t)dyn->arm_size;}
#define GETMARK ((dyn->insts)?dyn->insts[ninst].mark:(dyn->arm_size+4))
#define MARK2   if(dyn->insts) {dyn->insts[ninst].mark2 = (uintptr_t)dyn->arm_size;}
#define GETMARK2 ((dyn->insts)?dyn->insts[ninst].mark2:(dyn->arm_size+4))
#define MARK3   if(dyn->insts) {dyn->insts[ninst].mark3 = (uintptr_t)dyn->arm_size;}
#define GETMARK3 ((dyn->insts)?dyn->insts[ninst].mark3:(dyn->arm_size+4))
#define MARKF   if(dyn->insts) {dyn->insts[ninst].markf = (uintptr_t)dyn->arm_size;}
#define GETMARKF ((dyn->insts)?dyn->insts[ninst].markf:(dyn->arm_size+4))
#define MARKSEG if(dyn->insts) {dyn->insts[ninst].markseg = (uintptr_t)dyn->arm_size;}
#define GETMARKSEG ((dyn->insts)?dyn->insts[ninst].markseg:(dyn->arm_size+4))
#define MARKLOCK if(dyn->insts) {dyn->insts[ninst].marklock = (uintptr_t)dyn->arm_size;}
#define GETMARKLOCK ((dyn->insts)?dyn->insts[ninst].marklock:(dyn->arm_size+4))

// Branch to MARK if cond (use j32)
#define B_MARK(cond)    \
    j32 = GETMARK-(dyn->arm_size+8);    \
    Bcond(cond, j32)
// Branch to MARK2 if cond (use j32)
#define B_MARK2(cond)    \
    j32 = GETMARK2-(dyn->arm_size+8);   \
    Bcond(cond, j32)
// Branch to MARK3 if cond (use j32)
#define B_MARK3(cond)    \
    j32 = GETMARK3-(dyn->arm_size+8);   \
    Bcond(cond, j32)
// Branch to next instruction if cond (use j32)
#define B_NEXT(cond)     \
    j32 = (dyn->insts)?(dyn->insts[ninst].epilog-(dyn->arm_size+8)):0; \
    Bcond(cond, j32)
// Branch to MARKSEG if cond (use j32)
#define B_MARKSEG(cond)    \
    j32 = GETMARKSEG-(dyn->arm_size+8);   \
    Bcond(cond, j32)
// Branch to MARKLOCK if cond (use j32)
#define B_MARKLOCK(cond)    \
    j32 = GETMARKLOCK-(dyn->arm_size+8);   \
    Bcond(cond, j32)

#define IFX(A)  if(dyn->insts && (dyn->insts[ninst].x86.need_flags&(A)))
#define IFXX(A) if(dyn->insts && (dyn->insts[ninst].x86.need_flags==(A)))
#define IFX2X(A, B) if(dyn->insts && (dyn->insts[ninst].x86.need_flags==(A) || dyn->insts[ninst].x86.need_flags==(B) || dyn->insts[ninst].x86.need_flags==((A)|(B))))
#define IFXN(A, B)  if(dyn->insts && (dyn->insts[ninst].x86.need_flags&(A) && !(dyn->insts[ninst].x86.need_flags&(B))))

// Generate FCOM with s1 and s2 scratch regs (the VCMP is already done)
#define FCOM(s1, s2)    \
    VMRS_APSR();    /* 0b0100011100000000 */                                    \
    LDRH_IMM8(s2, xEmu, offsetof(x86emu_t, sw));   /*offset is 8bits right?*/   \
    MOVW(s1, 0b0100011100000000);                                               \
    BIC_REG_LSL_IMM5(s2, s2, s1, 0);                                            \
    MOVW_COND(cVS, s1, 0b0100010100000000); /* unordered */                     \
    MOVW_COND(cEQ, s1, 0b0100000000000000); /* zero */                          \
    MOVW_COND(cGT, s1, 0b0000000000000000); /* greater than */                  \
    MOVW_COND(cLO, s1, 0b0000000100000000); /* less than */                     \
    ORR_REG_LSL_IMM5(s2, s2, s1, 0);                                            \
    STRH_IMM8(s2, xEmu, offsetof(x86emu_t, sw))

// Generate FCOMI with s1 and s2 scratch regs (the VCMP is already done)
#define FCOMI(s1, s2)    \
    VMRS_APSR();    /* 0b111 */                             \
    MOVW_COND(cVS, s1, 0b1000101); /* unordered */          \
    MOVW_COND(cEQ, s1, 0b1000000); /* zero */               \
    MOVW_COND(cGT, s1, 0b0000000); /* greater than */       \
    MOVW_COND(cLO, s1, 0b0000001); /* less than */          \
    IFX(X_CF|X_PF|X_ZF|X_PEND) {                            \
        BIC_IMM8(xFlags, xFlags, 0b1000101, 0);             \
        ORR_REG_LSL_IMM5(xFlags, xFlags, s1, 0);            \
    }                                                       \
    SET_DFNONE(s1);                                         \
    IFX(X_OF|X_PEND) {                                      \
        BFC(xFlags, F_OF, 1);                               \
    }                                                       \
    IFX(X_AF|X_PEND) {                                      \
        BFC(xFlags, F_AF, 1);                               \
    }                                                       \
    IFX(X_SF|X_PEND) {                                      \
        BFC(xFlags, F_SF, 1);                               \
    }                                                       \


#define SET_DFNONE(S)    if(!dyn->dfnone) {MOVW(S, d_none); STR_IMM9(S, xEmu, offsetof(x86emu_t, df)); dyn->dfnone=1;}
#define SET_DF(S, N)     if(N) {MOVW(S, N); STR_IMM9(S, xEmu, offsetof(x86emu_t, df)); dyn->dfnone=0;} else SET_DFNONE(S)
#define SET_NODF()          dyn->dfnone = 0
#define SET_DFOK()          dyn->dfnone = 1

#ifndef READFLAGS
#define READFLAGS(A) \
    if(((A)!=X_PEND) && dyn->state_flags!=SF_SET) {     \
        if(dyn->state_flags!=SF_PENDING) {              \
            LDR_IMM9(x3, xEmu, offsetof(x86emu_t, df)); \
            TSTS_REG_LSL_IMM5(x3, x3, 0);               \
            j32 = (GETMARKF)-(dyn->arm_size+8);         \
            Bcond(cEQ, j32);                            \
        }                                               \
        CALL_(UpdateFlags, -1, 0);                      \
        MARKF;                                          \
        dyn->state_flags = SF_SET;                      \
        SET_DFOK();                                     \
    }
#endif
#ifndef SETFLAGS
#define SETFLAGS(A, B)  \
    if(dyn->state_flags!=SF_SET && B==SF_SUBSET && (dyn->insts[ninst].x86.need_flags&(~((A)|X_PEND)))) \
        READFLAGS(dyn->insts[ninst].x86.need_flags&(~(A)));    \
    dyn->state_flags = (B==SF_SUBSET)?SF_SET:B
#endif
#ifndef JUMP
#define JUMP(A) 
#endif
#ifndef BARRIER
#define BARRIER(A) 
#endif
#ifndef BARRIER_NEXT
#define BARRIER_NEXT(A)
#endif
#define UFLAG_OP1(A) if(dyn->insts && dyn->insts[ninst].x86.need_flags) {STR_IMM9(A, xEmu, offsetof(x86emu_t, op1));}
#define UFLAG_OP2(A) if(dyn->insts && dyn->insts[ninst].x86.need_flags) {STR_IMM9(A, xEmu, offsetof(x86emu_t, op2));}
#define UFLAG_OP12(A1, A2) if(dyn->insts && dyn->insts[ninst].x86.need_flags) {STR_IMM9(A1, xEmu, offsetof(x86emu_t, op1));STR_IMM9(A2, 0, offsetof(x86emu_t, op2));}
#define UFLAG_RES(A) if(dyn->insts && dyn->insts[ninst].x86.need_flags) {STR_IMM9(A, xEmu, offsetof(x86emu_t, res));}
#define UFLAG_DF(r, A) if(dyn->insts && dyn->insts[ninst].x86.need_flags) {SET_DF(r, A)}
#define UFLAG_IF if(dyn->insts && dyn->insts[ninst].x86.need_flags)
#ifndef DEFAULT
#define DEFAULT      *ok = -1; BARRIER(2)
#endif
#ifndef NEW_BARRIER_INST
#define NEW_BARRIER_INST
#endif

#if STEP < 2
#define PASS2IF(A, B) if(A)
#elif STEP == 2
#define PASS2IF(A, B) if(A) dyn->insts[ninst].pass2choice = B; if(dyn->insts[ninst].pass2choice == B)
#else
#define PASS2IF(A, B) if(dyn->insts[ninst].pass2choice == B)
#endif

void arm_epilog();
void* arm_next(x86emu_t* emu, uintptr_t addr);

#ifndef STEPNAME
#define STEPNAME3(N,M) N##M
#define STEPNAME2(N,M) STEPNAME3(N,M)
#define STEPNAME(N) STEPNAME2(N, STEP)
#endif

#define arm_pass        STEPNAME(arm_pass)

#define dynarec00       STEPNAME(dynarec00)
#define dynarec0F       STEPNAME(dynarec0F)
#define dynarecFS       STEPNAME(dynarecFS)
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
#define geted16         STEPNAME(geted16_)
#define jump_to_epilog  STEPNAME(jump_to_epilog_)
#define jump_to_next    STEPNAME(jump_to_next_)
#define ret_to_epilog   STEPNAME(ret_to_epilog_)
#define retn_to_epilog  STEPNAME(retn_to_epilog_)
#define iret_to_epilog  STEPNAME(iret_to_epilog_)
#define call_c          STEPNAME(call_c_)
#define grab_fsdata     STEPNAME(grab_fsdata_)
#define grab_tlsdata    STEPNAME(grab_tlsdata_)
#define emit_cmp8       STEPNAME(emit_cmp8)
#define emit_cmp16      STEPNAME(emit_cmp16)
#define emit_cmp32      STEPNAME(emit_cmp32)
#define emit_cmp8_0     STEPNAME(emit_cmp8_0)
#define emit_cmp16_0    STEPNAME(emit_cmp16_0)
#define emit_cmp32_0    STEPNAME(emit_cmp32_0)
#define emit_test8      STEPNAME(emit_test8)
#define emit_test16     STEPNAME(emit_test16)
#define emit_test32     STEPNAME(emit_test32)
#define emit_add32      STEPNAME(emit_add32)
#define emit_add32c     STEPNAME(emit_add32c)
#define emit_add8       STEPNAME(emit_add8)
#define emit_add8c      STEPNAME(emit_add8c)
#define emit_sub32      STEPNAME(emit_sub32)
#define emit_sub32c     STEPNAME(emit_sub32c)
#define emit_sub8       STEPNAME(emit_sub8)
#define emit_sub8c      STEPNAME(emit_sub8c)
#define emit_or32       STEPNAME(emit_or32)
#define emit_or32c      STEPNAME(emit_or32c)
#define emit_xor32      STEPNAME(emit_xor32)
#define emit_xor32c     STEPNAME(emit_xor32c)
#define emit_and32      STEPNAME(emit_and32)
#define emit_and32c     STEPNAME(emit_and32c)
#define emit_or8        STEPNAME(emit_or8)
#define emit_or8c       STEPNAME(emit_or8c)
#define emit_xor8       STEPNAME(emit_xor8)
#define emit_xor8c      STEPNAME(emit_xor8c)
#define emit_and8       STEPNAME(emit_and8)
#define emit_and8c      STEPNAME(emit_and8c)
#define emit_add16      STEPNAME(emit_add16)
#define emit_add16c     STEPNAME(emit_add16c)
#define emit_sub16      STEPNAME(emit_sub16)
#define emit_sub16c     STEPNAME(emit_sub16c)
#define emit_or16       STEPNAME(emit_or16)
#define emit_or16c      STEPNAME(emit_or16c)
#define emit_xor16      STEPNAME(emit_xor16)
#define emit_xor16c     STEPNAME(emit_xor16c)
#define emit_and16      STEPNAME(emit_and16)
#define emit_and16c     STEPNAME(emit_and16c)
#define emit_inc32      STEPNAME(emit_inc32)
#define emit_inc16      STEPNAME(emit_inc16)
#define emit_inc8       STEPNAME(emit_inc8)
#define emit_dec32      STEPNAME(emit_dec32)
#define emit_dec16      STEPNAME(emit_dec16)
#define emit_dec8       STEPNAME(emit_dec8)
#define emit_adc32      STEPNAME(emit_adc32)
#define emit_adc32c     STEPNAME(emit_adc32c)
#define emit_adc8       STEPNAME(emit_adc8)
#define emit_adc8c      STEPNAME(emit_adc8c)
#define emit_adc16      STEPNAME(emit_adc16)
#define emit_adc16c     STEPNAME(emit_adc16c)
#define emit_sbb32      STEPNAME(emit_sbb32)
#define emit_sbb32c     STEPNAME(emit_sbb32c)
#define emit_sbb8       STEPNAME(emit_sbb8)
#define emit_sbb8c      STEPNAME(emit_sbb8c)
#define emit_sbb16      STEPNAME(emit_sbb16)
#define emit_sbb16c     STEPNAME(emit_sbb16c)
#define emit_neg32      STEPNAME(emit_neg32)
#define emit_neg16      STEPNAME(emit_neg16)
#define emit_neg8       STEPNAME(emit_neg8)
#define emit_shl32      STEPNAME(emit_shl32)
#define emit_shl32c     STEPNAME(emit_shl32c)
#define emit_shr32      STEPNAME(emit_shr32)
#define emit_shr32c     STEPNAME(emit_shr32c)
#define emit_sar32c     STEPNAME(emit_sar32c)
#define emit_rol32c     STEPNAME(emit_rol32c)
#define emit_ror32c     STEPNAME(emit_ror32c)
#define emit_shrd32c    STEPNAME(emit_shrd32c)
#define emit_shld32c    STEPNAME(emit_shld32c)

#define emit_pf         STEPNAME(emit_pf)

#define x87_do_push     STEPNAME(x87_do_push)
#define x87_do_push_empty STEPNAME(x87_do_push_empty)
#define x87_do_pop      STEPNAME(x87_do_pop)
#define x87_get_cache   STEPNAME(x87_get_cache)
#define x87_get_st      STEPNAME(x87_get_st)
#define x87_refresh     STEPNAME(x87_refresh)
#define x87_forget      STEPNAME(x87_forget)
#define x87_reget_st    STEPNAME(x87_reget_st)
#define x87_stackcount  STEPNAME(x87_stackcount)
#define x87_setround    STEPNAME(x87_setround)
#define x87_restoreround STEPNAME(x87_restoreround)
#define mmx_get_reg     STEPNAME(mmx_get_reg)
#define mmx_get_reg_empty STEPNAME(mmx_get_reg_empty)
#define sse_get_reg     STEPNAME(sse_get_reg)
#define sse_get_reg_empty STEPNAME(sse_get_reg_empty)

#define fpu_pushcache   STEPNAME(fpu_pushcache)
#define fpu_popcache    STEPNAME(fpu_popcache)
#define fpu_reset       STEPNAME(fpu_reset)
#define fpu_purgecache  STEPNAME(fpu_purgecache)
#ifdef HAVE_TRACE
#define fpu_reflectcache STEPNAME(fpu_reflectcache)
#endif

// get the single reg that from the double "reg" (so Dx[idx])
#define fpu_get_single_reg      STEPNAME(fpu_get_single_reg)
// put back (if needed) the single reg in place
#define fpu_putback_single_reg  STEPNAME(fpu_putback_single_reg)

/* setup r2 to address pointed by */
uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int* fixedaddress, uint32_t absmax, uint32_t mask);

/* setup r2 to address pointed by */
uintptr_t geted16(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int* fixedaddress, uint32_t absmax, uint32_t mask);


// generic x86 helper
void jump_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst);
void jump_to_next(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst);
void ret_to_epilog(dynarec_arm_t* dyn, int ninst);
void retn_to_epilog(dynarec_arm_t* dyn, int ninst, int n);
void iret_to_epilog(dynarec_arm_t* dyn, int ninst);
void call_c(dynarec_arm_t* dyn, int ninst, void* fnc, int reg, int ret, uint32_t mask, int saveflags);
void grab_fsdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg);
void grab_tlsdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg);
void emit_lock(dynarec_arm_t* dyn, uintptr_t addr, int ninst);
void emit_unlock(dynarec_arm_t* dyn, uintptr_t addr, int ninst);
void emit_cmp8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_cmp16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_cmp32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_cmp8_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_cmp16_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_cmp32_0(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_test8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_test16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_test32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_add32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_add32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_add8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int save_s4);
void emit_add8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sub32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_sub32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sub8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int save_s4);
void emit_sub8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_or32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_or32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_xor32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_xor32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_and32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_and32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_or8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_or8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_xor8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_xor8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_and8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_and8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_add16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int save_s4);
void emit_add16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sub16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int save_s4);
void emit_sub16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_or16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_or16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_xor16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_xor16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_and16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_and16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_inc32(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_inc16(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_inc8(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_dec32(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_dec16(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_dec8(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_adc32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_adc32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_adc8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int save_s4);
void emit_adc8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_adc16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int save_s4);
void emit_adc16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sbb32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_sbb32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sbb8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int save_s4);
void emit_sbb8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sbb16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4, int save_s4);
void emit_sbb16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_neg32(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_neg16(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_neg8(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);
void emit_shl32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_shl32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_shr32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_shr32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sar32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_rol32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_ror32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_shrd32c(dynarec_arm_t* dyn, int ninst, int s1, int s2, int32_t c, int s3, int s4);
void emit_shld32c(dynarec_arm_t* dyn, int ninst, int s1, int s2, int32_t c, int s3, int s4);

void emit_pf(dynarec_arm_t* dyn, int ninst, int s1, int s3, int s4);

// x87 helper
// cache of the local stack counter, to avoid upadte at every call
void x87_stackcount(dynarec_arm_t* dyn, int ninst, int scratch);
// fpu push. Return the Dd value to be used
int x87_do_push(dynarec_arm_t* dyn, int ninst);
// fpu push. Do not allocate a cache register. Needs a scratch register to do x87stack synch (or 0 to not do it)
void x87_do_push_empty(dynarec_arm_t* dyn, int ninst, int s1);
// fpu pop. All previous returned Dd should be considered invalid
void x87_do_pop(dynarec_arm_t* dyn, int ninst);
// get cache index for a x87 reg, create the entry if needed
int x87_get_cache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a);
// get vfpu register for a x87 reg, create the entry if needed
int x87_get_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a);
// refresh a value from the cache ->emu (nothing done if value is not cached)
void x87_refresh(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st);
// refresh a value from the cache ->emu and then forget the cache (nothing done if value is not cached)
void x87_forget(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st);
// refresh the cache value from emu
void x87_reget_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st);
// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
// Restore round flag
void x87_restoreround(dynarec_arm_t* dyn, int ninst, int s1);

//MMX helpers
// get neon register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a);
// get neon register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a);

//SSE/SSE2 helpers
// get neon register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a);
// get neon register for a SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a);

// common coproc helpers
// reset the cache
void fpu_reset(dynarec_arm_t* dyn, int ninst);
// purge the FPU cache (needs 3 scratch registers)
void fpu_purgecache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
#ifdef HAVE_TRACE
void fpu_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
#endif
void fpu_pushcache(dynarec_arm_t* dyn, int ninst, int s1);
void fpu_popcache(dynarec_arm_t* dyn, int ninst, int s1);

// get the single reg that from the double "reg" (so Dx[idx])
int fpu_get_single_reg(dynarec_arm_t* dyn, int ninst, int reg, int idx);
// put back (if needed) the single reg in place
void fpu_putback_single_reg(dynarec_arm_t* dyn, int ninst, int reg, int idx, int s);

uintptr_t dynarec00(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarec0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
uintptr_t dynarecFS(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
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

#if STEP < 2
#define PASS2(A)
#else
#define PASS2(A)   A
#endif

#if STEP < 3
#define PASS3(A)
#else
#define PASS3(A)   A
#endif

#if STEP < 3
#define MAYUSE(A)   (void)A
#else
#define MAYUSE(A)   
#endif

#endif //__DYNAREC_ARM_HELPER_H__