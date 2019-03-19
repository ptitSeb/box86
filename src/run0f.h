    opcode = F8;
    goto *opcodes0f[opcode];

        _0f_0x10:                      /* MOVUPS Gx,Ex */
            nextop = F8;
            GET_EX;
            memcpy(&GX, EX, 16); // unaligned, so carreful
            NEXT;
        _0f_0x11:                      /* MOVUPS Ex,Gx */
            nextop = F8;
            GET_EX;
            memcpy(EX, &GX, 16); // unaligned, so carreful
            NEXT;
        _0f_0x12:                      
            nextop = F8;
            GET_EX;
            if((nextop&0xC0)==0xC0)    /* MOVHLPS Gx,Ex */
                GX.f.u64[0] = EX->f.u64[1];
            else
                GX.f.u64[0] = EX->f.u64[0];    /* MOVLPS Gx,Ex */
            NEXT;
        _0f_0x13:                      /* MOVLPS Ex,Gx */
            nextop = F8;
            GET_EX;
            EX->f.u64[0] = GX.f.u64[0];
            NEXT;
        _0f_0x14:                      /* UNPCKLPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.f = simde_mm_unpacklo_ps(GX.f, EX->f);
            NEXT;
        _0f_0x15:                      /* UNPCKHPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.f = simde_mm_unpackhi_ps(GX.f, EX->f);
            NEXT;
        _0f_0x16:                      /* MOVHPS Gx,Ex */
            nextop = F8;               /* MOVLHPS Gx,Ex (Ex==reg) */
            GET_EX;
            GX.f.u64[1] = EX->f.u64[0];
            NEXT;
        _0f_0x17:                      /* MOVHPS Ex,Gx */
            nextop = F8;
            GET_EX;
            EX->f.u64[0] = GX.f.u64[1];
            NEXT;

        _0f_0x1F:                      /* NOP (multi-byte) */
            nextop = F8;
            GET_ED;
            NEXT;

        _0f_0x28:                      /* MOVAPS Gx,Ex */
            nextop = F8;
            GET_EX;
            simde_mm_store_ps(&GX.f.f32[0], EX->f);
            NEXT;
        _0f_0x29:                      /* MOVAPS Ex,Gx */
            nextop = F8;
            GET_EX;
            simde_mm_store_ps(&EX->f.f32[0], GX.f);
            NEXT;
        _0f_0x2A:                      /* CVTPI2PS Gx, Em */
            nextop = F8;
            GET_EM;
            GX.f = simde_mm_cvtpi32_ps(GX.f, *EM);
            NEXT;

        _0f_0x2C:                      /* CVTTPS2PI Gm, Ex */
            nextop = F8;
            GET_EX;
            GM = simde_mm_cvttps_pi32(EX->f);
            NEXT;
        _0f_0x2D:                      /* CVTPS2PI Gm, Ex */
            // rounding should be done; and indefinite integer should also be assigned if overflow or NaN/Inf
            nextop = F8;
            GET_EX;
            GM = simde_mm_cvtps_pi32(EX->f);
            NEXT;
        _0f_0x2E:                      /* UCOMISS Gx, Ex */
            // same for now
        _0f_0x2F:                      /* COMISS Gx, Ex */
            RESET_FLAGS(emu);
            nextop = F8;
            GET_EX;
            if(isnan(GX.f.f32[0]) || isnan(EX->f.f32[0])) {
                SET_FLAG(F_ZF); SET_FLAG(F_PF); SET_FLAG(F_CF);
            } else if(isgreater(GX.f.f32[0], EX->f.f32[0])) {
                CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
            } else if(isless(GX.f.f32[0], EX->f.f32[0])) {
                CLEAR_FLAG(F_ZF); CLEAR_FLAG(F_PF); SET_FLAG(F_CF);
            } else {
                SET_FLAG(F_ZF); CLEAR_FLAG(F_PF); CLEAR_FLAG(F_CF);
            }
            CLEAR_FLAG(F_OF); CLEAR_FLAG(F_AF); CLEAR_FLAG(F_SF);
            NEXT;

        _0f_0x31:                   /* RDTSC */
            tmp64u = ReadTSC(emu);
            R_EDX = tmp64u>>32;
            R_EAX = tmp64u&0xFFFFFFFF;
            NEXT;
        
        #define GOCOND(BASE, PREFIX, CONDITIONAL) \
        _0f_##BASE##_0:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_OF))               \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_1:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_OF))              \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_2:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_CF))               \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_3:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_CF))              \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_4:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_ZF))               \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_5:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_ZF))              \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_6:                          \
            PREFIX                              \
            if((ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF)))  \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_7:                          \
            PREFIX                              \
            if(!(ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF))) \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_8:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF))               \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_9:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_SF))              \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_A:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_PF))               \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_B:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_PF))              \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_C:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))  \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_D:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF)) \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_E:                          \
            PREFIX                              \
            if(ACCESS_FLAG(F_ZF) || (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))) \
                CONDITIONAL                     \
            NEXT;                              \
        _0f_##BASE##_F:                          \
            PREFIX                              \
            if(!ACCESS_FLAG(F_ZF) && (ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF))) \
                CONDITIONAL                     \
            NEXT;

        GOCOND(0x40
            , nextop = F8;
            GET_ED;
            CHECK_FLAGS(emu);
            , GD.dword[0] = ED->dword[0];
        )                               /* 0x40 -> 0x4F CMOVxx Gd,Ed */ // conditional move, no sign
        GOCOND(0x80
            , tmp32s = F32S; CHECK_FLAGS(emu);
            , ip += tmp32s;
        )                               /* 0x80 -> 0x8F Jxx */
        GOCOND(0x90
            , nextop = F8; CHECK_FLAGS(emu);
            GET_EB;
            , EB->byte[0]=1; else EB->byte[0]=0;
        )                               /* 0x90 -> 0x9F SETxx Eb */

        #undef GOCOND

        _0f_0x50:                      /* MOVMSKPS Gd, Ex */
            nextop = F8;
            GET_EX;
            GD.dword[0] = simde_mm_movemask_ps(EX->f);
            NEXT;
        _0f_0x51:                      /* SQRTPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.f = simde_mm_sqrt_ps(EX->f);
            NEXT;

        _0f_0x54:                      /* ANDPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.f = simde_mm_and_ps(GX.f, EX->f);
            NEXT;
        _0f_0x55:                      /* ANDNPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.f = simde_mm_andnot_ps(GX.f, EX->f);
            NEXT;
        _0f_0x56:                      /* ORPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.f = simde_mm_or_ps(GX.f, EX->f);
            NEXT;
        _0f_0x57:                      /* XORPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.f = simde_mm_xor_ps(GX.f, EX->f);
            NEXT;
        _0f_0x58:                      /* ADDPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.f = simde_mm_add_ps(GX.f, EX->f);
            NEXT;
        _0f_0x59:                      /* MULPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.f = simde_mm_mul_ps(GX.f, EX->f);
            NEXT;
        _0f_0x5A:                      /* CVTPS2PD Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.d = simde_mm_cvtps_pd(EX->f);
            NEXT;
        _0f_0x5B:                      /* CVTDQ2PS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.f = simde_mm_cvtepi32_ps(EX->i);
            NEXT;
        _0f_0x5C:                      /* SUBPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.f = simde_mm_sub_ps(GX.f, EX->f);
            NEXT;
        _0f_0x5D:                      /* MINPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.f = simde_mm_min_ps(GX.f, EX->f);
            NEXT;
        _0f_0x5E:                      /* DIVPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.f = simde_mm_div_ps(GX.f, EX->f);
            NEXT;
        _0f_0x5F:                      /* MAXPS Gx, Ex */
            nextop = F8;
            GET_EX;
            GX.f = simde_mm_max_ps(GX.f, EX->f);
            NEXT;
        _0f_0x60:                      /* PUNPCKLBW Gm, Em */
            nextop = F8;
            GET_EM;
            GM = simde_mm_unpacklo_pi8(GM, *EM);
            NEXT;
        _0f_0x61:                      /* PUNPCKLWD Gm, Em */
            nextop = F8;
            GET_EM;
            GM = simde_mm_unpacklo_pi16(GM, *EM);
            NEXT;
        _0f_0x62:                      /* PUNPCKLDQ Gm, Em */
            nextop = F8;
            GET_EM;
            GM = simde_mm_unpacklo_pi32(GM, *EM);
            NEXT;

        _0f_0x6F:                      /* MOVQ Gm, Em */
            nextop = F8;
            GET_EM;
            GM = *EM;
            NEXT;

        _0f_0x77:                      /* EMMS */
            // empty MMX, FPU now usable
            NEXT;

        _0f_0x7F:                      /* MOVQ Em, Gm */
            nextop = F8;
            GET_EM;
            *EM = GM;
            NEXT;

        _0f_0xA2:                      /* CPUID */
            tmp32u = R_EAX;
            switch(tmp32u) {
                case 0x0:
                    // emulate a P4
                    R_EAX = 0x80000004;
                    // return GenuineIntel
                    R_EBX = 0x756E6547;
                    R_EDX = 0x49656E69;
                    R_ECX = 0x6C65746E;
                    break;
                case 0x1:
                    R_EAX = 0x00000101; // familly and all
                    R_EBX = 0;          // Brand indexe, CLFlush, Max APIC ID, Local APIC ID
                    R_EDX =   1         // fpu 
                            | 1<<8      // cmpxchg8
                            | 1<<11     // sep (sysenter & sysexit)
                            | 1<<15     // cmov
                            | 1<<19     // clflush (seems to be with SSE2)
                            | 1<<23     // mmx
                            | 1<<24     // fxsr (fxsave, fxrestore)
                            | 1<<25     // SSE
                            | 1<<26     // SSE2
                            ;
                    R_ECX =   1<<12     // fma
                            | 1<<13     // cx16 (cmpxchg16)
                            ;           // also 1<<0 is SSE3 and 1<<9 is SSSE3
                    break;
                case 0x2:   // TLB and Cache info. Sending 1st gen P4 info...
                    R_EAX = 0x665B5001;
                    R_EBX = 0x00000000;
                    R_ECX = 0x00000000;
                    R_EDX = 0x007A7000;
                    break;
                
                case 0x4:   // Cache info
                    switch (R_ECX) {
                        case 0: // L1 data cache
                            R_EAX = (1 | (1<<5) | (1<<8));   //type
                            R_EBX = (63 | (7<<22)); // size
                            R_ECX = 63;
                            R_EDX = 1;
                            break;
                        case 1: // L1 inst cache
                            R_EAX = (2 | (1<<5) | (1<<8)); //type
                            R_EBX = (63 | (7<<22)); // size
                            R_ECX = 63;
                            R_EDX = 1;
                            break;
                        case 2: // L2 cache
                            R_EAX = (3 | (2<<5) | (1<<8)); //type
                            R_EBX = (63 | (15<<22));    // size
                            R_ECX = 4095;
                            R_EDX = 1;
                            break;

                        default:
                            R_EAX = 0x00000000;
                            R_EBX = 0x00000000;
                            R_ECX = 0x00000000;
                            R_EDX = 0x00000000;
                            break;
                    }
                    break;
                case 0x5:   //mwait info
                    R_EAX = 0;
                    R_EBX = 0;
                    R_ECX = 1 | 2;
                    R_EDX = 0;
                    break;
                case 0x6:   // thermal
                case 0x9:   // direct cache access
                case 0xA:   // Architecture performance monitor
                    R_EAX = 0;
                    R_EBX = 0;
                    R_ECX = 0;
                    R_EDX = 0;
                    break;

                case 0x80000000:        // max extended
                    break;              // no extended, so return same value as beeing the max value!
                default:
                    printf_log(LOG_INFO, "Warning, CPUID command %X unsupported (ECX=%08x)\n", tmp32u, R_ECX);
                    R_EAX = 0;
            }
            NEXT;
        _0f_0xA3:                      /* BT Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp8u = GD.byte[0];
            if((nextop&0xC0)!=0xC0)
            {
                ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
            }
            tmp8u&=31;
            if(ED->dword[0] & (1<<tmp8u))
                SET_FLAG(F_CF);
            else
                CLEAR_FLAG(F_CF);
            NEXT;
        _0f_0xA4:                      /* SHLD Ed,Gd,Ib */
        _0f_0xA5:                      /* SHLD Ed,Gd,CL */
            nextop = F8;
            GET_ED;
            tmp8u = (opcode==0xA4)?(F8):R_CL;
            ED->dword[0] = shld32(emu, ED->dword[0], GD.dword[0], tmp8u);
            NEXT;

        _0f_0xAB:                      /* BTS Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp8u = GD.byte[0];
            if((nextop&0xC0)!=0xC0)
            {
                ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
            }
            tmp8u&=31;
            if(ED->dword[0] & (1<<tmp8u))
                SET_FLAG(F_CF);
            else {
                ED->dword[0] |= (1<<tmp8u);
                CLEAR_FLAG(F_CF);
            }
            NEXT;
        _0f_0xAC:                      /* SHRD Ed,Gd,Ib */
        _0f_0xAD:                      /* SHRD Ed,Gd,CL */
            nextop = F8;
            GET_ED;
            tmp8u = (opcode==0xAC)?(F8):R_CL;
            ED->dword[0] = shrd32(emu, ED->dword[0], GD.dword[0], tmp8u);
            NEXT;

        _0f_0xAE:                      /* Grp Ed (SSE) */
            nextop = F8;
            if((nextop&0xF8)==0xE8) {
                NEXT;                   /* LFENCE */
            }
            if((nextop&0xF8)==0xF0) {
                NEXT;                   /* MFENCE */
            }
            if((nextop&0xF8)==0xF8) {
                NEXT;                   /* SFENCE */
            }
            GET_ED;
            switch((nextop>>3)&7) {
                case 2:                 /* LDMXCSR Md */
                    emu->mxcsr = ED->dword[0];
                    break;
                case 3:                 /* STMXCSR Md */
                    ED->dword[0] = emu->mxcsr;
                    break;
                default:
                    goto _default;
            }
            NEXT;
        _0f_0xAF:                      /* IMUL Gd,Ed */
            nextop = F8;
            GET_ED;
            GD.dword[0] = imul32(emu, GD.dword[0], ED->dword[0]);
            NEXT;

        _0f_0xB0:                      /* CMPXCHG Eb,Gb */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_EB;
            cmp8(emu, R_AL, EB->byte[0]);
            if(ACCESS_FLAG(F_ZF)) {
                EB->byte[0] = GB;
            } else {
                R_AL = EB->byte[0];
            }
            NEXT;
        _0f_0xB1:                      /* CMPXCHG Ed,Gd */
            nextop = F8;
            GET_ED;
            cmp32(emu, R_EAX, ED->dword[0]);
            if(ACCESS_FLAG(F_ZF)) {
                ED->dword[0] = GD.dword[0];
            } else {
                R_EAX = ED->dword[0];
            }
            NEXT;

        _0f_0xB3:                      /* BTR Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp8u = GD.byte[0];
            if((nextop&0xC0)!=0xC0)
            {
                ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
            }
            tmp8u&=31;
            if(ED->dword[0] & (1<<tmp8u)) {
                SET_FLAG(F_CF);
                ED->dword[0] ^= (1<<tmp8u);
            } else
                CLEAR_FLAG(F_CF);
            NEXT;

        _0f_0xB6:                      /* MOVZX Gd,Eb */
            nextop = F8;
            GET_EB;
            GD.dword[0] = EB->byte[0];
            NEXT;
        _0f_0xB7:                      /* MOVZX Gd,Ew */
            nextop = F8;
            GET_EW;
            GD.dword[0] = EW->word[0];
            NEXT;

        _0f_0xBB:                      /* BTC Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp8u = GD.byte[0];
            if((nextop&0xC0)!=0xC0)
            {
                ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
            }
            tmp8u&=31;
            if(ED->dword[0] & (1<<tmp8u))
                SET_FLAG(F_CF);
            else
                CLEAR_FLAG(F_CF);
            ED->dword[0] ^= (1<<tmp8u);
            NEXT;

        _0f_0xBA:                      
            nextop = F8;
            switch((nextop>>3)&7) {
                case 4:                 /* BT Ed,Ib */
                    CHECK_FLAGS(emu);
                    GET_ED;
                    tmp8u = F8;
                    if((nextop&0xC0)!=0xC0)
                    {
                        ED=(reg32_t*)(((uint32_t*)(ED))+(tmp8u>>5));
                    }
                    tmp8u&=31;
                    if(ED->dword[0] & (1<<tmp8u))
                        SET_FLAG(F_CF);
                    else
                        CLEAR_FLAG(F_CF);
                    break;;
                default:
                    goto _default;
            }
            NEXT;
        _0f_0xBC:                      /* BSF Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp32u = ED->dword[0];
            if(tmp32u) {
                CLEAR_FLAG(F_ZF);
                tmp8u = 0;
                while(!(tmp32u&(1<<tmp8u))) ++tmp8u;
                GD.dword[0] = tmp8u;
            } else {
                SET_FLAG(F_ZF);
            }
            NEXT;
        _0f_0xBD:                      /* BSR Ed,Gd */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp32u = ED->dword[0];
            if(tmp32u) {
                CLEAR_FLAG(F_ZF);
                tmp8u = 31;
                while(!(tmp32u&(1<<tmp8u))) --tmp8u;
                GD.dword[0] = tmp8u;
            } else {
                SET_FLAG(F_ZF);
            }
            NEXT;
        _0f_0xBE:                      /* MOVSX Gd,Eb */
            nextop = F8;
            GET_EB;
            GD.sdword[0] = EB->sbyte[0];
            NEXT;
        _0f_0xBF:                      /* MOVSX Gd,Ew */
            nextop = F8;
            GET_EW;
            GD.sdword[0] = EW->sword[0];
            NEXT;

        _0f_0xC0:                      /* XADD Gb,Eb */
            nextop = F8;
            GET_EB;
            tmp8u = add8(emu, EB->byte[0], GB);
            GB = EB->byte[0];
            EB->byte[0] = tmp8u;
            NEXT;
        _0f_0xC1:                      /* XADD Gd,Ed */
            nextop = F8;
            GET_ED;
            tmp32u = add32(emu, ED->dword[0], GD.dword[0]);
            GD.dword[0] = ED->dword[0];
            ED->dword[0] = tmp32u;
            NEXT;
        _0f_0xC2:                      /* CMPPS Gx, Ex, Ib */
            nextop = F8;
            GET_EX;
            tmp8u = F8;
            switch(tmp8u&7) {
                case 0: GX.f=simde_mm_cmpeq_ps(GX.f, EX->f); break;
                case 1: GX.f=simde_mm_cmplt_ps(GX.f, EX->f); break;
                case 2: GX.f=simde_mm_cmple_ps(GX.f, EX->f); break;
                case 3: GX.f=simde_mm_cmpunord_ps(GX.f, EX->f); break;
                case 4: GX.f=simde_mm_cmpneq_ps(GX.f, EX->f); break;
                case 5: GX.f=simde_mm_cmpge_ps(GX.f, EX->f); break;
                case 6: GX.f=simde_mm_cmpgt_ps(GX.f, EX->f); break;
                case 7: GX.f=simde_mm_cmpord_ps(GX.f, EX->f); break;
            }
            NEXT;

        _0f_0xC6:                      /* SHUFPS Gx, Ex, Ib */
            nextop = F8;
            GET_EX;
            tmp8u = F8;
            GX.f = simde_mm_shuffle_ps(GX.f, EX->f, tmp8u);
            NEXT;
        _0f_0xC7:                      /* CMPXCHG8B Gq */
            CHECK_FLAGS(emu);
            nextop = F8;
            GET_ED;
            tmp32u = ED->dword[0];
            tmp32u2= ED->dword[1];
            if(R_EAX == tmp32u && R_EDX == tmp32u2) {
                SET_FLAG(F_ZF);
                ED->dword[0] = R_EBX;
                ED->dword[1] = R_ECX;
            } else {
                CLEAR_FLAG(F_ZF);
                R_EAX = tmp32u;
                R_EDX = tmp32u2;
            }
            NEXT;
        _0f_0xC8:
        _0f_0xC9:
        _0f_0xCA:
        _0f_0xCB:
        _0f_0xCC:
        _0f_0xCD:
        _0f_0xCE:
        _0f_0xCF:                  /* BSWAP reg */
            tmp8s = opcode&7;
            emu->regs[tmp8s].dword[0] = __builtin_bswap32(emu->regs[tmp8s].dword[0]);
            NEXT;

