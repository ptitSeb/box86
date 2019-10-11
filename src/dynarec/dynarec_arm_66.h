#include "arm_emitter.h"
#include "dynarec_arm_660f.h"

static uintptr_t dynarec66(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int* ok, int* need_epilog)
{
    uintptr_t ip = addr-1;
    uint8_t opcode = F8;
    uint8_t nextop;
    uint32_t u32;
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

        case 0x38:
            INST_NAME("CMP Ew, Gw");
            nextop = F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = xEAX+(nextop&7);
                UXTH(x1, ed, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3);
                STRH_IMM8(x1, ed, 0);
            }
            UXTH(x2, gd, 0);
            CALL(cmp16, -1, 0);
            UFLAGS(1);
            break;
        case 0x3B:
            INST_NAME("CMP Gw, Ew");
            nextop = F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = xEAX+(nextop&7);
                UXTH(x2, ed, 0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3);
                STRH_IMM8(x2, ed, 0);
            }
            UXTH(x1, gd, 0);
            CALL(cmp16, -1, 0);
            UFLAGS(1);
            break;
        case 0x3D:
            INST_NAME("CMP AX, Id");
            i32 = F16;
            MOV32(x2, i32);
            UXTH(x1, xEAX, 0);
            CALL(cmp16, -1, 0);
            UFLAGS(1);
            break;

        case 0x89:
            INST_NAME("MOV Ew, Gw");
            nextop = F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = xEAX+(nextop&7);
                if(ed!=gd) {
                    BFI(ed, gd, 0, 16);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2);
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
                    BFI(gd, ed, 0, 16);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                LDRH_IMM8(x1, ed, 0);
                BFI(gd, x1, 0, 16);
            }
            break;

        case 0x90:
            INST_NAME("NOP");
            break;

        case 0xA1:
            INST_NAME("MOV, AX, Od");
            u32 = F32;
            MOV32(x2, u32);
            LDRH_IMM8(x2, x2, 0);
            BFI(xEAX, x2, 0, 16);
            break;
        case 0xA3:
            INST_NAME("MOV Od, AX");
            u32 = F32;
            MOV32(x2, u32);
            STRH_IMM8(xEAX, x2, 0);
            break;


        case 0xC7:
            INST_NAME("MOV Ew, Iw");
            nextop = F8;
            if((nextop&0xC0)==0xC0) {
                ed = xEAX+(nextop&7);
                u16 = F16;
                MOVW(x1, u16);
                BFI(ed, x1, 0, 16);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2);
                u16 = F16;
                MOVW(x1, u16);
                STRH_IMM8(x1, ed, 0);
            }
            break;

        default:
            *ok = 0;
            DEFAULT;
    }
    return addr;
}

