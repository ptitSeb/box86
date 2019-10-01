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
            if(t32) {
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
    STR_IMM9(1, 0, offsetof(x86emu_t, flags)+F_ZF);
    MOVW_COND(cCS, 1, 1);
    MOVW_COND(cCC, 1, 0);
    STR_IMM9(1, 0, offsetof(x86emu_t, flags)+F_CF);
    MOVW_COND(cMI, 1, 1);
    MOVW_COND(cPL, 1, 0);
    STR_IMM9(1, 0, offsetof(x86emu_t, flags)+F_SF);
    MOVW_COND(cVS, 1, 1);
    MOVW_COND(cVC, 1, 0);
    STR_IMM9(1, 0, offsetof(x86emu_t, flags)+F_OF);
}

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
    uint8_t nextop;
    int ok = 1;
    int ninst = 0;
    uintptr_t ip = addr;
    uint8_t gd, ed;
    int32_t i32;
    int8_t i8;
    uint32_t tmp;
    int need_epilog = 1;
    INIT;
    while(ok) {
        ip = addr;
        nextop = F8;
        NEW_INST;
        switch(nextop) {

            case 0x31:
                INST_NAME("XOR Gd, Ed");
                FLAGS(X86_FLAGS_CHANGE);
                nextop = F8;
                if((nextop&0xC0)==0xC0) {   // reg <= reg
                    ed = xEAX+(nextop&7);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed);
                    LDR_IMM9(1, ed, 0);
                    ed = 1;
                }
                if(dyn->insts[ninst].x86.flags) {
                    XORS_REG_LSL_IMM8(ed, ed, gd, 0);
                    // generate flags!
                    arm_to_x86_flags(dyn, ninst);
                } else {
                    MOV32(3, i32);
                    XOR_REG_LSL_IMM8(ed, ed, gd, 0);
                }
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
                gd = xEAX+(nextop&0x07);
                PUSH(xESP, 1<<gd);
                break;

            case 0x65:
                addr = dynarecGS(dyn, addr, ninst, &ok, &need_epilog);
                break;
            
            case 0x81:
            case 0x83:
                if(nextop==0x81) {
                    INST_NAME("Grp Ed, Id");
                } else {
                    INST_NAME("Grp Ed, Ib");
                }
                tmp=nextop;
                nextop = F8;
                FLAGS(X86_FLAGS_CHANGE);
                if((nextop&0xC0)==0xC0) {   // reg <= reg
                    ed = xEAX+(nextop&7);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed);
                    LDR_IMM9(1, ed, 0);
                    ed = 1;
                }
                if(tmp==0x81)
                    i32 = F32S;
                else
                    i32 = F8S;
                switch((nextop>>3)&7) {
                    case 0: //ADD
                        if(dyn->insts[ninst].x86.flags) {
                            MOV32(3, i32);
                            ADDS_REG_LSL_IMM8(ed, ed, 3, 0);
                            // generate flags!
                            arm_to_x86_flags(dyn, ninst);
                            // how to get P(arity) and A flag?
                        } else {
                            if(i32>0 && i32<256) {
                                ADD_IMM8(ed, ed, i32);
                            } else {
                                MOV32(3, i32);
                                ADD_REG_LSL_IMM8(ed, ed, 3, 0);
                            }
                        }
                        break;
                    case 5: //SUB
                        if(dyn->insts[ninst].x86.flags) {
                            MOV32(3, i32);
                            SUBS_REG_LSL_IMM8(ed, ed, 3, 0);
                            // generate flags!
                            arm_to_x86_flags(dyn, ninst);
                            // how to get P(arity) and A flag?
                        } else {
                            if(i32>0 && i32<256) {
                                SUB_IMM8(ed, ed, i32);
                            } else {
                                MOV32(3, i32);
                                SUB_REG_LSL_IMM8(ed, ed, 3, 0);
                            }
                        }
                        break;
                    default:
                        ok = 0;
                        DEFAULT;
                }
                if(ed==1) {
                    STR_IMM9(1, 2, 0);
                }
                break;
            
            case 0x89:
                INST_NAME("MOV Ed, Gd");
                nextop=F8;
                gd = xEAX+((nextop&0x38)>>3);
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
                gd = xEAX+((nextop&0x38)>>3);
                if((nextop&0xC0)==0xC0) {   // reg <= reg
                    MOV_REG(gd, xEAX+(nextop&7));
                } else {                    // mem <= reg
                    addr = geted(dyn, addr, ninst, nextop, &ed);
                    LDR_IMM9(gd, ed, 0);
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
                    i32 = F32S;
                    MOV32(gd, i32);
                    STR_IMM9(gd, ed, 0);
                }
                break;

            case 0xE8:
                INST_NAME("CALL rel");
                i32 = F32S;
                MOV32(2, addr);
                PUSH(xESP, 1<<2);
                jump_to_epilog(dyn, addr+i32, 0, ninst);
                need_epilog = 0;
                ok = 0;
                break;

            case 0xFF:
                INST_NAME("Grp5 Ed");
                nextop = F8;
                if((nextop&0xC0)==0xC0) {   // reg <= reg
                    ed = xEAX+(nextop&7);
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed);
                    LDR_IMM9(1, ed, 0);
                    ed = 1;
                }
                switch((nextop>>3)&7) {
                    case 0: // INC Ed
                        FLAGS(X86_FLAGS_CHANGE);
                        if(dyn->insts[ninst].x86.flags) {
                            ADDS_IMM8(ed, ed, 1);
                            // generate flags!
                            arm_to_x86_flags(dyn, ninst);
                            // how to get P(arity) and A flag?
                        } else {
                            ADD_IMM8(ed, ed, 1);
                        }
                        break;
                    case 1: //DEC Ed
                        FLAGS(X86_FLAGS_CHANGE);
                        if(dyn->insts[ninst].x86.flags) {
                            SUBS_IMM8(ed, ed, 1);
                            // generate flags!
                            arm_to_x86_flags(dyn, ninst);
                            // how to get P(arity) and A flag?
                        } else {
                            SUB_IMM8(ed, ed, 1);
                        }
                        break;
                    case 2: // CALL Ed
                        MOV32(2, addr);
                        PUSH(xESP, 1<<2);
                        jump_to_epilog(dyn, 0, ed, ninst);
                        need_epilog = 0;
                        ok = 0;
                        break;
                    case 6: // Push Ed
                        PUSH(xESP, 1<<ed);
                        break;

                    default:
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
        jump_to_epilog(dyn, ip, 0, ninst);
    FINI;
}