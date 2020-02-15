    nextop = F8;
    switch(nextop) {
    
    case 0xC0:  /* FFREE STx */
    case 0xC1:
    case 0xC2:
    case 0xC3:
    case 0xC4:
    case 0xC5:
    case 0xC6:
    case 0xC7:
        fpu_do_free(emu, nextop-0xC0);
        break;

    case 0xD0:  /* FST ST0, STx */
    case 0xD1:
    case 0xD2:
    case 0xD3:
    case 0xD4:
    case 0xD5:
    case 0xD6:
    case 0xD7:
        ST(nextop&7).ll = ST0.ll;
        break;
    case 0xD8:  /* FSTP ST0, STx */
    case 0xD9:
    case 0xDA:
    case 0xDB:
    case 0xDC:
    case 0xDD:
    case 0xDE:
    case 0xDF:
        ST(nextop&7).ll = ST0.ll;
        fpu_do_pop(emu);
        break;
    case 0xE0:  /* FUCOM ST0, STx */
    case 0xE1:
    case 0xE2:
    case 0xE3:
    case 0xE4:
    case 0xE5:
    case 0xE6:
    case 0xE7:
        #ifdef USE_FLOAT
        fpu_fcom(emu, ST(nextop&7).f);
        #else
        fpu_fcom(emu, ST(nextop&7).d);   // bad, should handle QNaN and IA interrupt
        #endif
        break;
    case 0xE8:  /* FUCOMP ST0, STx */
    case 0xE9:
    case 0xEA:
    case 0xEB:
    case 0xEC:
    case 0xED:
    case 0xEE:
    case 0xEF:
        #ifdef USE_FLOAT
        fpu_fcom(emu, ST(nextop&7).f);
        #else
        fpu_fcom(emu, ST(nextop&7).d);   // bad, should handle QNaN and IA interrupt
        #endif
        fpu_do_pop(emu);
        break;

    case 0xC8:
    case 0xC9:
    case 0xCA:
    case 0xCB:
    case 0xCC:
    case 0xCD:
    case 0xCE:
    case 0xCF:
    case 0xF0:
    case 0xF1:
    case 0xF2:
    case 0xF3:
    case 0xF4:
    case 0xF5:
    case 0xF6:
    case 0xF7:
    case 0xF8:
    case 0xF9:
    case 0xFA:
    case 0xFB:
    case 0xFC:
    case 0xFD:
    case 0xFE:
    case 0xFF:
        goto _default;

    default:
        switch((nextop>>3)&7) {
            case 0: /* FLD double */
                GET_ED;
                fpu_do_push(emu);
                #ifdef USE_FLOAT
                *(uint64_t*)&d = *(int64_t*)ED;
                ST0.f = d;
                #else
                ST0.ll = *(int64_t*)ED;
                #endif
                break;
            case 1: /* FISTTP ED qword */
                GET_ED;
                #ifdef USE_FLOAT
                *(int64_t*)ED = ST0.f;
                #else
                *(int64_t*)ED = ST0.d;
                #endif
                fpu_do_pop(emu);
                break;
            case 2: /* FST double */
                GET_ED;
                #ifdef USE_FLOAT
                d = ST0.f;
                *(int64_t*)ED = *(uint64_t*)&d;
                #else
                *(int64_t*)ED = ST0.ll;
                #endif
                break;
            case 3: /* FSTP double */
                GET_ED;
                #ifdef USE_FLOAT
                d = ST0.f;
                *(int64_t*)ED = *(uint64_t*)&d;
                #else
                *(int64_t*)ED = ST0.ll;
                #endif
                fpu_do_pop(emu);
                break;
            case 4: /* FRSTOR m108byte */
                GET_ED;
                fpu_loadenv(emu, (char*)ED, 0);
                // get the STx
                {
                    char* p =(char*)ED;
                    p += 28;
                    for (int i=0; i<8; ++i) {
                        #ifdef USE_FLOAT
                        d = ST(i).f;
                        LD2D(p, &d);
                        #else
                        LD2D(p, &ST(i).d);
                        #endif
                        p+=10;
                    }
                }
                break;
            case 6: /* FNSAVE m108byte */
                GET_ED;
                // ENV first...
                // warning, incomplete
                fpu_savenv(emu, (char*)ED, 0);
                // save the STx
                {
                    char* p =(char*)ED;
                    p += 28;
                    for (int i=0; i<8; ++i) {
                        #ifdef USE_FLOAT
                        D2LD(&d, p);
                        ST(i).f = d;
                        #else
                        D2LD(&ST(i).d, p);
                        #endif
                        p+=10;
                    }
                }
                reset_fpu(emu);
                break;
            case 7: /* FNSTSW m2byte */
                GET_ED;
                emu->sw.f.F87_TOP = emu->top&7;
                *(uint16_t*)ED = emu->sw.x16;
                break;
            default:
                goto _default;
        }
    }
