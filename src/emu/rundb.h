    nextop = F8;
    switch(nextop) {
    case 0xC0:      /* FCMOVNB ST(0), ST(i) */
    case 0xC1:
    case 0xC2:
    case 0xC3:
    case 0xC4:
    case 0xC5:
    case 0xC6:
    case 0xC7:
        CHECK_FLAGS(emu);
        if(!ACCESS_FLAG(F_CF))
            ST0.ll = ST(nextop&7).ll;
        break;
    case 0xC8:      /* FCMOVNE ST(0), ST(i) */
    case 0xC9:
    case 0xCA:
    case 0xCB:
    case 0xCC:
    case 0xCD:
    case 0xCE:
    case 0xCF:
        CHECK_FLAGS(emu);
        if(!ACCESS_FLAG(F_ZF))
            ST0.ll = ST(nextop&7).ll;
        break;
    case 0xD0:      /* FCMOVNBE ST(0), ST(i) */
    case 0xD1:
    case 0xD2:
    case 0xD3:
    case 0xD4:
    case 0xD5:
    case 0xD6:
    case 0xD7:
        CHECK_FLAGS(emu);
        if(!(ACCESS_FLAG(F_CF) || ACCESS_FLAG(F_ZF)))
            ST0.ll = ST(nextop&7).ll;
        break;
    case 0xD8:      /* FCMOVNU ST(0), ST(i) */
    case 0xD9:
    case 0xDA:
    case 0xDB:
    case 0xDC:
    case 0xDD:
    case 0xDE:
    case 0xDF:
        CHECK_FLAGS(emu);
        if(!ACCESS_FLAG(F_PF))
            ST0.ll = ST(nextop&7).ll;
        break;

    case 0xE1:      /* FDISI8087_NOP */
        break;
    case 0xE2:      /* FNCLEX */
        //Clears the floating-point exception flags (PE, UE, OE, ZE, DE, and IE), 
        // the exception summary status flag (ES), the stack fault flag (SF), and the busy flag (B) in the FPU status word
        emu->sw.f.F87_PE = 0;
        emu->sw.f.F87_UE = 0;
        emu->sw.f.F87_OE = 0;
        emu->sw.f.F87_ZE = 0;
        emu->sw.f.F87_DE = 0;
        emu->sw.f.F87_IE = 0;
        emu->sw.f.F87_ES = 0;
        emu->sw.f.F87_SF = 0;
        emu->sw.f.F87_B = 0;
        break;
    case 0xE3:      /* FNINIT */
        reset_fpu(emu);
        break;
    case 0xE8:  /* FUCOMI ST0, STx */
    case 0xE9:
    case 0xEA:
    case 0xEB:
    case 0xEC:
    case 0xED:
    case 0xEE:
    case 0xEF:
        fpu_fcomi(emu, ST(nextop&7).d);   // bad, should handle QNaN and IA interrupt
        break;

    case 0xF0:  /* FCOMI ST0, STx */
    case 0xF1:
    case 0xF2:
    case 0xF3:
    case 0xF4:
    case 0xF5:
    case 0xF6:
    case 0xF7:
        fpu_fcomi(emu, ST(nextop&7).d);
        break;
    case 0xE0:
    case 0xE4:
    case 0xE5:
    case 0xE6:
    case 0xE7:
        goto _default;
    default:
        switch((nextop>>3)&7) {
            case 0: /* FILD ST0, Ed */
                GET_ED;
                fpu_do_push(emu);
                ST0.d = ED->sdword[0];
                break;
            case 1: /* FISTTP Ed, ST0 */
                GET_ED;
                tmp32s = ST0.d; // TODO: Handling of FPU Exception
                if(tmp32s==0x7fffffff && isgreater(ST0.d, (double)(int32_t)0x7fffffff))
                    tmp32s = 0x80000000;
                fpu_do_pop(emu);
                ED->sdword[0] = tmp32s;
                break;
            case 2: /* FIST Ed, ST0 */
                GET_ED;
                if(isgreater(ST0.d, (double)(int32_t)0x7fffffff) || isless(ST0.d, -(double)(int32_t)0x7fffffff) || !isfinite(ST0.d))
                    ED->sdword[0] = 0x80000000;
                else {
                    volatile int32_t tmp = fpu_round(emu, ST0.d);    // tmp to avoid BUS ERROR
                    ED->sdword[0] = tmp;
                }
                break;
            case 3: /* FISTP Ed, ST0 */
                GET_ED;
                if(isgreater(ST0.d, (double)(int32_t)0x7fffffff) || isless(ST0.d, -(double)(int32_t)0x7fffffff) || !isfinite(ST0.d))
                    ED->sdword[0] = 0x80000000;
                else {
                    volatile int32_t tmp = fpu_round(emu, ST0.d);    // tmp to avoid BUS ERROR
                    ED->sdword[0] = tmp;
                }
                fpu_do_pop(emu);
                break;
            case 5: /* FLD ST0, Et */
                GET_ED;
                fpu_do_push(emu);
                memcpy(&STld(0).ld, ED, 10);
                LD2D(&STld(0), &ST(0).d);
                STld(0).ref = ST0.ll;
                break;
            case 7: /* FSTP tbyte */
                GET_ED;
                if(ST0.ll!=STld(0).ref)
                    D2LD(&ST0.d, ED);
                else
                    memcpy(ED, &STld(0).ld, 10);
                fpu_do_pop(emu);
                break;
            default:
                goto _default;
        }
    }
