#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
    nextop = F8;
    switch(nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:  /* FADD */
            #ifdef USE_FLOAT
            ST(nextop&7).f += ST0.f;
            #else
            ST(nextop&7).d += ST0.d;
            #endif
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:  /* FMUL */
            #ifdef USE_FLOAT
            ST(nextop&7).f *= ST0.f;
            #else
            ST(nextop&7).d *= ST0.d;
            #endif
            break;
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:  /* FCOM */
            #ifdef USE_FLOAT
            fpu_fcom(emu, ST(nextop&7).f);
            #else
            fpu_fcom(emu, ST(nextop&7).d);
            #endif
            break;
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:  /* FCOMP */
            #ifdef USE_FLOAT
            fpu_fcom(emu, ST(nextop&7).f);
            #else
            fpu_fcom(emu, ST(nextop&7).d);  // TODO: is this ok?
            #endif
            fpu_do_pop(emu);
            break;
        case 0xE0:
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:  /* FSUBR */
            #ifdef USE_FLOAT
            ST(nextop&7).f = ST0.f -ST(nextop&7).f;
            #else
            ST(nextop&7).d = ST0.d -ST(nextop&7).d;
            #endif
            break;
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:  /* FSUB */
            #ifdef USE_FLOAT
            ST(nextop&7).f -= ST0.f;
            #else
            ST(nextop&7).d -= ST0.d;
            #endif
            break;
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:  /* FDIVR */
            #ifdef USE_FLOAT
            ST(nextop&7).f = ST0.f / ST(nextop&7).f;
            #else
            ST(nextop&7).d = ST0.d / ST(nextop&7).d;
            #endif
            break;
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:  /* FDIV */
            #ifdef USE_FLOAT
            ST(nextop&7).f /=  ST0.f;
            #else
            ST(nextop&7).d /=  ST0.d;
            #endif
            break;
        default:
            GET_ED;
            switch((nextop>>3)&7) {
            case 0:         /* FADD ST0, double */
                #ifdef USE_FLOAT
                if(!(((uintptr_t)ED)&7))
                    ST0.f += *(double*)ED;
                else {
                    *(uint64_t*)&d = *(uint64_t*)ED;
                    ST0.f += d;
                }
                #else
                if(!(((uintptr_t)ED)&7))
                    ST0.d += *(double*)ED;
                else {
                    *(uint64_t*)&d = *(uint64_t*)ED;
                    ST0.d += d;
                }
                #endif
                break;
            case 1:         /* FMUL ST0, double */
                #ifdef USE_FLOAT
                if(!(((uintptr_t)ED)&7))
                    ST0.f *= *(double*)ED;
                else {
                    *(uint64_t*)&d = *(uint64_t*)ED;
                    ST0.f *= d;
                }
                #else
                if(!(((uintptr_t)ED)&7))
                    ST0.d *= *(double*)ED;
                else {
                    *(uint64_t*)&d = *(uint64_t*)ED;
                    ST0.d *= d;
                }
                #endif
                break;
            case 2:      /* FCOM ST0, double */
                if(!(((uintptr_t)ED)&7))
                    fpu_fcom(emu, *(double*)ED);
                else {
                    *(uint64_t*)&d = *(uint64_t*)ED;
                    fpu_fcom(emu, d);
                }
                break;
            case 3:     /* FCOMP ST0, double */
                if(!(((uintptr_t)ED)&7))
                    fpu_fcom(emu, *(double*)ED);
                else {
                    *(uint64_t*)&d = *(uint64_t*)ED;
                    fpu_fcom(emu, d);
                }
                fpu_do_pop(emu);
                break;
            case 4:         /* FSUB ST0, double */
                #ifdef USE_FLOAT
                if(!(((uintptr_t)ED)&7))
                    ST0.f -= *(double*)ED;
                else {
                    *(uint64_t*)&d = *(uint64_t*)ED;
                    ST0.f -= d;
                }
                #else
                if(!(((uintptr_t)ED)&7))
                    ST0.d -= *(double*)ED;
                else {
                    *(uint64_t*)&d = *(uint64_t*)ED;
                    ST0.d -= d;
                }
                #endif
                break;
            case 5:         /* FSUBR ST0, double */
                #ifdef USE_FLOAT
                if(!(((uintptr_t)ED)&7))
                    ST0.f = *(double*)ED - ST0.f;
                else {
                    *(uint64_t*)&d = *(uint64_t*)ED;
                    ST0.f = d - ST0.f;
                }
                #else
                if(!(((uintptr_t)ED)&7))
                    ST0.d = *(double*)ED - ST0.d;
                else {
                    *(uint64_t*)&d = *(uint64_t*)ED;
                    ST0.d = d - ST0.d;
                }
                #endif
                break;
            case 6:         /* FDIV ST0, double */
                #ifdef USE_FLOAT
                if(!(((uintptr_t)ED)&7))
                    ST0.f /= *(double*)ED;
                else {
                    *(uint64_t*)&d = *(uint64_t*)ED;
                    ST0.f /= d;
                }
                #else
                if(!(((uintptr_t)ED)&7))
                    ST0.d /= *(double*)ED;
                else {
                    *(uint64_t*)&d = *(uint64_t*)ED;
                    ST0.d /= d;
                }
                #endif
                break;
            case 7:         /* FDIVR ST0, double */
                #ifdef USE_FLOAT
                if(!(((uintptr_t)ED)&7))
                    ST0.f = *(double*)ED / ST0.f;
                else {
                    *(uint64_t*)&d = *(uint64_t*)ED;
                    ST0.f = d / ST0.f;
                }
                #else
                if(!(((uintptr_t)ED)&7))
                    ST0.d = *(double*)ED / ST0.d;
                else {
                    *(uint64_t*)&d = *(uint64_t*)ED;
                    ST0.d = d / ST0.d;
                }
                #endif
                break;
            default:
                goto _default;
        }
    }
#pragma GCC diagnostic pop
