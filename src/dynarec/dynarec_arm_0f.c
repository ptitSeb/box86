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
#include "my_cpuid.h"
#include "emu/x87emu_private.h"

#include "dynarec_arm_functions.h"
#include "dynarec_arm_helper.h"

#define GETGX(a, w)    \
    gd = (nextop&0x38)>>3;  \
    a = sse_get_reg(dyn, ninst, x1, gd, w)
#define GETEX(a, w)    \
    if(MODREG) { \
        a = sse_get_reg(dyn, ninst, x1, nextop&7, w); \
    } else {    \
        SMREAD(); \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL); \
        a = fpu_get_scratch_quad(dyn); \
        VLD1Q_64(a, ed);    \
    }
#define GETGM(a)    \
    gd = (nextop&0x38)>>3;  \
    a = mmx_get_reg(dyn, ninst, x1, x2, x3, gd)
#define GETEM(a)    \
    if(MODREG) { \
        a = mmx_get_reg(dyn, ninst, x1, x2, x3, nextop&7); \
    } else {    \
        SMREAD(); \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL); \
        a = fpu_get_scratch_double(dyn); \
        VLD1_64(a, ed);    \
    }
#define PUTEM(a)    \
    if((nextop&0xC0)!=0xC0) { \
        VST1_64(a, ed);    \
        SMWRITE2(); \
    }

uintptr_t dynarec0F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    int32_t i32, i32_, j32;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2;
    uint8_t eb1, eb2;
    uint8_t gb1, gb2;
    int v0, v1, v2;
    int q0, q1;
    int d0, d1;
    int s0;
    int fixedaddress;
    int parity;
    int cacheupd;
    MAYUSE(s0);
    MAYUSE(q1);
    MAYUSE(v2);
    MAYUSE(gb2);
    MAYUSE(gb1);
    MAYUSE(eb2);
    MAYUSE(eb1);
    MAYUSE(wb2);
    MAYUSE(j32);
    MAYUSE(cacheupd);
    #if STEP == 3
    static const int8_t mask_shift8[] = { -7, -6, -5, -4, -3, -2, -1, 0 };
    #endif

    switch(opcode) {

        case 0x01:
            nextop = F8;
            if(MODREG)
            switch(nextop) {
                case 0xD0:
                    INST_NAME("FAKE xgetbv");
                    addr = fakeed(dyn, addr, ninst, nextop);
                    SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
                    //CALL(arm_ud, -1, 0);
                    SKIPTEST(x1, x14);
                    UDF(0);
                    break;
                case 0xE0:
                case 0xE1:
                case 0xE2:
                case 0xE3:
                case 0xE4:
                case 0xE5:
                case 0xE6:
                case 0xE7:
                    INST_NAME("SMSW Ed");
                    ed = xEAX+(nextop&7);
                    MOV32(ed, (1<<0) | (1<<4)); // only PE and ET set...
                    break;
                default:
                    DEFAULT;
            }
            else
                switch((nextop>>3)&7) {
                    case 0:
                        INST_NAME("SGDT Ed");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, NULL);
                        MOVW(x1, 0x7f);
                        STRH_IMM8(x1, wback, 0);
                        MOVW(x1, 0x000c);
                        STRH_IMM8(x1, wback, 2);
                        MOVW(x1, 0xd000);
                        STRH_IMM8(x1, wback, 4);
                        break;
                    case 1:
                        INST_NAME("SIDT Ed");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, NULL);
                        MOVW(x1, 0xfff);
                        STRH_IMM8(x1, wback, 0);
                        MOVW(x1, 0);
                        STR_IMM9(x1, wback, 2);
                        break;
                    case 4:
                        INST_NAME("SMSW Ew");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, NULL);
                        // dummy for now... Do I need to track CR0 state?
                        MOVW(x1, (1<<0) | (1<<4)); // only PE and ET set...
                        STRH_IMM8(x1, wback, 0);
                        break;
                    default:
                        DEFAULT;
                    }
            break;

        case 0x0B:
            INST_NAME("UD2");
            SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
            //CALL(arm_ud, -1, 0);
            SKIPTEST(x1, x14);
            UDF(0);
            break;

        case 0x0D:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 1:
                    INST_NAME("PREFETCHW");
                    //addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                    addr = fakeed(dyn,addr, ninst, nextop);
                    // should use PLDW, but it's not available on cortex-a8 for example
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x10:
            INST_NAME("MOVUPS Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-12, 0, 0, NULL);
                //LDRD also have alignment requirements
                LDR_IMM9(x2, ed, fixedaddress+0);
                LDR_IMM9(x3, ed, fixedaddress+4);
                VMOVtoV_D(v0, x2, x3);
                LDR_IMM9(x2, ed, fixedaddress+8);
                LDR_IMM9(x3, ed, fixedaddress+12);
                VMOVtoV_D(v0+1, x2, x3);
            }
            break;
        case 0x11:
            INST_NAME("MOVUPS Ex,Gx");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, nextop&7);
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-16, 0, 0, NULL);
                VMOVfrV_D(x2, x3, v0);
                // cannot use STRD, this needs alignement too
                STR_IMM9(x2, ed, fixedaddress+0);
                STR_IMM9(x3, ed, fixedaddress+4);
                VMOVfrV_D(x2, x3, v0+1);
                STR_IMM9(x2, ed, fixedaddress+8);
                STR_IMM9(x3, ed, fixedaddress+12);
                SMWRITE2();
            }
            break;
        case 0x12:
            nextop = F8;
            if(MODREG) {
                INST_NAME("MOVHLPS Gx,Ex");
                GETGX(v0, 1);
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
                VMOVD(v0, v1+1);
            } else {
                INST_NAME("MOVLPS Gx,Ex");
                GETGX(v0, 1);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                LDR_IMM9(x2, ed, fixedaddress);
                LDR_IMM9(x3, ed, fixedaddress+4);
                VMOVtoV_D(v0, x2, x3);
            }
            break;
        case 0x13:
            nextop = F8;
            INST_NAME("MOVLPS Ex,Gx");
            GETGX(v0, 0);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 1);
                VMOVD(v1, v0);
            } else {
                parity = getedparity(dyn, ninst, addr, nextop, 2);
                if(parity) {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                    VST1_32(v0, ed);  // better to use VST1 than VSTR_64, to avoid NEON->VFPU transfert I assume
                    SMWRITE2();
                } else {
                    VMOVfrV_D(x2, x3, v0);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                    STR_IMM9(x2, ed, fixedaddress);
                    STR_IMM9(x3, ed, fixedaddress+4);
                    SMWRITE2();
                }
            }
            break;
        case 0x14:
            INST_NAME("UNPCKLPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0, 1);
            if(MODREG) {
                q1 = fpu_get_scratch_quad(dyn);
                VMOVQ(q1, q0);
            } else q1 = q0;
            VZIPQ_32(v0, q1);
            break;
        case 0x15:
            INST_NAME("UNPCKHPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0, 1);
            if(MODREG) {
                q1 = fpu_get_scratch_quad(dyn);
                VMOVQ(q1, q0);
            } else q1 = q0;
            VZIPQ_32(v0, q1);
            VMOVQ(v0, q1);
            break;
        case 0x16:
            nextop = F8;
            if(MODREG) {
                INST_NAME("MOVLHPS Gx,Ex");
                GETGX(v0, 1);
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
                VMOVD(v0+1, v1);
            } else {
                INST_NAME("MOVHPS Gx,Ex");
                GETGX(v0, 1);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                VLD1_64(v0+1, ed);
            }
            break;
        case 0x17:
            nextop = F8;
            INST_NAME("MOVHPS Ex,Gx");
            GETGX(v0, 0);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 1);
                VMOVD(v1, v0+1);
            } else {
                parity = getedparity(dyn, ninst, addr, nextop, 2);
                if(parity) {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                    VST1_64(v0+1, ed);  // better to use VST1 than VSTR_64, to avoid NEON->VFPU transfert I assume
                    SMWRITE2();
                } else {
                    VMOVfrV_D(x2, x3, v0+1);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                    STR_IMM9(x2, ed, fixedaddress);
                    STR_IMM9(x3, ed, fixedaddress+4);
                    SMWRITE2();
                }
            }
            break;
        case 0x18:
            nextop = F8;
            if(MODREG) {
                INST_NAME("NOP (multibyte)");
            } else
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                case 2:
                case 3:
                    INST_NAME("PREFETCHh Ed");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0xffff, 0, 0, NULL);
                    MOVW(x3, fixedaddress);
                    if(fixedaddress<0) {
                        PLDn(ed, x3);
                    } else {
                        PLD(ed, x3);
                    }
                    break;
                default:
                    INST_NAME("NOP (multibyte)");
                    FAKEED;
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
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                VLD1Q_32(v0, ed);
            }
            break;
        case 0x29:
            INST_NAME("MOVAPS Ex,Gx");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, nextop&7);
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                VST1Q_32(v0, ed);
                SMWRITE2();
            }
            break;
        case 0x2A:
            INST_NAME("CVTPI2PS Gx, Em");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            if(MODREG) {
                v1 = mmx_get_reg(dyn, ninst, x1, x2, x3, nextop&7);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                v1 = fpu_get_scratch_double(dyn);
                VLD1_32(v1, ed);
            }
            VCVTn_F32_S32(v0, v1);
            break;
        case 0x2B:
            INST_NAME("MOVNTPS Ex,Gx");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, nextop&7);
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                VST1Q_32(v0, ed);
            }
            break;
        case 0x2C:
            INST_NAME("CVTTPS2PI Gm, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                v1 = fpu_get_scratch_double(dyn);
                VLD1_32(v1, ed);
            }
            VCVTn_S32_F32(v0, v1);
            break;
        case 0x2D:
            INST_NAME("CVTPS2PI Gm, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                v1 = fpu_get_scratch_double(dyn);
                VLD1_32(v1, ed);
            }
            if(v1<16)
                d1 = v1;
            else {
                d1 = fpu_get_scratch_double(dyn);
                VMOV_64(d1, v1);
            }
            if(v0<16)
                d0 = v0;
            else
                d0 = fpu_get_scratch_double(dyn);
            u8 = sse_setround(dyn, ninst, x1, x2, x14);
            VCVTR_S32_F32(d0*2, d1*2);
            VCVTR_S32_F32(d0*2+1, d1*2+1);
            if(v0>=16) {
                VMOV_64(v0, d0);
            }
            x87_restoreround(dyn, ninst, u8);
            break;
        case 0x2E:
            // no special check...
        case 0x2F:
            if(opcode==0x2F) {INST_NAME("COMISS Gx, Ex");} else {INST_NAME("UCOMISS Gx, Ex");}
            SETFLAGS(X_ALL, SF_SET_NODF);
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
                if(v1<16)
                    d0 = v1;
                else {
                    d0 = fpu_get_scratch_double(dyn);
                    VMOVD(d0, v1);
                }
                s0 = d0*2;
            } else {
                SMREAD();
                parity = getedparity(dyn, ninst, addr, nextop, 2);
                s0 = fpu_get_scratch_single(dyn);
                if(parity) {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL);
                    VLDR_32(s0, ed, fixedaddress);
                } else {
                    GETED;
                    VMOVtoV(s0, ed);
                }
            }
            if(v0<16)
                d1=v0;
            else {
                d1 = fpu_get_scratch_double(dyn);
                VMOVD(d1, v0);
            }
            VCMP_F32(d1*2, s0);
            FCOMI(x1, x2);
            break;

        case 0x31:
            INST_NAME("RDTSC");
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            SKIPTEST(x1, x14);
            CALL(ReadTSC, xEAX, 0);   // will return the u64 in x1:xEAX
            MOV_REG(xEDX, x1);
            break;

        case 0x38:
            //SSE3
            nextop=F8;
            switch(nextop) {
                case 0x00:
                    INST_NAME("PSHUFB Gm, Em");
                    nextop = F8;
                    GETGM(d0);
                    GETEM(d1);
                    v1 = fpu_get_scratch_double(dyn);
                    VMOV_8(v1, 0b10000111);
                    VANDD(v1, d1, v1);  // mask the index
                    VTBL1_8(d0, d0, v1);
                    break;
                case 0x04:
                    INST_NAME("PMADDUBSW Gm,Em");
                    nextop = F8;
                    GETGM(d0);
                    GETEM(d1);
                    v0 = fpu_get_scratch_quad(dyn);
                    v1 = fpu_get_scratch_quad(dyn);
                    VMOVL_U8(v0, d0);   // this is unsigned, so 0 extended
                    VMOVL_S8(v1, d1);   // this is signed
                    VMULQ_16(v0, v0, v1);
                    VPADDLQ_S16(v0, v0);
                    VQMOVN_S32(d0, v0);
                    break;
                case 0x0B:
                    INST_NAME("PMULHRSW Gm,Em");
                    nextop = F8;
                    GETGM(d0);
                    GETEM(d1);
                    VQRDMULH_S16(d0, d0, d1);
                    break;

                case 0xF0:
                    INST_NAME("MOVBE Gd, Ed");
                    nextop=F8;
                    GETGD;
                    if(MODREG) {   // reg <= reg
                        REV(gd, xEAX+(nextop&7));
                    } else {                    // mem <= reg
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 4095, 0, 0, NULL);
                        LDR_IMM9(gd, ed, fixedaddress);
                        REV(gd, gd);
                    }
                    break;
                case 0xF1:
                    INST_NAME("MOVBE Ed, Gd");
                    nextop=F8;
                    GETGD;
                    if(MODREG) {   // reg <= reg
                        REV(xEAX+(nextop&7), gd);
                    } else {                    // mem <= reg
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 4095, 0, 0, NULL);
                        REV(x1, gd);
                        STR_IMM9(x1, ed, fixedaddress);
                        SMWRITE();
                    }
                    break;

                default:
                    DEFAULT;
            }
            break;

        case 0x3A:  // these are some more SSSE3 opcodes
            opcode = F8;
            switch(opcode) {
                case 0x0F:
                    INST_NAME("PALIGNR Gm, Em, Ib");
                    nextop = F8;
                    GETGM(q0);
                    GETEM(q1);
                    u8 = F8;
                    if(u8>15) {
                        VEOR(q0, q0, q0);    
                    } else if(u8>7) {
                        d0 = fpu_get_scratch_quad(dyn);
                        VEOR(d0, d0, d0);
                        VEXT_8(q0, q0, d0, u8-8);
                    } else {
                        VEXT_8(q0, q1, q0, u8);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        
        #define GO(GETFLAGS, NO, YES, F)    \
            READFLAGS(F);                   \
            GETFLAGS;   \
            nextop=F8;  \
            GETGD;      \
            if(MODREG) {   \
                ed = xEAX+(nextop&7);   \
                MOV_REG_COND(YES, gd, ed); \
            } else { \
                SMREAD(); \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 4095, 0, 0, NULL);    \
                LDR_IMM9_COND(YES, gd, ed, fixedaddress); \
            }

        case 0x40:
            INST_NAME("CMOVO Gd, Ed");
            GO( TSTS_IMM8_ROR(xFlags, 0b10, 0x0b)
                , cEQ, cNE, X_OF)
            break;
        case 0x41:
            INST_NAME("CMOVNO Gd, Ed");
            GO( TSTS_IMM8_ROR(xFlags, 0b10, 0x0b)
                , cNE, cEQ, X_OF)
            break;
        case 0x42:
            INST_NAME("CMOVC Gd, Ed");
            GO( TSTS_IMM8(xFlags, 1<<F_CF)
                , cEQ, cNE, X_CF)
            break;
        case 0x43:
            INST_NAME("CMOVNC Gd, Ed");
            GO( TSTS_IMM8(xFlags, 1<<F_CF)
                , cNE, cEQ, X_CF)
            break;
        case 0x44:
            INST_NAME("CMOVZ Gd, Ed");
            GO( TSTS_IMM8(xFlags, 1<<F_ZF)
                , cEQ, cNE, X_ZF)
            break;
        case 0x45:
            INST_NAME("CMOVNZ Gd, Ed");
            GO( TSTS_IMM8(xFlags, 1<<F_ZF)
                , cNE, cEQ, X_ZF)
            break;
        case 0x46:
            INST_NAME("CMOVBE Gd, Ed");
            GO( TSTS_IMM8(xFlags, (1<<F_CF)|(1<<F_ZF))
                , cEQ, cNE, X_CF|X_ZF)
            break;
        case 0x47:
            INST_NAME("CMOVNBE Gd, Ed");
            GO( TSTS_IMM8(xFlags, (1<<F_CF)|(1<<F_ZF))
                , cNE, cEQ, X_CF|X_ZF)
            break;
        case 0x48:
            INST_NAME("CMOVS Gd, Ed");
            GO( TSTS_IMM8(xFlags, 1<<F_SF)
                , cEQ, cNE, X_SF)
            break;
        case 0x49:
            INST_NAME("CMOVNS Gd, Ed");
            GO( TSTS_IMM8(xFlags, 1<<F_SF)
                , cNE, cEQ, X_SF)
            break;
        case 0x4A:
            INST_NAME("CMOVP Gd, Ed");
            GO( TSTS_IMM8(xFlags, 1<<F_PF)
                , cEQ, cNE, X_PF)
            break;
        case 0x4B:
            INST_NAME("CMOVNP Gd, Ed");
            GO( TSTS_IMM8(xFlags, 1<<F_PF)
                , cNE, cEQ, X_PF)
            break;
        case 0x4C:
            INST_NAME("CMOVL Gd, Ed");
            GO( XOR_REG_LSL_IMM5(x1, xFlags, xFlags, F_OF-F_SF);
                TSTS_IMM8_ROR(x1, 0b10, 0x0b)
                , cEQ, cNE, X_SF|X_OF)
            break;
        case 0x4D:
            INST_NAME("CMOVGE Gd, Ed");
            GO( XOR_REG_LSL_IMM5(x1, xFlags, xFlags, F_OF-F_SF);
                TSTS_IMM8_ROR(x1, 0b10, 0x0b)
                , cNE, cEQ, X_SF|X_OF)
            break;
        case 0x4E:
            INST_NAME("CMOVLE Gd, Ed");
            GO( XOR_REG_LSL_IMM5(x1, xFlags, xFlags, F_OF-F_SF);
                ORR_REG_LSL_IMM5(x1, x1, xFlags, F_OF-F_ZF);
                TSTS_IMM8_ROR(x1, 0b10, 0x0b)
                , cEQ, cNE, X_SF|X_OF|X_ZF)
            break;
        case 0x4F:
            INST_NAME("CMOVG Gd, Ed");
            GO( XOR_REG_LSL_IMM5(x1, xFlags, xFlags, F_OF-F_SF);
                ORR_REG_LSL_IMM5(x1, x1, xFlags, F_OF-F_ZF);
                TSTS_IMM8_ROR(x1, 0b10, 0x0b)
                , cNE, cEQ, X_SF|X_OF|X_ZF)
            break;
        #undef GO

        case 0x50:
            INST_NAME("MOVMSPKPS Gd, Ex");
            nextop = F8;
            GETGD;
            XOR_REG_LSL_IMM5(gd, gd, gd, 0);
            if(MODREG) {
                // EX is an xmm reg
                GETEX(q0, 0);
                VMOVfrV_D(x1, x2, q0);
                UBFX(x3, x1, 31, 1);
                BFI(gd, x3, 0, 1);
                UBFX(x3, x2, 31, 1);
                BFI(gd, x3, 1, 1);
                VMOVfrV_D(x1, x2, q0+1);
                UBFX(x3, x1, 31, 1);
                BFI(gd, x3, 2, 1);
                UBFX(x3, x2, 31, 1);
                BFI(gd, x3, 3, 1);
            } else {
                // EX is memory
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4096-16, 0, 0, NULL);
                LDR_IMM9(x2, ed, fixedaddress+0);
                UBFX(x3, x2, 31, 1);
                BFI(gd, x3, 0, 1);
                LDR_IMM9(x2, ed, fixedaddress+4);
                UBFX(x3, x2, 31, 1);
                BFI(gd, x3, 1, 1);
                LDR_IMM9(x2, ed, fixedaddress+8);
                UBFX(x3, x2, 31, 1);
                BFI(gd, x3, 2, 1);
                LDR_IMM9(x2, ed, fixedaddress+12);
                UBFX(x3, x2, 31, 1);
                BFI(gd, x3, 3, 1);
            }
            break;
        case 0x51:
            INST_NAME("SQRTPS Gx, Ex");
            SKIPTEST(x1, x14);
            nextop = F8;
            GETEX(q0, 0);
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            #if 0
            // the approximate is not precise enough and and some incorrect behavior
            v2 = fpu_get_scratch_quad(dyn);
            v1 = fpu_get_scratch_quad(dyn);
            // Newton-Raphson: Xn+1 = Xn*(3-d*Xn²)/2 converge to 1/sqrt(d) with X0 = result of VSQRTE
            // and VSQRTS is (3-A*B)/2
            // so VSQRTE(d) -> X    is initial step
            // VSQRTS(d,X*X)*X -> X loop
            // than d*X -> SQRT(d) (because X ~ 1/sqrt(d))
            VRSQRTEQ_F32(v2, q0);       // v2 = X0 (=X), q0 = d
            VMULQ_F32(v1, v2, v2);      // v1 = X0²
            VRSQRTSQ_F32(v1, v1, q0);   // v1 = (3-d*X0²)/2
            VMULQ_F32(v2, v2, v1);      // v2 = X0*(3-d*X0²)/2 = X1
            VMULQ_F32(v1, v2, v2);      // v1 = x1²
            VRSQRTSQ_F32(v1, v1, q0);   // v1 = (3-d*X1²)/2
            VMULQ_F32(v2, v2, v1);      // v2 = X1*(3-d*X1²)/2 = X2
            if(!box86_dynarec_fastnan) {
                // need to preserve v0, in case it's == q0
                VMULQ_F32(v1, v2, q0);      // v1 = X2*d ~ SQRT(d)
                if(v0<dyn->n.fpu_scratch || v0!=q0)
                    q1 = v0;    // v0 is a scratch register
                else
                    q1 = fpu_get_scratch_quad(dyn); // need a new scratch
                VCEQQ_0_F32(q1, q0);    // prepare mask, 111 when == 0.0
                VMOVQ_H32(v2, 0xff);    // prepare +inf
                VSHRQ_U32(v2, v2, 1);   // 0xff000000 -> 0x7f800000 == +inf
                VCEQQ_32(v2, v2, q0);   // v2 mask: 111 when == +inf
                VORRQ(q1, q1, v2);      // q1 mask: 111 when ==0 || == +inf
                VBICQ(v1, v1, q1);      // mask value in result
                VANDQ(q1, q0, q1);      // keep original value that are 0 or +inf
                VORRQ(v0, v1, q1);      // copy over
            } else {
                VMULQ_F32(v0, v2, q0);  // v0 = X2*d ~ SQRT(d)
            }
            #else
            if((q0<15) && (v0<15)) {
                d0 = q0;
                d1 = v0;
            } else {
                s0 = fpu_get_scratch_quad(dyn);
                // source ok or not
                if(q0<15) {
                    d0 = q0;
                } else {
                    VMOVQ(s0, q0);
                    d0 = s0;
                }
                if(v0<15)
                    d1 = v0;
                else
                    d1 = s0;
            }
            for(int i=0; i<4; ++i) {
                VSQRT_F32(d1*2+i, d0*2+i);
            }
            if(v0<15) {
                //done
            } else {
                VMOVQ(v0, d1);
            }
            #endif
            break;
        case 0x52:
            INST_NAME("RSQRTPS Gx, Ex");
            SKIPTEST(x1, x14);
            nextop = F8;
            GETEX(q0, 0);
            gd = (nextop&0x38)>>3;
            q1 = sse_get_reg_empty(dyn, ninst, x1, gd);
            v0 = fpu_get_scratch_quad(dyn);
            if(q1==q0 || !box86_dynarec_fastnan)
                v1 = fpu_get_scratch_quad(dyn);
            else
                v1 = q1;
            VRSQRTEQ_F32(v0, q0);
            VMULQ_F32(v1, v0, q0);
            VRSQRTSQ_F32(v1, v1, v0);
            VMULQ_F32(q1, v1, v0);
            if(!box86_dynarec_fastnan) {
                VCEQQ_F32(v1, q1, q1);  // 00 is nan
                VANDQ(q1, q1, v1);  // if nan, remove
                VBICQ(v0, v0, v1);  // if nan, keep
                VORRQ(q1, q1, v0);
            }
            break;
        case 0x53:
            INST_NAME("RCPPS Gx, Ex");
            SKIPTEST(x1, x14);
            nextop = F8;
            GETEX(q0, 0);
            gd = (nextop&0x38)>>3;
            q1 = sse_get_reg_empty(dyn, ninst, x1, gd);
            if(q0 == q1)
                v1 = fpu_get_scratch_quad(dyn);
            else
                v1 = q1;
            v0 = fpu_get_scratch_quad(dyn);
            VRECPEQ_F32(v0, q0);
            VRECPSQ_F32(v1, v0, q0);
            VMULQ_F32(q1, v0, v1);
            break;
        case 0x54:
            INST_NAME("ANDPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0, 1);
            VANDQ(v0, v0, q0);
            break;
        case 0x55:
            INST_NAME("ANDNPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0, 1);
            VBICQ(v0, q0, v0);
            break;
        case 0x56:
            INST_NAME("ORPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0, 1);
            VORRQ(v0, v0, q0);
            break;
        case 0x57:
            INST_NAME("XORPS Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if((nextop&0xC7)==(0xC0|gd)) {
                q0 = v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            } else {
                GETEX(q0, 0);
                GETGX(v0, 1);
            }
            VEORQ(v0, v0, q0);
            break;
        case 0x58:
            INST_NAME("ADDPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0, 1);
            VADDQ_F32(v0, v0, q0);
            break;
        case 0x59:
            INST_NAME("MULPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0, 1);
            VMULQ_F32(v0, v0, q0);
            break;
        case 0x5A:
            INST_NAME("CVTPS2PD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            if(q0<16) {
                d0 = q0;
            } else {
                d0 = fpu_get_scratch_double(dyn);
                VMOVD(d0, q0);
            }
            VCVT_F64_F32(v0+1, d0*2+1);
            VCVT_F64_F32(v0, d0*2);
            break;
        case 0x5B:
            INST_NAME("CVTDQ2PS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            VCVTQ_F32_S32(v0, q0);
            break;
        case 0x5C:
            INST_NAME("SUBPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0, 1);
            VSUBQ_F32(v0, v0, q0);
            break;
        case 0x5D:
            INST_NAME("MINPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0);
            // VMIN/VMAX wll put default NaN if any input is NaN
            // but x86 will copy if either v0[x] or v1[x] is NaN, so lets force a copy if source is NaN
            if(!box86_dynarec_fastnan) {
                if(v0==v1) {
                    q1 = fpu_get_scratch_quad(dyn);
                    VMOVQ(q1, v1);
                } else
                    q1 = v1;
            }
            VMINQ_F32(v0, v0, v1);
            if(!box86_dynarec_fastnan) {
                q0 = fpu_get_scratch_quad(dyn);
                VCEQQ_F32(q0, v0, v0);   // 0 is NaN, -1 is not NaN
                VANDQ(v0, v0, q0);
                VBICQ(q0, q1, q0);
                VORRQ(v0, v0, q0);
            }
            break;
        case 0x5E:
            INST_NAME("DIVPS Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(q1, 1);
            // use Gx * 1/Ex, using RECP
            // Newton-Raphson: Xn+1 = Xn*(2-d*Xn) converge to 1/d if X0 if result of VRECPE(d)
            // VRECPE is 2-A*B
            v1 = fpu_get_scratch_quad(dyn);
            v2 = fpu_get_scratch_quad(dyn);
            VRECPEQ_F32(v2, q0);        // v2 = X0, q0 = d
            VRECPSQ_F32(v1, q0, v2);    // v1 = 2-d*X0
            VMULQ_F32(v2, v2, v1);      // v2 = X0*(2-d*X0) = X1
            VRECPSQ_F32(v1, q0, v2);    // v1 = 2-d*X1
            VMULQ_F32(v2, v2, v1);      // v2 = X1*(2-d*X1) = X2 ~ 1/d
            VMULQ_F32(q1, q1, v2);
            break;
        case 0x5F:
            INST_NAME("MAXPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0);
            // VMIN/VMAX wll put default NaN if any input is NaN
            // but x86 will copy if either v0[x] or v1[x] is NaN, so lets force a copy if source is NaN
            if(!box86_dynarec_fastnan) {
                if(v0==v1) {
                    q1 = fpu_get_scratch_quad(dyn);
                    VMOVQ(q1, v1);
                } else
                    q1 = v1;
            }
            VMAXQ_F32(v0, v0, v1);
            if(!box86_dynarec_fastnan) {
                q0 = fpu_get_scratch_quad(dyn);
                VCEQQ_F32(q0, v0, v0);   // 0 is NaN, -1 is not NaN
                VANDQ(v0, v0, q0);
                VBICQ(q0, q1, q0);
                VORRQ(v0, v0, q0);
            }
            break;
        case 0x60:
            INST_NAME("PUNPCKLBW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            if(MODREG) {
                v0 = fpu_get_scratch_double(dyn);
                VMOVD(v0, d1);
            } else v0 = d1;
            VZIP_8(d0, v0);
            break;
        case 0x61:
            INST_NAME("PUNPCKLWD Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            if(MODREG) {
                v0 = fpu_get_scratch_double(dyn);
                VMOVD(v0, d1);
            } else v0 = d1;
            VZIP_16(d0, v0);
            break;
        case 0x62:
            INST_NAME("PUNPCKLDQ Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            if(MODREG) {
                v0 = fpu_get_scratch_double(dyn);
                VMOVD(v0, d1);
            } else v0 = d1;
            VZIP_32(d0, v0);
            break;
        case 0x63:
            INST_NAME("PACKSSWB Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1);
            q0 = fpu_get_scratch_quad(dyn);
            VMOVD(q0+0, v0);
            VMOVD(q0+1, v1);
            VQMOVN_S16(v0, q0);
            break;
        case 0x64:
            INST_NAME("PCMPGTB Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VCGT_S8(d0, d0, d1);
            break;
        case 0x65:
            INST_NAME("PCMPGTW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VCGT_S16(d0, d0, d1);
            break;
        case 0x66:
            INST_NAME("PCMPGTD Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VCGT_S32(d0, d0, d1);
            break;
        case 0x67:
            INST_NAME("PACKUSWB Gm,Em");
            nextop = F8;
            GETGM(d0);
            q0 = fpu_get_scratch_quad(dyn);
            if(MODREG) {
                d1 = mmx_get_reg(dyn, ninst, x1, x2, x3, nextop&7);
                VMOVD(q0+1, d1);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                VLD1_64(q0+1, ed);
            }
            VMOVD(q0, d0);
            VQMOVUN_S16(d0, q0);
            break;
        case 0x68:
            INST_NAME("PUNPCKHBW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            if(MODREG) {
                v0 = fpu_get_scratch_double(dyn);
                VMOVD(v0, d1);
            } else v0 = d1;
            VZIP_8(d0, v0);
            VMOVD(d0, v0);
            break;
        case 0x69:
            INST_NAME("PUNPCKHWD Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            if(MODREG) {
                v0 = fpu_get_scratch_double(dyn);
                VMOVD(v0, d1);
            } else v0 = d1;
            VZIP_16(d0, v0);
            VMOVD(d0, v0);
            break;
        case 0x6A:
            INST_NAME("PUNPCKHDQ Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            if(MODREG) {
                v0 = fpu_get_scratch_double(dyn);
                VMOVD(v0, d1);
            } else v0 = d1;
            VZIP_32(d0, v0);
            VMOVD(d0, v0);
            break;
        case 0x6B:
            INST_NAME("PACKSSDW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1);
            if(!(v0&1) && (v1==(v0+1))) {
                q0 = v0;    // no need to move the regs...
            } else {
                q0 = fpu_get_scratch_quad(dyn);
                VMOVD(q0+0, v0);
                VMOVD(q0+1, v1);
            }
            VQMOVN_S32(v0, q0);
            break;

        case 0x6E:
            INST_NAME("MOVD Gm, Ed");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = mmx_get_reg_empty(dyn, ninst, x2, x3, x14, gd);
            GETED;
            VEOR(v0, v0, v0);
            VMOVtoDx_32(v0, 0, ed);
            break;
        case 0x6F:
            INST_NAME("MOVQ Gm, Em");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if(MODREG) {
                v1 = mmx_get_reg(dyn, ninst, x1, x2, x3, nextop&7);
                v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
                VMOVD(v0, v1);
            } else {
                v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                VLD1_64(v0, ed);
            }
            break;
        case 0x70:
            INST_NAME("PSHUFW Gm,Em,Ib");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            i32 = -1;
            v0 = mmx_get_reg(dyn, ninst, x1, x2, x3, gd);
            if(MODREG) {
                u8 = F8;
                v1 = mmx_get_reg(dyn, ninst, x1, x2, x3, nextop&7);
                // use stack as temporary storage
                SUB_IMM8(xSP, xSP, 4);
                if(v1==v0) {
                    d0 = fpu_get_scratch_double(dyn);
                    VMOVD(d0, v1);
                } else d0 = v1;
                if (u8) {
                    for (int i=0; i<4; ++i) {
                        int32_t idx = (u8>>(i*2))&3;
                        if(idx!=i32) {
                            VST1LANE_16(d0, xSP, idx);
                            i32 = idx;
                        }
                        VLD1LANE_16(v0, xSP, i);
                    }
                } else {
                    VST1LANE_16(v1, xSP, 0);
                    VLD1ALL_16(v0, xSP);
                }
                ADD_IMM8(xSP, xSP, 4);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                u8 = F8;
                if (u8) {
                    for (int i=0; i<4; ++i) {
                        int32_t idx = (u8>>(i*2))&3;
                        if(idx!=i32) {
                            ADD_IMM8(x2, ed, idx*2);
                            i32 = idx;
                        }
                        VLD1LANE_16(v0, x2, i);
                    }
                } else {
                    VLD1ALL_16(v0, ed);
                }
            }
            break;
        case 0x71:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLW Em, Ib");
                    if(MODREG) {
                        d0 = mmx_get_reg(dyn, ninst, x1, x2, x3, nextop&7);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        d0 = fpu_get_scratch_quad(dyn);
                        VLD1_16(d0, ed);
                    }
                    u8 = F8;
                    if(u8) {
                        if (u8>15) {
                            VEOR(d0, d0, d0);
                        } else if(u8) {
                            VSHR_U16(d0, d0, u8);
                        }
                        if((nextop&0xC0)!=0xC0) {
                            VST1_16(d0, ed);
                            SMWRITE2();
                        }
                    }
                    break;
                case 4:
                    INST_NAME("PSRAW Em, Ib");
                    if(MODREG) {
                        d0 = mmx_get_reg(dyn, ninst, x1, x2, x3, nextop&7);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        d0 = fpu_get_scratch_quad(dyn);
                        VLD1_16(d0, ed);
                    }
                    u8 = F8;
                    if(u8) {
                        VSHR_S16(d0, d0, (u8>15)?0:u8);
                    }
                    if((nextop&0xC0)!=0xC0) {
                        VST1_16(d0, ed);
                        SMWRITE2();
                    }
                    break;
                case 6:
                    INST_NAME("PSLLW Em, Ib");
                    if(MODREG) {
                        d0 = mmx_get_reg(dyn, ninst, x1, x2, x3, nextop&7);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        d0 = fpu_get_scratch_quad(dyn);
                        VLD1_16(d0, ed);
                    }
                    u8 = F8;
                    if(u8) {
                        if (u8>15) {
                            VEOR(d0, d0, d0);
                        } else {
                            VSHL_16(d0, d0, u8);
                        }
                        if((nextop&0xC0)!=0xC0) {
                            VST1_16(d0, ed);
                            SMWRITE2();
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x72:
            nextop = F8;
            SMREAD();
            GETEM(v0);
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLD Em, Ib");
                    u8 = F8;
                    if(u8) {
                        if (u8>31) {
                            VEOR(v0, v0, v0);
                        } else {
                            VSHR_U32(v0, v0, u8);
                        }
                        PUTEM(v0);
                    }
                    break;
                case 4:
                    INST_NAME("PSRAD Em, Ib");
                    u8 = F8;
                    if(u8) {
                        VSHR_S32(v0, v0, (u8>31)?0:u8);
                        PUTEM(v0);
                    }
                    break;
                case 6:
                    INST_NAME("PSLLD Em, Ib");
                    u8 = F8;
                    if (u8) {
                        if (u8>31) {
                            VEOR(v0, v0, v0);
                        } else {
                            VSHL_32(v0, v0, u8);
                        }
                        PUTEM(v0);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x73:
            nextop = F8;
            GETEM(v0);
            SMREAD();
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLQ Em, Ib");
                    u8 = F8;
                    if(u8) {
                        if(u8>63)
                            {VEOR(v0, v0, v0);}
                        else
                            {VSHR_U64(v0, v0, u8);}
                        PUTEM(v0);
                    }
                    break;
                case 6:
                    INST_NAME("PSLLQ Em, Ib");
                    u8 = F8;
                    if(u8) {
                        if(u8>63)
                            {VEOR(v0, v0, v0);}
                        else
                            {VSHL_64(v0, v0, u8);}
                        PUTEM(v0);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x74:
            INST_NAME("PCMPEQB Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VCEQ_8(d0, d0, d1);
            break;
        case 0x75:
            INST_NAME("PCMPEQW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VCEQ_16(d0, d0, d1);
            break;
        case 0x76:
            INST_NAME("PCMPEQD Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VCEQ_32(d0, d0, d1);
            break;
        case 0x77:
            INST_NAME("EMMS");
            // empty MMX, FPU now usable
            mmx_purgecache(dyn, ninst, 0, x1);
            break;

        case 0x7E:
            INST_NAME("MOVD Ed, Gm");
            nextop = F8;
            GETGM(v0);
            if(MODREG) {
                ed = xEAX + (nextop&7);
                VMOVfrDx_32(ed, v0, 0);
            } else {
                VMOVfrDx_32(x2, v0, 0); // there can be some bus error is storing the VFPU reg directly
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0, 0, NULL);
                STR_IMM9(x2, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x7F:
            INST_NAME("MOVQ Em, Gm");
            nextop = F8;
            GETGM(v0);
            if(MODREG) {
                v1 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, nextop&7);
                VMOVD(v1, v0);
            } else {
                parity = getedparity(dyn, ninst, addr, nextop, 3);
                if(parity) {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL);
                    VSTR_64(v0, ed, fixedaddress);
                    SMWRITE2();
                } else {
                    VMOVfrV_D(x2, x3, v0);
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                    STR_IMM9(x2, ed, fixedaddress);
                    STR_IMM9(x3, ed, fixedaddress+4);
                    SMWRITE2();
                }
            }
            break;

        #define GO(GETFLAGS, NO, YES, F)   \
            READFLAGS(F);                                               \
            i32_ = F32S;                                                \
            BARRIER(BARRIER_MAYBE);                                     \
            JUMP(addr+i32_, 1);                                         \
            GETFLAGS;                                                   \
            if(dyn->insts[ninst].x86.jmp_insts==-1 ||                   \
                CHECK_CACHE()) {                                        \
                /* out of the block */                                  \
                i32 = dyn->insts[ninst].epilog-(dyn->arm_size+8);       \
                Bcond(NO, i32);                                         \
                if(dyn->insts[ninst].x86.jmp_insts==-1) {               \
                    if(!(dyn->insts[ninst].x86.barrier&BARRIER_FLOAT))  \
                        fpu_purgecache(dyn, ninst, 1, x1, x2, x3);      \
                    jump_to_next(dyn, addr+i32_, 0, ninst);             \
                } else {                                                \
                    CacheTransform(dyn, ninst, cacheupd, x1, x2, x3);   \
                    i32 = dyn->insts[dyn->insts[ninst].x86.jmp_insts].address-(dyn->arm_size+8);\
                    Bcond(c__, i32);                                    \
                }                                                       \
            } else {                                                    \
                /* inside the block */                                  \
                i32 = dyn->insts[dyn->insts[ninst].x86.jmp_insts].address-(dyn->arm_size+8);    \
                Bcond(YES, i32);                                        \
            }

        case 0x80:
            INST_NAME("JO id");
            GO( TSTS_IMM8_ROR(xFlags, 0b10, 0x0b)
                , cEQ, cNE, X_OF)
            break;
        case 0x81:
            INST_NAME("JNO id");
            GO( TSTS_IMM8_ROR(xFlags, 0b10, 0x0b)
                , cNE, cEQ, X_OF)
            break;
        case 0x82:
            INST_NAME("JC id");
            GO( TSTS_IMM8(xFlags, 1<<F_CF)
                , cEQ, cNE, X_CF)
            break;
        case 0x83:
            INST_NAME("JNC id");
            GO( TSTS_IMM8(xFlags, 1<<F_CF)
                , cNE, cEQ, X_CF)
            break;
        case 0x84:
            INST_NAME("JZ id");
            GO( TSTS_IMM8(xFlags, 1<<F_ZF)
                , cEQ, cNE, X_ZF)
            break;
        case 0x85:
            INST_NAME("JNZ id");
            GO( TSTS_IMM8(xFlags, 1<<F_ZF)
                , cNE, cEQ, X_ZF)
            break;
        case 0x86:
            INST_NAME("JBE id");
            GO( TSTS_IMM8(xFlags, (1<<F_CF)|(1<<F_ZF))
                , cEQ, cNE, X_CF|X_ZF)
            break;
        case 0x87:
            INST_NAME("JNBE id");
            GO( TSTS_IMM8(xFlags, (1<<F_CF)|(1<<F_ZF))
                , cNE, cEQ, X_CF|X_ZF)
            break;
        case 0x88:
            INST_NAME("JS id");
            GO( TSTS_IMM8(xFlags, 1<<F_SF)
                , cEQ, cNE, X_SF)
            break;
        case 0x89:
            INST_NAME("JNS id");
            GO( TSTS_IMM8(xFlags, 1<<F_SF)
                , cNE, cEQ, X_SF)
            break;
        case 0x8A:
            INST_NAME("JP id");
            GO( TSTS_IMM8(xFlags, 1<<F_PF)
                , cEQ, cNE, X_PF)
            break;
        case 0x8B:
            INST_NAME("JNP id");
            GO( TSTS_IMM8(xFlags, 1<<F_PF)
                , cNE, cEQ, X_PF)
            break;
        case 0x8C:
            INST_NAME("JL id");
            GO( XOR_REG_LSL_IMM5(x1, xFlags, xFlags, F_OF-F_SF);
                TSTS_IMM8_ROR(x1, 0b10, 0x0b)
                , cEQ, cNE, X_SF|X_OF)
            break;
        case 0x8D:
            INST_NAME("JGE id");
            GO( XOR_REG_LSL_IMM5(x1, xFlags, xFlags, F_OF-F_SF);
                TSTS_IMM8_ROR(x1, 0b10, 0x0b)
                , cNE, cEQ, X_SF|X_OF)
            break;
        case 0x8E:
            INST_NAME("JLE id");
            GO( XOR_REG_LSL_IMM5(x1, xFlags, xFlags, F_OF-F_SF);
                ORR_REG_LSL_IMM5(x1, x1, xFlags, F_OF-F_ZF);
                TSTS_IMM8_ROR(x1, 0b10, 0x0b)
                , cEQ, cNE, X_SF|X_OF|X_ZF)
            break;
        case 0x8F:
            INST_NAME("JG id");
            GO( XOR_REG_LSL_IMM5(x1, xFlags, xFlags, F_OF-F_SF);
                ORR_REG_LSL_IMM5(x1, x1, xFlags, F_OF-F_ZF);
                TSTS_IMM8_ROR(x1, 0b10, 0x0b)
                , cNE, cEQ, X_SF|X_OF|X_ZF)
            break;
        #undef GO

        #define GO(GETFLAGS, NO, YES, F)    \
            READFLAGS(F);                   \
            GETFLAGS;   \
            nextop=F8;  \
            MOVW_COND(NO, x3, 0); \
            MOVW_COND(YES, x3, 1);  \
            if(MODREG) { \
                ed = (nextop&7);    \
                eb1 = xEAX+(ed&3);  \
                eb2 = ((ed&4)>>2);  \
                BFI(eb1, x3, eb2*8, 8); \
            } else {                \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 4095, 0, 0, NULL);\
                STRB_IMM9(x3, ed, fixedaddress);   \
                SMWRITE();          \
            }
        case 0x90:
            INST_NAME("SETO Eb");
            GO( TSTS_IMM8_ROR(xFlags, 0b10, 0x0b)
                , cEQ, cNE, X_OF)
            break;
        case 0x91:
            INST_NAME("SETNO Eb");
            GO( TSTS_IMM8_ROR(xFlags, 0b10, 0x0b)
                , cNE, cEQ, X_OF)
            break;
        case 0x92:
            INST_NAME("SETC Eb");
            GO( TSTS_IMM8(xFlags, 1<<F_CF)
                , cEQ, cNE, X_CF)
            break;
        case 0x93:
            INST_NAME("SETNC Eb");
            GO( TSTS_IMM8(xFlags, 1<<F_CF)
                , cNE, cEQ, X_CF)
            break;
        case 0x94:
            INST_NAME("SETZ Eb");
            GO( TSTS_IMM8(xFlags, 1<<F_ZF)
                , cEQ, cNE, X_ZF)
            break;
        case 0x95:
            INST_NAME("SETNZ Eb");
            GO( TSTS_IMM8(xFlags, 1<<F_ZF)
                , cNE, cEQ, X_ZF)
            break;
        case 0x96:
            INST_NAME("SETBE Eb");
            GO( TSTS_IMM8(xFlags, (1<<F_CF)|(1<<F_ZF))
                , cEQ, cNE, X_CF|X_ZF)
            break;
        case 0x97:
            INST_NAME("SETNBE Eb");
            GO( TSTS_IMM8(xFlags, (1<<F_CF)|(1<<F_ZF))
                , cNE, cEQ, X_CF|X_ZF)
            break;
        case 0x98:
            INST_NAME("SETS Eb");
            GO( TSTS_IMM8(xFlags, 1<<F_SF)
                , cEQ, cNE, X_SF)
            break;
        case 0x99:
            INST_NAME("SETNS Eb");
            GO( TSTS_IMM8(xFlags, 1<<F_SF)
                , cNE, cEQ, X_SF)
            break;
        case 0x9A:
            INST_NAME("SETP Eb");
            GO( TSTS_IMM8(xFlags, 1<<F_PF)
                , cEQ, cNE, X_PF)
            break;
        case 0x9B:
            INST_NAME("SETNP Eb");
            GO( TSTS_IMM8(xFlags, 1<<F_PF)
                , cNE, cEQ, X_PF)
            break;
        case 0x9C:
            INST_NAME("SETL Eb");
            GO( XOR_REG_LSL_IMM5(x1, xFlags, xFlags, F_OF-F_SF);
                TSTS_IMM8_ROR(x1, 0b10, 0x0b)
                , cEQ, cNE, X_SF|X_OF)
            break;
        case 0x9D:
            INST_NAME("SETGE Eb");
            GO( XOR_REG_LSL_IMM5(x1, xFlags, xFlags, F_OF-F_SF);
                TSTS_IMM8_ROR(x1, 0b10, 0x0b)
                , cNE, cEQ, X_SF|X_OF)
            break;
        case 0x9E:
            INST_NAME("SETLE Eb");
            GO( XOR_REG_LSL_IMM5(x1, xFlags, xFlags, F_OF-F_SF);
                ORR_REG_LSL_IMM5(x1, x1, xFlags, F_OF-F_ZF);
                TSTS_IMM8_ROR(x1, 0b10, 0x0b)
                , cEQ, cNE, X_SF|X_OF|X_ZF)
            break;
        case 0x9F:
            INST_NAME("SETG Eb");
            GO( XOR_REG_LSL_IMM5(x1, xFlags, xFlags, F_OF-F_SF);
                ORR_REG_LSL_IMM5(x1, x1, xFlags, F_OF-F_ZF);
                TSTS_IMM8_ROR(x1, 0b10, 0x0b)
                , cNE, cEQ, X_SF|X_OF|X_ZF)
            break;
        #undef GO

        case 0xA0:
            INST_NAME("PUSH FS");
            MOVW(x1, offsetof(x86emu_t, segs[_FS]));
            LDRH_REG(x2, xEmu, x1);
            PUSH1(x2);
            break;
        case 0xA1:
            INST_NAME("POP FS");
            MOVW(x1, offsetof(x86emu_t, segs[_FS]));
            POP1(x2);
            STRH_REG(x2, xEmu, x1);
            MOVW(x1, 0);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, segs_serial[_FS]));
            break;
        case 0xA2:
            INST_NAME("CPUID");
            MOV_REG(x1, xEAX);
            MOV32(x14, ip+2);   // EIP is useless, but why not...
            // not purging stuff like x87 here, there is no float math or anything
            STM(xEmu, (1<<xEAX)|(1<<xEBX)|(1<<xECX)|(1<<xEDX)|(1<<xESI)|(1<<xEDI)|(1<<xESP)|(1<<xEBP)|(1<<xEIP)|(1<<xFlags));
            CALL_S(my_cpuid, -1, 0);
            LDM(xEmu, (1<<xEAX)|(1<<xEBX)|(1<<xECX)|(1<<xEDX)|(1<<xESI)|(1<<xEDI)|(1<<xESP)|(1<<xEBP)|(1<<xEIP)|(1<<xFlags));
            break;
        case 0xA3:
            INST_NAME("BT Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE(x1);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xEAX+(nextop&7);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, 4095, 0, 0, NULL);
                MOV_REG_ASR_IMM5(x1, gd, 5);    // r1 = (gd>>5);
                ADD_REG_LSL_IMM5(x1, ed, x1, 2); //(&ed)+=r1*4;
                LDR_IMM9(x1, x1, fixedaddress);
                ed = x1;
            }
            AND_IMM8(x2, gd, 0x1f);
            MOV_REG_LSR_REG(x1, ed, x2);
            BFI(xFlags, x1, F_CF, 1);
            break;
        case 0xA4:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            GETED;
            GETGD;
            u8 = F8;
            emit_shld32c(dyn, ninst, ed, gd, u8&0x1f, x3, x14);
            WBACK;
            break;
        case 0xA5:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, CL");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            AND_IMM8(x3, xECX, 0x1f);
            GETED;
            GETGD;
            emit_shld32(dyn, ninst, ed, gd, x3, x14);
            WBACK;
            break;

        case 0xA8:
            INST_NAME("PUSH GS");
            MOVW(x1, offsetof(x86emu_t, segs[_GS]));
            LDRH_REG(x2, xEmu, x1);
            PUSH1(x2);
            break;
        case 0xA9:
            INST_NAME("POP GS");
            MOVW(x1, offsetof(x86emu_t, segs[_GS]));
            POP1(x2);
            STRH_REG(x2, xEmu, x1);
            MOVW(x1, 0);
            STR_IMM9(x1, xEmu, offsetof(x86emu_t, segs_serial[_GS]));
            break;

        case 0xAB:
            INST_NAME("BTS Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE(x1);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xEAX+(nextop&7);
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 4095, 0, 0, NULL);
                MOV_REG_ASR_IMM5(x1, gd, 5);    // r1 = (gd>>5);
                ADD_REG_LSL_IMM5(x3, wback, x1, 2); //(&ed)+=r1*4;
                LDR_IMM9(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            AND_IMM8(x2, gd, 0x1f);
            MOV_REG_LSR_REG(x14, ed, x2);
            ANDS_IMM8(x14, x14, 1);
            IFX(X_PEND | X_CF) {
                BFI(xFlags, x14, F_CF, 1);
            }
            B_NEXT(cNE); // bit already set, jump to next instruction
            MOVW(x14, 1);
            ORR_REG_LSL_REG(ed, ed, x14, x2);
            if(wback) {
                STR_IMM9(ed, wback, fixedaddress);
                SMWRITE();
            }
            break;
        case 0xAC:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, Ib");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            GETED;
            GETGD;
            u8 = F8;
            emit_shrd32c(dyn, ninst, ed, gd, u8, x3, x14);
            WBACK;
            break;
        case 0xAD:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, CL");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            AND_IMM8(x3, xECX, 0x1f);
            GETED;
            GETGD;
            emit_shrd32(dyn, ninst, ed, gd, x3, x14);
            WBACK;
            break;
        case 0xAE:
            nextop = F8;
            // xFENCE -> DMB is approximative, but should be good enough
            if((nextop&0xF8)==0xE8) {
                INST_NAME("LFENCE");
                SMDMB();
            } else
            if((nextop&0xF8)==0xF0) {
                INST_NAME("MFENCE");
                SMDMB();
            } else
            if((nextop&0xF8)==0xF8) {
                INST_NAME("SFENCE");
                SMDMB();
            } else {
                switch((nextop>>3)&7) {
                    case 0:
                        INST_NAME("FXSAVE Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        // A purge is probably not needed, just a full refresh
                        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                        if(MODREG) {
                            DEFAULT;
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                            if(ed!=x1) {MOV_REG(x1, ed);}
                            CALL(fpu_fxsave, -1, 0);
                        }
                        break;
                    case 1:
                        INST_NAME("FXRSTOR Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
                        if(MODREG) {
                            DEFAULT;
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                            if(ed!=x1) {MOV_REG(x1, ed);}
                            CALL(fpu_fxrstor, -1, 0);
                        }
                        break;
                    case 2:                 
                        INST_NAME("LDMXCSR Md");
                        GETED;
                        STR_IMM9(ed, xEmu, offsetof(x86emu_t, mxcsr));
                        if(box86_sse_flushto0) {
                            VMRS(x14);                      // get fpscr
                            MOV_REG_LSR_IMM5(x3, ed, 15);   // get FZ bit
                            BFI(x14, x3, 24, 1);            // inject FZ bit
                            VMSR(x14);                      // put new fpscr
                        }
                        break;
                    case 3:
                        INST_NAME("STMXCSR Md");
                        if(MODREG) {
                            ed = xEAX+(nextop&7);
                            LDR_IMM9(ed, xEmu, offsetof(x86emu_t, mxcsr));
                        } else {
                            addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 4095, 0, 0, NULL);
                            LDR_IMM9(x14, xEmu, offsetof(x86emu_t, mxcsr));
                            STR_IMM9(x14, ed, fixedaddress);
                        }
                        break;
                    case 5:
                        INST_NAME("XRSTOR Ed (not implemented");
                        FAKEED;
                        SETFLAGS(X_ALL, SF_SET_NODF);    // Hack to set flags in "don't care" state
                        STM(xEmu, (1<<xEAX)|(1<<xECX)|(1<<xEDX)|(1<<xEBX)|(1<<xESP)|(1<<xEBP)|(1<<xESI)|(1<<xEDI)|(1<<xFlags));
                        STR_IMM9(xEIP, xEmu, offsetof(x86emu_t, ip));
                        CALL(arm_ud, -1, 0);
                        LDR_IMM9(xEIP, xEmu, offsetof(x86emu_t, ip));
                        jump_to_epilog(dyn, 0, xEIP, ninst);
                        *need_epilog = 0;
                        *ok = 0;
                        break;
                    case 7:
                        INST_NAME("CLFLUSH Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization?\n");
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        if(ed!=x1) {MOV_REG(x1, ed);}
                        CALL_(arm_clflush, -1, 0);
                        break;
                    default:
                        DEFAULT;
                }
            }
            break;
        case 0xAF:
            INST_NAME("IMUL Gd, Ed");
            SETFLAGS(X_ALL, SF_PENDING);
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
            break;

        case 0xB0:
            INST_NAME("CMPXCHG Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x2)
            UXTB(x1, xEAX, 0);
            CMPS_REG_LSL_IMM5(x1, ed, 0);
            B_MARK(cNE);
            // AL == Eb
            GETGB(x1);
            MOV_REG(ed, x1);
            EBBACK;
            B_MARK3(c__);
            MARK;
            // AL != Eb
            BFI(xEAX, ed, 0, 8);
            BFC(xFlags, F_ZF, 1);
            MARK3;
            // done, do the cmp now
            emit_cmp8(dyn, ninst, x1, x2, x3, x14);
            break;
        case 0xB1:
            INST_NAME("CMPXCHG Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETED;
            GETGD;
            CMPS_REG_LSL_IMM5(xEAX, ed, 0);
            B_MARK(cNE);
            // EAX == Ed
            MOV_REG(x3, ed);
            MOV_REG(ed, gd);
            WBACK;
            emit_cmp32(dyn, ninst, xEAX, x3, x1, x14);
            B_MARK3(c__);   // not next, in case its called with a LOCK prefix
            MARK;
            // EAX != Ed
            emit_cmp32(dyn, ninst, xEAX, ed, x3, x14);
            MOV_REG(xEAX, ed);
            MARK3
            break;
        case 0xB3:
            INST_NAME("BTR Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE(x1);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xEAX+(nextop&7);
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 4095, 0, 0, NULL);
                MOV_REG_ASR_IMM5(x1, gd, 5);    // r1 = (gd>>5);
                ADD_REG_LSL_IMM5(x3, wback, x1, 2); //(&ed)+=r1*4;
                wback = x3;
                LDR_IMM9(x1, wback, fixedaddress);
                ed = x1;
            }
            AND_IMM8(x2, gd, 0x1f);
            MOV_REG_LSR_REG(x14, ed, x2);
            ANDS_IMM8(x14, x14, 1);
            IFX(X_PEND | X_CF) {
                BFI(xFlags, x14, F_CF, 1);
            }
            B_MARK3(cEQ); // bit already clear, jump to end of instruction
            MOVW(x14, 1);
            XOR_REG_LSL_REG(ed, ed, x14, x2);
            if(wback) {
                STR_IMM9(ed, wback, fixedaddress);
                SMWRITE();
            }
            MARK3;
            break;

        case 0xB6:
            INST_NAME("MOVZX Gd, Eb");
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = (nextop&7);
                eb1 = xEAX+(ed&3);  // Ax, Cx, Dx or Bx
                eb2 = (ed&4)>>2;    // L or H
                UXTB(gd, eb1, eb2);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 4095, 0, 0, NULL);
                LDRB_IMM9(gd, ed, fixedaddress);
            }
            break;
        case 0xB7:
            INST_NAME("MOVZX Gd, Ew");
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xEAX+(nextop&7);
                UXTH(gd, ed, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 255, 0, 0, NULL);
                LDRH_IMM8(gd, ed, fixedaddress);
            }
            break;
        
        case 0xBA:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 4:
                    INST_NAME("BT Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE(x1);
                    gd = x2;
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 4095, 0, 0, NULL);
                        LDR_IMM9(x1, wback, fixedaddress);
                        ed = x1;
                    }
                    u8 = F8;
                    u8&=0x1f;
                    if(u8) {
                        MOV_REG_LSR_IMM5(x1, ed, u8);
                        ed = x1;
                    }
                    BFI(xFlags, ed, F_CF, 1);
                    break;
                case 5:
                    INST_NAME("BTS Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE(x1);
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 4095, 0, 0, NULL);
                        LDR_IMM9(x1, wback, fixedaddress);
                        ed = x1;
                    }
                    u8 = F8;
                    u8&=0x1f;
                    if(u8) {
                        MOV_REG_LSR_IMM5(x14, ed, u8);
                        ANDS_IMM8(x14, x14, 1);
                    } else {
                        ANDS_IMM8(x14, ed, 1);
                    }
                    IFX(X_PEND | X_CF) {
                        BFI(xFlags, x14, F_CF, 1);
                    }
                    B_NEXT(cNE); // bit already set, jump to next instruction
                    MOVW(x14, 1);
                    XOR_REG_LSL_IMM5(ed, ed, x14, u8);
                    if(wback) {
                        STR_IMM9(ed, wback, fixedaddress);
                        SMWRITE();
                    }
                    break;
                case 6:
                    INST_NAME("BTR Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE(x1);
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 4095, 0, 0, NULL);
                        LDR_IMM9(x1, wback, fixedaddress);
                        ed = x1;
                    }
                    u8 = F8;
                    u8&=0x1f;
                    if(u8) {
                        MOV_REG_LSR_IMM5(x14, ed, u8);
                        ANDS_IMM8(x14, x14, 1);
                    } else {
                        ANDS_IMM8(x14, ed, 1);
                    }
                    IFX(X_PEND | X_CF) {
                        BFI(xFlags, x14, F_CF, 1);
                    }
                    B_NEXT(cEQ); // bit already clear, jump to next instruction
                    //MOVW(x14, 1); // already 0x01
                    XOR_REG_LSL_IMM5(ed, ed, x14, u8);
                    if(wback) {
                        STR_IMM9(ed, wback, fixedaddress);
                        SMWRITE();
                    }
                    break;
                case 7:
                    INST_NAME("BTC Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE(x1);
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 4095, 0, 0, NULL);
                        LDR_IMM9(x1, wback, fixedaddress);
                        ed = x1;
                    }
                    u8 = F8;
                    u8&=0x1f;
                    IFX(X_PEND | X_CF) {
                        if(u8) {
                            MOV_REG_LSR_IMM5(x14, ed, u8);
                            ANDS_IMM8(x14, x14, 1);
                        } else {
                            ANDS_IMM8(x14, ed, 1);
                        }
                        BFI(xFlags, x14, F_CF, 1);
                        MOVW_COND(cEQ, x14, 1); // may already 0x01
                    } else {
                        MOVW(x14, 1);
                    }
                    XOR_REG_LSL_IMM5(ed, ed, x14, u8);
                    if(wback) {
                        STR_IMM9(ed, wback, fixedaddress);
                        SMWRITE();
                    }
                    MARK3;
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xBB:
            INST_NAME("BTC Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE(x1);
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xEAX+(nextop&7);
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 4095, 0, 0, NULL);
                MOV_REG_ASR_IMM5(x1, gd, 5);    // r1 = (gd>>5);
                ADD_REG_LSL_IMM5(x3, wback, x1, 2); //(&ed)+=r1*4;
                LDR_IMM9(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            AND_IMM8(x2, gd, 0x1f);
            IFX(X_PEND | X_CF) {
                MOV_REG_LSR_REG(x14, ed, x2);
                BFI(xFlags, x14, F_CF, 1);
            }
            MOVW(x14, 1);
            XOR_REG_LSL_REG(ed, ed, x14, x2);
            if(wback) {
                STR_IMM9(ed, wback, fixedaddress);
                SMWRITE();
            }
            break;
        case 0xBC:
            INST_NAME("BSF Gd, Ed");
            SETFLAGS(X_ZF, SF_SUBSET);
            SET_DFNONE(x1);
            nextop = F8;
            GETED;
            GETGD;
            TSTS_REG_LSL_IMM5(ed, ed, 0);
            IFX(X_PEND | X_ZF) {
                MOVW_COND(cEQ, x1, 1);
            }
            B_MARK(cEQ);
            RBIT(x1, ed);   // reverse
            CLZ(gd, x1);    // x2 gets leading 0 == BSF
            IFX(X_PEND | X_ZF) {
                MOVW(x1, 0);    //ZF not set
            }
            MARK;
            IFX(X_PEND | X_ZF) {
                BFI(xFlags, x1, F_ZF, 1);
            }
            break;
        case 0xBD:
            INST_NAME("BSR Gd, Ed");
            SETFLAGS(X_ZF, SF_SUBSET);
            SET_DFNONE(x1);
            nextop = F8;
            GETED;
            GETGD;
            TSTS_REG_LSL_IMM5(ed, ed, 0);
            IFX(X_PEND | X_ZF) {
                MOVW_COND(cEQ, x1, 1);
            }
            B_MARK(cEQ);
            CLZ(gd, ed);    // x2 gets leading 0
            RSB_IMM8(gd, gd, 31); // complement
            IFX(X_PEND | X_ZF) {
                MOVW(x1, 0);    //ZF not set
            }
            MARK;
            IFX(X_PEND | X_ZF) {
                BFI(xFlags, x1, F_ZF, 1);
            }
            break;
        case 0xBE:
            INST_NAME("MOVSX Gd, Eb");
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = (nextop&7);
                eb1 = xEAX+(ed&3);  // Ax, Cx, Dx or Bx
                eb2 = (ed&4)>>2;    // L or H
                SXTB(gd, eb1, eb2);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 255, 0, 0, NULL);
                LDRSB_IMM8(gd, ed, fixedaddress);
            }
            break;
        case 0xBF:
            INST_NAME("MOVSX Gd, Ew");
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xEAX+(nextop&7);
                SXTH(gd, ed, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 255, 0, 0, NULL);
                LDRSH_IMM8(gd, ed, fixedaddress);
            }
            break;
        case 0xC0:
            INST_NAME("XADD Gb, Eb");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETEB(x2);
            GETGB(x1);
            BFI(gb1, ed, gb2*8, 8); // gb <- eb
            emit_add8(dyn, ninst, ed, gd, x14, x3, 1);
            EBBACK;
            break;
        case 0xC1:
            INST_NAME("XADD Gd, Ed");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGD;
            GETED;
            if(gd!=ed) {
                XOR_REG_LSL_IMM5(gd, gd, ed, 0);    // swap gd, ed
                XOR_REG_LSL_IMM5(ed, gd, ed, 0);
                XOR_REG_LSL_IMM5(gd, gd, ed, 0);
            }
            emit_add32(dyn, ninst, ed, gd, x3, x14);
            WBACK;
            break;
        case 0xC2:
            INST_NAME("CMPPS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0);
            u8 = F8;
            switch(u8&7) {
                // the reversiong of the params in the comparison is there to handle NaN the same way SSE does
                case 0: VCEQQ_F32(v0, v0, v1); break;   // Equal
                case 1: VCGTQ_F32(v0, v1, v0); break;   // Less than
                case 2: VCGEQ_F32(v0, v1, v0); break;   // Less or equal
                case 3: VCEQQ_F32(v0, v0, v0);
                        if(v0!=v1) {
                            q0 = fpu_get_scratch_quad(dyn); 
                            VCEQQ_F32(q0, v1, v1); 
                            VANDQ(v0, v0, q0);
                        }
                        VMVNQ(v0, v0); 
                        break;   // NaN (NaN is not equal to himself)
                case 4: VCEQQ_F32(v0, v0, v1); VMVNQ(v0, v0); break;   // Not Equal (or unordered on ARM, not on X86...)
                case 5: VCGTQ_F32(v0, v1, v0); VMVNQ(v0, v0); break;   // Greater or equal or unordered
                case 6: VCGEQ_F32(v0, v1, v0); VMVNQ(v0, v0); break;   // Greater or unordered
                case 7: VCEQQ_F32(v0, v0, v0); 
                        if(v0!=v1) {
                            q0 = fpu_get_scratch_quad(dyn); 
                            VCEQQ_F32(q0, v1, v1); 
                            VANDQ(v0, v0, q0);
                        }
                        break;   // not NaN
            }
            break;
        case 0xC3:
            INST_NAME("MOVNTI Ed, Gd");
            // ignoring "NTI"
            nextop=F8;
            GETGD;
            if(MODREG) {   // reg <= reg
                MOV_REG(xEAX+(nextop&7), gd);   // doesn't exist
            } else {                    // mem <= reg
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 4095, 0, 0, NULL);
                STR_IMM9(gd, ed, fixedaddress);
            }
            break;
        case 0xC4:
            INST_NAME("PINSRW Gm,Ed,Ib");
            nextop = F8;
            GETGM(d0);
            if(MODREG) {
                u8 = (F8)&3;
                ed = xEAX+(nextop&7);
                VMOVtoDx_16(d0, u8, ed);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, NULL);
                u8 = (F8)&3;
                VLD1LANE_16(d0, wback, u8);
            }
            break;
        case 0xC5:
            INST_NAME("PEXTRW Gd,Em,Ib");
            nextop = F8;
            gd = xEAX+((nextop&0x38)>>3);
            if(MODREG) {
                GETEM(d0);
                u8 = (F8)&3;
                VMOVfrDx_U16(gd, d0, u8);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255-8, 0, 0, NULL);
                u8 = (F8)&3;
                LDRH_IMM8(gd, wback, fixedaddress+u8*2);
            }
            break;
        case 0xC6:
            INST_NAME("SHUFPS Gx, Ex, Ib");
            nextop = F8;
            i32 = -1;
            j32 = 0;
            GETGX(v0, 1);
            v2 = fpu_get_scratch_double(dyn);
            if((nextop&0xC0)!=0xC0) {
                SMREAD();
                // from memory
                addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, 0, 0, 0, NULL);
                u8 = F8;
                q0 = v0;
                // Low part
                switch (u8&0xf) {
                    case 0b0000: VDUP_32(q0, v0, 0);        break;
                    case 0b0001: VREV64_32(q0, v0);         break;  // reverse low part
                    case 0b0010: VREV64_32(v2, v0+1);
                                 VEXT_8(q0, v2, v0, 4);     break;
                    case 0b0011: VEXT_8(q0, v0+1, v0, 4);   break;
                    case 0b0100:                            break; // same
                    case 0b0101: VDUP_32(q0, v0, 1);        break;
                    case 0b0110: VREV64_32(v2, v0+1);
                                 VEXT_8(q0, v2, v0, 4);     break;
                    case 0b0111: VREV64_32(v2, v0);
                                 VEXT_8(q0, v0+1, v2, 4);   break;
                    case 0b1000: VREV64_32(v2, v0);
                                 VEXT_8(q0, v2, v0+1, 4);   break;
                    case 0b1001: VEXT_8(q0, v0, v0+1, 4);   break;
                    case 0b1010: VDUP_32(q0, v0+1, 0);      break;
                    case 0b1011: VREV64_32(q0, v0+1);       break;
                    case 0b1100: VEXT_8(q0, v0+1, v0, 4);
                                 VREV64_32(q0, q0);         break;
                    case 0b1101: VREV64_32(v2, v0+1);
                                 VEXT_8(q0, v0, v2, 4);     break;
                    case 0b1110: VMOVD(q0, v0+1);           break;
                    case 0b1111: VDUP_32(q0, v0+1, 1);      break;
                }
                // High part
                if(((u8>>4)&3) == ((u8>>6)&3)) {
                    if((u8>>4)&3) {
                        ADD_IMM8(x3, wback, ((u8>>4)&3)*4);
                    }
                    VLD1ALL_32(q0+1, ((u8>>4)&3)?x3:wback);
                } else {
                    if((u8>>4)&3) {
                        ADD_IMM8(x3, wback, ((u8>>4)&3)*4);
                    }
                    VLD1LANE_32(q0+1, ((u8>>4)&3)?x3:wback, 0);
                    if((u8>>6)&3) {
                        ADD_IMM8(x3, wback, ((u8>>6)&3)*4);
                    }
                    VLD1LANE_32(q0+1, ((u8>>6)&3)?x3:wback, 1);
                }
            } else {
                // from register
                wback = 0; ed = nextop&7;
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                u8 = F8;
                if((v0==v1) && ((u8==0) || (u8==0b01010101) || (u8==0b10101010) || (u8==0b11111111))) {
                    // same value duplicated all along
                    q0 = v0;
                    VDUPQ_32(v0, v0+((u8>>1)&1), u8&1);
                } else if ((v0==v1) && (u8==0b01000100)) {
                    q0 = v0;
                    VMOVD(v0+1, v0);
                } else if ((v0==v1) && (u8==0b11101110)) {
                    q0 = v0;
                    VMOVD(v0, v0+1);
                } else if ((v0==v1) && (u8==0b10110001)) {
                    q0 = v0;
                    VREV64Q_32(v0, v0);
                } else if ((v0==v1) && (u8==0b01001110)) {
                    q0 = v0;
                    VSWP(v0, v0+1);
                } else if ((v0==v1) && (u8==0b11011000)) {
                    q0 = v0;
                    //VTRN_32(Dd, Dm) swap Dd[1] with Dm[0]
                    VTRN_32(v0, v0+1);
                } else if ((v0==v1) && (u8==0b00100111)) {
                    q0 = v0;
                    //VTRN_32(Dd, Dm) swap Dd[1] with Dm[0]
                    VTRN_32(v0+1, v0);
                } else if ((v0==v1) && (u8==0b01110010)) {
                    q0 = v0;
                    //VTRN_32(Dd, Dm) swap Dd[1] with Dm[0]
                    VTRN_32(v0, v0+1);
                    VREV64Q_32(v0, v0);
                } else if ((v0==v1) && (u8==0b10001101)) {
                    q0 = v0;
                    //VTRN_32(Dd, Dm) swap Dd[1] with Dm[0]
                    VTRN_32(v0+1, v0);
                    VREV64Q_32(v0, v0);
                } else {
                    if(v1!=v0)
                        q0 = v0;    // no need for temp storage
                    else if((u8&0b10100000)==0b10100000)
                        q0 = v0;    // no need for temp if only low/low and high/high are shuffled
                    else
                        q0 = fpu_get_scratch_quad(dyn); // temporary storage
                    // Low part
                    switch (u8&0xf) {
                        case 0b0000: VDUP_32(q0, v0, 0);        break;
                        case 0b0001: VREV64_32(q0, v0);         break;  // reverse low part
                        case 0b0010: VREV64_32(v2, v0+1);
                                     VEXT_8(q0, v2, v0, 4);     break;
                        case 0b0011: VEXT_8(q0, v0+1, v0, 4);   break;
                        case 0b0100: VMOVD(q0, v0);             break; // same
                        case 0b0101: VDUP_32(q0, v0, 1);        break;
                        case 0b0110: VEXT_8(q0, v0, v0+1, 4);
                                     VREV64_32(q0, q0);         break;
                        case 0b0111: VREV64_32(v2, v0);
                                     VEXT_8(q0, v0+1, v2, 4);   break;
                        case 0b1000: VREV64_32(v2, v0);
                                     VEXT_8(q0, v2, v0+1, 4);   break;
                        case 0b1001: VEXT_8(q0, v0, v0+1, 4);   break;
                        case 0b1010: VDUP_32(q0, v0+1, 0);      break;
                        case 0b1011: VREV64_32(q0, v0+1);       break;
                        case 0b1100: VEXT_8(q0, v0+1, v0, 4);
                                     VREV64_32(q0, q0);         break;
                        case 0b1101: VREV64_32(v2, v0+1);
                                     VEXT_8(q0, v0, v2, 4);     break;
                        case 0b1110: VMOVD(q0, v0+1);           break;
                        case 0b1111: VDUP_32(q0, v0+1, 1);      break;
                    }
                    // High part
                    switch ((u8>>4)&0xf) {
                        case 0b0000: VDUP_32(q0+1, v1, 0);        break;
                        case 0b0001: VREV64_32(q0+1, v1);         break;  // reverse low part
                        case 0b0010: VREV64_32(v2, v1+1);
                                     VEXT_8(q0+1, v2, v1, 4);     break;
                        case 0b0011: VEXT_8(q0+1, v1+1, v1, 4);   break;
                        case 0b0100: VMOVD(q0+1, v1);             break; // same
                        case 0b0101: VDUP_32(q0+1, v1, 1);        break;
                        case 0b0110: VEXT_8(q0+1, v1, v1+1, 4);
                                     VREV64_32(q0+1, q0+1);       break;
                        case 0b0111: VREV64_32(v2, v1);
                                     VEXT_8(q0+1, v1+1, v2, 4);   break;
                        case 0b1000: VREV64_32(v2, v1);
                                     VEXT_8(q0+1, v2, v1+1, 4);   break;
                        case 0b1001: VEXT_8(q0+1, v1, v1+1, 4);   break;
                        case 0b1010: VDUP_32(q0+1, v1+1, 0);      break;
                        case 0b1011: VREV64_32(q0+1, v1+1);       break;
                        case 0b1100: VEXT_8(q0+1, v1+1, v1, 4);
                                     VREV64_32(q0+1, q0+1);       break;
                        case 0b1101: VREV64_32(v2, v1+1);
                                     VEXT_8(q0+1, v1, v2, 4);     break;
                        case 0b1110: VMOVD(q0+1, v1+1);           break;
                        case 0b1111: VDUP_32(q0+1, v1+1, 1);      break;
                    }
                }
            }
            VMOVQ(v0, q0);
            break;
        case 0xC7:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 1:
                    INST_NAME("CMPXCHG8B Gq, Eq");
                    SETFLAGS(X_ZF, SF_SUBSET);
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 4095-4, 0, 0, NULL);
                    LDR_IMM9(x1, wback, fixedaddress+0);
                    LDR_IMM9(x2, wback, fixedaddress+4);
                    CMPS_REG_LSL_IMM5(xEAX, x1, 0);
                    B_MARK(cNE);    // EAX != Ed[0]
                    CMPS_REG_LSL_IMM5(xEDX, x2, 0);
                    B_MARK(cNE);    // EDX != Ed[1]
                    STR_IMM9(xEBX, wback, fixedaddress+0);
                    STR_IMM9(xECX, wback, fixedaddress+4);
                    MOVW(x1, 1);
                    B_MARK3(c__);
                    MARK;
                    MOV_REG(xEAX, x1);
                    MOV_REG(xEDX, x2);
                    MOVW(x1, 0);
                    MARK3;
                    BFI(xFlags, x1, F_ZF, 1);
                    SMWRITE();
                    break;
                default:
                    DEFAULT;
            }
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

        case 0xD1:
            INST_NAME("PSRLW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            q0 = fpu_get_scratch_quad(dyn);
            VMOVD(q0, d1);
            VMOVD(q0+1, d1);
            VQMOVN_S64(q0, q0); // 2*d1 in 32bits now
            VMOVD(q0+1, q0);
            VQMOVN_S32(q0, q0); // 4*d1 in 16bits now
            VNEGN_16(q0, q0);   // because we want SHR and not SHL
            VSHL_U16(d0, d0, q0);
            break;
        case 0xD2:
            INST_NAME("PSRLD Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            q0 = fpu_get_scratch_quad(dyn);
            VMOVD(q0, d1);
            VMOVD(q0+1, d1);
            VQMOVN_S64(q0, q0); // 2*d1 in 32bits now
            VNEGN_32(q0, q0);   // because we want SHR and not SHL
            VSHL_U32(d0, d0, q0);
            break;
        case 0xD3:
            INST_NAME("PSRLQ Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            q0 = fpu_get_scratch_quad(dyn);
            VMOVD(q0, d1);
            VEOR(q0+1, q0+1, q0+1);
            VQMOVN_S64(q0, q0); // d1 is 32bits now
            VNEGN_32(q0, q0);   // because we want SHR and not SHL, and there is no neg in S64
            VSHL_U64(d0, d0, q0);
            break;
        case 0xD4:
            INST_NAME("PADDQ Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1);
            VADD_64(v0, v0, v1);
            break;
        case 0xD5:
            INST_NAME("PMULLW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VMUL_16(d0, d0, d1);
            break;

        case 0xD7:
            nextop = F8;
            if(MODREG) {
                INST_NAME("PMOVMSKB Gd, Em");
                GETEM(d0);
                gd = xEAX+((nextop&0x38)>>3);
                #if STEP == 3
                MOV32_(x1, &mask_shift8);
                #else
                MOV32_(x1, 0);
                #endif
                d1 = fpu_get_scratch_double(dyn);
                VLD1_8(d1, x1);     // load shift
                v2 = fpu_get_scratch_double(dyn);
                VMOV_8(v2, 0x80);   // load mask
                VANDD(v2, v2, d0);  // keep highest bit
                VSHL_U8(v2, v2, d1);// shift
                VPADD_8(v2, v2, v2);// accumulate the bits
                VPADD_8(v2, v2, v2);// ...
                VPADD_8(v2, v2, v2);// ...
                VMOVfrDx_U8(gd, v2, 0);
            } else {
                DEFAULT;
            }
            break;
        case 0xD8:
            INST_NAME("PSUBUSB Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VQSUB_U8(d0, d0, d1);
            break;
        case 0xD9:
            INST_NAME("PSUBUSW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VQSUB_U16(d0, d0, d1);
            break;
        case 0xDA:
            INST_NAME("PMINUB Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VMIN_U8(d0, d0, d1);
            break;
        case 0xDB:
            INST_NAME("PAND Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1);
            VANDD(v0, v0, v1);
            break;
        case 0xDC:
            INST_NAME("PADDUSB Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VQADD_U8(d0, d0, d1);
            break;
        case 0xDD:
            INST_NAME("PADDUSW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VQADD_U16(d0, d0, d1);
            break;
        case 0xDE:
            INST_NAME("PMAXUB Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VMAX_U8(d0, d0, d1);
            break;
         case 0xDF:
            INST_NAME("PANDN Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1);
            VBICD(v0, v1, v0);
            break;
         case 0xE0:
            INST_NAME("PAVGB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1);
            VRHADD_U8(v0, v0, v1);
            break;
        case 0xE1:
            INST_NAME("PSRAW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            v0 = fpu_get_scratch_quad(dyn);
            VMOVD(v0, d1);
            VMOVD(v0+1, d1);
            VQMOVN_S64(v0, v0); // 2*d1 in 32bits now
            VMOVD(v0+1, v0);
            VQMOVN_S32(v0, v0); // 4*d1 in 16bits now
            VNEGN_16(v0, v0);   // because we want SHR and not SHL
            VSHL_S16(d0, d0, v0);
            break;
        case 0xE2:
            INST_NAME("PSRAD Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            v0 = fpu_get_scratch_quad(dyn);
            VMOVD(v0, d1);
            VMOVD(v0+1, d1);
            VQMOVN_S64(v0, v0); // 2*q1 in 32bits now
            VNEGN_32(v0, v0);   // because we want SHR and not SHL
            VSHL_S32(d0, d0, v0);
            break;
        case 0xE3:
            INST_NAME("PAVGW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VRHADD_U16(d0, d0, d1);
            break;
       case 0xE4:
            INST_NAME("PMULHUW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1);
            q0 = fpu_get_scratch_quad(dyn);
            VMULL_U32_U16(q0, v0, v1);
            VUZP_16(q0, q0+1);
            VMOVD(v0, q0+1);
            break;
        case 0xE5:
            INST_NAME("PMULHW Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1);
            q0 = fpu_get_scratch_quad(dyn);
            VMULL_S32_S16(q0, v0, v1);
            VUZP_16(q0, q0+1);
            VMOVD(v0, q0+1);
            break;

        case 0xE7:
            INST_NAME("MOVNTQ Em, Gm");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if(MODREG) {
                DEFAULT;
            } else {
                v0 = mmx_get_reg(dyn, ninst, x1, x2, x3, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                VST1_64(v0, ed);
            }
            break;
        case 0xE8:
            INST_NAME("PSUBSB Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VQSUB_S8(d0, d0, d1);
            break;
        case 0xE9:
            INST_NAME("PSUBSW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VQSUB_S16(d0, d0, d1);
            break;
        case 0xEA:
            INST_NAME("PMINPS Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VMIN_S16(d0, d0, d1);
            break;
        case 0xEB:
            INST_NAME("POR Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1);
            VORRD(v0, v0, v1);
            break;
        case 0xEC:
            INST_NAME("PADDSB Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VQADD_S8(d0, d0, d1);
            break;
        case 0xED:
            INST_NAME("PADDSW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VQADD_S16(d0, d0, d1);
            break;
        case 0xEE:
            INST_NAME("PMAXSW Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VMAX_S16(d0, d0, d1);
            break;
        case 0xEF:
            INST_NAME("PXOR Gm, Em");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if((nextop&0xC7)==(0xC0|gd)) {
                v1 = v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
            } else {
                GETGM(v0);
                GETEM(v1);
            }
            VEOR(v0, v0, v1);
            break;

        case 0xF1:
            INST_NAME("PSLLW Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            q0 = fpu_get_scratch_quad(dyn);
            VMOVD(q0, d1);
            VMOVD(q0+1, d1);
            VQMOVN_S64(q0, q0); // 2*d1 in 32bits now
            VMOVD(q0+1, q0);
            VQMOVN_S32(q0, q0); // 4*d1 in 16bits now
            VSHL_U16(d0, d0, q0);
            break;
        case 0xF2:
            INST_NAME("PSLLD Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            q0 = fpu_get_scratch_quad(dyn);
            VMOVD(q0, d1);
            VMOVD(q0+1, d1);
            VQMOVN_S64(q0, q0); // 2*d1 in 32bits now
            VSHL_U32(d0, d0, q0);
            break;
        case 0xF3:
            INST_NAME("PSLLQ Gm,Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VSHL_U64(d0, d0, d1);   // no need to transform here
            break;
        case 0xF4:
            INST_NAME("PMULUDQ Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1);
            q0 = fpu_get_scratch_quad(dyn);
            VMULL_U64_U32(q0, v0, v1);
            VMOVD(v0, q0);
            break;
        case 0xF5:
            INST_NAME("PMADDWD Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            q0 = fpu_get_scratch_quad(dyn);
            VMULL_S32_S16(q0, d0, d1);
            VTRN_32(q0, q0+1);
            VADD_32(d0, q0, q0+1);
            break;
        case 0xF6:
            INST_NAME("PSADBW Gm, Em");
            nextop = F8;
            GETGM(d0);
            GETEM(d1);
            VABD_U8(d0, d0, d1);
            VPADDL_U8(d0, d0);
            VPADDL_U16(d0, d0);
            VPADDL_U32(d0, d0);
            break;

        case 0xF8:
            INST_NAME("PSUBB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1);
            VSUB_8(v0, v0, v1);
            break;
        case 0xF9:
            INST_NAME("PSUBW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1);
            VSUB_16(v0, v0, v1);
            break;
        case 0xFA:
            INST_NAME("PSUBD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1);
            VSUB_32(v0, v0, v1);
            break;
        case 0xFB:
            INST_NAME("PSUBQ Gm,Em");
            nextop = F8;
            GETGM(v0);
            GETEM(q0);
            VSUB_64(v0, v0, q0);
            break;
        case 0xFC:
            INST_NAME("PADDB Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1);
            VADD_8(v0, v0, v1);
            break;
        case 0xFD:
            INST_NAME("PADDW Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1);
            VADD_16(v0, v0, v1);
            break;
        case 0xFE:
            INST_NAME("PADDD Gm, Em");
            nextop = F8;
            GETGM(v0);
            GETEM(v1);
            VADD_32(v0, v0, v1);
            break;

        default:
            DEFAULT;
    }
    return addr;
}

