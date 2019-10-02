#ifndef STEP
#error Meh?!
#endif

#include "arm_emitter.h"

#define F8      *(uint8_t*)(addr++)
#define F8S     *(int8_t*)(addr++)
#define F16     *(uint16_t*)(addr+=2, addr-2)
#define F32     *(uint32_t*)(addr+=4, addr-4)
#define F32S    *(int32_t*)(addr+=4, addr-4)
#define PK(a)   *(uint8_t*)(addr+a)

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
                MOV32(2, tmp);
                if (sib_reg!=4) {
                    ADD_REG_LSL_IMM8(2, 2, xEAX+sib_reg, (sib>>6));
                }
            } else {
                if (sib_reg!=4) {
                    ADD_REG_LSL_IMM8(2, xEAX+(sib&0x7), xEAX+sib_reg, (sib>>6));
                } else {
                    //MOV_REG(2, xEAX+(sib&0x7));
                    ret = xEAX+(sib&0x7);
                }
            }
        } else if((nextop&7)==5) {
            uint32_t tmp = F32;
            MOV32(2, tmp);
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
                ADD_IMM8(2, tmp, t32);
            } else if(i32<0 && i32>-256) {
                SUB_IMM8(2, tmp, -t32);
            } else if(t32) {
                MOV32(3, t32);
                ADD_REG_LSL_IMM8(2, tmp, 3, 0);
            } else
                ret = tmp;
        } else {
            int8_t t8 = F8S;
            if(t8<0) {
                SUB_IMM8(2, tmp, -t8);
            } else if (t8>0) {
                ADD_IMM8(2, tmp, t8);
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
    MESSAGE(LOG_DUMP, "Jump to linker\n");
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

#define GETGD   gd = xEAX+((nextop&0x38)>>3)
#define GETED   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    wback = 0;              \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback); \
                    LDR_IMM9(1, wback, 0);  \
                    ed = 1;                 \
                }
#define WBACK   if(wback) {STR_IMM9(ed, wback, 0);}
#ifndef UFLAGS
#define UFLAGS  if(dyn->insts[ninst].x86.flags) {arm_to_x86_flags(dyn, ninst);}
#endif
#ifndef USEFLAG
#define USEFLAG     
#endif
#ifndef JUMP
#define JUMP(A) 
#endif



static void grab_tlsdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg)
{
    MESSAGE(LOG_DUMP, "Get TLSData\n");
    PUSH(13, (1<<0) | (1<<12));
    void* p = GetGSBaseEmu;
    MOV32(1, (uintptr_t)p);
    BLX(1);
    MOV_REG(reg, 0);
    POP(13, (1<<0) | (1<<12));
}

static uintptr_t dynarecGS(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int* ok, int* need_epilog)
{
    uint8_t opcode = F8;
    uint8_t nextop;
    int32_t i32;
    switch(opcode) {
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

void NAME_STEP(dynarec_arm_t* dyn, uintptr_t addr)
{
    uint8_t nextop, opcode;
    int ok = 1;
    int ninst = 0;
    uintptr_t ip = addr;
    uint8_t gd, ed;
    int8_t i8;
    int32_t i32, tmp;
    uint32_t u32;
    int need_epilog = 1;
    dyn->tablei = 0;
    uint8_t wback;
    INIT;
    while(ok) {
        ip = addr;
        opcode = F8;
        NEW_INST;
        switch(opcode) {

            case 0x01:
                INST_NAME("ADD Ed, Gd");
                nextop = F8;
                GETGD;
                GETED;
                ADDS_REG_LSL_IMM8(ed, ed, gd, 0);
                WBACK;
                UFLAGS;
                break;

            case 0x08:
                INST_NAME("OR Ed, Gd");
                nextop = F8;
                GETGD;
                GETED;
                ORRS_REG_LSL_IMM8(ed, ed, gd, 0);
                WBACK;
                UFLAGS;
                break;

            case 0x21:
                INST_NAME("AND Ed, Gd");
                nextop = F8;
                GETGD;
                GETED;
                ANDS_REG_LSL_IMM8(ed, ed, gd, 0);
                WBACK;
                UFLAGS;
                break;

            case 0x29:
                INST_NAME("SUB Ed, Gd");
                nextop = F8;
                GETGD;
                GETED;
                SUBS_REG_LSL_IMM8(ed, ed, gd, 0);
                WBACK;
                UFLAGS;
                break;

            case 0x2E:
                INST_NAME("CS:");
                // ignored
                break;

            case 0x31:
                INST_NAME("XOR Ed, Gd");
                nextop = F8;
                GETGD;
                GETED;
                XORS_REG_LSL_IMM8(ed, ed, gd, 0);
                WBACK;
                UFLAGS;
                break;

            case 0x40:
            case 0x41:
            case 0x42:
            case 0x43:
            case 0x44:
            case 0x45:
            case 0x46:
            case 0x47:
                INST_NAME("INC reg");
                gd = xEAX+(opcode&0x07);
                ADDS_IMM8(gd, gd, 1);
                UFLAGS;
                break;
            case 0x48:
            case 0x49:
            case 0x4A:
            case 0x4B:
            case 0x4C:
            case 0x4D:
            case 0x4E:
            case 0x4F:
                INST_NAME("SUB reg");
                gd = xEAX+(opcode&0x07);
                SUBS_IMM8(gd, gd, 1);
                UFLAGS;
                break;
            case 0x50:
            case 0x51:
            case 0x52:
            case 0x53:
            case 0x54:
            case 0x55:
            case 0x56:
            case 0x57:
                INST_NAME("PUSH reg");
                gd = xEAX+(opcode&0x07);
                PUSH(xESP, 1<<gd);
                break;
            case 0x58:
            case 0x59:
            case 0x5A:
            case 0x5B:
            case 0x5C:
            case 0x5D:
            case 0x5E:
            case 0x5F:
                INST_NAME("POP reg");
                gd = xEAX+(opcode&0x07);
                POP(xESP, 1<<gd);
                break;

            case 0x65:
                addr = dynarecGS(dyn, addr, ninst, &ok, &need_epilog);
                break;

            case 0x72:
                INST_NAME("JC ib");
                i8 = F8S;
                USEFLAG;
                JUMP(addr+i8);
                MOVW(1, 1);
                LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                CMPS_REG_LSL_IMM8(1, 1, 2, 0);  // JC, so 1 - F_CF  => EQ
                if(dyn->insts) {
                    if(dyn->insts[ninst].x86.jmp_insts==-1) {
                        // out of the block
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                        Bcond(cNE, i32);    // jump to next on ok, inverted
                        jump_to_linker(dyn, addr+i8, 0, ninst);
                    } else {
                        // inside the block
                        i32 = dyn->insts[dyn->insts[ninst].x86.jmp_insts].address-(dyn->arm_size+8);
                        Bcond(cEQ, i32);
                    }
                }
                break;
            case 0x73:
                INST_NAME("JNC ib");
                i8 = F8S;
                USEFLAG;
                JUMP(addr+i8);
                MOVW(1, 1);
                LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                CMPS_REG_LSL_IMM8(1, 1, 2, 0);  // JZ, so 1 - F_CF  => NE
                if(dyn->insts) {
                    if(dyn->insts[ninst].x86.jmp_insts==-1) {
                        // out of the block
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                        Bcond(cEQ, i32);    // jump to next on ok, inverted
                        jump_to_linker(dyn, addr+i8, 0, ninst);
                    } else {
                        // inside the block
                        i32 = dyn->insts[dyn->insts[ninst].x86.jmp_insts].address-(dyn->arm_size+8);
                        Bcond(cNE, i32);
                    }
                }
                break;
            case 0x74:
                INST_NAME("JZ ib");
                i8 = F8S;
                USEFLAG;
                JUMP(addr+i8);
                MOVW(1, 1);
                LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_ZF]));
                CMPS_REG_LSL_IMM8(1, 1, 2, 0);  // JZ, so 1 - F_ZF  => EQ
                if(dyn->insts) {
                    if(dyn->insts[ninst].x86.jmp_insts==-1) {
                        // out of the block
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                        Bcond(cNE, i32);    // jump to next on ok, inverted
                        jump_to_linker(dyn, addr+i8, 0, ninst);
                    } else {
                        // inside the block
                        i32 = dyn->insts[dyn->insts[ninst].x86.jmp_insts].address-(dyn->arm_size+8);
                        Bcond(cEQ, i32);
                    }
                }
                break;
            case 0x75:
                INST_NAME("JNZ ib");
                i8 = F8S;
                USEFLAG;
                JUMP(addr+i8);
                MOVW(1, 1);
                LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_ZF]));
                CMPS_REG_LSL_IMM8(1, 1, 2, 0);  // JZ, so 1 - F_ZF  => NE
                if(dyn->insts) {
                    if(dyn->insts[ninst].x86.jmp_insts==-1) {
                        // out of the block
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                        Bcond(cEQ, i32);    // jump to next on ok, inverted
                        jump_to_linker(dyn, addr+i8, 0, ninst);
                    } else {
                        // inside the block
                        i32 = dyn->insts[dyn->insts[ninst].x86.jmp_insts].address-(dyn->arm_size+8);
                        Bcond(cNE, i32);
                    }
                }
                break;
            
            case 0x81:
            case 0x83:
                nextop = F8;
                switch((nextop>>3)&7) {
                    case 0: //ADD
                        if(opcode==0x81) {INST_NAME("ADD Ed, Id");} else {INST_NAME("ADD Ed, Ib");}
                        GETED;
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        if(i32>0 && i32<256) {
                            ADDS_IMM8(ed, ed, i32);
                        } else {
                            MOV32(3, i32);
                            ADDS_REG_LSL_IMM8(ed, ed, 3, 0);
                        }
                        WBACK;
                        UFLAGS;
                        break;
                    case 5: //SUB
                        if(opcode==0x81) {INST_NAME("SUB Ed, Id");} else {INST_NAME("SUB Ed, Ib");}
                        GETED;
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        if(i32>0 && i32<256) {
                            SUBS_IMM8(ed, ed, i32);
                        } else {
                            MOV32(3, i32);
                            SUBS_REG_LSL_IMM8(ed, ed, 3, 0);
                        }
                        WBACK;
                        UFLAGS;
                        break;
                    default:
                        if(opcode==0x81) {INST_NAME("GRP1 Ed, Id");} else {INST_NAME("GRP1 Ed, Ib");}
                        ok = 0;
                        DEFAULT;
                }
                break;
            
            case 0x85:
                INST_NAME("TEST Ed, Gd");
                nextop=F8;
                GETGD;
                GETED;
                TSTS_REG_LSL_IMM8(ed, ed, gd, 0);
                UFLAGS;
                break;

            case 0x89:
                INST_NAME("MOV Ed, Gd");
                nextop=F8;
                GETGD;
                if((nextop&0xC0)==0xC0) {   // reg <= reg
                    MOV_REG(xEAX+(nextop&7), gd);
                } else {                    // mem <= reg
                    addr = geted(dyn, addr, ninst, nextop, &ed);
                    STR_IMM9(gd, ed, 0);
                }
                break;

            case 0x8B:
                INST_NAME("MOV Gd, Ed");
                nextop=F8;
                GETGD;
                if((nextop&0xC0)==0xC0) {   // reg <= reg
                    MOV_REG(gd, xEAX+(nextop&7));
                } else {                    // mem <= reg
                    addr = geted(dyn, addr, ninst, nextop, &ed);
                    LDR_IMM9(gd, ed, 0);
                }
                break;

            case 0x8D:
                INST_NAME("LEA Gd, Ed");
                nextop=F8;
                GETGD;
                if((nextop&0xC0)==0xC0) {   // reg <= reg? that's an invalid operation
                    ok=0;
                    DEFAULT;
                } else {                    // mem <= reg
                    addr = geted(dyn, addr, ninst, nextop, &ed);
                    MOV_REG(gd, ed);
                }
                break;

            case 0xC3:
                INST_NAME("RET");
                ret_to_epilog(dyn, ninst);
                need_epilog = 0;
                ok = 0;
                break;

            case 0xC7:
                INST_NAME("MOV Ed, Id");
                nextop=F8;
                if((nextop&0xC0)==0xC0) {   // reg <= i32
                    i32 = F32S;
                    ed = xEAX+(nextop&7);
                    MOV32(ed, i32);
                } else {                    // mem <= i32
                    gd = 3;
                    addr = geted(dyn, addr, ninst, nextop, &ed);
                    u32 = F32;
                    MOV32(gd, u32);
                    STR_IMM9(gd, ed, 0);
                }
                break;

            case 0xE8:
                INST_NAME("CALL Id");
                i32 = F32S;
                MOV32(2, addr);
                PUSH(xESP, 1<<2);
                jump_to_linker(dyn, addr+i32, 0, ninst);
                need_epilog = 0;
                ok = 0;
                break;
            case 0xE9:
            case 0xEB:
                if(opcode==0xE9) {
                    INST_NAME("JMP Id");
                    i32 = F32S;
                } else {
                    INST_NAME("JMP Ib");
                    i32 = F8S;
                }
                JUMP(addr+i32);
                if(dyn->insts) {
                    if(dyn->insts[ninst].x86.jmp_insts==-1) {
                        // out of the block
                        jump_to_linker(dyn, addr+i32, 0, ninst);
                    } else {
                        // inside the block
                        tmp = dyn->insts[dyn->insts[ninst].x86.jmp_insts].address-(dyn->arm_size+8);
                        Bcond(c__, tmp);
                    }
                }
                need_epilog = 0;
                ok = 0;
                break;

            case 0xFF:
                nextop = F8;
                switch((nextop>>3)&7) {
                    case 0: // INC Ed
                        INST_NAME("INC Ed");
                        GETED;
                        ADDS_IMM8(ed, ed, 1);
                        WBACK;
                        UFLAGS;
                        break;
                    case 1: //DEC Ed
                        INST_NAME("DEC Ed");
                        GETED;
                        SUBS_IMM8(ed, ed, 1);
                        WBACK;
                        UFLAGS;
                        break;
                    case 2: // CALL Ed
                        INST_NAME("CALL Ed");
                        GETED;
                        MOV32(3, addr);
                        PUSH(xESP, 1<<3);
                        jump_to_epilog(dyn, 0, ed, ninst);  // it's variable, so no linker
                        need_epilog = 0;
                        ok = 0;
                        break;
                    case 4: // JMP Ed
                        INST_NAME("JMP Ed");
                        GETED;
                        MOV32(3, addr);
                        jump_to_epilog(dyn, 0, ed, ninst);     // it's variable, so no linker
                        need_epilog = 0;
                        ok = 0;
                        break;
                    case 6: // Push Ed
                        INST_NAME("PUSH Ed");
                        GETED;
                        PUSH(xESP, 1<<ed);
                        break;

                    default:
                        INST_NAME("Grp5 Ed");
                        ok = 0;
                        DEFAULT;
                }
                break;

            default:
                ok = 0;
                DEFAULT;
        }
        ++ninst;
    }
    if(need_epilog)
        jump_to_epilog(dyn, ip, 0, ninst);  // no linker here, it's an unknow instruction
    FINI;
}