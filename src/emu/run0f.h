    opcode = F8;
    goto *opcodes0f[opcode];
        _0f_0x00:                       /* VERx Ed */
            nextop = F8;
            switch((nextop>>3)&7) {
                case 4: //VERR
                case 5: //VERW
                    GET_EW;
                    if(!EW->word[0])
                        CLEAR_FLAG(F_ZF);
                    else
                        SET_FLAG(F_ZF); // should test if selector is ok
                    break;
                default:
                    goto _default;
            }
            NEXT;
        _0f_0x01:                      
            nextop = F8;
            switch((nextop>>3)&7) {
                case 4:                 /* SMSW Ew */
                    GET_ED;
                    // dummy for now... Do I need to track CR0 state?
                    ED->word[0] = (1<<0) | (1<<4); // only PE and ET set...
                    break;
                default:
                    goto _default;
            }
            NEXT;

        _0f_0x0B:                      /* UD2 */
            kill(getpid(), SIGILL);              // this is undefined instruction
            NEXT;
        _0f_0x10:                      /* MOVUPS Gx,Ex */
            nextop = F8;
            GET_EX;
            memcpy(&GX, EX, 16); // unaligned, so carreful
            NEXT;
        _0f_0x11:                      /* MOVUPS Ex,Gx */
            nextop = F8;
            GET_EX;
            memcpy(EX, &GX, 16); // unaligned, so carreful
            NEXT;
        _0f_0x12:                      
            nextop = F8;
            GET_EX;
            if((nextop&0xC0)==0xC0)    /* MOVHLPS Gx,Ex */
                GX.q[0] = EX->q[1];
            else
                GX.q[0] = EX->q[0];    /* MOVLPS Gx,Ex */
            NEXT;
        _0f_0x13:                      /* MOVLPS Ex,Gx */
            nextop = F8;
            GET_EX;
            EX->q[0] = GX.q[0];
            NEXT;
        _0f_0x14:                      /* UNPCKLPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.ud[3] = EX->ud[1];
            GX.ud[2] = GX.ud[1];
            GX.ud[1] = EX->ud[0];
            NEXT;
        _0f_0x15:                      /* UNPCKHPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.ud[0] = GX.ud[2];
            GX.ud[1] = EX->ud[2];
            GX.ud[2] = GX.ud[3];
            GX.ud[3] = EX->ud[3];
            NEXT;
        _0f_0x16:                      /* MOVHPS Gx,Ex */
            nextop = F8;               /* MOVLHPS Gx,Ex (Ex==reg) */
            GET_EX;
            GX.q[1] = EX->q[0];
            NEXT;
        _0f_0x17:                      /* MOVHPS Ex,Gx */
            nextop = F8;
            GET_EX;
            EX->q[0] = GX.q[1];
            NEXT;
        _0f_0x18:                       /* PREFETCHh Ed */
            nextop = F8;
            GET_ED;
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
            NEXT;

        _0f_0x1A:                      /* NOP (multi-byte) / ignored BNDLDX */
            nextop = F8;
            GET_ED;
            NEXT;
        _0f_0x1B:                      /* NOP (multi-byte) / ignored BNDSTX */
            nextop = F8;
            GET_ED;
            NEXT;

        _0f_0x1F:                      /* NOP (multi-byte) */
            nextop = F8;
            GET_ED;
            NEXT;

        _0f_0x28:                      /* MOVAPS Gx,Ex */
            nextop = F8;
            GET_EX;
            GX.q[0] = EX->q[0];
            GX.q[1] = EX->q[1];
            NEXT;
        _0f_0x29:                      /* MOVAPS Ex,Gx */
            nextop = F8;
            GET_EX;
            EX->q[0] = GX.q[0];
            EX->q[1] = GX.q[1];
            NEXT;
        _0f_0x2A:                      /* CVTPI2PS Gx, Em */
            nextop = F8;
            GET_EM;
            GX.f[0] = EM->sd[0];
            GX.f[1] = EM->sd[1];
            NEXT;
        _0f_0x2B:                      /* MOVNTPS Ex,Gx */
            nextop = F8;
            GET_EX;
            EX->q[0] = GX.q[0];
            EX->q[1] = GX.q[1];
            NEXT;
        _0f_0x2C:                      /* CVTTPS2PI Gm, Ex */
            nextop = F8;
            GET_EX;
            GM.sd[1] = EX->f[1];
            GM.sd[0] = EX->f[0];
            NEXT;
        _0f_0x2D:                      /* CVTPS2PI Gm, Ex */
            // rounding should be done; and indefinite integer should also be assigned if overflow or NaN/Inf
            nextop = F8;
            GET_EX;
            switch(emu->round) {
                case ROUND_Nearest:
                    GM.sd[1] = floorf(EX->f[1]+0.5f);
                    GM.sd[0] = floorf(EX->f[0]+0.5f);
                    break;
                case ROUND_Down:
                    GM.sd[1] = floorf(EX->f[1]);
                    GM.sd[0] = floorf(EX->f[0]);
                    break;
                case ROUND_Up:
                    GM.sd[1] = ceilf(EX->f[1]);
                    GM.sd[0] = ceilf(EX->f[0]);
                    break;
                case ROUND_Chop:
                    GM.sd[1] = EX->f[1];
                    GM.sd[0] = EX->f[0];
                    break;
            }
            NEXT;
        _0f_0x2E:                      /* UCOMISS Gx, Ex */
            // same for now
        _0f_0x2F:                      /* COMISS Gx, Ex */
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
            NEXT;

        _0f_0x31:                   /* RDTSC */
            tmp64u = ReadTSC(emu);
            R_EDX = tmp64u>>32;
            R_EAX = tmp64u&0xFFFFFFFF;
            NEXT;
        
        _0f_0x38:  // these are some SSE3 opcodes
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
                            GM.ub[i] = eax1.ub[EM->ub[i]&7];
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

                default:
                    goto _default;
            }
            NEXT;
            
        #define GOCOND(BASE, PREFIX, CONDITIONAL) \
        _0f_##BASE##_0:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_OF))               \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_1:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_OF))              \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_2:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_CF))               \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_3:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_CF))              \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_4:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_ZF))               \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_5:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_ZF))              \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_6:                          \
            PREFIX                              \
            if((ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF)))  \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_7:                          \
            PREFIX                              \
            if(!(ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF))) \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_8:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF))               \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_9:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_SF))              \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_A:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_PF))               \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_B:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_PF))              \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_C:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))  \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_D:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF)) \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_E:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_ZF) || (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))) \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_F:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_ZF) && (ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF))) \
                CONDITIONAL                     \
            NEXT;

        GOCOND(0x40
            , nextop = F8;
            GET_ED;
            CHECK_FLAGS(emu);
            , GD.dword[0] = ED->dword[0];
        )                               /* 0x40 -> 0x4F CMOVxx Gd,Ed */ // conditional move, no sign
        GOCOND(0x80
            , tmp32s = F32S; CHECK_FLAGS(emu);
            , ip += tmp32s;
        )                               /* 0x80 -> 0x8F Jxx */
        GOCOND(0x90
            , nextop = F8; CHECK_FLAGS(emu);
            GET_EB;
            , EB->byte[0]=1; else EB->byte[0]=0;
        )                               /* 0x90 -> 0x9F SETxx Eb */

        #undef GOCOND

        _0f_0x50:                      /* MOVMSKPS Gd, Ex */
            nextop = F8;
            GET_EX;
            GD.dword[0] = 0;
            for(int i=0; i<4; ++i)
                GD.dword[0] |= ((EX->ud[i]>>31)&1)<<i;
            NEXT;
        _0f_0x51:                      /* SQRTPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.f[i] = sqrtf(EX->f[i]);
            NEXT;
        _0f_0x52:                      /* RSQRTPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.f[i] = 1.0f/sqrtf(EX->f[i]);
            NEXT;
        _0f_0x53:                      /* RCPPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.f[i] = 1.0f/EX->f[i];
            NEXT;
        _0f_0x54:                      /* ANDPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.ud[i] &= EX->ud[i];
            NEXT;
        _0f_0x55:                      /* ANDNPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.ud[i] = (~GX.ud[i]) & EX->ud[i];
            NEXT;
        _0f_0x56:                      /* ORPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.ud[i] |= EX->ud[i];
            NEXT;
        _0f_0x57:                      /* XORPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.ud[i] ^= EX->ud[i];
            NEXT;
        _0f_0x58:                      /* ADDPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.f[i] += EX->f[i];
            NEXT;
        _0f_0x59:                      /* MULPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.f[i] *= EX->f[i];
            NEXT;
        _0f_0x5A:                      /* CVTPS2PD Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.d[1] = EX->f[1];
            GX.d[0] = EX->f[0];
            NEXT;
        _0f_0x5B:                      /* CVTDQ2PS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.f[0] = EX->sd[0];
            GX.f[1] = EX->sd[1];
            GX.f[2] = EX->sd[2];
            GX.f[3] = EX->sd[3];
            NEXT;
        _0f_0x5C:                      /* SUBPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.f[i] -= EX->f[i];
            NEXT;
        _0f_0x5D:                      /* MINPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i) {
                if (isnan(GX.f[i]) || isnan(EX->f[i]) || isless(EX->f[i], GX.f[i]))
                    GX.f[i] = EX->f[i];
            }
            NEXT;
        _0f_0x5E:                      /* DIVPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i)
                GX.f[i] /= EX->f[i];
            NEXT;
        _0f_0x5F:                      /* MAXPS Gx, Ex */
            nextop = F8;
            GET_EX;
            for(int i=0; i<4; ++i) {
                if (isnan(GX.f[i]) || isnan(EX->f[i]) || isgreater(EX->f[i], GX.f[i]))
                    GX.f[i] = EX->f[i];
            }
            NEXT;
        _0f_0x60:                      /* PUNPCKLBW Gm, Em */
            nextop = F8;
            GET_EM;
            GM.ub[7] = EM->ub[3];
            GM.ub[6] = GM.ub[3];
            GM.ub[5] = EM->ub[2];
            GM.ub[4] = GM.ub[2];
            GM.ub[3] = EM->ub[1];
            GM.ub[2] = GM.ub[1];
            GM.ub[1] = EM->ub[0];
            NEXT;
        _0f_0x61:                      /* PUNPCKLWD Gm, Em */
            nextop = F8;
            GET_EM;
            GM.uw[3] = EM->uw[1];
            GM.uw[2] = GM.uw[1];
            GM.uw[1] = EM->uw[0];
            NEXT;
        _0f_0x62:                      /* PUNPCKLDQ Gm, Em */
            nextop = F8;
            GET_EM;
            GM.ud[1] = EM->ud[0];
            NEXT;
        _0f_0x63:                      /* PACKSSWB Gm, Em */
            nextop = F8;
            GET_EM;
            GM.sb[0] = (GM.sw[0] > 127) ? 127 : ((GM.sw[0] < -128) ? -128 : GM.sw[0]);
            GM.sb[1] = (GM.sw[1] > 127) ? 127 : ((GM.sw[1] < -128) ? -128 : GM.sw[1]);
            GM.sb[2] = (GM.sw[2] > 127) ? 127 : ((GM.sw[2] < -128) ? -128 : GM.sw[2]);
            GM.sb[3] = (GM.sw[3] > 127) ? 127 : ((GM.sw[3] < -128) ? -128 : GM.sw[3]);
            GM.sb[4] = (EM->sw[0] > 127) ? 127 : ((EM->sw[0] < -128) ? -128 : EM->sw[0]);
            GM.sb[5] = (EM->sw[1] > 127) ? 127 : ((EM->sw[1] < -128) ? -128 : EM->sw[1]);
            GM.sb[6] = (EM->sw[2] > 127) ? 127 : ((EM->sw[2] < -128) ? -128 : EM->sw[2]);
            GM.sb[7] = (EM->sw[3] > 127) ? 127 : ((EM->sw[3] < -128) ? -128 : EM->sw[3]);
            NEXT;
        _0f_0x64:                       /* PCMPGTB Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i = 0; i < 8; i++) {
                GM.ub[i] = (GM.sb[i] > EM->sb[i]) ? 0xFF : 0;
            }
            NEXT;
        _0f_0x65:                       /* PCMPGTW Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i = 0; i < 4; i++) {
                GM.uw[i] = (GM.sw[i] > EM->sw[i]) ? 0xFFFF : 0;
            }
            NEXT;
        _0f_0x66:                       /* PCMPGTD Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i = 0; i < 2; i++) {
                GM.ud[i] = (GM.sd[i] > EM->sd[i]) ? 0xFFFFFFFF : 0;
            }
            NEXT;
        _0f_0x67:                       /* PACKUSWB Gm, Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i)
                GM.ub[i] = (GM.sw[i]<0)?0:((GM.sw[i]>0xff)?0xff:GM.sw[i]);
            for(int i=0; i<4; ++i)
                GM.ub[4+i] = (EM->sw[i]<0)?0:((EM->sw[i]>0xff)?0xff:EM->sw[i]);
            NEXT;
        _0f_0x68:                       /* PUNPCKHBW Gm,Em */
            nextop = F8;
            GET_EM;
            if(EM==&GM) {eam1 = GM; EM = &eam1;}   // copy is needed
            for(int i=0; i<4; ++i)
                GM.ub[2 * i] = GM.ub[i + 4];
            for(int i=0; i<4; ++i)
                GM.ub[2 * i + 1] = EM->ub[i + 4];
            NEXT;
        _0f_0x69:                       /* PUNPCKHWD Gm,Em */
            nextop = F8;
            GET_EM;
            if(EM==&GM) {eam1 = GM; EM = &eam1;}   // copy is needed
            for(int i=0; i<2; ++i)
                GM.uw[2 * i] = GM.uw[i + 2];
            for(int i=0; i<2; ++i)
                GM.uw[2 * i + 1] = EM->uw[i + 2];
            NEXT;
        _0f_0x6A:                       /* PUNPCKHDQ Gm,Em */
            nextop = F8;
            GET_EM;
            if(EM==&GM) {eam1 = GM; EM = &eam1;}   // copy is needed
            GM.ud[0] = GM.ud[1];
            GM.ud[1] = EM->ud[1];
            NEXT;
        _0f_0x6B:                       /* PACKSSDW Gm,Em */
            nextop = F8;
            GET_EM;
            if(EM==&GM) {eam1 = GM; EM = &eam1;}   // copy is needed
            for(int i=0; i<2; ++i)
                GM.sw[i] = (GM.sd[i]<-32768)?-32768:((GM.sd[i]>32767)?32767:GM.sd[i]);
            for(int i=0; i<2; ++i)
                GM.sw[2+i] = (EM->sd[i]<-32768)?-32768:((EM->sd[i]>32767)?32767:EM->sd[i]);
            NEXT;

        _0f_0x6E:                      /* MOVD Gm, Ed */
            nextop = F8;
            GET_ED;
            GM.q = ED->dword[0];    // zero extended
            NEXT;
        _0f_0x6F:                      /* MOVQ Gm, Em */
            nextop = F8;
            GET_EM;
            GM.q = EM->q;
            NEXT;
        _0f_0x70:                       /* PSHUFW Gm, Em, Ib */
            nextop = F8;
            GET_EM;
            tmp8u = F8;
            if(EM==&GM) {eam1 = GM; EM = &eam1;}   // copy is needed
            for(int i=0; i<4; ++i)
                GM.uw[i] = EM->uw[(tmp8u>>(i*2))&3];
            NEXT;
        _0f_0x71:  /* GRP */
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
                    goto _default;
            }
            NEXT;
        _0f_0x72:  /* GRP */
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
                    goto _default;
            }
            NEXT;
        _0f_0x73:  /* GRP */
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
                    goto _default;
            }
            NEXT;
        _0f_0x74:                       /* PCMPEQB Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i = 0; i < 8; i++) {
                GM.ub[i] = (GM.sb[i] == EM->sb[i]) ? 0xFF : 0;
            }
            NEXT;
        _0f_0x75:                       /* PCMPEQW Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i = 0; i < 4; i++) {
                GM.uw[i] = (GM.sw[i] == EM->sw[i]) ? 0xFFFF : 0;
            }
            NEXT;
        _0f_0x76:                       /* PCMPEQD Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i = 0; i < 2; i++) {
                GM.ud[i] = (GM.sd[i] == EM->sd[i]) ? 0xFFFFFFFF : 0;
            }
            NEXT;
        _0f_0x77:                      /* EMMS */
            // empty MMX, FPU now usable
            emu->top = 0;
            emu->fpu_stack = 0;
            NEXT;

        _0f_0x7E:                       /* MOVD Ed, Gm */
            nextop = F8;
            GET_ED;
            ED->dword[0] = GM.ud[0];
            NEXT;
        _0f_0x7F:                      /* MOVQ Em, Gm */
            nextop = F8;
            GET_EM;
            EM->q = GM.q;
            NEXT;

        _0f_0xA0:                      /* PUSH FS */
            Push(emu, emu->segs[_FS]);    // even if a segment is a 16bits, a 32bits push/pop is done
            NEXT;
        _0f_0xA1:                      /* POP FS */
            emu->segs[_FS] = Pop(emu);    // no check, no use....
            emu->segs_serial[_FS] = 0;
            NEXT;
        _0f_0xA2:                      /* CPUID */
            tmp32u = R_EAX;
            my_cpuid(emu, tmp32u);
            NEXT;
        _0f_0xA3:                      /* BT Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp8u = GD.byte[0];
            if((nextop&0xC0)!=0xC0)
            {
                ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
            }
            tmp8u&=31;
            if(ED->dword[0] & (1<<tmp8u))
                SET_FLAG(F_CF);
            else
                CLEAR_FLAG(F_CF);
            NEXT;
        _0f_0xA4:                      /* SHLD Ed,Gd,Ib */
        _0f_0xA5:                      /* SHLD Ed,Gd,CL */
            nextop = F8;
            GET_ED;
            tmp8u = (opcode==0xA4)?(F8):R_CL;
            ED->dword[0] = shld32(emu, ED->dword[0], GD.dword[0], tmp8u);
            NEXT;

        _0f_0xA8:                      /* PUSH GS */
            Push(emu, emu->segs[_GS]);    // even if a segment is a 16bits, a 32bits push/pop is done
            NEXT;
        _0f_0xA9:                      /* POP GS */
            emu->segs[_GS] = Pop(emu);
            emu->segs_serial[_GS] = 0;
            NEXT;


        _0f_0xAB:                      /* BTS Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp8u = GD.byte[0];
            if((nextop&0xC0)!=0xC0)
            {
                ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
            }
            tmp8u&=31;
            if(ED->dword[0] & (1<<tmp8u))
                SET_FLAG(F_CF);
            else {
                ED->dword[0] |= (1<<tmp8u);
                CLEAR_FLAG(F_CF);
            }
            NEXT;
        _0f_0xAC:                      /* SHRD Ed,Gd,Ib */
        _0f_0xAD:                      /* SHRD Ed,Gd,CL */
            nextop = F8;
            GET_ED;
            tmp8u = (opcode==0xAC)?(F8):R_CL;
            ED->dword[0] = shrd32(emu, ED->dword[0], GD.dword[0], tmp8u);
            NEXT;

        _0f_0xAE:                      /* Grp Ed (SSE) */
            nextop = F8;
            if((nextop&0xF8)==0xE8) {
                NEXT;                   /* LFENCE */
            }
            if((nextop&0xF8)==0xF0) {
                NEXT;                   /* MFENCE */
            }
            if((nextop&0xF8)==0xF8) {
                NEXT;                   /* SFENCE */
            }
            GET_ED;
            switch((nextop>>3)&7) {
                case 0:                 /* FXSAVE m512byte */
                    // should save flags & all
                    // copy MMX regs...
                    for(int i=0; i<8; ++i)
                        memcpy(((void*)(ED))+32+i*16, &emu->mmx[0], sizeof(emu->mmx[0]));
                    // copy SSE regs
                    memcpy(((void*)(ED))+160, &emu->xmm[0], sizeof(emu->xmm));
                    // put also FPU regs in a reserved area...
                    for(int i=0; i<8; ++i)
                        memcpy(((void*)(ED))+416+i*8, &emu->fpu[0], sizeof(emu->fpu[0]));
                    break;
                case 1:                 /* FXRSTOR m512byte */
                    // should restore flags & all
                    // copy back MMX regs...
                    for(int i=0; i<8; ++i)
                        memcpy(&emu->mmx[i], ((void*)(ED))+32+i*16, sizeof(emu->mmx[0]));
                    // copy SSE regs
                    memcpy(&emu->xmm[0], ((void*)(ED))+160/4, sizeof(emu->xmm));
                    for(int i=0; i<8; ++i)
                        memcpy(&emu->fpu[0], ((void*)(ED))+416+i*8, sizeof(emu->fpu[0]));
                    break;
                case 2:                 /* LDMXCSR Md */
                    emu->mxcsr = ED->dword[0];
                    break;
                case 3:                 /* STMXCSR Md */
                    ED->dword[0] = emu->mxcsr;
                    break;
                default:
                    goto _default;
            }
            NEXT;
        _0f_0xAF:                      /* IMUL Gd,Ed */
            nextop = F8;
            GET_ED;
            GD.dword[0] = imul32(emu, GD.dword[0], ED->dword[0]);
            NEXT;
        _0f_0xB0:                      /* CMPXCHG Eb,Gb */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_EB;
            cmp8(emu, R_AL, EB->byte[0]);
            if(ACCESS_FLAG(F_ZF)) {
                EB->byte[0] = GB;
            } else {
                R_AL = EB->byte[0];
            }
            NEXT;
        _0f_0xB1:                      /* CMPXCHG Ed,Gd */
            nextop = F8;
            GET_ED;
            cmp32(emu, R_EAX, ED->dword[0]);
            if(ACCESS_FLAG(F_ZF)) {
                ED->dword[0] = GD.dword[0];
            } else {
                R_EAX = ED->dword[0];
            }
            NEXT;

        _0f_0xB3:                      /* BTR Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp8u = GD.byte[0];
            if((nextop&0xC0)!=0xC0)
            {
                ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
            }
            tmp8u&=31;
            if(ED->dword[0] & (1<<tmp8u)) {
                SET_FLAG(F_CF);
                ED->dword[0] ^= (1<<tmp8u);
            } else
                CLEAR_FLAG(F_CF);
            NEXT;

        _0f_0xB6:                      /* MOVZX Gd,Eb */
            nextop = F8;
            GET_EB;
            GD.dword[0] = EB->byte[0];
            NEXT;
        _0f_0xB7:                      /* MOVZX Gd,Ew */
            nextop = F8;
            GET_EW;
            GD.dword[0] = EW->word[0];
            NEXT;

        _0f_0xBA:                      
            nextop = F8;
            switch((nextop>>3)&7) {
                case 4:                 /* BT Ed,Ib */
                    CHECK_FLAGS(emu);
                    GET_ED;
                    tmp8u = F8;
                    if((nextop&0xC0)!=0xC0)
                    {
                        ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
                    }
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
                    if((nextop&0xC0)!=0xC0)
                    {
                        ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
                    }
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
                    if((nextop&0xC0)!=0xC0)
                    {
                        ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
                    }
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
                    if((nextop&0xC0)!=0xC0)
                    {
                        ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
                    }
                    tmp8u&=31;
                    if(ED->dword[0] & (1<<tmp8u))
                        SET_FLAG(F_CF);
                    else
                        CLEAR_FLAG(F_CF);
                    ED->dword[0] ^= (1<<tmp8u);
                    break;

                default:
                    goto _default;
            }
            NEXT;
        _0f_0xBB:                      /* BTC Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp8u = GD.byte[0];
            if((nextop&0xC0)!=0xC0)
            {
                ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
            }
            tmp8u&=31;
            if(ED->dword[0] & (1<<tmp8u))
                SET_FLAG(F_CF);
            else
                CLEAR_FLAG(F_CF);
            ED->dword[0] ^= (1<<tmp8u);
            NEXT;
        _0f_0xBC:                      /* BSF Ed,Gd */
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
            NEXT;
        _0f_0xBD:                      /* BSR Ed,Gd */
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
            NEXT;
        _0f_0xBE:                      /* MOVSX Gd,Eb */
            nextop = F8;
            GET_EB;
            GD.sdword[0] = EB->sbyte[0];
            NEXT;
        _0f_0xBF:                      /* MOVSX Gd,Ew */
            nextop = F8;
            GET_EW;
            GD.sdword[0] = EW->sword[0];
            NEXT;

        _0f_0xC0:                      /* XADD Gb,Eb */
            nextop = F8;
            GET_EB;
            tmp8u = add8(emu, EB->byte[0], GB);
            GB = EB->byte[0];
            EB->byte[0] = tmp8u;
            NEXT;
        _0f_0xC1:                      /* XADD Gd,Ed */
            nextop = F8;
            GET_ED;
            tmp32u = add32(emu, ED->dword[0], GD.dword[0]);
            GD.dword[0] = ED->dword[0];
            ED->dword[0] = tmp32u;
            NEXT;
        _0f_0xC2:                      /* CMPPS Gx, Ex, Ib */
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
            NEXT;
        _0f_0xC3:                       /* MOVNTI Ed,Gd */
            nextop = F8;
            GET_ED;
            ED->dword[0] = GD.dword[0];
            NEXT;
        _0f_0xC4:                       /* PINSRW Gm,Ew,Ib */
            nextop = F8;
            GET_ED;
            tmp8u = F8;
            GM.uw[tmp8u&3] = ED->word[0];   // only low 16bits
            NEXT;
        _0f_0xC5:                       /* PEXTRW Gw,Em,Ib */
            nextop = F8;
            GET_EM;
            tmp8u = F8;
            GD.dword[0] = EM->uw[tmp8u&3];  // 16bits extract, 0 extended
            NEXT;
        _0f_0xC6:                      /* SHUFPS Gx, Ex, Ib */
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
            NEXT;
        _0f_0xC7:                      /* CMPXCHG8B Gq */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
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
            NEXT;
        _0f_0xC8:
        _0f_0xC9:
        _0f_0xCA:
        _0f_0xCB:
        _0f_0xCC:
        _0f_0xCD:
        _0f_0xCE:
        _0f_0xCF:                  /* BSWAP reg */
            tmp8s = opcode&7;
            emu->regs[tmp8s].dword[0] = __builtin_bswap32(emu->regs[tmp8s].dword[0]);
            NEXT;

        _0f_0xD1:                   /* PSRLW Gm,Em */
            nextop = F8;
            GET_EM;
            if(EM->q>15)
                GM.q=0; 
            else {
                tmp8u = EM->ub[0];
                for(int i=0; i<4; ++i)
                    GM.uw[i] >>= tmp8u;
            }
            NEXT;
        _0f_0xD2:                   /* PSRLD Gm,Em */
            nextop = F8;
            GET_EM;
            if(EM->q>31)
                GM.q=0;
            else {
                tmp8u = EM->ub[0];
                for(int i=0; i<2; ++i)
                    GM.ud[i] >>= tmp8u;
            }
            NEXT;
        _0f_0xD3:                   /* PSRLQ Gm,Em */
            nextop = F8;
            GET_EM;
            GM.q = (EM->q > 63) ? 0 : (GM.q >> EM->q);
            NEXT;
        _0f_0xD4:                   /* PADDQ Gm,Em */
            nextop = F8;
            GET_EM;
            GM.sq += EM->sq;
            NEXT;
        _0f_0xD5:                   /* PMULLW Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i) {
                tmp32s = (int32_t)GM.sw[i] * EM->sw[i];
                GM.sw[i] = tmp32s;
            }
            NEXT;

        _0f_0xD7:                   /* PMOVMSKB Gd,Em */
            nextop = F8;
            if((nextop&0xC0)==0xC0) {
                GET_EM;
                GD.dword[0] = 0;
                for (int i=0; i<8; ++i)
                    if(EM->ub[i]&0x80)
                        GD.dword[0] |= (1<<i);
                NEXT;
            } else
                goto _default;
        _0f_0xD8:                   /* PSUBUSB Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)GM.ub[i] - EM->ub[i];
                GM.ub[i] = (tmp32s < 0) ? 0 : tmp32s;
            }
            NEXT;
        _0f_0xD9:                   /* PSUBUSW Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i) {
                tmp32s = (int32_t)GM.uw[i] - EM->uw[i];
                GM.uw[i] = (tmp32s < 0) ? 0 : tmp32s;
            }
            NEXT;
        _0f_0xDA:                   /* PMINUB Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i=0; i<8; ++i)
                GM.ub[i] = (GM.ub[i]<EM->ub[i])?GM.ub[i]:EM->ub[i];
            NEXT;
        _0f_0xDB:                   /* PAND Gm,Em */
            nextop = F8;
            GET_EM;
            GM.q &= EM->q;
            NEXT;
        _0f_0xDC:                   /* PADDUSB Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<8; ++i) {
                tmp32u = (uint32_t)GM.ub[i] + EM->ub[i];
                GM.ub[i] = (tmp32u>255) ? 255 : tmp32u;
            }
            NEXT;
        _0f_0xDD:                   /* PADDUSW Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i) {
                tmp32u = (uint32_t)GM.uw[i] + EM->uw[i];
                GM.uw[i] = (tmp32u>65535) ? 65535 : tmp32u;
            }
            NEXT;
        _0f_0xDE:                   /* PMAXUB Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i=0; i<8; ++i)
                GM.ub[i] = (GM.ub[i]>EM->ub[i])?GM.ub[i]:EM->ub[i];
            NEXT;
        _0f_0xDF:                   /* PANDN Gm,Em */
            nextop = F8;
            GET_EM;
            GM.q = (~GM.q) & EM->q;
            NEXT;
        _0f_0xE0:                   /* PAVGB Gm, Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<8; ++i)
                    GM.ub[i] = ((uint32_t)GM.ub[i]+EM->ub[i]+1)>>1;
            NEXT;
        _0f_0xE1:                   /* PSRAW Gm, Em */
            nextop = F8;
            GET_EM;
            if(EM->q>15)
                tmp8u = 16;
            else
                tmp8u = EM->ub[0];
            for(int i=0; i<4; ++i)
                GM.sw[i] >>= tmp8u;
            NEXT;
        _0f_0xE2:                   /* PSRAD Gm, Em */
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
            NEXT;
        _0f_0xE3:                   /* PSRAQ Gm, Em */
            nextop = F8;
            GET_EM;
            if(EM->q>63)
                tmp8u = 64;
            else
                tmp8u = EM->ub[0];
            GM.sq >>= tmp8u;
            NEXT;
        _0f_0xE4:                   /* PMULHUW Gm, Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i) {
                tmp32u = (int32_t)GM.uw[i] * EM->uw[i];
                GM.uw[i] = (tmp32u>>16)&0xffff;
            }
            NEXT;
        _0f_0xE5:                   /* PMULHW Gm, Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i) {
                tmp32s = (int32_t)GM.sw[i] * EM->sw[i];
                GM.uw[i] = (tmp32s>>16)&0xffff;
            }
            NEXT;

        _0f_0xE7:                   /* MOVNTQ Em,Gm */
            nextop = F8;
            if((nextop&0xC0)==0xC0)
                goto _default;
            GET_EM;
            EM->q = GM.q;
            NEXT;
        _0f_0xE8:                   /* PSUBSB Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)GM.sb[i] - EM->sb[i];
                GM.sb[i] = (tmp32s>127)?127:((tmp32s<-128)?-128:tmp32s);
            }
            NEXT;
        _0f_0xE9:                   /* PSUBSW Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i) {
                tmp32s = (int32_t)GM.sw[i] - EM->sw[i];
                GM.sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
            }
            NEXT;
        _0f_0xEA:                   /* PMINSW Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i=0; i<4; ++i)
                GM.sw[i] = (GM.sw[i]<EM->sw[i])?GM.sw[i]:EM->sw[i];
            NEXT;
        _0f_0xEB:                   /* POR Gm, Em */
            nextop = F8;
            GET_EM;
            GM.q |= EM->q;
            NEXT;
        _0f_0xEC:                   /* PADDSB Gm, Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<8; ++i) {
                tmp32s = (int32_t)GM.sb[i] + EM->sb[i];
                GM.sb[i] = (tmp32s>127)?127:((tmp32s<-128)?-128:tmp32s);
            }
            NEXT;
        _0f_0xED:                   /* PADDSW Gm, Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i) {
                tmp32s = (int32_t)GM.sw[i] + EM->sw[i];
                GM.sw[i] = (tmp32s>32767)?32767:((tmp32s<-32768)?-32768:tmp32s);
            }
            NEXT;
        _0f_0xEE:                   /* PMAXSW Gm,Em */
            nextop = F8;
            GET_EM;
            for (int i=0; i<4; ++i)
                GM.sw[i] = (GM.sw[i]>EM->sw[i])?GM.sw[i]:EM->sw[i];
            NEXT;
        _0f_0xEF:                   /* PXOR Gm, Em */
            nextop = F8;
            GET_EM;
            GM.q ^= EM->q;
            NEXT;

        _0f_0xF1:                   /* PSLLW Gm, Em */
            nextop = F8;
            GET_EM;
            if(EM->q>15)
                GM.q = 0;
            else {
                tmp8u = EM->ub[0];
                for(int i=0; i<4; ++i)
                    GM.sw[i] <<= tmp8u;
            }
            NEXT;
        _0f_0xF2:                   /* PSLLD Gm, Em */
            nextop = F8;
            GET_EM;
            if(EM->q>31)
                GM.q = 0;
            else {
                tmp8u = EM->ub[0];
                for(int i=0; i<2; ++i)
                    GM.sd[i] <<= tmp8u;
            }
            NEXT;
        _0f_0xF3:                   /* PSLLQ Gm, Em */
            nextop = F8;
            GET_EM;
            GM.q = (EM->q > 63) ? 0 : (GM.q << EM->ub[0]);
            NEXT;
        _0f_0xF4:                   /* PMULUDQ Gm,Em */
            nextop = F8;
            GET_EM;
            GM.q = (uint64_t)GM.ud[0] * (uint64_t)EM->ud[0];
            NEXT;
        _0f_0xF5:                   /* PMADDWD Gm, Em */
            nextop = F8;
            GET_EM;
            for (int i=0; i<2; ++i) {
                int offset = i * 2;

                tmp32s = (int32_t)GM.sw[offset + 0] * EM->sw[offset + 0];
                tmp32s2 = (int32_t)GM.sw[offset + 1] * EM->sw[offset + 1];
                GM.sd[i] = tmp32s + tmp32s2;
            }
            NEXT;
        _0f_0xF6:                   /* PSADBW Gm, Em */
            nextop = F8;
            GET_EM;
            tmp32u = 0;
            for (int i=0; i<8; ++i)
                tmp32u += (GM.ub[i]>EM->ub[i])?(GM.ub[i] - EM->ub[i]):(EM->ub[i] - GM.ub[i]);
            GM.q = tmp32u;
            NEXT;
        _0f_0xF7:                   /* MASKMOVQ Gm, Em */
            nextop = F8;
            if((nextop&0xC0)==0xC0) {
                GET_EM;
                for (int i=0; i<8; ++i)
                    if(EM->ub[i]&0x80) ((uint8_t*)(R_EDI))[i] = GM.ub[i];
            } else
                goto _default;
            NEXT;
        _0f_0xF8:                   /* PSUBB Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<8; ++i)
                GM.sb[i] -= EM->sb[i];
            NEXT;
        _0f_0xF9:                   /* PSUBW Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i)
                GM.sw[i] -= EM->sw[i];
            NEXT;
        _0f_0xFA:                   /* PSUBD Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<2; ++i)
                GM.sd[i] -= EM->sd[i];
            NEXT;

        _0f_0xFC:                   /* PADDB Gm, Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<8; ++i)
                GM.sb[i] += EM->sb[i];
            NEXT;
        _0f_0xFD:                   /* PADDW Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<4; ++i)
                GM.sw[i] += EM->sw[i];
            NEXT;
        _0f_0xFE:                   /* PADDD Gm,Em */
            nextop = F8;
            GET_EM;
            for(int i=0; i<2; ++i)
                GM.sd[i] += EM->sd[i];
            NEXT;
