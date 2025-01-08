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
#define PK32(a) *(uint32_t*)(addr+a)
#define PK16(a) *(uint16_t*)(addr+a)
#define PKip(a) *(uint8_t*)(ip+a)
#define PKa(a)  *(uint8_t*)(a)

#define MODREG  ((nextop&0xC0)==0xC0)

// Strong mem emulation helpers
#define SMREAD_MIN  2
#define SMWRITE_MIN 1
// Sequence of Read will trigger a DMB on "first" read if strongmem is >= SMREAD_MIN
// Sequence of Write will trigger a DMB on "last" write if strongmem is >= 1
// All Write operation that might use a lock all have a memory barrier if strongmem is >= SMWRITE_MIN
// Opcode will read
#define SMREAD()    if((dyn->smread==0) && (box86_dynarec_strongmem>SMREAD_MIN)) {SMDMB();} else dyn->smread=1
// Opcode will read with option forced lock
#define SMREADLOCK(lock)    if((lock) || ((dyn->smread==0) && (box86_dynarec_strongmem>SMREAD_MIN))) {SMDMB();}
// Opcode might read (depend on nextop)
#define SMMIGHTREAD()   if(!MODREG) {SMREAD();}
// Opcode has wrote
#define SMWRITE()   dyn->smwrite=1
// Opcode has wrote (strongmem>1 only)
#define SMWRITE2()   if(box86_dynarec_strongmem>SMREAD_MIN) dyn->smwrite=1
// Opcode has wrote with option forced lock
#define SMWRITELOCK(lock)   if(lock || (box86_dynarec_strongmem>SMWRITE_MIN)) {SMDMB();} else dyn->smwrite=1
// Opcode might have wrote (depend on nextop)
#define SMMIGHTWRITE()   if(!MODREG) {SMWRITE();}
// Start of sequence
#define SMSTART()   SMEND()
// End of sequence
#define SMEND()     if(dyn->smwrite && box86_dynarec_strongmem) {if(box86_dynarec_strongmem){DSB_ISH();}else{DMB_ISH();}} dyn->smwrite=0; dyn->smread=0;
// Force a Data memory barrier (for LOCK: prefix)
#define SMDMB()     if(box86_dynarec_strongmem){DSB_ISH();}else{DMB_ISH();} dyn->smwrite=0; dyn->smread=1

//LOCK_* define
#define LOCK_LOCK   (int*)1

// GETGD    get x86 register in gd
#define GETGD   gd = xEAX+((nextop&0x38)>>3)
//GETED can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETED   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    wback = 0;              \
                } else {                    \
                    SMREAD();               \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 4095, 0, 0, NULL); \
                    LDR_IMM9(x1, wback, fixedaddress); \
                    ed = x1;                \
                }
//GETEDH can use hint for ed, and r1 or r2 for wback (depending on hint). wback is 0 if ed is xEAX..xEDI
#define GETEDH(hint)   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    wback = 0;              \
                } else {                    \
                    SMREAD();               \
                    addr = geted(dyn, addr, ninst, nextop, &wback, (hint==x2)?x1:x2, &fixedaddress, 4095, 0, 0, NULL); \
                    LDR_IMM9(hint, wback, fixedaddress); \
                    ed = hint;              \
                }
//GETEDW can use hint for wback and ret for ed. wback is 0 if ed is xEAX..xEDI
#define GETEDW(hint, ret)   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    MOV_REG(ret, ed);       \
                    wback = 0;              \
                } else {                    \
                    SMREAD();               \
                    addr = geted(dyn, addr, ninst, nextop, &wback, hint, &fixedaddress, 4095, 0, 0, NULL); \
                    ed = ret;               \
                    LDR_IMM9(ed, wback, fixedaddress); \
                }
// Write back ed in wback (if wback not 0)
#define WBACK       if(wback) {STR_IMM9(ed, wback, fixedaddress); SMWRITE();}
// Write back ed in wback (if wback not 0) (SMWRITE2 version)
#define WBACK2      if(wback) {STR_IMM9(ed, wback, fixedaddress); SMWRITE2();}
//GETEDO can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETEDO(O)   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    wback = 0;              \
                } else {                    \
                    SMREAD();               \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, NULL); \
                    LDR_REG_LSL_IMM5(x1, wback, O, 0);  \
                    ed = x1;                 \
                }
//GETEDO can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI, else O is added to wback
#define GETEDO2(O) if((nextop&0xC0)==0xC0) {            \
                    ed = xEAX+(nextop&7);               \
                    wback = 0;                          \
                } else {                                \
                    SMREAD();                           \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, NULL); \
                    ADD_REG_LSL_IMM5(x2, wback, O, 0);  \
                    if(wback != x2) wback = x2;         \
                    LDR_IMM9(x1, wback, 0);             \
                    ed = x1;                            \
                }
#define WBACKO(O)   if(wback) {STR_REG_LSL_IMM5(ed, wback, O, 0); SMWRITE();}
//FAKEELike GETED, but doesn't get anything
#define FAKEED  if((nextop&0xC0)!=0xC0) {   \
                    addr = fakeed(dyn, addr, ninst, nextop); \
                }
// GETGW extract x86 register in gd, that is i
#define GETGW(i) gd = xEAX+((nextop&0x38)>>3); UXTH(i, gd, 0); gd = i;
// GETSGW extract x86 signed register in gd, that is i
#define GETSGW(i) gd = xEAX+((nextop&0x38)>>3); SXTH(i, gd, 0); gd = i;
//GETEWW will use i for ed, and can use w for wback.
#define GETEWW(w, i) if((nextop&0xC0)==0xC0) {  \
                    wback = xEAX+(nextop&7);\
                    UXTH(i, wback, 0);      \
                    ed = i;                 \
                    wb1 = 0;                \
                } else {                    \
                    SMREAD();               \
                    addr = geted(dyn, addr, ninst, nextop, &wback, w, &fixedaddress, 255, 0, 0, NULL); \
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
                    SMREAD();               \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0, 0, NULL); \
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
                    SMREAD();               \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0, 0, NULL); \
                    LDRSH_IMM8(i, wback, fixedaddress);\
                    ed = i;                 \
                    wb1 = 1;                \
                }
// Write ed back to original register / memory
#define EWBACK   if(wb1) {STRH_IMM8(ed, wback, fixedaddress); SMWRITE();} else {BFI(wback, ed, 0, 16);}
// Write w back to original register / memory
#define EWBACKW(w)   if(wb1) {STRH_IMM8(w, wback, fixedaddress); SMWRITE();} else {BFI(wback, w, 0, 16);}
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
                    SMREAD();               \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 4095, 0, 0, NULL); \
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
                    SMREAD();               \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, NULL); \
                    ADD_REG_LSL_IMM5(x3, wback, i, 0);  \
                    if(wback!=x3) wback = x3;           \
                    LDRB_IMM9(i, wback, fixedaddress);  \
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
                    SMREAD();               \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0, 0, NULL); \
                    LDRSB_IMM8(i, wback, fixedaddress);\
                    wb1 = 1;                \
                    ed = i;                 \
                }
// Write eb (ed) back to original register / memory
#define EBBACK   if(wb1) {STRB_IMM9(ed, wback, fixedaddress); SMWRITE();} else {BFI(wback, ed, wb2*8, 8);}
// Write eb (ed) back to original register / memory (SMWRITE2 version)
#define EBBACK2   if(wb1) {STRB_IMM9(ed, wback, fixedaddress); SMWRITE2();} else {BFI(wback, ed, wb2*8, 8);}
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
// CALL_1D will use x3 for the call address. Return value in D0, 1 ARG in D0
#define CALL_1D(F, M) call_d(dyn, ninst, F, NULL, 1, x3, -1, M, 0)
// CALL_2D will use x3 for the call address. Return value in D0, 2 ARGs in D0, D1
#define CALL_2D(F, M) call_d(dyn, ninst, F, NULL, 2, x3, -1, M, 0)
// CALL_1DD will use x3 for the call address. Return value in D0, 1 ARG in D0, cals 2 functions in a row
#define CALL_1DD(F, F2, M) call_d(dyn, ninst, F, F2, 1, x3, -1, M, 0)
// CALL_1D_U64 will use S as scratch. Return value in ret/ret2, 1 ARG in D0
#define CALL_1DR_U64(R, ret, ret2, S, M) call_dr(dyn, ninst, R, 1, S, ret, ret2, M, 1)
// CALL_1DR will use S as scratch. Return value in D0, 1 ARG in D0
#define CALL_1DR(R, S, M) call_dr(dyn, ninst, R, 1, S, -1, -1, M, 0)
// CALL_1DI will use S as scratch. Return value in D0, 1 ARG in D0, 1 ARG in R0
#define CALL_1DDR(F, R, S, M) call_ddr(dyn, ninst, F, NULL, R, S, -1, M, 0)
// CALL_1RD will use S as scratch. Return value in D0, 1 ARG in R
#define CALL_1RD(F, R, S, M) call_rd(dyn, ninst, F, R, S, M, 0);

#define MARK        if(dyn->insts) {dyn->insts[ninst].mark = (uintptr_t)dyn->arm_size;}
#define GETMARK     ((dyn->insts)?dyn->insts[ninst].mark:(dyn->arm_size+4))
#define MARK2       if(dyn->insts) {dyn->insts[ninst].mark2 = (uintptr_t)dyn->arm_size;}
#define GETMARK2    ((dyn->insts)?dyn->insts[ninst].mark2:(dyn->arm_size+4))
#define MARK3       if(dyn->insts) {dyn->insts[ninst].mark3 = (uintptr_t)dyn->arm_size;}
#define GETMARK3    ((dyn->insts)?dyn->insts[ninst].mark3:(dyn->arm_size+4))
#define MARKF       if(dyn->insts) {dyn->insts[ninst].markf = (uintptr_t)dyn->arm_size;}
#define GETMARKF    ((dyn->insts)?dyn->insts[ninst].markf:(dyn->arm_size+4))
#define MARKF2      if(dyn->insts) {dyn->insts[ninst].markf2 = (uintptr_t)dyn->arm_size;}
#define GETMARKF2   ((dyn->insts)?dyn->insts[ninst].markf2:(dyn->arm_size+4))
#define MARKSEG     if(dyn->insts) {dyn->insts[ninst].markseg = (uintptr_t)dyn->arm_size;}
#define GETMARKSEG  ((dyn->insts)?dyn->insts[ninst].markseg:(dyn->arm_size+4))
#define MARKLOCK    if(dyn->insts) {dyn->insts[ninst].marklock = (uintptr_t)dyn->arm_size;}
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
    j32 = dyn->insts[ninst].epilog-(dyn->arm_size+8); \
    Bcond(cond, j32)
// Branch to MARKSEG if cond (use j32)
#define B_MARKSEG(cond)    \
    j32 = GETMARKSEG-(dyn->arm_size+8);   \
    Bcond(cond, j32)
// Branch to MARKLOCK if cond (use j32)
#define B_MARKLOCK(cond)    \
    j32 = GETMARKLOCK-(dyn->arm_size+8);   \
    Bcond(cond, j32)

#define IFX(A)  if((dyn->insts[ninst].x86.gen_flags&(A)))
#define IFX_PENDOR0  if((dyn->insts[ninst].x86.gen_flags&(X_PEND) || !dyn->insts[ninst].x86.gen_flags))
#define IFXX(A) if((dyn->insts[ninst].x86.gen_flags==(A)))
#define IFX2X(A, B) if((dyn->insts[ninst].x86.gen_flags==(A) || dyn->insts[ninst].x86.gen_flags==(B) || dyn->insts[ninst].x86.gen_flags==((A)|(B))))
#define IFXN(A, B)  if((dyn->insts[ninst].x86.gen_flags&(A) && !(dyn->insts[ninst].x86.gen_flags&(B))))

// Generate FCOM with s1 and s2 scratch regs (the VCMP is already done)
#define FCOM(s1, s2)                                                            \
    VMRS_APSR();    /* 0b0100011100000000 */                                    \
    LDRH_IMM8(s1, xEmu, offsetof(x86emu_t, sw));   /*offset is 8bits right?*/   \
    BIC_IMM8(s1, s1, 0b01000111, 12);                                           \
    ORR_IMM8_COND(cVS, s1, s1, 0b01000101, 12); /* unordered */                 \
    ORR_IMM8_COND(cEQ, s1, s1, 0b01000000, 12); /* equal */                     \
    ORR_IMM8_COND(cMI, s1, s1, 0b00000001, 12); /* less than */                 \
    /* greater than leave 0 */                                                  \
    STRH_IMM8(s1, xEmu, offsetof(x86emu_t, sw))

// Generate FCOMI with s1 and s2 scratch regs (the VCMP is already done)
#define FCOMI(s1, s2)                                                       \
    IFX(X_CF|X_PF|X_ZF|X_PEND) {                                            \
        VMRS_APSR();    /* 0b111 */                                         \
        BIC_IMM8(xFlags, xFlags, 0b1000101, 0);                             \
        ORR_IMM8_COND(cVS, xFlags, xFlags, 0b01000101, 0); /* unordered */  \
        ORR_IMM8_COND(cEQ, xFlags, xFlags, 0b01000000, 0); /* zero */       \
        ORR_IMM8_COND(cMI, xFlags, xFlags, 0b00000001, 0); /* less than */  \
        /* greater than leave 0 */                                          \
        /* clear 0b100010010000 */                                          \
        BIC_IMM8(xFlags, xFlags, 0b10001001, 14);                           \
    }                                                                       \
    SET_DFNONE(s1);                                                         \


#if STEP == 0
#define X87_PUSH_OR_FAIL(var, dyn, ninst, scratch, t)   var = x87_do_push(dyn, ninst, scratch, t)
#define X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, scratch)     x87_do_push_empty(dyn, ninst, scratch)
#define X87_POP_OR_FAIL(dyn, ninst, scratch)            x87_do_pop(dyn, ninst, scratch)
#else
#define X87_PUSH_OR_FAIL(var, dyn, ninst, scratch, t)   \
    if ((dyn->n.x87stack==8) || (dyn->n.pushed==8)) {   \
        if(box86_dynarec_dump) dynarec_log(LOG_NONE, " Warning, suspicious x87 Push, stack=%d/%d on inst %d\n", dyn->n.x87stack, dyn->n.pushed, ninst); \
        dyn->abort = 1;                                 \
        return addr;                                    \
    }                                                   \
    var = x87_do_push(dyn, ninst, scratch, t)

#define X87_PUSH_EMPTY_OR_FAIL(dyn, ninst, scratch)     \
    if ((dyn->n.x87stack==8) || (dyn->n.pushed==8)) {   \
        if(box86_dynarec_dump) dynarec_log(LOG_NONE, " Warning, suspicious x87 Push, stack=%d/%d on inst %d\n", dyn->n.x87stack, dyn->n.pushed, ninst); \
        dyn->abort = 1;                                 \
        return addr;                                    \
    }                                                   \
    x87_do_push_empty(dyn, ninst, scratch)

#define X87_POP_OR_FAIL(dyn, ninst, scratch)            \
    if ((dyn->n.x87stack==-8) || (dyn->n.poped==8)) {   \
        if(box86_dynarec_dump) dynarec_log(LOG_NONE, " Warning, suspicious x87 Pop, stack=%d/%d on inst %d\n", dyn->n.x87stack, dyn->n.poped, ninst); \
        dyn->abort = 1;                                 \
        return addr;                                    \
    }                                                   \
    x87_do_pop(dyn, ninst, scratch)
#endif


#define SET_DFNONE(S)    if(!dyn->f.dfnone) {MOVW(S, d_none); STR_IMM9(S, xEmu, offsetof(x86emu_t, df)); dyn->f.dfnone=1;}
#define SET_DF(S, N)     \
    if(N) {             \
        MOVW(S, N);\
        STR_IMM9(S, xEmu, offsetof(x86emu_t, df));  \
        if(dyn->f.pending==SF_PENDING && dyn->insts[ninst].x86.need_after && !(dyn->insts[ninst].x86.need_after&X_PEND)) {  \
            CALL_(UpdateFlags, -1, 0);              \
            dyn->f.pending = SF_SET;                \
            SET_NODF();     \
        }                   \
        dyn->f.dfnone=0;\
    } else SET_DFNONE(S)
#define SET_NODF()          dyn->f.dfnone = 0
#define SET_DFOK()          dyn->f.dfnone = 1

#ifndef MAYSETFLAGS
#define MAYSETFLAGS()
#endif

#ifndef READFLAGS
#define READFLAGS(A) \
    if(((A)!=X_PEND && dyn->f.pending!=SF_SET)          \
    && (dyn->f.pending!=SF_SET_PENDING)) {              \
        if(dyn->f.pending!=SF_PENDING) {                \
            LDR_IMM9(x3, xEmu, offsetof(x86emu_t, df)); \
            TSTS_REG_LSL_IMM5(x3, x3, 0);               \
            j32 = (GETMARKF)-(dyn->arm_size+8);         \
            Bcond(cEQ, j32);                            \
        }                                               \
        CALL_(UpdateFlags, -1, 0);                      \
        MARKF;                                          \
        dyn->f.pending = SF_SET;                        \
        SET_DFOK();                                     \
    }
#endif

#ifndef SETFLAGS
#define SETFLAGS(A, B)                                                                          \
    if(dyn->f.pending!=SF_SET                                                                   \
    && ((B)&SF_SUB)                                                                             \
    && (dyn->insts[ninst].x86.gen_flags&(~(A))))                                               \
        READFLAGS(((dyn->insts[ninst].x86.gen_flags&X_PEND)?X_ALL:dyn->insts[ninst].x86.gen_flags)&(~(A)));\
    if(dyn->insts[ninst].x86.gen_flags) switch(B) {                                            \
        case SF_SUBSET:                                                                         \
        case SF_SET: dyn->f.pending = SF_SET; break;                                            \
        case SF_SET_DF: dyn->f.pending = SF_SET; dyn->f.dfnone = 1; break;                      \
        case SF_SET_NODF: dyn->f.pending = SF_SET; dyn->f.dfnone = 0; break;                    \
        case SF_PENDING: dyn->f.pending = SF_PENDING; break;                                    \
        case SF_SUBSET_PENDING:                                                                 \
        case SF_SET_PENDING:                                                                    \
            dyn->f.pending = (dyn->insts[ninst].x86.gen_flags&X_PEND)?SF_SET_PENDING:SF_SET;   \
            break;                                                                              \
    } else dyn->f.pending = SF_SET
#endif
#ifndef JUMP
#define JUMP(A, C) SMEND()
#endif
#ifndef BARRIER
#define BARRIER(A) 
#endif
#ifndef BARRIER_NEXT
#define BARRIER_NEXT(A)
#endif
#ifndef SET_HASCALLRET
#define SET_HASCALLRET()
#endif
#define UFLAG_OP1(A) if(dyn->insts[ninst].x86.gen_flags) {STR_IMM9(A, xEmu, offsetof(x86emu_t, op1));}
#define UFLAG_OP2(A) if(dyn->insts[ninst].x86.gen_flags) {STR_IMM9(A, xEmu, offsetof(x86emu_t, op2));}
#define UFLAG_OP12(A1, A2) if(dyn->insts[ninst].x86.gen_flags) {STR_IMM9(A1, xEmu, offsetof(x86emu_t, op1));STR_IMM9(A2, 0, offsetof(x86emu_t, op2));}
#define UFLAG_RES(A) if(dyn->insts[ninst].x86.gen_flags) {STR_IMM9(A, xEmu, offsetof(x86emu_t, res));}
#define UFLAG_DF(r, A) if(dyn->insts[ninst].x86.gen_flags) {SET_DF(r, A)}
#define UFLAG_IF if(dyn->insts[ninst].x86.gen_flags)
#ifndef DEFAULT
#define DEFAULT      *ok = -1; BARRIER(BARRIER_NOFLAGS)
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
#define dynarec66F0     STEPNAME(dynarec66F0)
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
#define call_d          STEPNAME(call_d_)
#define call_ddr        STEPNAME(call_ddr_)
#define call_dr         STEPNAME(call_dr_)
#define call_rd         STEPNAME(call_rd_)
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
#define emit_shl8       STEPNAME(emit_shl8)
#define emit_shl8c      STEPNAME(emit_shl8c)
#define emit_shr8       STEPNAME(emit_shr8)
#define emit_shr8c      STEPNAME(emit_shr8c)
#define emit_sar8       STEPNAME(emit_sar8)
#define emit_sar8c      STEPNAME(emit_sar8c)
#define emit_shl16      STEPNAME(emit_shl16)
#define emit_shl16c     STEPNAME(emit_shl16c)
#define emit_shr16      STEPNAME(emit_shr16)
#define emit_shr16c     STEPNAME(emit_shr16c)
#define emit_sar16      STEPNAME(emit_sar16)
#define emit_sar16c     STEPNAME(emit_sar16c)
#define emit_rol32c     STEPNAME(emit_rol32c)
#define emit_rol8c      STEPNAME(emit_rol8c)
#define emit_ror32c     STEPNAME(emit_ror32c)
#define emit_ror8c      STEPNAME(emit_ror8c)
#define emit_rol16c     STEPNAME(emit_rol16c)
#define emit_ror16c     STEPNAME(emit_ror16c)
#define emit_shrd32c    STEPNAME(emit_shrd32c)
#define emit_shld32c    STEPNAME(emit_shld32c)
#define emit_shrd32     STEPNAME(emit_shrd32)
#define emit_shld32     STEPNAME(emit_shld32)

#define emit_pf         STEPNAME(emit_pf)

#define x87_do_push     STEPNAME(x87_do_push)
#define x87_do_push_empty STEPNAME(x87_do_push_empty)
#define x87_do_pop      STEPNAME(x87_do_pop)
#define x87_get_current_cache   STEPNAME(x87_get_current_cache)
#define x87_get_cache   STEPNAME(x87_get_cache)
#define x87_get_neoncache STEPNAME(x87_get_neoncache)
#define x87_get_st      STEPNAME(x87_get_st)
#define x87_get_st_empty  STEPNAME(x87_get_st)
#define x87_free        STEPNAME(x87_free)
#define x87_refresh     STEPNAME(x87_refresh)
#define x87_forget      STEPNAME(x87_forget)
#define x87_reget_st    STEPNAME(x87_reget_st)
#define x87_stackcount  STEPNAME(x87_stackcount)
#define x87_unstackcount      STEPNAME(x87_unstackcount)
#define x87_setround    STEPNAME(x87_setround)
#define x87_setround_reset    STEPNAME(x87_setround_reset)
#define x87_restoreround STEPNAME(x87_restoreround)
#define x87_swapreg     STEPNAME(x87_swapreg)
#define sse_setround    STEPNAME(sse_setround)
#define sse_setround_reset    STEPNAME(sse_setround_reset)
#define mmx_get_reg     STEPNAME(mmx_get_reg)
#define mmx_get_reg_empty STEPNAME(mmx_get_reg_empty)
#define sse_get_reg     STEPNAME(sse_get_reg)
#define sse_get_reg_empty STEPNAME(sse_get_reg_empty)
#define sse_forget_reg  STEPNAME(sse_forget_reg)
#define sse_reflect_reg STEPNAME(sse_reflect_reg)

#define fpu_pushcache   STEPNAME(fpu_pushcache)
#define fpu_popcache    STEPNAME(fpu_popcache)
#define fpu_reset_cache STEPNAME(fpu_reset_cache)
#define fpu_propagate_stack STEPNAME(fpu_propagate_stack)
#define fpu_purgecache  STEPNAME(fpu_purgecache)
#define x87_purgecache  STEPNAME(x87_purgecache)
#define mmx_purgecache  STEPNAME(mmx_purgecache)
#define fpu_reflectcache STEPNAME(fpu_reflectcache)
#define fpu_unreflectcache STEPNAME(fpu_unreflectcache)

// get the single reg that from the double "reg" (so Dx[idx])
#define fpu_get_single_reg      STEPNAME(fpu_get_single_reg)
// put back (if needed) the single reg in place
#define fpu_putback_single_reg  STEPNAME(fpu_putback_single_reg)

#define CacheTransform       STEPNAME(CacheTransform)

/* setup r2 to address pointed by */
uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int* fixedaddress, uint32_t absmax, uint32_t mask, int getfixonly, int* l);

/* setup r2 to address pointed by */
uintptr_t geted16(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint, int* fixedaddress, uint32_t absmax, uint32_t mask);


// generic x86 helper
void jump_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst);
void jump_to_next(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst);
void ret_to_epilog(dynarec_arm_t* dyn, int ninst);
void retn_to_epilog(dynarec_arm_t* dyn, int ninst, int n);
void iret_to_epilog(dynarec_arm_t* dyn, int ninst);
void call_c(dynarec_arm_t* dyn, int ninst, void* fnc, int reg, int ret, uint32_t mask, int saveflags);
void call_d(dynarec_arm_t* dyn, int ninst, void* fnc, void* fnc2, int n, int reg, int ret, uint32_t mask, int saveflags);
void call_dr(dynarec_arm_t* dyn, int ninst, int reg, int n, int s1, int ret, int ret2, uint32_t mask, int saveflags);
void call_ddr(dynarec_arm_t* dyn, int ninst, void* fnc, void* fnc2, int arg, int reg, int ret, uint32_t mask, int saveflags);
void call_rd(dynarec_arm_t* dyn, int ninst, void* fnc, int reg, int s1, uint32_t mask, int saveflags);
void grab_fsdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg);
void grab_tlsdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg);
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
void emit_shl8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_shl8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_shr8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_shr8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sar8(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_sar8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_shl16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_shl16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_shr16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_shr16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_sar16(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_sar16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_rol32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_rol8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_ror32c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_ror8c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_rol16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_ror16c(dynarec_arm_t* dyn, int ninst, int s1, int32_t c, int s3, int s4);
void emit_shrd32c(dynarec_arm_t* dyn, int ninst, int s1, int s2, int32_t c, int s3, int s4);
void emit_shld32c(dynarec_arm_t* dyn, int ninst, int s1, int s2, int32_t c, int s3, int s4);
void emit_shrd32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);
void emit_shld32(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int s4);

void emit_pf(dynarec_arm_t* dyn, int ninst, int s1, int s4);

// x87 helper
// cache of the local stack counter, to avoid upadte at every call
int x87_stackcount(dynarec_arm_t* dyn, int ninst, int scratch);
// restore the local stack counter
void x87_unstackcount(dynarec_arm_t* dyn, int ninst, int scratch, int count);
// fpu push. Return the Dd value to be used
int x87_do_push(dynarec_arm_t* dyn, int ninst, int s1, int t);
// fpu push. Do not allocate a cache register. Needs a scratch register to do x87stack synch (or 0 to not do it)
void x87_do_push_empty(dynarec_arm_t* dyn, int ninst, int s1);
// fpu pop. All previous returned Dd should be considered invalid
void x87_do_pop(dynarec_arm_t* dyn, int ninst, int s1);
// get cache index for a x87 reg, return -1 if cache doesn't exist
int x87_get_current_cache(dynarec_arm_t* dyn, int ninst, int st, int t);
// get cache index for a x87 reg, create the entry if needed
int x87_get_cache(dynarec_arm_t* dyn, int ninst, int populate, int s1, int s2, int a, int t);
// get neoncache index for a x87 reg
int x87_get_neoncache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a);
// get vfpu register for a x87 reg, create the entry if needed
int x87_get_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a, int t);
// get vfpu register for a x87 reg, create the entry if needed. Do not fetch the Stx if not already in cache
int x87_get_st_empty(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a, int t);
// Free st, using the FFREE opcode (so it's freed but stack is not moved)
void x87_free(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int st);
// refresh a value from the cache ->emu (nothing done if value is not cached)
void x87_refresh(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st);
// refresh a value from the cache ->emu and then forget the cache (nothing done if value is not cached)
void x87_forget(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st);
// refresh the cache value from emu
void x87_reget_st(dynarec_arm_t* dyn, int ninst, int s1, int s2, int st);
// Set rounding according to cw flags, return reg to restore flags
int x87_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
// Set rounding according to cw flags, return reg to restore flags, also enable exceptions and reset counters
int x87_setround_reset(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
// Restore round flag
void x87_restoreround(dynarec_arm_t* dyn, int ninst, int s1);
// swap 2 x87 regs
void x87_swapreg(dynarec_arm_t* dyn, int ninst, int s1, int s2, int a, int b);
// Set rounding according to mxcsr flags, return reg to restore flags
int sse_setround(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
// Set rounding according to mxcsr flags, return reg to restore flags, also enable exceptions and reset counters
int sse_setround_reset(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);

void CacheTransform(dynarec_arm_t* dyn, int ninst, int cacheupd, int s1, int s2, int s3);

#if STEP < 2
#define CHECK_CACHE()   0
#else
#define CHECK_CACHE()   (cacheupd = CacheNeedsTransform(dyn, ninst))
#endif

#define neoncache_st_coherency STEPNAME(neoncache_st_coherency)
int neoncache_st_coherency(dynarec_arm_t* dyn, int ninst, int a, int b);

#if STEP == 0
#define ST_IS_F(A)          0
#define X87_COMBINE(A, B)   NEON_CACHE_ST_D
#define X87_ST0             NEON_CACHE_ST_D
#define X87_ST(A)           NEON_CACHE_ST_D
#elif STEP == 1
#define ST_IS_F(A) (neoncache_get_current_st(dyn, ninst, A)==NEON_CACHE_ST_F)
#define X87_COMBINE(A, B) neoncache_combine_st(dyn, ninst, A, B)
#define X87_ST0     neoncache_get_current_st(dyn, ninst, 0)
#define X87_ST(A)   neoncache_get_current_st(dyn, ninst, A)
#else
#define ST_IS_F(A) (neoncache_get_st(dyn, ninst, A)==NEON_CACHE_ST_F)
#if STEP == 3
#define X87_COMBINE(A, B) neoncache_st_coherency(dyn, ninst, A, B)
#else
#define X87_COMBINE(A, B) neoncache_get_st(dyn, ninst, A)
#endif
#define X87_ST0     neoncache_get_st(dyn, ninst, 0)
#define X87_ST(A)   neoncache_get_st(dyn, ninst, A)
#endif

//MMX helpers
// get neon register for a MMX reg, create the entry if needed
int mmx_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int a);
// get neon register for a MMX reg, but don't try to synch it if it needed to be created
int mmx_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3, int a);

//SSE/SSE2 helpers
// get neon register for a SSE reg, create the entry if needed
int sse_get_reg(dynarec_arm_t* dyn, int ninst, int s1, int a, int forwrite);
// get neon register for a SSE reg, but don't try to synch it if it needed to be created
int sse_get_reg_empty(dynarec_arm_t* dyn, int ninst, int s1, int a);
// forget an SSE reg
void sse_forget_reg(dynarec_arm_t* dyn, int ninst, int a, int s1);
// sync reg with emu for sse reg (return 1 if reflect was done, s1 = &xmm[a] then)
int sse_reflect_reg(dynarec_arm_t* dyn, int ninst, int a, int s1);

// common coproc helpers
// reset the cache with n
void fpu_reset_cache(dynarec_arm_t* dyn, int ninst, int reset_n);
// propagate stack state
void fpu_propagate_stack(dynarec_arm_t* dyn, int ninst);
// purge the FPU cache (needs 3 scratch registers) next=1 if for a conditionnal branch jumping out of block (no tracking updated)
void fpu_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1, int s2, int s3);
void x87_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1, int s2, int s3);
void mmx_purgecache(dynarec_arm_t* dyn, int ninst, int next, int s1);
void fpu_reflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
void fpu_unreflectcache(dynarec_arm_t* dyn, int ninst, int s1, int s2, int s3);
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
uintptr_t dynarec66F0(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog);
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

#define NOTEST(s1, s2)                                      \
    if(box86_dynarec_test) {                                \
        MOVW(s1, 0);                                        \
        MOVW(s2, offsetof(x86emu_t, test.test));            \
        STR_REG_LSL_IMM5(s1, xEmu, s2, 0);                  \
        ADD_IMM8(s2, s2, offsetof(x86emu_t, test.clean)-offsetof(x86emu_t, test.test));\
        STR_REG_LSL_IMM5(s1, xEmu, s2, 0);                  \
    }
#define SKIPTEST(s1, s2)                                    \
    if(box86_dynarec_test) {                                \
        MOVW(s1, 0);                                        \
        MOVW(s2, offsetof(x86emu_t, test.clean));           \
        STR_REG_LSL_IMM5(s1, xEmu, s2, 0);                  \
    }
#define GOTEST(s1, s2)                                      \
    if(box86_dynarec_test) {                                \
        MOVW(s2, 1);                                        \
        MOVW(s1, offsetof(x86emu_t, test.test));            \
        STR_REG_LSL_IMM5(s2, xEmu, s1, 0);                  \
    }

#endif //__DYNAREC_ARM_HELPER_H__