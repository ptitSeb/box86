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
        GET_EW;
        , GW.word[0] = EW->word[0];
    )                               /* 0x40 -> 0x4F CMOVxx Gw,Ew */ // conditional move, no sign
    #undef GOCOND
        
    _6f_0x10:                      /* MOVUPD Gx, Ex */
        nextop = F8;
        GET_EX;
        memcpy(&GX, EX, 16); // unaligned...
        NEXT;
    _6f_0x11:                      /* MOVUPD Ex, Gx */
        nextop = F8;
        GET_EX;
        memcpy(EX, &GX, 16); // unaligned...
        NEXT;
    _6f_0x12:                      /* MOVLPD Gx, Eq */
        nextop = F8;
        GET_ED;
        GX.d.u64[0] = *(uint64_t*)ED;
        NEXT;
    _6f_0x13:                      /* MOVLPD Eq, Gx */
        nextop = F8;
        GET_ED;
        simde_mm_storel_pd((simde_float64*)ED, GX.d);
        NEXT;
    _6f_0x14:                      /* UNPCKLPD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_unpacklo_pd(GX.d, EX->d);
        NEXT;
    _6f_0x15:                      /* UNPCKHPD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_unpackhi_pd(GX.d, EX->d);
        NEXT;
    _6f_0x16:                      /* MOVHPD Gx, Ed */
        nextop = F8;
        GET_ED;
        GX.d.u64[1] = *(uint64_t*)ED;
        NEXT;
    _6f_0x17:                      /* MOVHPD Ed, Gx */
        nextop = F8;
        GET_ED;
        *(uint64_t*)ED = GX.d.u64[1];
        NEXT;

    _6f_0x1F:                      /* NOP (multi-byte) */
        nextop = F8;
        GET_ED;
        NEXT;
    
    _6f_0x28:                      /* MOVAPD Gx, Ex */
        nextop = F8;
        GET_EX;
        simde_mm_store_pd(&GX.d.f64[0], EX->d);
        NEXT;
    _6f_0x29:                      /* MOVAPD Ex, Gx */
        nextop = F8;
        GET_EX;
        simde_mm_store_pd(&EX->d.f64[0], GX.d);
        NEXT;
    _6f_0x2A:                      /* CVTPI2PD Gx, Em */
        nextop = F8;
        GET_EM;
        GX.d = simde_mm_cvtpi32_pd(*EM);
        NEXT;


    _6f_0x2C:                      /* CVTTPD2PI Gm, Ex */
        nextop = F8;
        GET_EX;
        GM = simde_mm_cvttpd_pi32(EX->d);
        NEXT;
    _6f_0x2D:                      /* CVTPD2PI Gm, Ex */
        nextop = F8;
        GET_EX;
        GM = simde_mm_cvtpd_pi32(EX->d);
        NEXT;
    _6f_0x2E:                      /* UCOMISD Gx, Ex */
        // no special check...
    _6f_0x2F:                      /* COMISD Gx, Ex */
        RESET_FLAGS(emu);
        nextop = F8;
        GET_EX;
        if(isnan(GX.d.f64[0]) || isnan(EX->d.f64[0])) {
            SET_FLAG(F_ZF); SET_FLAG(F_PF); SET_FLAG(F_CF);
        } else if(isgreater(GX.d.f64[0], EX->d.f64[0])) {
            CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
        } else if(isless(GX.d.f64[0], EX->d.f64[0])) {
            CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); SET_FLAG(F_CF);
        } else {
            SET_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
        }
        CLEAR_FLAG(F_OF); CLEAR_FLAG(F_AF); CLEAR_FLAG(F_SF);
        NEXT;

    _6f_0x50:                      /* MOVMSKPD Gd, Ex */
        nextop = F8;
        GET_EX;
        GD.dword[0] = simde_mm_movemask_pd(EX->d);
        NEXT;
    _6f_0x51:                      /* SQRTPD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_sqrt_pd(EX->d);
        NEXT;

    _6f_0x54:                      /* ANDPD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_and_pd(GX.d, EX->d);
        NEXT;
    _6f_0x55:                      /* ANDNPD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_andnot_pd(GX.d, EX->d);
        NEXT;
    _6f_0x56:                      /* ORPD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_or_pd(GX.d, EX->d);
        NEXT;
    _6f_0x57:                      /* XORPD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_xor_pd(GX.d, EX->d);
        NEXT;
    _6f_0x58:                      /* ADDPD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_add_pd(GX.d, EX->d);
        NEXT;
    _6f_0x59:                      /* MULPD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_mul_pd(GX.d, EX->d);
        NEXT;
    _6f_0x5A:                      /* CVTPD2PS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f = simde_mm_cvtpd_ps(EX->d);
        NEXT;
    _6f_0x5B:                      /* CVTPS2DQ Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_cvtps_epi32(EX->f);
        NEXT;
    _6f_0x5C:                      /* SUBPD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_sub_pd(GX.d, EX->d);
        NEXT;
    _6f_0x5D:                      /* MINPD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_min_pd(GX.d, EX->d);
        NEXT;
    _6f_0x5E:                      /* DIVPD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_div_pd(GX.d, EX->d);
        NEXT;
    _6f_0x5F:                      /* MAXPD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_max_pd(GX.d, EX->d);
        NEXT;

    _6f_0x60:  /* PUNPCKLBW Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_unpacklo_epi8(GX.i, EX->i);
        NEXT;
    _6f_0x61:  /* PUNPCKLWD Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_unpacklo_epi16(GX.i, EX->i);
        NEXT;
    _6f_0x62:  /* PUNPCKLDQ Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_unpacklo_epi32(GX.i, EX->i);
        NEXT;

    _6f_0x64:  /* PCMPGTB Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_cmpgt_epi8(GX.i, EX->i);
        NEXT;
    _6f_0x65:  /* PCMPGTW Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_cmpgt_epi16(GX.i, EX->i);
        NEXT;
    _6f_0x66:  /* PCMPGTD Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_cmpgt_epi32(GX.i, EX->i);
        NEXT;
    _6f_0x67:  /* PACKUSWB */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_packus_epi16(GX.i, EX->i);
        NEXT;
    _6f_0x68:  /* PUNPCKHBW Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_unpackhi_epi8(GX.i, EX->i);
        NEXT;
    _6f_0x69:  /* PUNPCKHWD Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_unpackhi_epi16(GX.i, EX->i);
        NEXT;
    _6f_0x6A:  /* PUNPCKHDQ Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_unpackhi_epi32(GX.i, EX->i);
        NEXT;
    _6f_0x6B:  /* PACKSSDW Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_packs_epi32(GX.i, EX->i);
        NEXT;
    _6f_0x6C:  /* PUNPCKLQDQ Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_unpacklo_epi64(GX.i, EX->i);
        NEXT;
    _6f_0x6D:  /* PUNPCKHQDQ Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_unpackhi_epi64(GX.i, EX->i);
        NEXT;
    _6f_0x6E:  /* MOVD Gx, Ed */
        nextop = F8;
        GET_ED;
        GX.i = simde_mm_cvtsi32_si128(ED->sword[0]);
        NEXT;
    _6f_0x6F:  /* MOVDQA Gx,Ex */
        nextop = F8;
        GET_EX;
        simde_mm_store_si128(&GX.i, EX->i);
        NEXT;
    _6f_0x70:  /* PSHUFD Gx,Ex,Ib */
        nextop = F8;
        GET_EX;
        tmp8u = F8;
        GX.i = simde_mm_shuffle_epi32(EX->i, tmp8u);
        NEXT;
    _6f_0x71:  /* GRP */
        nextop = F8;
        GET_EX;
        switch((nextop>>3)&7) {
            case 2:                 /* PSRLW Gx, Ib */
                tmp8u = F8;
                EX->i = simde_mm_srli_epi16(EX->i, tmp8u);
                break;
            case 4:                 /* PSRAW Gx, Ib */
                tmp8u = F8;
                EX->i = simde_mm_srai_epi16(EX->i, tmp8u);
                break;
            case 6:                 /* PSLLW Gx, Ib */
                tmp8u = F8;
                EX->i = simde_mm_slli_epi16(EX->i, tmp8u);
                break;
            default:
                goto _default;
        }
        NEXT;
    _6f_0x72:  /* GRP */
        nextop = F8;
        GET_EX;
        switch((nextop>>3)&7) {
            case 2:                 /* PSRLD Ex, Ib */
                tmp8u = F8;
                EX->i = simde_mm_srli_epi32(EX->i, tmp8u);
                break;
            case 4:                 /* PSRAD Ex, Ib */
                tmp8u = F8;
                EX->i = simde_mm_srai_epi32(EX->i, tmp8u);
                break;
            case 6:                 /* PSLLD Ex, Ib */
                tmp8u = F8;
                EX->i = simde_mm_slli_epi32(EX->i, tmp8u);
                break;
            default:
                goto _default;
        }
        NEXT;
    _6f_0x73:  /* GRP */
        nextop = F8;
        GET_EX;
        switch((nextop>>3)&7) {
            case 2:                 /* PSRLQ Ex, Ib */
                tmp8u = F8;
                EX->i = simde_mm_srli_epi64(EX->i, tmp8u);
                break;
            case 3:                 /* PSRLDQ Ex, Ib */
                tmp8u = F8;
                EX->i = simde_mm_srli_si128(EX->i, tmp8u);
                break;
            case 6:                 /* PSLLQ Ex, Ib */
                tmp8u = F8;
                EX->i = simde_mm_slli_epi64(EX->i, tmp8u);
                break;
            case 7:                 /* PSLLDQ Ex, Ib */
                tmp8u = F8;
                EX->i = simde_mm_slli_si128(EX->i, tmp8u);
                break;
            default:
                goto _default;
        }
        NEXT;
    _6f_0x74:  /* PCMPEQB Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_cmpeq_epi8(GX.i, EX->i);
        NEXT;
    _6f_0x75:  /* PCMPEQW Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_cmpeq_epi16(GX.i, EX->i);
        NEXT;
    _6f_0x76:  /* PCMPEQD Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_cmpeq_epi32(GX.i, EX->i);
        NEXT;

    _6f_0x7E:  /* MOVD Ed, Gx */
        nextop = F8;
        GET_ED;
        ED->dword[0] = simde_mm_cvtsi128_si32(GX.i);
        NEXT;
    _6f_0x7F:  /* MOVDQA Ex,Gx */
        nextop = F8;
        GET_EX;
        simde_mm_store_si128(&EX->i, GX.i);
        NEXT;

    _6f_0xA3:                      /* BT Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        GET_EW;
        if(EW->word[0] & (1<<(GW.word[0]&15)))
            SET_FLAG(F_CF);
        else
            CLEAR_FLAG(F_CF);
        NEXT;
    _6f_0xA4:                      /* SHLD Ew,Gw,Ib */
    _6f_0xA5:                      /* SHLD Ew,Gw,CL */
        nextop = F8;
        GET_EW;
        tmp8u = (opcode==0xA4)?(F8):R_CL;
        EW->word[0] = shld16(emu, EW->word[0], GW.word[0], tmp8u);
        NEXT;

    _6f_0xAB:                      /* BTS Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        GET_EW;
        if(EW->word[0] & (1<<(GW.word[0]&15)))
            SET_FLAG(F_CF);
        else {
            EW->word[0] |= (1<<(GW.word[0]&15));
            CLEAR_FLAG(F_CF);
        }
        NEXT;
    _6f_0xAC:                      /* SHRD Ew,Gw,Ib */
    _6f_0xAD:                      /* SHRD Ew,Gw,CL */
        nextop = F8;
        GET_EW;
        tmp8u = (opcode==0xAC)?(F8):R_CL;
        EW->word[0] = shrd16(emu, EW->word[0], GW.word[0], tmp8u);
        NEXT;

    _6f_0xAF:                      /* IMUL Gw,Ew */
        nextop = F8;
        GET_EW;
        GW.word[0] = imul16(emu, GW.word[0], EW->word[0]);
        NEXT;

    _6f_0xB1:                      /* CMPXCHG Ew,Gw */
        nextop = F8;
        GET_EW;
        cmp16(emu, R_AX, EW->word[0]);
        if(ACCESS_FLAG(F_ZF)) {
            EW->word[0] = GW.word[0];
        } else {
            R_AX = EW->word[0];
        }
        NEXT;

    _6f_0xB3:                      /* BTR Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        GET_EW;
        if(EW->word[0] & (1<<(GW.word[0]&15))) {
            SET_FLAG(F_CF);
            EW->word[0] ^= (1<<(GW.word[0]&15));
        } else
            CLEAR_FLAG(F_CF);
        NEXT;

    _6f_0xB6:                      /* MOVZX Gw,Eb */
        nextop = F8;
        GET_EB;
        GW.word[0] = EB->byte[0];
        NEXT;

    _6f_0xBB:                      /* BTC Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        GET_EW;
        if(EW->word[0] & (1<<(GW.word[0]&15)))
            SET_FLAG(F_CF);
        else
            CLEAR_FLAG(F_CF);
        EW->word[0] ^= (1<<(GW.word[0]&15));
        NEXT;
    _6f_0xBC:                      /* BSF Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        GET_EW;
        tmp16u = EW->word[0];
        if(tmp16u) {
            CLEAR_FLAG(F_ZF);
            tmp8u = 0;
            while(!(tmp16u&(1<<tmp8u))) ++tmp8u;
            GW.word[0] = tmp8u;
        } else {
            SET_FLAG(F_ZF);
        }
        NEXT;
    _6f_0xBD:                      /* BSR Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        GET_EW;
        tmp16u = EW->word[0];
        if(tmp16u) {
            CLEAR_FLAG(F_ZF);
            tmp8u = 15;
            while(!(tmp16u&(1<<tmp8u))) --tmp8u;
            GW.word[0] = tmp8u;
        } else {
            SET_FLAG(F_ZF);
        }
        NEXT;
    _6f_0xBE:                      /* MOVSX Gw,Eb */
        nextop = F8;
        GET_EB;
        GW.sword[0] = EB->sbyte[0];
        NEXT;

    _6f_0xC1:                      /* XADD Gw,Ew */ // Xchange and Add
        nextop = F8;
        GET_EW;
        tmp16u = add16(emu, EW->word[0], GW.word[0]);
        GW.word[0] = EW->word[0];
        EW->word[0] = tmp16u;
        NEXT;
    _6f_0xC2:                      /* CMPPD Gx, Ex, Ib */
        nextop = F8;
        GET_EX;
        tmp8u = F8;
        switch(tmp8u&7) {
            case 0: GX.d=simde_mm_cmpeq_sd(GX.d, EX->d); break;
            case 1: GX.d=simde_mm_cmplt_sd(GX.d, EX->d); break;
            case 2: GX.d=simde_mm_cmple_sd(GX.d, EX->d); break;
            case 3: GX.d=simde_mm_cmpunord_sd(GX.d, EX->d); break;
            case 4: GX.d=simde_mm_cmpneq_sd(GX.d, EX->d); break;
            case 5: GX.d=simde_mm_cmpge_sd(GX.d, EX->d); break;
            case 6: GX.d=simde_mm_cmpgt_sd(GX.d, EX->d); break;
            case 7: GX.d=simde_mm_cmpord_sd(GX.d, EX->d); break;
        }
        NEXT;

    _6f_0xC4:  /* PINSRW Gx,Ew,Ib */
        nextop = F8;
        GET_EW;
        tmp8u = F8;
        GX.i = simde_mm_insert_epi16(GX.i, EW->word[0], tmp8u&7);
        NEXT;
    _6f_0xC5:  /* PEXTRW Gw,Ex,Ib */
        nextop = F8;
        GET_EX;
        tmp8u = F8;
        GW.word[0] = simde_mm_extract_epi16(EX->i, tmp8u&7);
        NEXT;
    _6f_0xC6:  /* SHUFPD Gx, Ex, Ib */
        nextop = F8;
        GET_EX;
        tmp8u = F8;
        GX.d = simde_mm_shuffle_pd(GX.d, EX->d, tmp8u);
        NEXT;

    _6f_0xD1:  /* PSRLW Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_srl_epi16(GX.i, EX->i);
        NEXT;
    _6f_0xD2:  /* PSRLD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_srl_epi32(GX.i, EX->i);
        NEXT;
    _6f_0xD3:  /* PSRLQ Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_srl_epi64(GX.i, EX->i);
        NEXT;
    _6f_0xD4:  /* PADDQ Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_add_epi64(GX.i, EX->i);
        NEXT;

    _6f_0xD6:  /* MOVQ Ex,Gx */
        nextop = F8;
        GET_EX;
        EX->f.u64[0] = GX.f.u64[0];
        if((nextop&0xC0)==0xC0)
            EX->f.u64[1] = 0;
        NEXT;
    _6f_0xD7:  /* PMOVMSKB Gd,Ex */
        nextop = F8;
        GET_EX;
        GD.dword[0] = simde_mm_movemask_epi8(EX->i);
        NEXT;

    _6f_0xDB:  /* PAND Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_and_si128(GX.i, EX->i);
        NEXT;

    _6f_0xDF:  /* PANDN Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_andnot_si128(GX.i, EX->i);
        NEXT;

    _6f_0xE1:  /* PSRAW Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_sra_epi16(GX.i, EX->i);
        NEXT;
    _6f_0xE2:  /* PSRAD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_sra_epi32(GX.i, EX->i);
        NEXT;

    _6f_0xE6:  /* CVTTPD2DQ Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_cvttpd_epi32(EX->d);
        NEXT;

    _6f_0xEB:  /* POR Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_or_si128(GX.i, EX->i);
        NEXT;
    _6f_0xEC:  /* PADDSB Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_add_epi8(GX.i, EX->i);
        NEXT;
    _6f_0xED:  /* PADDSW Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_add_epi16(GX.i, EX->i);
        NEXT;
    _6f_0xEE:  /* PMAXSW Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_max_epi16(GX.i, EX->i);
        NEXT;
    _6f_0xEF:  /* PXOR Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_xor_si128(GX.i, EX->i);
        NEXT;

    _6f_0xF1:  /* PSLLW Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_sll_epi16(GX.i, EX->i);
        NEXT;
    _6f_0xF2:  /* PSLLD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_sll_epi32(GX.i, EX->i);
        NEXT;
    _6f_0xF3:  /* PSLLQ Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_sll_epi64(GX.i, EX->i);
        NEXT;
    _6f_0xF4:  /* PMULUDQ Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_mul_epu32(GX.i, EX->i);
        NEXT;

    _6f_0xFA:  /* PSUBD Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_sub_epi32(GX.i, EX->i);
        NEXT;
    _6f_0xFB:  /* PSUBQ Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_sub_epi64(GX.i, EX->i);
        NEXT;
    _6f_0xFC:  /* PADDB Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_add_epi8(GX.i, EX->i);
        NEXT;

    _6f_0xFE:  /* PADDD Gx,Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_add_epi32(GX.i, EX->i);
        NEXT;


