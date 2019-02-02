    opcode = F8;
    switch(opcode) {

    case 0x10:  /* MOVSS Gx Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->ud[0] = opx2->ud[0];
        if(!(opx2>=emu->xmm && opx2<=(emu->xmm+7))) {
            // op2 is not a register
            opx1->ud[1] = opx1->ud[2] = opx1->ud[3] = 0;
        }
        break;
    case 0x11:  /* MOVSS Ex Gx */
        nextop = F8;
        opx1=GetEx(emu, nextop);
        opx2=GetGx(emu, nextop);
        opx1->ud[0] = opx2->ud[0];
        break;

    case 0x2A:  /* CVTSI2SS Gx, Ed */
        nextop = F8;
        op2=GetEd(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->f[0] = op2->sdword[0];
        break;

    case 0x2C:  /* CVTTSS2SI Gd, Ex */
    case 0x2D:  /* CVTSS2SI Gd, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        op1=GetG(emu, nextop);
        op1->sdword[0] = opx2->f[0];
        break;

    case 0x51:  /* SQRTSS Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->f[0] = sqrtf(opx2->f[0]);
        break;

    case 0x58:  /* ADDSS Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->f[0] += opx2->f[0];
        break;
    case 0x59:  /* MULSS Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->f[0] *= opx2->f[0];
        break;
    case 0x5A:  /* CVTSS2SD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->d[0] = opx2->f[0];
        break;
    case 0x5B:  /* CVTTPS2DQ Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->sd[0] = opx2->f[0];
        opx1->sd[1] = opx2->f[1];
        opx1->sd[2] = opx2->f[2];
        opx1->sd[3] = opx2->f[3];
        break;

    case 0x5C:  /* SUBSS Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->f[0] -= opx2->f[0];
        break;
    case 0x5D:  /* MINSS Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        if(isnan(opx1->f[0]) || isnan(opx2->f[0]) || isless(opx2->f[0], opx1->f[0]))
            opx1->f[0] = opx2->f[0];
        break;
    case 0x5E:  /* DIVSS Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->f[0] /= opx2->f[0];
        break;
    case 0x5F:  /* MAXSS Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        if (isnan(opx1->f[0]) || isnan(opx2->f[0]) || isgreater(opx2->f[0], opx1->f[0]))
            opx1->f[0] = opx2->f[0];
        break;

    case 0x6F:  /* MOVDQU Gx, Ex */
        nextop = F8;
        opx1=GetEx(emu, nextop);
        opx2=GetGx(emu, nextop);
        memcpy(opx2, opx1, 16);
        break;
    case 0x70:  /* PSHUFHW Gx, Ex, Ib */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        tmp8u = F8;
        if(opx1==opx2) {
            for (int i=0; i<4; ++i)
                eax1.uw[4+i] = opx2->uw[4+((tmp8u>>(i*2))&3)];
            opx1->q[1] = eax1.q[1];
        } else {
            for (int i=0; i<4; ++i)
                opx1->uw[4+i] = opx2->uw[4+((tmp8u>>(i*2))&3)];
            opx1->q[0] = opx2->q[0];
        }
        break;

    case 0x7E:  /* MOVQ Gx, Ex */
        nextop = F8;
        opx1=GetEx(emu, nextop);
        opx2=GetGx(emu, nextop);
        opx2->q[0] = opx1->q[0];
        opx2->q[1] = 0;
        break;
    case 0x7F:  /* MOVDQU Ex, Gx */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx2->q[0] = opx1->q[0];
        opx2->q[1] = opx1->q[1];
        break;

    case 0xC2:  /* CMPSS Gx, Ex, Ib */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        tmp8u = F8;
        tmp8s = 0;
        switch(tmp8u&7) {
            case 0: tmp8s=(opx1->f[0] == opx2->f[0]); break;
            case 1: tmp8s=isless(opx1->f[0], opx2->f[0]); break;
            case 2: tmp8s=islessequal(opx1->f[0], opx2->f[0]); break;
            case 3: tmp8s=isnan(opx1->f[0]) || isnan(opx2->f[0]); break;
            case 4: tmp8s=(opx1->f[0] != opx2->f[0]); break;
            case 5: tmp8s=isgreaterequal(opx1->f[0], opx2->f[0]); break;
            case 6: tmp8s=isgreater(opx1->f[0], opx2->f[0]); break;
            case 7: tmp8s=!isnan(opx1->f[0]) && !isnan(opx2->f[0]); break;
        }
        if(tmp8s)
            opx1->ud[0] = 0xffffffff;
        else
            opx1->ud[0] = 0;
        break;

    case 0xD6:  /* MOVQ2DQ Gx, Em */
        nextop = F8;
        opm2=GetEm(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->q[0] = opm2->q;
        opx1->q[1] = 0;
        break;

    case 0xE6:  /* CVTDQ2PD Gx, Ex */
        nextop = F8;
        opx2=GetEx(emu, nextop);
        opx1=GetGx(emu, nextop);
        opx1->d[1] = opx2->sd[1];
        opx1->d[0] = opx2->sd[0];
        break;

    default:
        UnimpOpcode(emu);
        goto fini;
    }
