#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"


void Run660F(x86emu_t *emu)
{
    ++R_EIP;    // skip 0x0f
    uint8_t opcode = Fetch8(emu);
    uint8_t nextop;
    reg32_t *op1, *op2, *op3, *op4;
    reg32_t ea1, ea2, ea3, ea4;
    uint8_t tmp8u;
    int8_t tmp8s;
    uint16_t tmp16u;
    int16_t tmp16s;
    uint32_t tmp32u;
    int32_t tmp32s;
    uint64_t tmp64u;
    int64_t tmp64s;
    sse_regs_t *opx1, *opx2;
    sse_regs_t eax1;
    mmx_regs_t *opm1, *opm2;
    switch(opcode) {

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
        GetEw(emu, &op2, nextop);
        GetG(emu, &op1, nextop);
        , op1->word[0] = op2->word[0];
    )                               /* 0x40 -> 0x4F CMOVxx Gw,Ew */ // conditional move, no sign
        
    case 0x10:                      /* MOVUPD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        memcpy(opx1, opx2, 16); // unaligned...
        break;
    case 0x11:                      /* MOVUPD Ex, Gx */
        nextop = Fetch8(emu);
        GetEx(emu, &opx1, nextop);
        GetGx(emu, &opx2, nextop);
        memcpy(opx1, opx2, 16); // unaligned...
        break;

    case 0x14:                      /* UNPCKLPD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[1] = opx2->q[0];
        break;
    case 0x15:                      /* UNPCKHPD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] = opx1->q[1];
        opx1->q[1] = opx2->q[1];
        break;
    case 0x16:                      /* MOVHPD Gx, Ed */
        nextop = Fetch8(emu);
        GetEd(emu, &op2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[1] = *(uint64_t*)op2;
        break;
    case 0x17:                      /* MOVHPD Ed, Gx */
        nextop = Fetch8(emu);
        GetEd(emu, &op1, nextop);
        GetGx(emu, &opx2, nextop);
        *(uint64_t*)op1 = opx2->q[1];
        break;

    case 0x1F:                      /* NOP (multi-byte) */
        nextop = Fetch8(emu);
        GetEd(emu, &op1, nextop);
        break;
    
    case 0x28:                      /* MOVAPD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] = opx2->q[0];
        opx1->q[1] = opx2->q[1];
        break;
    case 0x29:                      /* MOVAPD Ex, Gx */
        nextop = Fetch8(emu);
        GetEx(emu, &opx1, nextop);
        GetGx(emu, &opx2, nextop);
        opx1->q[0] = opx2->q[0];
        opx1->q[1] = opx2->q[1];
        break;
    case 0x2A:                      /* CVTPI2PD Gx, Em */
        nextop = Fetch8(emu);
        GetEm(emu, &opm2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] = opm2->sd[0];
        opx1->d[1] = opm2->sd[1];
        break;


    case 0x2C:                      /* CVTTPD2PI Gm, Ex */
    case 0x2D:                      /* CVTPD2PI Gm, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGm(emu, &opm1, nextop);
        opm1->sd[0] = opx2->d[0];
        opm1->sd[1] = opx2->d[1];
        break;
    case 0x2E:                      /* UCOMISD Gx, Ex */
        // no special check...
    case 0x2F:                      /* COMISD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx1, nextop);
        GetGx(emu, &opx2, nextop);
        if(isnan(opx1->d[0]) || isnan(opx2->d[0])) {
            SET_FLAG(F_ZF); SET_FLAG(F_PF); SET_FLAG(F_CF);
        } else if(isgreater(opx2->d[0], opx1->d[0])) {
            CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
        } else if(isless(opx2->d[0], opx1->d[0])) {
            CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); SET_FLAG(F_CF);
        } else {
            SET_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
        }
        break;

    case 0x50:                      /* MOVMSKPD Gd, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetG(emu, &op1, nextop);
        op1->dword[0] = 0;
        for(int i=0; i<2; ++i)
            op1->dword[0] |= ((opx2->q[i]>>63)&1)<<i;
        break;
    case 0x51:                      /* SQRTPD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] = sqrt(opx2->d[0]);
        opx1->d[1] = sqrt(opx2->d[1]);
        break;

    case 0x54:                      /* ANDPD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] &= opx2->q[0];
        opx1->q[1] &= opx2->q[1];
        break;
    case 0x55:                      /* ANDNPD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] = (~opx1->q[0]) & opx2->q[0];
        opx1->q[1] = (~opx1->q[1]) & opx2->q[1];
        break;
    case 0x56:                      /* ORPD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] |= opx2->q[0];
        opx1->q[1] |= opx2->q[1];
        break;
    case 0x57:                      /* XORPD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] ^= opx2->q[0];
        opx1->q[1] ^= opx2->q[1];
        break;
    case 0x58:                      /* ADDPD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] += opx2->d[0];
        opx1->d[1] += opx2->d[1];
        break;
    case 0x59:                      /* MULPD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] *= opx2->d[0];
        opx1->d[1] *= opx2->d[1];
        break;
    case 0x5A:                      /* CVTPD2PS Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->f[0] = opx2->d[0];
        opx1->f[1] = opx2->d[1];
        opx1->q[1] = 0;
        break;
    case 0x5B:                      /* CVTPS2DQ Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->sd[0] = opx2->f[0];
        opx1->sd[1] = opx2->f[1];
        opx1->sd[2] = opx2->f[2];
        opx1->sd[3] = opx2->f[3];
        break;
    case 0x5C:                      /* SUBPD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] -= opx2->d[0];
        opx1->d[1] -= opx2->d[1];
        break;
    case 0x5D:                      /* MINPD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        if (isnan(opx1->d[0]) || isnan(opx2->d[0]) || isless(opx2->d[0], opx1->d[0]))
            opx1->d[0] = opx2->d[0];
        if (isnan(opx1->d[1]) || isnan(opx2->d[1]) || isless(opx2->d[1], opx1->d[1]))
            opx1->d[1] = opx2->d[1];
        break;
    case 0x5E:                      /* DIVPD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] /= opx2->d[0];
        opx1->d[1] /= opx2->d[1];
        break;
    case 0x5F:                      /* MAXPD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        if (isnan(opx1->d[0]) || isnan(opx2->d[0]) || isgreater(opx2->d[0], opx1->d[0]))
            opx1->d[0] = opx2->d[0];
        if (isnan(opx1->d[1]) || isnan(opx2->d[1]) || isgreater(opx2->d[1], opx1->d[1]))
            opx1->d[1] = opx2->d[1];
        break;

    case 0x60:  /* PUNPCKLBW Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        for(int i=7; i>0; --i)
            opx1->ub[2 * i] = opx1->ub[i];
        for(int i=0; i<8; ++i)
            opx1->ub[2 * i + 1] = opx2->ub[i];
        break;
    case 0x61:  /* PUNPCKLWD Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        for(int i=3; i>0; --i)
            opx1->uw[2 * i] = opx1->uw[i];
        for(int i=0; i<4; ++i)
            opx1->uw[2 * i + 1] = opx2->uw[i];
        break;
    case 0x62:  /* PUNPCKLDQ Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->ud[3] = opx2->ud[1];
        opx1->ud[2] = opx1->ud[1];
        opx1->ud[1] = opx2->ud[0];
        break;

    case 0x64:  /* PCMPGTB Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        for(int i=0; i<16; ++i)
            opx1->sb[i] = (opx1->sb[i]>opx2->sb[i])?0xFF:0x00;
        break;
    case 0x65:  /* PCMPGTW Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        for(int i=0; i<8; ++i)
            opx1->sw[i] = (opx1->sw[i]>opx2->sw[i])?0xFFFF:0x0000;
        break;
    case 0x66:  /* PCMPGTD Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        for(int i=0; i<4; ++i)
            opx1->sd[i] = (opx1->sd[i]>opx2->sd[i])?0xFFFFFFFF:0x00000000;
        break;
    case 0x67:  /* PACKUSWB */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        for(int i=0; i<4; ++i)
            opx1->ub[i] = (opx1->sw[i]<0)?0:((opx1->sw[i]>0xff)?0xff:opx1->sw[i]);
        for(int i=0; i<4; ++i)
            opx1->ub[4+i] = (opx2->sw[i]<0)?0:((opx2->sw[i]>0xff)?0xff:opx2->sw[i]);
        break;
    case 0x68:  /* PUNPCKHBW Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        for(int i=0; i<8; ++i)
            opx1->ub[2 * i] = opx1->ub[i + 8];
        for(int i=0; i<8; ++i)
            opx1->ub[2 * i + 1] = opx2->ub[i + 8];
        break;
    case 0x69:  /* PUNPCKHWD Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        for(int i=0; i<4; ++i)
            opx1->uw[2 * i] = opx1->uw[i + 4];
        for(int i=0; i<4; ++i)
            opx1->uw[2 * i + 1] = opx2->uw[i + 4];
        break;
    case 0x6A:  /* PUNPCKHDQ Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->uw[0] = opx1->uw[2];
        opx1->uw[2] = opx1->uw[3];
        opx1->uw[1] = opx2->uw[2];
        opx1->uw[3] = opx2->uw[3];
        break;

    case 0x6C:  /* PUNPCKLQDQ Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[1] = opx2->q[0];
        break;
    case 0x6D:  /* PUNPCKHQDQ Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] = opx1->q[1];
        opx1->q[1] = opx2->q[1];
        break;
    case 0x6E:  /* MOVD Gx, Ed */
        nextop = Fetch8(emu);
        GetEd(emu, &op2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] = op2->dword[0];
        opx1->q[1] = 0;
        break;
    case 0x6F:  /* MOVDQA Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] = opx2->q[0];
        opx1->q[1] = opx2->q[1];
        break;
    case 0x70:  /* PSHUFD Gx,Ex,Ib */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        tmp8u = Fetch8(emu);
        if(opx1!=opx2)
            for (int i=0; i<4; ++i)
                opx1->ud[i] = opx2->ud[(tmp8u>>(i*2))&3];
        else {
            for (int i=0; i<4; ++i)
                eax1.ud[i] = opx2->ud[(tmp8u>>(i*2))&3];
            memcpy(opx1, &eax1, sizeof(eax1));
        }
        break;
    case 0x71:  /* GRP */
        nextop = Fetch8(emu);
        GetEx(emu, &opx1, nextop);
        switch((nextop>>3)&7) {
            case 2:                 /* PSRLW Gx, Ib */
                tmp8u = Fetch8(emu);
                if(tmp8u>15)
                    {opx1->q[0] = opx1->q[1] = 0;}
                else
                    for (int i=0; i<8; ++i) opx1->uw[i] >>= tmp8u;
                break;
            case 4:                 /* PSRAW Gx, Ib */
                tmp8u = Fetch8(emu);
                for (int i=0; i<8; ++i) opx1->sw[i] >>= tmp8u;
                break;
            case 6:                 /* PSLLW Gx, Ib */
                tmp8u = Fetch8(emu);
                if(tmp8u>15)
                    {opx1->q[0] = opx1->q[1] = 0;}
                else
                    for (int i=0; i<8; ++i) opx1->uw[i] <<= tmp8u;
                break;
            default:
                UnimpOpcode(emu);
        }
        break;
    case 0x72:  /* GRP */
        nextop = Fetch8(emu);
        GetEx(emu, &opx1, nextop);
        switch((nextop>>3)&7) {
            case 2:                 /* PSRLD Ex, Ib */
                tmp8u = Fetch8(emu);
                if(tmp8u>31)
                    {opx1->q[0] = opx1->q[1] = 0;}
                else
                    for (int i=0; i<4; ++i) opx1->ud[i] >>= tmp8u;
                break;
            case 4:                 /* PSRAD Ex, Ib */
                tmp8u = Fetch8(emu);
                for (int i=0; i<4; ++i) opx1->sd[i] >>= tmp8u;
                break;
            case 6:                 /* PSLLD Ex, Ib */
                tmp8u = Fetch8(emu);
                if(tmp8u>31)
                    {opx1->q[0] = opx1->q[1] = 0;}
                else
                    for (int i=0; i<4; ++i) opx1->ud[i] <<= tmp8u;
                break;
            default:
                UnimpOpcode(emu);
        }
        break;
    case 0x73:  /* GRP */
        nextop = Fetch8(emu);
        GetEx(emu, &opx1, nextop);
        switch((nextop>>3)&7) {
            case 2:                 /* PSRLQ Ex, Ib */
                tmp8u = Fetch8(emu);
                if(tmp8u>63)
                    {opx1->q[0] = opx1->q[1] = 0;}
                else
                    {opx1->q[0] >>= tmp8u; opx1->q[1] >>= tmp8u;}
                break;
            case 3:                 /* PSRLDQ Ex, Ib */
                tmp8u = Fetch8(emu);
                if(tmp8u>15)
                    {opx1->q[0] = opx1->q[1] = 0;}
                else {
                    for (int i=tmp8u; i<16; ++i)
                        opx1->ub[i-tmp8u] = opx1->ub[i];
                    for (int i=16-tmp8u; i<16; ++i)
                        opx1->ub[i] = 0;
                }
                break;
            case 6:                 /* PSLLQ Ex, Ib */
                tmp8u = Fetch8(emu);
                if(tmp8u>63)
                    {opx1->q[0] = opx1->q[1] = 0;}
                else
                    {opx1->q[0] <<= tmp8u; opx1->q[1] <<= tmp8u;}
                break;
            case 7:                 /* PSLLDQ Ex, Ib */
                tmp8u = Fetch8(emu);
                if(tmp8u>15)
                    {opx1->q[0] = opx1->q[1] = 0;}
                else {
                    for (int i=16-tmp8u; i>=0; --i)
                        opx1->ub[i+tmp8u] = opx1->ub[i];
                    for (int i=0; i<tmp8u; ++i)
                        opx1->ub[i] = 0;
                }
                break;
            default:
                UnimpOpcode(emu);
        }
        break;
    case 0x74:  /* PCMPEQB Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        for (int i=0; i<16; ++i)
            opx1->ub[i] = (opx1->ub[i]==opx2->ub[i])?0xff:0;
        break;
    case 0x75:  /* PCMPEQW Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        for (int i=0; i<8; ++i)
            opx1->uw[i] = (opx1->uw[i]==opx2->uw[i])?0xffff:0;
        break;
    case 0x76:  /* PCMPEQD Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        for (int i=0; i<4; ++i)
            opx1->ud[i] = (opx1->ud[i]==opx2->ud[i])?0xffffffff:0;
        break;

    case 0x7E:  /* MOVD Ed, Gx */
        nextop = Fetch8(emu);
        GetEd(emu, &op2, nextop);
        GetGx(emu, &opx1, nextop);
        op2->dword[0] = opx1->ud[0];
        break;
    case 0x7F:  /* MOVDQA Ex,Gx */
        nextop = Fetch8(emu);
        GetEx(emu, &opx1, nextop);
        GetGx(emu, &opx2, nextop);
        opx1->q[0] = opx2->q[0];
        opx1->q[1] = opx2->q[1];
        break;

    case 0xA3:                      /* BT Ew,Gw */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, nextop);
        GetG(emu, &op2, nextop);
        if(op1->word[0] & (1<<(op2->word[0]&15)))
            SET_FLAG(F_CF);
        else
            CLEAR_FLAG(F_CF);
        break;
    case 0xA4:                      /* SHLD Ew,Gw,Ib */
    case 0xA5:                      /* SHLD Ew,Gw,CL */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, nextop);
        GetG(emu, &op2, nextop);
        tmp8u = (opcode==0xA4)?Fetch8(emu):R_CL;
        op1->word[0] = shld16(emu, op1->word[0], op2->word[0], tmp8u);
        break;

    case 0xAB:                      /* BTS Ew,Gw */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, nextop);
        GetG(emu, &op2, nextop);
        if(op1->word[0] & (1<<(op2->word[0]&15)))
            SET_FLAG(F_CF);
        else {
            op1->word[0] |= (1<<(op2->word[0]&15));
            CLEAR_FLAG(F_CF);
        }
        break;
    case 0xAC:                      /* SHRD Ew,Gw,Ib */
    case 0xAD:                      /* SHRD Ew,Gw,CL */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, nextop);
        GetG(emu, &op2, nextop);
        tmp8u = (opcode==0xAC)?Fetch8(emu):R_CL;
        op1->word[0] = shrd16(emu, op1->word[0], op2->word[0], tmp8u);
        break;

    case 0xAF:                      /* IMUL Gw,Ew */
        nextop = Fetch8(emu);
        GetEw(emu, &op2, nextop);
        GetG(emu, &op1, nextop);
        op1->word[0] = imul16(emu, op1->word[0], op2->word[0]);
        break;

    case 0xB1:                      /* CMPXCHG Ew,Gw */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, nextop);
        GetG(emu, &op2, nextop);
        if(R_AX == op1->word[0]) {
            SET_FLAG(F_ZF);
            op1->word[0] = op2->word[0];
        } else {
            CLEAR_FLAG(F_ZF);
            R_AX = op1->word[0];
        }
        break;

    case 0xB3:                      /* BTR Ew,Gw */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, nextop);
        GetG(emu, &op2, nextop);
        if(op1->word[0] & (1<<(op2->word[0]&15))) {
            SET_FLAG(F_CF);
            op1->word[0] ^= (1<<(op2->word[0]&15));
        } else
            CLEAR_FLAG(F_CF);
        break;

    case 0xB6:                      /* MOVZX Gw,Eb */
        nextop = Fetch8(emu);
        GetEb(emu, &op2, nextop);
        GetG(emu, &op1, nextop);
        op1->word[0] = op2->byte[0];
        break;

    case 0xBB:                      /* BTC Ew,Gw */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, nextop);
        GetG(emu, &op2, nextop);
        if(op1->word[0] & (1<<(op2->word[0]&15)))
            SET_FLAG(F_CF);
        else
            CLEAR_FLAG(F_CF);
        op1->word[0] ^= (1<<(op2->word[0]&15));
        break;
    case 0xBC:                      /* BSF Ew,Gw */
        nextop = Fetch8(emu);
        GetEd(emu, &op1, nextop);
        GetG(emu, &op2, nextop);
        tmp16u = op1->word[0];
        if(tmp16u) {
            CLEAR_FLAG(F_ZF);
            tmp8u = 0;
            while(!(tmp16u&(1<<tmp8u))) ++tmp8u;
            op2->word[0] = tmp8u;
        } else {
            SET_FLAG(F_ZF);
        }
        break;
    case 0xBD:                      /* BSR Ew,Gw */
        nextop = Fetch8(emu);
        GetEd(emu, &op1, nextop);
        GetG(emu, &op2, nextop);
        tmp16u = op1->word[0];
        if(tmp16u) {
            CLEAR_FLAG(F_ZF);
            tmp8u = 15;
            while(!(tmp16u&(1<<tmp8u))) --tmp8u;
            op2->word[0] = tmp8u;
        } else {
            SET_FLAG(F_ZF);
        }
        break;
    case 0xBE:                      /* MOVSX Gw,Eb */
        nextop = Fetch8(emu);
        GetEb(emu, &op2, nextop);
        GetG(emu, &op1, nextop);
        op1->sword[0] = (int8_t)op2->byte[0];
        break;

    case 0xC1:                      /* XADD Gw,Ew */ // Xchange and Add
        nextop = Fetch8(emu);
        GetEw(emu, &op1, nextop);
        GetG(emu, &op2, nextop);
        tmp16u = add16(emu, op1->word[0], op2->word[0]);
        op2->word[0] = op1->word[0];
        op1->word[0] = tmp16u;
        break;
    case 0xC2:                      /* CMPPD Gx, Ex, Ib */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        tmp8u = Fetch8(emu);
        for(int i=0; i<2; ++i) {
            tmp8s = 0;
            switch(tmp8u&7) {
                case 0: tmp8s=(opx1->d[i] == opx2->d[i]); break;
                case 1: tmp8s=isless(opx1->d[i], opx2->d[i]); break;
                case 2: tmp8s=islessequal(opx1->d[i], opx2->d[i]); break;
                case 3: tmp8s=isnan(opx1->d[i]) || isnan(opx2->d[i]); break;
                case 4: tmp8s=(opx1->d[i] != opx2->d[i]); break;
                case 5: tmp8s=isgreaterequal(opx1->d[i], opx2->d[i]); break;
                case 6: tmp8s=isgreater(opx1->d[i], opx2->d[i]); break;
                case 7: tmp8s=!isnan(opx1->d[i]) && !isnan(opx2->d[i]); break;
            }
            if(tmp8s)
                opx1->q[i] = 0xffffffffffffffffLL;
            else
                opx1->q[i] = 0;
        }
        break;

    case 0xC4:  /* PINSRW Gx,Ed,Ib */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, nextop);
        GetGx(emu, &opx2, nextop);
        tmp8u = Fetch8(emu);
        opx2->uw[tmp8u&7] = op1->word[0];
        break;
    case 0xC5:  /* PEXTRW Gd,Ex,Ib */
        nextop = Fetch8(emu);
        GetEx(emu, &opx1, nextop);
        GetG(emu, &op2, nextop);
        tmp8u = Fetch8(emu);
        op2->dword[0] = opx1->uw[tmp8u&7];
        break;
    case 0xC6:  /* SHUFPD Gx, Ex, Ib */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        tmp8u = Fetch8(emu);
        eax1.q[0] = opx1->q[tmp8u&1];
        eax1.q[1] = opx2->q[(tmp8u>>1)&1];
        opx1->q[0] = eax1.q[0];
        opx1->q[1] = eax1.q[1];
        break;

    case 0xD1:  /* PSRLW Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        if(opx2->q[0]>15)
            {opx1->q[0] = opx1->q[1] = 0;}
        else 
            {tmp8u=opx2->q[0]; for (int i=0; i<8; ++i) opx1->uw[i] >>= tmp8u;}
        break;
    case 0xD2:  /* PSRLD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        if(opx2->q[0]>31)
            {opx1->q[0] = opx1->q[1] = 0;}
        else 
            {tmp8u=opx2->q[0]; for (int i=0; i<4; ++i) opx1->ud[i] >>= tmp8u;}
        break;
    case 0xD3:  /* PSRLQ Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        if(opx2->q[0]>63)
            {opx1->q[0] = opx1->q[1] = 0;}
        else 
            {tmp8u=opx2->q[0]; for (int i=0; i<2; ++i) opx1->q[i] >>= tmp8u;}
        break;
    case 0xD4:  /* PADDQ Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] += opx2->q[0];
        opx1->q[1] += opx2->q[1];
        break;

    case 0xD6:  /* MOVQ Ex,Gx */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx2->q[0] = opx1->q[0];
        if((nextop&0xc7)>=0xc0 && (nextop&0xc7)<=0xc7)
            opx2->q[1] = 0;
        break;

    case 0xDB:  /* PAND Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] &= opx2->q[0];
        opx1->q[1] &= opx2->q[1];
        break;

    case 0xDF:  /* PANDN Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] = (~opx1->q[0]) & opx2->q[0];
        opx1->q[1] = (~opx1->q[1]) & opx2->q[1];
        break;

    case 0xE1:  /* PSRAW Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        tmp8u=opx2->q[0];
        for (int i=0; i<8; ++i) opx1->sw[i] >>= tmp8u;
        break;
    case 0xE2:  /* PSRAD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        tmp8u=opx2->q[0]; for (int i=0; i<4; ++i) opx1->sd[i] >>= tmp8u;
        break;

    case 0xE6:  /* CVTTPD2DQ Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->sd[0] = opx2->d[0];
        opx1->sd[1] = opx2->d[1];
        opx1->q[1] = 0;
        break;

    case 0xEB:  /* POR Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->ud[0] |= opx2->ud[0];
        opx1->ud[1] |= opx2->ud[1];
        opx1->ud[2] |= opx2->ud[2];
        opx1->ud[3] |= opx2->ud[3];
        break;
    case 0xEC:  /* PADDSB Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        for(int i=0; i<16; ++i)
            opx1->sb[i] += opx2->sb[i];
        break;
    case 0xED:  /* PADDSW Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        for(int i=0; i<8; ++i)
            opx1->sw[i] += opx2->sw[i];
        break;
    case 0xEE:  /* PMAXSW Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        for(int i=0; i<8; ++i)
            opx1->sw[i] = (opx1->sw[i]>opx2->sw[i])?opx1->sw[i]:opx2->sw[i];
        break;
    case 0xEF:  /* PXOR Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->ud[0] ^= opx2->ud[0];
        opx1->ud[1] ^= opx2->ud[1];
        opx1->ud[2] ^= opx2->ud[2];
        opx1->ud[3] ^= opx2->ud[3];
        break;

    case 0xF1:  /* PSLLW Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        if(opx2->q[0]>15)
            {opx1->q[0] = opx1->q[1] = 0;}
        else 
            {tmp8u=opx2->q[0]; for (int i=0; i<8; ++i) opx1->uw[i] <<= tmp8u;}
        break;
    case 0xF2:  /* PSLLD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        if(opx2->q[0]>31)
            {opx1->q[0] = opx1->q[1] = 0;}
        else 
            {tmp8u=opx2->q[0]; for (int i=0; i<4; ++i) opx1->ud[i] <<= tmp8u;}
        break;
    case 0xF3:  /* PSLLQ Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        if(opx2->q[0]>63)
            {opx1->q[0] = opx1->q[1] = 0;}
        else 
            {tmp8u=opx2->q[0]; for (int i=0; i<2; ++i) opx1->q[i] <<= tmp8u;}
        break;
    case 0xF4:  /* PMULUDQ Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[1] = (uint64_t)opx2->ud[2]*opx1->ud[2];
        opx1->q[0] = (uint64_t)opx2->ud[0]*opx1->ud[0];
        break;

    case 0xFA:  /* PSUBD Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->ud[0] -= opx2->ud[0];
        opx1->ud[1] -= opx2->ud[1];
        opx1->ud[2] -= opx2->ud[2];
        opx1->ud[3] -= opx2->ud[3];
        break;
    case 0xFB:  /* PSUBQ Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] -= opx2->q[0];
        opx1->q[1] -= opx2->q[1];
        break;
    case 0xFC:  /* PADDB Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        for(int i=0; i<16; ++i)
            opx1->ub[i] += opx2->ub[i];
        break;

    case 0xFE:  /* PADDD Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->ud[0] += opx2->ud[0];
        opx1->ud[1] += opx2->ud[1];
        opx1->ud[2] += opx2->ud[2];
        opx1->ud[3] += opx2->ud[3];
        break;


    default:
        UnimpOpcode(emu);
    }
}

void RunF20F(x86emu_t *emu)
{
    // ref opcode https://www.felixcloutier.com/x86/
    uint8_t opcode = Fetch8(emu);
    uint8_t nextop;
    reg32_t *op1, *op2, *op3, *op4;
    reg32_t ea1, ea2, ea3, ea4;
    uint8_t tmp8u;
    int8_t tmp8s;
    uint16_t tmp16u;
    int16_t tmp16s;
    uint32_t tmp32u;
    int32_t tmp32s;
    uint64_t tmp64u;
    int64_t tmp64s;
    sse_regs_t *opx1, *opx2;
    sse_regs_t eax1;
    mmx_regs_t *opm1;
    switch(opcode) {

    case 0x10:  /* MOVSD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] = opx2->q[0];
        if(!(opx2>=emu->xmm && opx2<=(emu->xmm+7))) {
            // op2 is not a register
            opx1->q[1] = 0;
        }
        break;
    case 0x11:  /* MOVSQ Ex, Gx */
        nextop = Fetch8(emu);
        GetEx(emu, &opx1, nextop);
        GetGx(emu, &opx2, nextop);
        opx1->q[0] = opx2->q[0];
        break;

    case 0x2A:  /* CVTSI2SD Gx, Ed */
        nextop = Fetch8(emu);
        GetEd(emu, &op2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] = op2->sdword[0];
        break;

    case 0x2C:  /* CVTTSD2SI Gd, Ex */
    case 0x2D:  /* CVTSD2SI Gd, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetG(emu, &op1, nextop);
        op1->sdword[0] = opx2->d[0];
        break;

    case 0x51:  /* SQRTSD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] = sqrt(opx2->d[0]);
        break;

    case 0x58:  /* ADDSD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] += opx2->d[0];
        break;
    case 0x59:  /* MULSD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] *= opx2->d[0];
        break;

    case 0x5A:  /* CVTSD2SS Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->f[0] = opx2->d[0];
        break;

    case 0x5C:  /* SUBSD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] -= opx2->d[0];
        break;
    case 0x5D:  /* MINSD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        if (isnan(opx1->d[0]) || isnan(opx2->d[0]) || isless(opx2->d[0], opx1->d[0]))
            opx1->d[0] = opx2->d[0];
        break;
    case 0x5E:  /* DIVSD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] /= opx2->d[0];
        break;
    case 0x5F:  /* MAXSD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        if (isnan(opx1->d[0]) || isnan(opx2->d[0]) || isgreater(opx2->d[0], opx1->d[0]))
            opx1->d[0] = opx2->d[0];
        break;

    case 0x70:  /* PSHUFLW Gx, Ex, Ib */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        tmp8u = Fetch8(emu);
        if(opx1==opx2) {
            for (int i=0; i<4; ++i)
                eax1.uw[i] = opx2->uw[(tmp8u>>(i*2))&3];
            opx1->q[0] = eax1.q[0];
        } else {
            for (int i=0; i<4; ++i)
                opx1->uw[i] = opx2->uw[(tmp8u>>(i*2))&3];
            opx1->q[1] = opx2->q[1];
        }
        break;

    case 0xC2:  /* CMPSD Gx, Ex, Ib */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        tmp8u = Fetch8(emu);
        tmp8s = 0;
        switch(tmp8u&7) {
            case 0: tmp8s=(opx1->d[0] == opx2->d[0]); break;
            case 1: tmp8s=isless(opx1->d[0], opx2->d[0]); break;
            case 2: tmp8s=islessequal(opx1->d[0], opx2->d[0]); break;
            case 3: tmp8s=isnan(opx1->d[0]) || isnan(opx2->d[0]); break;
            case 4: tmp8s=(opx1->d[0] != opx2->d[0]); break;
            case 5: tmp8s=isgreaterequal(opx1->d[0], opx2->d[0]); break;
            case 6: tmp8s=isgreater(opx1->d[0], opx2->d[0]); break;
            case 7: tmp8s=!isnan(opx1->d[0]) && !isnan(opx2->d[0]); break;
        }
        if(tmp8s)
            opx1->q[0] = 0xffffffffffffffffLL;
        else
            opx1->q[0] = 0;
        break;

    case 0xD6:  /* MOVDQ2Q Gm, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGm(emu, &opm1, nextop);
        opm1->q = opx2->q[0];
        break;

    case 0xE6:  /* CVTPD2DQ Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->sd[0] = opx2->d[0];
        opx1->sd[1] = opx2->d[1];
        opx1->q[1] = 0;
        break;

    default:
        UnimpOpcode(emu);
    }
}

void RunF30F(x86emu_t *emu)
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
    int32_t tmp32s;
    uint64_t tmp64u;
    int64_t tmp64s;
    sse_regs_t *opx1, *opx2;
    sse_regs_t eax1;
    mmx_regs_t *opm1, *opm2;
    switch(opcode) {

    case 0x10:  /* MOVSS Gx Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->ud[0] = opx2->ud[0];
        if(!(opx2>=emu->xmm && opx2<=(emu->xmm+7))) {
            // op2 is not a register
            opx1->ud[1] = opx1->ud[2] = opx1->ud[3] = 0;
        }
        break;
    case 0x11:  /* MOVSS Ex Gx */
        nextop = Fetch8(emu);
        GetEx(emu, &opx1, nextop);
        GetGx(emu, &opx2, nextop);
        opx1->ud[0] = opx2->ud[0];
        break;

    case 0x2A:  /* CVTSI2SS Gx, Ed */
        nextop = Fetch8(emu);
        GetEd(emu, &op2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->f[0] = op2->sdword[0];
        break;

    case 0x2C:  /* CVTTSS2SI Gd, Ex */
    case 0x2D:  /* CVTSS2SI Gd, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetG(emu, &op1, nextop);
        op1->sdword[0] = opx2->f[0];
        break;

    case 0x51:  /* SQRTSS Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->f[0] = sqrtf(opx2->f[0]);
        break;

    case 0x58:  /* ADDSS Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->f[0] += opx2->f[0];
        break;
    case 0x59:  /* MULSS Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->f[0] *= opx2->f[0];
        break;
    case 0x5A:  /* CVTSS2SD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] = opx2->f[0];
        break;
    case 0x5B:  /* CVTTPS2DQ Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->sd[0] = opx2->f[0];
        opx1->sd[1] = opx2->f[1];
        opx1->sd[2] = opx2->f[2];
        opx1->sd[3] = opx2->f[3];
        break;

    case 0x5C:  /* SUBSS Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->f[0] -= opx2->f[0];
        break;
    case 0x5D:  /* MINSS Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        if(isnan(opx1->f[0]) || isnan(opx2->f[0]) || isless(opx2->f[0], opx1->f[0]))
            opx1->f[0] = opx2->f[0];
        break;
    case 0x5E:  /* DIVSS Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->f[0] /= opx2->f[0];
        break;
    case 0x5F:  /* MAXSS Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        if (isnan(opx1->f[0]) || isnan(opx2->f[0]) || isgreater(opx2->f[0], opx1->f[0]))
            opx1->f[0] = opx2->f[0];
        break;

    case 0x6F:  /* MOVDQU Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx1, nextop);
        GetGx(emu, &opx2, nextop);
        memcpy(opx2, opx1, 16);
        break;
    case 0x70:  /* PSHUFHW Gx, Ex, Ib */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        tmp8u = Fetch8(emu);
        if(opx1==opx2) {
            for (int i=0; i<4; ++i)
                eax1.uw[4+i] = opx2->uw[4+((tmp8u>>(i*2))&3)];
            opx1->q[1] = eax1.q[1];
        } else {
            for (int i=0; i<4; ++i)
                opx1->uw[4+i] = opx2->uw[4+((tmp8u>>(i*2))&3)];
            opx1->q[0] = opx2->q[0];
        }
        break;

    case 0x7E:  /* MOVQ Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx1, nextop);
        GetGx(emu, &opx2, nextop);
        opx2->q[0] = opx1->q[0];
        opx2->q[1] = 0;
        break;
    case 0x7F:  /* MOVDQU Ex, Gx */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx2->q[0] = opx1->q[0];
        opx2->q[1] = opx1->q[1];
        break;

    case 0xC2:  /* CMPSS Gx, Ex, Ib */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        tmp8u = Fetch8(emu);
        tmp8s = 0;
        switch(tmp8u&7) {
            case 0: tmp8s=(opx1->f[0] == opx2->f[0]); break;
            case 1: tmp8s=isless(opx1->f[0], opx2->f[0]); break;
            case 2: tmp8s=islessequal(opx1->f[0], opx2->f[0]); break;
            case 3: tmp8s=isnan(opx1->f[0]) || isnan(opx2->f[0]); break;
            case 4: tmp8s=(opx1->f[0] != opx2->f[0]); break;
            case 5: tmp8s=isgreaterequal(opx1->f[0], opx2->f[0]); break;
            case 6: tmp8s=isgreater(opx1->f[0], opx2->f[0]); break;
            case 7: tmp8s=!isnan(opx1->f[0]) && !isnan(opx2->f[0]); break;
        }
        if(tmp8s)
            opx1->ud[0] = 0xffffffff;
        else
            opx1->ud[0] = 0;
        break;

    case 0xD6:  /* MOVQ2DQ Gx, Em */
        nextop = Fetch8(emu);
        GetEm(emu, &opm2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] = opm2->q;
        opx1->q[1] = 0;
        break;

    case 0xE6:  /* CVTDQ2PD Gx, Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[1] = opx2->sd[1];
        opx1->d[0] = opx2->sd[0];
        break;

    default:
        UnimpOpcode(emu);
    }
}