void arm_epilog();
void* arm_linker(x86emu_t* emu, void** table, uintptr_t addr);

/* setup r2 to address pointed by */
static uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop, uint8_t* ed) 
{
    uint8_t ret = 2;
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = (sib>>3)&7;
            if((sib&0x7)==5) {
                uint32_t tmp = F32;
                MOV32(ret, tmp);
                if (sib_reg!=4) {
                    ADD_REG_LSL_IMM8(ret, ret, xEAX+sib_reg, (sib>>6));
                }
            } else {
                if (sib_reg!=4) {
                    ADD_REG_LSL_IMM8(ret, xEAX+(sib&0x7), xEAX+sib_reg, (sib>>6));
                } else {
                    //MOV_REG(2, xEAX+(sib&0x7));
                    ret = xEAX+(sib&0x7);
                }
            }
        } else if((nextop&7)==5) {
            uint32_t tmp = F32;
            MOV32(ret, tmp);
        } else {
            //MOV_REG(2, xEAX+(nextop&7));
            ret = xEAX+(nextop&7);
        }
    } else {
        int tmp = 2;
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = (sib>>3)&7;
            if (sib_reg!=4) {
                ADD_REG_LSL_IMM8(2, xEAX+(sib&0x07), xEAX+sib_reg, (sib>>6));
            } else {
                tmp = xEAX+(sib&0x07);
            }
        } else {
            tmp = xEAX+(nextop&0x07);
        }
        if(nextop&0x80) {
            uint32_t t32 = F32;
            int32_t i32 = (int32_t)t32;
            if(i32>0 && i32<255) {
                ADD_IMM8(ret, tmp, t32);
            } else if(i32<0 && i32>-256) {
                SUB_IMM8(ret, tmp, -t32);
            } else if(t32) {
                MOV32(3, t32);
                ADD_REG_LSL_IMM8(ret, tmp, 3, 0);
            } else
                ret = tmp;
        } else {
            int8_t t8 = F8S;
            if(t8<0) {
                SUB_IMM8(ret, tmp, -t8);
            } else if (t8>0) {
                ADD_IMM8(ret, tmp, t8);
            } else
                ret = tmp;
        }
    }
    *ed = ret;
    return addr;
}

static void jump_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MESSAGE(LOG_DUMP, "Jump to epilog\n");
    if(reg) {
        MOV_REG(xEIP, reg);
    } else {
        MOV32(xEIP, ip);
    }
    void* epilog = arm_epilog;
    MOV32(2, (uintptr_t)epilog);
    BX(2);
}

static void jump_to_linker(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MESSAGE(LOG_DUMP, "Jump to linker (#%d)\n", dyn->tablei);
    if(reg) {
        MOV_REG(xEIP, reg);
    } else {
        MOV32(xEIP, ip);
    }
    uintptr_t* table = 0;
    if(dyn->tablesz) {
        table = &dyn->table[dyn->tablei];
        *table = (uintptr_t)arm_linker;
    }
    ++dyn->tablei;
    MOV32_(1, (uintptr_t)table);
    LDR_IMM9(2, 1, 0);
    BX(2);
}

static void ret_to_epilog(dynarec_arm_t* dyn, int ninst)
{
    MESSAGE(LOG_DUMP, "Ret epilog\n");
    POP(xESP, 1<<xEIP);
    void* epilog = arm_epilog;
    MOV32(2, (uintptr_t)epilog);
    BX(2);
}

static void retn_to_epilog(dynarec_arm_t* dyn, int ninst, int n)
{
    MESSAGE(LOG_DUMP, "Retn epilog\n");
    POP(xESP, 1<<xEIP);
    ADD_IMM8(xESP, xESP, n);
    void* epilog = arm_epilog;
    MOV32(2, (uintptr_t)epilog);
    BX(2);
}

static void arm_to_x86_flags(dynarec_arm_t* dyn, int ninst)
{
    MOVW_COND(cEQ, 1, 1);
    MOVW_COND(cNE, 1, 0);
    STR_IMM9(1, 0, offsetof(x86emu_t, flags[F_ZF]));
    MOVW_COND(cCS, 1, 1);
    MOVW_COND(cCC, 1, 0);
    STR_IMM9(1, 0, offsetof(x86emu_t, flags[F_CF]));
    MOVW_COND(cMI, 1, 1);
    MOVW_COND(cPL, 1, 0);
    STR_IMM9(1, 0, offsetof(x86emu_t, flags[F_SF]));
    MOVW_COND(cVS, 1, 1);
    MOVW_COND(cVC, 1, 0);
    STR_IMM9(1, 0, offsetof(x86emu_t, flags[F_OF]));
}

static void call_c(dynarec_arm_t* dyn, int ninst, void* fnc, int reg, int ret)
{
    MOV32(reg, (uintptr_t)fnc);
    PUSH(13, (1<<0));
    BLX(reg);
    if(ret>=0) {
        MOV_REG(ret, 0);
    }
    POP(13, (1<<0));
}

static void grab_tlsdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg)
{
    MESSAGE(LOG_DUMP, "Get TLSData\n");
    call_c(dyn, ninst, GetGSBaseEmu, 12, reg);
}

static int isNativeCall(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t* calladdress, int* retn)
{
    if(PK(0)==0xff && PK(1)==0x25)  // aboslute jump, maybe the GOT
        addr = PK32(2);
    if(PK(0)==0xCC && PK(1)=='S' && PK(2)=='C' && PK32(3)!=0) {
        // found !
        if(retn) *retn = (PK(3+4+4+1)==0xc2)?PK(3+4+4+2):0;
        if(calladdress) *calladdress = addr;
        return 1;
    }
    return 0;
}