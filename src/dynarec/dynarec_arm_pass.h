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

void NAME_STEP(dynarec_arm_t* dyn, uintptr_t addr)
{
    uint8_t nextop;
    int ok = 1;
    int ninst = 0;
    uintptr_t ip = addr;
    uint8_t gd;
    int32_t i32;
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
                STRB_NIMM9_W(gd, xESP, 4);
                break;
            
            case 0x89:
                INST_NAME("MOV Ed, Gd");
                nextop=F8;
                gd = xEAX+((nextop&0x38)>>3);
                if((nextop&0xC0)==0xC0) {   // reg <= reg
                    MOV_REG(xEAX+(nextop&7), gd);
                } else {                    // mem <= reg
                    addr = geted(dyn, addr, ninst, nextop);
                    STR_IMM9(2, gd, 0);
                }
                break;

            case 0xE8:
                INST_NAME("CALL rel");
                i32 = F32S;
                MOV32(2, addr);
                STRB_NIMM9_W(2, xESP, 4);
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