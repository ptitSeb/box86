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
        fpu_fcomi(emu, ST(nextop&7).d);   // bad, should handle QNaN and IA interrupt
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
        fpu_fcomi(emu, ST(nextop&7).d);
        fpu_do_pop(emu);
        break;
    default:
        switch((nextop>>3)&7) {
        case 0: /* FILD ST0, Gw */
            GET_EW;
            tmp16s = EW->sword[0];
            fpu_do_push(emu);
            ST0.d = tmp16s;
            break;
        case 1: /* FISTTP Ew, ST0 */
            GET_EW;
            tmp16s = ST0.d;
            EW->sword[0] = tmp16s;
            fpu_do_pop(emu);
            break;
        case 2: /* FIST Ew, ST0 */
            GET_EW;
            tmp32s = ST0.d; // Converting directly to short don't work correctly => it doesn't "saturate"
            if((tmp32s<-32768) || (tmp32s>32767))
                tmp16s=0x8000;
            else
                tmp16s = tmp32s;
            EW->sword[0] = tmp16s;
            break;
        case 3: /* FISTP Ew, ST0 */
            GET_EW;
            tmp32s = ST0.d; // Converting directly to short don't work correctly => it doesn't "saturate"
            if(tmp32s<-32768)
                tmp16s=-32768;
            else if(tmp32s>32767)
                tmp16s=32767;
            else
                tmp16s = tmp32s;
            EW->sword[0] = tmp16s;
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
            ST0.d = tmp64s;
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
                if(isgreater(ST0.d, (double)(int64_t)0x7fffffffffffffffLL) || isless(ST0.d, (double)(int64_t)0x8000000000000000LL))
                    *(int64_t*)ED = 0x8000000000000000LL;
                else
                    *(int64_t*)ED = (int64_t)ST0.d;
            }
            fpu_do_pop(emu);
            break;
        default:
            goto _default;
        }
    }
