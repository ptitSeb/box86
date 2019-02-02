    nextop = Fetch8(emu);
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
            GetEw(emu, &op2, nextop);
            tmp16s = (int16_t)op2->word[0];
            fpu_do_push(emu);
            ST0.d = tmp16s;
            break;
        case 1: /* FISTTP Ew, ST0 */
            GetEw(emu, &op2, nextop);
            tmp16s = ST0.d;
            op2->word[0] = (uint16_t)tmp16s;
            fpu_do_pop(emu);
            break;
        case 2: /* FIST Ew, ST0 */
            GetEw(emu, &op2, nextop);
            tmp32s = ST0.d; // Converting directly to short don't work correctly => it doesn't "saturate"
            if((tmp32s<-32768) || (tmp32s>32767))
                tmp16s=0x8000;
            else
                tmp16s = tmp32s;
            op2->word[0] = (uint16_t)tmp16s;
            break;
        case 3: /* FISTP Ew, ST0 */
            GetEw(emu, &op2, nextop);
            tmp32s = ST0.d; // Converting directly to short don't work correctly => it doesn't "saturate"
            if(tmp32s<-32768)
                tmp16s=-32768;
            else if(tmp32s>32767)
                tmp16s=32767;
            else
                tmp16s = tmp32s;
            op2->word[0] = (uint16_t)tmp16s;
            fpu_do_pop(emu);
            break;
        case 4: /* FBLD ST0, tbytes */
            GetEd(emu, &op2, nextop);
            fpu_do_push(emu);
            fpu_fbld(emu, (uint8_t*)op2);
            break;
        case 5: /* FILD ST0, Gq */
            GetEd(emu, &op2, nextop);
            tmp64s = *(int64_t*)&op2->dword[0];
            fpu_do_push(emu);
            ST0.d = tmp64s;
            STll(0).ll = tmp64s;
            STll(0).ref = ST0.ll;
            break;
        case 6: /* FBSTP tbytes, ST0 */
            GetEd(emu, &op2, nextop);
            fpu_fbst(emu, (uint8_t*)op2);
            fpu_do_pop(emu);
            break;
        case 7: /* FISTP i64 */
            GetEd(emu, &op1, nextop);
            if(STll(0).ref==ST(0).ll) {
                tmp64s = STll(0).ll;
            } else {
                if(isgreater(ST0.d, (double)(int64_t)0x7fffffffffffffffLL) || isless(ST0.d, (double)(int64_t)0x8000000000000000LL))
                    tmp64s = 0x8000000000000000LL;
                else
                    tmp64s = (int64_t)ST0.d;
            }
            *(int64_t*)&op1->dword[0] = tmp64s;
            fpu_do_pop(emu);
            break;
        default:
            UnimpOpcode(emu);
            goto fini;
        }
    }
