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
#include "emu/x86compstrings.h"

// Get EX as a quad
#define GETEX(a, w)             \
    if(MODREG) {   \
        a = sse_get_reg(dyn, ninst, x1, nextop&7, w);  \
    } else {                    \
        SMREAD();               \
        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL); \
        a = fpu_get_scratch_quad(dyn); \
        VLD1Q_8(a, ed);         \
    }
#define GETGX(a, w)         \
    gd = (nextop&0x38)>>3;  \
    a = sse_get_reg(dyn, ninst, x1, gd, w)
#define GETGX_empty(a)      \
    gd = (nextop&0x38)>>3;  \
    a = sse_get_reg_empty(dyn, ninst, x1, gd)

#define GETG    gd = (nextop&0x38)>>3

uintptr_t dynarec660F(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop, u8;
    int32_t i32, j32;
    uint32_t u32;
    uint8_t gd, ed;
    uint8_t wback, wb1;
    uint8_t eb1, eb2;
    int v0, v1;
    int q0, q1;
    int d0;
    int s0;
    int fixedaddress;
    int parity;

    MAYUSE(d0);
    MAYUSE(q1);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(j32);
    #if STEP == 3
    static const int8_t mask_shift8[] = { -7, -6, -5, -4, -3, -2, -1, 0 };
    #endif

    switch(opcode) {

        case 0x10:
            INST_NAME("MOVUPD Gx,Ex");
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
                LDR_IMM9(x2, ed, fixedaddress+0);
                LDR_IMM9(x3, ed, fixedaddress+4);
                VMOVtoV_D(v0, x2, x3);
                LDR_IMM9(x2, ed, fixedaddress+8);
                LDR_IMM9(x3, ed, fixedaddress+12);
                VMOVtoV_D(v0+1, x2, x3);
            }
            break;
        case 0x11:
            INST_NAME("MOVUPD Ex,Gx");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, nextop&7);
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 255-16, 0, 0, NULL);
                VMOVfrV_D(x2, x3, v0);
                STR_IMM9(x2, ed, fixedaddress+0);
                STR_IMM9(x3, ed, fixedaddress+4);
                VMOVfrV_D(x2, x3, v0+1);
                STR_IMM9(x2, ed, fixedaddress+8);
                STR_IMM9(x3, ed, fixedaddress+12);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("MOVLPD Gx, Eq");
            nextop = F8;
            GETGX(v0, 1);
            if(MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            parity = getedparity(dyn, ninst, addr, nextop, 3);
            SMREAD();
            if(parity) {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL);
                VLDR_64(v0, ed, fixedaddress);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-8, 0, 0, NULL);
                LDR_IMM9(x2, ed, fixedaddress);
                LDR_IMM9(x3, ed, fixedaddress+4);
                VMOVtoV_D(v0, x2, x3);
            }
            break;
        case 0x13:
            INST_NAME("MOVLPD Eq, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            parity = getedparity(dyn, ninst, addr, nextop, 3);
            if(parity) {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL);
                VSTR_64(v0, ed, fixedaddress);
            } else {
                VMOVfrV_D(x2, x3, v0);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-8, 0, 0, NULL);
                STR_IMM9(x2, ed, fixedaddress);
                STR_IMM9(x3, ed, fixedaddress+4);
            }
            SMWRITE2();
            break;
        case 0x14:
            INST_NAME("UNPCKLPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
                VMOVD(v0+1, v1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL);
                VLDR_64(v0+1, ed, fixedaddress);
            }
            break;
        case 0x15:
            INST_NAME("UNPCKHPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            VMOVD(v0, v0+1);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
                VMOVD(v0+1, v1+1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023-4, 3, 0, NULL);
                VLDR_64(v0+1, ed, fixedaddress+4);
            }
            break;
        case 0x16:
            INST_NAME("MOVHPD Gx, Ed");
            nextop = F8;
            GETGX(v0, 1);
            if(MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            parity = getedparity(dyn, ninst, addr, nextop, 3);
            SMREAD();
            if(parity) {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL);
                VLDR_64(v0+1, ed, fixedaddress);    // vfpu opcode here
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-8, 0, 0, NULL);
                LDR_IMM9(x2, ed, fixedaddress);
                LDR_IMM9(x3, ed, fixedaddress+4);
                VMOVtoV_D(v0+1,x2, x3);
            }
            break;
        case 0x17:
            INST_NAME("MOVHPD Ed, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                // access register instead of memory is bad opcode!
                DEFAULT;
                return addr;
            }
            parity = getedparity(dyn, ninst, addr, nextop, 3);
            if(parity) {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL);
                VSTR_64(v0+1, ed, fixedaddress);
            } else {
                VMOVfrV_D(x2, x3, v0+1);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-8, 0, 0, NULL);
                STR_IMM9(x2, ed, fixedaddress);
                STR_IMM9(x3, ed, fixedaddress+4);
            }
            SMWRITE2();
            break;

        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;
        
        case 0x28:
            INST_NAME("MOVAPD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                VLD1Q_64(v0, ed);
                SMWRITE2();
            }
            break;
        case 0x29:
            INST_NAME("MOVAPD Ex, Gx");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, nextop&7);
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                VST1Q_64(v0, ed);
                SMWRITE2();
            }
            break;


        case 0x2B:
            INST_NAME("MOVNTPD Ex, Gx");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, nextop&7);
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                VST1Q_64(v0, ed);
                SMWRITE2();
            }
            break;

        case 0x2E:
            // no special check...
        case 0x2F:
            if(opcode==0x2F) {INST_NAME("COMISD Gx, Ex");} else {INST_NAME("UCOMISD Gx, Ex");}
            SETFLAGS(X_ALL, SF_SET_NODF);
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            GETEX(q0, 0);
            VCMP_F64(v0, q0);
            FCOMI(x1, x2);
            break;

        case 0x38:  // SSSE3 opcodes
            nextop = F8;
            switch(nextop) {
                case 0x00:
                    INST_NAME("PSHUFB Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    q1 = fpu_get_scratch_quad(dyn);
                    if(MODREG) {
                        v0 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        VLD1Q_64(q1, ed);
                        v0 = q1;
                    }
                    v1 = fpu_get_scratch_quad(dyn);
                    VMOVQ_8(v1, 0b10001111);
                    VANDQ(q1, v0, v1);  // mask the index
                    VMOVQ(v1, q0);
                    VTBL2_8(q0+0, v1, q1+0);
                    VTBL2_8(q0+1, v1, q1+1);
                    break;
                case 0x01:
                    INST_NAME("PHADDW Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0);
                    VPADD_16(q0, q0, q0+1);
                    if(q0==q1) {
                        VMOVD(q0+1, q0);
                    } else {
                        VPADD_16(q0+1, q1, q1+1);
                    }
                    break;
                case 0x02:
                    INST_NAME("PHADDD Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0);
                    VPADD_32(q0, q0, q0+1);
                    if(q0==q1) {
                        VMOVD(q0+1, q0);
                    } else {
                        VPADD_32(q0+1, q1, q1+1);
                    }
                    break;
                case 0x03:
                    INST_NAME("PHADDSW Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0);
                    if((nextop&0xC0) == 0xC0) {
                        v1 = fpu_get_scratch_quad(dyn);
                        VMOVQ(v1, q1);
                    } else
                        v1 = q1;
                    VUZPQ_16(q0, v1);
                    VQADDQ_S16(q0, q0, v1);
                    break;
                case 0x04:
                    INST_NAME("PMADDUBSW Gx,Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0);
                    v0 = fpu_get_scratch_quad(dyn);
                    v1 = fpu_get_scratch_quad(dyn);
                    VMOVL_U8(v0, q0+0);   // this is unsigned, so 0 extended
                    VMOVL_S8(v1, q1+0);   // this is signed
                    VMULQ_16(v0, v0, v1);
                    VPADDLQ_S16(v0, v0);
                    VQMOVN_S32(q0+0, v0);
                    VMOVL_U8(v0, q0+1);   // this is unsigned
                    VMOVL_S8(v1, q1+1);   // this is signed
                    VMULQ_16(v0, v0, v1);
                    VPADDLQ_S16(v0, v0);
                    VQMOVN_S32(q0+1, v0);
                    break;
                case 0x05:
                    INST_NAME("PHSUBW Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0);
                    if((nextop&0xC0) == 0xC0) {
                        v1 = fpu_get_scratch_quad(dyn);
                        VMOVQ(v1, q1);
                    } else
                        v1 = q1;
                    VUZPQ_16(q0, v1);
                    VSUBQ_16(q0, q0, v1);
                    break;
                case 0x06:
                    INST_NAME("PHSUBD Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0);
                    if ((nextop & 0xC0) == 0xC0) {
                        v1 = fpu_get_scratch_quad(dyn);
                        VMOVQ(v1, q1);
                    } else
                        v1 = q1;
                    VUZPQ_32(q0, v1);
                    VSUBQ_32(q0, q0, v1);
                    break;
                case 0x08:
                    INST_NAME("PSIGNB Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0);
                    v1 = fpu_get_scratch_quad(dyn);
                    v0 = fpu_get_scratch_quad(dyn);
                    VNEGNQ_8(v0, q0);  // get NEG
                    VCLTQ_0_8(v1, q1); // calculate mask
                    VBICQ(q0, q0, v1);  // apply not mask on dest
                    VANDQ(v0, v0, v1);  // apply mask on src
                    VORRQ(q0, q0, v0);  // merge
                    VCEQQ_0_8(v1, q1); // handle case where Ex is 0
                    VBICQ(q0, q0, v1);
                    break;
                case 0x09:
                    INST_NAME("PSIGNW Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0);
                    v1 = fpu_get_scratch_quad(dyn);
                    v0 = fpu_get_scratch_quad(dyn);
                    VNEGNQ_16(v0, q0);  // get NEG
                    VCLTQ_0_16(v1, q1); // calculate mask
                    VBICQ(q0, q0, v1);  // apply not mask on dest
                    VANDQ(v0, v0, v1);  // apply mask on src
                    VORRQ(q0, q0, v0);  // merge
                    VCEQQ_0_16(v1, q1); // handle case where Ex is 0
                    VBICQ(q0, q0, v1);
                    break;
                case 0x0A:
                    INST_NAME("PSIGND Gx, Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0);
                    v1 = fpu_get_scratch_quad(dyn);
                    v0 = fpu_get_scratch_quad(dyn);
                    VNEGNQ_32(v0, q0);  // get NEG
                    VCLTQ_0_32(v1, q1); // calculate mask
                    VBICQ(q0, q0, v1);  // apply not mask on dest
                    VANDQ(v0, v0, v1);  // apply mask on src
                    VORRQ(q0, q0, v0);  // merge
                    VCEQQ_0_32(v1, q1); // handle case where Ex is 0
                    VBICQ(q0, q0, v1);
                    break;

                case 0x0B:
                    INST_NAME("PMULHRSW Gx,Ex");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0);
                    VQRDMULHQ_S16(q0, q0, q1);
                    break;

                case 0x1C:
                    INST_NAME("PABSB Gx,Ex");
                    nextop = F8;
                    GETEX(q1, 0);
                    gd = (nextop&0x38)>>3;
                    q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                    VABSQ_S8(q0, q1);
                    break;
                case 0x1D:
                    INST_NAME("PABSW Gx,Ex");
                    nextop = F8;
                    GETEX(q1, 0);
                    gd = (nextop&0x38)>>3;
                    q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                    VABSQ_S16(q0, q1);
                    break;
                case 0x1E:
                    INST_NAME("PABSD Gx,Ex");
                    nextop = F8;
                    GETEX(q1, 0);
                    gd = (nextop&0x38)>>3;
                    q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                    VABSQ_S32(q0, q1);
                    break;

                case 0x2B:
                    INST_NAME("PACKUSDW Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX(q1, 0);
                    GETGX(q0, 1);
                    v0 = fpu_get_scratch_quad(dyn);
                    v1 = fpu_get_scratch_quad(dyn);
                    VEORQ(v0, v0, v0);
                    VMAXQ_S32(v1, v0, q0);    // values < 0 => 0
                    VQMOVN_U32(q0, v1);
                    if(q0==q1) {
                        VMOV_64(q0+1, q0);
                    } else {
                        VMAXQ_S32(v0, v0, q1);
                        VQMOVN_U32(q0+1, v0);
                    }
                    break;

        		case 0x38:
                    INST_NAME("PMINSB Gx,Ex");
                    nextop = F8;
                    GETEX(q1, 0);
                    gd = (nextop&0x38)>>3;
                    q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                    VMINQ_S8(q0, q0, q1);
                    break;
        		case 0x39:
                    INST_NAME("PMINSD Gx,Ex");
                    nextop = F8;
                    GETEX(q1, 0);
                    gd = (nextop&0x38)>>3;
                    q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                    VMINQ_S32(q0, q0, q1);
                    break;
        		case 0x3A:
                    INST_NAME("PMINUW Gx,Ex");
                    nextop = F8;
                    GETEX(q1, 0);
                    gd = (nextop&0x38)>>3;
                    q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                    VMINQ_U16(q0, q0, q1);
                    break;
        		case 0x3B:
                    INST_NAME("PMINUD Gx,Ex");
                    nextop = F8;
                    GETEX(q1, 0);
                    gd = (nextop&0x38)>>3;
                    q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                    VMINQ_U32(q0, q0, q1);
                    break;
        		case 0x3C:
                    INST_NAME("PMAXSB Gx,Ex");
                    nextop = F8;
                    GETEX(q1, 0);
                    gd = (nextop&0x38)>>3;
                    q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                    VMAXQ_S8(q0, q0, q1);
                    break;
        		case 0x3D:
                    INST_NAME("PMAXSD Gx,Ex");
                    nextop = F8;
                    GETEX(q1, 0);
                    gd = (nextop&0x38)>>3;
                    q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                    VMAXQ_S32(q0, q0, q1);
                    break;
        		case 0x3E:
                    INST_NAME("PMAXUW Gx,Ex");
                    nextop = F8;
                    GETEX(q1, 0);
                    gd = (nextop&0x38)>>3;
                    q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                    VMAXQ_U16(q0, q0, q1);
                    break;
        		case 0x3F:
                    INST_NAME("PMAXUD Gx,Ex");
                    nextop = F8;
                    GETEX(q1, 0);
                    gd = (nextop&0x38)>>3;
                    q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                    VMINQ_U32(q0, q0, q1);
                    break;
                case 0x40:
                    INST_NAME("PMULLD Gx, Ex");  // SSE4 opcode!
                    nextop = F8;
                    GETEX(q1, 0);
                    GETGX(q0, 1);
                    VMULQ_32(q0, q0, q1);
                    break;

                case 0xDB:
                    INST_NAME("AESIMC Gx, Ex");  // AES-NI
                    nextop = F8;
                    if(arm_aes) {
                        GETEX(q1, 0);
                        GETGX_empty(q0);
                        AESIMC(q0, q1);
                    } else {
                        GETEX(q1, 0);
                        GETGX_empty(q0);
                        if(q0!=q1) {
                            VMOVQ(q0, q1);
                        }
                        sse_forget_reg(dyn, ninst, gd, x1);
                        MOV32(x1, gd);
                        CALL(arm_aesimc, -1, 0);
                    }
                    break;
                case 0xDC:
                    INST_NAME("AESENC Gx, Ex");  // AES-NI
                    nextop = F8;
                    if(arm_aes) {
                        GETEX(q1, 0);
                        GETGX(q0, 1);
                        v0 = fpu_get_scratch_quad(dyn);  // ARM64 internal operation differs a bit from x86_64
                        VEORQ(v0, q0, q1);
                        AESE(v0, q1);
                        AESMC(v0, v0);
                        VEORQ(q0, v0, q1);
                    } else {
                        GETG;
                        sse_forget_reg(dyn, ninst, gd, x1);
                        MOV32(x1, gd);
                        CALL(arm_aese, -1, 0);
                        GETGX(q0, 1);
                        GETEX(q1, 0);
                        VEORQ(q0, q0, q1);
                    }
                    break;
                case 0xDD:
                    INST_NAME("AESENCLAST Gx, Ex");  // AES-NI
                    nextop = F8;
                    if(arm_aes) {
                        GETEX(q1, 0);
                        GETGX(q0, 1);
                        v0 = fpu_get_scratch_quad(dyn);  // ARM64 internal operation differs a bit from x86_64
                        VEORQ(v0, q0, q1);
                        AESE(v0, q1);
                        VEORQ(q0, v0, q1);
                    } else {
                        GETG;
                        sse_forget_reg(dyn, ninst, gd, x1);
                        MOV32(x1, gd);
                        CALL(arm_aeselast, -1, 0);
                        GETGX(q0, 1);
                        GETEX(q1, 0);
                        VEORQ(q0, q0, q1);
                    }
                    break;
                case 0xDE:
                    INST_NAME("AESDEC Gx, Ex");  // AES-NI
                    nextop = F8;
                    if(arm_aes) {
                        GETEX(q1, 0);
                        GETGX(q0, 1);
                        v0 = fpu_get_scratch_quad(dyn);  // ARM64 internal operation differs a bit from x86_64
                        VEORQ(v0, q0, q1);
                        AESD(v0, q1);
                        AESIMC(v0, v0);
                        VEORQ(q0, v0, q1);
                    } else {
                        GETG;
                        sse_forget_reg(dyn, ninst, gd, x1);
                        MOV32(x1, gd);
                        CALL(arm_aesd, -1, 0);
                        GETGX(q0, 1);
                        GETEX(q1, 0);
                        VEORQ(q0, q0, q1);
                    }
                    break;
                case 0xDF:
                    INST_NAME("AESDECLAST Gx, Ex");  // AES-NI
                    nextop = F8;
                    if(arm_aes) {
                        GETEX(q1, 0);
                        GETGX(q0, 1);
                        v0 = fpu_get_scratch_quad(dyn);  // ARM64 internal operation differs a bit from x86_64
                        VEORQ(v0, q0, q1);
                        AESD(v0, q1);
                        VEORQ(q0, v0, q1);
                    } else {
                        GETG;
                        sse_forget_reg(dyn, ninst, gd, x1);
                        MOV32(x1, gd);
                        CALL(arm_aesdlast, -1, 0);
                        GETGX(q0, 1);
                        GETEX(q1, 0);
                        VEORQ(q0, q0, q1);
                    }
                    break;

                default:
                    DEFAULT;
            }
            break;

        case 0x3A:  // these are some more SSSE3 opcodes
            opcode = F8;
            switch(opcode) {
                case 0x0E:
                    INST_NAME("PBLENDW Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0);
                    v0 = fpu_get_scratch_double(dyn);
                    u8 = F8;
                    i32 = 0;
                    for(int i=0; i<4; ++i)
                        if((u8&(1<<i)))
                            i32|=(0b11<<(i*2));
                    if(i32 && (q0!=q1)) {
                        VMOV_D64(v0, i32);
                        VBICD(q0, q0, v0);
                        VANDD(v0, q1, v0);
                        VORRD(q0, q0, v0);
                    }
                    i32 = 0;
                    for(int i=0; i<4; ++i)
                        if((u8&(1<<(i+4))))
                            i32|=(0b11<<(i*2));
                    if(i32 && (q0!=q1)) {
                        VMOV_D64(v0, i32);
                        VBICD(q0+1, q0+1, v0);
                        VANDD(v0, q1+1, v0);
                        VORRD(q0+1, q0+1, v0);
                    }
                    break;
                case 0x0F:
                    INST_NAME("PALIGNR Gx, Ex, Ib");
                    nextop = F8;
                    GETGX(q0, 1);
                    GETEX(q1, 0);
                    u8 = F8;
                    if(u8>31) {
                        VEORQ(q0, q0, q0);    
                    } else if(u8>15) {
                        d0 = fpu_get_scratch_quad(dyn);
                        VEORQ(d0, d0, d0);
                        VEXTQ_8(q0, q0, d0, u8-16);
                    } else {
                        VEXTQ_8(q0, q1, q0, u8);
                    }
                    break;

                case 0x16:
                    INST_NAME("PEXTRD Ed, Gx, Ib");
                    nextop = F8;
                    GETGX(q0, 0);
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                        u8 = F8;
                        VMOVfrDx_32(ed, q0+((u8&2)>>1), u8&1);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, NULL);
                        u8 = F8;
                        VST1LANE_32(q0+((u8&2)>>1), wback, u8&1);
                        SMWRITE2();
                    }
                    break;

                case 0x20:
                    INST_NAME("PINSRB Gx, ED, Ib");
                    nextop = F8;
                    GETGX(q0, 1);
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                        u8 = F8;
                        VMOVtoDx_8(q0+((u8&8)>>3), u8&7, ed);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, NULL);
                        u8 = F8;
                        VLD1LANE_8(q0+((u8&8)>>3), wback, u8&7);
                        SMWRITE2();
                    }
                    break;

                case 0x22:
                    INST_NAME("PINSRD Gx, ED, Ib");
                    nextop = F8;
                    GETGX(q0, 1);
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                        u8 = F8;
                        VMOVtoDx_32(q0+((u8&2)>>1), u8&1, ed);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x2, &fixedaddress, 0, 0, 0, NULL);
                        u8 = F8;
                        VLD1LANE_32(q0+((u8&2)>>1), wback, u8&1);
                        SMWRITE2();
                    }
                    break;

                case 0x44:
                    INST_NAME("PCLMULQDQ Gx, Ex, Ib");
                    nextop = F8;
                    if(arm_pmull) {
                        GETGX(q0, 1);
                        GETEX(q1, 0);
                        u8 = F8;
                        switch (u8&0b00010001) {
                            case 0b00000000:
                                VMULL_P64(q0, q0, q1);
                                break;
                            case 0b00010001:
                                VMULL_P64(q0, q0+1, q1+1);
                                break;
                            case 0b00000001:
                                VMULL_P64(q0, q0+1, q1);
                                break;
                            case 0b00010000:
                                VMULL_P64(q0, q0, q1+1);
                                break;
                        }
                    } else {
                        GETG;
                        sse_forget_reg(dyn, ninst, gd, x1);
                        if(MODREG) {
                            ed = nextop&7;
                            sse_forget_reg(dyn, ninst, ed, x1);
                            MOV32(x2, ed);
                        } else {
                            MOV32(x2, 0);
                            addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0, 0, NULL);
                            if(ed!=x2) {
                                MOV_REG(x2, ed);
                            }
                        }
                        MOV32(x1, gd); // gx
                        u8 = F8;
                        MOV32(x3, u8);
                        CALL(arm_pclmul, -1, 0);
                    }
                    break;

                case 0x63:
                    INST_NAME("PCMPISTRI Gx, Ex, Ib");
                    SETFLAGS(X_ALL, SF_SET_DF);
                    nextop = F8;
                    GETG;
                    if(!sse_reflect_reg(dyn, ninst, gd, x2)) {
                        u32 = offsetof(x86emu_t, xmm[gd]);
                        if(!(u32&3) && (u32>>2)<256) {
                            ADD_IMM8_ROR(x2, xEmu, u32>>2, 15);
                        } else {
                            MOV32(x2, u32);
                            ADD_REG_LSL_IMM5(x2, xEmu, x2, 0);
                        }
                    }
                    if(MODREG) {
                        ed = (nextop&7);
                        if(!sse_reflect_reg(dyn, ninst, ed, x1)) {
                            u32 = offsetof(x86emu_t, xmm[ed]);
                            if(!(u32&3) && (u32>>2)<256) {
                                ADD_IMM8_ROR(x1, xEmu, u32>>2, 15);
                            } else {
                                MOV32(x1, u32);
                                ADD_REG_LSL_IMM5(x1, xEmu, x1, 0);
                            }
                        }
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        if(ed!=x1) {
                            MOV_REG(x1, ed);
                        }
                    }
                    u8 = F8;
                    MOV32(x3, u8);
                    CALL(sse42_compare_string_implicit_len, x1, 0);
                    CMPS_IMM8(x1, 0);
                    MOVW_COND(cEQ, xECX, (u8&1)?8:16);
                    if(u8&0b1000000) {
                        CLZ_COND(cNE, xECX, x1);
                        RSB_COND_IMM8(cNE, xECX, xECX, 31);
                    } else {
                        RBIT_COND(cNE, xECX, x1);
                        CLZ_COND(cNE, xECX, xECX);
                    }
                    break;
                    
                case 0xDF:
                    INST_NAME("AESKEYGENASSIST Gx, Ex, Ib");  // AES-NI
                    nextop = F8;
                    GETG;
                    sse_forget_reg(dyn, ninst, gd, x1);
                    MOV32(x1, gd); // gx
                    if(MODREG) {
                        ed = nextop&7;
                        sse_forget_reg(dyn, ninst, ed, x2);
                        MOV32(x2, ed);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 0, 0, 0, NULL);
                        if(ed!=x2) {
                            MOV_REG(x2, ed);
                        }
                    }
                    u8 = F8;
                    MOV32(x3, u8);
                    CALL(arm_aeskeygenassist, -1, 0);
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
            } else { \
                SMREAD(); \
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 255, 0, 0, NULL);\
                LDRH_IMM8_COND(YES, x1, ed, fixedaddress); \
                ed = x1;                        \
            }   \
            BFI_COND(YES, gd, ed, 0 ,16);

        case 0x40:
            INST_NAME("CMOVO Gw, Ew");
            GO( TSTS_IMM8_ROR(xFlags, 0b10, 0x0b)
                , cEQ, cNE, X_OF)
            break;
        case 0x41:
            INST_NAME("CMOVNO Gw, Ew");
            GO( TSTS_IMM8_ROR(xFlags, 0b10, 0x0b)
                , cNE, cEQ, X_OF)
            break;
        case 0x42:
            INST_NAME("CMOVC Gw, Ew");
            GO( TSTS_IMM8(xFlags, 1<<F_CF)
                , cEQ, cNE, X_CF)
            break;
        case 0x43:
            INST_NAME("CMOVNC Gw, Ew");
            GO( TSTS_IMM8(xFlags, 1<<F_CF)
                , cNE, cEQ, X_CF)
            break;
        case 0x44:
            INST_NAME("CMOVZ Gw, Ew");
            GO( TSTS_IMM8(xFlags, 1<<F_ZF)
                , cEQ, cNE, X_ZF)
            break;
        case 0x45:
            INST_NAME("CMOVNZ Gw, Ew");
            GO( TSTS_IMM8(xFlags, 1<<F_ZF)
                , cNE, cEQ, X_ZF)
            break;
        case 0x46:
            INST_NAME("CMOVBE Gw, Ew");
            GO( TSTS_IMM8(xFlags, (1<<F_CF)|(1<<F_ZF))
                , cEQ, cNE, X_CF|X_ZF)
            break;
        case 0x47:
            INST_NAME("CMOVNBE Gw, Ew");
            GO( TSTS_IMM8(xFlags, (1<<F_CF)|(1<<F_ZF))
                , cNE, cEQ, X_CF|X_ZF)
            break;
        case 0x48:
            INST_NAME("CMOVS Gw, Ew");
            GO( TSTS_IMM8(xFlags, 1<<F_SF)
                , cEQ, cNE, X_SF)
            break;
        case 0x49:
            INST_NAME("CMOVNS Gw, Ew");
            GO( TSTS_IMM8(xFlags, 1<<F_SF)
                , cNE, cEQ, X_SF)
            break;
        case 0x4A:
            INST_NAME("CMOVP Gw, Ew");
            GO( TSTS_IMM8(xFlags, 1<<F_PF)
                , cEQ, cNE, X_PF)
            break;
        case 0x4B:
            INST_NAME("CMOVNP Gw, Ew");
            GO( TSTS_IMM8(xFlags, 1<<F_PF)
                , cNE, cEQ, X_PF)
            break;
        case 0x4C:
            INST_NAME("CMOVL Gw, Ew");
            GO( UBFX(x2, xFlags, F_SF, 1);
                UBFX(x1, xFlags, F_OF, 1);
                CMPS_REG_LSL_IMM5(x1, x2, 0)
                , cEQ, cNE, X_OF|X_SF)
            break;
        case 0x4D:
            INST_NAME("CMOVGE Gw, Ew");
            GO( UBFX(x2, xFlags, F_SF, 1);
                UBFX(x1, xFlags, F_OF, 1);
                CMPS_REG_LSL_IMM5(x1, x2, 0)
                , cNE, cEQ, X_OF|X_SF)
            break;
        case 0x4E:
            INST_NAME("CMOVLE Gw, Ew");
            GO( UBFX(x2, xFlags, F_SF, 1);
                UBFX(x1, xFlags, F_OF, 1);
                XOR_REG_LSL_IMM5(x1, x1, x2, 0);
                UBFX(x2, xFlags, F_ZF, 1);
                ORRS_REG_LSL_IMM5(x2, x1, x2, 0);
                , cEQ, cNE, X_OF|X_SF|X_ZF)
            break;
        case 0x4F:
            INST_NAME("CMOVG Gw, Ew");
            GO( UBFX(x2, xFlags, F_SF, 1);
                UBFX(x1, xFlags, F_OF, 1);
                XOR_REG_LSL_IMM5(x1, x1, x2, 0);
                UBFX(x2, xFlags, F_ZF, 1);
                ORRS_REG_LSL_IMM5(x2, x1, x2, 0);
                , cNE, cEQ, X_OF|X_SF|X_ZF)
            break;
        #undef GO
        case 0x50:
            INST_NAME("MOVMSKPD Gd, Ex");
            nextop = F8;
            GETEX(q0, 0);
            gd = xEAX+((nextop&0x38)>>3);
            if(MODREG)
                q1 = fpu_get_scratch_quad(dyn);
            else 
                q1 = q0;
            VSHRQ_U64(q1, q0, 63);  // only bit 63 left
            VSHL_U64(q1+1, q1+1, 1);    // shift 1 left for higher bit
            VADD_8(q1, q1, q1+1);      // add low and high
            VMOVfrDx_U8(gd, q1, 0);     // grab the 2 sign bits
            break;
        case 0x54:
            INST_NAME("ANDPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0, 1);
            VANDQ(v0, v0, q0);
            break;
        case 0x55:
            INST_NAME("ANDNPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0, 1);
            VBICQ(v0, q0, v0);
            break;
        case 0x56:
            INST_NAME("ORPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0, 1);
            VORRQ(v0, v0, q0);
            break;
        case 0x57:
            INST_NAME("XORPD Gx, Ex");
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
            INST_NAME("ADDPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(q1, 1);
            if(!box86_dynarec_fastnan) {
                VMRS(x14);               // get fpscr
                ORR_IMM8(x3, x14, 0b010, 9); // enable exceptions
                BIC_IMM8(x3, x3, 0b10011111, 0);
                VMSR(x3);
            }
            VADD_F64(q1, q1, q0);
            if(!box86_dynarec_fastnan) {
                VMRS(x3);   // get the FPSCR reg and test FPU execption (invalid operation only)
                TSTS_IMM8_ROR(x3, 0b00000001, 0);
                VNEG_F64_cond(cNE, q1, q1);
                BIC_IMM8(x3, x3, 0b10011111, 0);
                VMSR(x3);
            }
            VADD_F64(q1+1, q1+1, q0+1);
            if(!box86_dynarec_fastnan) {
                VMRS(x3);   // get the FPSCR reg and test FPU execption (invalid operation only)
                TSTS_IMM8_ROR(x3, 0b00000001, 0);
                VNEG_F64_cond(cNE, q1+1, q1+1);
                VMSR(x14);  // restore fpscr
            }
            break;
        case 0x59:
            INST_NAME("MULPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(q1, 1);
            if(!box86_dynarec_fastnan) {
                VMRS(x14);               // get fpscr
                ORR_IMM8(x3, x14, 0b010, 9); // enable exceptions
                BIC_IMM8(x3, x3, 0b10011111, 0);
                VMSR(x3);
            }
            VMUL_F64(q1, q1, q0);
            if(!box86_dynarec_fastnan) {
                VMRS(x3);   // get the FPSCR reg and test FPU execption (invalid operation only)
                TSTS_IMM8_ROR(x3, 0b00000001, 0);
                VNEG_F64_cond(cNE, q1, q1);
                BIC_IMM8(x3, x3, 0b10011111, 0);
                VMSR(x3);
            }
            VMUL_F64(q1+1, q1+1, q0+1);
            if(!box86_dynarec_fastnan) {
                VMRS(x3);   // get the FPSCR reg and test FPU execption (invalid operation only)
                TSTS_IMM8_ROR(x3, 0b00000001, 0);
                VNEG_F64_cond(cNE, q1+1, q1+1);
                VMSR(x14);  // restore fpscr
            }
            break;
        case 0x5A:
            INST_NAME("CVTPD2PS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0);
            s0 = fpu_get_single_reg(dyn, ninst, v0, 0);
            VCVT_F32_F64(s0, v1);
            fpu_putback_single_reg(dyn, ninst, v0, 0, s0);
            s0 = fpu_get_single_reg(dyn, ninst, v0, 1);
            VCVT_F32_F64(s0, v1+1);
            fpu_putback_single_reg(dyn, ninst, v0, 1, s0);
            VEOR(v0+1, v0+1, v0+1);
            break;
        case 0x5B:
            INST_NAME("CVTPS2DQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0);
            if(v0>15)
                q0 = fpu_get_scratch_quad(dyn);
            else
                q0 = v0;
            if(v1>15) {
                q1 = fpu_get_scratch_quad(dyn);
                VMOVQ(q1, v1);
            } else
                q1 = v1;
            // need rounding!
            //VCVTQ_S32_F32(v0, v1);
            if(!box86_dynarec_fastround)
                u8 = sse_setround_reset(dyn, ninst, x1, x2, x14);
            else
                u8 = sse_setround(dyn, ninst, x1, x2, x14);
            for(int i=0; i<4; ++i) {
                VCVTR_S32_F32(q0*2+i, q1*2+i);
                if(!box86_dynarec_fastround) {
                    VMRS(x3);   // get the FPCSR reg and test FPU exception (IO only)
                    TSTS_IMM8_ROR(x3, 0b00000001, 0);
                    MOV_IMM_COND(cNE, x1, 0b10, 1);   // 0x80000000
                    VMOVtoVcond(cNE, q0*2+i, x1);
                    BIC_IMM8_COND(cNE, x3, x3, 1, 0);    // reset FPU
                    VMSR_cond(cNE, x3);
                }
        }
            if(q0!=v0) {
                VMOVQ(v0, q0);
            }
            x87_restoreround(dyn, ninst, u8);
            break;
        case 0x5C:
            INST_NAME("SUBPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(q1, 1);
            if(!box86_dynarec_fastnan) {
                VMRS(x14);               // get fpscr
                ORR_IMM8(x3, x14, 0b010, 9); // enable exceptions
                BIC_IMM8(x3, x3, 0b10011111, 0);
                VMSR(x3);
            }
            VSUB_F64(q1, q1, q0);
            if(!box86_dynarec_fastnan) {
                VMRS(x3);   // get the FPSCR reg and test FPU execption (invalid operation only)
                TSTS_IMM8_ROR(x3, 0b00000001, 0);
                VNEG_F64_cond(cNE, q1, q1);
                BIC_IMM8(x3, x3, 0b10011111, 0);
                VMSR(x3);
            }
            VSUB_F64(q1+1, q1+1, q0+1);
            if(!box86_dynarec_fastnan) {
                VMRS(x3);   // get the FPSCR reg and test FPU execption (invalid operation only)
                TSTS_IMM8_ROR(x3, 0b00000001, 0);
                VNEG_F64_cond(cNE, q1+1, q1+1);
                VMSR(x14);  // restore fpscr
            }
            break;
        case 0x5D:
            INST_NAME("MINPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0, 1);
            // MINPD: if any input is NaN, or Ex[i]<Gx[i], copy Ex[i] -> Gx[i]
            VCMP_F64(v0, q0);
            VMRS_APSR();
            VMOVcond_64(cCS, v0, q0);
            VCMP_F64(v0+1, q0+1);
            VMRS_APSR();
            VMOVcond_64(cCS, v0+1, q0+1);
            break;
        case 0x5E:
            INST_NAME("DIVPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(q1, 1);
            if(!box86_dynarec_fastnan) {
                VMRS(x14);               // get fpscr
                ORR_IMM8(x3, x14, 0b010, 9); // enable exceptions
                BIC_IMM8(x3, x3, 0b10011111, 0);
                VMSR(x3);
            }
            VDIV_F64(q1, q1, q0);
            if(!box86_dynarec_fastnan) {
                VMRS(x3);   // get the FPSCR reg and test FPU execption (invalid operation only)
                TSTS_IMM8_ROR(x3, 0b00000001, 0);
                VNEG_F64_cond(cNE, q1, q1);
                BIC_IMM8(x3, x3, 0b10011111, 0);
                VMSR(x3);
            }
            VDIV_F64(q1+1, q1+1, q0+1);
            if(!box86_dynarec_fastnan) {
                VMRS(x3);   // get the FPSCR reg and test FPU execption (invalid operation only)
                TSTS_IMM8_ROR(x3, 0b00000001, 0);
                VNEG_F64_cond(cNE, q1+1, q1+1);
                VMSR(x14);  // restore fpscr
            }
            break;
        case 0x5F:
            INST_NAME("MAXPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0);
            GETGX(v0, 1);
            // MAXPD: if any input is NaN, or Ex[i]>Gx[i], copy Ex[i] -> Gx[i]
            VCMP_F64(q0, v0);
            VMRS_APSR();
            VMOVcond_64(cCS, v0, q0);
            VCMP_F64(q0+1, v0+1);
            VMRS_APSR();
            VMOVcond_64(cCS, v0+1, q0+1);
            break;
        case 0x60:
            INST_NAME("PUNPCKLBW Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            GETEX(q0, 0);
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            VMOVD(v0+1, q0);  // get a copy
            VZIP_8(v0, v0+1);
            break;
        case 0x61:
            INST_NAME("PUNPCKLWD Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            GETEX(q0, 0);
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            VMOVD(v0+1, q0);  // get a copy
            VZIP_16(v0, v0+1);
            break;
        case 0x62:
            INST_NAME("PUNPCKLDQ Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            GETEX(q0, 0);
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            VMOVD(v0+1, q0);  // get a copy
            VTRN_32(v0, v0+1);  // same as VZIP_32
            break;
        case 0x63:
            INST_NAME("PACKSSWB Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            VQMOVN_S16(q0+0, q0);
            if(q0==q1) {
                VMOVD(q0+1, q0+0);
            } else {
                VQMOVN_S16(q0+1, q1);
            }
            break;
        case 0x64:
            INST_NAME("PCMPGTB Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0);
            VCGTQ_S8(v0, v0, v1);
            break;
        case 0x65:
            INST_NAME("PCMPGTW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0);
            VCGTQ_S16(v0, v0, v1);
            break;
        case 0x66:
            INST_NAME("PCMPGTD Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0);
            VCGTQ_S32(v0, v0, v1);
            break;
        case 0x67:
            INST_NAME("PACKUSWB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0);
            VQMOVUN_S16(v0, v0);
            if(v0==v1) {
                VMOVD(v0+1, v0);
            } else {
                VQMOVUN_S16(v0+1, v1);
            }
            break;
        case 0x68:
            INST_NAME("PUNPCKHBW Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            GETEX(q0, 0);
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            VMOVD(v0, v0+1);    // get high part into low
            VMOVD(v0+1, q0+1);  // get a copy
            VZIP_8(v0, v0+1);
            break;
        case 0x69:
            INST_NAME("PUNPCKHWD Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            GETEX(q0, 0);
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            VMOVD(v0, v0+1);    // get high part into low
            VMOVD(v0+1, q0+1);  // get a copy
            VZIP_16(v0, v0+1);
            break;
        case 0x6A:
            INST_NAME("PUNPCKHDQ Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            GETEX(q0, 0);
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            VMOVD(v0, v0+1);    // get high part into low
            VMOVD(v0+1, q0+1);  // get a copy
            VZIP_32(v0, v0+1);
            break;
        case 0x6B:
            INST_NAME("PACKSSDW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0);
            VQMOVN_S32(v0, v0);
            if(v0==v1) {
                VMOVD(v0+1, v0);
            } else {
                VQMOVN_S32(v0+1, v1);
            }
            break;
        case 0x6C:
            INST_NAME("PUNPCKLQDQ Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
                VMOVD(v0+1, v1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL);
                VLDR_64(v0+1, ed, fixedaddress);
            }
            break;
        case 0x6D:
            INST_NAME("PUNPCKHQDQ Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            VMOVD(q0, q0+1);
            if(MODREG) {
                q1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
                VMOVD(q0+1, q1+1);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023-8, 3, 0, NULL);
                VLDR_64(q0+1, ed, fixedaddress+8);
            }
            break;
        case 0x6E:
            INST_NAME("MOVD Gx, Ed");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            GETED;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            VEORQ(v0, v0, v0); // v0 = U32{0, 0}
            VMOVtoDx_32(v0, 0, ed);// d0 = U32{ed, 0}
            break;
        case 0x6F:
            INST_NAME("MOVDQA Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if(MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                VMOVQ(v0, v1);
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                VLD1Q_64(v0, ed);
            }
            break;
        case 0x70:
            INST_NAME("PSHUFD Gx,Ex,Ib");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            i32 = -1;
            if(MODREG) {
                u8 = F8;
                v1 = sse_get_reg(dyn, ninst, x1, nextop&7, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                if ((u8==0) || (u8==0b01010101) || (u8==0b10101010) || (u8==0b11111111)) {
                    VDUPQ_32(v0, v1+((u8>>1)&1), u8&1);
                } else if (u8==0b01000100) {
                    VMOVD(v0+1, v1);
                    VMOVD(v0, v1);
                } else if (u8==0b11101110) {
                    VMOVD(v0, v1+1);
                    VMOVD(v0+1, v1+1);
                } else if (u8==0b10110001) {
                    VREV64Q_32(v0, v1);
                } else if (u8==0b01010001) {
                    VDUP_32(v0+1, v1, 1);
                    VREV64Q_32(v0, v1);
                } else if (u8==0b00011011) {
                    VREV64Q_32(v0, v1);
                    VSWP(v0, v0+1);
                } else if (u8==0b01001110) {
                    if(v0==v1) {
                        VSWP(v0, v0+1);
                    } else {
                        VMOVD(v0, v1+1);
                        VMOVD(v0+1, v1);
                    }
                } else if (u8==0b11011000) {
                    //VTRN_32(Dd, Dm) swap Dd[1] with Dm[0]
                    VMOVQ(v0, v1);
                    VTRN_32(v0, v0+1);
                } else if (u8==0b00100111) {
                    //VTRN_32(Dd, Dm) swap Dd[1] with Dm[0]
                    VMOVQ(v0, v1);
                    VTRN_32(v0+1, v0);
                } else if (u8==0b10001101) {
                    //VTRN_32(Dd, Dm) swap Dd[1] with Dm[0]
                    VREV64Q_32(v0, v1); // 10 11 00 01
                    VTRN_32(v0, v0+1);  // 10 00 11 01
                } else if (u8==0b01110010) {
                    //VTRN_32(Dd, Dm) swap Dd[1] with Dm[0]
                    VREV64Q_32(v0, v1); // 10 11 00 01
                    VTRN_32(v0+1, v0);  // 01 11 00 10
                } else {
                    q0 = fpu_get_scratch_quad(dyn); // temporary storage
                    d0 = fpu_get_scratch_double(dyn);
                    if(v1!=v0)
                        q0 = v0;    // no need for temp storage
                    else if((u8&0xf)==((u8>>4)&0xf))
                        q0 = v0;
                    // Low part
                    switch (u8&0xf) {
                        case 0b0000: VDUP_32(q0, v1, 0);        break;
                        case 0b0001: VREV64_32(q0, v1);         break;  // reverse low part
                        case 0b0010: VREV64_32(d0, v1+1);
                                     VEXT_8(q0, d0, v1, 4);     break;
                        case 0b0011: VEXT_8(q0, v1+1, v1, 4);   break;
                        case 0b0100: VMOVD(q0, v1);             break; // same
                        case 0b0101: VDUP_32(q0, v1, 1);        break;
                        case 0b0110: VEXT_8(q0, v1, v1+1, 4);
                                     VREV64_32(q0, q0);         break;
                        case 0b0111: VREV64_32(d0, v1);
                                     VEXT_8(q0, v1+1, d0, 4);   break;
                        case 0b1000: VREV64_32(d0, v1);
                                     VEXT_8(q0, d0, v1+1, 4);   break;
                        case 0b1001: VEXT_8(q0, v1, v1+1, 4);   break;
                        case 0b1010: VDUP_32(q0, v1+1, 0);      break;
                        case 0b1011: VREV64_32(q0, v1+1);       break;
                        case 0b1100: VEXT_8(q0, v1+1, v1, 4);
                                     VREV64_32(q0, q0);         break;
                        case 0b1101: VREV64_32(d0, v1+1);
                                     VEXT_8(q0, v1, d0, 4);     break;
                        case 0b1110: VMOVD(q0, v1+1);           break;
                        case 0b1111: VDUP_32(q0, v1+1, 1);      break;
                    }
                    // High part
                    if((u8&0xf)==((u8>>4)&0xf)) {
                        VMOVD(q0+1, q0);
                    } else switch ((u8>>4)&0xf) {
                        case 0b0000: VDUP_32(q0+1, v1, 0);        break;
                        case 0b0001: VREV64_32(q0+1, v1);         break;  // reverse low part
                        case 0b0010: VREV64_32(d0, v1+1);
                                     VEXT_8(q0+1, d0, v1, 4);     break;
                        case 0b0011: VEXT_8(q0+1, v1+1, v1, 4);   break;
                        case 0b0100: VMOVD(q0+1, v1);             break; // same
                        case 0b0101: VDUP_32(q0+1, v1, 1);        break;
                        case 0b0110: VEXT_8(q0+1, v1, v1+1, 4);
                                     VREV64_32(q0+1, q0+1);       break;
                        case 0b0111: VREV64_32(d0, v1);
                                     VEXT_8(q0+1, v1+1, d0, 4);   break;
                        case 0b1000: VREV64_32(d0, v1);
                                     VEXT_8(q0+1, d0, v1+1, 4);   break;
                        case 0b1001: VEXT_8(q0+1, v1, v1+1, 4);   break;
                        case 0b1010: VDUP_32(q0+1, v1+1, 0);      break;
                        case 0b1011: VREV64_32(q0+1, v1+1);       break;
                        case 0b1100: VEXT_8(q0+1, v1+1, v1, 4);
                                     VREV64_32(q0+1, q0+1);       break;
                        case 0b1101: VREV64_32(d0, v1+1);
                                     VEXT_8(q0+1, v1, d0, 4);     break;
                        case 0b1110: VMOVD(q0+1, v1+1);           break;
                        case 0b1111: VDUP_32(q0+1, v1+1, 1);      break;
                    }
                    VMOVQ(v0, q0);
                }
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                u8 = F8;
                if ((u8==0) || (u8==0b01010101) || (u8==0b10101010) || (u8==0b11111111)) {
                    if(u8&3) {
                        ADD_IMM8(x3, ed, (u8&3)*4);
                    }
                    VLD1QALL_32(v0, (u8&3)?x3:ed);
                } else {
                    for (int i=0; i<4; ++i) {
                        int32_t idx = (u8>>(i*2))&3;
                        if(idx!=i32) {
                            ADD_IMM8(x2, ed, idx*4);
                            i32 = idx;
                        }
                        VLD1LANE_32(v0+(i/2), x2, i&1);
                    }
                }
            }
            break;
        case 0x71:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLW Ex, Ib");
                    if(MODREG) {
                        q0 = sse_get_reg(dyn, ninst, x1, nextop&7, 1);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        q0 = fpu_get_scratch_quad(dyn);
                        VLD1Q_16(q0, ed);
                    }
                    u8 = F8;
                    if(u8) {
                        if (u8>15) {
                            VEORQ(q0, q0, q0);
                        } else if(u8) {
                            VSHRQ_U16(q0, q0, u8);
                        }
                        if((nextop&0xC0)!=0xC0) {
                            VST1Q_16(q0, ed);
                            SMWRITE2();
                        }
                    }
                    break;
                case 4:
                    INST_NAME("PSRAW Ex, Ib");
                    if(MODREG) {
                        q0 = sse_get_reg(dyn, ninst, x1, nextop&7, 1);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        q0 = fpu_get_scratch_quad(dyn);
                        VLD1Q_16(q0, ed);
                    }
                    u8 = F8;
                    if(u8) {
                        VSHRQ_S16(q0, q0, (u8>15)?0:u8);
                    }
                    if((nextop&0xC0)!=0xC0) {
                        VST1Q_16(q0, ed);
                        SMWRITE2();
                    }
                    break;
                case 6:
                    INST_NAME("PSLLW Ex, Ib");
                    if(MODREG) {
                        q0 = sse_get_reg(dyn, ninst, x1, nextop&7, 1);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        q0 = fpu_get_scratch_quad(dyn);
                        VLD1Q_16(q0, ed);
                    }
                    u8 = F8;
                    if(u8) {
                        if (u8>15) {
                            VEORQ(q0, q0, q0);
                        } else {
                            VSHLQ_16(q0, q0, u8);
                        }
                        if((nextop&0xC0)!=0xC0) {
                            VST1Q_16(q0, ed);
                            SMWRITE2();
                        }
                    }
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
            break;
        case 0x72:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLD Ex, Ib");
                    if(MODREG) {
                        q0 = sse_get_reg(dyn, ninst, x1, nextop&7, 1);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        q0 = fpu_get_scratch_quad(dyn);
                        VLD1Q_32(q0, ed);
                    }
                    u8 = F8;
                    if(u8) {
                        if (u8>31) {
                            VEORQ(q0, q0, q0);
                        } else if(u8) {
                            VSHRQ_U32(q0, q0, u8);
                        }
                        if((nextop&0xC0)!=0xC0) {
                            VST1Q_32(q0, ed);
                            SMWRITE2();
                        }
                    }
                    break;
                case 4:
                    INST_NAME("PSRAD Ex, Ib");
                    if(MODREG) {
                        q0 = sse_get_reg(dyn, ninst, x1, nextop&7, 1);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        q0 = fpu_get_scratch_quad(dyn);
                        VLD1Q_32(q0, ed);
                    }
                    u8 = F8;
                    if(u8) {
                        VSHRQ_S32(q0, q0, (u8>31)?0:u8);
                    }
                    if((nextop&0xC0)!=0xC0) {
                        VST1Q_32(q0, ed);
                        SMWRITE2();
                    }
                    break;
                case 6:
                    INST_NAME("PSLLD Ex, Ib");
                    if(MODREG) {
                        q0 = sse_get_reg(dyn, ninst, x1, nextop&7, 1);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        q0 = fpu_get_scratch_quad(dyn);
                        VLD1Q_32(q0, ed);
                    }
                    u8 = F8;
                    if(u8) {
                        if (u8>31) {
                            VEORQ(q0, q0, q0);
                        } else {
                            VSHLQ_32(q0, q0, u8);
                        }
                        if((nextop&0xC0)!=0xC0) {
                            VST1Q_32(q0, ed);
                            SMWRITE2();
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x73:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 2:
                    INST_NAME("PSRLQ Ex, Ib");
                    if(MODREG) {
                        q0 = sse_get_reg(dyn, ninst, x1, nextop&7, 1);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        q0 = fpu_get_scratch_quad(dyn);
                        VLD1Q_64(q0, ed);
                    }
                    u8 = F8;
                    if(u8) {
                        if (u8>63) {
                            VEORQ(q0, q0, q0);
                        } else if(u8) {
                            VSHRQ_U64(q0, q0, u8);
                        }
                        if((nextop&0xC0)!=0xC0) {
                            VST1Q_64(q0, ed);
                            SMWRITE2();
                        }
                    }
                    break;
                case 3:
                    INST_NAME("PSRLDQ Ex, Ib");
                    if(MODREG) {
                        q0 = sse_get_reg(dyn, ninst, x1, nextop&7, 1);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        q0 = fpu_get_scratch_quad(dyn);
                        VLD1Q_64(q0, ed);
                    }
                    u8 = F8;
                    if(u8) {
                        if(u8>15) {
                            VEORQ(q0, q0, q0);
                        } else {
                            q1 = fpu_get_scratch_quad(dyn);
                            VEORQ(q1, q1, q1);
                            VEXTQ_8(q0, q0, q1, u8);
                        }
                        if((nextop&0xC0)!=0xC0) {
                            VST1Q_64(q0, ed);
                            SMWRITE2();
                        }
                    }
                    break;
                case 6:
                    INST_NAME("PSLLQ Ex, Ib");
                    if(MODREG) {
                        q0 = sse_get_reg(dyn, ninst, x1, nextop&7, 1);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        q0 = fpu_get_scratch_quad(dyn);
                        VLD1Q_64(q0, ed);
                    }
                    u8 = F8;
                    if(u8) {
                        if (u8>63) {
                            VEORQ(q0, q0, q0);
                        } else {
                            VSHLQ_64(q0, q0, u8);
                        }
                        if((nextop&0xC0)!=0xC0) {
                            VST1Q_64(q0, ed);
                            SMWRITE2();
                        }
                    }
                    break;
                case 7:
                    INST_NAME("PSLLDQ Ex, Ib");
                    if(MODREG) {
                        q0 = sse_get_reg(dyn, ninst, x1, nextop&7, 1);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                        q0 = fpu_get_scratch_quad(dyn);
                        VLD1Q_64(q0, ed);
                    }
                    u8 = F8;
                    if(u8) {
                        if(u8>15) {
                            VEORQ(q0, q0, q0);
                        } else if(u8>0) {
                            q1 = fpu_get_scratch_quad(dyn);
                            VEORQ(q1, q1, q1);
                            VEXTQ_8(q0, q1, q0, 16-u8);
                        }
                        if((nextop&0xC0)!=0xC0) {
                            VST1Q_64(q0, ed);
                            SMWRITE2();
                        }
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0x74:
            INST_NAME("PCMPEQB Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VCEQQ_8(v0, v0, q0);
            break;
        case 0x75:
            INST_NAME("PCMPEQW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VCEQQ_16(v0, v0, q0);
            break;
        case 0x76:
            INST_NAME("PCMPEQD Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VCEQQ_32(v0, v0, q0);
            break;

        case 0x7E:
            INST_NAME("MOVD Ed,Gx");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                ed = xEAX + (nextop&7);
                VMOVfrDx_32(ed, v0, 0);
            } else {
                VMOVfrDx_32(x2, v0, 0); // to avoid Bus Error, using regular store
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095, 0, 0, NULL);
                STR_IMM9(x2, ed, fixedaddress);
                SMWRITE2();
            }
            break;
        case 0x7F:
            INST_NAME("MOVDQA Ex,Gx");
            nextop = F8;
            GETGX(v0, 0);
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, nextop&7);
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                VST1Q_32(v0, ed);
                SMWRITE2();
            }
            break;

        case 0xA3:
            INST_NAME("BT Ew, Gw");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE(x1);
            nextop = F8;
            GETGD;  // there is an AND below, so 32bits is the same (no need for GETGW)
            if(MODREG) {
                ed = xEAX+(nextop&7);   // no need for extract
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, &fixedaddress, 255, 0, 0, NULL);
                SBFX(x1, gd, 4, 12);    // r1 = (gw>>4);
                ADD_REG_LSL_IMM5(x1, ed, x1, 1); //(&ed)+=r1*2;
                LDRH_IMM8(x1, x1, fixedaddress);
                ed = x1;
            }
            AND_IMM8(x2, gd, 0x0f);
            MOV_REG_LSR_REG(x1, ed, x2);
            BFI(xFlags, x1, F_CF, 1);
            break;
        case 0xA4:
        case 0xA5:
            nextop = F8;
            if(opcode==0xA4) {
                INST_NAME("SHLD Ew, Gw, Ib");
            } else {
                INST_NAME("SHLD Ew, Gw, CL");
            }
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            SETFLAGS(X_ALL, SF_SET_DF);
            GETEWW(x14, x1);
            GETGW(x2);
            if(opcode==0xA4) {
                u8 = F8;
                MOVW(x3, u8);
            } else {
                UXTB(x3, xECX, 0);
            }
            CALL(shld16, x1, (1<<wback));
            EWBACKW(x1);
            break;

        case 0xAB:
            INST_NAME("BTS Ew, Gw");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE(x1);
            nextop = F8;
            GETGD;  // there is an AND below, to 32bits is the same
            if(MODREG) {
                ed = xEAX+(nextop&7);
                wback = 0;   // no need for extract
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0, 0, NULL);
                SBFX(x14, gd, 4, 12);    // r14 = (gw>>4);
                ADD_REG_LSL_IMM5(x3, wback, x14, 1); //(&ew)+=r14*2;
                LDRH_IMM8(x14, x3, fixedaddress);
                wback = x3;
                ed = x14;
            }
            AND_IMM8(x2, gd, 15);
            MOV_REG_LSR_REG(x1, ed, x2);
            ANDS_IMM8(x1, x1, 1);
            BFI(xFlags, x1, F_CF, 1);
            B_NEXT(cNE);
            MOVW(x1, 1);
            ORR_REG_LSL_REG(ed, ed, x1, x2);
            if(wback) {
                STRH_IMM8(ed, wback, fixedaddress);
                SMWRITE();
            }
            break;
        case 0xAC:
        case 0xAD:
            nextop = F8;
            if(opcode==0xAC) {
                INST_NAME("SHRD Ew, Gw, Ib");
            } else {
                INST_NAME("SHRD Ew, Gw, CL");
            }
            MESSAGE(LOG_DUMP, "Need Optimization\n");
            SETFLAGS(X_ALL, SF_SET_DF);
            GETEWW(x14, x1);
            GETGW(x2);
            if(opcode==0xAC) {
                u8 = F8;
                MOVW(x3, u8);
            } else {
                UXTB(x3, xECX, 0);
            }
            CALL(shrd16, x1, (1<<wback));
            EWBACKW(x1);
            break;

        case 0xAF:
            INST_NAME("IMUL Gw,Ew");
            SETFLAGS(X_ALL, SF_PENDING);
            nextop = F8;
            if(MODREG) {
                ed = xEAX+(nextop&7);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0, 0, NULL);
                LDRH_IMM8(x1, wback, fixedaddress);
                ed = x1;
            }
            gd = xEAX+((nextop&0x38)>>3);
            SMULBB(x2, gd, ed);
            UFLAG_RES(x2);
            BFI((xEAX+((nextop&0x38)>>3)), x2, 0, 16);
            UFLAG_DF(x1, d_imul16);
            break;

        case 0xB3:
            INST_NAME("BTR Ew, Gw");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE(x1);
            nextop = F8;
            GETGD;  // there is an AND below, to 32bits is the same
            if(MODREG) {
                ed = xEAX+(nextop&7);
                wback = 0;   // no need for extract
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0, 0, NULL);
                SBFX(x14, gd, 4, 12);    // r14 = (gw>>4);
                ADD_REG_LSL_IMM5(x3, wback, x14, 1); //(&ew)+=r14*2;
                LDRH_IMM8(x14, x3, fixedaddress);
                wback = x3;
                ed = x14;
            }
            AND_IMM8(x2, gd, 15);
            MOV_REG_LSR_REG(x1, ed, x2);
            ANDS_IMM8(x1, x1, 1);
            BFI(xFlags, x1, F_CF, 1);
            B_NEXT(cEQ);
            MOVW(x1, 1);
            XOR_REG_LSL_REG(ed, ed, x1, x2);
            if(wback) {
                STRH_IMM8(ed, wback, fixedaddress);
                SMWRITE();
            }
            break;

        case 0xB6:
            INST_NAME("MOVZX Gw, Eb");
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = (nextop&7);
                eb1 = xEAX+(ed&3);  // Ax, Cx, Dx or Bx
                eb2 = (ed&4)>>2;    // L or H
                UXTB(x1, eb1, eb2);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 4095, 0, 0, NULL);
                LDRB_IMM9(x1, ed, fixedaddress);
            }
            BFI(gd, x1, 0, 16);
            break;
        case 0xB7:
            INST_NAME("MOVZX Gw, Ew");
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = xEAX + (nextop&7);
                UXTH(x1, ed, 0);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 255, 0, 0, NULL);
                LDRH_IMM8(x1, ed, fixedaddress);
            }
            BFI(gd, x1, 0, 16);
            break;

        case 0xBA:
            nextop = F8;
            switch((nextop>>3)&7) {
                case 4:
                    INST_NAME("BT Ew, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE(x1);
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0, 0, NULL);
                        LDRH_IMM8(x1, wback, fixedaddress);
                        ed = x1;
                    }
                    u8 = F8;
                    u8&=0x0f;
                    if(u8) {
                        MOV_REG_LSR_IMM5(x1, ed, u8);
                        ed = x1;
                    }
                    BFI(xFlags, ed, F_CF, 1);
                    break;
                case 5:
                    INST_NAME("BTS Ew, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE(x1);
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0, 0, NULL);
                        LDRH_IMM8(x1, wback, fixedaddress);
                        ed = x1;
                    }
                    u8 = F8;
                    u8&=0x0f;
                    if(u8) {
                        MOV_REG_LSR_IMM5(x14, ed, u8);
                        ANDS_IMM8(x14, x14, 1);
                    } else {
                        ANDS_IMM8(x14, ed, 1);
                    }
                    BFI(xFlags, x14, F_CF, 1);
                    B_MARK3(cNE); // bit already set, jump to next instruction
                    MOVW(x14, 1);
                    XOR_REG_LSL_IMM5(ed, ed, x14, u8);
                    if(wback) {
                        STRH_IMM8(ed, wback, fixedaddress);
                        SMWRITE();
                    }
                    MARK3;
                    break;
                case 6:
                    INST_NAME("BTR Ew, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE(x1);
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0, 0, NULL);
                        LDRH_IMM8(x1, wback, fixedaddress);
                        ed = x1;
                    }
                    u8 = F8;
                    u8&=0x0f;
                    if(u8) {
                        MOV_REG_LSR_IMM5(x14, ed, u8);
                        ANDS_IMM8(x14, x14, 1);
                    } else {
                        ANDS_IMM8(x14, ed, 1);
                    }
                    BFI(xFlags, x14, F_CF, 1);
                    B_MARK3(cEQ); // bit already clear, jump to next instruction
                    //MOVW(x14, 1); // already 0x01
                    XOR_REG_LSL_IMM5(ed, ed, x14, u8);
                    if(wback) {
                        STRH_IMM8(ed, wback, fixedaddress);
                        SMWRITE();
                    }
                    MARK3;
                    break;
                case 7:
                    INST_NAME("BTC Ew, Ib");
                    SETFLAGS(X_CF, SF_SUBSET);
                    SET_DFNONE(x1);
                    if(MODREG) {
                        ed = xEAX+(nextop&7);
                        wback = 0;
                    } else {
                        SMREAD();
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0, 0, NULL);
                        LDRH_IMM8(x1, wback, fixedaddress);
                        ed = x1;
                    }
                    u8 = F8;
                    u8&=0x0f;
                    if(u8) {
                        MOV_REG_LSR_IMM5(x14, ed, u8);
                        ANDS_IMM8(x14, x14, 1);
                    } else {
                        ANDS_IMM8(x14, ed, 1);
                    }
                    BFI(xFlags, x14, F_CF, 1);
                    MOVW_COND(cEQ, x14, 1); // may already 0x01
                    XOR_REG_LSL_IMM5(ed, ed, x14, u8);
                    if(wback) {
                        STRH_IMM8(ed, wback, fixedaddress);
                        SMWRITE();
                    }
                    MARK3;
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xBB:
            INST_NAME("BTC Ew, Gw");
            SETFLAGS(X_CF, SF_SUBSET);
            SET_DFNONE(x1);
            nextop = F8;
            GETGD;  // there is an AND below, to 32bits is the same
            if(MODREG) {
                ed = xEAX+(nextop&7);
                wback = 0;   // no need for extract
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255, 0, 0, NULL);
                SBFX(x14, gd, 4, 12);    // r14 = (gw>>4);
                ADD_REG_LSL_IMM5(x3, wback, x14, 1); //(&ew)+=r14*2;
                LDRH_IMM8(x14, x3, fixedaddress);
                wback = x3;
                ed = x14;
            }
            AND_IMM8(x2, gd, 15);
            MOV_REG_LSR_REG(x1, ed, x2);
            BFI(xFlags, x1, F_CF, 1);
            MOVW(x1, 1);
            XOR_REG_LSL_REG(ed, ed, x1, x2);
            if(wback) {
                STRH_IMM8(ed, wback, fixedaddress);
                SMWRITE();
            }
            break;
        case 0xBC:
            INST_NAME("BSF Ew,Gw");
            SETFLAGS(X_ZF, SF_SUBSET);
            SET_DFNONE(x1);
            nextop = F8;
            GETEW(x1);  // Get EW
            TSTS_REG_LSL_IMM5(x1, x1, 0);
            XOR_IMM8_COND(cEQ, x1, x1, 1);
            B_MARK(cEQ);
            RBIT(x1, x1);   // reverse
            CLZ(x2, x1);    // x2 gets leading 0 == BSF
            BFI(xEAX+((nextop&0x38)>>3), x2, 0, 16);
            XOR_REG_LSL_IMM5(x1, x1, x1, 0);    //ZF not set
            MARK;
            BFI(xFlags, x1, F_ZF, 1);
            break;
        case 0xBD:
            INST_NAME("BSR Ew,Gw");
            SETFLAGS(X_ZF, SF_SUBSET);
            SET_DFNONE(x1);
            nextop = F8;
            GETEW(x1);  // Get EW
            TSTS_REG_LSL_IMM5(x1, x1, 0);
            XOR_IMM8_COND(cEQ, x1, x1, 1);
            B_MARK(cEQ);
            MOV_REG_LSL_IMM5(x1, x1, 16);   // put bits on top
            CLZ(x2, x1);    // x2 gets leading 0
            RSB_IMM8(x2, x2, 15); // complement
            BFI(xEAX+((nextop&0x38)>>3), x2, 0, 16);
            XOR_REG_LSL_IMM5(x1, x1, x1, 0);    //ZF not set
            MARK;
            BFI(xFlags, x1, F_ZF, 1);
            break;
        case 0xBE:
            INST_NAME("MOVSX Gw, Eb");
            nextop = F8;
            GETGD;
            if(MODREG) {
                ed = (nextop&7);
                eb1 = xEAX+(ed&3);  // Ax, Cx, Dx or Bx
                eb2 = (ed&4)>>2;    // L or H
                SXTB(x1, eb1, eb2);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, &fixedaddress, 255, 0, 0, NULL);
                LDRSB_IMM8(x1, ed, fixedaddress);
            }
            BFI(gd, x1, 0, 16);
            break;
        case 0xBF:
            INST_NAME("MOVSX Gw, Ew");
            nextop = F8;
            GETGD;
            GETEW(x1);
            BFI(gd, x1, 0, 16);
            break;

        case 0xC1:
            INST_NAME("XADD Gw, Ew");
            SETFLAGS(X_ALL, SF_SET_PENDING);
            nextop = F8;
            GETGW(x1);
            GETEW(x2);
            BFI((xEAX+((nextop&0x38)>>3)), x2, 0, 16);  // GW <- EW
            emit_add16(dyn, ninst, x2, x1, x14, x3, 1);
            EWBACK;
            break;
        case 0xC2:
            INST_NAME("CMPPD Gx, Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
            GETEX(d0, 0);
            u8 = F8&7;
            // 0
            VCMP_F64(v0, d0);
            VMRS_APSR();
            MOVW(x2, 0);
            switch(u8&7) {
                case 0: MVN_COND_REG_LSL_IMM5(cEQ, x2, x2, 0); break;   // Equal
                case 1: MVN_COND_REG_LSL_IMM5(cCC, x2, x2, 0); break;   // Less than
                case 2: MVN_COND_REG_LSL_IMM5(cLS, x2, x2, 0); break;   // Less or equal
                case 3: MVN_COND_REG_LSL_IMM5(cVS, x2, x2, 0); break;   // NaN
                case 4: MVN_COND_REG_LSL_IMM5(cNE, x2, x2, 0); break;   // Not Equal or unordered
                case 5: MVN_COND_REG_LSL_IMM5(cCS, x2, x2, 0); break;   // Greater or equal or unordered
                case 6: MVN_COND_REG_LSL_IMM5(cHI, x2, x2, 0); break;   // Greater or unordered, test inverted, N!=V so unordereded or less than (inverted)
                case 7: MVN_COND_REG_LSL_IMM5(cVC, x2, x2, 0); break;   // not NaN
            }
            VMOVtoV_D(v0, x2, x2);
            // 1
            VCMP_F64(v0+1, d0+1);
            VMRS_APSR();
            MOVW(x2, 0);
            switch(u8&7) {
                case 0: MVN_COND_REG_LSL_IMM5(cEQ, x2, x2, 0); break;   // Equal
                case 1: MVN_COND_REG_LSL_IMM5(cCC, x2, x2, 0); break;   // Less than
                case 2: MVN_COND_REG_LSL_IMM5(cLS, x2, x2, 0); break;   // Less or equal
                case 3: MVN_COND_REG_LSL_IMM5(cVS, x2, x2, 0); break;   // NaN
                case 4: MVN_COND_REG_LSL_IMM5(cNE, x2, x2, 0); break;   // Not Equal or unordered
                case 5: MVN_COND_REG_LSL_IMM5(cCS, x2, x2, 0); break;   // Greater or equal or unordered
                case 6: MVN_COND_REG_LSL_IMM5(cHI, x2, x2, 0); break;   // Greater or unordered, test inverted, N!=V so unordereded or less than (inverted)
                case 7: MVN_COND_REG_LSL_IMM5(cVC, x2, x2, 0); break;   // not NaN
            }
            VMOVtoV_D(v0+1, x2, x2);
            break;

        case 0xC4:
            INST_NAME("PINSRW Gx,Ed,Ib");
            nextop = F8;
            GETGX(v0, 1);
            if(MODREG) {
                u8 = (F8)&7;
                ed = xEAX+(nextop&7);
                VMOVtoDx_16(v0+(u8/4), u8&3, ed);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 0, 0, 0, NULL);
                u8 = (F8)&7;
                VLD1LANE_16(v0+(u8/4), wback, u8&3);
            }
            break;
        case 0xC5:
            INST_NAME("PEXTRW Gd,Ex,Ib");
            nextop = F8;
            gd = xEAX+((nextop&0x38)>>3);
            if(MODREG) {
                GETEX(v0, 0);
                u8 = (F8)&7;
                VMOVfrDx_U16(gd, v0+(u8/4), (u8&3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 255-14, 0, 0, NULL);
                u8 = (F8)&7;
                LDRH_IMM8(gd, wback, fixedaddress+u8*2);
            }
            break;
        case 0xC6:
            INST_NAME("SHUFPD Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0);
            u8 = F8;
            if(v0==v1) {
                switch (u8&3) {
                    case 0b00: VMOVD(v0+1, v0); break;
                    case 0b01: VSWP(v0, v0+1);  break;
                    case 0b10:                  break;
                    case 0b11: VMOVD(v0, v0+1); break;
                }
            } else {
                switch(u8&1) {
                    case 0:                     break;
                    case 1: VMOVD(v0, v0+1);    break;
                }
                switch((u8>>1)&1) {
                    case 0: VMOVD(v0+1, v1);    break;
                    case 1: VMOVD(v0+1, v1+1);  break;
                }
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
            REV16(x1, gd);
            BFI(gd, x1, 0, 16);
            break;
        case 0xD0:
            INST_NAME("ADDSUBPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0);
            VSUB_F64(v0+0, v0+0, v1+0);
            VADD_F64(v0+1, v0+1, v1+1);
            break;
        case 0xD1:
            INST_NAME("PSRLW Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            v0 = fpu_get_scratch_quad(dyn);
            VQMOVN_S64(v0, q1);
            VQMOVN_S32(v0, v0);
            VQMOVN_S16(v0, v0);
            VDUPQ_8(v0, v0, 0); // only 8 bits are taken
            VNEGNQ_8(v0, v0);   // because we want SHR and not SHL
            VSHLQ_U16(q0, q0, v0);
            break;
        case 0xD2:
            INST_NAME("PSRLD Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            v0 = fpu_get_scratch_quad(dyn);
            VQMOVN_S64(v0, q1);
            VQMOVN_S32(v0, v0);
            VQMOVN_S16(v0, v0);
            VDUPQ_8(v0, v0, 0); // only 8 bits are taken
            VNEGNQ_8(v0, v0);   // because we want SHR and not SHL
            VSHLQ_U32(q0, q0, v0);
            break;
        case 0xD3:
            INST_NAME("PSRLQ Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            v0 = fpu_get_scratch_quad(dyn);
            VQMOVN_S64(v0, q1);
            VQMOVN_S32(v0, v0);
            VQMOVN_S16(v0, v0);
            VDUPQ_8(v0, v0, 0); // only 8 bits are taken
            VNEGNQ_8(v0, v0);   // because we want SHR and not SHL
            VSHLQ_U64(q0, q0, v0);
            break;
        case 0xD4:
            INST_NAME("PADDQ Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VADDQ_64(v0, v0, q0);
            break;
        case 0xD5:
            INST_NAME("PMULLW Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            VMULQ_16(q0, q0, q1);
            break;
        case 0xD6:
            INST_NAME("MOVQ Ex,Gx");
            nextop = F8;
            GETGX(q0, 0);
            if(MODREG) {
                q1 = sse_get_reg_empty(dyn, ninst, x1, nextop&7);
                VMOVD(q1, q0);
                VEOR(q1+1, q1+1, q1+1);
            } else {
                parity = getedparity(dyn, ninst, addr, nextop, 3);
                // can be unaligned sometimes
                if(parity) {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 1023, 3, 0, NULL);
                    VSTR_64(q0, ed, fixedaddress);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                    VMOVfrV_D(x2, x3, q0);
                    STR_IMM9(x2, ed, fixedaddress+0);
                    STR_IMM9(x3, ed, fixedaddress+4);
                }
                SMWRITE2();
            }
            break;
        case 0xD7:
            nextop = F8;
            if(MODREG) {
                INST_NAME("PMOVMSKB Gd, Ex");
                GETEX(q0, 0);
                gd = xEAX+((nextop&0x38)>>3);
                #if STEP == 3
                MOV32_(x1, &mask_shift8);
                #else
                MOV32_(x1, 0);
                #endif
                v0 = fpu_get_scratch_double(dyn);
                VLD1_8(v0, x1);     // load shift
                v1 = fpu_get_scratch_double(dyn);
                VMOV_8(v1, 0x80);   // load mask
                VANDD(v1, v1, q0+0);  // keep highest bit
                VSHL_U8(v1, v1, v0);// shift
                VPADD_8(v1, v1, v1);// accumulate the bits
                VPADD_8(v1, v1, v1);// ...
                VPADD_8(v1, v1, v1);// ...
                VMOVfrDx_U8(gd, v1, 0);
                // and now the high part
                VMOV_8(v1, 0x80);   // load mask
                VANDD(v1, v1, q0+1);  // keep highest bit
                VSHL_U8(v1, v1, v0);// shift
                VPADD_8(v1, v1, v1);// accumulate the bits
                VPADD_8(v1, v1, v1);// ...
                VPADD_8(v1, v1, v1);// ...
                VMOVfrDx_U8(x1, v1, 0);
                BFI(gd, x1, 8, 8);
            } else {
                DEFAULT;
            }
            break;
        case 0xD8:
            INST_NAME("PSUBUSB Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            VQSUBQ_U8(q0, q0, q1);
            break;
        case 0xD9:
            INST_NAME("PSUBUSW Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            VQSUBQ_U16(q0, q0, q1);
            break;
        case 0xDA:
            INST_NAME("PMINUB Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            VMINQ_U8(q0, q0, q1);
            break;
        case 0xDB:
            INST_NAME("PAND Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VANDQ(v0, v0, q0);
            break;
        case 0xDC:
            INST_NAME("PADDUSB Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            VQADDQ_U8(q0, q0, q1);
            break;
        case 0xDD:
            INST_NAME("PADDUSW Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            VQADDQ_U16(q0, q0, q1);
            break;
        case 0xDE:
            INST_NAME("PMAXUB Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            VMAXQ_U8(q0, q0, q1);
            break;
        case 0xDF:
            INST_NAME("PANDN Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VBICQ(v0, q0, v0);
            break;
        case 0xE0:
            INST_NAME("PAVGB Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VRHADDQ_U8(v0, v0, q0);
            break;
        case 0xE1:
            INST_NAME("PSRAW Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            v0 = fpu_get_scratch_quad(dyn);
            VQMOVN_S64(v0, q1);
            VQMOVN_S32(v0, v0);
            VQMOVN_S16(v0, v0);
            VDUPQ_8(v0, v0, 0); // only 8 bits are taken
            VNEGNQ_8(v0, v0);   // because we want SHR and not SHL
            VSHLQ_S16(q0, q0, v0);
            break;
        case 0xE2:
            INST_NAME("PSRAD Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            v0 = fpu_get_scratch_quad(dyn);
            VQMOVN_S64(v0, q1);
            VQMOVN_S32(v0, v0);
            VQMOVN_S16(v0, v0);
            VDUPQ_8(v0, v0, 0); // only 8 bits are taken
            VNEGNQ_8(v0, v0);   // because we want SHR and not SHL
            VSHLQ_S32(q0, q0, v0);
            break;
        case 0xE3:
            INST_NAME("PAVGW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VRHADDQ_U16(v0, v0, q0);
            break;
        case 0xE4:
            INST_NAME("PMULHUW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0);
            q0 = fpu_get_scratch_quad(dyn);
            VMULL_U32_U16(q0, v0, v1);
            VSHRN_32(v0, q0, 16);
            VMULL_U32_U16(q0, v0+1, v1+1);
            VSHRN_32(v0+1, q0, 16);
            break;
        case 0xE5:
            INST_NAME("PMULHW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0);
            q0 = fpu_get_scratch_quad(dyn);
            VMULL_S32_S16(q0, v0, v1);
            VSHRN_32(v0, q0, 16);
            VMULL_S32_S16(q0, v0+1, v1+1);
            VSHRN_32(v0+1, q0, 16);
            break;
        case 0xE6:
            INST_NAME("CVTTPD2DQ Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            if(q0<16) {
                VCVT_S32_F64(q0*2, q1);
                VCVT_S32_F64(q0*2+1, q1+1);
            } else {
                d0 = fpu_get_scratch_double(dyn);
                VCVT_S32_F64(d0*2, q1);
                VCVT_S32_F64(d0*2+1, q1+1);
                VMOVD(q0, d0);
            }
            VEOR(q0+1, q0+1, q0+1);
            break;
        case 0xE7:
            INST_NAME("MOVNTDQ Ex, Gx");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if(MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, nextop&7);
                VMOVQ(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x1, &fixedaddress, 0, 0, 0, NULL);
                VST1Q_64(v0, ed);
            }
            break;
        case 0xE8:
            INST_NAME("PSUBSB Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VQSUBQ_S8(v0, v0, q0);
            break;
        case 0xE9:
            INST_NAME("PSUBSW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VQSUBQ_S16(v0, v0, q0);
            break;
        case 0xEA:
            INST_NAME("PMINSW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VMINQ_S16(v0, v0, q0);
            break;
        case 0xEB:
            INST_NAME("POR Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VORRQ(v0, v0, q0);
            break;
        case 0xEC:
            INST_NAME("PADDSB Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VQADDQ_S8(v0, v0, q0);
            break;
        case 0xED:
            INST_NAME("PADDSW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VQADDQ_S16(v0, v0, q0);
            break;
        case 0xEE:
            INST_NAME("PMAXSW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VMAXQ_S16(v0, v0, q0);
            break;
        case 0xEF:
            INST_NAME("PXOR Gx,Ex");
            nextop = F8;
            gd = (nextop&0x38)>>3;
            if((nextop&0xC7)==(0xC0|gd)) {
                // special case for PXOR Gx, Gx
                q1 = q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            } else {
                q0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                GETEX(q1, 0);
            }
            VEORQ(q0, q0, q1);
            break;

        case 0xF1:
            INST_NAME("PSLLW Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            v0 = fpu_get_scratch_quad(dyn);
            VQMOVN_S64(v0, q1);
            VQMOVN_S32(v0, v0);
            VQMOVN_S16(v0, v0);
            VDUPQ_8(v0, v0, 0); // only 8 bits are taken
            VSHLQ_U16(q0, q0, v0);
            break;
        case 0xF2:
            INST_NAME("PSLLD Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            v0 = fpu_get_scratch_quad(dyn);
            VQMOVN_S64(v0, q1);
            VQMOVN_S32(v0, v0);
            VQMOVN_S16(v0, v0);
            VDUPQ_8(v0, v0, 0); // only 8 bits are taken
            VSHLQ_U32(q0, q0, v0);
            break;
        case 0xF3:
            INST_NAME("PSLLQ Gx,Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            v0 = fpu_get_scratch_quad(dyn);
            VQMOVN_S64(v0, q1);
            VQMOVN_S32(v0, v0);
            VQMOVN_S16(v0, v0);
            VDUPQ_8(v0, v0, 0); // only 8 bits are taken
            VSHLQ_U64(q0, q0, v0);
            break;
        case 0xF4:
            INST_NAME("PMULUDQ Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0);
            q0 = fpu_get_scratch_quad(dyn);
            VMOVQ(q0, v0);
            VTRN_32(q0, q0+1);  // transpose GX
            if(MODREG) {
                q1 = fpu_get_scratch_quad(dyn);
                VMOVQ(q1, v1);
            } else {
                q1 = v1;
            }
            VTRN_32(q1, q1+1);  // transpose EX
            VMULL_U64_U32(v0, q0, q1);
            break;
        case 0xF5:
            INST_NAME("PMADDWD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0);
            q1 = fpu_get_scratch_quad(dyn);
            VMULL_S32_S16(q1, v0, v1);
            VPADD_32(v0, q1, q1+1);
            VMULL_S32_S16(q1, v0+1, v1+1);
            VPADD_32(v0+1, q1, q1+1);
            break;
        case 0xF6:
            INST_NAME("PSADBW Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            VABDQ_U8(q0, q0, q1);   // needs VABDLQ in fact
            VPADDLQ_U8(q0, q0);
            VPADDLQ_U16(q0, q0);
            VPADDLQ_U32(q0, q0);
            break;
        case 0xF7:
            INST_NAME("MASKMOVDQU Gx, Ex");
            nextop = F8;
            GETGX(q0, 1);
            GETEX(q1, 0);
            v0 = fpu_get_scratch_quad(dyn);
            VLD1Q_8(v0, xEDI);  // hopefully it's always aligned?
            if(MODREG)
                v1 = fpu_get_scratch_quad(dyn); // need to preserve the register
            else
                v1 = q1;
            VCLTQ_0_8(v1, q1);  // get the mask
            VBICQ(v0, v0, v1);  // mask destination
            VANDQ(v1, q0, v1);  // mask source
            VORRQ(v1, v1, v0);  // combine
            VST1Q_8(v1, xEDI);  // put back
            break;
        case 0xF8:
            INST_NAME("PSUBB Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VSUBQ_8(v0, v0, q0);
            break;
        case 0xF9:
            INST_NAME("PSUBW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VSUBQ_16(v0, v0, q0);
            break;
        case 0xFA:
            INST_NAME("PSUBD Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VSUBQ_32(v0, v0, q0);
            break;
        case 0xFB:
            INST_NAME("PSUBQ Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VSUBQ_64(v0, v0, q0);
            break;
        case 0xFC:
            INST_NAME("PADDB Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VADDQ_8(v0, v0, q0);
            break;
        case 0xFD:
            INST_NAME("PADDW Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VADDQ_16(v0, v0, q0);
            break;
        case 0xFE:
            INST_NAME("PADDD Gx,Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0);
            VADDQ_32(v0, v0, q0);
            break;

        default:
            DEFAULT;
    }
    return addr;
}

