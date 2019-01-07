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
        GetEw(emu, &op2, &ea2, nextop);
        GetG(emu, &op1, nextop);
        , op1->word[0] = op2->word[0];
    )                               /* 0x40 -> 0x4F CMOVxx Gw,Ew */ // conditional move, no sign
        
    case 0x60:  /* PUNPCKLBW Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, &eax1, nextop);
        GetGx(emu, &opx1, nextop);
        for(int i=7; i>0; --i)
            opx1->ub.b[2 * i] = opx1->ub.b[i];
        for(int i=0; i<8; ++i)
            opx1->ub.b[2 * i + 1] = opx2->ub.b[i];
        break;
    case 0x61:  /* PUNPCKLWD Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, &eax1, nextop);
        GetGx(emu, &opx1, nextop);
        for(int i=3; i>0; --i)
            opx1->uw.w[2 * i] = opx1->uw.w[i];
        for(int i=0; i<4; ++i)
            opx1->uw.w[2 * i + 1] = opx2->uw.w[i];
        break;
    case 0x62:  /* PUNPCKLDQ Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, &eax1, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->ud.d[2] = opx1->ud.d[1];
        opx1->ud.d[1] = opx2->ud.d[0];
        opx1->ud.d[3] = opx2->ud.d[1];
        break;

    case 0x68:  /* PUNPCKHBW Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, &eax1, nextop);
        GetGx(emu, &opx1, nextop);
        for(int i=0; i<8; ++i)
            opx1->ub.b[2 * i] = opx1->ub.b[i + 8];
        for(int i=0; i<8; ++i)
            opx1->ub.b[2 * i + 1] = opx2->ub.b[i + 8];
        break;
    case 0x69:  /* PUNPCKHWD Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, &eax1, nextop);
        GetGx(emu, &opx1, nextop);
        for(int i=0; i<4; ++i)
            opx1->uw.w[2 * i] = opx1->uw.w[i + 4];
        for(int i=0; i<4; ++i)
            opx1->uw.w[2 * i + 1] = opx2->uw.w[i + 4];
        break;
    case 0x6A:  /* PUNPCKHDQ Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, &eax1, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->uw.w[0] = opx1->uw.w[2];
        opx1->uw.w[2] = opx1->uw.w[3];
        opx1->uw.w[1] = opx2->uw.w[2];
        opx1->uw.w[3] = opx2->uw.w[3];
        break;

    case 0x6C:  /* PUNPCKLQDQ Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, &eax1, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[1] = opx2->q[0];
        break;
    case 0x6D:  /* PUNPCKHQDQ Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, &eax1, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] = opx1->q[1];
        opx1->q[1] = opx2->q[1];
        break;

    case 0x6F:  /* MOVDQA Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, &eax1, nextop);
        GetGx(emu, &opx1, nextop);
        memcpy(opx1, opx2, sizeof(sse_regs_t));
        break;

    case 0x74:  /* PCMPEQB Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, &eax1, nextop);
        GetGx(emu, &opx1, nextop);
        for (int i=0; i<16; ++i)
            opx1->ub.b[i] = (opx1->ub.b[i]==opx2->ub.b[i])?0xff:0;
        break;
    case 0x75:  /* PCMPEQW Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, &eax1, nextop);
        GetGx(emu, &opx1, nextop);
        for (int i=0; i<8; ++i)
            opx1->uw.w[i] = (opx1->uw.w[i]==opx2->uw.w[i])?0xffff:0;
        break;
    case 0x76:  /* PCMPEQD Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, &eax1, nextop);
        GetGx(emu, &opx1, nextop);
        for (int i=0; i<4; ++i)
            opx1->ud.d[i] = (opx1->ud.d[i]==opx2->ud.d[i])?0xffffffff:0;
        break;

    case 0xEF:  /* PXOR Gx,Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, &eax1, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->ud.d[0] ^= opx2->ud.d[0];
        opx1->ud.d[1] ^= opx2->ud.d[1];
        opx1->ud.d[2] ^= opx2->ud.d[2];
        opx1->ud.d[3] ^= opx2->ud.d[3];
        break;

    case 0xA3:                      /* BT Ew,Gw */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, &ea1, nextop);
        GetG(emu, &op2, nextop);
        CLEAR_FLAG(F_CF);
        if(op1->word[0] & (1<<(op2->word[0]&15)))
            SET_FLAG(F_CF);
        break;
    case 0xA4:                      /* SHLD Ew,Gw,Ib */
    case 0xA5:                      /* SHLD Ew,Gw,CL */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, &ea1, nextop);
        GetG(emu, &op2, nextop);
        tmp8u = (opcode==0xA4)?Fetch8(emu):R_CL;
        op1->word[0] = shld16(emu, op1->word[0], op2->word[0], tmp8u);
        break;

    case 0xAB:                      /* BTS Ew,Gw */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, &ea1, nextop);
        GetG(emu, &op2, nextop);
        CLEAR_FLAG(F_CF);
        if(op1->word[0] & (1<<(op2->word[0]&15)))
            SET_FLAG(F_CF);
        else
            op1->word[0] |= (1<<(op2->word[0]&15));
        break;
    case 0xAC:                      /* SHRD Ew,Gw,Ib */
    case 0xAD:                      /* SHRD Ew,Gw,CL */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, &ea1, nextop);
        GetG(emu, &op2, nextop);
        tmp8u = (opcode==0xAC)?Fetch8(emu):R_CL;
        op1->word[0] = shrd16(emu, op1->word[0], op2->word[0], tmp8u);
        break;

    case 0xAF:                      /* IMUL Gw,Ew */
        nextop = Fetch8(emu);
        GetEw(emu, &op2, &ea2, nextop);
        GetG(emu, &op1, nextop);
        op1->word[0] = imul16(emu, op1->word[0], op2->word[0]);
        break;

    case 0xB3:                      /* BTR Ew,Gw */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, &ea1, nextop);
        GetG(emu, &op2, nextop);
        CLEAR_FLAG(F_CF);
        if(op1->word[0] & (1<<(op2->word[0]&15))) {
            SET_FLAG(F_CF);
            op1->word[0] ^= (1<<(op2->word[0]&15));
        }
        break;

    case 0xBB:                      /* BTC Ew,Gw */
        nextop = Fetch8(emu);
        GetEw(emu, &op1, &ea1, nextop);
        GetG(emu, &op2, nextop);
        CLEAR_FLAG(F_CF);
        if(op1->word[0] & (1<<(op2->word[0]&15)))
            SET_FLAG(F_CF);
        op1->word[0] ^= (1<<(op2->word[0]&15));
        break;
    case 0xBC:                      /* BSF Ew,Gw */
        nextop = Fetch8(emu);
        GetEd(emu, &op1, &ea1, nextop);
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
        GetEd(emu, &op1, &ea1, nextop);
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

    default:
        UnimpOpcode(emu);
    }
}

void RunF30F(x86emu_t *emu)
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
    switch(opcode) {

    case 0x10:  /* MOVSS Gx Ex */
        nextop = Fetch8(emu);
        GetEx(emu, &opx2, &eax1, nextop);
        GetGx(emu, &opx1, nextop);
        memcpy(opx1, opx2, sizeof(uint32_t));
        if(!(opx2>=emu->xmm && opx2<=(emu->xmm+7))) {
            // op2 is not a register
            opx1->ud.d[1] = opx1->ud.d[2] = opx1->ud.d[3] = 0;
        }
        break;
    case 0x11:  /* MOVSS Ex Gx */
        nextop = Fetch8(emu);
        GetEx(emu, &opx1, &eax1, nextop);
        GetGx(emu, &opx2, nextop);
        opx1->ud.d[0] = opx2->ud.d[0];
        break;

    default:
        UnimpOpcode(emu);
    }
}