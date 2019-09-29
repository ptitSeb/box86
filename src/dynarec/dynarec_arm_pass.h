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
static uintptr_t geted(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop) 
{
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            if((sib&0x7)==5) {
                uint32_t tmp = F32;
                MOV32(2, tmp);
            } else {
                MOV_REG(2, xEAX+(sib&0x7));
            }
            int sib_reg = (sib>>3)&7;
            if (sib_reg!=4) {
                ADD_REG_LSL_IMM8(2, 2, xEAX+sib_reg, (sib>>6));
            }
        } else if((nextop&7)==5) {
            uint32_t tmp = F32;
            MOV32(2, tmp);
        } else {
            MOV_REG(2, xEAX+(nextop&7));
        }
    } else {
        uintptr_t base;
        if((nextop&7)==4) {
            uint8_t sib = F8;
            MOV_REG(2, xEAX+(sib&0x07));
            int sib_reg = (sib>>3)&7;
            if (sib_reg!=4) {
                ADD_REG_LSL_IMM8(2, 2, xEAX+sib_reg, (sib>>6));
            }
        } else {
            MOV_REG(2, xEAX+(nextop&0x07));
        }
        if(nextop&0x80) {
            uint32_t t32 = F32;
            MOV32(3, t32);
            ADD_REG_LSL_IMM8(2, 2, 3, 0);
        } else {
            int8_t t8 = F8;
            if(t8<0) {
                SUB_IMM8(2, 2, -t8);
            } else if (t8>0) {
                ADD_IMM8(2, 2, t8);
            }
        }
    }
   
    return addr;
}

static void jump_to_epilog(dynarec_arm_t* dyn, uintptr_t ip, int ninst)
{
    MESSAGE(LOG_DEBUG, "Jump to epilog\n");
    MOV32(xEIP, ip);
    void* epilog = arm_epilog;
    MOV32(2, (uintptr_t)epilog);
    BX(2);
}

static void ret_to_epilog(dynarec_arm_t* dyn, int ninst)
{
    MESSAGE(LOG_DEBUG, "Ret epilog\n");
    LDR_IMM9_W(xEIP, xESP, 4);
    void* epilog = arm_epilog;
    MOV32(2, (uintptr_t)epilog);
    BX(2);
}

static void arm_to_x86_flags(dynarec_arm_t* dyn, int ninst)
{
    ADD_IMM8(1, 0, offsetof(x86emu_t, flags)+F_ZF);
    MOVW_COND(cEQ, 1, 1);
    MOVW_COND(cNE, 1, 0);
    ADD_IMM8(1, 0, offsetof(x86emu_t, flags)+F_CF);
    MOVW_COND(cCS, 1, 1);
    MOVW_COND(cCC, 1, 0);
    ADD_IMM8(1, 0, offsetof(x86emu_t, flags)+F_SF);
    MOVW_COND(cMI, 1, 1);
    MOVW_COND(cPL, 1, 0);
    ADD_IMM8(1, 0, offsetof(x86emu_t, flags)+F_OF);
    MOVW_COND(cVS, 1, 1);
    MOVW_COND(cVC, 1, 0);
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
                STR_NIMM9_W(gd, xESP, 4);
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
                    addr = geted(dyn, addr, ninst, nextop);
                    ed = 1;
                    LDR_IMM9(1, 2, 0);
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
                        } else {
                            if(i32>0 && i32<256) {
                                ADD_IMM8(ed, ed, i32);
                            } else {
                                MOV32(3, i32);
                                ADD_REG_LSL_IMM8(ed, ed, 3, 0);
                            }
                        }
                        if(dyn->insts[ninst].x86.flags) {
                            // generate flags!
                            arm_to_x86_flags(dyn, ninst);
                            // how to get P(arity) and A flag?
                        }
                        break;
                    case 5: //SUB
                        if(dyn->insts[ninst].x86.flags) {
                            MOV32(3, i32);
                            SUBS_REG_LSL_IMM8(ed, ed, 3, 0);
                        } else {
                            if(i32>0 && i32<256) {
                                SUB_IMM8(ed, ed, i32);
                            } else {
                                MOV32(3, i32);
                                SUB_REG_LSL_IMM8(ed, ed, 3, 0);
                            }
                        }
                        if(dyn->insts[ninst].x86.flags) {
                            // generate flags!
                            arm_to_x86_flags(dyn, ninst);
                            // how to get P(arity) and A flag?
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
                    if((nextop&0xC0)==0 && (nextop&7)!=4 && (nextop&7)!=5) {
                        ed = xEAX + (nextop&7);
                        STR_IMM9(ed, gd, 0);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop);
                        STR_IMM9(2, gd, 0);
                    }
                }
                break;

            case 0x8B:
                INST_NAME("MOV Gd, Ed");
                nextop=F8;
                gd = xEAX+((nextop&0x38)>>3);
                if((nextop&0xC0)==0xC0) {   // reg <= reg
                    MOV_REG(gd, xEAX+(nextop&7));
                } else {                    // mem <= reg
                    if((nextop&0xC0)==0 && (nextop&7)!=4 && (nextop&7)!=5) {
                        ed = xEAX + (nextop&7);
                        LDR_IMM9(gd, ed, 0);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop);
                        LDR_IMM9(gd, 2, 0);
                    }
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
                    if((nextop&0xC0)==0 && (nextop&7)!=4 && (nextop&7)!=5) {
                        ed = xEAX + (nextop&7);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop);
                        ed = 2;
                    }
                    i32 = F32S;
                    MOV32(gd, i32);
                    STR_IMM9(gd, ed, 0);
                }
                break;

            case 0xE8:
                INST_NAME("CALL rel");
                i32 = F32S;
                MOV32(2, addr);
                STR_NIMM9_W(2, xESP, 4);
                jump_to_epilog(dyn, addr+i32, ninst);
                need_epilog = 0;
                ok = 0;
                break;

            default:
                ok = 0;
                DEFAULT;
        }
        ++ninst;
    }
    if(need_epilog)
        jump_to_epilog(dyn, ip, ninst);
    FINI;
}