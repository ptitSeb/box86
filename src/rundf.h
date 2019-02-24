    nextop = F8;
    switch (nextop) {
    case 0xE0:  /* FNSTSW AX */
        emu->sw.f.F87_TOP = emu->top&7;
        R_AX = emu->sw.x16;
        break;

    case 0xE8:  /* FUCOMIP ST0, STx */
    case 0xE9:
    case 0xEA:
    case 0xEB:
    case 0xEC:
    case 0xED:
    case 0xEE:
    case 0xEF:
        #ifdef USE_FLOAT
        fpu_fcomi(emu, ST(nextop&7).f);
        #else
        fpu_fcomi(emu, ST(nextop&7).d);   // bad, should handle QNaN and IA interrupt
        #endif
        fpu_do_pop(emu);
        break;

    case 0xF0:  /* FCOMIP ST0, STx */
    case 0xF1:
    case 0xF2:
    case 0xF3:
    case 0xF4:
    case 0xF5:
    case 0xF6:
    case 0xF7:
        #ifdef USE_FLOAT
        fpu_fcomi(emu, ST(nextop&7).f);
        #else
        fpu_fcomi(emu, ST(nextop&7).d);
        #endif
        fpu_do_pop(emu);
        break;

    case 0xC0:
    case 0xC1:
    case 0xC2:
    case 0xC3:
    case 0xC4:
    case 0xC5:
    case 0xC6:
    case 0xC7:
    case 0xC8:
    case 0xC9:
    case 0xCA:
    case 0xCB:
    case 0xCC:
    case 0xCD:
    case 0xCE:
    case 0xCF:
    case 0xD0:
    case 0xD1:
    case 0xD2:
    case 0xD3:
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
    case 0xE1:
    case 0xE2:
    case 0xE3:
    case 0xE4:
    case 0xE5:
    case 0xE6:
    case 0xE7:
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
        case 0: /* FILD ST0, Gw */
            GET_EW;
            tmp16s = EW->sword[0];
            fpu_do_push(emu);
            #ifdef USE_FLOAT
            ST0.f = tmp16s;
            #else
            ST0.d = tmp16s;
            #endif
            break;
        case 1: /* FISTTP Ew, ST0 */
            GET_EW;
            #ifdef USE_FLOAT
            tmp16s = ST0.f;
            #else
            tmp16s = ST0.d;
            #endif
            EW->sword[0] = tmp16s;
            fpu_do_pop(emu);
            break;
        case 2: /* FIST Ew, ST0 */
            GET_EW;
            #ifdef USE_FLOAT
            if(isgreater(ST0.f, (float)(int32_t)0x7fff) || isless(ST0.f, -(float)(int32_t)0x7fff))
                EW->sword[0] = 0x8000;
            else {
                switch(emu->round) {
                    case ROUND_Nearest:
                        EW->sword[0] = floorf(ST0.f+0.5);
                        break;
                    case ROUND_Down:
                        EW->sword[0] = floorf(ST0.f);
                        break;
                    case ROUND_Up:
                        EW->sword[0] = ceilf(ST0.f);
                        break;
                    case ROUND_Chop:
                        EW->sword[0] = ST0.f;
                        break;
                }
            }
            #else
            if(isgreater(ST0.d, (double)(int32_t)0x7fff) || isless(ST0.d, -(double)(int32_t)0x7fff))
                EW->sword[0] = 0x8000;
            else {
                switch(emu->round) {
                    case ROUND_Nearest:
                        EW->sword[0] = floor(ST0.d+0.5);
                        break;
                    case ROUND_Down:
                        EW->sword[0] = floor(ST0.d);
                        break;
                    case ROUND_Up:
                        EW->sword[0] = ceil(ST0.d);
                        break;
                    case ROUND_Chop:
                        EW->sword[0] = ST0.d;
                        break;
                }
            }
            #endif
            break;
        case 3: /* FISTP Ew, ST0 */
            GET_EW;
            #ifdef USE_FLOAT
            if(isgreater(ST0.f, (float)(int32_t)0x7fff) || isless(ST0.f, -(float)(int32_t)0x7fff))
                EW->sword[0] = 0x8000;
            else {
                switch(emu->round) {
                    case ROUND_Nearest:
                        EW->sword[0] = floorf(ST0.f+0.5);
                        break;
                    case ROUND_Down:
                        EW->sword[0] = floorf(ST0.f);
                        break;
                    case ROUND_Up:
                        EW->sword[0] = ceilf(ST0.f);
                        break;
                    case ROUND_Chop:
                        EW->sword[0] = ST0.f;
                        break;
                }
            }
            #else
            if(isgreater(ST0.d, (double)(int32_t)0x7fff) || isless(ST0.d, -(double)(int32_t)0x7fff))
                EW->sword[0] = 0x8000;
            else {
                switch(emu->round) {
                    case ROUND_Nearest:
                        EW->sword[0] = floor(ST0.d+0.5);
                        break;
                    case ROUND_Down:
                        EW->sword[0] = floor(ST0.d);
                        break;
                    case ROUND_Up:
                        EW->sword[0] = ceil(ST0.d);
                        break;
                    case ROUND_Chop:
                        EW->sword[0] = ST0.d;
                        break;
                }
            }
            #endif
            fpu_do_pop(emu);
            break;
        case 4: /* FBLD ST0, tbytes */
            GET_ED;
            fpu_do_push(emu);
            fpu_fbld(emu, (uint8_t*)ED);
            break;
        case 5: /* FILD ST0, Gq */
            GET_ED;
            tmp64s = *(int64_t*)ED;
            fpu_do_push(emu);
            #ifdef USE_FLOAT
            ST0.f = tmp64s;
            #else
            ST0.d = tmp64s;
            #endif
            STll(0).ll = tmp64s;
            STll(0).ref = ST0.ll;
            break;
        case 6: /* FBSTP tbytes, ST0 */
            GET_ED;
            fpu_fbst(emu, (uint8_t*)ED);
            fpu_do_pop(emu);
            break;
        case 7: /* FISTP i64 */
            GET_ED;
            if(STll(0).ref==ST(0).ll) {
                *(int64_t*)ED = STll(0).ll;
            } else {
                #ifdef USE_FLOAT
                if(isgreater(ST0.f, (float)(int64_t)0x7fffffffffffffffLL) || isless(ST0.f, (float)(int64_t)0x8000000000000000LL))
                    *(int64_t*)ED = 0x8000000000000000LL;
                else
                    *(int64_t*)ED = (int64_t)ST0.f;
                #else
                if(isgreater(ST0.d, (double)(int64_t)0x7fffffffffffffffLL) || isless(ST0.d, (double)(int64_t)0x8000000000000000LL))
                    *(int64_t*)ED = 0x8000000000000000LL;
                else
                    *(int64_t*)ED = (int64_t)ST0.d;
                #endif
            }
            fpu_do_pop(emu);
            break;
        default:
            goto _default;
        }
    }
