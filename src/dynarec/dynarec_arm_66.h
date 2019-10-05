#include "arm_emitter.h"

static uintptr_t dynarec66(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int* ok, int* need_epilog)
{
    uintptr_t ip = addr-1;
    uint8_t opcode = F8;
    uint8_t nextop;
    int32_t i32;
    int16_t i16;
    uint16_t u16;
    uint8_t gd, ed, wback;
    switch(opcode) {
        
        case 0x89:
            INST_NAME("MOV Ew, Gw");
            nextop = F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {
                if(ed!=gd) {
                    ed = xEAX+(nextop&7);
                    // need to preserve upperbit... It's bit complicated, isn't there an opcode for just that?
                    MOVW(1, 0xffff);
                    BIC_REG_LSL_IMM8(ed, ed, 1, 0);
                    ADD_REG_LSL_IMM8(1, 1, gd, 0);
                    ORR_REG_LSL_IMM8(ed, ed, 1, 0);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed);
                STRH_IMM8(gd, ed, 0);
            }
            break;

        case 0xC7:
            INST_NAME("MOV Ew, Iw");
            nextop = F8;
            if((nextop&0xC0)==0xC0) {
                ed = xEAX+(nextop&7);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed);
            }
            u16 = F16;
            MOVW(1, u16);
            STRH_IMM8(1, ed, 0);
            break;

        default:
            *ok = 0;
            DEFAULT;
    }
    return addr;
}

