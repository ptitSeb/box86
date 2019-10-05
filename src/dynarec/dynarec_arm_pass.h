#ifndef STEP
#error Meh?!
#endif

#include "arm_emitter.h"
#include "../emu/x86primop.h"

#define F8      *(uint8_t*)(addr++)
#define F8S     *(int8_t*)(addr++)
#define F16     *(uint16_t*)(addr+=2, addr-2)
#define F32     *(uint32_t*)(addr+=4, addr-4)
#define F32S    *(int32_t*)(addr+=4, addr-4)
#define PK(a)   *(uint8_t*)(addr+a)
#define PKip(a)   *(uint8_t*)(ip+a)

void arm_epilog();
void* arm_linker(x86emu_t* emu, void** table, uintptr_t addr);

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
                MOV32(ret, tmp);
                if (sib_reg!=4) {
                    ADD_REG_LSL_IMM8(ret, ret, xEAX+sib_reg, (sib>>6));
                }
            } else {
                if (sib_reg!=4) {
                    ADD_REG_LSL_IMM8(ret, xEAX+(sib&0x7), xEAX+sib_reg, (sib>>6));
                } else {
                    //MOV_REG(2, xEAX+(sib&0x7));
                    ret = xEAX+(sib&0x7);
                }
            }
        } else if((nextop&7)==5) {
            uint32_t tmp = F32;
            MOV32(ret, tmp);
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
            int32_t i32 = (int32_t)t32;
            if(i32>0 && i32<255) {
                ADD_IMM8(ret, tmp, t32);
            } else if(i32<0 && i32>-256) {
                SUB_IMM8(ret, tmp, -t32);
            } else if(t32) {
                MOV32(3, t32);
                ADD_REG_LSL_IMM8(ret, tmp, 3, 0);
            } else
                ret = tmp;
        } else {
            int8_t t8 = F8S;
            if(t8<0) {
                SUB_IMM8(ret, tmp, -t8);
            } else if (t8>0) {
                ADD_IMM8(ret, tmp, t8);
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

static void jump_to_linker(dynarec_arm_t* dyn, uintptr_t ip, int reg, int ninst)
{
    MESSAGE(LOG_DUMP, "Jump to linker (#%d)\n", dyn->tablei);
    if(reg) {
        MOV_REG(xEIP, reg);
    } else {
        MOV32(xEIP, ip);
    }
    uintptr_t* table = 0;
    if(dyn->tablesz) {
        table = &dyn->table[dyn->tablei];
        *table = (uintptr_t)arm_linker;
    }
    ++dyn->tablei;
    MOV32_(1, (uintptr_t)table);
    LDR_IMM9(2, 1, 0);
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

static void retn_to_epilog(dynarec_arm_t* dyn, int ninst, int n)
{
    MESSAGE(LOG_DUMP, "Retn epilog\n");
    POP(xESP, 1<<xEIP);
    ADD_IMM8(xESP, xESP, n);
    void* epilog = arm_epilog;
    MOV32(2, (uintptr_t)epilog);
    BX(2);
}

static void arm_to_x86_flags(dynarec_arm_t* dyn, int ninst)
{
    MOVW_COND(cEQ, 1, 1);
    MOVW_COND(cNE, 1, 0);
    STR_IMM9(1, 0, offsetof(x86emu_t, flags[F_ZF]));
    MOVW_COND(cCS, 1, 1);
    MOVW_COND(cCC, 1, 0);
    STR_IMM9(1, 0, offsetof(x86emu_t, flags[F_CF]));
    MOVW_COND(cMI, 1, 1);
    MOVW_COND(cPL, 1, 0);
    STR_IMM9(1, 0, offsetof(x86emu_t, flags[F_SF]));
    MOVW_COND(cVS, 1, 1);
    MOVW_COND(cVC, 1, 0);
    STR_IMM9(1, 0, offsetof(x86emu_t, flags[F_OF]));
}

static void call_c(dynarec_arm_t* dyn, int ninst, void* fnc, int reg, int ret)
{
    MOV32(reg, (uintptr_t)fnc);
    PUSH(13, (1<<0));
    BLX(reg);
    if(ret>=0) {
        MOV_REG(ret, 0);
    }
    POP(13, (1<<0));
}

#define GETGD   gd = xEAX+((nextop&0x38)>>3)
#define GETED   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    wback = 0;              \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback); \
                    LDR_IMM9(1, wback, 0);  \
                    ed = 1;                 \
                }
#define WBACK   if(wback) {STR_IMM9(ed, wback, 0);}
#define GETEDO(O)   if((nextop&0xC0)==0xC0) {   \
                    ed = xEAX+(nextop&7);   \
                    wback = 0;              \
                } else {                    \
                    addr = geted(dyn, addr, ninst, nextop, &wback); \
                    LDR_REG_LSL_IMM5(1, wback, O, 0);  \
                    ed = 1;                 \
                }
#define WBACKO(O)   if(wback) {STR_REG_LSL_IMM5(ed, wback, O, 0);}
#define CALL(F, ret) call_c(dyn, ninst, F, 12, ret)
#define CALL_(F, ret) call_c(dyn, ninst, F, 3, ret)
#ifndef UFLAGS
#define UFLAGS(A)  dyn->cleanflags=A
#endif
#ifndef USEFLAG
#define USEFLAG   if(!dyn->cleanflags) {CALL(UpdateFlags, -1);}
#endif
#ifndef JUMP
#define JUMP(A) 
#endif
#define UFLAG_OP1(A) if(dyn->insts && dyn->insts[ninst].x86.flags) {STR_IMM9(A, 0, offsetof(x86emu_t, op1));}
#define UFLAG_OP2(A) if(dyn->insts && dyn->insts[ninst].x86.flags) {STR_IMM9(A, 0, offsetof(x86emu_t, op2));}
#define UFLAG_OP12(A1, A2) if(dyn->insts && dyn->insts[ninst].x86.flags) {STR_IMM9(A1, 0, offsetof(x86emu_t, op1));STR_IMM9(A2, 0, offsetof(x86emu_t, op2));}
#define UFLAG_RES(A) if(dyn->insts && dyn->insts[ninst].x86.flags) {STR_IMM9(A, 0, offsetof(x86emu_t, res));}
#define UFLAG_DF(r, A) if(dyn->insts && dyn->insts[ninst].x86.flags) {MOVW(r, A); STR_IMM9(r, 0, offsetof(x86emu_t, df));}
#define UFLAG_IF(A) if(dyn->insts && dyn->insts[ninst].x86.flags) {A}

static void grab_tlsdata(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int reg)
{
    MESSAGE(LOG_DUMP, "Get TLSData\n");
    CALL(GetGSBaseEmu, reg);
}

static uintptr_t dynarecGS(dynarec_arm_t* dyn, uintptr_t addr, int ninst, int* ok, int* need_epilog)
{
    uintptr_t ip = addr-1;
    uint8_t opcode = F8;
    uint8_t nextop;
    int32_t i32;
    uint8_t gd, ed, wback;
    switch(opcode) {
        case 0x33:
            grab_tlsdata(dyn, addr, ninst, 12);
            INST_NAME("GS:XOR Gd, Ed");
            nextop = F8;
            GETGD;
            GETEDO(12);
            XOR_REG_LSL_IMM8(gd, gd, ed, 0);
            UFLAG_RES(gd);
            UFLAG_DF(1, d_xor32);
            UFLAGS(0);
            break;

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
    uint8_t nextop, opcode;
    int ok = 1;
    int ninst = 0;
    uintptr_t ip = addr;
    uint8_t gd, ed;
    int8_t i8;
    int32_t i32, tmp;
    uint8_t u8;
    uint32_t u32;
    int need_epilog = 1;
    dyn->tablei = 0;
    uint8_t wback;
    INIT;
    while(ok) {
        ip = addr;
#ifdef HAVE_TRACE
        if(dyn->emu->dec && box86_dynarec_trace) {
            MESSAGE(LOG_DUMP, "TRACE ----\n");
            STM(0, (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<11));
            MOV32(1, ip);
            STR_IMM9(1, 0, offsetof(x86emu_t, ip));
            MOV32(2, 1);
            CALL(PrintTrace, -1);
            MESSAGE(LOG_DUMP, "----------\n");
        }
#endif
        opcode = F8;
        NEW_INST;
        if(dyn->insts && dyn->insts[ninst].x86.barrier) {
            dyn->cleanflags = 0;
        }
        switch(opcode) {

            case 0x01:
                INST_NAME("ADD Ed, Gd");
                nextop = F8;
                GETGD;
                GETED;
                UFLAG_OP12(gd, ed);
                ADD_REG_LSL_IMM8(ed, ed, gd, 0);
                WBACK;
                UFLAG_RES(ed);
                UFLAG_DF(1, d_add32);
                UFLAGS(0);
                break;

            case 0x03:
                INST_NAME("ADD Gd, Ed");
                nextop = F8;
                GETGD;
                GETED;
                UFLAG_OP12(ed, gd);
                ADD_REG_LSL_IMM8(gd, gd, ed, 0);
                UFLAG_RES(gd);
                UFLAG_DF(1, d_add32);
                UFLAGS(0);
                break;

            case 0x05:
                INST_NAME("ADD EAX, Id");
                i32 = F32S;
                MOV32(1, i32);
                UFLAG_OP12(1, xEAX);
                ADD_REG_LSL_IMM8(xEAX, xEAX, 1, 0);
                UFLAG_RES(xEAX);
                UFLAG_DF(1, d_add32);
                UFLAGS(0);
                break;

            case 0x08:
                INST_NAME("OR Ed, Gd");
                nextop = F8;
                GETGD;
                GETED;
                ORR_REG_LSL_IMM8(ed, ed, gd, 0);
                WBACK;
                UFLAG_RES(ed);
                UFLAG_DF(1, d_or32);
                UFLAGS(0);
                break;

            case 0x0B:
                INST_NAME("OR Gd, Ed");
                nextop = F8;
                GETGD;
                GETED;
                ORR_REG_LSL_IMM8(gd, gd, ed, 0);
                UFLAG_RES(gd);
                UFLAG_DF(1, d_or32);
                UFLAGS(0);
                break;

            case 0x0D:
                INST_NAME("OR EAX, Id");
                i32 = F32S;
                MOV32(1, i32);
                ORR_REG_LSL_IMM8(xEAX, xEAX, 1, 0);
                UFLAG_RES(xEAX);
                UFLAG_DF(1, d_or32);
                UFLAGS(0);
                break;

            case 0x21:
                INST_NAME("AND Ed, Gd");
                nextop = F8;
                GETGD;
                GETED;
                AND_REG_LSL_IMM8(ed, ed, gd, 0);
                WBACK;
                UFLAG_RES(ed);
                UFLAG_DF(1, d_and32);
                UFLAGS(0);
                break;

            case 0x23:
                INST_NAME("AND Gd, Ed");
                nextop = F8;
                GETGD;
                GETED;
                AND_REG_LSL_IMM8(gd, gd, ed, 0);
                UFLAG_RES(gd);
                UFLAG_DF(1, d_and32);
                UFLAGS(0);
                break;

            case 0x25:
                INST_NAME("AND EAX, Id");
                i32 = F32S;
                MOV32(1, i32);
                AND_REG_LSL_IMM8(xEAX, xEAX, 1, 0);
                UFLAG_RES(xEAX);
                UFLAG_DF(1, d_and32);
                UFLAGS(0);
                break;

            case 0x29:
                INST_NAME("SUB Ed, Gd");
                nextop = F8;
                GETGD;
                GETED;
                UFLAG_OP12(gd, ed);
                SUB_REG_LSL_IMM8(ed, ed, gd, 0);
                WBACK;
                UFLAG_RES(ed);
                UFLAG_DF(1, d_sub32);
                UFLAGS(0);
                break;

            case 0x2B:
                INST_NAME("SUB Gd, Ed");
                nextop = F8;
                GETGD;
                GETED;
                UFLAG_OP12(ed, gd);
                SUB_REG_LSL_IMM8(gd, gd, ed, 0);
                UFLAG_RES(gd);
                UFLAG_DF(1, d_sub32);
                UFLAGS(0);
                break;

            case 0x2D:
                INST_NAME("SUB EAX, Id");
                i32 = F32S;
                MOV32(1, i32);
                UFLAG_OP12(1, xEAX);
                SUB_REG_LSL_IMM8(xEAX, xEAX, 1, 0);
                UFLAG_RES(xEAX);
                UFLAG_DF(1, d_sub32);
                UFLAGS(0);
                break;

            case 0x2E:
                INST_NAME("CS:");
                // ignored
                break;

            case 0x31:
                INST_NAME("XOR Ed, Gd");
                nextop = F8;
                GETGD;
                GETED;
                XOR_REG_LSL_IMM8(ed, ed, gd, 0);
                WBACK;
                UFLAG_RES(ed);
                UFLAG_DF(1, d_xor32);
                UFLAGS(0);
                break;

            case 0x33:
                INST_NAME("XOR Gd, Ed");
                nextop = F8;
                GETGD;
                GETED;
                XOR_REG_LSL_IMM8(gd, gd, ed, 0);
                UFLAG_RES(gd);
                UFLAG_DF(1, d_xor32);
                UFLAGS(0);
                break;

            case 0x35:
                INST_NAME("XOR EAX, Id");
                i32 = F32S;
                MOV32(1, i32);
                XOR_REG_LSL_IMM8(xEAX, xEAX, 1, 0);
                UFLAG_RES(xEAX);
                UFLAG_DF(1, d_xor32);
                UFLAGS(0);
                break;

            case 0x39:
                INST_NAME("CMP Ed, Gd");
                nextop = F8;
                GETGD;
                GETED;
                if(ed!=1) {MOV_REG(1, ed);};
                MOV_REG(2, gd);
                CALL(cmp32, -1);
                UFLAGS(1);
                break;

            case 0x3B:
                INST_NAME("CMP Gd, Ed");
                nextop = F8;
                GETGD;
                GETED;
                if(ed!=2) {MOV_REG(2,ed);}
                MOV_REG(1, gd);
                CALL(cmp32, -1);
                UFLAGS(1);
                break;

            case 0x3D:
                INST_NAME("CMP EAX, Id");
                i32 = F32S;
                MOV32(2, i32);
                MOV_REG(1, xEAX);
                CALL(cmp32, -1);
                UFLAGS(1);
                break;

            case 0x40:
            case 0x41:
            case 0x42:
            case 0x43:
            case 0x44:
            case 0x45:
            case 0x46:
            case 0x47:
                INST_NAME("INC reg");
                gd = xEAX+(opcode&0x07);
                UFLAG_OP1(gd);
                ADD_IMM8(gd, gd, 1);
                UFLAG_RES(gd);
                UFLAG_DF(1, d_inc32);
                UFLAGS(0);
                break;
            case 0x48:
            case 0x49:
            case 0x4A:
            case 0x4B:
            case 0x4C:
            case 0x4D:
            case 0x4E:
            case 0x4F:
                INST_NAME("DEC reg");
                gd = xEAX+(opcode&0x07);
                UFLAG_OP1(gd);
                SUB_IMM8(gd, gd, 1);
                UFLAG_RES(gd);
                UFLAG_DF(1, d_dec32);
                UFLAGS(0);
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
                gd = xEAX+(opcode&0x07);
                PUSH(xESP, 1<<gd);
                break;
            case 0x58:
            case 0x59:
            case 0x5A:
            case 0x5B:
            case 0x5C:
            case 0x5D:
            case 0x5E:
            case 0x5F:
                INST_NAME("POP reg");
                gd = xEAX+(opcode&0x07);
                POP(xESP, 1<<gd);
                break;

            case 0x65:
                addr = dynarecGS(dyn, addr, ninst, &ok, &need_epilog);
                break;

            case 0x68:
                INST_NAME("PUSH Id");
                i32 = F32S;
                MOV32(3, i32);
                PUSH(xESP, 1<<3);
                break;

            case 0x6A:
                INST_NAME("PUSH Ib");
                i32 = F8S;
                MOV32(3, i32);
                PUSH(xESP, 1<<3);
                break;

            #define GO(GETFLAGS, NO, YES)   \
                i8 = F8S;   \
                USEFLAG;    \
                JUMP(addr+i8);\
                GETFLAGS;   \
                if(dyn->insts) {    \
                    if(dyn->insts[ninst].x86.jmp_insts==-1) {   \
                        /* out of the block */                  \
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8); \
                        Bcond(NO, i32);     \
                        jump_to_linker(dyn, addr+i8, 0, ninst); \
                    } else {    \
                        /* inside the block */  \
                        i32 = dyn->insts[dyn->insts[ninst].x86.jmp_insts].address-(dyn->arm_size+8);    \
                        Bcond(YES, i32);    \
                    }   \
                }

            case 0x70:
                INST_NAME("JO ib");
                GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_OF]));
                    CMPS_IMM8(2, 2, 1)
                    , cNE, cEQ)
                break;
            case 0x71:
                INST_NAME("JNO ib");
                GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_OF]));
                    CMPS_IMM8(2, 2, 1)
                    , cEQ, cNE)
                break;
            case 0x72:
                INST_NAME("JC ib");
                GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                    CMPS_IMM8(2, 2, 1)
                    , cNE, cEQ)
                break;
            case 0x73:
                INST_NAME("JNC ib");
                GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                    CMPS_IMM8(2, 2, 1)
                    , cEQ, cNE)
                break;
            case 0x74:
                INST_NAME("JZ ib");
                GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_ZF]));
                    CMPS_IMM8(2, 2, 1)
                    , cNE, cEQ)
                break;
            case 0x75:
                INST_NAME("JNZ ib");
                GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_ZF]));
                    CMPS_IMM8(2, 2, 1)
                    , cEQ, cNE)
                break;
            case 0x76:
                INST_NAME("JBE ib");
                GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                    LDR_IMM9(3, 0, offsetof(x86emu_t, flags[F_ZF]));
                    ORR_REG_LSL_IMM8(2, 2, 3, 0);
                    CMPS_IMM8(2, 2, 1)
                    , cNE, cEQ)
                break;
            case 0x77:
                INST_NAME("JNBE ib");
                GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_CF]));
                    LDR_IMM9(3, 0, offsetof(x86emu_t, flags[F_ZF]));
                    ORR_REG_LSL_IMM8(2, 2, 3, 0);
                    CMPS_IMM8(2, 2, 1)
                    , cEQ, cNE)
                break;
            case 0x78:
                INST_NAME("JS ib");
                GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                    CMPS_IMM8(2, 2, 1)
                    , cNE, cEQ)
                break;
            case 0x79:
                INST_NAME("JNS ib");
                GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                    CMPS_IMM8(2, 2, 1)
                    , cEQ, cNE)
                break;
            case 0x7A:
                INST_NAME("JP ib");
                GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_PF]));
                    CMPS_IMM8(2, 2, 1)
                    , cNE, cEQ)
                break;
            case 0x7B:
                INST_NAME("JNP ib");
                GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_PF]));
                    CMPS_IMM8(2, 2, 1)
                    , cEQ, cNE)
                break;
            case 0x7C:
                INST_NAME("JL ib");
                GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                    LDR_IMM9(1, 0, offsetof(x86emu_t, flags[F_OF]));
                    CMPS_REG_LSL_IMM8(1, 1, 2, 0)
                    , cEQ, cNE)
                break;
            case 0x7D:
                INST_NAME("JGE ib");
                GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                    LDR_IMM9(1, 0, offsetof(x86emu_t, flags[F_OF]));
                    CMPS_REG_LSL_IMM8(1, 1, 2, 0)
                    , cNE, cEQ)
                break;
            case 0x7E:
                INST_NAME("JLE ib");
                GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                    LDR_IMM9(1, 0, offsetof(x86emu_t, flags[F_OF]));
                    XOR_REG_LSL_IMM8(3, 1, 2, 0);
                    LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_ZF]));
                    ORR_REG_LSL_IMM8(2, 2, 3, 0);
                    CMPS_IMM8(2, 2, 1);
                    , cNE, cEQ)
                break;
            case 0x7F:
                INST_NAME("JG ib");
                GO( LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_SF]));
                    LDR_IMM9(1, 0, offsetof(x86emu_t, flags[F_OF]));
                    XOR_REG_LSL_IMM8(3, 1, 2, 0);
                    LDR_IMM9(2, 0, offsetof(x86emu_t, flags[F_ZF]));
                    XOR_IMM8(3, 3, 1);
                    XOR_IMM8(2, 2, 1);
                    TSTS_REG_LSL_IMM8(2, 2, 3, 0);
                    , cEQ, cNE)
                break;
            
            case 0x81:
            case 0x83:
                nextop = F8;
                switch((nextop>>3)&7) {
                    case 0: //ADD
                        if(opcode==0x81) {
                            INST_NAME("ADD Ed, Id");
                        } else {
                            INST_NAME("ADD Ed, Ib");
                        }
                        GETED;
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        UFLAG_OP2(ed);
                        if(i32>=0 && i32<256) {
                            UFLAG_IF(MOV32(3, i32); UFLAG_OP1(3););
                            ADD_IMM8(ed, ed, i32);
                        } else {
                            MOV32(3, i32);
                            UFLAG_OP1(3);
                            ADD_REG_LSL_IMM8(ed, ed, 3, 0);
                        }
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(3, d_add32);
                        UFLAGS(0);
                        break;
                    case 4: //AND
                        if(opcode==0x81) {INST_NAME("AND Ed, Id");} else {INST_NAME("AND Ed, Ib");}
                        GETED;
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        if(i32>0 && i32<256) {
                            AND_IMM8(ed, ed, i32);
                        } else {
                            MOV32(3, i32);
                            AND_REG_LSL_IMM8(ed, ed, 3, 0);
                        }
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(3, d_and32);
                        UFLAGS(0);
                        break;
                    case 5: //SUB
                        if(opcode==0x81) {INST_NAME("SUB Ed, Id");} else {INST_NAME("SUB Ed, Ib");}
                        GETED;
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        UFLAG_OP2(ed);
                        if(i32>0 && i32<256) {
                            UFLAG_IF(MOV32(3, i32); UFLAG_OP1(3);)
                            SUB_IMM8(ed, ed, i32);
                        } else {
                            MOV32(3, i32);
                            UFLAG_OP1(3);
                            SUB_REG_LSL_IMM8(ed, ed, 3, 0);
                        }
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(3, d_sub32);
                        UFLAGS(0);
                        break;
                    case 6: //XOR
                        if(opcode==0x81) {INST_NAME("XOR Ed, Id");} else {INST_NAME("XOR Ed, Ib");}
                        GETED;
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        if(i32>0 && i32<256) {
                            XOR_IMM8(ed, ed, i32);
                        } else {
                            MOV32(3, i32);
                            XOR_REG_LSL_IMM8(ed, ed, 3, 0);
                        }
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(3, d_xor32);
                        UFLAGS(0);
                        break;
                    case 7: //CMP
                        if(opcode==0x81) {INST_NAME("CMP Ed, Id");} else {INST_NAME("CMP Ed, Ib");}
                        GETED;
                        if(opcode==0x81) i32 = F32S; else i32 = F8S;
                        if(ed!=1) {MOV_REG(1,ed);}
                        MOV32(2, i32);
                        CALL(cmp32, -1);
                        UFLAGS(1);
                        break;
                    default:
                        if(opcode==0x81) {INST_NAME("GRP1 Ed, Id");} else {INST_NAME("GRP1 Ed, Ib");}
                        ok = 0;
                        DEFAULT;
                }
                break;
            
            case 0x85:
                INST_NAME("TEST Ed, Gd");
                nextop=F8;
                GETGD;
                GETED;
                if(ed!=1) {MOV_REG(1, ed);};
                MOV_REG(2, gd);
                CALL(test32, -1);
                UFLAGS(1);
                break;

            case 0x89:
                INST_NAME("MOV Ed, Gd");
                nextop=F8;
                GETGD;
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
                GETGD;
                if((nextop&0xC0)==0xC0) {   // reg <= reg
                    MOV_REG(gd, xEAX+(nextop&7));
                } else {                    // mem <= reg
                    addr = geted(dyn, addr, ninst, nextop, &ed);
                    LDR_IMM9(gd, ed, 0);
                }
                break;

            case 0x8D:
                INST_NAME("LEA Gd, Ed");
                nextop=F8;
                GETGD;
                if((nextop&0xC0)==0xC0) {   // reg <= reg? that's an invalid operation
                    ok=0;
                    DEFAULT;
                } else {                    // mem <= reg
                    addr = geted(dyn, addr, ninst, nextop, &ed);
                    MOV_REG(gd, ed);
                }
                break;

            case 0x90:
                INST_NAME("NOP");
                break;

            case 0xB8:
            case 0xB9:
            case 0xBA:
            case 0xBB:
            case 0xBC:
            case 0xBD:
            case 0xBE:
            case 0xBF:
                INST_NAME("MOV Reg, Id");
                gd = xEAX+(opcode&7);
                i32 = F32S;
                MOV32(gd, i32);
                break;

            case 0xC1:
                nextop = F8;
                switch((nextop>>3)&7) {
                    case 7:
                        INST_NAME("SAR Ed, Id");
                        GETED;
                        u8 = (F8)&0x1f;
                        UFLAG_IF(MOV32(12, u8); UFLAG_OP2(12));
                        UFLAG_OP1(ed);
                        MOV_REG_ASR_IMM5(ed, ed, u8);
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(3, d_sar32);
                        UFLAGS(0);
                        break;
                    default:
                        INST_NAME("GRP3 Ed, Id");
                        ok = 0;
                        DEFAULT;
                }
                break;

            case 0xC2:
                INST_NAME("RETN");
                i32 = F16;
                retn_to_epilog(dyn, ninst, i32);
                need_epilog = 0;
                ok = 0;
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
                    addr = geted(dyn, addr, ninst, nextop, &ed);
                    i32 = F32S;
                    MOV32(3, i32);
                    STR_IMM9(3, ed, 0);
                }
                break;

            case 0xCC:
                if(PK(0)=='S' && PK(1)=='C') {
                    addr+=2;
                    UFLAG_DF(1, d_none);
                    UFLAGS(1);  // cheating...
                    INST_NAME("Special Box86 instruction");
                    if((PK(0)==0) && (PK(1)==0) && (PK(2)==0) && (PK(3)==0))
                    {
                        addr+=4;
                        MESSAGE(LOG_DEBUG, "Exit x86 Emu\n");
                        MOV32(12, ip+1+2);
                        STM(0, (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12));
                        MOV32(1, 1);
                        STR_IMM9(1, 0, offsetof(x86emu_t, quit));
                        ok = 0;
                        need_epilog = 1;
                    } else {
                        MOV32(12, ip+1); // read the 0xCC
                        addr+=4+4;
                        STM(0, (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12));
                        CALL_(x86Int3, -1);
                        LDM(0, (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<11)|(1<<12));
                        LDR_IMM9(1, 0, offsetof(x86emu_t, quit));
                        CMPS_IMM8(1, 1, 1);
                        i32 = dyn->insts[ninst+1].address-(dyn->arm_size+8);
                        Bcond(cNE, i32);
                        jump_to_epilog(dyn, 0, 12, ninst);
                    }
                } else {
                    INST_NAME("INT 3");
                    ok = 0;
                    DEFAULT;
                }
                break;

            case 0xE8:
                INST_NAME("CALL Id");
                i32 = F32S;
                MOV32(2, addr);
                PUSH(xESP, 1<<2);
                jump_to_linker(dyn, addr+i32, 0, ninst);
                need_epilog = 0;
                ok = 0;
                break;
            case 0xE9:
            case 0xEB:
                if(opcode==0xE9) {
                    INST_NAME("JMP Id");
                    i32 = F32S;
                } else {
                    INST_NAME("JMP Ib");
                    i32 = F8S;
                }
                JUMP(addr+i32);
                if(dyn->insts) {
                    if(dyn->insts[ninst].x86.jmp_insts==-1) {
                        // out of the block
                        jump_to_linker(dyn, addr+i32, 0, ninst);
                    } else {
                        // inside the block
                        tmp = dyn->insts[dyn->insts[ninst].x86.jmp_insts].address-(dyn->arm_size+8);
                        Bcond(c__, tmp);
                    }
                }
                need_epilog = 0;
                ok = 0;
                break;

            case 0xFF:
                nextop = F8;
                switch((nextop>>3)&7) {
                    case 0: // INC Ed
                        INST_NAME("INC Ed");
                        GETED;
                        UFLAG_OP1(ed);
                        ADD_IMM8(ed, ed, 1);
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(1, d_inc32);
                        UFLAGS(0);
                        break;
                    case 1: //DEC Ed
                        INST_NAME("DEC Ed");
                        GETED;
                        UFLAG_OP1(ed);
                        SUB_IMM8(ed, ed, 1);
                        WBACK;
                        UFLAG_RES(ed);
                        UFLAG_DF(1, d_dec32);
                        UFLAGS(0);
                        break;
                    case 2: // CALL Ed
                        INST_NAME("CALL Ed");
                        GETED;
                        MOV32(3, addr);
                        PUSH(xESP, 1<<3);
                        jump_to_epilog(dyn, 0, ed, ninst);  // it's variable, so no linker
                        need_epilog = 0;
                        ok = 0;
                        break;
                    case 4: // JMP Ed
                        INST_NAME("JMP Ed");
                        GETED;
                        jump_to_epilog(dyn, 0, ed, ninst);     // it's variable, so no linker
                        need_epilog = 0;
                        ok = 0;
                        break;
                    case 6: // Push Ed
                        INST_NAME("PUSH Ed");
                        GETED;
                        PUSH(xESP, 1<<ed);
                        break;

                    default:
                        INST_NAME("Grp5 Ed");
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
        jump_to_epilog(dyn, ip, 0, ninst);  // no linker here, it's an unknow instruction
    FINI;
}