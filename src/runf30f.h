    opcode = F8;
    switch(opcode) {

    case 0x10:  /* MOVSS Gx Ex */
        nextop = F8;
        GET_EX;
        GX.f.u32[0] = EX->f.u32[0];
        if((nextop&0xC0)!=0xC0) {
            // EX is not a register (reg to reg only move 31:0)
            GX.f.u32[1] = GX.f.u32[2] = GX.f.u32[3] = 0;
        }
        break;
    case 0x11:  /* MOVSS Ex Gx */
        nextop = F8;
        GET_EX;
        EX->f = simde_mm_move_ss(EX->f, GX.f);
        break;

    case 0x2A:  /* CVTSI2SS Gx, Ed */
        nextop = F8;
        GET_ED;
        GX.f = simde_mm_cvtsi32_ss(GX.f, ED->sdword[0]);
        break;

    case 0x2C:  /* CVTTSS2SI Gd, Ex */
        nextop = F8;
        GET_EX;
        GD.sdword[0] = simde_mm_cvttss_si32(EX->f);
        break;
    case 0x2D:  /* CVTSS2SI Gd, Ex */
        nextop = F8;
        GET_EX;
        GD.sdword[0] = simde_mm_cvtss_si32(EX->f);
        break;

    case 0x51:  /* SQRTSS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f = simde_mm_sqrt_ss(EX->f);
        break;

    case 0x58:  /* ADDSS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f = simde_mm_add_ss(GX.f, EX->f);
        break;
    case 0x59:  /* MULSS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f = simde_mm_mul_ss(GX.f, EX->f);
        break;
    case 0x5A:  /* CVTSS2SD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_cvtss_sd(GX.d, EX->f);
        break;
    case 0x5B:  /* CVTTPS2DQ Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_cvttps_epi32(EX->f);
        break;

    case 0x5C:  /* SUBSS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f = simde_mm_sub_ss(GX.f, EX->f);
        break;
    case 0x5D:  /* MINSS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f = simde_mm_min_ss(GX.f, EX->f);
        break;
    case 0x5E:  /* DIVSS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f = simde_mm_div_ss(GX.f, EX->f);
        break;
    case 0x5F:  /* MAXSS Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.f = simde_mm_max_ss(GX.f, EX->f);
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
        GX.i = simde_mm_shufflehi_epi16(EX->i, tmp8u);
        break;

    case 0x7E:  /* MOVQ Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.i = simde_mm_move_epi64(EX->i);
        break;
    case 0x7F:  /* MOVDQU Ex, Gx */
        nextop = F8;
        GET_EX;
        memcpy(EX, &GX, 16);    // unaligned...
        break;

    case 0xBC:  /* TZCNT Ew,Gw */
        CHECK_FLAGS(emu);
        nextop = F8;
        GET_ED;
        tmp32u = ED->dword[0];
        if(tmp32u) {
            CLEAR_FLAG(F_ZF);
            tmp8u = 0;
            while(!(tmp32u&(1<<tmp8u))) ++tmp8u;
            GD.dword[0] = tmp8u;
            CONDITIONAL_SET_FLAG(tmp8u, F_ZF);
            CLEAR_FLAG(F_CF);
        } else {
            CLEAR_FLAG(F_ZF);
            SET_FLAG(F_CF);
            GD.dword[0] = 32;
        }
        break;

    case 0xC2:  /* CMPSS Gx, Ex, Ib */
        nextop = F8;
        GET_EX;
        tmp8u = F8;
        switch(tmp8u&7) {
            case 0: GX.f=simde_mm_cmpeq_ss(GX.f, EX->f); break;
            case 1: GX.f=simde_mm_cmplt_ss(GX.f, EX->f); break;
            case 2: GX.f=simde_mm_cmple_ss(GX.f, EX->f); break;
            case 3: GX.f=simde_mm_cmpunord_ss(GX.f, EX->f); break;
            case 4: GX.f=simde_mm_cmpneq_ss(GX.f, EX->f); break;
            case 5: GX.f=simde_mm_cmpge_ss(GX.f, EX->f); break;
            case 6: GX.f=simde_mm_cmpgt_ss(GX.f, EX->f); break;
            case 7: GX.f=simde_mm_cmpord_ss(GX.f, EX->f); break;
        }
        break;

    case 0xD6:  /* MOVQ2DQ Gx, Em */
        nextop = F8;
        GET_EM;
        GX.i = simde_mm_movpi64_epi64(*EM);
        break;

    case 0xE6:  /* CVTDQ2PD Gx, Ex */
        nextop = F8;
        GET_EX;
        GX.d = simde_mm_cvtepi32_pd(EX->i);
        break;

    default:
        goto _default;
    }
