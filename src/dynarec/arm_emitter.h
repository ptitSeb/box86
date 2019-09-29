
/*
    ARM Emitter

Most ARM instruction are encoded using the same pattern

Condition is stored on bits 28-31
Barel roll is stored on bits 5-6 for op, 7-11 for value, 0-4 for reg
Source register is 12-15
Dest register is 16-19
Op is 20-27
*/

// x86 Register mapping
#define xEAX    4
#define xECX    5
#define xEDX    6
#define xEBX    7
#define xESP    8
#define xEBP    9
#define xESI    10
#define xEDI    11
#define xEIP    12

// barel roll operations (4 possibles)
#define brLSL(i, r) (0<<5 | ((i&31)<<7) | r)
#define brLSR(i, r) (1<<5 | ((i&31)<<7) | r)
#define brASR(i, r) (2<<5 | ((i&31)<<7) | r)
#define brROR(i, r) (3<<5 | ((i&31)<<7) | r)
#define brIMM(r)    (r)

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

//  nop
#define NOP     EMIT(0xe1a00000)

// mov dst, src
#define MOV_REG(dst, src) EMIT(0xe1a00000 | ((dst) << 12) | (src) )

// movw dst, #imm16
#define MOVW(dst, imm16) EMIT(0xe3000000 | ((dst) << 12) | (((imm16) & 0xf000) << 4) | brIMM((imm16) & 0x0fff) )
// movt dst, #imm16
#define MOVT(dst, imm16) EMIT(0xe3400000 | ((dst) << 12) | (((imm16) & 0xf000) << 4) | brIMM((imm16) & 0x0fff) )
// pseudo insruction: mov reg, #imm with imm a 32bits value
#define MOV32(dst, imm32)                   \
    MOVW(dst, ((uint32_t)imm32)&0xffff);    \
    if (((uint32_t)imm32)>>16) {            \
        MOVT(dst, (((uint32_t)imm32)>>16)); }
// movw.cond dst, #imm16
#define MOVW_COND(cond, dst, imm16) EMIT(cond | 0x03000000 | ((dst) << 12) | (((imm16) & 0xf000) << 4) | brIMM((imm16) & 0x0fff) )


// sub dst, src, #(imm8)
#define SUB_IMM8(dst, src, imm8) EMIT(0xe2400000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// add.s dst, src, #(imm8)
#define SUBS_IMM8(dst, src, imm8) EMIT(0xe2500000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// add dst, src1, src2, lsl #imm
#define SUB_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0400000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// add.s dst, src1, src2, lsl #imm
#define SUBS_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0500000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )

// add dst, src, #(imm8)
#define ADD_IMM8(dst, src, imm8) EMIT(0xe2800000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// add.s dst, src, #(imm8)
#define ADDS_IMM8(dst, src, imm8) EMIT(0xe2900000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// add dst, src1, src2, lsl #imm
#define ADD_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0800000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// add.s dst, src1, src2, lsl #imm
#define ADDS_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0900000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// xor dst, src1, src2, lsl #imm
#define XOR_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0200000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// xor.s dst, src1, src2, lsl #imm
#define XORS_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0300000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )

// ldr reg, [addr, #imm9]
#define LDR_IMM9(reg, addr, imm9) EMIT(0xe5900000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// ldr reg, [addr, #imm9]!
#define LDR_IMM9_W(reg, addr, imm9) EMIT(0xe5b00000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )

// str reg, [addr, #imm9]
#define STR_IMM9(reg, addr, imm9) EMIT(0xe5800000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// str reg, [addr, #-(imm9)]!
#define STR_NIMM9_W(reg, addr, imm9) EMIT(0xe5200000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )

// bx reg
#define BX(reg) EMIT(0xe12fff10 | (reg) )

// blx reg
#define BLX(reg) EMIT(0xe12fff30 | (reg) )

// push reg!, {list}
//                           all |    const    |pre-index| subs    | no PSR  |writeback| store   |   base    |reg list
#define PUSH(reg, list) EMIT(c__ | (0b100<<25) | (1<<24) | (0<<23) | (0<<22) | (1<<21) | (0<<20) | (reg<<16) | (list))

// pop reg!, {list}
//                           all |    const    |postindex|  add    | no PSR  |writeback|  load   |   base    |reg list
#define POP(reg, list)  EMIT(c__ | (0b100<<25) | (0<<24) | (1<<23) | (0<<22) | (1<<21) | (1<<20) | (reg<<16) | (list))
