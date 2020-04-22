#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "arm_printer.h"

// conditions
#define cEQ (0b0000<<28)
#define cNE (0b0001<<28)
#define cCS (0b0010<<28)
#define cHS cCS
#define cCC (0b0011<<28)
#define cLO cCC
#define cMI (0b0100<<28)
#define cPL (0b0101<<28)
#define cVS (0b0110<<28)
#define cVC (0b0111<<28)
#define cHI (0b1000<<28)
#define cLS (0b1001<<28)
#define cGE (0b1010<<28)
#define cLT (0b1011<<28)
#define cGT (0b1100<<28)
#define cLE (0b1101<<28)
#define c__ (0b1110<<28)    // means all

static const char* conds[] = {
    "EQ", "NE", "CS", "CC", "MI", "PL",
    "VS", "VC", "HI", "LS", "GE", "LT",
    "GT", "LE", ""
};

static const char* opcodes[] = {
    "AND", "EOR", "SUB", "RSB", "ADD", "ADC",
    "SBC", "RSC", "TST", "TEQ", "CMP", "CMN",
    "ORR", "MOV", "BIC", "MVN"
};

static const char* blocktransfert_other[] = {
    "STMDA", "STMIA", "STMDB", "STMIB",
    "LDMDA", "LDMIA", "LDMDB", "LDMIB"
};
static const char* blocktransfert_stack[] = {
    "STMED", "STMEA", "STMFD", "STMFA",
    "LDMFA", "LDMFD", "LDMEA", "LDMED"
};
static const char* regname[] = {
    "r0", "r1", "r2", "r3", "r4",
    "r5", "r6", "r7", "r8", "r9",
    "r10", "r11", "r12", "sp", "lr", "pc"
};

static const char* shift_type[] = {
    "lsl", "lsr", "asr", "ror"
};

const char* print_shift(int shift, int comma) {
    static char ret[20];
    memset(ret, 0, sizeof(ret));
    if (shift) {
        int sh_op = (shift>>1)&3;
        if(shift&1) {
            int rs = shift>>4;
            sprintf(ret, "%s%s %s", comma?",":"", shift_type[sh_op], regname[rs]);
        } else {
            uint8_t amount = shift>>3;
            sprintf(ret, "%s%s #%u", comma?",":"", shift_type[sh_op], amount);
        }
    }
    return ret;    
}

//  nop
#define NOP     (0xe1a00000)

const char* arm_print(uint32_t opcode)
{
    static __thread char ret[128];
    memset(ret, 0, sizeof(ret));
    if((opcode & (0b1111<<28))==(0b1111<<28)) {
        // NEON?
        if(((opcode>>24)&0b1111)==0b0100 && ((opcode>>20)&0b1001)==0b0000)
        {
            // VLD1 / VST1
            int D = (opcode>>22)&1;
            int Rn = (opcode>>16)&15;
            int Vd = (opcode>>12)&15;
            int type = (opcode>>8)&15;
            int size = (opcode>>6)&3;
            int align = (opcode>>4)&3;
            int l = (opcode>>21)&1;
            int Rm = opcode&15;
            int regs = 0;
            int Dd = (D<<4) | Vd;
            char reglist[50] = {0};
            switch (type) {
                case 0b0111: regs = 1; sprintf(reglist, "D%d", Dd); break;
                case 0b1010: regs = 2; sprintf(reglist, "D%d, D%d", Dd, Dd+1); break;
                case 0b0110: regs = 3; sprintf(reglist, "D%d, D%d, D%d", Dd, Dd+1, Dd+2); break;
                case 0b0010: regs = 4; sprintf(reglist, "D%d, D%d, D%d, D%d", Dd, Dd+1, Dd+2, Dd+3); break;
            }
            char salign[10] = {0};
            if (align) sprintf(salign, ":%d", 4<<align);
            char wback[10] = {0};
            if(Rm!=15) {
                if(Rm==13) strcpy(wback, "!");
                else sprintf(wback, ", %s", regname[Rm]);
            }
            if (!regs)
                strcpy(ret, "VLD1?????");
            else
                sprintf(ret, "V%s1%s.%d {%s}, [%s%s]%s", l?"LD":"ST", (regs>2)?"q":"", 8<<size, reglist, regname[Rn], salign, wback);
        } else
        if(((opcode>>24)&0b1111)==0b0011 && ((opcode>>20)&0b1011)==0b0000 && ((opcode>>8)&0b1111)==0b0001 && ((opcode>>4)&1)==1)
        {
            // VEOR
            int D = (opcode>>22)&1;
            int Vn = (opcode>>16)&15;
            int Vd = (opcode>>12)&15;
            int N = (opcode>>7)&1;
            int Q = (opcode>>6)&1;
            int M = (opcode>>5)&1;
            int Vm = (opcode)&15;
            int Dd = (D<<4) | Vd;
            int Dn = (N<<4) | Vn;
            int Dm = (M<<4) | Vm;
            if (Q)
                sprintf(ret, "VEORQ Q%d, Q%d, Q%d", Dd/2, Dn/2, Dm/2);
            else
                sprintf(ret, "VEOR D%d, D%d, D%d", Dd, Dn, Dm);
        } else
        if(((opcode>>24)&0b1111)==0b0010 && ((opcode>>20)&0b1000)==0b0000 && ((opcode>>8)&0b1111)==0b1000 && ((opcode>>4)&1)==0)
        {
            // VADD
            int D = (opcode>>22)&1;
            int Vn = (opcode>>16)&15;
            int Vd = (opcode>>12)&15;
            int N = (opcode>>7)&1;
            int Q = (opcode>>6)&1;
            int M = (opcode>>5)&1;
            int Vm = (opcode)&15;
            int size = (opcode>>20)&3;
            int Dd = (D<<4) | Vd;
            int Dn = (N<<4) | Vn;
            int Dm = (M<<4) | Vm;
            if (Q)
                sprintf(ret, "VADDQ.%d Q%d, Q%d, Q%d", 8<<size, Dd/2, Dn/2, Dm/2);
            else
                sprintf(ret, "VADD.%d D%d, D%d, D%d", 8<<size, Dd, Dn, Dm);
        } else
        if(((opcode>>24)&0b1111)==0b0011 && ((opcode>>20)&0b1000)==0b0000 && ((opcode>>8)&0b1111)==0b1000 && ((opcode>>4)&1)==1)
        {
            // VCEQ
            int D = (opcode>>22)&1;
            int Vn = (opcode>>16)&15;
            int Vd = (opcode>>12)&15;
            int N = (opcode>>7)&1;
            int Q = (opcode>>6)&1;
            int M = (opcode>>5)&1;
            int Vm = (opcode)&15;
            int size = (opcode>>20)&3;
            int Dd = (D<<4) | Vd;
            int Dn = (N<<4) | Vn;
            int Dm = (M<<4) | Vm;
            if (Q)
                sprintf(ret, "VCEQQ.%d Q%d, Q%d, Q%d", 8<<size, Dd/2, Dn/2, Dm/2);
            else
                sprintf(ret, "VCEQ.%d D%d, D%d, D%d", 8<<size, Dd, Dn, Dm);
        } else
        if(((opcode>>24)&0b1111)==0b0010 && ((opcode>>20)&0b1011)==0b0010 && ((opcode>>8)&0b1111)==0b0001 && ((opcode>>4)&1)==1)
        {
            // VORR / VMOV (simd)
            int D = (opcode>>22)&1;
            int Vn = (opcode>>16)&15;
            int Vd = (opcode>>12)&15;
            int N = (opcode>>7)&1;
            int Q = (opcode>>6)&1;
            int M = (opcode>>5)&1;
            int Vm = (opcode)&15;
            int Dd = (D<<4) | Vd;
            int Dn = (N<<4) | Vn;
            int Dm = (M<<4) | Vm;
            if(Vn!=Vm) {
                if (Q)
                    sprintf(ret, "VORRQ Q%d, Q%d, Q%d", Dd/2, Dn/2, Dm/2);
                else
                    sprintf(ret, "VORR D%d, D%d, D%d", Dd, Dn, Dm);
            } else {
                if (Q)
                    sprintf(ret, "VMOVQ Q%d, Q%d", Dd/2, Dm/2);
                else
                    sprintf(ret, "VMOV D%d, D%d", Dd, Dm);
            }
        } else
        if(((opcode>>24)&0b1111)==0b0010 && ((opcode>>20)&0b1011)==0b0000 && ((opcode>>8)&0b1111)==0b0001 && ((opcode>>4)&1)==1)
        {
            // VAND
            int D = (opcode>>22)&1;
            int Vn = (opcode>>16)&15;
            int Vd = (opcode>>12)&15;
            int N = (opcode>>7)&1;
            int Q = (opcode>>6)&1;
            int M = (opcode>>5)&1;
            int Vm = (opcode)&15;
            int Dd = (D<<4) | Vd;
            int Dn = (N<<4) | Vn;
            int Dm = (M<<4) | Vm;
            if (Q)
                sprintf(ret, "VANDQ Q%d, Q%d, Q%d", Dd/2, Dn/2, Dm/2);
            else
                sprintf(ret, "VAND D%d, D%d, D%d", Dd, Dn, Dm);
        } else
        if(((opcode>>24)&0b1111)==0b0010 && ((opcode>>20)&0b1011)==0b0001 && ((opcode>>8)&0b1111)==0b0001 && ((opcode>>4)&1)==1)
        {
            // VBIC
            int D = (opcode>>22)&1;
            int Vn = (opcode>>16)&15;
            int Vd = (opcode>>12)&15;
            int N = (opcode>>7)&1;
            int Q = (opcode>>6)&1;
            int M = (opcode>>5)&1;
            int Vm = (opcode)&15;
            int Dd = (D<<4) | Vd;
            int Dn = (N<<4) | Vn;
            int Dm = (M<<4) | Vm;
            if (Q)
                sprintf(ret, "VBICQ Q%d, Q%d, Q%d", Dd/2, Dn/2, Dm/2);
            else
                sprintf(ret, "VBIC D%d, D%d, D%d", Dd, Dn, Dm);
        } else
        if(((opcode>>24)&0b1111)==0b0100 && ((opcode>>20)&0b1011)==0b1000 && ((opcode>>8)&0b0011)==0b0000)
        {
            // VST1LANE
            int D = (opcode>>22)&1;
            int Rn = (opcode>>16)&15;
            int Vd = (opcode>>12)&15;
            int size = (opcode>>10)&3;
            int index_align = (opcode>>4)&15;
            int Rm = (opcode)&15;
            int Dd = (D<<4) | Vd;
            int rsize = 32;
            char ralign[30] = {0};
            int index = 0;
            int alignment = 1;
            switch(size) {
                case 0:
                    rsize = 8;
                    index = (index_align>>1)&7;
                    break;
                case 1:
                    rsize = 16;
                    index = (index_align>>2)&3;
                    alignment += (index_align&1);
                    break;
                case 2:
                    rsize = 32;
                    index = (index_align>>3)&1;
                    if((index_align&3)!=0)
                        alignment = 4;
                    break;
            }
            if(alignment!=1)
                sprintf(ret, ":%d", 4<<alignment);
            char post[40] = {0};
            if(Rm!=15) {
                if (Rm==13)
                    strcpy(post, "!");
                else
                    sprintf(post, ", %s", regname[Rm]);
            }
            sprintf(ret, "VST1.%d {D%d[%d]}, [%s%s]%s", rsize, Dd, index, regname[Rn], ralign, post);
        } else
        if(((opcode>>24)&0b1111)==0b0100 && ((opcode>>20)&0b1011)==0b1010 && ((opcode>>8)&0b0011)==0b0000)
        {
            // VLD1LANE
            int D = (opcode>>22)&1;
            int Rn = (opcode>>16)&15;
            int Vd = (opcode>>12)&15;
            int size = (opcode>>10)&3;
            int index_align = (opcode>>4)&15;
            int Rm = (opcode)&15;
            int Dd = (D<<4) | Vd;
            int rsize = 32;
            char ralign[30] = {0};
            int index = 0;
            int T = 0;
            int alignment = 1;
            char list[40] = {0};
            switch(size) {
                case 0:
                    rsize = 8;
                    index = (index_align>>1)&7;
                    sprintf(list, "{D%d[%d]}", Dd, index);
                    break;
                case 1:
                    rsize = 16;
                    index = (index_align>>2)&3;
                    alignment += (index_align&1);
                    sprintf(list, "{D%d[%d]}", Dd, index);
                    break;
                case 2:
                    rsize = 32;
                    index = (index_align>>3)&1;
                    if((index_align&3)!=0)
                        alignment = 4;
                    sprintf(list, "{D%d[%d]}", Dd, index);
                    break;
                case 3:
                    rsize = 8 <<((index_align>>2)&3);
                    index = -1;
                    T = (index_align>>1)&1;
                    if(index_align&1)
                        alignment = (index_align>>2)&3;
                    if(T)
                        sprintf(list, "{D%d[], D%d[]}", Dd, Dd+1);
                    else
                        sprintf(list, "{D%d[]}", Dd);
                    break;
            }
            if(alignment!=1)
                sprintf(ret, ":%d", 4<<alignment);
            char post[40] = {0};
            if(Rm!=15) {
                if (Rm==13)
                    strcpy(post, "!");
                else
                    sprintf(post, ", %s", regname[Rm]);
            }
            sprintf(ret, "VLD1.%d %s, [%s%s]%s", rsize, list, regname[Rn], ralign, post);
        } else
        if(((opcode>>24)&0b1110)==0b0010 && ((opcode>>20)&0b1000)==0b1000 && ((opcode>>16)&0b0111)==0b0000 && ((opcode>>8)&0b1111)==0b1010 && ((opcode>>4)&1)==1)
        {
            // VMOVL
            int D = (opcode>>22)&1;
            int U = (opcode>>24)&1;
            int imm3 = (opcode>>19)&7;
            int Vd = (opcode>>12)&15;
            int M = (opcode>>5)&1;
            int Vm = (opcode)&15;
            int Dd = (D<<4) | Vd;
            int Dm = (M<<4) | Vm;
            sprintf(ret, "VMOVL.%s%d Q%d, D%d", U?"U":"S", 4<<imm3,Dd/2, Dm);
        } else
        if(((opcode>>24)&0b1111)==0b0010 && ((opcode>>20)&0b1011)==0b0000 && ((opcode>>8)&0b1111)==0b1101 && ((opcode>>4)&1)==0)
        {
            // VADD_F
            int D = (opcode>>22)&1;
            int Vn = (opcode>>16)&15;
            int Vd = (opcode>>12)&15;
            int N = (opcode>>7)&1;
            int Q = (opcode>>6)&1;
            int M = (opcode>>5)&1;
            int Vm = (opcode)&15;
            int Dd = (D<<4) | Vd;
            int Dn = (N<<4) | Vn;
            int Dm = (M<<4) | Vm;
            if (Q)
                sprintf(ret, "VADD.F32 Q%d, Q%d, Q%d", Dd/2, Dn/2, Dm/2);
            else
                sprintf(ret, "VADD.F32 D%d, D%d, D%d", Dd, Dn, Dm);
        } else
        if(((opcode>>24)&0b1111)==0b0010 && ((opcode>>20)&0b1011)==0b0010 && ((opcode>>8)&0b1111)==0b1101 && ((opcode>>4)&1)==0)
        {
            // VSUB_F
            int D = (opcode>>22)&1;
            int Vn = (opcode>>16)&15;
            int Vd = (opcode>>12)&15;
            int N = (opcode>>7)&1;
            int Q = (opcode>>6)&1;
            int M = (opcode>>5)&1;
            int Vm = (opcode)&15;
            int Dd = (D<<4) | Vd;
            int Dn = (N<<4) | Vn;
            int Dm = (M<<4) | Vm;
            if (Q)
                sprintf(ret, "VSUB.F32 Q%d, Q%d, Q%d", Dd/2, Dn/2, Dm/2);
            else
                sprintf(ret, "VSUB.F32 D%d, D%d, D%d", Dd, Dn, Dm);
        } else
        if(((opcode>>24)&0b1111)==0b0011 && ((opcode>>20)&0b1011)==0b0000 && ((opcode>>8)&0b1111)==0b1101 && ((opcode>>4)&1)==1)
        {
            // VMUL_F
            int D = (opcode>>22)&1;
            int Vn = (opcode>>16)&15;
            int Vd = (opcode>>12)&15;
            int N = (opcode>>7)&1;
            int Q = (opcode>>6)&1;
            int M = (opcode>>5)&1;
            int Vm = (opcode)&15;
            int Dd = (D<<4) | Vd;
            int Dn = (N<<4) | Vn;
            int Dm = (M<<4) | Vm;
            if (Q)
                sprintf(ret, "VMUL.F32 Q%d, Q%d, Q%d", Dd/2, Dn/2, Dm/2);
            else
                sprintf(ret, "VMUL.F32 D%d, D%d, D%d", Dd, Dn, Dm);
        } else
        if(((opcode>>24)&0b1111)==0b0011 && ((opcode>>20)&0b1011)==0b0010 && ((opcode>>8)&0b1111)==0b1110 && ((opcode>>4)&1)==0)
        {
            // VCGT.F32
            int D = (opcode>>22)&1;
            int Vn = (opcode>>16)&15;
            int Vd = (opcode>>12)&15;
            int N = (opcode>>7)&1;
            int Q = (opcode>>6)&1;
            int M = (opcode>>5)&1;
            int Vm = (opcode)&15;
            int Dd = (D<<4) | Vd;
            int Dn = (N<<4) | Vn;
            int Dm = (M<<4) | Vm;
            if (Q)
                sprintf(ret, "VCGT.F32 Q%d, Q%d, Q%d", Dd/2, Dn/2, Dm/2);
            else
                sprintf(ret, "VCGT.F32 D%d, D%d, D%d", Dd, Dn, Dm);
        } else
        strcpy(ret, "?????");
    } else {
        const char* cond = conds[(opcode>>28)&15];
        if((opcode&0b00001111111111111111111111010000)==0b00000001001011111111111100010000) {
            int l = (opcode>>5)&1;
            sprintf(ret, "B%sX%s r%d", l?"L":"", cond, opcode&0b1111);
        } else if (((opcode>>25)&0b111)==0b101) {
            int32_t offset = opcode&0xffffff;
            if(opcode&0x800000) offset |= 0xff000000;
            sprintf(ret, "B%s%s %+d", cond, (opcode&(1<<24))?"L":"", offset+2);
        } else if((opcode&0b00001111110000000000000010010000)==0b00000000000000000000000010010000) {
            int a = (opcode>>21)&1;
            int s = (opcode>>20)&1;
            int rd = (opcode>>16)&15;
            int rn = (opcode>>12)&15;
            int rs = (opcode>>8)&15;
            int rm = (opcode)&15;
            if(a) {
                sprintf(ret, "MLA%s%s %s, %s, %s, %s", cond, s?"S":"", regname[rd], regname[rm], regname[rs], regname[rn]);
            } else {
                sprintf(ret, "MUL%s%s %s, %s, %s", cond, s?"S":"", regname[rd], regname[rm], regname[rs]);
            }
        } else if((opcode&0b00001111100000000000000010010000)==0b00000000100000000000000010010000) {
            int u = (opcode>>22)&1;
            int a = (opcode>>21)&1;
            int s = (opcode>>20)&1;
            int rdhi = (opcode>>16)&15;
            int rdlo = (opcode>>12)&15;
            int rs = (opcode>>8)&15;
            int rm = (opcode)&15;
            if(a) {
                sprintf(ret, "%sMLAL%s%s %s, %s, %s, %s", u?"S":"U", cond, s?"S":"", regname[rdlo], regname[rdhi], regname[rm], regname[rs]);
            } else {
                sprintf(ret, "%sMULL%s%s %s, %s, %s, %s", u?"S":"U", cond, s?"S":"", regname[rdlo], regname[rdhi], regname[rm], regname[rs]);
            }
        } else if((opcode&0b00001111101100000000111111110000)==0b00000001000000000000000010010000) {
            int b = (opcode>>22)&1;
            int rn = (opcode>>16)&15;
            int rd = (opcode>>12)&15;
            int rm = (opcode)&15;
            sprintf(ret, "SWP%s%s r%d, r%d, [r%d]", cond, b?"B":"", rd, rm, rn);
        } else if((opcode&0b000011111111111110000111111110000)==0b00000110101111110000111100110000) {
            int rd = (opcode>>12)&15;
            int rm = (opcode)&15;
            sprintf(ret, "BSWAP%s %s, %s", cond, regname[rd], regname[rm]);
        } else if((opcode&0b00001111111100000000000000000000)==0b00000011000000000000000000000000) {
            uint16_t imm16 = (opcode&0x0fff) | ((opcode>>4)&0xf000);
            int rn = (opcode>>12)&15;
            sprintf(ret, "MOVW%s %s, #0x%x", cond, regname[rn], imm16);
        } else if((opcode&0b00001111111100000000000000000000)==0b00000011010000000000000000000000) {
            uint16_t imm16 = (opcode&0x0fff) | ((opcode>>4)&0xf000);
            int rn = (opcode>>12)&15;
            sprintf(ret, "MOVT%s %s, #0x%x", cond, regname[rn], imm16);
        } else
        {
        uint32_t cat = (opcode>>25)&0b111;
            switch (cat) {
                case 0b000:
                    // many things are in here, but Branches are already printed, so only exchange and hlf data tranfert are left
                    if(((opcode>>5)&3)!=0 && ((opcode>>4)&0b1001)==0b1001) {
                        int p = (opcode>>24)&1;
                        int u = (opcode>>23)&1;
                        int o = (opcode>>22)&1;
                        int w = (opcode>>21)&1;
                        int l = (opcode>>20)&1;
                        int rn = (opcode>>16)&15;
                        int rd = (opcode>>12)&15;
                        int offset = ((opcode>>4)&0xf0) | (opcode&0xf);
                        int rm = opcode&15;
                        int sh = (opcode>>5)&3;
                        const char* shs[] = {"swp", "H", "SB", "SH"};
                        char op2[40];
                        if(o) {
                            sprintf(op2, "#%d", u?offset:-offset);
                        } else {
                            sprintf(op2, "%s%s", u?"":"-", regname[rm]);
                        }
                        char addr[50];
                        if(p) { // pre-index
                            if(o && !offset) {
                                sprintf(addr, "[%s]", regname[rn]);
                            } else {
                                sprintf(addr, "[%s, %s]%s", regname[rn], op2, w?"!":"");
                            }
                        } else {
                            sprintf(addr, "[%s], %s", regname[rn], op2);
                        }
                        sprintf(ret, "%s%s%s%s %s, %s", l?"LDR":"STR", cond, shs[sh], (w && p)?"T":"", regname[rd], addr);
                        break;
                    } else
                    if((((opcode>>25)&0b111)==0b000) && (((opcode>>20)&1)==0) && ((opcode>>4)&0b1101)==0b1101)
                    {
                        //STRD / LDRD
                        int p = (opcode>>24)&1;
                        int u = (opcode>>23)&1;
                        int o = (opcode>>22)&1;
                        int w = (opcode>>21)&1;
                        int rn = (opcode>>16)&15;
                        int rt = (opcode>>12)&15;
                        int imm4H = (opcode>>8)&15;
                        int imm4L = opcode&15;
                        int rm = opcode&15;
                        int s = (opcode>>5)&1;
                        int imm8 = (imm4H<<4) | imm4L;
                        char op2[40];
                        if(o) {
                            sprintf(op2, "#%d", u?imm8:-imm8);
                        } else {
                            sprintf(op2, "%s%s", u?"":"-", regname[rm]);
                        }
                        char addr[50];
                        if(p) { // pre-index
                            if(o && !imm8) {
                                sprintf(addr, "[%s]", regname[rn]);
                            } else {
                                sprintf(addr, "[%s, %s]%s", regname[rn], op2, w?"!":"");
                            }
                        } else {
                            sprintf(addr, "[%s], %s", regname[rn], op2);
                        }
                        sprintf(ret, "%s%s%s %s, %s, %s", (rt&1)?"!!":"", s?"STRD":"LDRD", cond, regname[rt], regname[rt+1], addr);
                    }
                    break;

                case 0b001:
                     // data operation
                    {
                        int i = (opcode>>25)&1;
                        int s = (opcode>>20)&1;
                        int op = (opcode>>21)&15;
                        int rn = (opcode>>16)&15;
                        int rd = (opcode>>12)&15;
                        int op2 = opcode&0xfff;
                        char tmp[40] = {0};
                        if(i) { // op2 is immediate
                            uint8_t imm = op2&255;
                            int rot = (op2>>8)&0xf;
                            sprintf(tmp, "#%d", imm<<(rot*2));
                        } else {
                            int rm = op2&15;
                            int shift = op2>>4;
                            sprintf(tmp, "%s%s", regname[rm], print_shift(shift, 1));
                        }
                        if(op==0b1101 || op==0b1111) {  // MOV, MVN
                            sprintf(ret, "%s%s%s %s, %s", opcodes[op], cond, s?"S":"", regname[rd], tmp);
                        } else if(op==0b1000 || op==0b1001 || op==0b1010 || op==0b1011) {
                            // TST, TEQ, CMP, CMN
                            sprintf(ret, "%s%s %s, %s", opcodes[op], cond, regname[rn], tmp);
                        } else {
                            sprintf(ret, "%s%s%s %s, %s, %s", opcodes[op], cond, s?"S":"", regname[rd], regname[rn], tmp);
                        }
                    }
                    break;
                case 0b100:
                    // block data transfert
                    {
                        int p = (opcode>>24)&1;
                        int u = (opcode>>23)&1;
                        int s = (opcode>>22)&1;
                        int w = (opcode>>21)&1;
                        int l = (opcode>>20)&1;
                        int rn = (opcode>>16)&15;
                        int list = opcode&0b111111111111111;
                        int op = (l<<2)|(p<<1)|(u);
                        sprintf(ret, "%s %s%s,{", (rn==13)?blocktransfert_stack[op]:blocktransfert_other[op], regname[rn], w?"!":"");
                        int last=-2;
                        int cnt = 0;
                        char tmp[5];
                        for (int i=0; i<16; ++i) {
                            if(list&(1<<i)) { // present
                                if(last==i-1) {
                                    last = i;
                                    ++cnt;
                                } else {
                                    sprintf(tmp, "r%d", i);
                                    if(last>=0) strcat(ret, ",");
                                    strcat(ret, tmp);
                                    last=i;
                                    cnt = 0;
                                }
                            } else {
                                if(last==i-1) {
                                    if(cnt) {
                                        sprintf(tmp, "-r%d", last);
                                        strcat(ret, tmp);
                                    }
                                    last=-2;
                                }
                            }
                        }
                        if(last==15) {
                            sprintf(tmp, "-r%d", last);
                            strcat(ret, tmp);
                        }
                        strcat(ret, "}");
                        if(s)
                            strcat(ret, "^");
                    }
                    break;
                case 0b010:
                case 0b011:
                    if(cat==0b011 && ((opcode>>16)&15)==15 && ((opcode>>4)&15)==0b0111) {
                        //Sign extention
                        int b = ((opcode>>20)&1);
                        int s = ((opcode>>22)&1);
                        int rd = ((opcode>>12)&15);
                        int rm = ((opcode)&15);
                        int rot = ((opcode>>10)&3);
                        char tmp[20] = {0};
                        if (rot)
                            sprintf(tmp, " ror %d", rot*8);
                        sprintf(ret, "%sXT%s %s, %s%s", s?"U":"S", b?"H":"B", regname[rd], regname[rm], tmp);
                    } else if(((opcode>>21)&0b1111111)==0b0111111 && ((opcode>>4)&7)==0b101) {
                        int widthm1 = ((opcode>>16)&31);
                        int rd = ((opcode>>12)&15);
                        int lsb = ((opcode>>7)&31);
                        int rn = ((opcode)&15);
                        sprintf(ret, "UBFX %s, %s, #%d, #%d", regname[rd], regname[rn], lsb, widthm1+1);
                    } else if(((opcode>>21)&0b1111111)==0b0111110 && ((opcode>>4)&7)==0b001) {
                        int msb = ((opcode>>16)&31);
                        int rd = ((opcode>>12)&15);
                        int lsb = ((opcode>>7)&31);
                        int rn = ((opcode)&15);
                        sprintf(ret, "BFI %s, %s, #%d, #%d", regname[rd], regname[rn], lsb, (msb-lsb+1));
                    }
                    else
                    {
                        // single data tranfert
                        int i = ((opcode>>25)&1);
                        int p = ((opcode>>24)&1);
                        int u = ((opcode>>23)&1);
                        int b = ((opcode>>22)&1);
                        int w = ((opcode>>21)&1);
                        int l = ((opcode>>20)&1);
                        int rn = ((opcode>>16)&15);
                        int rd = ((opcode>>12)&15);
                        int offset = (opcode&0xfff);
                        char op2[40];
                        if(i) {
                            int shift = offset>>4;
                            int rm = offset&15;
                            sprintf(op2, "%s%s%s", u?"":"-", regname[rm], print_shift(shift, 1));
                        } else {
                            sprintf(op2, "#%d", u?offset:-offset);
                        }
                        char addr[50];
                        if(p) { // pre-index
                            if(!offset) {
                                sprintf(addr, "[%s]", regname[rn]);
                            } else {
                                if(i)
                                    sprintf(addr, "[%s, %s]%s", regname[rn], op2, w?"!":"");
                                else
                                    sprintf(addr, "[%s, %s]%s", regname[rn], op2, w?"!":"");
                            }
                        } else {
                            sprintf(addr, "[%s], %s", regname[rn], op2);
                        }
                        sprintf(ret, "%s%s%s%s %s, %s", l?"LDR":"STR", cond, b?"B":"", (w && p)?"T":"", regname[rd], addr);
                    }
                    break;
                case 0b111:
                    if(((opcode>>21)&0b1111111)==0b1110000 && (((opcode>>8)&0b1111)==0b1010) && ((opcode&0b1111111)==0b0010000)) {
                        // VMOV ARM to/from Sm
                        int rt = ((opcode>>12)&15);
                        int vn = ((opcode>>16)&15)<<1 | ((opcode>>7)&1);
                        int op = ((opcode>>20)&1);
                        if(op==0)
                            sprintf(ret, "VMOV%s S%d, %s", cond, vn, regname[rt]);
                        else
                            sprintf(ret, "VMOV%s %s, S%d", cond, regname[rt], vn);
                    } else
                    if(((opcode>>21)&0b1111111)==0b1110111 && (((opcode>>16)&0b1111)==0b0001) && ((opcode&0b111111111111)==0b101000010000)) {
                        // VMOV ARM to/from FPCSR
                        int rt = ((opcode>>12)&15);
                        int op = ((opcode>>20)&1);
                        if(op==0)
                            sprintf(ret, "VMSR%s FPSCR, %s", cond, regname[rt]);
                        else
                            sprintf(ret, "VMRS%s %s, FPCSR", cond, regname[rt]);
                    } else
                    if(((opcode>>19)&0b111110111)==0b111010111 && (((opcode>>9)&0b111)==0b101) && (((opcode>>4)&0b101)==0b100)) {
                        // VCVT float / integer
                        int sz = ((opcode>>8)&1);
                        int op = ((opcode>>7)&1);
                        int opc2 = ((opcode>>16)&7);
                        int D = (opcode>>22)&1;
                        int Vd = (opcode>>12)&15;
                        int M = (opcode>>5)&1;
                        int Vm = (opcode)&15;
                        int d, m;
                        int to_integer = (opc2&4)?1:0;
                        if(to_integer) {
                            d = (Vd<<1) | D;
                            m = (sz)?((M<<4)|Vm):((Vm<<1)|M);
                        } else {
                            m = ((Vm<<1)|M);
                            d = (sz)?((D<<4)|Vd):((Vd<<1)|D);
                        }
                        switch(opc2) {
                            case 0:
                                sprintf(ret, "VCVT%s.F%d.%c32 %c%d, S%d", cond, sz?64:32, op?'S':'U', sz?'D':'S', d, m);
                                break;
                            case 0b100:
                                sprintf(ret, "VCVT%s%s.U32.F%d S%d, %c%d", op?"":"R", cond, sz?64:32, d, sz?'D':'S', m);
                                break;
                            case 0b101:
                                sprintf(ret, "VCVT%s%s.S32.F%d S%d, %c%d", op?"":"R", cond, sz?64:32, d, sz?'D':'S', m);
                                break;
                            default:
                                sprintf(ret, "VCVT????%s ???", cond);
                        }
                    } else
                    if(((opcode>>20)&0b11111011)==0b11100010 && (((opcode>>9)&0b111)==0b101) && (((opcode>>4)&0b101)==0b000)) {
                        // VMUL
                        int sz = ((opcode>>8)&1);
                        int D = (opcode>>22)&1;
                        int Vn = (opcode>>16)&15;
                        int Vd = (opcode>>12)&15;
                        int N = (opcode>>7)&1;
                        int M = (opcode>>5)&1;
                        int Vm = (opcode)&15;
                        int d = (sz)?((D<<4)|Vd):((Vd<<1)|D);
                        int n = (sz)?((N<<4)|Vn):((Vn<<1)|N);
                        int m = (sz)?((M<<4)|Vm):((Vm<<1)|M);
                        char r = sz?'D':'S';
                        sprintf(ret, "VMUL%s.F%d %c%d, %c%d, %c%d", cond, sz?64:32, r, d, r, n, r, m);
                    } else
                    if(((opcode>>20)&0b11111011)==0b11101000 && (((opcode>>9)&0b111)==0b101) && (((opcode>>4)&0b101)==0b000)) {
                        // VDIV
                        int sz = ((opcode>>8)&1);
                        int D = (opcode>>22)&1;
                        int Vn = (opcode>>16)&15;
                        int Vd = (opcode>>12)&15;
                        int N = (opcode>>7)&1;
                        int M = (opcode>>5)&1;
                        int Vm = (opcode)&15;
                        int d = (sz)?((D<<4)|Vd):((Vd<<1)|D);
                        int n = (sz)?((N<<4)|Vn):((Vn<<1)|N);
                        int m = (sz)?((M<<4)|Vm):((Vm<<1)|M);
                        char r = sz?'D':'S';
                        sprintf(ret, "VDIV%s.F%d %c%d, %c%d, %c%d", cond, sz?64:32, r, d, r, n, r, m);
                    } else
                    if(((opcode>>20)&0b11111011)==0b11100011 && (((opcode>>9)&0b111)==0b101) && (((opcode>>4)&0b101)==0b000)) {
                        // VADD
                        int sz = ((opcode>>8)&1);
                        int D = (opcode>>22)&1;
                        int Vn = (opcode>>16)&15;
                        int Vd = (opcode>>12)&15;
                        int N = (opcode>>7)&1;
                        int M = (opcode>>5)&1;
                        int Vm = (opcode)&15;
                        int d = (sz)?((D<<4)|Vd):((Vd<<1)|D);
                        int n = (sz)?((N<<4)|Vn):((Vn<<1)|N);
                        int m = (sz)?((M<<4)|Vm):((Vm<<1)|M);
                        char r = sz?'D':'S';
                        sprintf(ret, "VADD%s.F%d %c%d, %c%d, %c%d", cond, sz?64:32, r, d, r, n, r, m);
                    } else
                    if(((opcode>>20)&0b11111011)==0b11100011 && (((opcode>>9)&0b111)==0b101) && (((opcode>>4)&0b101)==0b100)) {
                        // VSUB
                        int sz = ((opcode>>8)&1);
                        int D = (opcode>>22)&1;
                        int Vn = (opcode>>16)&15;
                        int Vd = (opcode>>12)&15;
                        int N = (opcode>>7)&1;
                        int M = (opcode>>5)&1;
                        int Vm = (opcode)&15;
                        int d = (sz)?((D<<4)|Vd):((Vd<<1)|D);
                        int n = (sz)?((N<<4)|Vn):((Vn<<1)|N);
                        int m = (sz)?((M<<4)|Vm):((Vm<<1)|M);
                        char r = sz?'D':'S';
                        sprintf(ret, "VSUB%s.F%d %c%d, %c%d, %c%d", cond, sz?64:32, r, d, r, n, r, m);
                    } else
                    if(((opcode>>20)&0b11111011)==0b11101011 && (((opcode>>16)&0b1111)==0b0001) && (((opcode>>9)&0b111)==0b101) && (((opcode>>4)&0b1101)==0b0100)) {
                        // VNEG
                        int sz = ((opcode>>8)&1);
                        int D = (opcode>>22)&1;
                        int Vd = (opcode>>12)&15;
                        int M = (opcode>>5)&1;
                        int Vm = (opcode)&15;
                        //int opc2 = (opcode>>16)&15;
                        //int opc3 = (opcode>>6)&3;
                        int d = (sz)?((D<<4)|Vd):((Vd<<1)|D);
                        int m = (sz)?((M<<4)|Vm):((Vm<<1)|M);
                        char r = sz?'D':'S';
                        sprintf(ret, "VNEG%s.F%d %c%d, %c%d", cond, sz?64:32, r, d, r, m);
                    } else
                    if(((opcode>>20)&0b11111011)==0b11101011 && (((opcode>>16)&0b1111)==0b0001) && (((opcode>>9)&0b111)==0b101) && (((opcode>>4)&0b1101)==0b1100)) {
                        // VSQRT
                        int sz = ((opcode>>8)&1);
                        int D = (opcode>>22)&1;
                        int Vd = (opcode>>12)&15;
                        int M = (opcode>>5)&1;
                        int Vm = (opcode)&15;
                        //int opc2 = (opcode>>16)&15;
                        //int opc3 = (opcode>>6)&3;
                        int d = (sz)?((D<<4)|Vd):((Vd<<1)|D);
                        int m = (sz)?((M<<4)|Vm):((Vm<<1)|M);
                        char r = sz?'D':'S';
                        sprintf(ret, "VSQRT%s.F%d %c%d, %c%d", cond, sz?64:32, r, d, r, m);
                    } else
                    if(((opcode>>20)&0b11111011)==0b11101011 && (((opcode>>16)&0b1111)==0b0100) && (((opcode>>8)&0b1110)==0b1010) && (((opcode>>4)&0b0101)==0b0100)) {
                        // VCMP single/double reg
                        int sz = ((opcode>>8)&1);
                        int D = (opcode>>22)&1;
                        int Vd = (opcode>>12)&15;
                        int M = (opcode>>5)&1;
                        int Vm = (opcode)&15;
                        int vd = (sz)?((D<<4) | Vd):(D | (Vd<<1));
                        int vm = (sz)?((M<<4) | Vm):(M | (Vm<<1));
                        int E = (opcode>>7)&1;
                        sprintf(ret, "VCMP%s%s.F%d %s%d, %s%d", E?"E":"", cond, sz?64:32, sz?"D":"S", vd, sz?"D":"S", vm);
                    } else
                    if(((opcode>>20)&0b11111011)==0b11101011 && (((opcode>>16)&0b1111)==0b0101) && (((opcode>>8)&0b1110)==0b1010) && (((opcode>>4)&0b0101)==0b0100)) {
                        // VCMP single/double reg to 0
                        int sz = ((opcode>>8)&1);
                        int D = (opcode>>22)&1;
                        int Vd = (opcode>>12)&15;
                        int vd = (sz)?((D<<4) | Vd):(D | (Vd<<1));
                        int E = (opcode>>7)&1;
                        sprintf(ret, "VCMP%s%s.F%d %s%d, #0", E?"E":"", cond, sz?64:32, sz?"D":"S", vd);
                    } else
                    if(((opcode>>20)&0b11111011)==0b11101011 && (((opcode>>8)&0b1110)==0b1010) && (((opcode>>4)&0b1101)==0b0100)) {
                        // VMOV single/double reg
                        int sz = ((opcode>>8)&1);
                        int D = (opcode>>22)&1;
                        int Vd = (opcode>>12)&15;
                        int M = (opcode>>5)&1;
                        int Vm = (opcode)&15;
                        int vd = (sz)?((D<<4) | Vd):(D | (Vd<<1));
                        int vm = (sz)?((M<<4) | Vm):(M | (Vm<<1));
                        sprintf(ret, "VMOV%s.F%d %s%d, %s%d", cond, sz?64:32, sz?"D":"S", vd, sz?"D":"S", vm);
                    } else
                    if(((opcode>>20)&0b11111011)==0b11101011 && (((opcode>>16)&0b1111)==0b0000) && (((opcode>>8)&0b1110)==0b1010) && (((opcode>>4)&0b1101)==0b1100)) {
                        // VABS single/double reg
                        int sz = ((opcode>>8)&1);
                        int D = (opcode>>22)&1;
                        int Vd = (opcode>>12)&15;
                        int M = (opcode>>5)&1;
                        int Vm = (opcode)&15;
                        int vd = (sz)?((D<<4) | Vd):(D | (Vd<<1));
                        int vm = (sz)?((M<<4) | Vm):(M | (Vm<<1));
                        sprintf(ret, "VABS%s.F%d %s%d, %s%d", cond, sz?64:32, sz?"D":"S", vd, sz?"D":"S", vm);
                    } else
                    if(((opcode>>20)&0b11111011)==0b11101011 && (((opcode>>16)&0b1111)==0b0111) && (((opcode>>4)&0b11101101)==0b10101100)) {
                        // VCVT float / float
                        int sz = ((opcode>>8)&1);
                        if(sz) {    // double to single
                            int vd = ((opcode>>12)&15)<<1 | ((opcode>>22)&1);
                            int vm = ((opcode>>5)&1)<<4 | (opcode&15);
                            sprintf(ret, "VCVT%s.F32.F64 S%d, D%d", cond, vd, vm);
                        } else {
                            int vd = ((opcode>>12)&15) | (((opcode>>22)&1)<<4);
                            int vm = ((opcode>>5)&1) | ((opcode&15)<<1);
                            sprintf(ret, "VCVT%s.F64.F32 D%d, S%d", cond, vd, vm);
                        }
                    }
                    break;
                case 0b110:
                    if(((opcode>>21)&0b1111111)==0b1100010 && (((opcode>>8)&0b1111)==0b1010) && (((opcode>>4)&0b1101)==0b0001)) {
                        // VMOV ARM to/from 2*SM
                        int rt = ((opcode>>12)&15);
                        int rt2 = ((opcode>>16)&15);
                        int vm = (opcode&15)<<1 | ((opcode>>5)&1);
                        int op = ((opcode>>20)&1);
                        if(op==0)
                            sprintf(ret, "VMOV%s S%d, S%d, %s, %s", cond, vm, vm+1, regname[rt], regname[rt2]);
                        else
                            sprintf(ret, "VMOV%s %s, %s, S%d, S%d", cond, regname[rt], regname[rt2], vm, vm+1);
                    } else
                    if(((opcode>>21)&0b1111111)==0b1100010 && (((opcode>>8)&0b1111)==0b1011) && (((opcode>>4)&0b1101)==0b0001)) {
                        // VMOV ARM to/from 2*SM
                        int rt = ((opcode>>12)&15);
                        int rt2 = ((opcode>>16)&15);
                        int vm = (opcode&15) | ((opcode>>5)&1)<<4;
                        int op = ((opcode>>20)&1);
                        if(op==0)
                            sprintf(ret, "VMOV%s D%d, %s, %s", cond, vm, regname[rt], regname[rt2]);
                        else
                            sprintf(ret, "VMOV%s %s, %s, D%d", cond, regname[rt], regname[rt2], vm);
                    } else
                    if(((opcode>>21)&0b1111001)==0b1101000 && (((opcode>>8)&0b1110)==0b1010)) {
                        // VLDR/VSTR to/from Single/Double
                        uint32_t ldr = ((opcode>>20)&1);
                        uint32_t u = ((opcode>>23)&1);
                        uint32_t rn = ((opcode>>16)&15);
                        int32_t imm8 = (opcode&255)<<2;
                        uint32_t D = (opcode>>22)&1;
                        uint32_t Vd = (opcode>>12)&15;
                        uint32_t notsingle = ((opcode>>8)&1);
                        uint32_t vd = (notsingle)?(D<<4 | Vd):(Vd<<1 | D);
                        char offset[50] = {0};
                        if(imm8)
                            sprintf(offset, ", #%d", u?imm8:-imm8);
                        sprintf(ret, "V%s%s %s%d, [%s%s]", ldr?"LDR":"STR", cond, notsingle?"D":"S", vd, regname[rn], offset);
                    } else
                    if(((opcode>>23)&0b111111111)==0b111100100 && (((opcode>>8)&0b1111)==0b1111) && (((opcode>>4)&0b1)==0b0)) {
                        // VMIN/VMAX
                        int op = (opcode>>21)&1;
                        int sz = (opcode>>20)&1;
                        int Q = (opcode>>6)&1;
                        int D = (opcode>>22)&1;
                        int Vd = (opcode>>12)&15;
                        int N = (opcode>>7)&1;
                        int Vn = (opcode>>16)&15;
                        int M = (opcode>>5)&1;
                        int Vm = (opcode)&15;
                        sprintf(ret, "%sV%s%s.F32 %c%d, %c%d, %c%d", sz?"!!":"", op?"MIN":"MAX", Q?"Q":"", Q?'Q':'D', (D<<4 | Vd), Q?'Q':'D', (N<<4 | Vn), Q?'Q':'D', (M<<4 | Vm));
                    }
                    break;
                default:
                    sprintf(ret, "???%s", cond);
            }
        }
    }

    return ret;
}