#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "box86stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"
#include "x87emu_private.h"
#include "box86context.h"
#include "bridge.h"

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t TestF30F(x86test_t *test, uintptr_t addr)
#else
uintptr_t RunF30F(x86emu_t *emu, uintptr_t addr)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    int8_t tmp8s;
    uint8_t tmp8u;
    uint32_t tmp32u;
    int64_t tmp64s;
    uint64_t tmp64u;
    reg32_t *oped, *opgd;
    sse_regs_t *opex, *opgx, eax1;
    mmx87_regs_t *opem;
    #ifdef TEST_INTERPRETER
    x86emu_t*emu = test->emu;
    #endif

    #ifdef TERMUX
    extern int isinff(float);
    extern int isnanf(float);
    #endif

    opcode = F8;

    switch(opcode) {

    case 0x10:  /* MOVSS Gx Ex */
        nextop = F8;
        GET_EX;
        GX.ud[0] = EX->ud[0];
        if((nextop&0xC0)!=0xC0) {
            // EX is not a register (reg to reg only move 31:0)
            GX.ud[1] = GX.ud[2] = GX.ud[3] = 0;
        }
        break;
    case 0x11:  /* MOVSS Ex Gx */
        nextop = F8;
        GET_EX;
        EX->ud[0] = GX.ud[0];
        break;
    case 0x12:  /* MOVSLDUP Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.ud[1] = GX.ud[0] = EX->ud[0];
        GX.ud[3] = GX.ud[2] = EX->ud[2];
        break;
    
    case 0x16:  /* MOVSHDUP Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.ud[1] = GX.ud[0] = EX->ud[1];
        GX.ud[3] = GX.ud[2] = EX->ud[3];
        break;
    
    case 0x1E:  /* NOP / ENDBR32 / ENDBR64 */
        nextop = F8;
        GET_ED;
        break;
    case 0x1F:  /* REP NOP */
        nextop = F8;
        GET_ED;
        break;

    case 0x2A:  /* CVTSI2SS Gx, Ed */
        nextop = F8;
        GET_ED;
        GX.f[0] = ED->sdword[0];
        break;

    case 0x2C:  /* CVTTSS2SI Gd, Ex */
        nextop = F8;
        GET_EX;
        if(isnanf(EX->f[0]) || isinff(EX->f[0]) || EX->f[0]>=(float)0x80000000U || EX->f[0]<-(float)0x80000000U)
            GD.dword[0] = 0x80000000;
        else
            GD.sdword[0] = EX->f[0];
        break;
    case 0x2D:  /* CVTSS2SI Gd, Ex */
        nextop = F8;
        GET_EX;
        if(isnanf(EX->f[0]) || isinff(EX->f[0]) || EX->f[0]>=(float)0x80000000U || EX->f[0]<-(float)0x80000000U)
            GD.dword[0] = 0x80000000;
        else
            switch(emu->mxcsr.f.MXCSR_RC) {
                case ROUND_Nearest:
                    GD.sdword[0] = nearbyintf(EX->f[0]);
                    break;
                case ROUND_Down:
                    GD.sdword[0] = floorf(EX->f[0]);
                    break;
                case ROUND_Up:
                    GD.sdword[0] = ceilf(EX->f[0]);
                    break;
                case ROUND_Chop:
                    GD.sdword[0] = EX->f[0];
                    break;
            }
        break;

    case 0x51:  /* SQRTSS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f[0] = sqrtf(EX->f[0]);
        break;
    case 0x52:  /* RSQRTSS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f[0] = 1.0f/sqrtf(EX->f[0]);
        break;
    case 0x53:  /* RCPSS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f[0] = 1.0f/EX->f[0];
        break;

    case 0x58:  /* ADDSS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f[0] += EX->f[0];
        break;
    case 0x59:  /* MULSS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f[0] *= EX->f[0];
        break;
    case 0x5A:  /* CVTSS2SD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d[0] = EX->f[0];
        break;
    case 0x5B:  /* CVTTPS2DQ Gx, Ex */
        nextop = F8;
        GET_EX;
        for(int i=0; i<4; ++i)
            if(isnanf(EX->f[i]) || isinff(EX->f[i]) || EX->f[i]>=(float)0x80000000U || EX->f[i]<-(float)0x80000000U)
                GX.ud[i] = 0x80000000;
            else
                GX.sd[i] = EX->f[i];
        break;

    case 0x5C:  /* SUBSS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f[0] -= EX->f[0];
        break;
    case 0x5D:  /* MINSS Gx, Ex */
        nextop = F8;
        GET_EX;
        if(isnanf(GX.f[0]) || isnanf(EX->f[0]) || islessequal(EX->f[0], GX.f[0]))
            GX.f[0] = EX->f[0];
        break;
    case 0x5E:  /* DIVSS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f[0] /= EX->f[0];
        break;
    case 0x5F:  /* MAXSS Gx, Ex */
        nextop = F8;
        GET_EX;
        if (isnanf(GX.f[0]) || isnanf(EX->f[0]) || isgreaterequal(EX->f[0], GX.f[0]))
            GX.f[0] = EX->f[0];
        break;

    case 0x6F:  /* MOVDQU Gx, Ex */
        nextop = F8;
        GET_EX;
        memcpy(&GX, EX, 16);    // unaligned...
        break;
    case 0x70:  /* PSHUFHW Gx, Ex, Ib */
        nextop = F8;
        GET_EX;
        tmp8u = F8;
        if(&GX==EX) {
            for (int i=0; i<4; ++i)
                eax1.uw[4+i] = EX->uw[4+((tmp8u>>(i*2))&3)];
            GX.q[1] = eax1.q[1];
        } else {
            for (int i=0; i<4; ++i)
                GX.uw[4+i] = EX->uw[4+((tmp8u>>(i*2))&3)];
            GX.q[0] = EX->q[0];
        }
        break;

    case 0x7E:  /* MOVQ Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.q[0] = EX->q[0];
        GX.q[1] = 0;
        break;
    case 0x7F:  /* MOVDQU Ex, Gx */
        nextop = F8;
        GET_EX;
        memcpy(EX, &GX, 16);    // unaligned...
        break;

    case 0xB8:  /* POPCNT Gd,Ed */
        nextop = F8;
        GET_ED;
        GD.dword[0] = __builtin_popcount(ED->dword[0]);
        RESET_FLAGS(emu);
        CLEAR_FLAG(F_OF);
        CLEAR_FLAG(F_SF);
        CLEAR_FLAG(F_ZF);
        CLEAR_FLAG(F_AF);
        CLEAR_FLAG(F_CF);
        CLEAR_FLAG(F_PF);
        CONDITIONAL_SET_FLAG(GD.dword[0]==0, F_ZF);
        break;

    case 0xBC:  /* TZCNT Ed,Gd */
        CHECK_FLAGS(emu);
        nextop = F8;
        GET_ED;
        tmp32u = ED->dword[0];
        if(tmp32u) {
            CLEAR_FLAG(F_ZF);
            tmp8u = 0;
            while(!(tmp32u&(1<<tmp8u))) ++tmp8u;
            GD.dword[0] = tmp8u;
            CONDITIONAL_SET_FLAG(tmp8u==0, F_ZF);
            CLEAR_FLAG(F_CF);
        } else {
            CLEAR_FLAG(F_ZF);
            SET_FLAG(F_CF);
            GD.dword[0] = 32;
        }
        break;

    case 0xBD:                      /* LZCNT Ed,Gd */
        CHECK_FLAGS(emu);
        nextop = F8;
        GET_ED;
        tmp32u = ED->dword[0];
        if(tmp32u) {
            tmp8u = 0;
            while(!(tmp32u&(1<<(31-tmp8u)))) ++tmp8u;
            GD.dword[0] = tmp8u;
            CONDITIONAL_SET_FLAG(tmp8u==0, F_ZF);
            CLEAR_FLAG(F_CF);
        } else {
            SET_FLAG(F_CF);
            CLEAR_FLAG(F_ZF);
            GD.dword[0] = 32;
        }
        break;

    case 0xC2:  /* CMPSS Gx, Ex, Ib */
        nextop = F8;
        GET_EX;
        tmp8u = F8;
        tmp8s = 0;
        switch(tmp8u&7) {
            case 0: tmp8s=(GX.f[0] == EX->f[0]); break;
            case 1: tmp8s=isless(GX.f[0], EX->f[0]) && !(isnan(GX.f[0]) || isnan(EX->f[0])); break;
            case 2: tmp8s=islessequal(GX.f[0], EX->f[0]) && !(isnan(GX.f[0]) || isnan(EX->f[0])); break;
            case 3: tmp8s=isnan(GX.f[0]) || isnan(EX->f[0]); break;
            case 4: tmp8s=(GX.f[0] != EX->f[0]); break;
            case 5: tmp8s=isnan(GX.f[0]) || isnan(EX->f[0]) || isgreaterequal(GX.f[0], EX->f[0]); break;
            case 6: tmp8s=isnan(GX.f[0]) || isnan(EX->f[0]) || isgreater(GX.f[0], EX->f[0]); break;
            case 7: tmp8s=!isnan(GX.f[0]) && !isnan(EX->f[0]); break;
        }
        GX.ud[0]=(tmp8s)?0xffffffff:0;
        break;

    case 0xD6:  /* MOVQ2DQ Gx, Em */
        nextop = F8;
        GET_EM;
        GX.q[0] = EM->q;
        GX.q[1] = 0;
        break;

    case 0xE6:  /* CVTDQ2PD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d[1] = EX->sd[1];
        GX.d[0] = EX->sd[0];
        break;

    default:
        return 0;
    }
    return addr;
}