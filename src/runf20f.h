    
    opcode = (fetched>>16)&0xff;
    R_EIP++;
    nextop = (fetched>>24)&0xff;
    switch(opcode) {

    case 0x10:  /* MOVSD Gx, Ex */
        R_EIP++;
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->q[0] = opx2->q[0];
        if(!(opx2>=emu->xmm && opx2<=(emu->xmm+7))) {
            // op2 is not a register
            opx1->q[1] = 0;
        }
        break;
    case 0x11:  /* MOVSQ Ex, Gx */
        R_EIP++;
        GetEx(emu, &opx1, nextop);
        GetGx(emu, &opx2, nextop);
        opx1->q[0] = opx2->q[0];
        break;

    case 0x2A:  /* CVTSI2SD Gx, Ed */
        R_EIP++;
        GetEd(emu, &op2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] = op2->sdword[0];
        break;

    case 0x2C:  /* CVTTSD2SI Gd, Ex */
    case 0x2D:  /* CVTSD2SI Gd, Ex */
        R_EIP++;
        GetEx(emu, &opx2, nextop);
        GetG(emu, &op1, nextop);
        op1->sdword[0] = opx2->d[0];
        break;

    case 0x51:  /* SQRTSD Gx, Ex */
        R_EIP++;
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] = sqrt(opx2->d[0]);
        break;

    case 0x58:  /* ADDSD Gx, Ex */
        R_EIP++;
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] += opx2->d[0];
        break;
    case 0x59:  /* MULSD Gx, Ex */
        R_EIP++;
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] *= opx2->d[0];
        break;

    case 0x5A:  /* CVTSD2SS Gx, Ex */
        R_EIP++;
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->f[0] = opx2->d[0];
        break;

    case 0x5C:  /* SUBSD Gx, Ex */
        R_EIP++;
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] -= opx2->d[0];
        break;
    case 0x5D:  /* MINSD Gx, Ex */
        R_EIP++;
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        if (isnan(opx1->d[0]) || isnan(opx2->d[0]) || isless(opx2->d[0], opx1->d[0]))
            opx1->d[0] = opx2->d[0];
        break;
    case 0x5E:  /* DIVSD Gx, Ex */
        R_EIP++;
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->d[0] /= opx2->d[0];
        break;
    case 0x5F:  /* MAXSD Gx, Ex */
        R_EIP++;
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        if (isnan(opx1->d[0]) || isnan(opx2->d[0]) || isgreater(opx2->d[0], opx1->d[0]))
            opx1->d[0] = opx2->d[0];
        break;

    case 0x70:  /* PSHUFLW Gx, Ex, Ib */
        R_EIP++;
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
        R_EIP++;
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
        R_EIP++;
        GetEx(emu, &opx2, nextop);
        GetGm(emu, &opm1, nextop);
        opm1->q = opx2->q[0];
        break;

    case 0xE6:  /* CVTPD2DQ Gx, Ex */
        R_EIP++;
        GetEx(emu, &opx2, nextop);
        GetGx(emu, &opx1, nextop);
        opx1->sd[0] = opx2->d[0];
        opx1->sd[1] = opx2->d[1];
        opx1->q[1] = 0;
        break;

    default:
        UnimpOpcode(emu);
        goto fini;
    }

