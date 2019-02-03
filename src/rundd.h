    nextop = F8;
    switch(nextop) {
    
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
        fpu_fcom(emu, ST(nextop&7).d);   // bad, should handle QNaN and IA interrupt
        break;
    case 0xE8:  /* FUCOMP ST0, STx */
    case 0xE9:
    case 0xEA:
    case 0xEB:
    case 0xEC:
    case 0xED:
    case 0xEE:
    case 0xEF:
        fpu_fcom(emu, ST(nextop&7).d);   // bad, should handle QNaN and IA interrupt
        fpu_do_pop(emu);
        break;

    default:
        switch((nextop>>3)&7) {
            case 0: /* FLD double */
                GET_ED;
                fpu_do_push(emu);
                ST0.ll = *(int64_t*)op1;
                break;
            case 2: /* FST double */
                GET_ED;
                *(int64_t*)op1 = ST0.ll;
                break;
            case 3: /* FSTP double */
                GET_ED;
                *(int64_t*)op1 = ST0.ll;
                fpu_do_pop(emu);
                break;
            case 4: /* FRSTOR m108byte */
                GET_ED;
                fpu_loadenv(emu, (char*)op1, 0);
                // get the STx
                {
                    char* p =(char*)op1;
                    p += 28;
                    for (int i=0; i<8; ++i) {
                        LD2D(p, &ST(i).d);
                        p+=10;
                    }
                }
                break;
            case 6: /* FNSAVE m108byte */
                GET_ED;
                // ENV first...
                // warning, incomplete
                fpu_savenv(emu, (char*)op1, 0);
                // save the STx
                {
                    char* p =(char*)op1;
                    p += 28;
                    for (int i=0; i<8; ++i) {
                        D2LD(&ST(i).d, p);
                        p+=10;
                    }
                }
                reset_fpu(emu);
                break;
            default:
                goto _default;
        }
    }
