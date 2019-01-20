#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"


void Run0F(x86emu_t *emu)
{
    uint8_t opcode = Fetch8(emu);
    uint8_t nextop;
    reg32_t *op1, *op2, *op3, *op4;
    reg32_t ea1, ea2, ea3, ea4;
    uint8_t tmp8u;
    int8_t tmp8s;
    uint16_t tmp16u;
    int16_t tmp16s;
    uint32_t tmp32u;
    uint32_t tmp32u2;
    int32_t tmp32s;
    uint64_t tmp64u;
    int64_t tmp64s;
    sse_regs_t *opx1, *opx2;
    sse_regs_t eax1;
    switch(opcode) {
        case 0x10:                      /* MOVUPS Gd,Ed */
            nextop = Fetch8(emu);
            GetEx(emu, &opx2, nextop);
            GetGx(emu, &opx1, nextop);
            memcpy(opx1, opx2, sizeof(sse_regs_t));
            break;
        case 0x11:                      /* MOVUPS Ed,Gd */
            nextop = Fetch8(emu);
            GetEx(emu, &opx1, nextop);
            GetGx(emu, &opx2, nextop);
            memcpy(opx1, opx2, sizeof(sse_regs_t));
            break;
        case 0x12:                      /* MOVLPS Gd,Ed */
            nextop = Fetch8(emu);
            GetEx(emu, &opx2, nextop);
            GetGx(emu, &opx1, nextop);
            opx1->q[0] = opx2->q[0];
            break;
        case 0x13:                      /* MOVLPS Ed,Gd */
            nextop = Fetch8(emu);
            GetEx(emu, &opx1, nextop);
            GetGx(emu, &opx2, nextop);
            opx1->q[0] = opx2->q[0];
            break;

        case 0x16:                      /* MOVHPS Ed,Gd */
            nextop = Fetch8(emu);
            GetEx(emu, &opx1, nextop);
            GetGx(emu, &opx2, nextop);
            opx2->q[1] = opx1->q[0];
            break;
        case 0x17:                      /* MOVHPS Gd,Ed */
            nextop = Fetch8(emu);
            GetEx(emu, &opx2, nextop);
            GetGx(emu, &opx1, nextop);
            opx2->q[0] = opx1->q[1];
            break;

        case 0x28:                      /* MOVAPS Gd,Ed */
            nextop = Fetch8(emu);
            GetEx(emu, &opx2, nextop);
            GetGx(emu, &opx1, nextop);
            memcpy(opx1, opx2, sizeof(sse_regs_t));
            break;
        case 0x29:                      /* MOVAPS Ed,Gd */
            nextop = Fetch8(emu);
            GetEx(emu, &opx1, nextop);
            GetGx(emu, &opx2, nextop);
            memcpy(opx1, opx2, sizeof(sse_regs_t));
            break;
        
        #define GOCOND(BASE, PREFIX, CONDITIONAL) \
        case BASE+0x0:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_OF))               \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x1:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_OF))              \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x2:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_CF))               \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x3:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_CF))              \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x4:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_ZF))               \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x5:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_ZF))              \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x6:                          \
            PREFIX                              \
            if((ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF)))  \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x7:                          \
            PREFIX                              \
            if(!(ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF))) \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x8:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF))               \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x9:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_SF))              \
                CONDITIONAL                     \
            break;                              \
        case BASE+0xA:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_PF))               \
                CONDITIONAL                     \
            break;                              \
        case BASE+0xB:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_PF))              \
                CONDITIONAL                     \
            break;                              \
        case BASE+0xC:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))  \
                CONDITIONAL                     \
            break;                              \
        case BASE+0xD:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF)) \
                CONDITIONAL                     \
            break;                              \
        case BASE+0xE:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_ZF) || (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))) \
                CONDITIONAL                     \
            break;                              \
        case BASE+0xF:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_ZF) && (ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF))) \
                CONDITIONAL                     \
            break;

        GOCOND(0x40
            , nextop = Fetch8(emu);
            GetEd(emu, &op2, nextop);
            GetG(emu, &op1, nextop);
            , op1->dword[0] = op2->dword[0];
        )                               /* 0x40 -> 0x4F CMOVxx Gd,Ed */ // conditional move, no sign
        GOCOND(0x80
            , tmp32s = Fetch32s(emu);
            , R_EIP += tmp32s;
        )                               /* 0x80 -> 0x8F Jxx */
        GOCOND(0x90
            , nextop = Fetch8(emu);
            GetEb(emu, &op1, nextop);
            , op1->byte[0]=1; else op1->byte[0]=0;
        )                               /* 0x90 -> 0x9F SETxx Eb */

        #undef GOCOND

        case 0x57:                      /* XORPS Gx, Ex */
            nextop = Fetch8(emu);
            GetEx(emu, &opx2, nextop);
            GetGx(emu, &opx1, nextop);
            for(int i=0; i<4; ++i)
                opx1->ud.d[i] ^= opx2->ud.d[i];
            break;

        case 0xA2:                      /* CPUID */
            tmp32u = R_EAX;
            switch(tmp32u) {
                case 0x0:
                    // emulate a P4
                    R_EAX = 0x80000004;
                    // return GuenuineIntel
                    R_EBX = 0x756E6547;
                    R_EDX = 0x49656E69;
                    R_ECX = 0x6C65746E;
                    break;
                case 0x1:
                    R_EAX = 0x00000101; // familly and all
                    R_EBX = 0;          // Brand indexe, CLFlush, Max APIC ID, Local APIC ID
                    R_EDX =   1         // fpu 
                            | 1<<8      // cmpxchg8
                            | 1<<11     // sep (sysenter & sysexit)
                            | 1<<15     // cmov
                            | 1<<19     // clflush (seems to be with SSE2)
                            | 1<<23     // mmx
                            | 1<<24     // fxsr (fxsave, fxrestore)
                            | 1<<25     // SSE
                            | 1<<26     // SSE2
                            ;
                    R_ECX =   1<<12     // fma
                            | 1<<13     // cx16 (cmpxchg16)
                            ;           // also 1<<0 is SSE3 and 1<<9 is SSSE3
                    break;
                default:
                    printf_log(LOG_INFO, "Warning, CPUID command %X unsupported\n", tmp32u);
                    R_EAX = 0;
            }
            break;
        case 0xA3:                      /* BT Ed,Gd */
            nextop = Fetch8(emu);
            GetEd(emu, &op1, nextop);
            GetG(emu, &op2, nextop);
            if(op1->dword[0] & (1<<(op2->dword[0]&31)))
                SET_FLAG(F_CF);
            else
                CLEAR_FLAG(F_CF);
            break;
        case 0xA4:                      /* SHLD Ed,Gd,Ib */
        case 0xA5:                      /* SHLD Ed,Gd,CL */
            nextop = Fetch8(emu);
            GetEd(emu, &op1, nextop);
            GetG(emu, &op2, nextop);
            tmp8u = (opcode==0xA4)?Fetch8(emu):R_CL;
            op1->dword[0] = shld32(emu, op1->dword[0], op2->dword[0], tmp8u);
            break;

        case 0xAB:                      /* BTS Ed,Gd */
            nextop = Fetch8(emu);
            GetEd(emu, &op1, nextop);
            GetG(emu, &op2, nextop);
            if(op1->dword[0] & (1<<(op2->dword[0]&31)))
                SET_FLAG(F_CF);
            else {
                op1->dword[0] |= (1<<(op2->dword[0]&31));
                CLEAR_FLAG(F_CF);
            }
            break;
        case 0xAC:                      /* SHRD Ed,Gd,Ib */
        case 0xAD:                      /* SHRD Ed,Gd,CL */
            nextop = Fetch8(emu);
            GetEd(emu, &op1, nextop);
            GetG(emu, &op2, nextop);
            tmp8u = (opcode==0xAC)?Fetch8(emu):R_CL;
            op1->dword[0] = shrd32(emu, op1->dword[0], op2->dword[0], tmp8u);
            break;

        case 0xAE:                      /* Grp Ed (SSE) */
            nextop = Fetch8(emu);
            GetEd(emu, &op1, nextop);
            switch((nextop>>3)&7) {
                case 2:                 /* LDMXCSR Md */
                    emu->mxcsr = op1->dword[0];
                    break;
                case 3:                 /* SDMXCSR Md */
                    op1->dword[0] = emu->mxcsr;
                    break;
                default:
                    UnimpOpcode(emu);
            }
            break;
        case 0xAF:                      /* IMUL Gd,Ed */
            nextop = Fetch8(emu);
            GetEd(emu, &op2, nextop);
            GetG(emu, &op1, nextop);
            op1->dword[0] = imul32(emu, op1->dword[0], op2->dword[0]);
            break;

        case 0xB0:                      /* CMPXCHG Eb,Gb */
            nextop = Fetch8(emu);
            GetEb(emu, &op1, nextop);
            GetGb(emu, &op2, nextop);
            if(R_AL == op1->byte[0]) {
                SET_FLAG(F_ZF);
                op1->byte[0] = op2->byte[0];
            } else {
                CLEAR_FLAG(F_ZF);
                R_AL = op1->byte[0];
            }
            break;
        case 0xB1:                      /* CMPXCHG Ed,Gd */
            nextop = Fetch8(emu);
            GetEd(emu, &op1, nextop);
            GetG(emu, &op2, nextop);
            if(R_EAX == op1->dword[0]) {
                SET_FLAG(F_ZF);
                op1->dword[0] = op2->dword[0];
            } else {
                CLEAR_FLAG(F_ZF);
                R_EAX = op1->dword[0];
            }
            break;

        case 0xB3:                      /* BTR Ed,Gd */
            nextop = Fetch8(emu);
            GetEd(emu, &op1, nextop);
            GetG(emu, &op2, nextop);
            if(op1->dword[0] & (1<<(op2->dword[0]&31))) {
                SET_FLAG(F_CF);
                op1->dword[0] ^= (1<<(op2->dword[0]&31));
            } else
                CLEAR_FLAG(F_CF);
            break;

        case 0xB6:                      /* MOVZX Gd,Eb */ // Move with zero extend
            nextop = Fetch8(emu);
            GetEb(emu, &op2, nextop);
            GetG(emu, &op1, nextop);
            op1->dword[0] = op2->byte[0];
            break;
        case 0xB7:                      /* MOVZX Gd,Ew */ // Move with zero extend
            nextop = Fetch8(emu);
            GetEw(emu, &op2, nextop);
            GetG(emu, &op1, nextop);
            op1->dword[0] = op2->word[0];
            break;

        case 0xBB:                      /* BTC Ed,Gd */
            nextop = Fetch8(emu);
            GetEd(emu, &op1, nextop);
            GetG(emu, &op2, nextop);
            if(op1->dword[0] & (1<<(op2->dword[0]&31)))
                SET_FLAG(F_CF);
            else
                CLEAR_FLAG(F_CF);
            op1->dword[0] ^= (1<<(op2->dword[0]&31));
            break;

        case 0xBA:                      
            nextop = Fetch8(emu);
            switch((nextop>>3)&7) {
                case 4:                 /* BT Ed,Ib */
                    GetEd(emu, &op1, nextop);
                    tmp8u = Fetch8(emu);
                    if(op1->dword[0] & (1<<tmp8u))
                        SET_FLAG(F_CF);
                    else
                        CLEAR_FLAG(F_CF);
                    break;
                default:
                    UnimpOpcode(emu);
            }
            break;
        case 0xBC:                      /* BSF Ed,Gd */
            nextop = Fetch8(emu);
            GetEd(emu, &op1, nextop);
            GetG(emu, &op2, nextop);
            tmp32u = op1->dword[0];
            if(tmp32u) {
                CLEAR_FLAG(F_ZF);
                tmp8u = 0;
                while(!(tmp32u&(1<<tmp8u))) ++tmp8u;
                op2->dword[0] = tmp8u;
            } else {
                SET_FLAG(F_ZF);
            }
            break;
        case 0xBD:                      /* BSR Ed,Gd */
            nextop = Fetch8(emu);
            GetEd(emu, &op1, nextop);
            GetG(emu, &op2, nextop);
            tmp32u = op1->dword[0];
            if(tmp32u) {
                CLEAR_FLAG(F_ZF);
                tmp8u = 31;
                while(!(tmp32u&(1<<tmp8u))) --tmp8u;
                op2->dword[0] = tmp8u;
            } else {
                SET_FLAG(F_ZF);
            }
            break;
        case 0xBE:                      /* MOVSX Gd,Eb */ // Move with sign extend
            nextop = Fetch8(emu);
            GetEb(emu, &op2, nextop);
            GetG(emu, &op1, nextop);
            *(int32_t*)&op1->dword[0] = (int8_t)op2->byte[0];
            break;
        case 0xBF:                      /* MOVSX Gd,Ew */ // Move with sign extend
            nextop = Fetch8(emu);
            GetEw(emu, &op2, nextop);
            GetG(emu, &op1, nextop);
            *(int32_t*)&op1->dword[0] = (int16_t)op2->word[0];
            break;

        case 0xC0:                      /* XADD Gb,Eb */ // Xchange and Add
            nextop = Fetch8(emu);
            GetEb(emu, &op1, nextop);
            GetGb(emu, &op2, nextop);
            tmp8u = add8(emu, op1->byte[0], op2->byte[0]);
            op2->byte[0] = op1->byte[0];
            op1->byte[0] = tmp8u;
            break;
        case 0xC1:                      /* XADD Gd,Ed */ // Xchange and Add
            nextop = Fetch8(emu);
            GetEd(emu, &op1, nextop);
            GetG(emu, &op2, nextop);
            tmp32u = add32(emu, op1->dword[0], op2->dword[0]);
            op2->dword[0] = op1->dword[0];
            op1->dword[0] = tmp32u;
            break;

        case 0xC6:                      /* SHUFPS Gx, Ex, Ib */
            nextop = Fetch8(emu);
            GetEx(emu, &opx2, nextop);
            GetGx(emu, &opx1, nextop);
            tmp8u = Fetch8(emu);
            for(int i=0; i<2; ++i) {
                eax1.ud.d[i] = opx1->ud.d[(tmp8u>>(i*2))&3];
            }
            for(int i=2; i<4; ++i) {
                eax1.ud.d[i] = opx2->ud.d[(tmp8u>>(i*2))&3];
            }
            memcpy(opx1, &eax1, sizeof(eax1));
            break;
        case 0xC7:                      /* CMPXCHG8B Gq */
            nextop = Fetch8(emu);
            GetEd(emu, &op1, nextop);
            tmp32u = op1->dword[0];
            tmp32u2= op1->dword[1];
            if(R_EAX == tmp32u && R_EDX == tmp32u2) {
                SET_FLAG(F_ZF);
                op1->dword[0] = R_EBX;
                op1->dword[1] = R_ECX;
            } else {
                CLEAR_FLAG(F_ZF);
                R_EAX = op1->dword[0];
                R_EDX = op1->dword[1];
            }
            break;

        default:
            UnimpOpcode(emu);
    }
}
