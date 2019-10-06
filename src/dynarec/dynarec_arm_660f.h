#include "arm_emitter.h"

static uintptr_t dynarec660f(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int* ok, int* need_epilog)
{
    uintptr_t ip = addr-2;
    uint8_t opcode = F8;
    uint8_t nextop;
    int32_t i32, i32_;
    int16_t i16;
    uint16_t u16;
    uint8_t gd, ed, wback;
    uint8_t eb1, eb2;
    switch(opcode) {

        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;
        
        default:
            *ok = 0;
            DEFAULT;
    }
    return addr;
}

