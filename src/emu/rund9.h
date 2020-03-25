#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
    nextop = F8;
    switch (nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:  /* FLD STx */
            ll = ST(nextop&7).ll;
            fpu_do_push(emu);
            ST0.ll = ll;
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:  /* FXCH STx */
            ll = ST(nextop&7).ll;
            ST(nextop&7).ll = ST0.ll;
            ST0.ll = ll;
            break;

        case 0xD0:  /* FNOP */
            break;

        case 0xE0:  /* FCHS */
            #ifdef USE_FLOAT
            ST0.f = -ST0.f;
            #else
            ST0.d = -ST0.d;
            #endif
            break;
        case 0xE1:  /* FABS */
            #ifdef USE_FLOAT
            ST0.f = fabsf(ST0.f);
            #else
            ST0.d = fabs(ST0.d);
            #endif
            break;
        
        case 0xE4:  /* FTST */
            fpu_ftst(emu);
            break;
        case 0xE5:  /* FXAM */
            fpu_fxam(emu);
            break;

        case 0xE8:  /* FLD1 */
            fpu_do_push(emu);
            #ifdef USE_FLOAT
            ST0.f = 1.0f;
            #else
            ST0.d = 1.0;
            #endif
            break;
        case 0xE9:  /* FLDL2T */
            fpu_do_push(emu);
            #ifdef USE_FLOAT
            ST0.f = L2T;
            #else
            ST0.d = L2T;
            #endif
            break;
        case 0xEA:  /* FLDL2E */
            fpu_do_push(emu);
            #ifdef USE_FLOAT
            ST0.f = L2E;
            #else
            ST0.d = L2E;
            #endif
            break;
        case 0xEB:  /* FLDPI */
            fpu_do_push(emu);
            #ifdef USE_FLOAT
            ST0.f = PI;
            #else
            ST0.d = PI;
            #endif
            break;
        case 0xEC:  /* FLDLG2 */
            fpu_do_push(emu);
            #ifdef USE_FLOAT
            ST0.f = LG2;
            #else
            ST0.d = LG2;
            #endif
            break;
        case 0xED:  /* FLDLN2 */
            fpu_do_push(emu);
            #ifdef USE_FLOAT
            ST0.f = LN2;
            #else
            ST0.d = LN2;
            #endif
            break;
        case 0xEE:  /* FLDZ */
            fpu_do_push(emu);
            #ifdef USE_FLOAT
            ST0.f = 0.0f;
            #else
            ST0.d = 0.0;
            #endif
            break;

        case 0xF0:  /* F2XM1 */
            #ifdef USE_FLOAT
            ST0.f = exp2f(ST0.f) - 1.0f;
            #else
            ST0.d = exp2(ST0.d) - 1.0;
            #endif
            break;
        case 0xF1:  /* FYL2X */
            #ifdef USE_FLOAT
            ST(1).f = log2f(ST0.f)*ST(1).f;
            #else
            ST(1).d = log2(ST0.d)*ST(1).d;
            #endif
            fpu_do_pop(emu);
            break;
        case 0xF2:  /* FTAN */
            #ifdef USE_FLOAT
            ST0.f = tanf(ST0.f);
            fpu_do_push(emu);
            ST0.f = 1.0f;
            #else
            ST0.d = tan(ST0.d);
            fpu_do_push(emu);
            ST0.d = 1.0;
            #endif
            break;
        case 0xF3:  /* FPATAN */
            #ifdef USE_FLOAT
            ST1.f = atan2f(ST1.f, ST0.f);
            #else
            ST1.d = atan2(ST1.d, ST0.d);
            #endif
            fpu_do_pop(emu);
            break;
        case 0xF4:  /* FXTRACT */
            #ifdef USE_FLOAT
            tmp32s = (ST0.ll&0x7f700000)>>23;
            tmp32s -= 127;
            ST0.f /= exp2f(tmp32s);
            fpu_do_push(emu);
            ST0.f = tmp32s;
            #else
            tmp32s = (ST0.ll&0x7ff0000000000000LL)>>52;
            tmp32s -= 1023;
            ST0.d /= exp2(tmp32s);
            fpu_do_push(emu);
            ST0.d = tmp32s;
            #endif
            break;

        case 0xF8:  /* FPREM */
            #ifdef USE_FLOAT
            ll = ST0.f / ST1.f;
            ST0.f -= ST1.f * ll;
            #else
            ll = ST0.d / ST1.d;
            ST0.d -= ST1.d * ll;
            #endif
            emu->sw.f.F87_C2 = 0;
            emu->sw.f.F87_C0 = (ll&1);
            emu->sw.f.F87_C3 = ((ll>>1)&1);
            emu->sw.f.F87_C1 = ((ll>>2)&1);
            break;
        case 0xF5:  /* FPREM1 */
            // get exponant(ST(0))-exponant(ST(1)) in temp32s
            #ifdef USE_FLOAT
            tmp32s = ((ST0.ll&0x7f700000)>>23) - ((ST1.ll&0x7f700000)>>23);
            #else
            tmp32s = ((ST0.ll&0x7ff0000000000000LL)>>52) - ((ST1.ll&0x7ff0000000000000LL)>>52);
            #endif
            if(tmp32s<64)
            {
                #ifdef USE_FLOAT
                ll = (int64_t)round(ST0.f)/ST1.f;
                ST0.f = ST0.f - (ST1.f*ll);
                #else
                ll = (int64_t)round(ST0.d)/ST1.d;
                ST0.d = ST0.d - (ST1.d*ll);
                #endif
                emu->sw.f.F87_C2 = 0;
                emu->sw.f.F87_C1 = (ll&1)?1:0;
                emu->sw.f.F87_C2 = (ll&2)?1:0;
                emu->sw.f.F87_C0 = (ll&4)?1:0;
            } else {
                #ifdef USE_FLOAT
                ll = (int64_t)floor((ST0.f/ST1.f)/powf(2, 32));
                ST0.f = ST0.f - ST1.f*ll*powf(2, 32);
                #else
                ll = (int64_t)floor((ST0.d/ST1.d)/pow(2, 32));
                ST0.d = ST0.d - ST1.d*ll*pow(2, 32);
                #endif
                emu->sw.f.F87_C2 = 1;
            }
            break;

        case 0xF9:  /* FYL2XP1 */
            #ifdef USE_FLOAT
            ST(1).f = log2f(ST0.f + 1.0f)*ST(1).f;
            #else
            ST(1).d = log2(ST0.d + 1.0)*ST(1).d;
            #endif
            fpu_do_pop(emu);
            break;
        case 0xFA:  /* FSQRT */
            #ifdef USE_FLOAT
            ST0.f = sqrtf(ST0.f);
            #else
            ST0.d = sqrt(ST0.d);
            #endif
            break;
        case 0xFB:  /* FSINCOS */
            fpu_do_push(emu);
            #ifdef USE_FLOAT
            sincosf(ST1.f, &ST1.f, &ST0.f);
            #else
            sincos(ST1.d, &ST1.d, &ST0.d);
            #endif
            break;
        case 0xFC:  /* FRNDINT */
            #ifdef USE_FLOAT
            ST0.f = fpu_round(emu, ST0.f);
            #else
            ST0.d = fpu_round(emu, ST0.d);
            #endif
            break;
        case 0xFD:  /* FSCALE */
            // this could probably be done by just altering the exponant part of the float...
            #ifdef USE_FLOAT
            ST0.f *= exp2f(truncf(ST1.f));
            #else
            ST0.d *= exp2(trunc(ST1.d));
            #endif
            break;
        case 0xFE:  /* FSIN */
            #ifdef USE_FLOAT
            ST0.f = sinf(ST0.f);
            #else
            ST0.d = sin(ST0.d);
            #endif
            break;
        case 0xFF:  /* FCOS */
            #ifdef USE_FLOAT
            ST0.f = cosf(ST0.f);
            #else
            ST0.d = cos(ST0.d);
            #endif
            break;


        case 0xD1:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
        case 0xE2:
        case 0xE3:
        case 0xE6:
        case 0xE7:
        case 0xEF:
        case 0xF6:
        case 0xF7:
            goto _default;
        default:
        switch((nextop>>3)&7) {
            case 0:     /* FLD ST0, Ed float */
                GET_ED;
                fpu_do_push(emu);
                #ifdef USE_FLOAT
                ST0.ll = ED->dword[0];
                #else
                if(!(((uintptr_t)ED)&3))
                    ST0.d = *(float*)ED;
                else {
                    memcpy(&f, ED->dword, sizeof(float));
                    ST0.d = f;
                }
                #endif
                break;
            case 2:     /* FST Ed, ST0 */
                GET_ED;
                #ifdef USE_FLOAT
                *(float*)ED = ST0.f;
                #else
                if(!(((uintptr_t)ED)&3))
                    *(float*)ED = ST0.d;
                else {
                    f = ST0.d;
                    memcpy(ED->dword, &f, sizeof(float));
                }
                #endif
                break;
            case 3:     /* FSTP Ed, ST0 */
                GET_ED;
                #ifdef USE_FLOAT
                ED->dword[0] = ST0.ll;
                #else
                if(!(((uintptr_t)ED)&3))
                    *(float*)ED = ST0.d;
                else {
                    f = ST0.d;
                    memcpy(ED->dword, &f, sizeof(float));
                }
                #endif
                fpu_do_pop(emu);
                break;
            case 4:     /* FLDENV m */
                // warning, incomplete
                GET_ED;
                fpu_loadenv(emu, (char*)ED, 0);
                break;
            case 5:     /* FLDCW Ew */
                GET_EW;
                emu->cw = EW->word[0];
                // do something with cw?
                emu->round = (fpu_round_t)((emu->cw >> 10) & 3);
                break;
            case 6:     /* FNSTENV m */
                // warning, incomplete
                GET_ED;
                fpu_savenv(emu, (char*)ED, 0);
                ED->dword[0] = emu->cw;
                ED->dword[1] = emu->sw.x16;
                // tagword: 2bits*8
                ED->word[4] = 0;
                for (int i=0; i<8; ++i)
                    ED->word[4] |= (emu->p_regs[i].tag)<<(i*2);
                // intruction pointer: 48bits
                // data (operand) pointer: 48bits
                // last opcode: 11bits save: 16bits restaured (1st and 2nd opcode only)
                break;
            case 7: /* FNSTCW Ew */
                GET_EW;
                EW->word[0] = emu->cw;
                break;
            default:
                goto _default;
        }
    }
#pragma GCC diagnostic pop
