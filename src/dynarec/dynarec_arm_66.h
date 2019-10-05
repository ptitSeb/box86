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

