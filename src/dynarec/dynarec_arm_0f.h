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
    uint8_t eb1, eb2;
    switch(opcode) {
        
        #define GO(GETFLAGS, NO, YES)   \
            USEFLAG;    \
            GETFLAGS;   \
            nextop=F8;  \
            GETGD;      \
            if((nextop&0xC0)==0xC0) {   \
                ed = xEAX+(nextop&7);   \
                MOV_REG_COND(YES, gd, ed); \
            } else { \
                addr = geted(dyn, addr, ninst, nextop, &ed);    \
                LDR_IMM9_COND(YES, gd, ed, 0); \
            }

        case 0x40:
            INST_NAME("CMOVO Gd, Ed");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_OF]));
                CMPS_IMM8(2, 2, 1)
                , cNE, cEQ)
            break;
        case 0x41:
            INST_NAME("CMOVNO Gd, Ed");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_OF]));
                CMPS_IMM8(2, 2, 1)
                , cEQ, cNE)
            break;
        case 0x42:
            INST_NAME("CMOVC Gd, Ed");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                CMPS_IMM8(2, 2, 1)
                , cNE, cEQ)
            break;
        case 0x43:
            INST_NAME("CMOVNC Gd, Ed");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                CMPS_IMM8(2, 2, 1)
                , cEQ, cNE)
            break;
        case 0x44:
            INST_NAME("CMOVZ Gd, Ed");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_ZF]));
                CMPS_IMM8(2, 2, 1)
                , cNE, cEQ)
            break;
        case 0x45:
            INST_NAME("CMOVNZ Gd, Ed");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_ZF]));
                CMPS_IMM8(2, 2, 1)
                , cEQ, cNE)
            break;
        case 0x46:
            INST_NAME("CMOVBE Gd, Ed");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                LDR_IMM9(3, 0, offsetof(x86emu_t, flags[F_ZF]));
                ORR_REG_LSL_IMM8(2, 2, 3, 0);
                CMPS_IMM8(2, 2, 1)
                , cNE, cEQ)
            break;
        case 0x47:
            INST_NAME("CMOVNBE Gd, Ed");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                LDR_IMM9(3, 0, offsetof(x86emu_t, flags[F_ZF]));
                ORR_REG_LSL_IMM8(2, 2, 3, 0);
                CMPS_IMM8(2, 2, 1)
                , cEQ, cNE)
            break;
        case 0x48:
            INST_NAME("CMOVS Gd, Ed");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                CMPS_IMM8(2, 2, 1)
                , cNE, cEQ)
            break;
        case 0x49:
            INST_NAME("CMOVNS Gd, Ed");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                CMPS_IMM8(2, 2, 1)
                , cEQ, cNE)
            break;
        case 0x4A:
            INST_NAME("CMOVP Gd, Ed");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_PF]));
                CMPS_IMM8(2, 2, 1)
                , cNE, cEQ)
            break;
        case 0x4B:
            INST_NAME("CMOVNP Gd, Ed");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_PF]));
                CMPS_IMM8(2, 2, 1)
                , cEQ, cNE)
            break;
        case 0x4C:
            INST_NAME("CMOVL Gd, Ed");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(1, 0, offsetof(x86emu_t, flags[F_OF]));
                CMPS_REG_LSL_IMM8(1, 1, 2, 0)
                , cEQ, cNE)
            break;
        case 0x4D:
            INST_NAME("CMOVGE Gd, Ed");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(1, 0, offsetof(x86emu_t, flags[F_OF]));
                CMPS_REG_LSL_IMM8(1, 1, 2, 0)
                , cNE, cEQ)
            break;
        case 0x4E:
            INST_NAME("CMOVLE Gd, Ed");
            GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                LDR_IMM9(1, 0, offsetof(x86emu_t, flags[F_OF]));
                XOR_REG_LSL_IMM8(3, 1, 2, 0);
                LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_ZF]));
                ORR_REG_LSL_IMM8(2, 2, 3, 0);
                CMPS_IMM8(2, 2, 1);
                , cNE, cEQ)
            break;
        case 0x4F:
            INST_NAME("CMOVG Gd, Ed");
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

        case 0xA3:
            INST_NAME("BT Ed, Gd");
            nextop = F8;
            USEFLAG;
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = xEAX+(nextop&7);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed);
                if(wback>=xEAX) {ADD_REG_LSR_IMM8(1, ed, gd, 5); ed=1;}
                LDR_IMM9(1, ed, 0);
                ed = 1;
            }
            AND_IMM8(2, gd, 0x1f);
            MOV_REG_LSR_REG(1, ed, 2);
            AND_IMM8(1, 1, 1);
            STR_IMM9(1, 0, offsetof(x86emu_t, flags[F_CF]));
            UFLAGS(1);
            break;

        case 0xAF:
            INST_NAME("IMUL Gd, Ed");
            nextop = F8;
            GETGD;
            GETED;
            UFLAG_IF {
                SMULL(3, gd, ed, gd);
                UFLAG_OP1(3);
                UFLAG_RES(gd);
                UFLAG_DF(3, d_imul32);
            } else {
                MUL(gd, ed, gd);
            }
            UFLAGS(0);
            break;

        case 0xB6:
            INST_NAME("MOVZX Gd, Eb");
            nextop = F8;
            GETGD;
            if((nextop&0xC0)==0xC0) {
                ed = (nextop&7);
                eb1 = xEAX+(ed&3);  // Ax, Cx, Dx or Bx
                eb2 = (ed&4)>>2;    // L or H
                if(gd==eb1) {
                    if(eb2) {
                        MOV_REG_LSR_IMM5(gd, gd, 8);
                    }
                    AND_IMM8(gd, gd, 0xff);
                } else {
                    if(eb2) {
                        MOV_REG_LSR_IMM5(gd, eb1, 8);
                        AND_IMM8(gd, gd, 0xff);
                    } else {
                        AND_IMM8(gd, eb1, 0xff);
                    }
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed);
                LDRB_IMM9(gd, ed, 0);
            }
            break;
            
        default:
            *ok = 0;
            DEFAULT;
    }
    return addr;
}

