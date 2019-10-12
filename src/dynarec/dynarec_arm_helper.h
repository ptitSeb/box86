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
#define F32     *(uint32_t*)(addr+=4, addr-4)
#define F32S    *(int32_t*)(addr+=4, addr-4)
#define PK(a)   *(uint8_t*)(addr+a)
#define PK32(a)   *(uint32_t*)(addr+a)
#define PK16(a)   *(uint16_t*)(addr+a)
#define PKip(a)   *(uint8_t*)(ip+a)

#define GETGD   gd = xEAX+((nextop&0x38)>>3)
//GETED can use r1 for ed, and r2 for wback. wback is 0 if ed is xEAX..xEDI
#define GETED   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    wback = 0;              \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2); \
                    LDR_IMM9(x1, wback, 0); \
                    ed = x1;                \
                }
//GETEDH can use hint for ed, and r1 or r2 for wback (depending on hint). wback is 0 if ed is xEAX..xEDI
#define GETEDH(hint)   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    wback = 0;              \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, (hint==x2)?x1:x2); \
                    LDR_IMM9(hint, wback, 0); \
                    ed = hint;              \
                }
//GETEDW can use hint for wback and ret for ed. wback is 0 if ed is xEAX..xEDI
#define GETEDW(hint, ret)   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    MOV_REG(ret, ed);       \
                    wback = 0;              \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback, hint); \
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
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2); \
                    LDR_REG_LSL_IMM5(x1, wback, O, 0);  \
                    ed = x1;                 \
                }
#define WBACKO(O)   if(wback) {STR_REG_LSL_IMM5(ed, wback, O, 0);}
#define FAKEED  if((nextop&0xC0)!=0xC0) {   \
                    addr = fakeed(dyn, addr, ninst, nextop); \
                }
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
#ifndef UFLAGS
#define UFLAGS(A)  dyn->cleanflags=A
#endif
#ifndef USEFLAG
// USEFLAG will check status of defered flags the call update flags if needed. x3 will be used
#define USEFLAG   \
    if(!dyn->cleanflags) {  \
        LDR_IMM9(x3, xEmu, offsetof(x86emu_t, df)); \
        TSTS_REG_LSL_IMM8(x3, x3, x3, 0);    \
        i32 = (GETMARKF)-(dyn->arm_size+8); \
        Bcond(cEQ, i32);    \
        CALL_(UpdateFlags, -1, 0); \
        MARKF;              \
        dyn->cleanflags=1;  \
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

void arm_epilog();
void* arm_linker(x86emu_t* emu, void** table, uintptr_t addr);

#ifndef STEPNAME
#define STEPNAME3(N,M) N##M
#define STEPNAME2(N,M) STEPNAME3(N,M)
#define STEPNAME(N) STEPNAME2(N, STEP)
#endif

#define arm_pass        STEPNAME(arm_pass)

#define dynarec0f       STEPNAME(dynarec0f)
#define dynarecGS       STEPNAME(dynarecGS)
#define dynarec66       STEPNAME(dynarec66)
#define dynarec660f     STEPNAME(dynarec660f)

#define geted           STEPNAME(geted_)
#define fakeed          STEPNAME(fakeed_)
#define jump_to_epilog  STEPNAME(jump_to_epilog_)
#define jump_to_linker  STEPNAME(jump_to_linker_)
#define ret_to_epilog   STEPNAME(ret_to_epilog_)
#define retn_to_epilog  STEPNAME(retn_to_epilog_)
#define call_c          STEPNAME(call_c_)
#define grab_tlsdata    STEPNAME(grab_tlsdata_)
#define isNativeCall    STEPNAME(isNativeCall_)

/* setup r2 to address pointed by */
uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed, uint8_t hint);

// Do the GETED, but don't emit anything...
uintptr_t fakeed(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop);

void jump_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst);
void jump_to_linker(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst);
void ret_to_epilog(dynarec_arm_t* dyn, int ninst);
void retn_to_epilog(dynarec_arm_t* dyn, int ninst, int n);
void call_c(dynarec_arm_t* dyn, int ninst, void* fnc, int reg, int ret, uint32_t mask);
void grab_tlsdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg);
int isNativeCall(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t* calladdress, int* retn);

uintptr_t dynarec0f(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int* ok, int* need_epilog);
uintptr_t dynarecGS(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int* ok, int* need_epilog);
uintptr_t dynarec66(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int* ok, int* need_epilog);
uintptr_t dynarec660f(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int* ok, int* need_epilog);

#endif //__DYNAREC_ARM_HELPER_H__