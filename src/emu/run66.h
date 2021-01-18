    opcode = F8;
    goto *opcodes66[opcode];

    #define GO(B, OP)                       \
    _66_##B##_0: \
        nextop = F8;               \
        GET_EB;             \
        EB->byte[0] = OP##8(emu, EB->byte[0], GB);  \
        NEXT;                              \
    _66_##B##_1:                               \
        nextop = F8;                \
        GET_EW;                   \
        EW->word[0] = OP##16(emu, EW->word[0], GW.word[0]); \
        NEXT;                              \
    _66_##B##_2: \
        nextop = F8;               \
        GET_EB;                   \
        GB = OP##8(emu, GB, EB->byte[0]); \
        NEXT;                              \
    _66_##B##_3:                               \
        nextop = F8;                \
        GET_EW;                   \
        GW.word[0] = OP##16(emu, GW.word[0], EW->word[0]); \
        NEXT;                              \
    _66_##B##_4: \
        R_AL = OP##8(emu, R_AL, F8); \
        NEXT;                              \
    _66_##B##_5:                               \
        R_AX = OP##16(emu, R_AX, F16); \
        NEXT;

    GO(0x00, add)                   /* ADD 0x01 ~> 0x05 */
    GO(0x08, or)                    /*  OR 0x09 ~> 0x0D */
    GO(0x10, adc)                   /* ADC 0x11 ~> 0x15 */
    GO(0x18, sbb)                   /* SBB 0x19 ~> 0x1D */
    GO(0x20, and)                   /* AND 0x21 ~> 0x25 */
    GO(0x28, sub)                   /* SUB 0x29 ~> 0x2D */
    GO(0x30, xor)                   /* XOR 0x31 ~> 0x35 */
    //GO(0x38, cmp)                   /* CMP 0x39 ~> 0x3D */
    #undef GO
    _66_0x06:                      /* PUSH ES */
        Push16(emu, emu->segs[_ES]);
        NEXT;
    _66_0x07:                      /* POP ES */
        emu->segs[_ES] = Pop16(emu);    // no check, no use....
        emu->segs_serial[_ES] = 0;
        NEXT;

    _66_0x0F:                      /* 66 0f prefix */
        emu->old_ip = R_EIP;
        R_EIP = ip-2;   // don't count 66 0F yet
        Run660F(emu); // implemented in Run660f.c
        ip = R_EIP;
        if(emu->quit) goto fini;
        STEP
        NEXT;

    _66_0x1F:                      /* POP DS */
        emu->segs[_DS] = Pop16(emu);    // no check, no use....
        emu->segs_serial[_DS] = 0;
        NEXT;
        
    _66_0x26:                      /* ES: */
        // ignored
        opcode = F8;
        goto *opcodes66[opcode];
    _66_0x2E:                      /* CS: */
        // ignored
        opcode = F8;
        goto *opcodes66[opcode];
    _66_0x36:                      /* SS: */
        // ignored
        opcode = F8;
        goto *opcodes66[opcode];

    _66_0x39:
        nextop = F8;
        GET_EW;
        cmp16(emu, EW->word[0], GW.word[0]);
        NEXT;
    _66_0x3B:
        nextop = F8;
        GET_EW;
        cmp16(emu, GW.word[0], EW->word[0]);
        NEXT;
    _66_0x3D:
        cmp16(emu, R_AX, F16);
        NEXT;
    
    _66_0x40:
    _66_0x41:
    _66_0x42:
    _66_0x43:
    _66_0x44:
    _66_0x45:
    _66_0x46:
    _66_0x47:                              /* INC Reg */
        tmp8u = opcode&7;
        emu->regs[tmp8u].word[0] = inc16(emu, emu->regs[tmp8u].word[0]);
        NEXT;
    _66_0x48:
    _66_0x49:
    _66_0x4A:
    _66_0x4B:
    _66_0x4C:
    _66_0x4D:
    _66_0x4E:
    _66_0x4F:                              /* DEC Reg */
        tmp8u = opcode&7;
        emu->regs[tmp8u].word[0] = dec16(emu, emu->regs[tmp8u].word[0]);
        NEXT;
    _66_0x50:
    _66_0x51:
    _66_0x52:
    _66_0x53:
    _66_0x54:
    _66_0x55:
    _66_0x56:
    _66_0x57:                              /* PUSH Reg */
        tmp8u = opcode&7;
        Push16(emu, emu->regs[tmp8u].word[0]);
        NEXT;
    _66_0x58:
    _66_0x59:
    _66_0x5A:
    _66_0x5B:
    _66_0x5C:                      
    _66_0x5D:
    _66_0x5E:
    _66_0x5F:                              /* POP Reg */
        tmp8u = opcode&7;
        emu->regs[tmp8u].word[0] = Pop16(emu);
        NEXT;
    _66_0x60:                              /* PUSHA */
        tmp16u = R_SP;
        Push16(emu, R_AX);
        Push16(emu, R_CX);
        Push16(emu, R_DX);
        Push16(emu, R_BX);
        Push16(emu, tmp16u);
        Push16(emu, R_BP);
        Push16(emu, R_SI);
        Push16(emu, R_DI);
        NEXT;
    _66_0x61:                              /* POPA */
        R_DI = Pop16(emu);
        R_SI = Pop16(emu);
        R_BP = Pop16(emu);
        R_ESP+=2;   // POP ESP
        R_BX = Pop16(emu);
        R_DX = Pop16(emu);
        R_CX = Pop16(emu);
        R_AX = Pop16(emu);
        NEXT;

    _66_0x64:
        R_EIP = ip-2;
        RunFS66(emu, GetFSBaseEmu(emu));
        ip = R_EIP;
        STEP;
        NEXT;

    _66_0x66:
        goto _0x66; // 0x66 0x66 => can remove one 0x66

    _66_0x68:                       /* PUSH u16 */
        tmp16u = F16;
        Push16(emu, tmp16u);
        NEXT;
    _66_0x69:                      /* IMUL Gw,Ew,Iw */
        nextop = F8;
        GET_EW;
        tmp16u = F16;
        GW.word[0] = imul16(emu, EW->word[0], tmp16u);
        NEXT;
    _66_0x6A:                      /* PUSH Ib (as signed word) */
        tmp16s = F8S;
        Push16(emu, (uint16_t)tmp16s);
        NEXT;
    _66_0x6B:                      /* IMUL Gw,Ew,Ib */
        nextop = F8;
        GET_EW;
        tmp16s = F8S;
        GW.word[0] = imul16(emu, EW->word[0], (uint16_t)tmp16s);
        NEXT;

    _66_0x81:                              /* GRP3 Ew,Iw */
    _66_0x83:                              /* GRP3 Ew,Ib */
        nextop = F8;
        GET_EW;
        if(opcode==0x81) 
            tmp16u = F16;
        else {
            tmp16s = F8S;
            tmp16u = (uint16_t)tmp16s;
        }
        switch((nextop>>3)&7) {
            case 0: EW->word[0] = add16(emu, EW->word[0], tmp16u); break;
            case 1: EW->word[0] =  or16(emu, EW->word[0], tmp16u); break;
            case 2: EW->word[0] = adc16(emu, EW->word[0], tmp16u); break;
            case 3: EW->word[0] = sbb16(emu, EW->word[0], tmp16u); break;
            case 4: EW->word[0] = and16(emu, EW->word[0], tmp16u); break;
            case 5: EW->word[0] = sub16(emu, EW->word[0], tmp16u); break;
            case 6: EW->word[0] = xor16(emu, EW->word[0], tmp16u); break;
            case 7:               cmp16(emu, EW->word[0], tmp16u); break;
        }
        NEXT;

    _66_0x85:                              /* TEST Ew,Gw */
        nextop = F8;
        GET_EW;
        test16(emu, EW->word[0], GW.word[0]);
        NEXT;

    _66_0x87:                              /* XCHG Ew,Gw */
        nextop = F8;
        GET_EW;
        tmp16u = GW.word[0];
        GW.word[0] = EW->word[0];
        EW->word[0] = tmp16u;
        NEXT;

    _66_0x89:                              /* MOV Ew,Gw */
        nextop = F8;
        GET_EW;
        EW->word[0] = GW.word[0];
        NEXT;

    _66_0x8B:                              /* MOV Gw,Ew */
        nextop = F8;
        GET_EW;
        GW.word[0] = EW->word[0];
        NEXT;
    _66_0x8C:                              /* MOV Ew,Seg */
        nextop = F8;
        GET_EW;
        EW->word[0] = emu->segs[(nextop&0x38)>>3];
        NEXT;
    
    _66_0x8E:                               /* MOV Seg,Ew */
        nextop = F8;
        GET_EW;
        emu->segs[((nextop&0x38)>>3)] = EW->word[0];
        emu->segs_serial[((nextop&0x38)>>3)] = 0;
        NEXT;
    _66_0x8F:                              /* POP Ew */
        nextop = F8;
        GET_EW;
        EW->word[0] = Pop16(emu);
        NEXT;
    _66_0x90:                              /* NOP */
        NEXT;

    _66_0x91:
    _66_0x92:
    _66_0x93:
    _66_0x94:
    _66_0x95:
    _66_0x96:
    _66_0x97:                      /* XCHG reg,EAX */
        tmp16u = R_AX;
        R_AX = emu->regs[opcode&7].word[0];
        emu->regs[opcode&7].word[0] = tmp16u;
        NEXT;

    _66_0x98:                               /* CBW */
        emu->regs[_AX].sword[0] = emu->regs[_AX].sbyte[0];
        NEXT;
    _66_0x99:                              /* CWD */
        R_DX=((R_AX & 0x8000)?0xFFFF:0x0000);
        NEXT;

    _66_0x9C:                              /* PUSHFW */
        CHECK_FLAGS(emu);
        Push16(emu, (uint16_t)emu->eflags.x32);
        NEXT;
    _66_0x9D:                              /* POPFW */
        CHECK_FLAGS(emu);
        emu->eflags.x32 &=0xffff0000;
        emu->eflags.x32 |= (Pop16(emu) & 0x3F7FD7) | 0x2;
        NEXT;

    _66_0xA1:                              /* MOV AX,Ow */
        R_AX = *(uint16_t*)F32;
        NEXT;
    _66_0xA3:                              /* MOV Ow,AX */
        *(uint16_t*)F32 = R_AX;
        NEXT;
    _66_0xA5:                              /* MOVSW */
        tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
        *(uint16_t*)R_EDI = *(uint16_t*)R_ESI;
        R_EDI += tmp8s;
        R_ESI += tmp8s;
        NEXT;

    _66_0xA7:                              /* CMPSW */
        tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
        tmp16u  = *(uint16_t*)R_EDI;
        tmp16u2 = *(uint16_t*)R_ESI;
        R_EDI += tmp8s;
        R_ESI += tmp8s;
        cmp16(emu, tmp16u2, tmp16u);
        NEXT;

    _66_0xA9:                             /* TEST AX,Iw */
        test16(emu, R_AX, F16);
        NEXT;

    _66_0xAB:                              /* STOSW */
        tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
        *(uint16_t*)R_EDI = R_AX;
        R_EDI += tmp8s;
        NEXT;
    _66_0xAD:                              /* LODSW */
        tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
        R_AX = *(uint16_t*)R_ESI;
        R_ESI += tmp8s;
        NEXT;
    _66_0xAF:                              /* SCASW */
        tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
        cmp16(emu, R_AX, *(uint16_t*)R_EDI);
        R_EDI += tmp8s;
        NEXT;

    _66_0xB8:                              /* MOV AX,Iw */
    _66_0xB9:                              /* MOV CX,Iw */
    _66_0xBA:                              /* MOV DX,Iw */
    _66_0xBB:                              /* MOV BX,Iw */
    _66_0xBC:                              /*    ...     */
    _66_0xBD:
    _66_0xBE:
    _66_0xBF:
        emu->regs[opcode&7].word[0] = F16;
        NEXT;

    _66_0xC1:                              /* GRP2 Ew,Ib */
        nextop = F8;
        GET_EW;
        tmp8u = F8 /*& 0x1f*/;
        switch((nextop>>3)&7) {
            case 0: EW->word[0] = rol16(emu, EW->word[0], tmp8u); break;
            case 1: EW->word[0] = ror16(emu, EW->word[0], tmp8u); break;
            case 2: EW->word[0] = rcl16(emu, EW->word[0], tmp8u); break;
            case 3: EW->word[0] = rcr16(emu, EW->word[0], tmp8u); break;
            case 4:
            case 6: EW->word[0] = shl16(emu, EW->word[0], tmp8u); break;
            case 5: EW->word[0] = shr16(emu, EW->word[0], tmp8u); break;
            case 7: EW->word[0] = sar16(emu, EW->word[0], tmp8u); break;
        }
        NEXT;

    _66_0xC7:                              /* MOV Ew,Iw */
        nextop = F8;
        GET_EW;
        EW->word[0] = F16;
        NEXT;

    _66_0xCB:                               /* FAR RET */
        ip = Pop(emu);
        emu->segs[_CS] = Pop(emu);    // no check, no use....
        emu->segs_serial[_CS] = 0;
        // need to check status of CS register!
        STEP
        NEXT;
    _66_0xCC:                              /* INT3 */
        emu->old_ip = R_EIP;
        R_EIP = ip-1;
        if(my_context->signals[SIGTRAP])
            raise(SIGTRAP);
        ip = R_EIP;
        if(emu->quit) goto fini;
        STEP
        NEXT;
    _66_0xD1:                              /* GRP2 Ew,1  */
    _66_0xD3:                              /* GRP2 Ew,CL */
        nextop = F8;
        GET_EW;
        tmp8u=(opcode==0xD3)?R_CL:1;
        switch((nextop>>3)&7) {
            case 0: EW->word[0] = rol16(emu, EW->word[0], tmp8u); break;
            case 1: EW->word[0] = ror16(emu, EW->word[0], tmp8u); break;
            case 2: EW->word[0] = rcl16(emu, EW->word[0], tmp8u); break;
            case 3: EW->word[0] = rcr16(emu, EW->word[0], tmp8u); break;
            case 4: 
            case 6: EW->word[0] = shl16(emu, EW->word[0], tmp8u); break;
            case 5: EW->word[0] = shr16(emu, EW->word[0], tmp8u); break;
            case 7: EW->word[0] = sar16(emu, EW->word[0], tmp8u); break;
        }
        NEXT;
    
    _66_0xD9:
        emu->old_ip = R_EIP;
        R_EIP = ip;
        Run66D9(emu);
        ip = R_EIP;
        if(emu->quit) goto fini;
        STEP
        NEXT;

    _66_0xDD:
        emu->old_ip = R_EIP;
        R_EIP = ip;
        Run66DD(emu);
        ip = R_EIP;
        if(emu->quit) goto fini;
        STEP
        NEXT;

    _66_0xF0:                      /* LOCK prefix */
        emu->old_ip = R_EIP;
        R_EIP = ip - 2;
        RunLock66(emu);
        ip = R_EIP;
        if(emu->quit) goto fini;
        STEP
        NEXT;

    _66_0xF2:                      /* REPNZ prefix */
    _66_0xF3:                      /* REPZ prefix */
        nextop = F8;
        tmp8s = ACCESS_FLAG(F_DF)?-2:+2;
        tmp32u = R_ECX;
        switch(nextop) {
            case 0xA4:              /* REP MOVSB */
                while(tmp32u) {
                    --tmp32u;
                    *(uint8_t*)R_EDI = *(uint8_t*)R_ESI;
                    R_EDI += tmp8s;
                    R_ESI += tmp8s;
                }
                break;
            case 0xA5:              /* REP MOVSW */
                while(tmp32u) {
                    --tmp32u;
                    *(uint16_t*)R_EDI = *(uint16_t*)R_ESI;
                    R_EDI += tmp8s;
                    R_ESI += tmp8s;
                }
                break;
            case 0xA7:              /* REP(N)Z CMPSW */
                tmp16u = 0;
                tmp16u2 = 0;
                while(tmp32u) {
                    --tmp32u;
                    tmp16u  = *(uint16_t*)R_EDI;
                    tmp16u2 = *(uint16_t*)R_ESI;
                    R_EDI += tmp8s;
                    R_ESI += tmp8s;
                    if((tmp16u==tmp16u2)==(opcode==0xF2))
                        break;
                }
                if(R_ECX) cmp16(emu, tmp16u2, tmp16u);
                break;
            case 0xAB:              /* REP STOSW */
                while(tmp32u) {
                    --tmp32u;
                    *(uint16_t*)R_EDI = R_AX;
                    R_EDI += tmp8s;
                }
                break;
            case 0xAD:              /* REP LODSW */
                while(tmp32u) {
                    --tmp32u;
                    R_AX = *(uint16_t*)R_ESI;
                    R_ESI += tmp8s;
                }
                break;
            case 0xAF:              /* REP(N)Z SCASW */
                tmp16u = 0;
                while(tmp32u) {
                    --tmp32u;
                    tmp16u = *(uint16_t*)R_EDI;
                    R_EDI += tmp8s;
                    if((R_AX==tmp16u)==(opcode==0xF2))
                        break;
                }
                if(R_ECX) cmp16(emu, R_AX, tmp16u);
                break;
            default:
                goto _default;
        }
        R_ECX = tmp32u;
        NEXT;

    _66_0xF7:                      /* GRP3 Ew(,Iw) */
        nextop = F8;
        GET_EW;
        switch((nextop>>3)&7) {
            case 0: 
            case 1:                 /* TEST Ew,Iw */
                test16(emu, EW->word[0], F16);
                break;
            case 2:                 /* NOT Ew */
                EW->word[0] = not16(emu, EW->word[0]);
                break;
            case 3:                 /* NEG Ew */
                EW->word[0] = neg16(emu, EW->word[0]);
                break;
            case 4:                 /* MUL AX,Ew */
                mul16(emu, EW->word[0]);
                break;
            case 5:                 /* IMUL AX,Ew */
                imul16_eax(emu, EW->word[0]);
                break;
            case 6:                 /* DIV Ew */
                div16(emu, EW->word[0]);
                break;
            case 7:                 /* IDIV Ew */
                idiv16(emu, EW->word[0]);
                break;
        }
        NEXT;
    _66_0xF8:                       /* CLC */
        CHECK_FLAGS(emu);
        CLEAR_FLAG(F_CF);
        NEXT;
    _66_0xF9:                       /* STC */
        CHECK_FLAGS(emu);
        SET_FLAG(F_CF);
        NEXT;

    _66_0xFF:                      /* GRP 5 Ew */
        nextop = F8;
        GET_EW;
        switch((nextop>>3)&7) {
            case 0:                 /* INC Ed */
                EW->word[0] = inc16(emu, EW->word[0]);
                break;
            case 1:                 /* DEC Ed */
                EW->word[0] = dec16(emu, EW->word[0]);
                break;
            case 6:
                Push16(emu, EW->word[0]);
                break;
            default:
                emu->old_ip = R_EIP;
                R_EIP = ip;
                printf_log(LOG_NONE, "Illegal Opcode 66 %02X %02X\n", opcode, nextop);
                emu->quit=1;
                emu->error |= ERR_ILLEGAL;
                goto fini;
        }
        NEXT;
