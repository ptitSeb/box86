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
            sprintf(ret, "%s%s #%ud", comma?",":"", shift_type[sh_op], amount);
        }
    }
    return ret;    
}

//  nop
#define NOP     (0xe1a00000)

const char* arm_print(uint32_t opcode)
{
    static __thread char ret[100];
    memset(ret, 0, sizeof(ret));
    if((opcode & (0b1111<<28))==(0b1111<<28)) {
        strcpy(ret, "?????");
    } else {
        const char* cond = conds[(opcode>>28)&15];
        if((opcode&0b00001111111111111111111111010000)==0b00000001001011111111111100010000) {
            int l = (opcode>>5)&1;
            sprintf(ret, "B%sX%s r%d", l?"L":"", cond, opcode&0b1111);
        } else if (((opcode>>25)&0b111)==0b101) {
            int32_t offset = opcode&0x3fffff;
            if(opcode&(1<<23)) offset = -offset;
            offset <<=2;
            sprintf(ret, "B%s%s #%d", cond, (opcode&(1<<24))?"L":"", offset);
        } else if((opcode&0b00001111110000000000000010010000)==0b00000000000000000000000010010000) {
            sprintf(ret, "MUL ???");
        } else if((opcode&0b00001111100000000000000010010000)==0b00000000100000000000000010010000) {
            sprintf(ret, "MULL ???");
        } else if((opcode&0b00001111101100000000111111110000)==0b00000001000000000000000010010000) {
            int b = (opcode>>22)&1;
            int rn = (opcode>>16)&15;
            int rd = (opcode>>12)&15;
            int rm = (opcode)&15;
            sprintf(ret, "SWP%s%s r%d, r%d, [r%d]", cond, b?"B":"", rd, rm, rn);
        } else if (((opcode>>26)&0b11)==0b01) {
            // ldr/str
            int i = (opcode>>25)&1;
            int p = (opcode>>24)&1;
            int u = (opcode>>23)&1;
            int b = (opcode>>22)&1;
            int w = (opcode>>21)&1;
            int l = (opcode>>20)&1;
            int rn = (opcode>>16)&15;
            int rd = (opcode>>12)&15;
            int offset = opcode&0xfff;
            char tmp[30] = {0};
            sprintf(ret, "%s%s%s%s %s,", l?"LDR":"STR", cond, b?"B":"", (w && p)?"T":"", regname[rd]);
            if(p) {     // pre-index
                if(!offset) {
                    sprintf(tmp, "[%s]", regname[rn]);
                } else {
                    if(i) {
                        int rm = offset&15;
                        int shift = offset>>4;
                        sprintf(tmp, "[%s, #%s%s%s]" ,regname[rn], u?"":"-", regname[rm], print_shift(shift, 1));
                    } else {
                        sprintf(tmp, "[%s, #%s%d]", regname[rn], u?"":"-", offset);
                    }
                    if(w)
                        strcat(tmp, "!");
                }
            } else {    // post-index
                if(i) {
                    int rm = offset&15;
                    int shift = offset>>4;
                    sprintf(tmp, "[%s], #%s%s%s" ,regname[rn], u?"":"-", regname[rm], print_shift(shift, 1));
                } else {
                    sprintf(tmp, "[%s], #%s%d", regname[rn], u?"":"-", offset);
                }
            }
            strcat(ret, tmp);
        } else if((opcode&0b00001111111100000000000000000000)==0b00000011000000000000000000000000) {
            uint16_t imm16 = opcode&0x0fff | ((opcode>>4)&0xf000);
            int rn = (opcode>>12)&15;
            sprintf(ret, "MOVW%s %s, #0x%x", cond, regname[rn], imm16);
        } else if((opcode&0b00001111111100000000000000000000)==0b00000011010000000000000000000000) {
            uint16_t imm16 = opcode&0x0fff | ((opcode>>4)&0xf000);
            int rn = (opcode>>12)&15;
            sprintf(ret, "MOVT%s %s, #0x%x", cond, regname[rn], imm16);
        } else
        {
        uint32_t cat = (opcode>>25)&0b111;
            switch (cat) {
                case 0b000:
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
                    // single data tranfert
                    {
                        int i = (opcode>>25)&1;
                        int p = (opcode>>24)&1;
                        int u = (opcode>>23)&1;
                        int b = (opcode>>22)&1;
                        int w = (opcode>>21)&1;
                        int l = (opcode>>20)&1;
                        int rn = (opcode>>16)&15;
                        int rd = (opcode>>12)&15;
                        int offset = opcode&0xfff;
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
                default:
                    sprintf(ret, "???%s", cond);
            }
        }
    }

    return ret;
}