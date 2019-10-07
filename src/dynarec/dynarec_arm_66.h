#include "arm_emitter.h"
#include "dynarec_arm_660f.h"

static uintptr_t dynarec66(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int* ok, int* need_epilog)
{
    uintptr_t ip = addr-1;
    uint8_t opcode = F8;
    uint8_t nextop;
    int32_t i32;
    int16_t i16;
    uint16_t u16;
    uint8_t gd, ed, wback;
    while(opcode==0x66) opcode = F8;    // "unlimited" 0x66 as prefix for variable sized NOP
    if(opcode==0x2E) opcode = F8;       // cs: is ignored
    switch(opcode) {
        
        case 0x0F:
            addr = dynarec660f(dyn, addr, ninst, ok, need_epilog);
            break;

        case 0x89:
            INST_NAME("MOV Ew, Gw");
            nextop = F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = xEAX+(nextop&7);
                if(ed!=gd) {
                    // need to preserve upperbit... It's bit complicated, isn't there an opcode for just that?
                    MOVW(1, 0xffff);
                    BIC_REG_LSL_IMM8(ed, ed, 1, 0);
                    AND_REG_LSL_IMM8(1, 1, gd, 0);
                    ORR_REG_LSL_IMM8(ed, ed, 1, 0);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, 2);
                STRH_IMM8(gd, ed, 0);
            }
            break;
        case 0x8B:
            INST_NAME("MOV Gw, Ew");
            nextop = F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = xEAX+(nextop&7);
                if(ed!=gd) {
                    // need to preserve upperbit... It's bit complicated, isn't there an opcode for just that?
                    MOVW(1, 0xffff);
                    BIC_REG_LSL_IMM8(gd, gd, 1, 0);
                    AND_REG_LSL_IMM8(1, 1, ed, 0);
                    ORR_REG_LSL_IMM8(gd, gd, 1, 0);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, 2);
                LDRH_IMM8(gd, ed, 0);
            }
            break;

        case 0x90:
            INST_NAME("NOP");
            break;


        case 0xC7:
            INST_NAME("MOV Ew, Iw");
            nextop = F8;
            if((nextop&0xC0)==0xC0) {
                ed = xEAX+(nextop&7);
                u16 = F16;
                MOV32(12, 0xffff);
                MOVW(1, u16);
                BIC_REG_LSL_IMM8(ed, ed, 12, 0);
                ORR_REG_LSL_IMM8(ed, ed, 1, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, 2);
                u16 = F16;
                MOVW(1, u16);
                STRH_IMM8(1, ed, 0);
            }
            break;

        default:
            *ok = 0;
            DEFAULT;
    }
    return addr;
}

