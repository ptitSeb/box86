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

#include "dynarec_arm_functions.h"
#include "dynarec_arm_helper.h"

uintptr_t dynarec0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    int32_t i32, i32_;
    int16_t i16;
    uint16_t u16;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    uint8_t eb1, eb2;
    uint8_t gb1, gb2;
    int v0, v1, v2;
    int q0, q1;
    int d0, d1;
    int s0, s1;
    int fixedaddress;
    switch(opcode) {

        case 0x10:
            INST_NAME("MOVUPS Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            if((nextop&0xC0)==0xC0) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7);
                VMOVQ(v0, v1);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                LDRD_IMM8(x2, ed, 0);
                VMOVtoV_D(v0, x2, x3);
                LDRD_IMM8(x2, ed, 8);
                VMOVtoV_D(v0+1, x2, x3);
            }
            break;
        case 0x11:
            INST_NAME("MOVUPS Ex,Gx");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            if((nextop&0xC0)==0xC0) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7);
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                VMOVfrV_D(x2, x3, v0);
                STRD_IMM8(x2, ed, 0);
                VMOVfrV_D(x2, x3, v0+1);
                STRD_IMM8(x2, ed, 8);
            }
            break;

        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;

        case 0x28:
            INST_NAME("MOVAPS Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            if((nextop&0xC0)==0xC0) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7);
                VMOVQ(v0, v1);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                VLD1Q_32(v0, ed);
            }
            break;
        case 0x29:
            INST_NAME("MOVAPS Ex,Gx");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            if((nextop&0xC0)==0xC0) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7);
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                VST1Q_32(v0, ed);
            }
            break;

        case 0x2E:
            // no special check...
        case 0x2F:
            if(opcode==0x2F) {INST_NAME("COMISS Gx, Ex");} else {INST_NAME("UCOMISS Gx, Ex");}
            UFLAGS(0);
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            d0 = fpu_get_scratch_double(dyn);
            s0 = fpu_get_scratch_single(dyn);
            s1 = fpu_get_scratch_single(dyn);
            if((nextop&0xC0)==0xC0) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7);
                VMOV_64(d0, v1);
                VMOV_32(s0, d0*2);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                VLDR_32(s0, ed, 0);
            }
            VMOV_64(d0, v0);
            VMOV_32(s1, d0*2);
            VCMP_F32(s1, s0);
            FCOMI(x1, x2);
            UFLAGS(1);
            break;

        
        #define GO(GETFLAGS, NO, YES)   \
            USEFLAG(1); \
            GETFLAGS;   \
            nextop=F8;  \
            GETGD;      \
            if((nextop&0xC0)==0xC0) {   \
                ed = xEAX+(nextop&7);   \
                MOV_REG_COND(YES, gd, ed); \
            } else { \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress);    \
                LDR_IMM9_COND(YES, gd, ed, 0); \
            }

        case 0x40:
            INST_NAME("CMOVO Gd, Ed");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_OF]));
                CMPS_IMM8(x2, 1)
                , cNE, cEQ)
            break;
        case 0x41:
            INST_NAME("CMOVNO Gd, Ed");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_OF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x42:
            INST_NAME("CMOVC Gd, Ed");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                CMPS_IMM8(x2, 1)
                , cNE, cEQ)
            break;
        case 0x43:
            INST_NAME("CMOVNC Gd, Ed");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                CMPS_IMM8(2, 1)
                , cEQ, cNE)
            break;
        case 0x44:
            INST_NAME("CMOVZ Gd, Ed");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                CMPS_IMM8(x2, 1)
                , cNE, cEQ)
            break;
        case 0x45:
            INST_NAME("CMOVNZ Gd, Ed");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x46:
            INST_NAME("CMOVBE Gd, Ed");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                ORRS_REG_LSL_IMM8(x2, x2, x3, 0);
                , cEQ, cNE)
            break;
        case 0x47:
            INST_NAME("CMOVNBE Gd, Ed");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                ORRS_REG_LSL_IMM8(x2, x2, x3, 0);
                , cNE, cEQ)
            break;
        case 0x48:
            INST_NAME("CMOVS Gd, Ed");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                CMPS_IMM8(x2, x1)
                , cNE, cEQ)
            break;
        case 0x49:
            INST_NAME("CMOVNS Gd, Ed");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x4A:
            INST_NAME("CMOVP Gd, Ed");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_PF]));
                CMPS_IMM8(x2, x1)
                , cNE, cEQ)
            break;
        case 0x4B:
            INST_NAME("CMOVNP Gd, Ed");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_PF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x4C:
            INST_NAME("CMOVL Gd, Ed");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                CMPS_REG_LSL_IMM8(x1, x2, 0)
                , cEQ, cNE)
            break;
        case 0x4D:
            INST_NAME("CMOVGE Gd, Ed");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                CMPS_REG_LSL_IMM8(x1, x2, 0)
                , cNE, cEQ)
            break;
        case 0x4E:
            INST_NAME("CMOVLE Gd, Ed");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                XOR_REG_LSL_IMM8(x1, x1, x2, 0);
                LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                ORRS_REG_LSL_IMM8(x2, x1, x2, 0);
                , cEQ, cNE)
            break;
        case 0x4F:
            INST_NAME("CMOVG Gd, Ed");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                XOR_REG_LSL_IMM8(x1, x1, x2, 0);
                LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                ORRS_REG_LSL_IMM8(x2, x1, x2, 0);
                , cNE, cEQ)
            break;
        #undef GO

        case 0x57:
            INST_NAME("XORPS Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd);
            if((nextop&0xC0)==0xC0) {
                q0 = sse_get_reg(dyn, ninst, x1, nextop&7);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress);
                q0 = fpu_get_scratch_quad(dyn);
                VLD1Q_64(q0, ed);
            }
            VEORQ(v0, v0, q0);
            break;

        #define GO(GETFLAGS, NO, YES)   \
            i32_ = F32S;    \
            USEFLAG(1);     \
            BARRIER(2);     \
            JUMP(addr+i32_);\
            GETFLAGS;   \
            if(dyn->insts) {    \
                if(dyn->insts[ninst].x86.jmp_insts==-1) {   \
                    /* out of the block */                  \
                    i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8); \
                    Bcond(NO, i32);     \
                    jump_to_linker(dyn, addr+i32_, 0, ninst); \
                } else {    \
                    /* inside the block */  \
                    i32 = dyn->insts[dyn->insts[ninst].x86.jmp_insts].address-(dyn->arm_size+8);    \
                    Bcond(YES, i32);    \
                }   \
            }

        case 0x80:
            INST_NAME("JO id");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_OF]));
                CMPS_IMM8(x2, 1)
                , cNE, cEQ)
            break;
        case 0x81:
            INST_NAME("JNO id");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_OF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x82:
            INST_NAME("JC id");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                CMPS_IMM8(x2, 1)
                , cNE, cEQ)
            break;
        case 0x83:
            INST_NAME("JNC id");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x84:
            INST_NAME("JZ id");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                CMPS_IMM8(x2, 1)
                , cNE, cEQ)
            break;
        case 0x85:
            INST_NAME("JNZ id");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x86:
            INST_NAME("JBE id");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                ORRS_REG_LSL_IMM8(x2, x2, x3, 0);
                , cEQ, cNE)
            break;
        case 0x87:
            INST_NAME("JNBE id");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                ORRS_REG_LSL_IMM8(x2, x2, x3, 0);
                , cNE, cEQ)
            break;
        case 0x88:
            INST_NAME("JS id");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                CMPS_IMM8(x2, 1)
                , cNE, cEQ)
            break;
        case 0x89:
            INST_NAME("JNS id");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x8A:
            INST_NAME("JP id");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_PF]));
                CMPS_IMM8(x2, 1)
                , cNE, cEQ)
            break;
        case 0x8B:
            INST_NAME("JNP id");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_PF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x8C:
            INST_NAME("JL id");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                CMPS_REG_LSL_IMM8(x1, x2, 0)
                , cEQ, cNE)
            break;
        case 0x8D:
            INST_NAME("JGE id");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                CMPS_REG_LSL_IMM8(x1, x2, 0)
                , cNE, cEQ)
            break;
        case 0x8E:
            INST_NAME("JLE id");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                XOR_REG_LSL_IMM8(x1, x1, x2, 0);
                LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                ORRS_REG_LSL_IMM8(x2, x1, x2, 0);
                , cEQ, cNE)
            break;
        case 0x8F:
            INST_NAME("JG id");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                XOR_REG_LSL_IMM8(x1, x1, x2, 0);
                LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                ORRS_REG_LSL_IMM8(x2, x1, x2, 0);
                , cNE, cEQ)
            break;
        #undef GO
        #define GO(GETFLAGS, NO, YES)   \
            USEFLAG(1); \
            GETFLAGS;   \
            nextop=F8;  \
            MOVW_COND(NO, x3, 0); \
            MOVW_COND(YES, x3, 1);  \
            if((nextop&0xC0)==0xC0) { \
                ed = (nextop&7);    \
                eb1 = xEAX+(ed&3);  \
                eb2 = ((ed&4)>>2);  \
                BFI(eb1, x3, eb2*8, 8); \
            } else {                \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress);    \
                STRB_IMM9(x3, ed, 0);   \
            }

        case 0x90:
            INST_NAME("SETO Eb");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_OF]));
                CMPS_IMM8(x2, 1)
                , cNE, cEQ)
            break;
        case 0x91:
            INST_NAME("SETNO Eb");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_OF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x92:
            INST_NAME("SETC Eb");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                CMPS_IMM8(x2, 1)
                , cNE, cEQ)
            break;
        case 0x93:
            INST_NAME("SETNC Eb");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                CMPS_IMM8(2, 1)
                , cEQ, cNE)
            break;
        case 0x94:
            INST_NAME("SETZ Eb");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                CMPS_IMM8(x2, 1)
                , cNE, cEQ)
            break;
        case 0x95:
            INST_NAME("SETNZ Eb");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x96:
            INST_NAME("SETBE Eb");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                ORRS_REG_LSL_IMM8(x2, x2, x3, 0);
                , cEQ, cNE)
            break;
        case 0x97:
            INST_NAME("SETNBE Eb");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_CF]));
                LDR_IMM9(x3, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                ORRS_REG_LSL_IMM8(x2, x2, x3, 0);
                , cNE, cEQ)
            break;
        case 0x98:
            INST_NAME("SETS Eb");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                CMPS_IMM8(x2, x1)
                , cNE, cEQ)
            break;
        case 0x99:
            INST_NAME("SETNS Eb");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x9A:
            INST_NAME("SETP Eb");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_PF]));
                CMPS_IMM8(x2, x1)
                , cNE, cEQ)
            break;
        case 0x9B:
            INST_NAME("SETNP Eb");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_PF]));
                CMPS_IMM8(x2, 1)
                , cEQ, cNE)
            break;
        case 0x9C:
            INST_NAME("SETL Eb");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                CMPS_REG_LSL_IMM8(x1, x2, 0)
                , cEQ, cNE)
            break;
        case 0x9D:
            INST_NAME("SETGE Eb");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                CMPS_REG_LSL_IMM8(x1, x2, 0)
                , cNE, cEQ)
            break;
        case 0x9E:
            INST_NAME("SETLE Eb");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                XOR_REG_LSL_IMM8(x1, x1, x2, 0);
                LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                ORRS_REG_LSL_IMM8(x2, x1, x2, 0);
                , cEQ, cNE)
            break;
        case 0x9F:
            INST_NAME("SETG Eb");
            GO( LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
                XOR_REG_LSL_IMM8(x1, x1, x2, 0);
                LDR_IMM9(x2, xEmu, offsetof(x86emu_t, flags[F_ZF]));
                ORRS_REG_LSL_IMM8(x2, x1, x2, 0);
                , cNE, cEQ)
            break;
        #undef GO

        case 0xA2:
            INST_NAME("CPUID");
            MOV_REG(x1, xEAX);
            MOV32(x12, ip+2);   // EIP is useless, but why not...
            // not purging stuff like x87 here, there is no float math or anything
            STM(xEmu, (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12));
            CALL_(arm_cpuid, -1, 0);
            LDM(xEmu, (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12));
            break;
        case 0xA3:
            INST_NAME("BT Ed, Gd");
            nextop = F8;
            USEFLAG(1);
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = xEAX+(nextop&7);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress);
                UBFX(x1, gd, 5, 3); // r1 = (gd>>5);
                ADD_REG_LSL_IMM8(x1, ed, x1, 2); //(&ed)+=r1*4;
                LDR_IMM9(x1, x1, 0);
                ed = x1;
            }
            AND_IMM8(x2, gd, 0x1f);
            MOV_REG_LSR_REG(x1, ed, x2);
            AND_IMM8(x1, x1, 1);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_CF]));
            break;
        case 0xA4:
        case 0xA5:
            nextop = F8;
            if(opcode==0xA4) {
                INST_NAME("SHLD Ed, Gd, Ib");
            } else {
                INST_NAME("SHLD Ed, Gd, CL");
                UXTB(x3, xECX, 0);
            }
            GETEDW(x12, x1);
            GETGD;
            if(opcode==0xA4) {
                u8 = F8;
                MOVW(x3, u8);
            }
            MOV_REG(x2, gd);
            CALL(shld32, ed, (wback?(1<<wback):0));
            WBACK;
            UFLAGS(1);
            break;

        case 0xAB:
            INST_NAME("BTS Ed, Gd");
            nextop = F8;
            USEFLAG(1);
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = xEAX+(nextop&7);
                wback = 0;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress);
                UBFX(x1, gd, 5, 3); // r1 = (gd>>5);
                ADD_REG_LSL_IMM8(x3, wback, x1, 2); //(&ed)+=r1*4;
                LDR_IMM9(x1, x3, 0);
                ed = x1;
                wback = x3;
            }
            AND_IMM8(x2, gd, 0x1f);
            MOV_REG_LSR_REG(x12, ed, x2);
            ANDS_IMM8(x12, x12, 1);
            STR_IMM9(x12, xEmu, offsetof(x86emu_t, flags[F_CF]));
            B_NEXT(cNE); // bit already set, jump to next instruction
            MOVW(x12, 1);
            ORR_REG_LSL_REG(ed, ed, x12, x2);
            if(wback) {
                STR_IMM9(ed, wback, 0);
            }
            break;
        case 0xAC:
        case 0xAD:
            nextop = F8;
            if(opcode==0xAC) {
                INST_NAME("SHRD Ed, Gd, Ib");
            } else {
                INST_NAME("SHRD Ed, Gd, CL");
                UXTB(x3, xECX, 0);
            }
            GETEDW(x12, x1);
            GETGD;
            if(opcode==0xAC) {
                u8 = F8;
                MOVW(x3, u8);
            }
            MOV_REG(x2, gd);
            CALL(shrd32, ed, (wback?(1<<wback):0));
            WBACK;
            UFLAGS(1);
            break;
        case 0xAE:
            nextop = F8;
            if((nextop&0xF8)==0xE8) {
                INST_NAME("LFENCE");
            } else
            if((nextop&0xF8)==0xF0) {
                INST_NAME("MFENCE");
            } else
            if((nextop&0xF8)==0xF8) {
                INST_NAME("SFENCE");
            } else {
                switch((nextop>>3)&7) {
                    case 2:                 
                        INST_NAME("LDMXCSR Md");
                        GETED;
                        STR_IMM9(ed, xEmu, offsetof(x86emu_t, mxcsr));
                        break;
                    case 3:
                        INST_NAME("STMXCSR Md");
                        if((nextop&0xC0)==0xC0) {
                            ed = xEAX+(nextop&7);
                            LDR_IMM9(ed, xEmu, offsetof(x86emu_t, mxcsr));
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress);
                            LDR_IMM9(x12, xEmu, offsetof(x86emu_t, mxcsr));
                            STR_IMM9(x12, ed, 0);
                        }
                        break;
                    default:
                        *ok = 0;
                        DEFAULT;
                }
            }
            break;
        case 0xAF:
            INST_NAME("IMUL Gd, Ed");
            nextop = F8;
            GETGD;
            GETED;
            UFLAG_IF {
                SMULL(x3, gd, gd, ed);
                UFLAG_OP1(x3);
                UFLAG_RES(gd);
                UFLAG_DF(x3, d_imul32);
            } else {
                MUL(gd, gd, ed);
            }
            UFLAGS(0);
            break;

        case 0xB0:
            INST_NAME("CMPXCHG Eb, Gb");
            nextop = F8;
            UFLAGS(0);
            MOVW(x1, 0);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, df)); // d_none == 0
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_AF]));
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_PF]));
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_SF]));
            GETEB(x2)
            UXTB(x1, xEAX, 0);
            // Use a quick CMP, without setting A or P...
            CMPS_REG_LSL_IMM8(x1, ed, 0);
            MOVW_COND(cCC, x1, 1);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_CF]));
            B_MARK(cNE);
            // AL == Eb
            GETGB(x1);
            MOV_REG(ed, x1);
            EBBACK;
            MOVW(x1, 1);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_ZF]));
            // done
            B_MARK3(c__);
            MARK;
            // AL != Eb
            BFI(xEAX, ed, 0, 8);
            MOVW(x1, 0);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_ZF]));
            UFLAGS(1);
            MARK3;
            break;
        case 0xB1:
            INST_NAME("CMPXCHG Ed, Gd");
            nextop = F8;
            UFLAGS(0);
            MOVW(x1, 0);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, df)); // d_none == 0
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_AF]));
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_PF]));
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_OF]));
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_SF]));
            GETED;
            GETGD;
            // Use a quick CMP, without setting A or P...
            CMPS_REG_LSL_IMM8(xEAX, ed, 0);
            MOVW_COND(cCC, x1, 1);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_CF]));
            B_MARK(cNE);
            // EAX == Ed
            MOV_REG(ed, gd);
            WBACK;
            MOVW(x1, 1);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_ZF]));
            // done
            B_MARK3(c__);   // not next, in case its called with a LOCK prefix
            MARK;
            // EAX != Ed
            MOV_REG(xEAX, ed);
            MOVW(x1, 0);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_ZF]));
            UFLAGS(1);
            MARK3
            break;
        case 0xB3:
            INST_NAME("BTR Ed, Gd");
            nextop = F8;
            USEFLAG(1);
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = xEAX+(nextop&7);
                wback = 0;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress);
                UBFX(x1, gd, 5, 3); // r1 = (gd>>5);
                ADD_REG_LSL_IMM8(x3, wback, x1, 2); //(&ed)+=r1*4;
                LDR_IMM9(x1, wback, 0);
                ed = x1;
                wback = x3;
            }
            AND_IMM8(x2, gd, 0x1f);
            MOV_REG_LSR_REG(x12, ed, x2);
            ANDS_IMM8(x12, x12, 1);
            STR_IMM9(x12, xEmu, offsetof(x86emu_t, flags[F_CF]));
            B_MARK3(cEQ); // bit already clear, jump to end of instruction
            MOVW(x12, 1);
            XOR_REG_LSL_REG(ed, ed, x12, x2);
            if(wback) {
                STR_IMM9(ed, wback, 0);
            }
            MARK3;
            break;

        case 0xB6:
            INST_NAME("MOVZX Gd, Eb");
            nextop = F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = (nextop&7);
                eb1 = xEAX+(ed&3);  // Ax, Cx, Dx or Bx
                eb2 = (ed&4)>>2;    // L or H
                UXTB(gd, eb1, eb2);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress);
                LDRB_IMM9(gd, ed, 0);
            }
            break;
        case 0xB7:
            INST_NAME("MOVZX Gd, Ew");
            nextop = F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = xEAX+(nextop&7);
                UXTH(gd, ed, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress);
                LDRH_IMM8(gd, ed, 0);
            }
            break;
        
        case 0xBA:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 4:
                    INST_NAME("BT Ed, Ib");
                    USEFLAG(1);
                    gd = x2;
                    if((nextop&0xC0)==0xC0) {
                        ed = xEAX+(nextop&7);
                        u8 = F8;
                        MOVW(gd, u8);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress);
                        u8 = F8;
                        MOVW(gd, u8);
                        UBFX(x1, gd, 5, 3); // r1 = (gd>>5);
                        ADD_REG_LSL_IMM8(x1, ed, x1, 2); //(&ed)+=r1*4;
                        LDR_IMM9(x1, x1, 0);
                        ed = x1;
                    }
                    AND_IMM8(x2, gd, 0x1f);
                    MOV_REG_LSR_REG(x1, ed, x2);
                    AND_IMM8(x1, x1, 1);
                    STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_CF]));
                    break;
                case 6:
                    INST_NAME("BTR Ed, Ib");
                    USEFLAG(1);
                    gd = x2;
                    if((nextop&0xC0)==0xC0) {
                        ed = xEAX+(nextop&7);
                        u8 = F8;
                        MOVW(gd, u8);
                        wback = 0;
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress);
                        u8 = F8;
                        MOVW(gd, u8);
                        UBFX(x1, gd, 5, 3); // r1 = (gd>>5);
                        ADD_REG_LSL_IMM8(x3, ed, x1, 2); //(&ed)+=r1*4;
                        LDR_IMM9(x1, x3, 0);
                        ed = x1;
                        wback = x3;
                    }
                    AND_IMM8(x2, gd, 0x1f);
                    MOV_REG_LSR_REG(x12, ed, x2);
                    ANDS_IMM8(x12, x12, 1);
                    STR_IMM9(x12, xEmu, offsetof(x86emu_t, flags[F_CF]));
                    B_MARK3(cEQ); // bit already clear, jump to next instruction
                    //MOVW(x12, 1); // already 0x01
                    XOR_REG_LSL_REG(ed, ed, x12, x2);
                    if(wback) {
                        STR_IMM9(ed, wback, 0);
                    }
                    MARK3;
                default:
                    *ok = 0;
                    DEFAULT;
            }
            break;

        case 0xBB:
            INST_NAME("BTC Ed, Gd");
            nextop = F8;
            USEFLAG(1);
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = xEAX+(nextop&7);
                wback = 0;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress);
                UBFX(x1, gd, 5, 3); // r1 = (gd>>5);
                ADD_REG_LSL_IMM8(x3, wback, x1, 2); //(&ed)+=r1*4;
                LDR_IMM9(x1, x3, 0);
                ed = x1;
                wback = x3;
            }
            AND_IMM8(x2, gd, 0x1f);
            MOV_REG_LSR_REG(x12, ed, x2);
            AND_IMM8(x12, x12, 1);
            STR_IMM9(x12, xEmu, offsetof(x86emu_t, flags[F_CF]));
            MOVW(x12, 1);
            XOR_REG_LSL_REG(ed, ed, x12, x2);
            if(wback) {
                STR_IMM9(ed, wback, 0);
            }
            break;

        case 0xBE:
            INST_NAME("MOVSX Gd, Eb");
            nextop = F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = (nextop&7);
                eb1 = xEAX+(ed&3);  // Ax, Cx, Dx or Bx
                eb2 = (ed&4)>>2;    // L or H
                SXTB(gd, eb1, eb2);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress);
                LDRSB_IMM8(gd, ed, 0);
            }
            break;
        case 0xBF:
            INST_NAME("MOVSX Gd, Ew");
            nextop = F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = xEAX+(nextop&7);
                SXTH(gd, ed, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress);
                LDRSH_IMM8(gd, ed, 0);
            }
            break;
        case 0xC0:
            INST_NAME("XADD Gb, Eb");
            nextop = F8;
            GETEB(x2);
            GETGB(x1);
            UFLAG_OP12(gd, ed);
            ADD_REG_LSL_IMM8(x12, ed, gd, 0);
            UFLAG_RES(x12);
            gd = ed;
            GBBACK;
            ed = x12;
            EBBACK;
            UFLAG_DF(x3, d_add8);
            UFLAGS(0);
            break;
        case 0xC1:
            INST_NAME("XADD Gd, Ed");
            nextop = F8;
            GETGD;
            GETED;
            UFLAG_OP12(gd, ed);
            ADD_REG_LSL_IMM8(x12, gd, ed, 0);
            UFLAG_RES(x12);
            MOV_REG(gd, ed);
            if(wback) {
                STR_IMM9(x12, wback, 0);
            } else {
                MOV_REG(ed, x12);
            }
            UFLAG_DF(x1, d_add32);
            UFLAGS(0);
            break;

        case 0xC7:
            INST_NAME("CMPXCHG8B Gq, Eq");
            nextop = F8;
            USEFLAG(1);
            addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress);
            LDR_IMM9(x1, wback, 0);
            LDR_IMM9(x2, wback, 4);
            CMPS_REG_LSL_IMM8(xEAX, x1, 0);
            B_MARK(cNE);    // EAX != Ed[0]
            CMPS_REG_LSL_IMM8(xEDX, x2, 0);
            B_MARK(cNE);    // EDX != Ed[1]
            STR_IMM9(xEBX, wback, 0);
            STR_IMM9(xECX, wback, 4);
            MOVW(x1, 1);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_ZF]));
            B_MARK3(c__);
            MARK;
            MOV_REG(xEAX, x1);
            MOV_REG(xEDX, x2);
            MOVW(x1, 0);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, flags[F_ZF]));
            UFLAGS(1);
            MARK3;
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:                  /* BSWAP reg */
            INST_NAME("BSWAP Reg");
            gd = xEAX+(opcode&7);
            REV(gd, gd);
            break;

        default:
            *ok = 0;
            DEFAULT;
    }
    return addr;
}

