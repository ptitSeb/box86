    nextop = F8;
    switch(nextop) {

    case 0xC0:      /* FCMOVB ST(0), ST(i) */
    case 0xC1:
    case 0xC2:
    case 0xC3:
    case 0xC4:
    case 0xC5:
    case 0xC6:
    case 0xC7:
        CHECK_FLAGS(emu);
        if(ACCESS_FLAG(F_CF))
            ST0.ll = ST(nextop&7).ll;
        break;
    case 0xC8:      /* FCMOVE ST(0), ST(i) */
    case 0xC9:
    case 0xCA:
    case 0xCB:
    case 0xCC:
    case 0xCD:
    case 0xCE:
    case 0xCF:
        CHECK_FLAGS(emu);
        if(ACCESS_FLAG(F_ZF))
            ST0.ll = ST(nextop&7).ll;
        break;
    case 0xD0:      /* FCMOVBE ST(0), ST(i) */
    case 0xD1:
    case 0xD2:
    case 0xD3:
    case 0xD4:
    case 0xD5:
    case 0xD6:
    case 0xD7:
        CHECK_FLAGS(emu);
        if(ACCESS_FLAG(F_CF) || ACCESS_FLAG(F_ZF))
            ST0.ll = ST(nextop&7).ll;
        break;
    case 0xD8:      /* FCMOVU ST(0), ST(i) */
    case 0xD9:
    case 0xDA:
    case 0xDB:
    case 0xDC:
    case 0xDD:
    case 0xDE:
    case 0xDF:
        CHECK_FLAGS(emu);
        if(ACCESS_FLAG(F_PF))
            ST0.ll = ST(nextop&7).ll;
        break;
    
    case 0xE9:      /* FUCOMPP */
        #ifdef USE_FLOAT
        fpu_fcom(emu, ST1.f);
        #else
        fpu_fcom(emu, ST1.d);   // bad, should handle QNaN and IA interrupt
        #endif
        fpu_do_pop(emu);
        fpu_do_pop(emu);
        break;

    case 0xE4:
    case 0xF0:
    case 0xF1:
    case 0xF4:
    case 0xF5:
    case 0xF6:
    case 0xF7:
    case 0xF8:
    case 0xF9:
    case 0xFD:
        UnimpOpcode(emu);
        break;
    default:
        switch((nextop>>3)&7) {
            case 0:     /* FIADD ST0, Ed int */
                GET_ED;
                #ifdef USE_FLOAT
                ST0.f += ED->sdword[0];
                #else
                ST0.d += ED->sdword[0];
                #endif
                break;
            case 1:     /* FIMUL ST0, Ed int */
                GET_ED;
                #ifdef USE_FLOAT
                ST0.f *= ED->sdword[0];
                #else
                ST0.d *= ED->sdword[0];
                #endif
                break;
            case 2:     /* FICOM ST0, Ed int */
                GET_ED;
                fpu_fcom(emu, ED->sdword[0]);
                break;
            case 3:     /* FICOMP ST0, Ed int */
                GET_ED;
                fpu_fcom(emu, ED->sdword[0]);
                fpu_do_pop(emu);
                break;
            case 4:     /* FISUB ST0, Ed int */
                GET_ED;
                #ifdef USE_FLOAT
                ST0.f -= ED->sdword[0];
                #else
                ST0.d -= ED->sdword[0];
                #endif
                break;
            case 5:     /* FISUBR ST0, Ed int */
                GET_ED;
                #ifdef USE_FLOAT
                ST0.f = ED->sdword[0] - ST0.f;
                #else
                ST0.d = ED->sdword[0] - ST0.d;
                #endif
                break;
            case 6:     /* FIDIV ST0, Ed int */
                GET_ED;
                #ifdef USE_FLOAT
                ST0.f /= ED->sdword[0];
                #else
                ST0.d /= ED->sdword[0];
                #endif
                break;
            case 7:     /* FIDIVR ST0, Ed int */
                GET_ED;
                #ifdef USE_FLOAT
                ST0.f = ED->sdword[0] / ST0.f;
                #else
                ST0.d = ED->sdword[0] / ST0.d;
                #endif
                break;
        }
    }
