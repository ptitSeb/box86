static uintptr_t dynarecGS(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int* ok, int* need_epilog)
{
    uintptr_t ip = addr-1;
    uint8_t opcode = F8;
    uint8_t nextop;
    int32_t i32;
    uint8_t gd, ed, wback;
    switch(opcode) {
        case 0x33:
            grab_tlsdata(dyn, addr, ninst, 12);
            INST_NAME("GS:XOR Gd, Ed");
            nextop = F8;
            GETGD;
            GETEDO(12);
            XOR_REG_LSL_IMM8(gd, gd, ed, 0);
            UFLAG_RES(gd);
            UFLAG_DF(1, d_xor32);
            UFLAGS(0);
            break;

        case 0xA1:
            grab_tlsdata(dyn, addr, ninst, 1);
            INST_NAME("GS:MOV EAX,Id");
            i32 = F32S;
            if(i32>0 && i32<256) {
                LDR_IMM9(xEAX, 1, i32);
            } else {
                MOV32(2, i32);
                ADD_REG_LSL_IMM8(1, 1, 2, 0);
                LDR_IMM9(xEAX, 1, 0);
            }
            break;
        default:
            *ok = 0;
            DEFAULT;
    }
    return addr;
}

