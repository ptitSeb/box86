#include "arm_emitter.h"

static uintptr_t dynarec0f(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int* ok, int* need_epilog)
{
    uintptr_t ip = addr-1;
    uint8_t opcode = F8;
    uint8_t nextop;
    int32_t i32, i32_;
    int16_t i16;
    uint16_t u16;
    uint8_t gd, ed, wback;
    switch(opcode) {
        
        #define GO(GETFLAGS, NO, YES)   \
            i32_ = F32S;   \
            USEFLAG;    \
            JUMP(addr+i32_);\
            GETFLAGS;   \
            if(dyn->insts) {    \
                if(dyn->insts[ninst].x86.jmp_insts==-1) {   \
                    /* out of the block */                  \
                    i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8); \
                    Bcond(NO, i32);     \
                    jump_to_linker(dyn, addr+i32_, 0, ninst); \
                } else {    \
                    /* inside the block */  \
                    i32 = dyn->insts[dyn->insts[ninst].x86.jmp_insts].address-(dyn->arm_size+8);    \
                    Bcond(YES, i32);    \
                }   \
            }

        case 0x80:
            INST_NAME("JO id");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_OF]));
                CMPS_IMM8(2, 2, 1)
                , cNE, cEQ)
            break;
        case 0x81:
            INST_NAME("JNO id");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_OF]));
                CMPS_IMM8(2, 2, 1)
                , cEQ, cNE)
            break;
        case 0x82:
            INST_NAME("JC id");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                CMPS_IMM8(2, 2, 1)
                , cNE, cEQ)
            break;
        case 0x83:
            INST_NAME("JNC id");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                CMPS_IMM8(2, 2, 1)
                , cEQ, cNE)
            break;
        case 0x84:
            INST_NAME("JZ id");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_ZF]));
                CMPS_IMM8(2, 2, 1)
                , cNE, cEQ)
            break;
        case 0x85:
            INST_NAME("JNZ id");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_ZF]));
                CMPS_IMM8(2, 2, 1)
                , cEQ, cNE)
            break;
        case 0x86:
            INST_NAME("JBE id");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                LDR_IMM9(3, 0, offsetof(x86emu_t, flags[F_ZF]));
                ORR_REG_LSL_IMM8(2, 2, 3, 0);
                CMPS_IMM8(2, 2, 1)
                , cNE, cEQ)
            break;
        case 0x87:
            INST_NAME("JNBE id");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                LDR_IMM9(3, 0, offsetof(x86emu_t, flags[F_ZF]));
                ORR_REG_LSL_IMM8(2, 2, 3, 0);
                CMPS_IMM8(2, 2, 1)
                , cEQ, cNE)
            break;
        case 0x88:
            INST_NAME("JS id");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                CMPS_IMM8(2, 2, 1)
                , cNE, cEQ)
            break;
        case 0x89:
            INST_NAME("JNS id");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                CMPS_IMM8(2, 2, 1)
                , cEQ, cNE)
            break;
        case 0x8A:
            INST_NAME("JP id");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_PF]));
                CMPS_IMM8(2, 2, 1)
                , cNE, cEQ)
            break;
        case 0x8B:
            INST_NAME("JNP id");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_PF]));
                CMPS_IMM8(2, 2, 1)
                , cEQ, cNE)
            break;
        case 0x8C:
            INST_NAME("JL id");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(1, 0, offsetof(x86emu_t, flags[F_OF]));
                CMPS_REG_LSL_IMM8(1, 1, 2, 0)
                , cEQ, cNE)
            break;
        case 0x8D:
            INST_NAME("JGE id");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(1, 0, offsetof(x86emu_t, flags[F_OF]));
                CMPS_REG_LSL_IMM8(1, 1, 2, 0)
                , cNE, cEQ)
            break;
        case 0x8E:
            INST_NAME("JLE id");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(1, 0, offsetof(x86emu_t, flags[F_OF]));
                XOR_REG_LSL_IMM8(3, 1, 2, 0);
                LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_ZF]));
                ORR_REG_LSL_IMM8(2, 2, 3, 0);
                CMPS_IMM8(2, 2, 1);
                , cNE, cEQ)
            break;
        case 0x8F:
            INST_NAME("JG id");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(1, 0, offsetof(x86emu_t, flags[F_OF]));
                XOR_REG_LSL_IMM8(3, 1, 2, 0);
                LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_ZF]));
                XOR_IMM8(3, 3, 1);
                XOR_IMM8(2, 2, 1);
                TSTS_REG_LSL_IMM8(2, 2, 3, 0);
                , cEQ, cNE)
            break;
        #undef GO
            
        default:
            *ok = 0;
            DEFAULT;
    }
    return addr;
}

