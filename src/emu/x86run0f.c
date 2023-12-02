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
#include "my_cpuid.h"
#include "bridge.h"
#include "signals.h"
#ifdef DYNAREC
#include "../dynarec/arm_lock_helper.h"
#include "custommem.h"
#endif

#include "modrm.h"

#ifdef TEST_INTERPRETER
uintptr_t Test0F(x86test_t *test, uintptr_t addr, int *step)
#else
uintptr_t Run0F(x86emu_t *emu, uintptr_t addr, int *step)
#endif
{
    uint8_t opcode;
    uint8_t nextop;
    reg32_t *oped;
    uint8_t tmp8u, tmp8u2;
    int8_t tmp8s;
    uint16_t tmp16u, tmp16u2;
    int16_t tmp16s;
    uint32_t tmp32u, tmp32u2, tmp32u3;
    int32_t tmp32s, tmp32s2;
    uint64_t tmp64u;
    int64_t tmp64s;
    double d;
    float f;
    int64_t ll;
    sse_regs_t *opex, eax1;
    mmx87_regs_t *opem, eam1;

#ifdef TEST_INTERPRETER
    x86emu_t *emu = test->emu;
#endif
#ifdef TERMUX
    extern int isinff(float);
    extern int isnanf(float);
#endif
    opcode = F8;

    switch(opcode) {

        case 0x00:                       /* VERx Ed */
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:                 /* SLDT Ew */
                    GET_EW;
                    EW->word[0] = 0;
                    if((nextop&0xC0)==0xC0)
                        EW->word[1] = 0;
                    break;
                case 1:                 /* STR Ew */
                    GET_EW;
                    EW->word[0] = 0x7f; // dummy return
                    if((nextop&0xC0)==0xC0)
                        EW->word[1] = 0;
                    break;
                case 4: //VERR
                case 5: //VERW
                    GET_EW;
                    if(!EW->word[0])
                        CLEAR_FLAG(F_ZF);
                    else
                        SET_FLAG(F_ZF); // should test if selector is ok
                    break;
                default:
                    return 0;
            }
            break;
        case 0x01:                      
            nextop = F8;
            switch((nextop>>3)&7) {
                case 0:                 /* SGDT Ed */
                    GET_ED;
                    ED->word[0] = 0x7f;    // dummy return...
                    ED->word[1] = 0x000c;
                    ED->word[2] = 0xd000;
                    break;
                case 1:                 /* SIDT Ed */
                    GET_ED;
                    ED->word[0] = 0xfff;    // dummy return, like "disabled"
                    ED->word[1] = 0;
                    ED->word[2] = 0;
                    break;
                case 4:                 /* SMSW Ew */
                    GET_ED;
                    // dummy for now... Do I need to track CR0 state?
                    ED->word[0] = (1<<0) | (1<<4); // only PE and ET set...
                    break;
                default:
                    return 0;
            }
            break;

        case 0x0B:                      /* UD2 */
            #ifndef TEST_INTERPRETER
            emit_signal(emu, SIGILL, (void*)R_EIP, 0);
            #endif
            break;
        
        case 0x0D:                   /* PREFETCH group */
            nextop = F8;
            switch((nextop>>3)&7) {
                case 1: /* PREFETCHW */
                    GET_ED; //nothing for now. Could maybe use some builtin prefetch instruction
                    break;
                default:
                    return 0;
            }
            break;
        case 0x0E:                      /* FEMMS */
            #ifndef TEST_INTERPRETER
            emit_signal(emu, SIGILL, (void*)R_EIP, 0);
            #endif
            break;

        case 0x10:                      /* MOVUPS Gx,Ex */
            nextop = F8;
            GET_EX;
            memcpy(&GX, EX, 16); // unaligned, so carreful
            break;
        case 0x11:                      /* MOVUPS Ex,Gx */
            nextop = F8;
            GET_EX;
            memcpy(EX, &GX, 16); // unaligned, so carreful
            break;
        case 0x12:                      
            nextop = F8;
            GET_EX;
            if((nextop&0xC0)==0xC0)    /* MOVHLPS Gx,Ex */
                GX.q[0] = EX->q[1];
            else
                GX.q[0] = EX->q[0];    /* MOVLPS Gx,Ex */
            break;
        case 0x13:                      /* MOVLPS Ex,Gx */
            nextop = F8;
            GET_EX;
            EX->q[0] = GX.q[0];
            break;
        case 0x14:                      /* UNPCKLPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.ud[3] = EX->ud[1];
            GX.ud[2] = GX.ud[1];
            GX.ud[1] = EX->ud[0];
            break;
        case 0x15:                      /* UNPCKHPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.ud[0] = GX.ud[2];
            GX.ud[1] = EX->ud[2];
            GX.ud[2] = GX.ud[3];
            GX.ud[3] = EX->ud[3];
            break;
        case 0x16:                      /* MOVHPS Gx,Ex */
            nextop = F8;               /* MOVLHPS Gx,Ex (Ex==reg) */
            GET_EX;
            GX.q[1] = EX->q[0];
            break;
        case 0x17:                      /* MOVHPS Ex,Gx */
            nextop = F8;
            GET_EX;
            EX->q[0] = GX.q[1];
            break;
        case 0x18:                       /* PREFETCHh Ed */
            nextop = F8;
            GET_ED_;
            if((nextop&0xC0)==0xC0) {
            } else
            switch((nextop>>3)&7) {
                case 0: //PREFETCHnta
                case 1: //PREFETCH1
                case 2: //PREFETCH2
                case 3: //PREFETCH3
                    __builtin_prefetch((void*)ED, 0, 0); // ignoring wich level of cache
                    break;
                default:    //NOP
                    break;
            }
            break;
        case 0x19:                      /* HINT_NOP (multi-byte) */
        case 0x1A:                      /* NOP (multi-byte) / ignored BNDLDX */
        case 0x1B:                      /* NOP (multi-byte) / ignored BNDSTX */
        case 0x1C:                      /* HINT_NOP (multi-byte) */
        case 0x1D:                      /* HINT_NOP (multi-byte) */
        case 0x1E:                      /* HINT_NOP (multi-byte) */
        case 0x1F:                      /* NOP (multi-byte) */
            nextop = F8;
            GET_ED_;
            break;

        case 0x28:                      /* MOVAPS Gx,Ex */
            nextop = F8;
            GET_EX;
            GX.q[0] = EX->q[0];
            GX.q[1] = EX->q[1];
            break;
        case 0x29:                      /* MOVAPS Ex,Gx */
            nextop = F8;
            GET_EX;
            EX->q[0] = GX.q[0];
            EX->q[1] = GX.q[1];
            break;
        case 0x2A:                      /* CVTPI2PS Gx, Em */
            nextop = F8;
            GET_EM;
            GX.f[0] = EM->sd[0];
            GX.f[1] = EM->sd[1];
            break;
        case 0x2B:                      /* MOVNTPS Ex,Gx */
            nextop = F8;
            GET_EX;
            EX->q[0] = GX.q[0];
            EX->q[1] = GX.q[1];
            break;
        case 0x2C:                      /* CVTTPS2PI Gm, Ex */
            nextop = F8;
            GET_EX;
            if(isnanf(EX->f[1]) || isinff(EX->f[1]) || EX->f[1]>=(float)0x80000000U || EX->f[1]<-(float)0x80000000U)
                GM.sd[1] = 0x80000000;
            else
                GM.sd[1] = EX->f[1];
            if(isnanf(EX->f[0]) || isinff(EX->f[0]) || EX->f[0]>=(float)0x80000000U || EX->f[1]<-(float)0x80000000U)
                GM.sd[0] = 0x80000000;
            else
                GM.sd[0] = EX->f[0];
            break;
        case 0x2D:                      /* CVTPS2PI Gm, Ex */
            // rounding should be done; and indefinite integer should also be assigned if overflow or NaN/Inf
            nextop = F8;
            GET_EX;
            for(int i=1; i>=0; --i)
                if(isnanf(EX->f[i]) || isinff(EX->f[i]) || EX->f[i]>=(float)0x80000000U || EX->f[i]<-(float)0x80000000U)
                    GM.sd[i] = 0x80000000;
                else
                    switch(emu->mxcsr.f.MXCSR_RC) {
                        case ROUND_Nearest:
                            GM.sd[i] = nearbyintf(EX->f[i]);
                            break;
                        case ROUND_Down:
                            GM.sd[i] = floorf(EX->f[i]);
                            break;
                        case ROUND_Up:
                            GM.sd[i] = ceilf(EX->f[i]);
                            break;
                        case ROUND_Chop:
                            GM.sd[i] = EX->f[i];
                            break;
                    }
            break;
        case 0x2E:                      /* UCOMISS Gx, Ex */
            // same for now
        case 0x2F:                      /* COMISS Gx, Ex */
            RESET_FLAGS(emu);
            nextop = F8;
            GET_EX;
            if(isnan(GX.f[0]) || isnan(EX->f[0])) {
                SET_FLAG(F_ZF); SET_FLAG(F_PF); SET_FLAG(F_CF);
            } else if(isgreater(GX.f[0], EX->f[0])) {
                CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
            } else if(isless(GX.f[0], EX->f[0])) {
                CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); SET_FLAG(F_CF);
            } else {
                SET_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
            }
            CLEAR_FLAG(F_OF); CLEAR_FLAG(F_AF); CLEAR_FLAG(F_SF);
            break;

        case 0x31:                   /* RDTSC */
            tmp64u = ReadTSC(emu);
            R_EDX = tmp64u>>32;
            R_EAX = tmp64u&0xFFFFFFFF;
            break;
        
        case 0x38:  // these are some SSE3 opcodes
            opcode = F8;
            switch(opcode) {
                case 0x00:  /* PSHUFB */
                    nextop = F8;
                    GET_EM;
                    eam1 = GM;
                    for (int i=0; i<8; ++i) {
                        if(EM->ub[i]&128)
                            GM.ub[i] = 0;
                        else
                            GM.ub[i] = eam1.ub[EM->ub[i]&7];
                    }
                    break;
                case 0x01:  /* PHADDW Gm, Em */
                    nextop = F8;
                    GET_EM;
                    for (int i=0; i<2; ++i)
                        GM.sw[i] = GM.sw[i*2+0]+GM.sw[i*2+1];
                    if(&GM == EM) {
                        GM.ud[1] = GM.ud[0];
                    } else {
                        for (int i=0; i<2; ++i)
                            GM.sw[2+i] = EM->sw[i*2+0] + EM->sw[i*2+1];
                    }
                    break;
                case 0x02:  /* PHADDD Gm, Em */
                    nextop = F8;
                    GET_EM;
                    GM.sd[0] = GM.sd[0]+GM.sd[1];
                    if(&GM == EM)
                        GM.ud[1] = GM.ud[0];
                    else
                        GM.sd[1] = EM->sd[0] + EM->sd[1];
                    break;
                case 0x03:  /* PHADDSW Gm, Em */
                    nextop = F8;
                    GET_EM;
                    for (int i=0; i<2; ++i) {
                        tmp32s = GM.sw[i*2+0]+GM.sw[i*2+1];
                        GM.sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
                    }
                    if(&GM == EM) {
                        GM.ud[1] = GM.ud[0];
                    } else {
                        for (int i=0; i<2; ++i) {
                            tmp32s = EM->sw[i*2+0] + EM->sw[i*2+1];
                            GM.sw[2+i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
                        }
                    }
                    break;
                case 0x04:  /* PMADDUBSW Gm,Em */
                    nextop = F8;
                    GET_EM;
                    for (int i=0; i<4; ++i) {
                        tmp32s = (int32_t)(GM.ub[i*2+0])*EM->sb[i*2+0] + (int32_t)(GM.ub[i*2+1])*EM->sb[i*2+1];
                        GM.sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
                    }
                    break;
                case 0x05:  /* PHSUBW Gm, Em */
                    nextop = F8;
                    GET_EM;
                    for (int i=0; i<2; ++i)
                        GM.sw[i] = GM.sw[i*2+0]-GM.sw[i*2+1];
                    if(&GM == EM) {
                        GM.ud[1] = GM.ud[0];
                    } else {
                        for (int i=0; i<2; ++i)
                            GM.sw[2+i] = EM->sw[i*2+0] - EM->sw[i*2+1];
                    }
                    break;
                case 0x06:  /* PHSUBD Gm, Em */
                    nextop = F8;
                    GET_EM;
                    GM.sd[0] = GM.sd[0]-GM.sd[1];
                    if(&GM == EM)
                        GM.ud[1] = GM.ud[0];
                    else
                        GM.sd[1] = EM->sd[0] - EM->sd[1];
                    break;
                case 0x07:  /* PHSUBSW Gm, Em */
                    nextop = F8;
                    GET_EM;
                    for (int i=0; i<2; ++i) {
                        tmp32s = GM.sw[i*2+0]-GM.sw[i*2+1];
                        GM.sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
                    }
                    if(&GM == EM) {
                        GM.ud[1] = GM.ud[0];
                    } else {
                        for (int i=0; i<2; ++i) {
                            tmp32s = EM->sw[i*2+0] - EM->sw[i*2+1];
                            GM.sw[2+i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
                        }
                    }
                    break;
                case 0x08:  /* PSIGNB Gm, Em */
                    nextop = F8;
                    GET_EM;
                    for (int i=0; i<8; ++i) {
                        if (EM->sb[i] < 0) {
                            GM.sb[i] = -GM.sb[i];
                        } else if (EM->sb[i] == 0) {
                            GM.sb[i] = 0;
                        }
                    }
                    break;
                case 0x09:  /* PSIGNW Gm, Em */
                    nextop = F8;
                    GET_EM;
                    for (int i=0; i<4; ++i) {
                        if (EM->sw[i] < 0) {
                            GM.sw[i] = -GM.sw[i];
                        } else if (EM->sw[i] == 0) {
                            GM.sw[i] = 0;
                        }
                    }
                    break;
                case 0x0B:  /* PMULHRSW Gm, Em */
                    nextop = F8;
                    GET_EM;
                    for (int i=0; i<4; ++i) {
                        tmp32s = ((((int32_t)(GM.sw[i])*(int32_t)(EM->sw[i]))>>14) + 1)>>1;
                        GM.uw[i] = tmp32s&0xffff;
                    }
                    break;

                case 0x1C:  /* PABSB Gm, Em */
                    nextop = F8;
                    GET_EM;
                    for (int i=0; i<8; ++i) {
                        GM.sb[i] = abs(EM->sb[i]);
                    }
                    break;
                case 0x1D:  /* PABSW Gm, Em */
                    nextop = F8;
                    GET_EM;
                    for (int i=0; i<4; ++i) {
                        GM.sw[i] = abs(EM->sw[i]);
                    }
                    break;
                case 0x1E:  /* PABSD Gm, Em */
                    nextop = F8;
                    GET_EM;
                    for (int i=0; i<2; ++i) {
                        GM.sd[i] = abs(EM->sd[i]);
                    }
                    break;

                case 0xF0: /* MOVBE Gd, Ed*/
                    nextop = F8;
                    GET_ED;
                    GD.dword[0] = __builtin_bswap32(ED->dword[0]);
                    break;
                case 0xF1: /* MOVBE Ed, Gd*/
                    nextop = F8;
                    GET_ED;
                    ED->dword[0] = __builtin_bswap32(GD.dword[0]);
                    break;

                default:
                    return 0;
            }
            break;

        case 0x3A:
            opcode = F8;
            switch(opcode) {
                case 0x0F:  /* palignr */
                    nextop = F8;
                    GET_EM;
                    tmp8u = F8;
                    if (tmp8u >= 16) {
                        GM.q = 0;
                    } else if (tmp8u > 8) {
                        tmp8u -= 8;
                        GM.q >>= tmp8u*8;
                    } else if (tmp8u == 8) {
                        // nothing
                    } else if (tmp8u == 0) {
                        GM.q = EM->q;
                    } else {
                        GM.q <<= (8-tmp8u)*8;
                        GM.q |= (EM->q >> tmp8u*8);
                    }
                    break;

                default:
                    return 0;
            }
            break;

        case 0x3F:
            #ifndef TEST_INTERPRETER
            emit_signal(emu, SIGILL, (void*)R_EIP, 0);
            #endif
            break;
            
        #define GOCOND(BASE, PREFIX, CONDITIONAL) \
        case BASE+0x00:                         \
            PREFIX                              \
            if(ACCESS_FLAG(F_OF))               \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x01:                         \
            PREFIX                              \
            if(!ACCESS_FLAG(F_OF))              \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x02:                         \
            PREFIX                              \
            if(ACCESS_FLAG(F_CF))               \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x03:                         \
            PREFIX                              \
            if(!ACCESS_FLAG(F_CF))              \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x04:                         \
            PREFIX                              \
            if(ACCESS_FLAG(F_ZF))               \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x05:                         \
            PREFIX                              \
            if(!ACCESS_FLAG(F_ZF))              \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x06:                         \
            PREFIX                              \
            if((ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF)))  \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x07:                         \
            PREFIX                              \
            if(!(ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF))) \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x08:                         \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF))               \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x09:                         \
            PREFIX                              \
            if(!ACCESS_FLAG(F_SF))              \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x0A:                         \
            PREFIX                              \
            if(ACCESS_FLAG(F_PF))               \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x0B:                         \
            PREFIX                              \
            if(!ACCESS_FLAG(F_PF))              \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x0C:                         \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))  \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x0D:                         \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF)) \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x0E:                         \
            PREFIX                              \
            if(ACCESS_FLAG(F_ZF) || (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))) \
                CONDITIONAL                     \
            break;                              \
        case BASE+0x0F:                         \
            PREFIX                              \
            if(!ACCESS_FLAG(F_ZF) && (ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF))) \
                CONDITIONAL                     \
            break;

        GOCOND(0x40
            , nextop = F8;
            GET_ED;
            CHECK_FLAGS(emu);
            , GD.dword[0] = ED->dword[0];
        )                               /* 0x40 -> 0x4F CMOVxx Gd,Ed */ // conditional move, no sign
        GOCOND(0x80
            , tmp32s = F32S; CHECK_FLAGS(emu);
            , addr += tmp32s; STEP3;
        )                               /* 0x80 -> 0x8F Jxx */
        GOCOND(0x90
            , nextop = F8; CHECK_FLAGS(emu);
            GET_EB;
            , EB->byte[0]=1; else EB->byte[0]=0;
        )                               /* 0x90 -> 0x9F SETxx Eb */

        #undef GOCOND

        case 0x50:                      /* MOVMSKPS Gd, Ex */
            nextop = F8;
            GET_EX;
            GD.dword[0] = 0;
            for(int i=0; i<4; ++i)
                GD.dword[0] |= ((EX->ud[i]>>31)&1)<<i;
            break;
        case 0x51:                      /* SQRTPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.f[i] = sqrtf(EX->f[i]);
            break;
        case 0x52:                      /* RSQRTPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i) {
                if(EX->f[i]==0)
                    GX.f[i] = 1.0f/EX->f[i];
                else if (EX->f[i]<0)
                    GX.f[i] = NAN;
                else if (isnan(EX->f[i]))
                    GX.f[i] = EX->f[i];
                else if (isinf(EX->f[i]))
                    GX.f[i] = 0.0;
                else
                    GX.f[i] = 1.0f/sqrtf(EX->f[i]);
            }
            break;
        case 0x53:                      /* RCPPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.f[i] = 1.0f/EX->f[i];
            break;
        case 0x54:                      /* ANDPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.ud[i] &= EX->ud[i];
            break;
        case 0x55:                      /* ANDNPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.ud[i] = (~GX.ud[i]) & EX->ud[i];
            break;
        case 0x56:                      /* ORPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.ud[i] |= EX->ud[i];
            break;
        case 0x57:                      /* XORPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.ud[i] ^= EX->ud[i];
            break;
        case 0x58:                      /* ADDPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.f[i] += EX->f[i];
            break;
        case 0x59:                      /* MULPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.f[i] *= EX->f[i];
            break;
        case 0x5A:                      /* CVTPS2PD Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.d[1] = EX->f[1];
            GX.d[0] = EX->f[0];
            break;
        case 0x5B:                      /* CVTDQ2PS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.f[0] = EX->sd[0];
            GX.f[1] = EX->sd[1];
            GX.f[2] = EX->sd[2];
            GX.f[3] = EX->sd[3];
            break;
        case 0x5C:                      /* SUBPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.f[i] -= EX->f[i];
            break;
        case 0x5D:                      /* MINPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i) {
                if (isnan(GX.f[i]) || isnan(EX->f[i]) || isless(EX->f[i], GX.f[i]))
                    GX.f[i] = EX->f[i];
            }
            break;
        case 0x5E:                      /* DIVPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.f[i] /= EX->f[i];
            break;
        case 0x5F:                      /* MAXPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i) {
                if (isnan(GX.f[i]) || isnan(EX->f[i]) || isgreater(EX->f[i], GX.f[i]))
                    GX.f[i] = EX->f[i];
            }
            break;
        case 0x60:                      /* PUNPCKLBW Gm, Em */
            nextop = F8;
            GET_EM;
            GM.ub[7] = EM->ub[3];
            GM.ub[6] = GM.ub[3];
            GM.ub[5] = EM->ub[2];
            GM.ub[4] = GM.ub[2];
            GM.ub[3] = EM->ub[1];
            GM.ub[2] = GM.ub[1];
            GM.ub[1] = EM->ub[0];
            break;
        case 0x61:                      /* PUNPCKLWD Gm, Em */
            nextop = F8;
            GET_EM;
            GM.uw[3] = EM->uw[1];
            GM.uw[2] = GM.uw[1];
            GM.uw[1] = EM->uw[0];
            break;
        case 0x62:                      /* PUNPCKLDQ Gm, Em */
            nextop = F8;
            GET_EM;
            GM.ud[1] = EM->ud[0];
            break;
        case 0x63:                      /* PACKSSWB Gm, Em */
            nextop = F8;
            GET_EM;
            GM.sb[0] = (GM.sw[0] > 127) ? 127 : ((GM.sw[0] < -128) ? -128 : GM.sw[0]);
            GM.sb[1] = (GM.sw[1] > 127) ? 127 : ((GM.sw[1] < -128) ? -128 : GM.sw[1]);
            GM.sb[2] = (GM.sw[2] > 127) ? 127 : ((GM.sw[2] < -128) ? -128 : GM.sw[2]);
            GM.sb[3] = (GM.sw[3] > 127) ? 127 : ((GM.sw[3] < -128) ? -128 : GM.sw[3]);
            if(EM==&GM)
                GM.ud[1] = GM.ud[0];
            else {
                GM.sb[4] = (EM->sw[0] > 127) ? 127 : ((EM->sw[0] < -128) ? -128 : EM->sw[0]);
                GM.sb[5] = (EM->sw[1] > 127) ? 127 : ((EM->sw[1] < -128) ? -128 : EM->sw[1]);
                GM.sb[6] = (EM->sw[2] > 127) ? 127 : ((EM->sw[2] < -128) ? -128 : EM->sw[2]);
                GM.sb[7] = (EM->sw[3] > 127) ? 127 : ((EM->sw[3] < -128) ? -128 : EM->sw[3]);
            }
            break;
        case 0x64:                       /* PCMPGTB Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i = 0; i < 8; i++) {
                GM.ub[i] = (GM.sb[i] > EM->sb[i]) ? 0xFF : 0;
            }
            break;
        case 0x65:                       /* PCMPGTW Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i = 0; i < 4; i++) {
                GM.uw[i] = (GM.sw[i] > EM->sw[i]) ? 0xFFFF : 0;
            }
            break;
        case 0x66:                       /* PCMPGTD Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i = 0; i < 2; i++) {
                GM.ud[i] = (GM.sd[i] > EM->sd[i]) ? 0xFFFFFFFF : 0;
            }
            break;
        case 0x67:                       /* PACKUSWB Gm, Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i)
                GM.ub[i] = (GM.sw[i]<0)?0:((GM.sw[i]>0xff)?0xff:GM.sw[i]);
            if(EM==&GM)
                GM.ud[1] = GM.ud[0];
            else
                for(int i=0; i<4; ++i)
                    GM.ub[4+i] = (EM->sw[i]<0)?0:((EM->sw[i]>0xff)?0xff:EM->sw[i]);
            break;
        case 0x68:                       /* PUNPCKHBW Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i)
                GM.ub[2 * i] = GM.ub[i + 4];
            if(EM==&GM)
                for(int i=0; i<4; ++i)
                    GM.ub[2 * i + 1] = GM.ub[2 * i];
            else
                for(int i=0; i<4; ++i)
                    GM.ub[2 * i + 1] = EM->ub[i + 4];
            break;
        case 0x69:                       /* PUNPCKHWD Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<2; ++i)
                GM.uw[2 * i] = GM.uw[i + 2];
            if(EM==&GM)
                for(int i=0; i<2; ++i)
                    GM.uw[2 * i + 1] = GM.uw[2 * i];
            else
                for(int i=0; i<2; ++i)
                    GM.uw[2 * i + 1] = EM->uw[i + 2];
            break;
        case 0x6A:                       /* PUNPCKHDQ Gm,Em */
            nextop = F8;
            GET_EM;
            GM.ud[0] = GM.ud[1];
            if(EM!=&GM)
                GM.ud[1] = EM->ud[1];
            break;
        case 0x6B:                       /* PACKSSDW Gm,Em */
            nextop = F8;
            GET_EM;
            if(EM==&GM) {eam1 = GM; EM = &eam1;}   // copy is needed
            for(int i=0; i<2; ++i)
                GM.sw[i] = (GM.sd[i]<-32768)?-32768:((GM.sd[i]>32767)?32767:GM.sd[i]);
            if(EM==&GM)
                GM.ud[1] = GM.ud[0];
            else
                for(int i=0; i<2; ++i)
                    GM.sw[2+i] = (EM->sd[i]<-32768)?-32768:((EM->sd[i]>32767)?32767:EM->sd[i]);
            break;

        case 0x6E:                      /* MOVD Gm, Ed */
            nextop = F8;
            GET_ED;
            GM.q = ED->dword[0];    // zero extended
            break;
        case 0x6F:                      /* MOVQ Gm, Em */
            nextop = F8;
            GET_EM;
            GM.q = EM->q;
            break;
        case 0x70:                       /* PSHUFW Gm, Em, Ib */
            nextop = F8;
            GET_EM;
            tmp8u = F8;
            if(EM==&GM) {eam1 = GM; EM = &eam1;}   // copy is needed
            for(int i=0; i<4; ++i)
                GM.uw[i] = EM->uw[(tmp8u>>(i*2))&3];
            break;
        case 0x71:  /* GRP */
            nextop = F8;
            GET_EM;
            switch((nextop>>3)&7) {
                case 2:                 /* PSRLW Em, Ib */
                    tmp8u = F8;
                    if(tmp8u>15)
                        {EM->q = 0;}
                    else
                        for (int i=0; i<4; ++i) EM->uw[i] >>= tmp8u;
                    break;
                case 4:                 /* PSRAW Em, Ib */
                    tmp8u = F8;
                    for (int i=0; i<4; ++i) EM->sw[i] >>= tmp8u;
                    break;
                case 6:                 /* PSLLW Em, Ib */
                    tmp8u = F8;
                    if(tmp8u>15)
                        {EM->q = 0;}
                    else
                        for (int i=0; i<4; ++i) EM->uw[i] <<= tmp8u;
                    break;
                default:
                    return 0;
            }
            break;
        case 0x72:  /* GRP */
            nextop = F8;
            GET_EM;
            switch((nextop>>3)&7) {
                case 2:                 /* PSRLD Em, Ib */
                    tmp8u = F8;
                    if(tmp8u>31)
                        {EM->q = 0;}
                    else
                        for (int i=0; i<2; ++i) EM->ud[i] >>= tmp8u;
                    break;
                case 4:                 /* PSRAD Em, Ib */
                    tmp8u = F8;
                    for (int i=0; i<2; ++i) EM->sd[i] >>= tmp8u;
                    break;
                case 6:                 /* PSLLD Em, Ib */
                    tmp8u = F8;
                    if(tmp8u>31)
                        {EM->q = 0;}
                    else
                        for (int i=0; i<2; ++i) EM->ud[i] <<= tmp8u;
                    break;
                default:
                    return 0;
            }
            break;
        case 0x73:  /* GRP */
            nextop = F8;
            GET_EM;
            switch((nextop>>3)&7) {
                case 2:                 /* PSRLQ Em, Ib */
                    tmp8u = F8;
                    if(tmp8u>63)
                        {EM->q = 0;}
                    else
                        {EM->q >>= tmp8u;}
                    break;
                case 6:                 /* PSLLQ Em, Ib */
                    tmp8u = F8;
                    if(tmp8u>63)
                        {EM->q = 0;}
                    else
                        {EM->q <<= tmp8u;}
                    break;
                default:
                    return 0;
            }
            break;
        case 0x74:                       /* PCMPEQB Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i = 0; i < 8; i++) {
                GM.ub[i] = (GM.sb[i] == EM->sb[i]) ? 0xFF : 0;
            }
            break;
        case 0x75:                       /* PCMPEQW Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i = 0; i < 4; i++) {
                GM.uw[i] = (GM.sw[i] == EM->sw[i]) ? 0xFFFF : 0;
            }
            break;
        case 0x76:                       /* PCMPEQD Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i = 0; i < 2; i++) {
                GM.ud[i] = (GM.sd[i] == EM->sd[i]) ? 0xFFFFFFFF : 0;
            }
            break;
        case 0x77:                      /* EMMS */
            // empty MMX, FPU now usable
            emu->top = 0;
            emu->fpu_stack = 0;
            break;

        case 0x7E:                       /* MOVD Ed, Gm */
            nextop = F8;
            GET_ED;
            ED->dword[0] = GM.ud[0];
            break;
        case 0x7F:                      /* MOVQ Em, Gm */
            nextop = F8;
            GET_EM;
            EM->q = GM.q;
            break;

        case 0xA0:                      /* PUSH FS */
            Push(emu, emu->segs[_FS]);    // even if a segment is a 16bits, a 32bits push/pop is done
            break;
        case 0xA1:                      /* POP FS */
            emu->segs[_FS] = Pop(emu);    // no check, no use....
            emu->segs_serial[_FS] = 0;
            break;
        case 0xA2:                      /* CPUID */
            tmp32u = R_EAX;
            my_cpuid(emu, tmp32u);
            break;
        case 0xA3:                      /* BT Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp32s = GD.sdword[0];
            tmp8u=tmp32s&31;
            tmp32s >>= 5;
            if((nextop&0xC0)!=0xC0)
            {
                #ifdef TEST_INTERPRETER
                test->memaddr=((test->memaddr)+tmp32s);
                *(uint32_t*)test->mem = *(uint32_t*)test->memaddr;
                #else
                ED=(reg32_t*)(((uint32_t*)(ED))+tmp32s);
                #endif
            }
            if(ED->dword[0] & (1<<tmp8u))
                SET_FLAG(F_CF);
            else
                CLEAR_FLAG(F_CF);
            break;
        case 0xA4:                      /* SHLD Ed,Gd,Ib */
        case 0xA5:                      /* SHLD Ed,Gd,CL */
            nextop = F8;
            GET_ED;
            tmp8u = (opcode==0xA4)?(F8):R_CL;
            ED->dword[0] = shld32(emu, ED->dword[0], GD.dword[0], tmp8u);
            break;

        case 0xA8:                      /* PUSH GS */
            Push(emu, emu->segs[_GS]);    // even if a segment is a 16bits, a 32bits push/pop is done
            break;
        case 0xA9:                      /* POP GS */
            emu->segs[_GS] = Pop(emu);
            emu->segs_serial[_GS] = 0;
            break;


        case 0xAB:                      /* BTS Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp32s = GD.sdword[0];
            tmp8u=tmp32s&31;
            tmp32s >>= 5;
            if((nextop&0xC0)!=0xC0)
            {
                #ifdef TEST_INTERPRETER
                test->memaddr=((test->memaddr)+tmp32s);
                *(uint32_t*)test->mem = *(uint32_t*)test->memaddr;
                #else
                ED=(reg32_t*)(((uint32_t*)(ED))+tmp32s);
                #endif
            }
            if(ED->dword[0] & (1<<tmp8u))
                SET_FLAG(F_CF);
            else {
                ED->dword[0] |= (1<<tmp8u);
                CLEAR_FLAG(F_CF);
            }
            break;
        case 0xAC:                      /* SHRD Ed,Gd,Ib */
        case 0xAD:                      /* SHRD Ed,Gd,CL */
            nextop = F8;
            GET_ED;
            tmp8u = (opcode==0xAC)?(F8):R_CL;
            ED->dword[0] = shrd32(emu, ED->dword[0], GD.dword[0], tmp8u);
            break;

        case 0xAE:                      /* Grp Ed (SSE) */
            nextop = F8;
            if((nextop&0xF8)==0xE8) {
                break;                   /* LFENCE */
            }
            if((nextop&0xF8)==0xF0) {
                break;                   /* MFENCE */
            }
            if((nextop&0xF8)==0xF8) {
                break;                   /* SFENCE */
            }
            switch((nextop>>3)&7) {
                case 0:                 /* FXSAVE m512byte */
                    GET_ED_;
                    #ifndef TEST_INTERPRETER
                    fpu_fxsave(emu, ED);
                    #endif
                    break;
                case 1:                 /* FXRSTOR m512byte */
                    GET_ED_;
                    fpu_fxrstor(emu, ED);
                    break;
                case 2:                 /* LDMXCSR Md */
                    GET_ED_;
                    emu->mxcsr.x32 = ED->dword[0];
                    break;
                case 3:                 /* STMXCSR Md */
                    GET_ED;
                    ED->dword[0] = emu->mxcsr.x32;
                    #ifndef TEST_INTERPRETER
                    if(box86_sse_flushto0)
                        applyFlushTo0(emu);
                    #endif
                    break;
                case 7:                 /* CLFLUSH Ed */
                    GET_ED_(0);
                    #if defined(DYNAREC) && !defined(TEST_INTERPRETER)
                    if(box86_dynarec)
                        cleanDBFromAddressRange((uintptr_t)ED, 8, 0);
                    #endif
                    break;
                default:
                    return 0;
            }
            break;
        case 0xAF:                      /* IMUL Gd,Ed */
            nextop = F8;
            GET_ED;
            GD.dword[0] = imul32(emu, GD.dword[0], ED->dword[0]);
            break;
        case 0xB0:                      /* CMPXCHG Eb,Gb */
            nextop = F8;
            GET_EB;
            cmp8(emu, R_AL, EB->byte[0]);
            if(ACCESS_FLAG(F_ZF)) {
                EB->byte[0] = GB;
            } else {
                R_AL = EB->byte[0];
            }
            break;
        case 0xB1:                      /* CMPXCHG Ed,Gd */
            nextop = F8;
            GET_ED;
            cmp32(emu, R_EAX, ED->dword[0]);
            if(ACCESS_FLAG(F_ZF)) {
                ED->dword[0] = GD.dword[0];
            } else {
                R_EAX = ED->dword[0];
            }
            break;

        case 0xB3:                      /* BTR Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp32s = GD.sdword[0];
            tmp8u=tmp32s&31;
            tmp32s >>= 5;
            if((nextop&0xC0)!=0xC0)
            {
                #ifdef TEST_INTERPRETER
                test->memaddr=((test->memaddr)+tmp32s);
                *(uint32_t*)test->mem = *(uint32_t*)test->memaddr;
                #else
                ED=(reg32_t*)(((uint32_t*)(ED))+tmp32s);
                #endif
            }
            if(ED->dword[0] & (1<<tmp8u)) {
                SET_FLAG(F_CF);
                ED->dword[0] ^= (1<<tmp8u);
            } else
                CLEAR_FLAG(F_CF);
            break;

        case 0xB6:                      /* MOVZX Gd,Eb */
            nextop = F8;
            GET_EB;
            GD.dword[0] = EB->byte[0];
            break;
        case 0xB7:                      /* MOVZX Gd,Ew */
            nextop = F8;
            GET_EW;
            GD.dword[0] = EW->word[0];
            break;

        case 0xBA:                      
            nextop = F8;
            switch((nextop>>3)&7) {
                case 4:                 /* BT Ed,Ib */
                    CHECK_FLAGS(emu);
                    GET_ED;
                    tmp8u = F8;
                    tmp8u&=31;
                    if(ED->dword[0] & (1<<tmp8u))
                        SET_FLAG(F_CF);
                    else
                        CLEAR_FLAG(F_CF);
                    break;
                case 5:             /* BTS Ed, Ib */
                    CHECK_FLAGS(emu);
                    GET_ED;
                    tmp8u = F8;
                    tmp8u&=31;
                    if(ED->dword[0] & (1<<tmp8u)) {
                        SET_FLAG(F_CF);
                    } else {
                        ED->dword[0] ^= (1<<tmp8u);
                        CLEAR_FLAG(F_CF);
                    }
                    break;
                case 6:             /* BTR Ed, Ib */
                    CHECK_FLAGS(emu);
                    GET_ED;
                    tmp8u = F8;
                    tmp8u&=31;
                    if(ED->dword[0] & (1<<tmp8u)) {
                        SET_FLAG(F_CF);
                        ED->dword[0] ^= (1<<tmp8u);
                    } else
                        CLEAR_FLAG(F_CF);
                    break;
                case 7:             /* BTC Ed, Ib */
                    CHECK_FLAGS(emu);
                    GET_ED;
                    tmp8u = F8;
                    tmp8u&=31;
                    if(ED->dword[0] & (1<<tmp8u))
                        SET_FLAG(F_CF);
                    else
                        CLEAR_FLAG(F_CF);
                    ED->dword[0] ^= (1<<tmp8u);
                    break;

                default:
                    return 0;
            }
            break;
        case 0xBB:                      /* BTC Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp32s = GD.sdword[0];
            tmp8u=tmp32s&31;
            tmp32s >>= 5;
            if((nextop&0xC0)!=0xC0)
            {
                #ifdef TEST_INTERPRETER
                test->memaddr=((test->memaddr)+tmp32s);
                *(uint32_t*)test->mem = *(uint32_t*)test->memaddr;
                #else
                ED=(reg32_t*)(((uint32_t*)(ED))+tmp32s);
                #endif
            }
            if(ED->dword[0] & (1<<tmp8u))
                SET_FLAG(F_CF);
            else
                CLEAR_FLAG(F_CF);
            ED->dword[0] ^= (1<<tmp8u);
            break;
        case 0xBC:                      /* BSF Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp32u = ED->dword[0];
            if(tmp32u) {
                CLEAR_FLAG(F_ZF);
                tmp8u = 0;
                while(!(tmp32u&(1<<tmp8u))) ++tmp8u;
                GD.dword[0] = tmp8u;
            } else {
                SET_FLAG(F_ZF);
            }
            break;
        case 0xBD:                      /* BSR Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp32u = ED->dword[0];
            if(tmp32u) {
                CLEAR_FLAG(F_ZF);
                tmp8u = 31;
                while(!(tmp32u&(1<<tmp8u))) --tmp8u;
                GD.dword[0] = tmp8u;
            } else {
                SET_FLAG(F_ZF);
            }
            break;
        case 0xBE:                      /* MOVSX Gd,Eb */
            nextop = F8;
            GET_EB;
            GD.sdword[0] = EB->sbyte[0];
            break;
        case 0xBF:                      /* MOVSX Gd,Ew */
            nextop = F8;
            GET_EW;
            GD.sdword[0] = EW->sword[0];
            break;

        case 0xC0:                      /* XADD Gb,Eb */
            nextop = F8;
            GET_EB;
            tmp8u = add8(emu, EB->byte[0], GB);
            GB = EB->byte[0];
            EB->byte[0] = tmp8u;
            break;
        case 0xC1:                      /* XADD Gd,Ed */
            nextop = F8;
            GET_ED;
            tmp32u = add32(emu, ED->dword[0], GD.dword[0]);
            GD.dword[0] = ED->dword[0];
            ED->dword[0] = tmp32u;
            break;
        case 0xC2:                      /* CMPPS Gx, Ex, Ib */
            nextop = F8;
            GET_EX;
            tmp8u = F8;
            for(int i=0; i<4; ++i) {
                tmp8s = 0;
                switch(tmp8u&7) {
                    case 0: tmp8s=(GX.f[i] == EX->f[i]); break;
                    case 1: tmp8s=isless(GX.f[i], EX->f[i]); break;
                    case 2: tmp8s=islessequal(GX.f[i], EX->f[i]); break;
                    case 3: tmp8s=isnan(GX.f[i]) || isnan(EX->f[i]); break;
                    case 4: tmp8s=(GX.f[i] != EX->f[i]); break;
                    case 5: tmp8s=isnan(GX.f[i]) || isnan(EX->f[i]) || isgreaterequal(GX.f[i], EX->f[i]); break;
                    case 6: tmp8s=isnan(GX.f[i]) || isnan(EX->f[i]) || isgreater(GX.f[i], EX->f[i]); break;
                    case 7: tmp8s=!isnan(GX.f[i]) && !isnan(EX->f[i]); break;
                }
                GX.ud[i]=(tmp8s)?0xffffffff:0;
            }
            break;
        case 0xC3:                       /* MOVNTI Ed,Gd */
            nextop = F8;
            GET_ED;
            ED->dword[0] = GD.dword[0];
            break;
        case 0xC4:                       /* PINSRW Gm,Ew,Ib */
            nextop = F8;
            GET_ED;
            tmp8u = F8;
            GM.uw[tmp8u&3] = ED->word[0];   // only low 16bits
            break;
        case 0xC5:                       /* PEXTRW Gw,Em,Ib */
            nextop = F8;
            GET_EM;
            tmp8u = F8;
            GD.dword[0] = EM->uw[tmp8u&3];  // 16bits extract, 0 extended
            break;
        case 0xC6:                      /* SHUFPS Gx, Ex, Ib */
            nextop = F8;
            GET_EX;
            tmp8u = F8;
            for(int i=0; i<2; ++i) {
                eax1.ud[i] = GX.ud[(tmp8u>>(i*2))&3];
            }
            for(int i=2; i<4; ++i) {
                eax1.ud[i] = EX->ud[(tmp8u>>(i*2))&3];
            }
            GX.q[0] = eax1.q[0];
            GX.q[1] = eax1.q[1];
            break;
        case 0xC7:                      /* CMPXCHG8B Gq */
            nextop = F8;
            GET_ED8;
            switch((nextop>>3)&7) {
                case 1:
                    CHECK_FLAGS(emu);
                    tmp32u = ED->dword[0];
                    tmp32u2= ED->dword[1];
                    if(R_EAX == tmp32u && R_EDX == tmp32u2) {
                        SET_FLAG(F_ZF);
                        ED->dword[0] = R_EBX;
                        ED->dword[1] = R_ECX;
                    } else {
                        CLEAR_FLAG(F_ZF);
                        R_EAX = tmp32u;
                        R_EDX = tmp32u2;
                    }
                    break;
                default:
                    return 0;
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
            tmp8s = opcode&7;
            emu->regs[tmp8s].dword[0] = __builtin_bswap32(emu->regs[tmp8s].dword[0]);
            break;

        case 0xD1:                   /* PSRLW Gm,Em */
            nextop = F8;
            GET_EM;
            if(EM->q>15)
                GM.q=0; 
            else {
                tmp8u = EM->ub[0];
                for(int i=0; i<4; ++i)
                    GM.uw[i] >>= tmp8u;
            }
            break;
        case 0xD2:                   /* PSRLD Gm,Em */
            nextop = F8;
            GET_EM;
            if(EM->q>31)
                GM.q=0;
            else {
                tmp8u = EM->ub[0];
                for(int i=0; i<2; ++i)
                    GM.ud[i] >>= tmp8u;
            }
            break;
        case 0xD3:                   /* PSRLQ Gm,Em */
            nextop = F8;
            GET_EM;
            GM.q = (EM->q > 63) ? 0 : (GM.q >> EM->q);
            break;
        case 0xD4:                   /* PADDQ Gm,Em */
            nextop = F8;
            GET_EM;
            GM.sq += EM->sq;
            break;
        case 0xD5:                   /* PMULLW Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i) {
                tmp32s = (int32_t)GM.sw[i] * EM->sw[i];
                GM.sw[i] = tmp32s;
            }
            break;

        case 0xD7:                   /* PMOVMSKB Gd,Em */
            nextop = F8;
            if((nextop&0xC0)==0xC0) {
                GET_EM;
                GD.dword[0] = 0;
                for (int i=0; i<8; ++i)
                    if(EM->ub[i]&0x80)
                        GD.dword[0] |= (1<<i);
                break;
            } else
                return 0;
        case 0xD8:                   /* PSUBUSB Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)GM.ub[i] - EM->ub[i];
                GM.ub[i] = (tmp32s < 0) ? 0 : tmp32s;
            }
            break;
        case 0xD9:                   /* PSUBUSW Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i) {
                tmp32s = (int32_t)GM.uw[i] - EM->uw[i];
                GM.uw[i] = (tmp32s < 0) ? 0 : tmp32s;
            }
            break;
        case 0xDA:                   /* PMINUB Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i=0; i<8; ++i)
                GM.ub[i] = (GM.ub[i]<EM->ub[i])?GM.ub[i]:EM->ub[i];
            break;
        case 0xDB:                   /* PAND Gm,Em */
            nextop = F8;
            GET_EM;
            GM.q &= EM->q;
            break;
        case 0xDC:                   /* PADDUSB Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<8; ++i) {
                tmp32u = (uint32_t)GM.ub[i] + EM->ub[i];
                GM.ub[i] = (tmp32u>255) ? 255 : tmp32u;
            }
            break;
        case 0xDD:                   /* PADDUSW Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i) {
                tmp32u = (uint32_t)GM.uw[i] + EM->uw[i];
                GM.uw[i] = (tmp32u>65535) ? 65535 : tmp32u;
            }
            break;
        case 0xDE:                   /* PMAXUB Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i=0; i<8; ++i)
                GM.ub[i] = (GM.ub[i]>EM->ub[i])?GM.ub[i]:EM->ub[i];
            break;
        case 0xDF:                   /* PANDN Gm,Em */
            nextop = F8;
            GET_EM;
            GM.q = (~GM.q) & EM->q;
            break;
        case 0xE0:                   /* PAVGB Gm, Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<8; ++i)
                    GM.ub[i] = ((uint32_t)GM.ub[i]+EM->ub[i]+1)>>1;
            break;
        case 0xE1:                   /* PSRAW Gm, Em */
            nextop = F8;
            GET_EM;
            if(EM->q>15)
                tmp8u = 16;
            else
                tmp8u = EM->ub[0];
            for(int i=0; i<4; ++i)
                GM.sw[i] >>= tmp8u;
            break;
        case 0xE2:                   /* PSRAD Gm, Em */
            nextop = F8;
            GET_EM;
            if(EM->q>31) {
                for(int i=0; i<2; ++i)
                    GM.sd[i] = (GM.sd[i]<0)?-1:0;
            } else {
                tmp8u = EM->ub[0];
                for(int i=0; i<2; ++i)
                    GM.sd[i] >>= tmp8u;
            }
            break;
        case 0xE3:                   /* PAVGW Gm, Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i)
                GM.uw[i] = ((uint32_t)GM.uw[i]+EM->uw[i]+1)>>1;
            break;
        case 0xE4:                   /* PMULHUW Gm, Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i) {
                tmp32u = (uint32_t)GM.uw[i] * EM->uw[i];
                GM.uw[i] = (tmp32u>>16)&0xffff;
            }
            break;
        case 0xE5:                   /* PMULHW Gm, Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i) {
                tmp32s = (int32_t)GM.sw[i] * EM->sw[i];
                GM.uw[i] = (tmp32s>>16)&0xffff;
            }
            break;

        case 0xE7:                   /* MOVNTQ Em,Gm */
            nextop = F8;
            if((nextop&0xC0)==0xC0)
                return 0;
            GET_EM;
            EM->q = GM.q;
            break;
        case 0xE8:                   /* PSUBSB Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)GM.sb[i] - EM->sb[i];
                GM.sb[i] = (tmp32s>127)?127:((tmp32s<-128)?-128:tmp32s);
            }
            break;
        case 0xE9:                   /* PSUBSW Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i) {
                tmp32s = (int32_t)GM.sw[i] - EM->sw[i];
                GM.sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
            }
            break;
        case 0xEA:                   /* PMINSW Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i=0; i<4; ++i)
                GM.sw[i] = (GM.sw[i]<EM->sw[i])?GM.sw[i]:EM->sw[i];
            break;
        case 0xEB:                   /* POR Gm, Em */
            nextop = F8;
            GET_EM;
            GM.q |= EM->q;
            break;
        case 0xEC:                   /* PADDSB Gm, Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)GM.sb[i] + EM->sb[i];
                GM.sb[i] = (tmp32s>127)?127:((tmp32s<-128)?-128:tmp32s);
            }
            break;
        case 0xED:                   /* PADDSW Gm, Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i) {
                tmp32s = (int32_t)GM.sw[i] + EM->sw[i];
                GM.sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
            }
            break;
        case 0xEE:                   /* PMAXSW Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i=0; i<4; ++i)
                GM.sw[i] = (GM.sw[i]>EM->sw[i])?GM.sw[i]:EM->sw[i];
            break;
        case 0xEF:                   /* PXOR Gm, Em */
            nextop = F8;
            GET_EM;
            GM.q ^= EM->q;
            break;

        case 0xF1:                   /* PSLLW Gm, Em */
            nextop = F8;
            GET_EM;
            if(EM->q>15)
                GM.q = 0;
            else {
                tmp8u = EM->ub[0];
                for(int i=0; i<4; ++i)
                    GM.sw[i] <<= tmp8u;
            }
            break;
        case 0xF2:                   /* PSLLD Gm, Em */
            nextop = F8;
            GET_EM;
            if(EM->q>31)
                GM.q = 0;
            else {
                tmp8u = EM->ub[0];
                for(int i=0; i<2; ++i)
                    GM.sd[i] <<= tmp8u;
            }
            break;
        case 0xF3:                   /* PSLLQ Gm, Em */
            nextop = F8;
            GET_EM;
            GM.q = (EM->q > 63) ? 0 : (GM.q << EM->ub[0]);
            break;
        case 0xF4:                   /* PMULUDQ Gm,Em */
            nextop = F8;
            GET_EM;
            GM.q = (uint64_t)GM.ud[0] * (uint64_t)EM->ud[0];
            break;
        case 0xF5:                   /* PMADDWD Gm, Em */
            nextop = F8;
            GET_EM;
            for (int i=0; i<2; ++i) {
                int offset = i * 2;

                tmp32s = (int32_t)GM.sw[offset + 0] * EM->sw[offset + 0];
                tmp32s2 = (int32_t)GM.sw[offset + 1] * EM->sw[offset + 1];
                GM.sd[i] = tmp32s + tmp32s2;
            }
            break;
        case 0xF6:                   /* PSADBW Gm, Em */
            nextop = F8;
            GET_EM;
            tmp32u = 0;
            for (int i=0; i<8; ++i)
                tmp32u += (GM.ub[i]>EM->ub[i])?(GM.ub[i] - EM->ub[i]):(EM->ub[i] - GM.ub[i]);
            GM.q = tmp32u;
            break;
        case 0xF7:                   /* MASKMOVQ Gm, Em */
            nextop = F8;
            if((nextop&0xC0)==0xC0) {
                GET_EM;
                for (int i=0; i<8; ++i)
                    if(EM->ub[i]&0x80) ((uint8_t*)(R_EDI))[i] = GM.ub[i];
            } else
                return 0;
            break;
        case 0xF8:                   /* PSUBB Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<8; ++i)
                GM.sb[i] -= EM->sb[i];
            break;
        case 0xF9:                   /* PSUBW Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i)
                GM.sw[i] -= EM->sw[i];
            break;
        case 0xFA:                   /* PSUBD Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<2; ++i)
                GM.sd[i] -= EM->sd[i];
            break;
        case 0xFB:                   /* PSUBQ Gm,Em */
            nextop = F8;
            GET_EM;
            GM.sq -= EM->sq;
            break;
        case 0xFC:                   /* PADDB Gm, Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<8; ++i)
                GM.sb[i] += EM->sb[i];
            break;
        case 0xFD:                   /* PADDW Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i)
                GM.sw[i] += EM->sw[i];
            break;
        case 0xFE:                   /* PADDD Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<2; ++i)
                GM.sd[i] += EM->sd[i];
            break;

        default:
            return 0;
    }
    return addr;
}
