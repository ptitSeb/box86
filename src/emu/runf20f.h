    opcode = F8;
    switch(opcode) {

    case 0x10:  /* MOVSD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.q[0] = EX->q[0];
        if((nextop&0xC0)!=0xC0) {
            // EX is not a register
            GX.q[1] = 0;
        }
        break;
    case 0x11:  /* MOVSD Ex, Gx */
        nextop = F8;
        GET_EX;
        EX->q[0] = GX.q[0];
        break;
    case 0x12:  /* MOVDDUP Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.q[1] = GX.q[0] = EX->q[0];
        break;

    case 0x2A:  /* CVTSI2SD Gx, Ed */
        nextop = F8;
        GET_ED;
        GX.d[0] = ED->sdword[0];
        break;

    case 0x2C:  /* CVTTSD2SI Gd, Ex */
        nextop = F8;
        GET_EX;
        GD.sdword[0] = EX->d[0];
        break;
    case 0x2D:  /* CVTSD2SI Gd, Ex */
        nextop = F8;
        GET_EX;
        switch((emu->mxcsr>>13)&3) {
            case ROUND_Nearest:
                GD.sdword[0] = floor(EX->d[0]+0.5);
                break;
            case ROUND_Down:
                GD.sdword[0] = floor(EX->d[0]);
                break;
            case ROUND_Up:
                GD.sdword[0] = ceil(EX->d[0]);
                break;
            case ROUND_Chop:
                GD.sdword[0] = EX->d[0];
                break;
        }
        break;
    case 0x51:  /* SQRTSD Gx, Ex */
        nextop = F8;
        GET_EX;
        if(EX->d[0]<0.0 )
            GX.d[0] = -NAN;
        else
            GX.d[0] = sqrt(EX->d[0]);
        break;

    case 0x58:  /* ADDSD Gx, Ex */
        nextop = F8;
        GET_EX;
        #ifndef NOALIGN
        // add generate a -NAN only if doing inf + -inf
        if((isinf(GX.d[0]) && isinf(EX->d[0]) && (EX->q[0]&0x8000000000000000LL)!=(GX.q[0]&0x8000000000000000LL)))
            GX.d[0] = -NAN;
        else
        #endif
        GX.d[0] += EX->d[0];
        break;
    case 0x59:  /* MULSD Gx, Ex */
        nextop = F8;
        GET_EX;
        #ifndef NOALIGN
        // mul generate a -NAN only if doing (+/-)inf * (+/-)0
        if((isinf(GX.d[0]) && EX->d[0]==0.0) || (isinf(EX->d[0]) && GX.d[0]==0.0))
            GX.d[0] = -NAN;
        else
        #endif
        GX.d[0] *= EX->d[0];
        break;
    case 0x5A:  /* CVTSD2SS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f[0] = EX->d[0];
        break;

    case 0x5C:  /* SUBSD Gx, Ex */
        nextop = F8;
        GET_EX;
        #ifndef NOALIGN
            // sub generate a -NAN only if doing inf - inf
            if((isinf(GX.d[0]) && isinf(EX->d[0]) && (EX->q[0]&0x8000000000000000LL)==(GX.q[0]&0x8000000000000000LL)))
                GX.d[0] = -NAN;
            else
        #endif
        GX.d[0] -= EX->d[0];
        break;
    case 0x5D:  /* MINSD Gx, Ex */
        nextop = F8;
        GET_EX;
        if (isnan(GX.d[0]) || isnan(EX->d[0]) || (EX->d[0]<=GX.d[0]))
            GX.d[0] = EX->d[0];
        break;
    case 0x5E:  /* DIVSD Gx, Ex */
        nextop = F8;
        GET_EX;
        #ifndef NOALIGN
        is_nan = isnan(GX.d[0]) || isnan(EX->d[0]);
        #endif
        GX.d[0] /= EX->d[0];
        #ifndef NOALIGN
        if(!is_nan && isnan(GX.d[0]))
            GX.d[0] = -NAN;
        #endif
        break;
    case 0x5F:  /* MAXSD Gx, Ex */
        nextop = F8;
        GET_EX;
        if (isnan(GX.d[0]) || isnan(EX->d[0]) || isgreater(EX->d[0], GX.d[0]))
            GX.d[0] = EX->d[0];
        break;

    case 0x70:  /* PSHUFLW Gx, Ex, Ib */
        nextop = F8;
        GET_EX;
        tmp8u = F8;
        if(&GX==EX) {
            for (int i=0; i<4; ++i)
                eax1.uw[i] = EX->uw[(tmp8u>>(i*2))&3];
            GX.q[0] = eax1.q[0];
        } else {
            for (int i=0; i<4; ++i)
                GX.uw[i] = EX->uw[(tmp8u>>(i*2))&3];
            GX.q[1] = EX->q[1];
        }
        break;

    case 0x7C:  /* HADDPS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f[0] += GX.f[1];
        GX.f[1] = GX.f[2] + GX.f[3];
        if(EX==&GX) {
            GX.f[2] = GX.f[0];
            GX.f[3] = GX.f[1];
        } else {
            GX.f[2] = EX->f[0] + EX->f[1];
            GX.f[3] = EX->f[2] + EX->f[3];
        }
        break;

    #define GOCOND(BASE, PREFIX, CONDITIONAL)   \
    case BASE+0:                                \
        PREFIX                                  \
        if(ACCESS_FLAG(F_OF))                   \
            CONDITIONAL                         \
        break;                                  \
    case BASE+1:                                \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_OF))                  \
            CONDITIONAL                         \
        break;                                  \
    case BASE+2:                                \
        PREFIX                                  \
        if(ACCESS_FLAG(F_CF))                   \
            CONDITIONAL                         \
        break;                                  \
    case BASE+3:                                \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_CF))                  \
            CONDITIONAL                         \
        break;                                  \
    case BASE+4:                                \
        PREFIX                                  \
        if(ACCESS_FLAG(F_ZF))                   \
            CONDITIONAL                         \
        break;                                  \
    case BASE+5:                                \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_ZF))                  \
            CONDITIONAL                         \
        break;                                  \
    case BASE+6:                                \
        PREFIX                                  \
        if((ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF)))  \
            CONDITIONAL                         \
        break;                                  \
    case BASE+7:                                \
        PREFIX                                  \
        if(!(ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF))) \
            CONDITIONAL                         \
        break;                                  \
    case BASE+8:                                \
        PREFIX                                  \
        if(ACCESS_FLAG(F_SF))                   \
            CONDITIONAL                         \
        break;                                  \
    case BASE+9:                                \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_SF))                  \
            CONDITIONAL                         \
        break;                                  \
    case BASE+0xA:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_PF))                   \
            CONDITIONAL                         \
        break;                                  \
    case BASE+0xB:                              \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_PF))                  \
            CONDITIONAL                         \
        break;                                  \
    case BASE+0xC:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))  \
            CONDITIONAL                         \
        break;                                  \
    case BASE+0xD:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF)) \
            CONDITIONAL                         \
        break;                                  \
    case BASE+0xE:                              \
        PREFIX                                  \
        if(ACCESS_FLAG(F_ZF) || (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))) \
            CONDITIONAL                         \
        break;                                  \
    case BASE+0xF:                              \
        PREFIX                                  \
        if(!ACCESS_FLAG(F_ZF) && (ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF))) \
            CONDITIONAL                         \
        break;
        
    GOCOND(0x80
        , tmp32s = F32S; CHECK_FLAGS(emu);
        , ip += tmp32s;
    )                               /* 0x80 -> 0x8F Jxx */
        
    case 0xC2:  /* CMPSD Gx, Ex, Ib */
        nextop = F8;
        GET_EX;
        tmp8u = F8;
        tmp8s = 0;
        switch(tmp8u&7) {
            case 0: tmp8s=(GX.d[0] == EX->d[0]); break;
            case 1: tmp8s=isless(GX.d[0], EX->d[0]) && !(isnan(GX.d[0]) || isnan(EX->d[0])); break;
            case 2: tmp8s=islessequal(GX.d[0], EX->d[0]) && !(isnan(GX.d[0]) || isnan(EX->d[0])); break;
            case 3: tmp8s=isnan(GX.d[0]) || isnan(EX->d[0]); break;
            case 4: tmp8s=(GX.d[0] != EX->d[0]); break;
            case 5: tmp8s=isnan(GX.d[0]) || isnan(EX->d[0]) || isgreaterequal(GX.d[0], EX->d[0]); break;
            case 6: tmp8s=isnan(GX.d[0]) || isnan(EX->d[0]) || isgreater(GX.d[0], EX->d[0]); break;
            case 7: tmp8s=!isnan(GX.d[0]) && !isnan(EX->d[0]); break;
        }
        GX.q[0]=(tmp8s)?0xffffffffffffffffLL:0LL;
        break;

    case 0xD0:  /* ADDSUBPS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f[0] -= EX->f[0];
        GX.f[1] += EX->f[1];
        GX.f[2] -= EX->f[2];
        GX.f[3] += EX->f[3];
        break;

    case 0xD6:  /* MOVDQ2Q Gm, Ex */
        nextop = F8;
        GET_EX;
        GM.q = EX->q[0];
        break;

    case 0xE6:  /* CVTPD2DQ Gx, Ex */
        nextop = F8;
        GET_EX;
        switch((emu->mxcsr>>13)&3) {
            case ROUND_Nearest:
                GX.sd[0] = floor(EX->d[0]+0.5);
                GX.sd[1] = floor(EX->d[1]+0.5);
                break;
            case ROUND_Down:
                GX.sd[0] = floor(EX->d[0]);
                GX.sd[1] = floor(EX->d[1]);
                break;
            case ROUND_Up:
                GX.sd[0] = ceil(EX->d[0]);
                GX.sd[1] = ceil(EX->d[1]);
                break;
            case ROUND_Chop:
                GX.sd[0] = EX->d[0];
                GX.sd[1] = EX->d[1];
                break;
        }
        GX.q[1] = 0;
        break;

    case 0xF0:  /* LDDQU Gx, Ex */
        nextop = F8;
        GET_EX;
        memcpy(&GX, EX, 16);
        break;

    default:
        goto _default;
    }

