    opcode = F8;
    switch(opcode) {

    case 0x10:  /* MOVSD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d.u64[0] = EX->d.u64[0];
        if((nextop&0xC0)!=0xC0) {
            // EX is not a register
            GX.d.u64[1] = 0;
        }
        break;
    case 0x11:  /* MOVSD Ex, Gx */
        nextop = F8;
        GET_EX;
        EX->d = simde_mm_move_sd(EX->d, GX.d);
        break;
    case 0x12:  /* MOVDDUP Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_movedup_pd(EX->d);
        break;

    case 0x2A:  /* CVTSI2SD Gx, Ed */
        nextop = F8;
        GET_ED;
        GX.d = simde_mm_cvtsi32_sd(GX.d, ED->sdword[0]);
        break;

    case 0x2C:  /* CVTTSD2SI Gd, Ex */
        nextop = F8;
        GET_EX;
        GD.sdword[0] = simde_mm_cvttsd_si32(EX->d);
        break;
    case 0x2D:  /* CVTSD2SI Gd, Ex */
        nextop = F8;
        GET_EX;
        GD.sdword[0] = simde_mm_cvtsd_si32(EX->d);
        break;
    case 0x51:  /* SQRTSD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_sqrt_sd(GX.d, EX->d);
        break;

    case 0x58:  /* ADDSD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_add_sd(GX.d, EX->d);
        break;
    case 0x59:  /* MULSD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_mul_sd(GX.d, EX->d);
        break;

    case 0x5A:  /* CVTSD2SS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f = simde_mm_cvtsd_ss(GX.f, EX->d);
        break;

    case 0x5C:  /* SUBSD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_sub_sd(GX.d, EX->d);
        break;
    case 0x5D:  /* MINSD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_min_sd(GX.d, EX->d);
        break;
    case 0x5E:  /* DIVSD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_div_sd(GX.d, EX->d);
        break;
    case 0x5F:  /* MAXSD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_max_sd(GX.d, EX->d);
        break;

    case 0x70:  /* PSHUFLW Gx, Ex, Ib */
        nextop = F8;
        GET_EX;
        tmp8u = F8;
        GX.i = simde_mm_shufflelo_epi16(EX->i, tmp8u);
        break;

    case 0xC2:  /* CMPSD Gx, Ex, Ib */
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
        break;

    case 0xD6:  /* MOVDQ2Q Gm, Ex */
        nextop = F8;
        GET_EX;
        GM.u64[0] = EX->i.u64[0];
        break;

    case 0xE6:  /* CVTPD2DQ Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_cvtpd_epi32(EX->d);
        break;

    default:
        goto _default;
    }

