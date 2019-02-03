    opcode = F8;
    goto *opcodes660f[opcode];

    #define GOCOND(BASE, PREFIX, CONDITIONAL) \
    _6f_##BASE##_0:                          \
        PREFIX                              \
        if(ACCESS_FLAG(F_OF))               \
            CONDITIONAL                     \
        NEXT;                              \
    _6f_##BASE##_1:                          \
        PREFIX                              \
        if(!ACCESS_FLAG(F_OF))              \
            CONDITIONAL                     \
        NEXT;                              \
    _6f_##BASE##_2:                          \
        PREFIX                              \
        if(ACCESS_FLAG(F_CF))               \
            CONDITIONAL                     \
        NEXT;                              \
    _6f_##BASE##_3:                          \
        PREFIX                              \
        if(!ACCESS_FLAG(F_CF))              \
            CONDITIONAL                     \
        NEXT;                              \
    _6f_##BASE##_4:                          \
        PREFIX                              \
        if(ACCESS_FLAG(F_ZF))               \
            CONDITIONAL                     \
        NEXT;                              \
    _6f_##BASE##_5:                          \
        PREFIX                              \
        if(!ACCESS_FLAG(F_ZF))              \
            CONDITIONAL                     \
        NEXT;                              \
    _6f_##BASE##_6:                          \
        PREFIX                              \
        if((ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF)))  \
            CONDITIONAL                     \
        NEXT;                              \
    _6f_##BASE##_7:                          \
        PREFIX                              \
        if(!(ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF))) \
            CONDITIONAL                     \
        NEXT;                              \
    _6f_##BASE##_8:                          \
        PREFIX                              \
        if(ACCESS_FLAG(F_SF))               \
            CONDITIONAL                     \
        NEXT;                              \
    _6f_##BASE##_9:                          \
        PREFIX                              \
        if(!ACCESS_FLAG(F_SF))              \
            CONDITIONAL                     \
        NEXT;                              \
    _6f_##BASE##_A:                          \
        PREFIX                              \
        if(ACCESS_FLAG(F_PF))               \
            CONDITIONAL                     \
        NEXT;                              \
    _6f_##BASE##_B:                          \
        PREFIX                              \
        if(!ACCESS_FLAG(F_PF))              \
            CONDITIONAL                     \
        NEXT;                              \
    _6f_##BASE##_C:                          \
        PREFIX                              \
        if(ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))  \
            CONDITIONAL                     \
        NEXT;                              \
    _6f_##BASE##_D:                          \
        PREFIX                              \
        if(ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF)) \
            CONDITIONAL                     \
        NEXT;                              \
    _6f_##BASE##_E:                          \
        PREFIX                              \
        if(ACCESS_FLAG(F_ZF) || (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))) \
            CONDITIONAL                     \
        NEXT;                              \
    _6f_##BASE##_F:                          \
        PREFIX                              \
        if(!ACCESS_FLAG(F_ZF) && (ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF))) \
            CONDITIONAL                     \
        NEXT;

    GOCOND(0x40
        , nextop = F8;
        CHECK_FLAGS(emu);
        op2=GetEw(emu, nextop);
        op1=GetG(emu, nextop);
        , op1->word[0] = op2->word[0];
    )                               /* 0x40 -> 0x4F CMOVxx Gw,Ew */ // conditional move, no sign
    #undef GOCOND
        
    _6f_0x10:                      /* MOVUPD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        memcpy(opx1, opx2, 16); // unaligned...
        NEXT;
    _6f_0x11:                      /* MOVUPD Ex, Gx */
        nextop = F8;
        opx1=GetEx(emu, nextop);
        opx2=GetGx(emu, nextop);
        memcpy(opx1, opx2, 16); // unaligned...
        NEXT;

    _6f_0x14:                      /* UNPCKLPD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[1] = opx2->q[0];
        NEXT;
    _6f_0x15:                      /* UNPCKHPD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[0] = opx1->q[1];
        opx1->q[1] = opx2->q[1];
        NEXT;
    _6f_0x16:                      /* MOVHPD Gx, Ed */
        nextop = F8;
        op2=GetEd(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[1] = *(uint64_t*)op2;
        NEXT;
    _6f_0x17:                      /* MOVHPD Ed, Gx */
        nextop = F8;
        op1=GetEd(emu, nextop);
        opx2=GetGx(emu, nextop);
        *(uint64_t*)op1 = opx2->q[1];
        NEXT;

    _6f_0x1F:                      /* NOP (multi-byte) */
        nextop = F8;
        op1=GetEd(emu, nextop);
        NEXT;
    
    _6f_0x28:                      /* MOVAPD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[0] = opx2->q[0];
        opx1->q[1] = opx2->q[1];
        NEXT;
    _6f_0x29:                      /* MOVAPD Ex, Gx */
        nextop = F8;
        opx1=GetEx(emu, nextop);
        opx2=GetGx(emu, nextop);
        opx1->q[0] = opx2->q[0];
        opx1->q[1] = opx2->q[1];
        NEXT;
    _6f_0x2A:                      /* CVTPI2PD Gx, Em */
        nextop = F8;
        opm2=GetEm(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->d[0] = opm2->sd[0];
        opx1->d[1] = opm2->sd[1];
        NEXT;


    _6f_0x2C:                      /* CVTTPD2PI Gm, Ex */
    _6f_0x2D:                      /* CVTPD2PI Gm, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opm1=GetGm(emu, nextop);
        opm1->sd[0] = opx2->d[0];
        opm1->sd[1] = opx2->d[1];
        NEXT;
    _6f_0x2E:                      /* UCOMISD Gx, Ex */
        // no special check...
    _6f_0x2F:                      /* COMISD Gx, Ex */
        RESET_FLAGS(emu);
        nextop = F8;
        opx1=GetEx(emu, nextop);
        opx2=GetGx(emu, nextop);
        if(isnan(opx1->d[0]) || isnan(opx2->d[0])) {
            SET_FLAG(F_ZF); SET_FLAG(F_PF); SET_FLAG(F_CF);
        } else if(isgreater(opx2->d[0], opx1->d[0])) {
            CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
        } else if(isless(opx2->d[0], opx1->d[0])) {
            CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); SET_FLAG(F_CF);
        } else {
            SET_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
        }
        NEXT;

    _6f_0x50:                      /* MOVMSKPD Gd, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        op1=GetG(emu, nextop);
        op1->dword[0] = 0;
        for(int i=0; i<2; ++i)
            op1->dword[0] |= ((opx2->q[i]>>63)&1)<<i;
        NEXT;
    _6f_0x51:                      /* SQRTPD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->d[0] = sqrt(opx2->d[0]);
        opx1->d[1] = sqrt(opx2->d[1]);
        NEXT;

    _6f_0x54:                      /* ANDPD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[0] &= opx2->q[0];
        opx1->q[1] &= opx2->q[1];
        NEXT;
    _6f_0x55:                      /* ANDNPD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[0] = (~opx1->q[0]) & opx2->q[0];
        opx1->q[1] = (~opx1->q[1]) & opx2->q[1];
        NEXT;
    _6f_0x56:                      /* ORPD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[0] |= opx2->q[0];
        opx1->q[1] |= opx2->q[1];
        NEXT;
    _6f_0x57:                      /* XORPD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[0] ^= opx2->q[0];
        opx1->q[1] ^= opx2->q[1];
        NEXT;
    _6f_0x58:                      /* ADDPD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->d[0] += opx2->d[0];
        opx1->d[1] += opx2->d[1];
        NEXT;
    _6f_0x59:                      /* MULPD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->d[0] *= opx2->d[0];
        opx1->d[1] *= opx2->d[1];
        NEXT;
    _6f_0x5A:                      /* CVTPD2PS Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->f[0] = opx2->d[0];
        opx1->f[1] = opx2->d[1];
        opx1->q[1] = 0;
        NEXT;
    _6f_0x5B:                      /* CVTPS2DQ Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->sd[0] = opx2->f[0];
        opx1->sd[1] = opx2->f[1];
        opx1->sd[2] = opx2->f[2];
        opx1->sd[3] = opx2->f[3];
        NEXT;
    _6f_0x5C:                      /* SUBPD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->d[0] -= opx2->d[0];
        opx1->d[1] -= opx2->d[1];
        NEXT;
    _6f_0x5D:                      /* MINPD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        if (isnan(opx1->d[0]) || isnan(opx2->d[0]) || isless(opx2->d[0], opx1->d[0]))
            opx1->d[0] = opx2->d[0];
        if (isnan(opx1->d[1]) || isnan(opx2->d[1]) || isless(opx2->d[1], opx1->d[1]))
            opx1->d[1] = opx2->d[1];
        NEXT;
    _6f_0x5E:                      /* DIVPD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->d[0] /= opx2->d[0];
        opx1->d[1] /= opx2->d[1];
        NEXT;
    _6f_0x5F:                      /* MAXPD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        if (isnan(opx1->d[0]) || isnan(opx2->d[0]) || isgreater(opx2->d[0], opx1->d[0]))
            opx1->d[0] = opx2->d[0];
        if (isnan(opx1->d[1]) || isnan(opx2->d[1]) || isgreater(opx2->d[1], opx1->d[1]))
            opx1->d[1] = opx2->d[1];
        NEXT;

    _6f_0x60:  /* PUNPCKLBW Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        for(int i=7; i>0; --i)
            opx1->ub[2 * i] = opx1->ub[i];
        for(int i=0; i<8; ++i)
            opx1->ub[2 * i + 1] = opx2->ub[i];
        NEXT;
    _6f_0x61:  /* PUNPCKLWD Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        for(int i=3; i>0; --i)
            opx1->uw[2 * i] = opx1->uw[i];
        for(int i=0; i<4; ++i)
            opx1->uw[2 * i + 1] = opx2->uw[i];
        NEXT;
    _6f_0x62:  /* PUNPCKLDQ Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->ud[3] = opx2->ud[1];
        opx1->ud[2] = opx1->ud[1];
        opx1->ud[1] = opx2->ud[0];
        NEXT;

    _6f_0x64:  /* PCMPGTB Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        for(int i=0; i<16; ++i)
            opx1->sb[i] = (opx1->sb[i]>opx2->sb[i])?0xFF:0x00;
        NEXT;
    _6f_0x65:  /* PCMPGTW Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        for(int i=0; i<8; ++i)
            opx1->sw[i] = (opx1->sw[i]>opx2->sw[i])?0xFFFF:0x0000;
        NEXT;
    _6f_0x66:  /* PCMPGTD Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        for(int i=0; i<4; ++i)
            opx1->sd[i] = (opx1->sd[i]>opx2->sd[i])?0xFFFFFFFF:0x00000000;
        NEXT;
    _6f_0x67:  /* PACKUSWB */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        for(int i=0; i<4; ++i)
            opx1->ub[i] = (opx1->sw[i]<0)?0:((opx1->sw[i]>0xff)?0xff:opx1->sw[i]);
        for(int i=0; i<4; ++i)
            opx1->ub[4+i] = (opx2->sw[i]<0)?0:((opx2->sw[i]>0xff)?0xff:opx2->sw[i]);
        NEXT;
    _6f_0x68:  /* PUNPCKHBW Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        for(int i=0; i<8; ++i)
            opx1->ub[2 * i] = opx1->ub[i + 8];
        for(int i=0; i<8; ++i)
            opx1->ub[2 * i + 1] = opx2->ub[i + 8];
        NEXT;
    _6f_0x69:  /* PUNPCKHWD Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        for(int i=0; i<4; ++i)
            opx1->uw[2 * i] = opx1->uw[i + 4];
        for(int i=0; i<4; ++i)
            opx1->uw[2 * i + 1] = opx2->uw[i + 4];
        NEXT;
    _6f_0x6A:  /* PUNPCKHDQ Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->uw[0] = opx1->uw[2];
        opx1->uw[2] = opx1->uw[3];
        opx1->uw[1] = opx2->uw[2];
        opx1->uw[3] = opx2->uw[3];
        NEXT;

    _6f_0x6C:  /* PUNPCKLQDQ Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[1] = opx2->q[0];
        NEXT;
    _6f_0x6D:  /* PUNPCKHQDQ Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[0] = opx1->q[1];
        opx1->q[1] = opx2->q[1];
        NEXT;
    _6f_0x6E:  /* MOVD Gx, Ed */
        nextop = F8;
        op2=GetEd(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[0] = op2->dword[0];
        opx1->q[1] = 0;
        NEXT;
    _6f_0x6F:  /* MOVDQA Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[0] = opx2->q[0];
        opx1->q[1] = opx2->q[1];
        NEXT;
    _6f_0x70:  /* PSHUFD Gx,Ex,Ib */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        tmp8u = F8;
        if(opx1!=opx2)
            for (int i=0; i<4; ++i)
                opx1->ud[i] = opx2->ud[(tmp8u>>(i*2))&3];
        else {
            for (int i=0; i<4; ++i)
                eax1.ud[i] = opx2->ud[(tmp8u>>(i*2))&3];
            memcpy(opx1, &eax1, sizeof(eax1));
        }
        NEXT;
    _6f_0x71:  /* GRP */
        nextop = F8;
        opx1=GetEx(emu, nextop);
        switch((nextop>>3)&7) {
            case 2:                 /* PSRLW Gx, Ib */
                tmp8u = F8;
                if(tmp8u>15)
                    {opx1->q[0] = opx1->q[1] = 0;}
                else
                    for (int i=0; i<8; ++i) opx1->uw[i] >>= tmp8u;
                break;
            case 4:                 /* PSRAW Gx, Ib */
                tmp8u = F8;
                for (int i=0; i<8; ++i) opx1->sw[i] >>= tmp8u;
                break;
            case 6:                 /* PSLLW Gx, Ib */
                tmp8u = F8;
                if(tmp8u>15)
                    {opx1->q[0] = opx1->q[1] = 0;}
                else
                    for (int i=0; i<8; ++i) opx1->uw[i] <<= tmp8u;
                break;
            default:
                goto _default;
        }
        NEXT;
    _6f_0x72:  /* GRP */
        nextop = F8;
        opx1=GetEx(emu, nextop);
        switch((nextop>>3)&7) {
            case 2:                 /* PSRLD Ex, Ib */
                tmp8u = F8;
                if(tmp8u>31)
                    {opx1->q[0] = opx1->q[1] = 0;}
                else
                    for (int i=0; i<4; ++i) opx1->ud[i] >>= tmp8u;
                break;
            case 4:                 /* PSRAD Ex, Ib */
                tmp8u = F8;
                for (int i=0; i<4; ++i) opx1->sd[i] >>= tmp8u;
                break;
            case 6:                 /* PSLLD Ex, Ib */
                tmp8u = F8;
                if(tmp8u>31)
                    {opx1->q[0] = opx1->q[1] = 0;}
                else
                    for (int i=0; i<4; ++i) opx1->ud[i] <<= tmp8u;
                break;
            default:
                goto _default;
        }
        NEXT;
    _6f_0x73:  /* GRP */
        nextop = F8;
        opx1=GetEx(emu, nextop);
        switch((nextop>>3)&7) {
            case 2:                 /* PSRLQ Ex, Ib */
                tmp8u = F8;
                if(tmp8u>63)
                    {opx1->q[0] = opx1->q[1] = 0;}
                else
                    {opx1->q[0] >>= tmp8u; opx1->q[1] >>= tmp8u;}
                break;
            case 3:                 /* PSRLDQ Ex, Ib */
                tmp8u = F8;
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
                tmp8u = F8;
                if(tmp8u>63)
                    {opx1->q[0] = opx1->q[1] = 0;}
                else
                    {opx1->q[0] <<= tmp8u; opx1->q[1] <<= tmp8u;}
                break;
            case 7:                 /* PSLLDQ Ex, Ib */
                tmp8u = F8;
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
                goto _default;
        }
        NEXT;
    _6f_0x74:  /* PCMPEQB Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        for (int i=0; i<16; ++i)
            opx1->ub[i] = (opx1->ub[i]==opx2->ub[i])?0xff:0;
        NEXT;
    _6f_0x75:  /* PCMPEQW Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        for (int i=0; i<8; ++i)
            opx1->uw[i] = (opx1->uw[i]==opx2->uw[i])?0xffff:0;
        NEXT;
    _6f_0x76:  /* PCMPEQD Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        for (int i=0; i<4; ++i)
            opx1->ud[i] = (opx1->ud[i]==opx2->ud[i])?0xffffffff:0;
        NEXT;

    _6f_0x7E:  /* MOVD Ed, Gx */
        nextop = F8;
        op2=GetEd(emu, nextop);
        opx1=GetGx(emu, nextop);
        op2->dword[0] = opx1->ud[0];
        NEXT;
    _6f_0x7F:  /* MOVDQA Ex,Gx */
        nextop = F8;
        opx1=GetEx(emu, nextop);
        opx2=GetGx(emu, nextop);
        opx1->q[0] = opx2->q[0];
        opx1->q[1] = opx2->q[1];
        NEXT;

    _6f_0xA3:                      /* BT Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        op1=GetEw(emu, nextop);
        op2=GetG(emu, nextop);
        if(op1->word[0] & (1<<(op2->word[0]&15)))
            SET_FLAG(F_CF);
        else
            CLEAR_FLAG(F_CF);
        NEXT;
    _6f_0xA4:                      /* SHLD Ew,Gw,Ib */
    _6f_0xA5:                      /* SHLD Ew,Gw,CL */
        nextop = F8;
        op1=GetEw(emu, nextop);
        op2=GetG(emu, nextop);
        tmp8u = (opcode==0xA4)?F8:R_CL;
        op1->word[0] = shld16(emu, op1->word[0], op2->word[0], tmp8u);
        NEXT;

    _6f_0xAB:                      /* BTS Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        op1=GetEw(emu, nextop);
        op2=GetG(emu, nextop);
        if(op1->word[0] & (1<<(op2->word[0]&15)))
            SET_FLAG(F_CF);
        else {
            op1->word[0] |= (1<<(op2->word[0]&15));
            CLEAR_FLAG(F_CF);
        }
        NEXT;
    _6f_0xAC:                      /* SHRD Ew,Gw,Ib */
    _6f_0xAD:                      /* SHRD Ew,Gw,CL */
        nextop = F8;
        op1=GetEw(emu, nextop);
        op2=GetG(emu, nextop);
        tmp8u = (opcode==0xAC)?F8:R_CL;
        op1->word[0] = shrd16(emu, op1->word[0], op2->word[0], tmp8u);
        NEXT;

    _6f_0xAF:                      /* IMUL Gw,Ew */
        nextop = F8;
        op2=GetEw(emu, nextop);
        op1=GetG(emu, nextop);
        op1->word[0] = imul16(emu, op1->word[0], op2->word[0]);
        NEXT;

    _6f_0xB1:                      /* CMPXCHG Ew,Gw */
        nextop = F8;
        op1=GetEw(emu, nextop);
        op2=GetG(emu, nextop);
        cmp16(emu, R_AX, op1->word[0]);
        if(R_AX == op1->word[0]) {
            SET_FLAG(F_ZF);
            op1->word[0] = op2->word[0];
        } else {
            CLEAR_FLAG(F_ZF);
            R_AX = op1->word[0];
        }
        NEXT;

    _6f_0xB3:                      /* BTR Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        op1=GetEw(emu, nextop);
        op2=GetG(emu, nextop);
        if(op1->word[0] & (1<<(op2->word[0]&15))) {
            SET_FLAG(F_CF);
            op1->word[0] ^= (1<<(op2->word[0]&15));
        } else
            CLEAR_FLAG(F_CF);
        NEXT;

    _6f_0xB6:                      /* MOVZX Gw,Eb */
        nextop = F8;
        op2=GetEb(emu, nextop);
        op1=GetG(emu, nextop);
        op1->word[0] = op2->byte[0];
        NEXT;

    _6f_0xBB:                      /* BTC Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        op1=GetEw(emu, nextop);
        op2=GetG(emu, nextop);
        if(op1->word[0] & (1<<(op2->word[0]&15)))
            SET_FLAG(F_CF);
        else
            CLEAR_FLAG(F_CF);
        op1->word[0] ^= (1<<(op2->word[0]&15));
        NEXT;
    _6f_0xBC:                      /* BSF Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        op1=GetEd(emu, nextop);
        op2=GetG(emu, nextop);
        tmp16u = op1->word[0];
        if(tmp16u) {
            CLEAR_FLAG(F_ZF);
            tmp8u = 0;
            while(!(tmp16u&(1<<tmp8u))) ++tmp8u;
            op2->word[0] = tmp8u;
        } else {
            SET_FLAG(F_ZF);
        }
        NEXT;
    _6f_0xBD:                      /* BSR Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        op1=GetEd(emu, nextop);
        op2=GetG(emu, nextop);
        tmp16u = op1->word[0];
        if(tmp16u) {
            CLEAR_FLAG(F_ZF);
            tmp8u = 15;
            while(!(tmp16u&(1<<tmp8u))) --tmp8u;
            op2->word[0] = tmp8u;
        } else {
            SET_FLAG(F_ZF);
        }
        NEXT;
    _6f_0xBE:                      /* MOVSX Gw,Eb */
        nextop = F8;
        op2=GetEb(emu, nextop);
        op1=GetG(emu, nextop);
        op1->sword[0] = (int8_t)op2->byte[0];
        NEXT;

    _6f_0xC1:                      /* XADD Gw,Ew */ // Xchange and Add
        nextop = F8;
        op1=GetEw(emu, nextop);
        op2=GetG(emu, nextop);
        tmp16u = add16(emu, op1->word[0], op2->word[0]);
        op2->word[0] = op1->word[0];
        op1->word[0] = tmp16u;
        NEXT;
    _6f_0xC2:                      /* CMPPD Gx, Ex, Ib */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        tmp8u = F8;
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
        NEXT;

    _6f_0xC4:  /* PINSRW Gx,Ed,Ib */
        nextop = F8;
        op1=GetEw(emu, nextop);
        opx2=GetGx(emu, nextop);
        tmp8u = F8;
        opx2->uw[tmp8u&7] = op1->word[0];
        NEXT;
    _6f_0xC5:  /* PEXTRW Gd,Ex,Ib */
        nextop = F8;
        opx1=GetEx(emu, nextop);
        op2=GetG(emu, nextop);
        tmp8u = F8;
        op2->dword[0] = opx1->uw[tmp8u&7];
        NEXT;
    _6f_0xC6:  /* SHUFPD Gx, Ex, Ib */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        tmp8u = F8;
        eax1.q[0] = opx1->q[tmp8u&1];
        eax1.q[1] = opx2->q[(tmp8u>>1)&1];
        opx1->q[0] = eax1.q[0];
        opx1->q[1] = eax1.q[1];
        NEXT;

    _6f_0xD1:  /* PSRLW Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        if(opx2->q[0]>15)
            {opx1->q[0] = opx1->q[1] = 0;}
        else 
            {tmp8u=opx2->q[0]; for (int i=0; i<8; ++i) opx1->uw[i] >>= tmp8u;}
        NEXT;
    _6f_0xD2:  /* PSRLD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        if(opx2->q[0]>31)
            {opx1->q[0] = opx1->q[1] = 0;}
        else 
            {tmp8u=opx2->q[0]; for (int i=0; i<4; ++i) opx1->ud[i] >>= tmp8u;}
        NEXT;
    _6f_0xD3:  /* PSRLQ Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        if(opx2->q[0]>63)
            {opx1->q[0] = opx1->q[1] = 0;}
        else 
            {tmp8u=opx2->q[0]; for (int i=0; i<2; ++i) opx1->q[i] >>= tmp8u;}
        NEXT;
    _6f_0xD4:  /* PADDQ Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[0] += opx2->q[0];
        opx1->q[1] += opx2->q[1];
        NEXT;

    _6f_0xD6:  /* MOVQ Ex,Gx */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx2->q[0] = opx1->q[0];
        if((nextop&0xc7)>=0xc0 && (nextop&0xc7)<=0xc7)
            opx2->q[1] = 0;
        NEXT;

    _6f_0xDB:  /* PAND Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[0] &= opx2->q[0];
        opx1->q[1] &= opx2->q[1];
        NEXT;

    _6f_0xDF:  /* PANDN Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[0] = (~opx1->q[0]) & opx2->q[0];
        opx1->q[1] = (~opx1->q[1]) & opx2->q[1];
        NEXT;

    _6f_0xE1:  /* PSRAW Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        tmp8u=opx2->q[0];
        for (int i=0; i<8; ++i) opx1->sw[i] >>= tmp8u;
        NEXT;
    _6f_0xE2:  /* PSRAD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        tmp8u=opx2->q[0]; for (int i=0; i<4; ++i) opx1->sd[i] >>= tmp8u;
        NEXT;

    _6f_0xE6:  /* CVTTPD2DQ Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->sd[0] = opx2->d[0];
        opx1->sd[1] = opx2->d[1];
        opx1->q[1] = 0;
        NEXT;

    _6f_0xEB:  /* POR Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->ud[0] |= opx2->ud[0];
        opx1->ud[1] |= opx2->ud[1];
        opx1->ud[2] |= opx2->ud[2];
        opx1->ud[3] |= opx2->ud[3];
        NEXT;
    _6f_0xEC:  /* PADDSB Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        for(int i=0; i<16; ++i)
            opx1->sb[i] += opx2->sb[i];
        NEXT;
    _6f_0xED:  /* PADDSW Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        for(int i=0; i<8; ++i)
            opx1->sw[i] += opx2->sw[i];
        NEXT;
    _6f_0xEE:  /* PMAXSW Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        for(int i=0; i<8; ++i)
            opx1->sw[i] = (opx1->sw[i]>opx2->sw[i])?opx1->sw[i]:opx2->sw[i];
        NEXT;
    _6f_0xEF:  /* PXOR Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->ud[0] ^= opx2->ud[0];
        opx1->ud[1] ^= opx2->ud[1];
        opx1->ud[2] ^= opx2->ud[2];
        opx1->ud[3] ^= opx2->ud[3];
        NEXT;

    _6f_0xF1:  /* PSLLW Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        if(opx2->q[0]>15)
            {opx1->q[0] = opx1->q[1] = 0;}
        else 
            {tmp8u=opx2->q[0]; for (int i=0; i<8; ++i) opx1->uw[i] <<= tmp8u;}
        NEXT;
    _6f_0xF2:  /* PSLLD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        if(opx2->q[0]>31)
            {opx1->q[0] = opx1->q[1] = 0;}
        else 
            {tmp8u=opx2->q[0]; for (int i=0; i<4; ++i) opx1->ud[i] <<= tmp8u;}
        NEXT;
    _6f_0xF3:  /* PSLLQ Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        if(opx2->q[0]>63)
            {opx1->q[0] = opx1->q[1] = 0;}
        else 
            {tmp8u=opx2->q[0]; for (int i=0; i<2; ++i) opx1->q[i] <<= tmp8u;}
        NEXT;
    _6f_0xF4:  /* PMULUDQ Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[1] = (uint64_t)opx2->ud[2]*opx1->ud[2];
        opx1->q[0] = (uint64_t)opx2->ud[0]*opx1->ud[0];
        NEXT;

    _6f_0xFA:  /* PSUBD Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->ud[0] -= opx2->ud[0];
        opx1->ud[1] -= opx2->ud[1];
        opx1->ud[2] -= opx2->ud[2];
        opx1->ud[3] -= opx2->ud[3];
        NEXT;
    _6f_0xFB:  /* PSUBQ Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[0] -= opx2->q[0];
        opx1->q[1] -= opx2->q[1];
        NEXT;
    _6f_0xFC:  /* PADDB Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        for(int i=0; i<16; ++i)
            opx1->ub[i] += opx2->ub[i];
        NEXT;

    _6f_0xFE:  /* PADDD Gx,Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->ud[0] += opx2->ud[0];
        opx1->ud[1] += opx2->ud[1];
        opx1->ud[2] += opx2->ud[2];
        opx1->ud[3] += opx2->ud[3];
        NEXT;


