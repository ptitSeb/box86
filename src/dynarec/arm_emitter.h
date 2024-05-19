#ifndef __ARM_EMITTER_H__
#define __ARM_EMITTER_H__
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
#define xFlags  12
#define xEIP    14
// scratch registers
#define x1      1
#define x2      2
#define x3      3
#define x14     14
// emu is r0
#define xEmu    0
// ARM SP is r13
#define xSP     13      

// barel roll operations (4 possibles)
#define brLSL(i, r) (0<<4 | 0<<5 | (((i)&31)<<7) | (r))
#define brLSR(i, r) (0<<4 | 1<<5 | (((i)&31)<<7) | (r))
#define brASR(i, r) (0<<4 | 2<<5 | (((i)&31)<<7) | (r))
#define brROR(i, r) (0<<4 | 3<<5 | (((i)&31)<<7) | (r))
#define brIMM(r)    (abs(r))
// barel roll with a register
#define brRLSL(i, r) (1<<4 | 0<<5 | (((i)&15)<<8) | (r))
#define brRLSR(i, r) (1<<4 | 1<<5 | (((i)&15)<<8) | (r))
#define brRASR(i, r) (1<<4 | 2<<5 | (((i)&15)<<8) | (r))
#define brRROR(i, r) (1<<4 | 3<<5 | (((i)&15)<<8) | (r))


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
#define NOP     EMIT(0xe320f000)

// mov dst, src
#define MOV_REG(dst, src) EMIT(0xe1a00000 | ((dst) << 12) | (src) )
// movxx dst, src
#define MOV_REG_COND(cond, dst, src) EMIT( cond | 0x01a00000 | ((dst) << 12) | (src) )

// movw dst, #imm16
#define MOVW(dst, imm16) EMIT(0xe3000000 | ((dst) << 12) | (((imm16) & 0xf000) << 4) | brIMM((imm16) & 0x0fff) )
// movt dst, #imm16
#define MOVT(dst, imm16) EMIT(0xe3400000 | ((dst) << 12) | (((imm16) & 0xf000) << 4) | brIMM((imm16) & 0x0fff) )
// pseudo insruction: mov reg, #imm with imm a 32bits value
#define MOV32(dst, imm32)                       \
    if((~(uint32_t)(imm32))<0x100) {            \
        MVN_IMM8(dst, ~(uint32_t)(imm32), 0);   \
    } else {                                    \
        MOVW(dst, ((uint32_t)imm32)&0xffff);    \
        if (((uint32_t)(imm32))>>16) {          \
        MOVT(dst, (((uint32_t)imm32)>>16));}    \
    }
// pseudo insruction: mov reg, #imm with imm a 32bits value, fixed size
#define MOV32_(dst, imm32)                  \
    MOVW(dst, ((uint32_t)(imm32))&0xffff);  \
    MOVT(dst, (((uint32_t)(imm32))>>16))
// movw.cond dst, #imm16
#define MOVW_COND(cond, dst, imm16) EMIT(cond | 0x03000000 | ((dst) << 12) | (((imm16) & 0xf000) << 4) | brIMM((imm16) & 0x0fff) )
// mov dst #imm8 ror imm4*2
#define MOV_IMM(dst, imm8, rimm4) EMIT(0xe3a00000 | ((dst) << 12) | (imm8) | ((rimm4) << 8) )
// mov.cond dst #imm8 ror imm4*2
#define MOV_IMM_COND(cond, dst, imm8, rimm4) EMIT(cond | 0x03a00000 | ((dst) << 12) | (imm8) | ((rimm4) << 8) )

// mov dst, src lsl imm5
#define MOV_REG_LSL_IMM5(dst, src, imm5) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (0<<4) | (0<<5) | ((imm5)<<7))
// mov dst, src lsr imm5
#define MOV_REG_LSR_IMM5(dst, src, imm5) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (0<<4) | (1<<5) | ((imm5)<<7))
// mov.cond dst, src lsr imm5
#define MOV_REG_LSR_IMM5_COND(cond, dst, src, imm5) EMIT(cond | 0x01a00000 | ((dst) << 12) | (src) | (0<<4) | (1<<5) | ((imm5)<<7))
// mov dst, src asr imm5
#define MOV_REG_ASR_IMM5(dst, src, imm5) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (0<<4) | (2<<5) | ((imm5)<<7))
// mov dst, src ror imm5
#define MOV_REG_ROR_IMM5(dst, src, imm5) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (0<<4) | (3<<5) | ((imm5)<<7))
// mov dst, src rrx (1)
#define MOV_REG_RRX(dst, src) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (0<<4) | (3<<5) | ((0)<<7))
// mov.s dst, src lsl imm5
#define MOVS_REG_LSL_IMM5(dst, src, imm5) EMIT(0xe1b00000 | ((dst) << 12) | (src) | (0<<4) | (0<<5) | ((imm5)<<7))
// mov.s dst, src lsr imm5
#define MOVS_REG_LSR_IMM5(dst, src, imm5) EMIT(0xe1b00000 | ((dst) << 12) | (src) | (0<<4) | (1<<5) | ((imm5)<<7))
// mov.s dst, src asr imm5
#define MOVS_REG_ASR_IMM5(dst, src, imm5) EMIT(0xe1b00000 | ((dst) << 12) | (src) | (0<<4) | (2<<5) | ((imm5)<<7))
// mov.s dst, src ror imm5
#define MOVS_REG_ROR_IMM5(dst, src, imm5) EMIT(0xe1b00000 | ((dst) << 12) | (src) | (0<<4) | (3<<5) | ((imm5)<<7))
// mov.s dst, src rrx (1)
#define MOVS_REG_RRX(dst, src) EMIT(0xe1b00000 | ((dst) << 12) | (src) | (0<<4) | (3<<5) | ((0)<<7))

// mov.s dst, src lsl rs
#define MOVS_REG_LSL_REG(dst, src, rs) EMIT(0xe1b00000 | ((dst) << 12) | (src) | (1<<4) | (0<<5) | ((rs)<<8))
// mov.s dst, src lsr rs
#define MOVS_REG_LSR_REG(dst, src, rs) EMIT(0xe1b00000 | ((dst) << 12) | (src) | (1<<4) | (1<<5) | ((rs)<<8))
// mov dst, src lsl rs
#define MOV_REG_LSL_REG(dst, src, rs) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (1<<4) | (0<<5) | ((rs)<<8))
// mov.cond dst, src lsl rs
#define MOV_REG_LSL_REG_COND(cond, dst, src, rs) EMIT(cond | 0x01a00000 | ((dst) << 12) | (src) | (1<<4) | (0<<5) | ((rs)<<8))
// mov dst, src lsr rs
#define MOV_REG_LSR_REG(dst, src, rs) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (1<<4) | (1<<5) | ((rs)<<8))
// mov dst, src asr rs
#define MOV_REG_ASR_REG(dst, src, rs) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (1<<4) | (2<<5) | ((rs)<<8))
// mov dst, src ror rs
#define MOV_REG_ROR_REG(dst, src, rs) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (1<<4) | (3<<5) | ((rs)<<8))

// sub dst, src, #(imm8)
#define SUB_IMM8(dst, src, imm8) \
    EMIT(0xe2400000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// sub cond dst, src, #(imm8)
#define SUB_COND_IMM8(cond, dst, src, imm8) \
    EMIT((cond) | 0x02400000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// sub.s dst, src, #(imm8)
#define SUBS_IMM8(dst, src, imm8) \
    EMIT(0xe2500000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// sub dst, src1, src2, lsl #imm
#define SUB_REG_LSL_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe0400000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// sub.s dst, src1, src2, lsl #imm
#define SUBS_REG_LSL_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe0500000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )

// sbc dst, src, #(imm8)
#define SBC_IMM8(dst, src, imm8) \
    EMIT(0xe2c00000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// sbc.s dst, src, #(imm8)
#define SBCS_IMM8(dst, src, imm8) \
    EMIT(0xe2d00000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// sbc dst, src1, src2, lsl #imm
#define SBC_REG_LSL_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe0c00000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// sbc.s dst, src1, src2, lsl #imm
#define SBCS_REG_LSL_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe0d00000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )

// rsb dst, src, #(imm8)
#define RSB_IMM8(dst, src, imm8) \
    EMIT(0xe2600000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// rsb.s dst, src, #(imm8)
#define RSBS_IMM8(dst, src, imm8) \
    EMIT(0xe2700000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// rsb cond dst, src, #(imm8)
#define RSB_COND_IMM8(cond, dst, src, imm8) \
    EMIT((cond) | 0x02600000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// rsc dst, src, #(imm8)
#define RSC_IMM8(dst, src, imm8) \
    EMIT(0xe2E00000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )

// and dst, src1, src2, lsl #imm
#define AND_REG_LSL_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe0000000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// and.s dst, src1, src2, lsl #imm
#define ANDS_REG_LSL_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe0100000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// and dst, src1, src2, lsr #imm
#define AND_REG_LSR_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe0000000 | ((dst) << 12) | ((src1) << 16) | brLSR(imm5, src2) )
// and dst, src, #(imm8)
#define AND_IMM8(dst, src, imm8) \
    EMIT(0xe2000000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// and dst, src1, #imm ror rot*2
#define AND_IMM8_ROR(dst, src, imm8, rot) \
    EMIT(0xe2000000 | ((dst) << 12) | ((src) << 16) | ((rot)<<8) | brIMM(imm8) )
// and.c dst, src, #(imm8)
#define AND_IMM8_COND(cond, dst, src, imm8) \
    EMIT((cond) | 0x02000000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// and.s dst, src, #(imm8)
#define ANDS_IMM8(dst, src, imm8) \
    EMIT(0xe2100000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// and.s dst, src1, #imm ror rot*2
#define ANDS_IMM8_ROR(dst, src, imm8, rot) \
    EMIT(0xe2100000 | ((dst) << 12) | ((src) << 16) | ((rot)<<8) | brIMM(imm8) )
// add dst, src, #(imm8)
#define ADD_IMM8(dst, src, imm8) \
    EMIT(0xe2800000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// add dst, src, #imm8 ror rot*2
#define ADD_IMM8_ROR(dst, src, imm8, rot) \
    EMIT(0xe2800000 | ((dst) << 12) | ((src) << 16) | ((rot)<<8) | brIMM((imm8)) )
// add.s dst, src, #(imm8)
#define ADDS_IMM8(dst, src, imm8) \
    EMIT(0xe2900000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// add.c dst, src, #(imm8)
#define ADD_IMM8_cond(cond, dst, src, imm8) \
    EMIT((cond) | 0x02800000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// add dst, src1, src2, lsl #imm
#define ADD_REG_LSL_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe0800000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// add.c dst, src1, src2, lsl #imm
#define ADD_REG_LSL_IMM5_COND(cond, dst, src1, src2, imm5) \
    EMIT((cond) | 0x00800000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// add.s dst, src1, src2, lsl #imm
#define ADDS_REG_LSL_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe0900000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// add dst, src1, src2, lsr #imm
#define ADD_REG_LSR_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe0800000 | ((dst) << 12) | ((src1) << 16) | brLSR(imm5, src2) )
#define ADD_REG_LSR_IMM5_COND(cond, dst, src1, src2, imm5) \
    EMIT((cond) | 0x00800000 | ((dst) << 12) | ((src1) << 16) | brLSR(imm5, src2) )
#define ADC_IMM8(dst, src, imm8) \
    EMIT(0xe2a00000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// add.s dst, src, #(imm8)
#define ADCS_IMM8(dst, src, imm8) \
    EMIT(0xe2b00000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// add dst, src1, src2, lsl #imm
#define ADC_REG_LSL_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe0a00000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// add.s dst, src1, src2, lsl #imm
#define ADCS_REG_LSL_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe0b00000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// cmp.s dst, src1, src2, lsl #imm
#define CMPS_REG_LSL_IMM5(src1, src2, imm5) \
    EMIT(0xe1500000 | ((0) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// cmp.cond.s dst, src1, src2, lsl #imm
#define CMPS_REG_LSL_IMM5_COND(cond, src1, src2, imm5) \
    EMIT((cond) | 0x01500000 | ((0) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// cmp.s dst, src, #imm
#define CMPS_IMM8_COND(cond, src, imm8) \
    EMIT((cond) | 0x03500000 | ((0) << 12) | ((src) << 16) | brIMM(imm8) )
// cmp.s dst, src, #imm
#define CMPS_IMM8(src, imm8) \
    EMIT(0xe3500000 | ((0) << 12) | ((src) << 16) | brIMM(imm8) )
// cmn.s.cond dst, src, #imm
#define CMNS_IMM8_COND(cond, src, imm8) \
    EMIT((cond) | 0x03700000 | ((0) << 12) | ((src) << 16) | brIMM(imm8) )
// cmn.s dst, src, #imm
#define CMNS_IMM8(src, imm8) \
    EMIT(c__ | 0x03700000 | ((0) << 12) | ((src) << 16) | brIMM(imm8) )
// tst.s dst, src1, src2, lsl #imm
#define TSTS_REG_LSL_IMM5(src1, src2, imm5) \
    EMIT(0xe1100000 | ((0) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// tst.s dst, src1, #imm
#define TSTS_IMM8(src, imm8) \
    EMIT(0xe3100000 | ((0) << 12) | ((src) << 16) | brIMM(imm8) )
// tst.s dst, src1, #imm ror rot*2
#define TSTS_IMM8_ROR(src, imm8, rot) \
    EMIT(0xe3100000 | ((0) << 12) | ((src) << 16) | ((rot)<<8) | brIMM(imm8) )
// tst.s.cond dst, src1, #imm ror rot*2
#define TSTS_IMM8_ROR_COND(cond, src, imm8, rot) \
    EMIT((cond) | 0x03100000 | ((0) << 12) | ((src) << 16) | ((rot)<<8) | brIMM(imm8) )
// orr dst, src1, src2, lsl #imm
#define ORR_REG_LSL_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe1800000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// orr.cond dst, src1, src2, lsl #imm
#define ORR_REG_LSL_IMM5_COND(cond, dst, src1, src2, imm5) \
    EMIT(cond | 0x01800000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// orr.s dst, src1, src2, lsl #imm
#define ORRS_REG_LSL_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe1900000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// orr dst, src1, src2, lsr #imm
#define ORR_REG_LSR_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe1800000 | ((dst) << 12) | ((src1) << 16) | brLSR(imm5, src2) )
// orr.cond dst, src1, src2, lsr #imm
#define ORR_REG_LSR_IMM5_COND(cond, dst, src1, src2, imm5) \
    EMIT(cond | 0x01800000 | ((dst) << 12) | ((src1) << 16) | brLSR(imm5, src2) )
// orr.s dst, src1, src2, lsr #imm
#define ORRS_REG_LSR_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe1900000 | ((dst) << 12) | ((src1) << 16) | brLSR(imm5, src2) )
// orr.s dst, src1, src2, lsl rs
#define ORRS_REG_LSL_REG(dst, src1, src2, rs) \
    EMIT(0xe1900000 | ((dst) << 12) | ((src1) << 16) | brRLSL(rs, src2) )
// orr.s dst, src1, src2, lsr rs
#define ORRS_REG_LSR_REG(dst, src1, src2, rs) \
    EMIT(0xe1900000 | ((dst) << 12) | ((src1) << 16) | brRLSR(rs, src2) )
// orr dst, src1, #imm8
#define ORR_IMM8(dst, src, imm8, rot) \
    EMIT(0xe3800000 | ((dst) << 12) | ((src) << 16) | ((rot)<<8) | imm8 )
// orr.s dst, src1, #imm8
#define ORRS_IMM8(dst, src, imm8, rot) \
    EMIT(0xe3900000 | ((dst) << 12) | ((src) << 16) | ((rot)<<8) | imm8 )
// orr.cond dst, src1, #imm8 ror rot*2
#define ORR_IMM8_COND(cond, dst, src, imm8, rot) \
    EMIT((cond) | 0x03800000 | ((dst) << 12) | ((src) << 16) | ((rot)<<8) | imm8 )
// orr dst, src1, src2, lsl rs
#define ORR_REG_LSL_REG(dst, src1, src2, rs) \
    EMIT(0xe1800000 | ((dst) << 12) | ((src1) << 16) | brRLSL(rs, src2) )
// orr dst, src1, src2, lsr rs
#define ORR_REG_LSR_REG(dst, src1, src2, rs) \
    EMIT(0xe1800000 | ((dst) << 12) | ((src1) << 16) | brRLSR(rs, src2) )
// xor dst, src1, src2, lsl #imm
#define XOR_REG_LSL_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe0200000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// xor.s dst, src1, src2, lsl #imm
#define XORS_REG_LSL_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe0300000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
#define XOR_REG_LSL_IMM5_COND(cond, dst, src1, src2, imm5) \
    EMIT((cond) | 0x00200000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// xor dst, src, #(imm8)
#define XOR_IMM8(dst, src, imm8) \
    EMIT(0xe2200000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
#define XORS_IMM8(dst, src, imm8) \
    EMIT(0xe2300000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// xor dst, src, #(imm8 ror rot*2)
#define XOR_IMM8_ROR(dst, src, imm8, rot) \
    EMIT(0xe2200000 | ((dst) << 12) | ((src) << 16) | ((rot)<<8) | brIMM(imm8) )
// xor.cond dst, src, #(imm8)
#define XOR_IMM8_COND(cond, dst, src, imm8) \
    EMIT(cond | 0x02200000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// xor dst, src1, src2, lsl #rs
#define XOR_REG_LSL_REG(dst, src1, src2, rs) \
    EMIT(0xe0200000 | ((dst) << 12) | ((src1) << 16) | brRLSL(rs, src2) )
// xor dst, src1, src2, lsl #imm
#define XOR_REG_LSR_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0200000 | ((dst) << 12) | ((src1) << 16) | brLSR(imm8, src2) )
// xor.cond dst, src1, src2, lsl #imm
#define XOR_REG_LSR_IMM8_COND(cond, dst, src1, src2, imm8) \
    EMIT(cond | 0x00200000 | ((dst) << 12) | ((src1) << 16) | brLSR(imm8, src2) )
// bic dst, src1, src2, lsl #imm
#define BIC_REG_LSL_IMM5(dst, src1, src2, imm5) \
    EMIT(0xe1c00000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm5, src2) )
// bic dst, src, IMM8 ror rot*2
#define BIC_IMM8(dst, src, imm8, rot) \
    EMIT(0xe3c00000 | ((dst) << 12) | ((src) << 16) | ((rot)<<8) | imm8 )
// bic.cond dst, src, IMM8
#define BIC_IMM8_COND(cond, dst, src, imm8, rot) \
    EMIT((cond) | 0x03c00000 | ((dst) << 12) | ((src) << 16) | ((rot)<<8) | imm8 )
// bic dst, src1, #imm ror rot*2
#define BIC_IMM8_ROR(dst, src, imm8, rot) \
    EMIT(0xe3c00000 | ((dst) << 12) | ((src) << 16) | ((rot)<<8) | brIMM(imm8) )
// bic.s dst, src1, #imm ror rot*2
#define BICS_IMM8_ROR(dst, src, imm8, rot) \
    EMIT(0xe3d00000 | ((dst) << 12) | ((src) << 16) | ((rot)<<8) | brIMM(imm8) )
// bic dst, src1, src2, lsl #imm
#define BIC_REG_LSL_REG(dst, src1, src2, rs) \
    EMIT(0xe1c00000 | ((dst) << 12) | ((src1) << 16) | brRLSL(rs, src2) )
// mvn dst, src1, src2, lsl #imm
#define MVN_REG_LSL_IMM5(dst, rm, imm5) \
    EMIT(0xe1e00000 | ((dst) << 12) | (0 << 16) | brLSL(imm5, rm) )
// mvn dst, src, IMM8
#define MVN_IMM8(dst, imm8, rot) \
    EMIT(0xe3e00000 | ((dst) << 12) | (rot)<<8 | ((imm8)&0xff) )
// mvn dst, src1, src2, lsr #imm
#define MVN_REG_LSR_IMM8(dst, rm, imm8) \
    EMIT(0xe1e00000 | ((dst) << 12) | (0 << 16) | brLSR(imm8, rm) )
// mvn dst, rm lsr rs
#define MVN_REG_LSR_REG(dst, rm, rs) \
    EMIT(0xe1e00000 | ((dst) << 12) | (0 << 16) | (1<<4) | (1<<5) | (rs<<8) | (rm) )
// mvn with condition dst, src1, src2, lsl #imm
#define MVN_COND_REG_LSL_IMM5(cond, dst, rm, imm5) \
    EMIT(cond | 0x01e00000 | ((dst) << 12) | (0 << 16) | brLSL(imm5, rm) )

// ldr reg, [addr, #+/-imm9]
#define LDR_IMM9(reg, addr, imm9) EMIT(0xe5100000 | (((imm9)<0)?0:1)<<23 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// ldrxx reg, [addr, #+/-imm9]
#define LDR_IMM9_COND(cond, reg, addr, imm9) EMIT(cond | 0x05100000 | (((imm9)<0)?0:1)<<23 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// ldrb reg, [addr, #+/-imm9]
#define LDRB_IMM9(reg, addr, imm9) EMIT(0xe5500000 | (((imm9)<0)?0:1)<<23 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// ldr reg, [addr, #+/-imm9]!
#define LDR_IMM9_W(reg, addr, imm9) EMIT(0xe5300000 | (((imm9)<0)?0:1)<<23 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// ldr reg, [addr, rm lsl imm5]
#define LDR_REG_LSL_IMM5(reg, addr, rm, imm5) EMIT(c__ | (0b011<<25) | (1<<24) | (1<<23) | (0<<21) | (1<<20) | ((reg) << 12) | ((addr) << 16) | brLSL(imm5, rm) )
// ldr reg, [addr, rm lsr imm5]
#define LDR_REG_LSR_IMM5(reg, addr, rm, imm5) EMIT(c__ | (0b011<<25) | (1<<24) | (1<<23) | (0<<21) | (1<<20) | ((reg) << 12) | ((addr) << 16) | brLSR(imm5, rm) )
// ldrb reg, [addr, rm lsl imm5]
#define LDRB_REG_LSL_IMM5(reg, addr, rm, imm5) EMIT(0xe5d00000 | ((reg) << 12) | ((addr) << 16) | (1<<25) | brLSL(imm5, rm) )
// ldr reg, [addr], #+/-imm9
#define LDRAI_IMM9_W(reg, addr, imm9)   EMIT(0xe4100000 | (((imm9)<0)?0:1)<<23 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// ldrb reg, [addr], #+/-imm9
#define LDRBAI_IMM9_W(reg, addr, imm9)  EMIT(0xe4100000 | (((imm9)<0)?0:1)<<23 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// ldr reg, [addr], rm lsl imm5
#define LDRAI_REG_LSL_IMM5(reg, addr, rm, imm5)  EMIT(0xe6900000 | ((reg) << 12) | ((addr) << 16) | (1<<25) | brLSL(imm5, rm) )
// ldrb reg, [addr], rm lsl imm5
#define LDRBAI_REG_LSL_IMM5(reg, addr, rm, imm5) EMIT(0xe6d00000 | ((reg) << 12) | ((addr) << 16) | (1<<25) | brLSL(imm5, rm) )
// ldrd reg, reg+1, [addr, #+/-imm9], reg must be even, reg+1 is implicit
#define LDRD_IMM8(reg, addr, imm8) EMIT(c__ | 0b000<<25 | 1<<24  | (((imm8)<0)?0:1)<<23 | 1<<22 | 0<<21 | 0<<20 | ((reg) << 12) | ((addr) << 16) | ((abs(imm8))&0xf0)<<(8-4) | (0b1101<<4) | ((abs(imm8))&0x0f) )
// ldrd reg, reg+1, [addr, +rm], reg must be even, reg+1 is implicit
#define LDRD_REG(reg, addr, rm) EMIT(c__ | 0b000<<25 | 1<<24  | 1<<23 | 0<<22 | 0<<21 | 0<<20 | ((reg) << 12) | ((addr) << 16) | 0b1101<<4 | (rm) )
// ldr reg, [pc+imm]
#define LDR_literal(reg, imm12) EMIT(c__ | 0b010<<25 | 1<<24 | (((imm12)>=0)?1:0)<<23 | 0<<21 | 1<<20 | 0b1111<<16 | (reg)<<12 | (abs(imm12)))

// str reg, [addr, #+/-imm9]
#define STR_IMM9(reg, addr, imm9) EMIT(0xe5000000 | (((imm9)<0)?0:1)<<23 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// str with cond reg, [addr, #+/-imm9]
#define STR_IMM9_COND(cond, reg, addr, imm9) EMIT(cond | 0x05000000 | (((imm9)<0)?0:1)<<23 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// str reg, [addr, #+/-imm9]!
#define STR_IMM9_W(reg, addr, imm9) EMIT(0xe5200000 | (((imm9)<0)?0:1)<<23 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// strb reg, [addr, #+/-imm9]
#define STRB_IMM9(reg, addr, imm9) EMIT(0xe5400000 | (((imm9)<0)?0:1)<<23 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// str reg, [addr], #+/-imm9
#define STRAI_IMM9_W(reg, addr, imm9)  EMIT(0xe4000000 | (((imm9)<0)?0:1)<<23 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// str reg, [addr, rm lsl imm5]
#define STR_REG_LSL_IMM5(reg, addr, rm, imm5) EMIT(0xe5800000 | ((reg) << 12) | ((addr) << 16) | (1<<25) | brLSL(imm5, rm) )
// strb reg, [addr, rm lsl imm5]
#define STRB_REG_LSL_IMM5(reg, addr, rm, imm5) EMIT(0xe5c00000 | ((reg) << 12) | ((addr) << 16) | (1<<25) | brLSL(imm5, rm) )
// strb reg, [addr], #+/-imm9
#define STRBAI_IMM9_W(reg, addr, imm9)  EMIT(0xe4400000 | (((imm9)<0)?0:1)<<23 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// str reg, [addr], rm lsl imm5
#define STRAI_REG_LSL_IMM5(reg, addr, rm, imm5)  EMIT(0xe6800000 | ((reg) << 12) | ((addr) << 16) | (1<<25) | brLSL(imm5, rm) )
// strb reg, [addr], rm lsl imm5
#define STRBAI_REG_LSL_IMM5(reg, addr, rm, imm5) EMIT(0xe6c00000 | ((reg) << 12) | ((addr) << 16) | (1<<25) | brLSL(imm5, rm) )
// strd reg, reg+1, [addr, #+/-imm8], reg must be even, reg+1 is implicit
#define STRD_IMM8(reg, addr, imm8) EMIT(c__ | 0b000<<25 | 1<<24 | (((imm8)<0)?0:1)<<23 | 1<<22 | 0<<21 | 0<<20 | ((reg) << 12) | ((addr) << 16) | ((abs(imm8))&0xf0)<<(8-4) | (0b1111<<4) | ((abs(imm8))&0x0f) )
// strd reg, reg+1, [addr, +rm], reg must be even, reg+1 is implicit
#define STRD_REG(reg, addr, rm) EMIT(c__ | 0b000<<25 | 1<<24 | 1<<23 | 0<<22 | 0<<21 | 0<<20 | ((reg) << 12) | ((addr) << 16) | 0b1111<<4 | (rm) )

// bx reg
#define BX(reg) EMIT(0xe12fff10 | (reg) )
// bx cond reg
#define BXcond(C, reg) EMIT(C | 0x012fff10 | (reg) )

// blx reg
#define BLX(reg) EMIT(0xe12fff30 | (reg) )
// blx cond reg
#define BLXcond(C, reg) EMIT(C | 0x012fff30 | (reg) )

// b cond offset
#define Bcond(C, O) EMIT(C | (0b101<<25) | (0<<24) | (((O)>>2)&0xffffff))

// bl cond offset
#define BLcond(C, O) EMIT(C | (0b101<<25) | (1<<24) | (((O)>>2)&0xffffff))

// adr
#define ADR_gen(cond, P, Rd, Imm12) (cond | (1<<25) | (((P)?0b0100:0b0010)<<21) | (0b1111<<16) | ((Rd)<<12) | (Imm12))
#define ADR(cond, Rd, Imm)  EMIT(ADR_gen(cond, (Imm>=0), Rd, ((Imm>=0)?Imm:-Imm)&0b111111111111))

// push reg!, {list}
//                           all |    const    |pre-index| subs    | no PSR  |writeback| store   |   base    |reg list
#define PUSH(reg, list) EMIT(c__ | (0b100<<25) | (1<<24) | (0<<23) | (0<<22) | (1<<21) | (0<<20) | ((reg)<<16) | (list))

// push reg to xESP
#define PUSH1(reg)   STR_IMM9_W(reg, xESP, -4)

// pop reg!, {list}
//                           all |    const    |postindex|  add    | no PSR  |writeback|  load   |   base    |reg list
#define POP(reg, list)  EMIT(c__ | (0b100<<25) | (0<<24) | (1<<23) | (0<<22) | (1<<21) | (1<<20) | ((reg)<<16) | (list))

// pop reg from xESP
#define POP1(reg)   LDRAI_IMM9_W(reg, xESP, 4)

// STMDB reg, {list}
//                            all |    const    |pre-index| subs    | no PSR  |  no wb  | store   |   base    |reg list
#define STMDB(reg, list) EMIT(c__ | (0b100<<25) | (1<<24) | (0<<23) | (0<<22) | (0<<21) | (0<<20) | ((reg)<<16) | (list))
// STMia reg, {list}
//                          all |    const    |postindex|   add   | no PSR  |  no wb  |  store  |   base    |reg list
#define STM(reg, list) EMIT(c__ | (0b100<<25) | (0<<24) | (1<<23) | (0<<22) | (0<<21) | (0<<20) | ((reg)<<16) | (list))
// LDMia reg, {list}
//                          all |    const    |postindex|   add   | no PSR  |  no wb  |  load   |   base    |reg list
#define LDM(reg, list) EMIT(c__ | (0b100<<25) | (0<<24) | (1<<23) | (0<<22) | (0<<21) | (1<<20) | ((reg)<<16) | (list))


// Half Word and signed data transfert construction
#define HWS_REG(Cond, P, U, W, L, Rn, Rd, S, H, Rm)     (Cond | 0b000<<25 | (P)<<24 | (U)<<23 | 0<<22 | (W)<<21 | (L)<<20 | (Rn)<<16 | (Rd)<<12 | 1<<7 | (S)<<6 | (H)<<5 | 1<<4 | (Rm))
#define HWS_OFF(Cond, P, U, W, L, Rn, Rd, S, H, Imm8)   (Cond | 0b000<<25 | (P)<<24 | (U)<<23 | 1<<22 | (W)<<21 | (L)<<20 | (Rn)<<16 | (Rd)<<12 | ((Imm8)&0xf0)<<(8-4) | 1<<7 | (S)<<6 | (H)<<5 | 1<<4 | ((Imm8)&0x0f))

#define LDRSB_IMM8(reg, addr, imm8) EMIT(HWS_OFF(c__, 1, (((imm8)<0)?0:1), 0, 1, addr, reg, 1, 0, abs(imm8)))
#define LDRSH_IMM8(reg, addr, imm8) EMIT(HWS_OFF(c__, 1, (((imm8)<0)?0:1), 0, 1, addr, reg, 1, 1, abs(imm8)))
#define LDRH_IMM8(reg, addr, imm8) EMIT(HWS_OFF(c__, 1,  (((imm8)<0)?0:1), 0, 1, addr, reg, 0, 1, abs(imm8)))
#define LDRH_IMM8_COND(cond, reg, addr, imm8) EMIT(HWS_OFF(cond, 1, (((imm8)<0)?0:1), 0, 1, addr, reg, 0, 1, abs(imm8)))
#define STRH_IMM8(reg, addr, imm8) EMIT(HWS_OFF(c__, 1, (((imm8)<0)?0:1), 0, 0, addr, reg, 0, 1, abs(imm8)))

#define LDRH_REG(reg, addr, rm) EMIT(HWS_REG(c__, 1, 1, 0, 1, addr, reg, 0, 1, rm))
#define STRH_REG(reg, addr, rm) EMIT(HWS_REG(c__, 1, 1, 0, 0, addr, reg, 0, 1, rm))

#define LDRHAI_REG_LSL_IMM5(reg, addr, rm)  EMIT(HWS_REG(c__, 0, 1, 0, 1, addr, reg, 0, 1, rm))
#define STRHAI_REG_LSL_IMM5(reg, addr, rm)  EMIT(HWS_REG(c__, 0, 1, 0, 0, addr, reg, 0, 1, rm))
#define LDRHB_IMM8(reg, addr, imm8)         EMIT(HWS_OFF(c__, 1, (((imm8)<0)?0:1), 1, 1, addr, reg, 0, 1, abs(imm8)))
#define LDRHA_IMM8(reg, addr, imm8)         EMIT(HWS_OFF(c__, 0, (((imm8)<0)?0:1), 0, 1, addr, reg, 0, 1, abs(imm8)))
#define STRHB_IMM8(reg, addr, imm8)         EMIT(HWS_OFF(c__, 1, (((imm8)<0)?0:1), 1, 0, addr, reg, 0, 1, abs(imm8)))
#define STRHA_IMM8(reg, addr, imm8)         EMIT(HWS_OFF(c__, 0, (((imm8)<0)?0:1), 0, 0, addr, reg, 0, 1, abs(imm8)))
#define PUSH16(reg, addr)                   EMIT(HWS_OFF(c__, 1, 0, 1, 0, addr, reg, 0, 1, 2))
#define POP16(reg, addr)                    EMIT(HWS_OFF(c__, 0, 1, 0, 1, addr, reg, 0, 1, 2))

// Mul Long construction
#define MULLONG(Cond, U, A, S, RdHi, RdLo, Rs, Rm)     (Cond | 0b00001<<23 | (U)<<22 | (A)<<21 | (S)<<20 | (RdHi)<<16 | (RdLo)<<12 | (Rs)<<8 | 0b1001<<4 | (Rm))

#define UMULL(RdHi, RdLo, Rs, Rm)   EMIT(MULLONG(c__, 0, 0, 0, RdHi, RdLo, Rs, Rm))
#define SMULL(RdHi, RdLo, Rs, Rm)   EMIT(MULLONG(c__, 1, 0, 0, RdHi, RdLo, Rs, Rm))

// Mul and MulA
#define MULMULA(Cond, A, S, Rd, Rn, Rs, Rm)     (Cond | 0b000000<<22 | (A)<<21 | (S)<<20 | (Rd)<<16 | (Rn)<<12 | (Rs)<<8 | 0b1001<<4 | (Rm))
#define MUL(Rd, Rm, Rn)     EMIT(MULMULA(c__, 0, 0, (Rd), 0, (Rm), (Rn)))
#define MLA(Rd, Rn, Rm, Ra) EMIT(MULMULA(c__, 1, 0, (Rd), (Ra), (Rm), (Rn)))

#define MLS_gen(Cond, Rd, Ra, Rm, Rn)     (Cond | 0b00000110<<20 | (Rd)<<16 | (Ra)<<12 | (Rm)<<8 | 0b1001<<4 | (Rn))
//MLS : Rd = Ra - Rm*Rn
#define MLS(Rd, Rm, Rn, Ra)         EMIT(MLS_gen(c__, Rd, Ra, Rm, Rn))

#define SMUL_16_gen(cond, Rd, Rm, M, N, Rn) (cond | 0b00010110<<20 | (Rd)<<16 | (Rm)<<8 | 1<<7 | (M)<<6 | (N)<<5 | (Rn))
// Signed Mul between Rn[0..15] * Rm[0..15] => Rd
#define SMULBB(Rd, Rn, Rm)  EMIT(SMUL_16_gen(c__, Rd, Rm, 0, 0, Rn))

// SXTB rd, rm ror rot
#define SXTB(rd, rm, rot)   EMIT(c__ | (0b01101010<<20) | (0x0f<<16) | ((rd)<<12) | (rot)<<10 | (0b0111<<4) | (rm))
// UXTB rd, rm ror rot
#define UXTB(rd, rm, rot)   EMIT(c__ | (0b01101110<<20) | (0x0f<<16) | ((rd)<<12) | (rot)<<10 | (0b0111<<4) | (rm))
// SXTH rd, rm ror rot
#define SXTH(rd, rm, rot)   EMIT(c__ | (0b01101011<<20) | (0x0f<<16) | ((rd)<<12) | (rot)<<10 | (0b0111<<4) | (rm))
// UXTH rd, rm ror rot
#define UXTH(rd, rm, rot)   EMIT(c__ | (0b01101111<<20) | (0x0f<<16) | ((rd)<<12) | (rot)<<10 | (0b0111<<4) | (rm))

// UBFX: Unsigned Bit Field Extract: extract any number of bits from Rn, zero extend and put in Rd
#define UBFX(rd, rn, lsb, width)    EMIT(c__ | (0b0111111<<21) | (((width)-1)<<16) | ((rd)<<12) | ((lsb)<<7) | (0b101<<4) | (rn))
// UBFX: Unsigned Bit Field Extract: extract any number of bits from Rn, zero extend and put in Rd
#define UBFX_COND(cond, rd, rn, lsb, width)    EMIT(cond | (0b0111111<<21) | (((width)-1)<<16) | ((rd)<<12) | ((lsb)<<7) | (0b101<<4) | (rn))
// SBFX: Signed Bit Field Extract: extract any number of bits from Rn, zero extend and put in Rd
#define SBFX(rd, rn, lsb, width)    EMIT(c__ | (0b0111101<<21) | (((width)-1)<<16) | ((rd)<<12) | ((lsb)<<7) | (0b101<<4) | (rn))

// BFI: Bit Field Insert: copy any number of low order bit from Rn to any position of Rd
#define BFI(rd, rn, lsb, width) EMIT(c__ | (0b0111110<<21) | (((lsb)+(width)-1)<<16) | ((rd)<<12) | ((lsb)<<7) | (0b001<<4) | (rn))
// BFI_COND: Bit Field Insert with condition: copy any number of low order bit from Rn to any position of Rd
#define BFI_COND(cond, rd, rn, lsb, width) EMIT(cond | (0b0111110<<21) | (((lsb)+(width)-1)<<16) | ((rd)<<12) | ((lsb)<<7) | (0b001<<4) | (rn))

// BFC: Bit Field Clear: clear any number of adjacent bit from Rd
#define BFC(rd, lsb, width) EMIT(c__ | (0b0111110<<21) | (((lsb)+(width)-1)<<16) | ((rd)<<12) | ((lsb)<<7) | (0b001<<4) | 0b1111)
// BFC_COND: Bit Field Clear with condition: clear any number of adjacent bit from Rd
#define BFC_COND(cond, rd, lsb, width) EMIT(cond | (0b0111110<<21) | (((lsb)+(width)-1)<<16) | ((rd)<<12) | ((lsb)<<7) | (0b001<<4) | 0b1111)

// REV: Reverse byte of a 32bits word
#define REV(rd, rm)     EMIT(c__ | (0b01101<<23) | (0<<22) | (0b11<<20) | (0b1111<<16) | ((rd)<<12) | (0b1111<<8) | (0b0011<<4) | (rm))
// REV16: Reverse byte of a 16bits word
#define REV16(rd, rm)   EMIT(c__ | (0b01101<<23) | (0<<22) | (0b11<<20) | (0b1111<<16) | ((rd)<<12) | (0b1111<<8) | (0b1011<<4) | (rm))

#define SSAT_gen(cond, sat_imm, Rd, imm5, sh, Rn) (cond | 0b0110101<<21 | (sat_imm)<<16 | (Rd)<<12 | (imm5)<<7 | (sh)<<6 | 0b01<<4 | (Rn))
// Signed Staturate Rn to 2^(staturate_to-1) into Rd. Optionnaly shift left Rn before saturate
#define SSAT_REG_LSL_IMM5(Rd, saturate_to, Rn, shift)   EMIT(SSAT_gen(c__, (saturate_to)-1, Rd, shift, 0, Rn))
// Signed Staturate Rn to 2^(staturate_to-1) into Rd. Optionnaly shift left Rn before saturate
#define SSAT_REG_LSL_IMM5_COND(cond, Rd, saturate_to, Rn, shift)   EMIT(SSAT_gen((cond), (saturate_to)-1, Rd, shift, 0, Rn))

#define USAT_gen(cond, sat_imm, Rd, imm5, sh, Rn) (cond | 0b0110111<<21 | (sat_imm)<<16 | (Rd)<<12 | (imm5)<<7 | (sh)<<6 | 0b01<<4 | (Rn))
// Unsigned Staturate Rn to 2^(staturate_to-1) into Rd. Optionnaly shift left Rn before saturate
#define USAT_REG_LSL_IMM5(Rd, saturate_to, Rn, shift)   EMIT(SSAT_gen(c__, (saturate_to), Rd, shift, 0, Rn))

#define MRS_gen(cond, Rd)   (cond | 0b00010<<23 | 0b1111<<16 | (Rd)<<12)
// Load MRS ASPR flags
#define MRS_aspr(Rd)    EMIT(MRS_gen(c__, Rd))

#define MSR_gen(cond, mask, Rn) (cond | 0b00010<<23 | 0b10<<20 | (mask)<<18 | 0b1111<<12 | (Rn))
// Store MRS ASPR nzcvq flags from Rn
#define MSR_nzcvq(Rn)   EMIT(MSR_gen(c__, 0b10, Rn))
#define MSR_imm_gen(cond, mask, imm12) (cond | 0b00110<<23 | 0b10<<20 | (mask)<<18 | 0b1111<<12 | (imm12))
#define MSR_nzcvq_0()   EMIT(MSR_imm_gen(c__, 0b10, 0))

#define LDREXD_gen(cond, Rn, Rt) (cond | 0b000<<25 | 0b11011<<20 | (Rn)<<16 | (Rt)<<12 | 0b1111<<8 | 0b1001<<4 | 0b1111)
// Load Exclusive Rt/Rt+1 from Rn (tagging the memory)
#define LDREXD(Rt, Rn)  EMIT(LDREXD_gen(c__, Rn, Rt))
#define LDREXD_COND(cond, Rt, Rn)  EMIT(LDREXD_gen(cond, Rn, Rt))

#define STREXD_gen(cond, Rd, Rn, Rt)   (cond | 0b000<<25 | 0b11010<<20 | (Rn)<<16 | (Rd)<<12 | 0b1111<<8 | 0b1001<<4 | (Rt))
// Store Exclusive Rt/Rt+1 to Rn, with result in Rd if tag is ok (Rd!=Rn && Rd!=Rt && Rd!=Rt+1), Rd==1 if store failed
#define STREXD(Rd, Rt, Rn)  EMIT(STREXD_gen(c__, Rd, Rn, Rt))
#define STREXD_COND(cond, Rd, Rt, Rn)  EMIT(STREXD_gen(cond, Rd, Rn, Rt))

#define LDREX_gen(cond, Rn, Rt)     (cond | 0b0001100<<21 | 1<<20 | (Rn)<<16 | (Rt)<<12 | 0b1111<<8 | 0b1001<<4 | 0b1111)
// Load Exclusive Rt from Rn (tagging the memory)
#define LDREX(Rt, Rn)       EMIT(LDREX_gen(c__, Rn, Rt))
#define LDREX_COND(cond, Rt, Rn)       EMIT(LDREX_gen(cond, Rn, Rt))

#define STREX_gen(cond, Rd, Rn, Rt) (cond | 0b0001100<<21 | 0<<20 | (Rn)<<16 | (Rd)<<12 | 0b1111<<8 | 0b1001<<4 | (Rt))
// Store Exclusive Rt to Rn, with result in Rd=0 if tag is ok, Rd==1 if store failed (Rd!=Rn && Rd!=Rt)
#define STREX(Rd, Rt, Rn)   EMIT(STREX_gen(c__, Rd, Rn, Rt))
#define STREX_COND(cond, Rd, Rt, Rn)   EMIT(STREX_gen(cond, Rd, Rn, Rt))

#define LDREXB_gen(cond, Rn, Rt)        (cond | 0b0001110<<21 | 1<<20 | (Rn)<<16 | (Rt)<<12 | 0b1111<<8 | 0b1001<<4 | 0b1111)
// Load Exclusive Byte Rt from Rn (tagging the memory)
#define LDREXB(Rt, Rn)      EMIT(LDREXB_gen(c__, Rn, Rt))
#define LDREXB_COND(cond, Rt, Rn)      EMIT(LDREXB_gen(cond, Rn, Rt))

#define STREXB_gen(cond, Rd, Rn, Rt)    (cond | 0b0001110<<21 | 0<<20 | (Rn)<<16 | (Rd)<<12 | 0b1111<<8 | 0b1001<<4 | (Rt))
// Store Exclusive byte Rt to Rn, with result in Rd=0 if tag is ok, Rd==1 if store failed (Rd!=Rn && Rd!=Rt)
#define STREXB(Rd, Rt, Rn)  EMIT(STREXB_gen(c__, Rd, Rn, Rt))
#define STREXB_COND(cond, Rd, Rt, Rn)  EMIT(STREXB_gen(cond, Rd, Rn, Rt))

#define LDREXH_gen(cond, Rn, Rt)        (cond | 0b0001111<<21 | 1<<20 | (Rn)<<16 | (Rt)<<12 | 0b1111<<8 | 0b1001<<4 | 0b1111)
// Load Exclusive Half-Word Rt from Rn (tagging the memory)
#define LDREXH(Rt, Rn)                  EMIT(LDREXH_gen(c__, Rn, Rt))
#define LDREXH_COND(cond, Rt, Rn)       EMIT(LDREXH_gen(cond, Rn, Rt))

#define STREXH_gen(cond, Rd, Rn, Rt)    (cond | 0b0001111<<21 | 0<<20 | (Rn)<<16 | (Rd)<<12 | 0b1111<<8 | 0b1001<<4 | (Rt))
// Store Exclusive Half-Word Rt to Rn, with result in Rd=0 if tag is ok, Rd==1 if store failed (Rd!=Rn && Rd!=Rt)
#define STREXH(Rd, Rt, Rn)              EMIT(STREXH_gen(c__, Rd, Rn, Rt))
#define STREXH_COND(cond, Rd, Rt, Rn)   EMIT(STREXH_gen(cond, Rd, Rn, Rt))

// Count leading 0 bit of Rm, store result in Rd
#define CLZ(Rd, Rm)  EMIT(c__ | 0b00010110<<20 | 0b1111<<16 | (Rd)<<12 | 0b1111<<8 | 0b0001<<4 | (Rm))
// Count leading 0 bit of Rm, store result in Rd with cond
#define CLZ_COND(cond, Rd, Rm)  EMIT(cond | 0b00010110<<20 | 0b1111<<16 | (Rd)<<12 | 0b1111<<8 | 0b0001<<4 | (Rm))
// Reverse bits of Rm, store result in Rd
#define RBIT(Rd, Rm) EMIT(c__ | 0b01101111<<20 | 0b1111<<16 | (Rd)<<12 | 0b1111<<8 | 0b0011<<4 | (Rm))
// Reverse bits of Rm, store result in Rd with cond
#define RBIT_COND(cond, Rd, Rm) EMIT(cond | 0b01101111<<20 | 0b1111<<16 | (Rd)<<12 | 0b1111<<8 | 0b0011<<4 | (Rm))

#define PLD_gen(U, R, Rn, Imm5, type, Rm) (0b1111<<28 | 0b0111<<24 | (U)<<23 | (R)<<22 | 0b01<<20 | (Rn)<<16 | 0b1111<<12 | (Imm5)<<7 | (type)<<5 | (Rm))
// Preload Cache Rn+Rm
#define PLD(Rn, Rm) EMIT(PLD_gen(1, 1, Rn, 0, 0, Rm))
// Preload Cache Rn-Rm
#define PLDn(Rn, Rm) EMIT(PLD_gen(0, 1, Rn, 0, 0, Rm))

#define DMB_gen(opt)    (0b1111<<28 | 0b01010111<<20 | 0b1111<<16 | 0b1111<<12 | 0b0000<<8 | 0b0101<<4 | (opt))
// Data memory barrier Inner Sharable
#ifdef PANDORA
// The Pandora is single core and ordered, DMB is not usefull here so just emit nothing
#define DMB_ISH()
#else
#define DMB_ISH()   EMIT(DMB_gen(0b1011))
#endif

#define DSB_gen(opt)    (0b1111<<28 | 0b01010111<<20 | 0b1111<<16 | 0b1111<<12 | 0b0000<<8 | 0b0100<<4 | (opt))
// Data memory barrier Inner Sharable
#ifdef PANDORA
// The Pandora is single core and ordered, DSB is not usefull here so just emit nothing
#define DSB_ISH()
#else
#define DSB_ISH()   EMIT(DSB_gen(0b1011))
#endif

#define SWP_gen(cond, B, Rn, Rt, Rt2)   (cond | 0b0001<<24 | (B)<<22 | (Rn)<<16 | (Rt)<<12 | 0b1001<<4 | (Rt2))
// SWAP (atomic) [Rn]->Rt2 / Rt->[Rn], Rt can be same as Rt2
#define SWP(Rt, Rt2, Rn)    EMIT(SWP_gen(c__, 0, Rn, Rt, Rt2))
// SWAP Byte (atomic) [Rn]->Rt2 / Rt->[Rn], Rt can be same as Rt2, Byte is zero extended
#define SWPB(Rt, Rt2, Rn)   EMIT(SWP_gen(c__, 1, Rn, Rt, Rt2))

#define SDIV_gen(cond, Rd, Rm, Rn)  (cond | 0b0111<<24 | 0b0001<<20 | (Rd)<<16 | 0b1111<<12 | (Rm)<<8 | 0b0001<<4 | (Rn))
// Signed Div Rd <- Rn/Rm
#define SDIV(Rd, Rn, Rm)    EMIT(SDIV_gen(c__, Rd, Rm, Rn))

#define UDIV_gen(cond, Rd, Rm, Rn)  (cond | 0b0111<<24 | 0b0011<<20 | (Rd)<<16 | 0b1111<<12 | (Rm)<<8 | 0b0001<<4 | (Rn))
// Unsigned Div Rd <- Rn/Rm
#define UDIV(Rd, Rn, Rm)    EMIT(UDIV_gen(c__, Rd, Rm, Rn))

// Yield
#define YIELD(cond) EMIT(cond | 0b00110010<<20 | 0b1111<<12 | 1)

// UDF
#define UDF(imm16)    EMIT(0b1110<<28 | 0b011<<25 | 0b11111<<20 | (((imm16)>>4)&0xfff) | 0b1111<<4 | ((imm16)&0xf))

// Move from FPSCR to Arm register
#define VMRS(Rt)    EMIT(c__ | (0b1110<<24) | (0b1111<<20) | (0b0001<<16) | ((Rt)<<12) | (0b1010<<8) | (0b0001<<4) | (0b0000))
// Move to FPSCR from Arm register
#define VMSR(Rt)    EMIT(c__ | (0b1110<<24) | (0b1110<<20) | (0b0001<<16) | ((Rt)<<12) | (0b1010<<8) | (0b0001<<4) | (0b0000))
// Move to FPSCR from Arm register with cond
#define VMSR_cond(cond, Rt)    EMIT(cond | (0b1110<<24) | (0b1110<<20) | (0b0001<<16) | ((Rt)<<12) | (0b1010<<8) | (0b0001<<4) | (0b0000))
// Move to FPSCR from Arm flags APSR
#define VMRS_APSR()    VMRS(15)

// Move between Rt to Sm
#define VMOVtoV(Sm, Rt) EMIT(c__ | (0b1110<<24) | (0b000<<21) | (0<<20) | ((((Sm)&0b11110)>>1)<<16) | ((Rt)<<12) | (0b1010<<8) | (((Sm)&1)<<7) |(0b00<<6) | (1<<4))
// Move between Rt to Sm with condition
#define VMOVtoVcond(cond, Sm, Rt) EMIT(cond | (0b1110<<24) | (0b000<<21) | (0<<20) | ((((Sm)&0b11110)>>1)<<16) | ((Rt)<<12) | (0b1010<<8) | (((Sm)&1)<<7) |(0b00<<6) | (1<<4))
// Move between Sm to Rt
#define VMOVfrV(Rt, Sm) EMIT(c__ | (0b1110<<24) | (0b000<<21) | (1<<20) | ((((Sm)&0b11110)>>1)<<16) | ((Rt)<<12) | (0b1010<<8) | (((Sm)&1)<<7) |(0b00<<6) | (1<<4))

// Move between Rt to Sm and Rt2 to Sm+1 (Sm cannot be 31!, rt and Rt2 can be the same)
#define VMOVtoV_64(Sm, Rt, Rt2) EMIT(c__ | (0b1100<<24) | (0b010<<21) | (0<<20) | ((Rt2)<<16) | ((Rt)<<12) | (0b1010<<8) | (0b00<<6) | (1<<4) | ((Sm)&1)>>1 | ((Sm)&0b10000)<<1)
// Move between Sm to Rt and Sm+1 to Rt2 (Sm cannot be 31!, Rt and Rt2 must be different)
#define VMOVfrV_64(Rt, Rt2, Sm) EMIT(c__ | (0b1100<<24) | (0b010<<21) | (1<<20) | ((Rt2)<<16) | ((Rt)<<12) | (0b1010<<8) | (0b00<<6) | (1<<4) | ((Sm)&1)>>1 | ((Sm)&0b10000)<<1)

// Move between Rt/Rt2 to Dm
#define VMOVtoV_D(Dm, Rt, Rt2) EMIT(c__ | (0b1100<<24) | (0b010<<21) | (0<<20) | ((Rt2)<<16) | ((Rt)<<12) | (0b1011<<8) | (0b00<<6) | (1<<4) | ((Dm)&0b1111) | (((Dm)>>4)&1)<<5)
// Move between Dm and Rt/Rt2 (Rt and Rt2 must be different)
#define VMOVfrV_D(Rt, Rt2, Dm) EMIT(c__ | (0b1100<<24) | (0b010<<21) | (1<<20) | ((Rt2)<<16) | ((Rt)<<12) | (0b1011<<8) | (0b00<<6) | (1<<4) | ((Dm)&0b1111) | (((Dm)>>4)&1)<<5)

// Move between Dd and Dm
#define VMOV_64(Dd, Dm)     EMIT(c__ | (0b11101<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (0b01<<6) | ((((Dm)>>4)&1)<<5) | ((Dm)&15))
// Move between Sd and Sm
#define VMOV_32(Sd, Sm)     EMIT(c__ | (0b11101<<23) | (((Sd)&1)<<22) | (0b11<<20) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (0<<8) | (0b01<<6) | (((Sm)&1)<<5) | (((Sm)>>1)&15))
// Move with condition between Dd and Dm
#define VMOVcond_64(cond, Dd, Dm)     EMIT(cond | (0b11101<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (0b01<<6) | ((((Dm)>>4)&1)<<5) | ((Dm)&15))
// Move with condition between Sd and Sm
#define VMOVcond_32(cond, Sd, Sm)     EMIT(cond | (0b11101<<23) | (((Sd)&1)<<22) | (0b11<<20) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (0<<8) | (0b01<<6) | (((Sm)&1)<<5) | (((Sm)>>1)&15))
// Move between Sd and Imm
#define VMOV_i_32(Sd, Imm8)     EMIT(c__ | (0b11101<<23) | (((Sd)&1)<<22) | (0b11<<20) | ((((Imm8)>>4)&0xf)<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (0<<8) | (0b00<<6) | ((Imm8)&15))
// Move between Dd and Imm
#define VMOV_i_64(Dd, Imm8)     EMIT(c__ | (0b11101<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | ((((Imm8)>>4)&0xf)<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (0b00<<6) | ((Imm8)&15))

#define VMOVtoDx_gen(Vd, D, Rt, opc1, opc2)   (0b1110<<24 | 0<<23 | (opc1)<<21 | (Vd)<<16 | (Rt)<<12 | 0b1011<<8 | (D)<<7 | (opc2)<<5 | 1<<4)
// Move between Rt and Dm[x]
#define VMOVtoDx_32(Dm, x, Rt) EMIT(c__ | VMOVtoDx_gen((Dm)&15, ((Dm)>>4)&1, Rt, x, 0))
// Move between Rt and Dm[x]
#define VMOVtoDx_16(Dm, x, Rt) EMIT(c__ | VMOVtoDx_gen((Dm)&15, ((Dm)>>4)&1, Rt, (x)>>1, ((x)&1)<<1 | 1))
// Move between Rt and Dm[x]
#define VMOVtoDx_8(Dm, x, Rt)  EMIT(c__ | VMOVtoDx_gen((Dm)&15, ((Dm)>>4)&1, Rt, 2| (x)>>2, (x)&3))

#define VMOVfrDx_gen(Vn, N, Rt, U, opc1, opc2) (0b1110<<24 | (U)<<23 | (opc1)<<21 | 1<<20 | (Vn)<<16 | (Rt)<<12 | 0b1011<<8 | (N)<<7 | (opc2)<<5 | 1<<4)
// Move between Dn[x] and Rt
#define VMOVfrDx_32(Rt, Dn, x) EMIT(c__ | VMOVfrDx_gen((Dn)&15, ((Dn)>>4)&1, Rt, 0, x, 0))
// Move between Dn[x] and Rt
#define VMOVfrDx_U16(Rt, Dn, x) EMIT(c__ | VMOVfrDx_gen((Dn)&15, ((Dn)>>4)&1, Rt, 1, (x)>>1, ((x)&1)<<1 | 1))
// Move between Dn[x] and Rt
#define VMOVfrDx_U8(Rt, Dn, x) EMIT(c__ | VMOVfrDx_gen((Dn)&15, ((Dn)>>4)&1, Rt, 1, 2| (x)>>2, (x)&3))
// Move between Dn[x] and Rt
#define VMOVfrDx_S16(Rt, Dn, x) EMIT(c__ | VMOVfrDx_gen((Dn)&15, ((Dn)>>4)&1, Rt, 0, (x)>>1, ((x)&1)<<1 | 1))
// Move between Dn[x] and Rt
#define VMOVfrDx_S8(Rt, Dn, x) EMIT(c__ | VMOVfrDx_gen((Dn)&15, ((Dn)>>4)&1, Rt, 0, 2| (x)>>2, (x)&3))

// Load from memory to double  VLDR Dd, [Rn, #+/-imm8], imm8&3 ignored!
#define VLDR_64(Dd, Rn, Imm8)    EMIT(c__ | (0b1101<<24) | (((Imm8)<0)?0:1)<<23 | ((((Dd)>>4)&1)<<22) | (1<<20) | ((Rn)<<16) | (((Dd)&15)<<12) | (0b1011<<8) | ((abs(Imm8)>>2)&255))
// Load from memory to single  VLDR Sd, [Rn, #+/-imm8], imm8&3 ignored!
#define VLDR_32(Sd, Rn, Imm8)    EMIT(c__ | (0b1101<<24) | (((Imm8)<0)?0:1)<<23 | (((Sd)&1)<<22) | (1<<20) | ((Rn)<<16) | ((((Sd)>>1)&15)<<12) | (0b1010<<8) | ((abs(Imm8)>>2)&255))

// Store to memory to double  VSTR Dd, [Rn, #+/-imm8], imm8&3 ignored!
#define VSTR_64(Dd, Rn, Imm8)    EMIT(c__ | (0b1101<<24) | (((Imm8)<0)?0:1)<<23 | ((((Dd)>>4)&1)<<22) | (0<<20) | ((Rn)<<16) | (((Dd)&15)<<12) | (0b1011<<8) | ((abs(Imm8)>>2)&255))
// Store to memory to single  VSTR Sd, [Rn, #+/-imm8], imm8&3 ignored!
#define VSTR_32(Sd, Rn, Imm8)    EMIT(c__ | (0b1101<<24) | (((Imm8)<0)?0:1)<<23 | (((Sd)&1)<<22) | (0<<20) | ((Rn)<<16) | ((((Sd)>>1)&15)<<12) | (0b1010<<8) | ((abs(Imm8)>>2)&255))

#define VSTM_64_gen(cond, P, U, D, W, Rn, Vd, imm8)	(cond | 0b110<<25 | (P)<<24 | (U)<<23 | (D)<<22 | (W)<<21 | (Rn)<<16 | (Vd)<<12 | 0b1011<<8 | (imm8))
// Store a sery of Dx, post imcrementing Rn
#define VSTM_64_W(Dd, Rn)	    EMIT(VSTM_64_gen(c__, 0, 1, ((Dd)>>4)&1, 1, Rn, (Dd)&15, 2))

// Convert from single Sm to double Dd
#define VCVT_F64_F32(Dd, Sm)    EMIT(c__ | (0b1110<<24) | (1<<23) |  ((((Dd)>>4)&1)<<22) | (0b11<<20) | (0b0111<<16) | (((Dd)&15)<<12) | (0b101<<9) | (0<<8) | (0b11<<6) | (((Sm)&1)<<5) | (0<<4) | (((Sm)>>1)&15))
// Convert from double Dm to single Sd
#define VCVT_F32_F64(Sd, Dm)    EMIT(c__ | (0b1110<<24) | (1<<23) |  (((Sd)&1)<<22) | (0b11<<20) | (0b0111<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (1<<8) | (0b11<<6) | ((((Dm)>>4)&1)<<5) | (0<<4) | ((Dm)&15))

// Convert from double Dm to int32 Sd, with Round toward Zero mode
#define VCVT_S32_F64(Sd, Dm)    EMIT(c__ | (0b1110<<24) | (1<<23) | (((Sd)&1)<<22) | (0b111<<19) | (0b101<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (1<<8) | (1<<7) | (1<<6) | ((((Dm)>>4)&1)<<5) | ((Dm)&15) )
// Convert from double Dm to uint32 Sd, with Round toward Zero mode
#define VCVT_U32_F64(Sd, Dm)    EMIT(c__ | (0b1110<<24) | (1<<23) | (((Sd)&1)<<22) | (0b111<<19) | (0b100<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (1<<8) | (1<<7) | (1<<6) | ((((Dm)>>4)&1)<<5) | ((Dm)&15) )
// Convert from double Dm to int32 Sd, with Round selection from FPSCR
#define VCVTR_S32_F64(Sd, Dm)   EMIT(c__ | (0b1110<<24) | (1<<23) | (((Sd)&1)<<22) | (0b111<<19) | (0b101<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (1<<8) | (0<<7) | (1<<6) | ((((Dm)>>4)&1)<<5) | ((Dm)&15) )
// Convert from int32 Sm to double Dd
#define VCVT_F64_S32(Dd, Sm)    EMIT(c__ | (0b1110<<24) | (1<<23) | ((((Dd)>>4)&1)<<22) | (0b111<<19) | (0b000<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (1<<7) | (1<<6) | (((Sm)&1)<<5) | (((Sm)>>1)&15) )
// Convert from uint32 Sm to double Dd
#define VCVT_F64_U32(Dd, Sm)    EMIT(c__ | (0b1110<<24) | (1<<23) | ((((Dd)>>4)&1)<<22) | (0b111<<19) | (0b000<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (0<<7) | (1<<6) | (((Sm)&1)<<5) | (((Sm)>>1)&15) )
// Convert from single Sm to int32 Sd, with Round toward Zero mode
#define VCVT_S32_F32(Sd, Sm)    EMIT(c__ | (0b1110<<24) | (1<<23) | (((Sd)&1)<<22) | (0b111<<19) | (0b101<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (0<<8) | (1<<7) | (1<<6) | (((Sm)&1)<<5) | (((Sm)>>1)&15) )
// Convert from single Sm to int32 Sd, with Round selection from FPSCR
#define VCVTR_S32_F32(Sd, Sm)   EMIT(c__ | (0b1110<<24) | (1<<23) | (((Sd)&1)<<22) | (0b111<<19) | (0b101<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (0<<8) | (0<<7) | (1<<6) | (((Sm)&1)<<5) | (((Sm)>>1)&15) )
// Convert from int32 Sm to single Dd
#define VCVT_F32_S32(Sd, Sm)    EMIT(c__ | (0b1110<<24) | (1<<23) | (((Sd)&1)<<22) | (0b111<<19) | (0b000<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (0<<8) | (1<<7) | (1<<6) | (((Sm)&1)<<5) | (((Sm)>>1)&15) )

#define VCVT_FP_gen(cond, D, op, U, Vd, sf, sx, i, imm4)   (cond | 0b1110<<24 | 1<<23 | (D)<<22 | 0b11<<20 | 1<<19 | (op)<<18 | 1<<17 | (U)<<16 | (Vd)<<12 | 0b101<<9 | (sf)<<8 | (sx)<<7 | 1<<6 | (i)<<5 | (imm4))
// Inplace convert from F64 to S16. Rounding is not selectable
#define VCVT_S16_F64(Dd)    EMIT(VCVT_FP_gen(c__, ((Dd)>>4)&1, 1, 0, (Dd)&15, 1, 0, 16&1, (16>>1)&15))

#define VRINT_FP_gen(D, RM, Vd, size, M, Vm)    (0b111111101<<23 | (D)<<22 | 0b111<<19 | (RM)<<16 | (Vd)<<12 | 0b10<<10 | (size)<<8 | 1<<6 | (M)<<4 | (Vm))
// Round floating-point to integer to Nearest with Ties to Away
#define VRINTA_F32(Sd, Sm)  EMIT(VRINT_FP_gen(((Sd)&1), 0b00, ((Sd)>>1)&15, 0b10, (Sm)&1, ((Sm)>>1)&15))
// Round floating-point to integer to Nearest with Ties to Away
#define VRINTA_F64(Dd, Dm)  EMIT(VRINT_FP_gen(((Dd)>>4)&1, 0b00, (Dd)&15, 0b11, ((Dm)>>4)&1, (Dm)&15))
// Round floating-point to integer towards -Infinity
#define VRINTM_F32(Sd, Sm)  EMIT(VRINT_FP_gen(((Sd)&1), 0b11, ((Sd)>>1)&15, 0b10, (Sm)&1, ((Sm)>>1)&15))
// Round floating-point to integer towards -Infinity
#define VRINTM_F64(Dd, Dm)  EMIT(VRINT_FP_gen(((Dd)>>4)&1, 0b11, (Dd)&15, 0b11, ((Dm)>>4)&1, (Dm)&15))
// Round floating-point to integer to Nearest
#define VRINTN_F32(Sd, Sm)  EMIT(VRINT_FP_gen(((Sd)&1), 0b01, ((Sd)>>1)&15, 0b10, (Sm)&1, ((Sm)>>1)&15))
// Round floating-point to integer to Nearest
#define VRINTN_F64(Dd, Dm)  EMIT(VRINT_FP_gen(((Dd)>>4)&1, 0b01, (Dd)&15, 0b11, ((Dm)>>4)&1, (Dm)&15))
// Round floating-point to integer towards +Infinity
#define VRINTP_F32(Sd, Sm)  EMIT(VRINT_FP_gen(((Sd)&1), 0b10, ((Sd)>>1)&15, 0b10, (Sm)&1, ((Sm)>>1)&15))
// Round floating-point to integer towards +Infinity
#define VRINTP_F64(Dd, Dm)  EMIT(VRINT_FP_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 0b11, ((Dm)>>4)&1, (Dm)&15))

#define VRINTR_gen(cond, D, Vd, size, M, Vm)    (cond | 0b11101<<23 | (D)<<22 | 0b11<<20 | 0b110<<16 | (Vd)<<12 | 0b10<<10 | (size)<<8 | 1<<6 | (M)<<5 | (Vm))
// Round floating-point to integer rounds a floating-point value to an integral floating-point value of the same size using the rounding mode specified in the FPSCR
#define VRINTR_F32(Sd, Sm)  EMIT(VRINTR_gen(c__, (Sd)&1, ((Sd)>>1)&15, 0b10, (Sm)&1, ((Sm)>>1)&15))
// Round floating-point to integer rounds a floating-point value to an integral floating-point value of the same size using the rounding mode specified in the FPSCR
#define VRINTR_F64(Dd, Dm)  EMIT(VRINTR_gen(c__, ((Dd)>>4)&1, (Dd)&15, 0b11, ((Dm)>>4)&1, (Dm)&15))

#define VRINTX_gen(cond, D, Vd, size, M, Vm)    (cond | 0b11101<<23 | (D)<<22 | 0b11<<20 | 0b111<<16 | (Vd)<<12 | 0b10<<10 | (size)<<8 | 1<<6 | (M)<<5 | (Vm))
// Round floating-point to integer rounds a floating-point value to an integral floating-point value of the same size using the rounding mode specified in the FPSCR with eXecptions if inexact
#define VRINTX_F32(Sd, Sm)  EMIT(VRINTX_gen(c__, (Sd)&1, ((Sd)>>1)&15, 0b10, (Sm)&1, ((Sm)>>1)&15))
// Round floating-point to integer rounds a floating-point value to an integral floating-point value of the same size using the rounding mode specified in the FPSCR with eXecptions if inexact
#define VRINTX_F64(Dd, Dm)  EMIT(VRINTX_gen(c__, ((Dd)>>4)&1, (Dd)&15, 0b11, ((Dm)>>4)&1, (Dm)&15))

#define VRINTZ_gen(cond, D, Vd, size, M, Vm)    (cond | 0b11101<<23 | (D)<<22 | 0b11<<20 | 0b110<<16 | (Vd)<<12 | 0b10<<10 | (size)<<8 | 1<<7 | 1<<6 | (M)<<5 | (Vm))
// Round floating-point to integer towards Zero
#define VRINTZ_F32(Sd, Sm)  EMIT(VRINTZ_gen(c__, (Sd)&1, ((Sd)>>1)&15, 0b10, (Sm)&1, ((Sm)>>1)&15))
// Round floating-point to integer towards Zero
#define VRINTZ_F64(Dd, Dm)  EMIT(VRINTZ_gen(c__, ((Dd)>>4)&1, (Dd)&15, 0b11, ((Dm)>>4)&1, (Dm)&15))

// Mutiply F64 Dd = Dn*Dm
#define VMUL_F64(Dd, Dn, Dm)    EMIT(c__ | (0b1110<<24) | (0<<23) | ((((Dd)>>4)&1)<<22) | (0b10<<20) | (((Dn)&15)<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | ((((Dn)>>4)&1)<<7) | ((((Dm)>>4)&1)<<5) | ((Dm)&15) )

// Divide F64 Dd = Dn/Dm
#define VDIV_F64(Dd, Dn, Dm)    EMIT(c__ | (0b1110<<24) | (1<<23) | ((((Dd)>>4)&1)<<22) | (0b00<<20) | (((Dn)&15)<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | ((((Dn)>>4)&1)<<7) | ((((Dm)>>4)&1)<<5) | ((Dm)&15) )

// Add F64 Dd = Dn + Dm
#define VADD_F64(Dd, Dn, Dm)    EMIT(c__ | (0b1110<<24) | (0<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (((Dn)&15)<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | ((((Dn)>>4)&1)<<7) | (0<<6) | ((((Dm)>>4)&1)<<5) | ((Dm)&15) )

// Sub F64 Dd = Dn + Dm
#define VSUB_F64(Dd, Dn, Dm)    EMIT(c__ | (0b1110<<24) | (0<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (((Dn)&15)<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | ((((Dn)>>4)&1)<<7) | (1<<6) | ((((Dm)>>4)&1)<<5) | ((Dm)&15) )

// Mutiply F32 Sd = Sn*Sm
#define VMUL_F32(Sd, Sn, Sm)    EMIT(c__ | (0b1110<<24) | (0<<23) | (((Sd)&1)<<22) | (0b10<<20) | ((((Sn)>>1)&15)<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (0<<8) | (((Sn)&1)<<7) | (((Sm)&1)<<5) | (((Sm)>>1)&15) )

// Divide F32 Sd = Sn/Sm
#define VDIV_F32(Sd, Sn, Sm)    EMIT(c__ | (0b1110<<24) | (1<<23) | (((Sd)&1)<<22) | (0b00<<20) | ((((Sn)>>1)&15)<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (0<<8) | (((Sn)&1)<<7) | (((Sm)&1)<<5) | (((Sm)>>1)&15) )

// ADD F32 Sd = Sn + Sm
#define VADD_F32(Sd, Sn, Sm)    EMIT(c__ | (0b1110<<24) | (0<<23) | (((Sd)&1)<<22) | (0b11<<20) | ((((Sn)>>1)&15)<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (0<<8) | (((Sn)&1)<<7) | (0<<6) | (((Sm)&1)<<5) | (((Sm)>>1)&15) )

// Sub F32 Sd = Sn + Sm
#define VSUB_F32(Sd, Sn, Sm)    EMIT(c__ | (0b1110<<24) | (0<<23) | (((Sd)&1)<<22) | (0b11<<20) | ((((Sn)>>1)&15)<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (0<<8) | (((Sn)&1)<<7) | (1<<6) | (((Sm)&1)<<5) | (((Sm)>>1)&15) )

// Cmp between 2 double Dd and Dm
#define VCMP_F64(Dd, Dm)    EMIT(c__ | (0b1110<<24) | (1<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (0b0100<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (0<<7) | (1<<6) | ((((Dm)>>4)&1)<<5) | ((Dm)&15) )
// Cmp between 1 double Dd and 0.0
#define VCMP_F64_0(Dd)      EMIT(c__ | (0b1110<<24) | (1<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (0b0101<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (0<<7) | (1<<6) | (0<<5) | (0) )
// Cmp between 2 single Sd and Sm
#define VCMP_F32(Sd, Sm)    EMIT(c__ | (0b1110<<24) | (1<<23) | (((Sd)&1)<<22) | (0b11<<20) | (0b0100<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (0<<8) | (0<<7) | (1<<6) | (((Sm)&1)<<5) | (((Sm)>>1)&15) )
// Cmp between 1 single Sd and 0.0
#define VCMP_F32_0(Sd)      EMIT(c__ | (0b1110<<24) | (1<<23) | (((Sd)&1)<<22) | (0b11<<20) | (0b0101<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (0<<8) | (0<<7) | (1<<6) | (0<<5) | (0) )

// Neg F64 Dd = - Dm
#define VNEG_F64(Dd, Dm)    EMIT(c__ | (0b1110<<24) | (1<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (0b0001<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (0b01<<6) | ((((Dm)>>4)&1)<<5) | ((Dm)&15) )
// Neg F64 Dd = - Dm
#define VNEG_F64_cond(cond, Dd, Dm)    EMIT((cond) | (0b1110<<24) | (1<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (0b0001<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (0b01<<6) | ((((Dm)>>4)&1)<<5) | ((Dm)&15) )
// Neg F32 Dd = - Dm
#define VNEG_F32(Sd, Sm)    EMIT(c__ | (0b1110<<24) | (1<<23) | (((Sd)&1)<<22) | (0b11<<20) | (0b0001<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (0<<8) | (0b01<<6) | (((Sm)&1)<<5) | (((Sm)>>1)&15) )
// Neg F32 Dd = - Dm
#define VNEG_F32_cond(cond, Sd, Sm)    EMIT((cond) | (0b1110<<24) | (1<<23) | (((Sd)&1)<<22) | (0b11<<20) | (0b0001<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (0<<8) | (0b01<<6) | (((Sm)&1)<<5) | (((Sm)>>1)&15) )

// Sqrt F64 Dd = - Dm
#define VSQRT_F64(Dd, Dm)    EMIT(c__ | (0b1110<<24) | (1<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (0b0001<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (0b11<<6) | ((((Dm)>>4)&1)<<5) | ((Dm)&15) )
// Sqrt F32 Dd = - Dm
#define VSQRT_F32(Sd, Sm)    EMIT(c__ | (0b1110<<24) | (1<<23) | (((Sd)&1)<<22) | (0b11<<20) | (0b0001<<16) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (0<<8) | (0b11<<6) | (((Sm)&1)<<5) | (((Sm)>>1)&15) )

// Abs Dd = |Dm|
#define VABS_F64(Dd, Dm)     EMIT(c__ | (0b11101<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (0b11<<6) | ((((Dm)>>4)&1)<<5) | ((Dm)&15))
// Abs Dd = |Dm|
#define VABS_F32(Sd, Sm)     EMIT(c__ | (0b11101<<23) | (((Sd)&1)<<22) | (0b11<<20) | ((((Sd)>>1)&15)<<12) | (0b101<<9) | (0<<8) | (0b11<<6) | (((Sm)&1)<<5) | (((Sm)>>1)&15))

// MLS Dd = Dd - Dn*Dm
#define VMLS_F64(Dd, Dn, Dm)    EMIT(c__ | (0b1110<<24) | (0<<23) | ((((Dd)>>4)&1)<<22) | (0b00<<20) | (((Dn)&15)<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | ((((Dn)>>4)&1)<<7) | 1<<6 | ((((Dm)>>4)&1)<<5) | ((Dm)&15) )
// MLA Dd = Dd + Dn*Dm
#define VMLA_F64(Dd, Dn, Dm)    EMIT(c__ | (0b1110<<24) | (0<<23) | ((((Dd)>>4)&1)<<22) | (0b00<<20) | (((Dn)&15)<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | ((((Dn)>>4)&1)<<7) | 0<<6 | ((((Dm)>>4)&1)<<5) | ((Dm)&15) )

// NEON

// L is 1 for VLD1, 0 for VST1 Dd is V:Vd, type:0b0111=64, 0b1010=128, 0b0110=192, 0b0010=256, size:0=8,1=16,2=32,3=64, align:"4<<align", wback=rm!=15, reg_index:rm!=13&&rm!=15
#define Vxx1gen(L, D, Rn, Vd, type, size, align, Rm) (0b1111<<28 | 0b0100<<24 | 0<<23 | (D)<<22 | (L)<<21 | 0<<20 | (Rn)<<16 | (Vd)<<12 | (type)<<8 | (size)<<6 | (align)<<4 | (Rm))
// Load [Rn] => Dd/Dd+1. Align is 4
#define VLD1Q_32(Dd, Rn) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 2, 0, 15))
// Load [Rn] => Dd. Align is 4
#define VLD1_32(Dd, Rn) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b0111, 2, 0, 15))
// Load [Rn]! => Dd. Align is 4
#define VLD1_32_W(Dd, Rn) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b0111, 2, 0, 13))
// Load [Rn]! => Dd. Align is 8
#define VLD1_64_W(Dd, Rn) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b0111, 3, 0, 13))
// Load [Rn]! => Dd/Dd+1. Align is 4
#define VLD1Q_32_W(Dd, Rn) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 2, 0, 13))
// Load [Rn, Rm]! => Dd/Dd+1. If Rm==15, no writeback, Rm ignored, else writeback Rn <- Rn+Rm. Align is 4
#define VLD1Q_32_REG(Dd, Rn, Rm) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 2, 0, Rm))
// Load [Rn] => Dd/Dd+1. Align is 4
#define VLD1Q_8(Dd, Rn) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 0, 0, 15))
// Load [Rn] => Dd/Dd+1. Align is 4
#define VLD1Q_16(Dd, Rn) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 1, 0, 15))
// Load [Rn] => Dd/Dd+1. Align is 4
#define VLD1Q_64(Dd, Rn) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 3, 0, 15))
// Load [Rn] => Dd. Align is 4
#define VLD1_64(Dd, Rn) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b0111, 3, 0, 15))
// Load [Rn] => Dd. Align is 4
#define VLD1_32(Dd, Rn) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b0111, 2, 0, 15))
// Load [Rn] => Dd. Align is 4
#define VLD1_16(Dd, Rn) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b0111, 1, 0, 15))
// Load [Rn] => Dd. Align is 4
#define VLD1_8(Dd, Rn)  EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b0111, 0, 0, 15))

// Store [Rn]! => Dd. Align is 4
#define VST1_32_W(Dd, Rn) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b0111, 2, 0, 13))
// Store [Rn]! => Dd. Align is 8
#define VST1_64_W(Dd, Rn) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b0111, 3, 0, 13))
// Store [Rn] => Dd/Dd+1. Align is 4
#define VST1Q_32(Dd, Rn) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 2, 0, 15))
// Store [Rn]! => Dd/Dd+1. Align is 4
#define VST1Q_32_W(Dd, Rn) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 2, 0, 13))
// Store [Rn, Rm]! => Dd/Dd+1. If Rm==15, no writeback, Rm ignored, else writeback Rn <- Rn+Rm. Align is 4
#define VST1Q_32_REG(Dd, Rn, Rm) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 2, 0, Rm))
// Store [Rn] => Dd/Dd+1. Align is 4
#define VST1Q_8(Dd, Rn) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 0, 0, 15))
// Store [Rn] => Dd/Dd+1. Align is 4
#define VST1Q_16(Dd, Rn) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 1, 0, 15))
// Store [Rn] => Dd/Dd+1. Align is 4
#define VST1Q_64(Dd, Rn) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 3, 0, 15))
// Store [Rn] => Dd. Align is 4
#define VST1_64(Dd, Rn) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b0111, 3, 0, 15))
// Store [Rn] => Dd. Align is 4
#define VST1_32(Dd, Rn) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b0111, 2, 0, 15))
// Store [Rn] => Dd. Align is 4
#define VST1_16(Dd, Rn) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b0111, 1, 0, 15))
// Store [Rn] => Dd. Align is 4
#define VST1_8(Dd, Rn)  EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b0111, 0, 0, 15))

#define VEOR_gen(D, Vn, Vd, N, Q, M, Vm) (0b1111<<28 | 0b0011<<24 | 0<<23 | (D)<<22 | 0b00<<20 | (Vn)<<16 | (Vd)<<12 | 0b0001<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 1<<4 | (Vm))
#define VEOR(Dd, Dn, Dm)    EMIT(VEOR_gen(((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VEORQ(Dd, Dn, Dm)   EMIT(VEOR_gen(((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VMOVL_gen(U, D, imm3, Vd, M, Vm) (0b1111<<28 | 0b001<<25 | (U)<<24 | 1<<23 | (D)<<22 | (imm3)<<19 | (Vd)<<12 | 0b1010<<8 | (M)<<5 | 1<<4 | (Vm))
#define VMOVL_S8(Dd, Dm)    EMIT(VMOVL_gen(0, ((Dd)>>4)&1, 0b001, (Dd)&15, ((Dm)>>4)&1, (Dm)&15))
#define VMOVL_U8(Dd, Dm)    EMIT(VMOVL_gen(1, ((Dd)>>4)&1, 0b001, (Dd)&15, ((Dm)>>4)&1, (Dm)&15))
#define VMOVL_S32(Dd, Dm)   EMIT(VMOVL_gen(0, ((Dd)>>4)&1, 0b100, (Dd)&15, ((Dm)>>4)&1, (Dm)&15))
#define VMOVL_U32(Dd, Dm)   EMIT(VMOVL_gen(1, ((Dd)>>4)&1, 0b100, (Dd)&15, ((Dm)>>4)&1, (Dm)&15))

#define VMOV_gen(D,Vd, M, Vm, Q) (0b1111<<28 | 0b0010<<24 | 0<<23 | (D)<<22 | 0b10<<20 | (Vm)<<16 | (Vd)<<12 | 0b0001<<8 | (M)<<7 | (Q)<<6 | (M)<<5 | 1<<4 | (Vm))
#define VMOVD(Dd, Dm) if((Dd)!=(Dm)) {EMIT(VMOV_gen(((Dd)>>4)&1, (Dd)&15, ((Dm)>>4)&1, (Dm)&15, 0));}
#define VMOVQ(Dd, Dm) if((Dd)!=(Dm)) {EMIT(VMOV_gen(((Dd)>>4)&1, (Dd)&15, ((Dm)>>4)&1, (Dm)&15, 1));}

#define VMOV_igen(i, D, imm3, Vd, cmode, Q, op, imm4)   (0b1111<<28 | 0b001<<25 | (i)<<24 | 1<<23 | (D)<<22 | (imm3)<<16 | (Vd)<<12 | (cmode)<<8 | (Q)<<6 | (op)<<5 | 1<<4 | (imm4))
#define VMOV_8(Dd, imm)     EMIT(VMOV_igen(((imm)>>7)&1, ((Dd)>>4)&1, ((imm)>>4)&7, (Dd)&15, 0b1110, 0, 0, (imm)&15))
#define VMOVQ_8(Dd, imm)    EMIT(VMOV_igen(((imm)>>7)&1, ((Dd)>>4)&1, ((imm)>>4)&7, (Dd)&15, 0b1110, 1, 0, (imm)&15))
// Dd <= imm8 6bits duplicated 1bit -> 1byte
#define VMOV_D64(Dd, imm)    EMIT(VMOV_igen(((imm)>>7)&1, ((Dd)>>4)&1, ((imm)>>4)&7, (Dd)&15, 0b1110, 0, 1, (imm)&15))
// Dd <= imm8 in high bits
#define VMOV_H32(Dd, imm8)  EMIT(VMOV_igen(((imm8)>>7)&1, ((Dd)>>4)&1, ((imm8)>>4)&7, (Dd)&15, 0b0110, 0, 0, (imm8)&15))
// Dd <= imm8 in high bits
#define VMOV_H16(Dd, imm8)  EMIT(VMOV_igen(((imm8)>>7)&1, ((Dd)>>4)&1, ((imm8)>>4)&7, (Dd)&15, 0b1010, 0, 0, (imm8)&15))
// Qd <= imm8 in high bits
#define VMOVQ_H32(Dd, imm8) EMIT(VMOV_igen(((imm8)>>7)&1, ((Dd)>>4)&1, ((imm8)>>4)&7, (Dd)&15, 0b0110, 1, 0, (imm8)&15))
// Qd <= imm8 in high bits
#define VMOVQ_H16(Dd, imm8) EMIT(VMOV_igen(((imm8)>>7)&1, ((Dd)>>4)&1, ((imm8)>>4)&7, (Dd)&15, 0b1010, 1, 0, (imm8)&15))

#define VLD1LANE_gen(D, Rn, Vd, size, index_align, Rm) (0b1111<<28 | 0b0100<<24 | 1<<23 | (D)<<22 | 0b10<<20 | (Rn)<<16 | (Vd)<<12 | (size)<<10 | (index_align)<<4 | (Rm))
#define VLD1LANE_8(Dd, Rn, index)    EMIT(VLD1LANE_gen(((Dd)>>4)&1, (Rn), (Dd)&15, 0, (index)<<1, 15))
#define VLD1LANE_16(Dd, Rn, index)   EMIT(VLD1LANE_gen(((Dd)>>4)&1, (Rn), (Dd)&15, 1, (index)<<2, 15))
#define VLD1LANE_32(Dd, Rn, index)   EMIT(VLD1LANE_gen(((Dd)>>4)&1, (Rn), (Dd)&15, 2, (index)<<3, 15))

#define VLD1ALL_gen(D, Rn, Vd, size, T, a, Rm) (0b1111<<28 | 0b0100<<24 | 1<<23 | (D)<<22 | 0b10<<20 | (Rn)<<16 | (Vd)<<12 | 0b11<<10| (size)<<6 | (T)<<5 | (a)<<4 | (Rm))
#define VLD1ALL_16(Dd, Rn)      EMIT(VLD1ALL_gen(((Dd)>>4)&1, (Rn), (Dd)&15, 1, 0, 0, 15))
#define VLD1ALL_32(Dd, Rn)      EMIT(VLD1ALL_gen(((Dd)>>4)&1, (Rn), (Dd)&15, 2, 0, 0, 15))
#define VLD1QALL_32(Dd, Rn)     EMIT(VLD1ALL_gen(((Dd)>>4)&1, (Rn), (Dd)&15, 2, 1, 0, 15))

#define VST1LANE_gen(D, Rn, Vd, size, index_align, Rm) (0b1111<<28 | 0b0100<<24 | 1<<23 | (D)<<22 | 0b00<<20 | (Rn)<<16 | (Vd)<<12 | (size)<<10 | (index_align)<<4 | (Rm))
#define VST1LANE_8(Dd, Rn, index)    EMIT(VST1LANE_gen(((Dd)>>4)&1, (Rn), (Dd)&15, 0, (index)<<1, 15))
#define VST1LANE_16(Dd, Rn, index)   EMIT(VST1LANE_gen(((Dd)>>4)&1, (Rn), (Dd)&15, 1, (index)<<2, 15))
#define VST1LANE_32(Dd, Rn, index)   EMIT(VST1LANE_gen(((Dd)>>4)&1, (Rn), (Dd)&15, 2, (index)<<3, 15))

#define VADD_gen(size, D, Vn, Vd, N, Q, M, Vm) (0b1111<<28 | 0b0010<<24 | 0<<23 | (D)<<22 | (size)<<20 | (Vn)<<16 | (Vd)<<12 | 0b1000<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 0<<4 | (Vm))
#define VADD_8(Dd, Dn, Dm)     EMIT(VADD_gen(0, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VADDQ_8(Dd, Dn, Dm)    EMIT(VADD_gen(0, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VADD_16(Dd, Dn, Dm)    EMIT(VADD_gen(1, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VADDQ_16(Dd, Dn, Dm)   EMIT(VADD_gen(1, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VADD_32(Dd, Dn, Dm)    EMIT(VADD_gen(2, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VADDQ_32(Dd, Dn, Dm)   EMIT(VADD_gen(2, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VADD_64(Dd, Dn, Dm)    EMIT(VADD_gen(3, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VADDQ_64(Dd, Dn, Dm)   EMIT(VADD_gen(3, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VSUB_gen(size, D, Vn, Vd, N, Q, M, Vm) (0b1111<<28 | 0b0011<<24 | 0<<23 | (D)<<22 | (size)<<20 | (Vn)<<16 | (Vd)<<12 | 0b1000<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 0<<4 | (Vm))
#define VSUB_8(Dd, Dn, Dm)     EMIT(VSUB_gen(0, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSUBQ_8(Dd, Dn, Dm)    EMIT(VSUB_gen(0, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSUB_16(Dd, Dn, Dm)    EMIT(VSUB_gen(1, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSUBQ_16(Dd, Dn, Dm)   EMIT(VSUB_gen(1, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSUB_32(Dd, Dn, Dm)    EMIT(VSUB_gen(2, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSUBQ_32(Dd, Dn, Dm)   EMIT(VSUB_gen(2, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSUB_64(Dd, Dn, Dm)    EMIT(VSUB_gen(3, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSUBQ_64(Dd, Dn, Dm)   EMIT(VSUB_gen(3, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VORR_gen(D,Vd, N, Vn, M, Vm, Q) (0b1111<<28 | 0b0010<<24 | 0<<23 | (D)<<22 | 0b10<<20 | (Vn)<<16 | (Vd)<<12 | 0b0001<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 1<<4 | (Vm))
#define VORRD(Dd, Dn, Dm) EMIT(VORR_gen(((Dd)>>4)&1, (Dd)&15, ((Dn)>>4)&1, (Dn)&15, ((Dm)>>4)&1, (Dm)&15, 0))
#define VORRQ(Dd, Dn, Dm) EMIT(VORR_gen(((Dd)>>4)&1, (Dd)&15, ((Dn)>>4)&1, (Dn)&15, ((Dm)>>4)&1, (Dm)&15, 1))

#define VAND_gen(D,Vd, N, Vn, M, Vm, Q) (0b1111<<28 | 0b0010<<24 | 0<<23 | (D)<<22 | 0b00<<20 | (Vn)<<16 | (Vd)<<12 | 0b0001<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 1<<4 | (Vm))
#define VANDD(Dd, Dn, Dm) EMIT(VAND_gen(((Dd)>>4)&1, (Dd)&15, ((Dn)>>4)&1, (Dn)&15, ((Dm)>>4)&1, (Dm)&15, 0))
#define VANDQ(Dd, Dn, Dm) EMIT(VAND_gen(((Dd)>>4)&1, (Dd)&15, ((Dn)>>4)&1, (Dn)&15, ((Dm)>>4)&1, (Dm)&15, 1))

#define VBIC_gen(D,Vd, N, Vn, M, Vm, Q) (0b1111<<28 | 0b0010<<24 | 0<<23 | (D)<<22 | 0b01<<20 | (Vn)<<16 | (Vd)<<12 | 0b0001<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 1<<4 | (Vm))
#define VBICD(Dd, Dn, Dm) EMIT(VBIC_gen(((Dd)>>4)&1, (Dd)&15, ((Dn)>>4)&1, (Dn)&15, ((Dm)>>4)&1, (Dm)&15, 0))
#define VBICQ(Dd, Dn, Dm) EMIT(VBIC_gen(((Dd)>>4)&1, (Dd)&15, ((Dn)>>4)&1, (Dn)&15, ((Dm)>>4)&1, (Dm)&15, 1))

#define VBIC_igen(i, D, imm3, Vd, cmode, Q, imm4)  (0b1111<<28 | 0b001<<25 | (i)<<24 | 1<<23 | (D)<<22 | (imm3)<<16 | (Vd)<<12 | (cmode)<<8 | (Q)<<6 | 0b11<<4 | (imm4))
#define VBICQ_16_imm(Dd, imm)   EMIT(VBIC_16_imm(((imm)>>7)&1, (Dd>>4)&1, ((imm)>>4)&7, (Dd)&15, 0b1001, 1, (imm)&15))

#define VCEQ_I_gen(size, D, Vn, Vd, N, Q, M, Vm) (0b1111<<28 | 0b0011<<24 | 0<<23 | (D)<<22 | (size)<<20 | (Vn)<<16 | (Vd)<<12 | 0b1000<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 1<<4 | (Vm))
#define VCEQ_8(Dd, Dn, Dm)     EMIT(VCEQ_I_gen(0, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCEQQ_8(Dd, Dn, Dm)    EMIT(VCEQ_I_gen(0, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCEQ_16(Dd, Dn, Dm)    EMIT(VCEQ_I_gen(1, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCEQQ_16(Dd, Dn, Dm)   EMIT(VCEQ_I_gen(1, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCEQ_32(Dd, Dn, Dm)    EMIT(VCEQ_I_gen(2, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCEQQ_32(Dd, Dn, Dm)   EMIT(VCEQ_I_gen(2, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
//#define VCEQ_64(Dd, Dn, Dm)    EMIT(VCEQ_I_gen(3, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))   // doesn't exist
#define VCEQQ_64(Dd, Dn, Dm)   EMIT(VCEQ_I_gen(3, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCEQ_F_gen(sz, D, Vn, Vd, N, Q, M, Vm)  (0b1111<<28 | 0b0010<<24 | 0<<23 | (D)<<22 | (sz)<<20 | (Vn)<<16 | (Vd)<<12 | 0b1110<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 0<<4 | (Vm))
#define VCEQ_F32(Dd, Dn, Dm)    EMIT(VCEQ_F_gen(0, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCEQQ_F32(Dd, Dn, Dm)   EMIT(VCEQ_F_gen(0, ((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VCEQ_0_gen(D, size, Vd, F, Q, M, Vm)    (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | 0b01<<16 | (Vd)<<12 | (F)<<10 | 0b010<<7 | (Q)<<6 | (M)<<5 | (Vm))
#define VCEQ_0_8(Dd, Dm)        EMIT(VCEQ_0_gen(((Dd)>>4)&1, 0, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCEQ_0_16(Dd, Dm)       EMIT(VCEQ_0_gen(((Dd)>>4)&1, 1, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCEQ_0_32(Dd, Dm)       EMIT(VCEQ_0_gen(((Dd)>>4)&1, 2, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCEQQ_0_8(Dd, Dm)       EMIT(VCEQ_0_gen(((Dd)>>4)&1, 0, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCEQQ_0_16(Dd, Dm)      EMIT(VCEQ_0_gen(((Dd)>>4)&1, 1, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCEQQ_0_32(Dd, Dm)      EMIT(VCEQ_0_gen(((Dd)>>4)&1, 2, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCEQQ_0_F32(Dd, Dm)     EMIT(VCEQ_0_gen(((Dd)>>4)&1, 2, (Dd)&15, 1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VCGT_I_gen(U, D, size, Vn, Vd, N, Q, M, Vm) (0b1111<<28 | 0b001<<25 | (U)<<24 | 0<<23 | (D)<<22 | (size)<<20 | (Vn)<<16 | (Vd)<<12 | 0b0011<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | (Vm))
#define VCGT_U8(Dd, Dn, Dm)     EMIT(VCGT_I_gen(1, ((Dd)>>4)&1, 0, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCGTQ_U8(Dd, Dn, Dm)    EMIT(VCGT_I_gen(1, ((Dd)>>4)&1, 0, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCGT_U16(Dd, Dn, Dm)    EMIT(VCGT_I_gen(1, ((Dd)>>4)&1, 1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCGTQ_U16(Dd, Dn, Dm)   EMIT(VCGT_I_gen(1, ((Dd)>>4)&1, 1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCGT_U32(Dd, Dn, Dm)    EMIT(VCGT_I_gen(1, ((Dd)>>4)&1, 2, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCGTQ_U32(Dd, Dn, Dm)   EMIT(VCGT_I_gen(1, ((Dd)>>4)&1, 2, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCGT_U64(Dd, Dn, Dm)    EMIT(VCGT_I_gen(1, ((Dd)>>4)&1, 3, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCGTQ_U64(Dd, Dn, Dm)   EMIT(VCGT_I_gen(1, ((Dd)>>4)&1, 3, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCGT_S8(Dd, Dn, Dm)     EMIT(VCGT_I_gen(0, ((Dd)>>4)&1, 0, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCGTQ_S8(Dd, Dn, Dm)    EMIT(VCGT_I_gen(0, ((Dd)>>4)&1, 0, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCGT_S16(Dd, Dn, Dm)    EMIT(VCGT_I_gen(0, ((Dd)>>4)&1, 1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCGTQ_S16(Dd, Dn, Dm)   EMIT(VCGT_I_gen(0, ((Dd)>>4)&1, 1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCGT_S32(Dd, Dn, Dm)    EMIT(VCGT_I_gen(0, ((Dd)>>4)&1, 2, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCGTQ_S32(Dd, Dn, Dm)   EMIT(VCGT_I_gen(0, ((Dd)>>4)&1, 2, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCGT_S64(Dd, Dn, Dm)    EMIT(VCGT_I_gen(0, ((Dd)>>4)&1, 3, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCGTQ_S64(Dd, Dn, Dm)   EMIT(VCGT_I_gen(0, ((Dd)>>4)&1, 3, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCGT_F_gen(D, sz, Vn, Vd, N, Q, M, Vm)  (0b1111<<28 | 0b0011<<24 | 0<<23 | (D)<<22 | 1<<21 | (sz)<<20 | (Vn)<<16 | (Vd)<<12 | 0b1110<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 0<<4 | (Vm))
#define VCGT_F32(Dd, Dn, Dm)    EMIT(VCGT_F_gen(((Dd)>>4)&1, 0, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCGTQ_F32(Dd, Dn, Dm)   EMIT(VCGT_F_gen(((Dd)>>4)&1, 0, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VCGE_F_gen(D, sz, Vn, Vd, N, Q, M, Vm)  (0b1111<<28 | 0b0011<<24 | 0<<23 | (D)<<22 | 0<<21 | (sz)<<20 | (Vn)<<16 | (Vd)<<12 | 0b1110<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 0<<4 | (Vm))
#define VCGE_F32(Dd, Dn, Dm)    EMIT(VCGE_F_gen(((Dd)>>4)&1, 0, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCGEQ_F32(Dd, Dn, Dm)   EMIT(VCGE_F_gen(((Dd)>>4)&1, 0, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VCLT_0_gen(D, size, Vd, F, Q, M, Vm)    (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | 0b01<<16 | (Vd)<<12 | (F)<<10 | 0b100<<7 | (Q)<<6 | (M)<<5 | (Vm))
#define VCLT_0_8(Dd, Dm)        EMIT(VCLT_0_gen(((Dd)>>4)&1, 0, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCLT_0_16(Dd, Dm)       EMIT(VCLT_0_gen(((Dd)>>4)&1, 1, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCLT_0_32(Dd, Dm)       EMIT(VCLT_0_gen(((Dd)>>4)&1, 2, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCLTQ_0_8(Dd, Dm)       EMIT(VCLT_0_gen(((Dd)>>4)&1, 0, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCLTQ_0_16(Dd, Dm)      EMIT(VCLT_0_gen(((Dd)>>4)&1, 1, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCLTQ_0_32(Dd, Dm)      EMIT(VCLT_0_gen(((Dd)>>4)&1, 2, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))


#define VSHR_gen(U, D, imm6, Vd, L, Q, M, Vm) (0b1111<<28 | 0b001<<25 | (U)<<24 | 1<<23 | (D)<<22 | (imm6)<<16 | (Vd)<<12 | (L)<<7 | (Q)<<6 | (M)<<5 | 1<<4 | (Vm))
#define VSHR_U8(Dd, Dm, imm3)    EMIT(VSHR_gen(1, ((Dd)>>4)&1, 0b001<<3 | (8-(imm3)), (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHR_S8(Dd, Dm, imm3)    EMIT(VSHR_gen(0, ((Dd)>>4)&1, 0b001<<3 | (8-(imm3)), (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHR_U16(Dd, Dm, imm4)   EMIT(VSHR_gen(1, ((Dd)>>4)&1, 0b01<<4 | (16-(imm4)), (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHR_S16(Dd, Dm, imm4)   EMIT(VSHR_gen(0, ((Dd)>>4)&1, 0b01<<4 | (16-(imm4)), (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHR_U32(Dd, Dm, imm5)   EMIT(VSHR_gen(1, ((Dd)>>4)&1, 0b1<<5 | (32-(imm5)), (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHR_S32(Dd, Dm, imm5)   EMIT(VSHR_gen(0, ((Dd)>>4)&1, 0b1<<5 | (32-(imm5)), (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHR_U64(Dd, Dm, imm6)   EMIT(VSHR_gen(1, ((Dd)>>4)&1, (64-(imm6)), (Dd)&15, 1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHR_S64(Dd, Dm, imm6)   EMIT(VSHR_gen(0, ((Dd)>>4)&1, (64-(imm6)), (Dd)&15, 1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHRQ_U8(Dd, Dm, imm3)    EMIT(VSHR_gen(1, ((Dd)>>4)&1, 0b001<<3 | (8-(imm3)), (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHRQ_S8(Dd, Dm, imm3)    EMIT(VSHR_gen(0, ((Dd)>>4)&1, 0b001<<3 | (8-(imm3)), (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHRQ_U16(Dd, Dm, imm4)   EMIT(VSHR_gen(1, ((Dd)>>4)&1, 0b01<<4 | (16-(imm4)), (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHRQ_S16(Dd, Dm, imm4)   EMIT(VSHR_gen(0, ((Dd)>>4)&1, 0b01<<4 | (16-(imm4)), (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHRQ_U32(Dd, Dm, imm5)   EMIT(VSHR_gen(1, ((Dd)>>4)&1, 0b1<<5 | (32-(imm5)), (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHRQ_S32(Dd, Dm, imm5)   EMIT(VSHR_gen(0, ((Dd)>>4)&1, 0b1<<5 | (32-(imm5)), (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHRQ_U64(Dd, Dm, imm6)   EMIT(VSHR_gen(1, ((Dd)>>4)&1, (64-(imm6)), (Dd)&15, 1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHRQ_S64(Dd, Dm, imm6)   EMIT(VSHR_gen(0, ((Dd)>>4)&1, (64-(imm6)), (Dd)&15, 1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VSHL_gen(D, imm6, Vd, L, Q, M, Vm) (0b1111<<28 | 0b0010<<24 | 1<<23 | (D)<<22 | (imm6)<<16 | (Vd)<<12 | 0b0101<<8 | (L)<<7 | (Q)<<6 | (M)<<5 | 1<<4 | (Vm))
#define VSHL_8(Dd, Dm, imm3)    EMIT(VSHL_gen(((Dd)>>4)&1, 0b001<<3 | ((imm3)+8), (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHL_16(Dd, Dm, imm4)   EMIT(VSHL_gen(((Dd)>>4)&1, 0b01<<4 | ((imm4)+16), (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHL_32(Dd, Dm, imm5)   EMIT(VSHL_gen(((Dd)>>4)&1, 0b1<<5 | ((imm5)+32), (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHL_64(Dd, Dm, imm6)   EMIT(VSHL_gen(((Dd)>>4)&1, (imm6), (Dd)&15, 1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHLQ_8(Dd, Dm, imm3)   EMIT(VSHL_gen(((Dd)>>4)&1, 0b001<<3 | ((imm3)+8), (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHLQ_16(Dd, Dm, imm4)  EMIT(VSHL_gen(((Dd)>>4)&1, 0b01<<4 | ((imm4)+16), (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHLQ_32(Dd, Dm, imm5)  EMIT(VSHL_gen(((Dd)>>4)&1, 0b1<<5 | ((imm5)+32), (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHLQ_64(Dd, Dm, imm6)  EMIT(VSHL_gen(((Dd)>>4)&1, (imm6), (Dd)&15, 1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VSHLR_gen(U, D, size, Vn, Vd, N, Q, M, Vm)  (0b1111<<28 | 0b001<<25 | (U)<<24 | (D)<<22 | (size)<<20 | (Vn)<<16 | (Vd)<<12 | 0b0100<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | (Vm))
#define VSHL_U8(Dd, Dm, Dn)     EMIT(VSHLR_gen(1, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHL_U16(Dd, Dm, Dn)    EMIT(VSHLR_gen(1, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHL_U32(Dd, Dm, Dn)    EMIT(VSHLR_gen(1, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHL_U64(Dd, Dm, Dn)    EMIT(VSHLR_gen(1, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHLQ_U8(Dd, Dm, Dn)    EMIT(VSHLR_gen(1, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHLQ_U16(Dd, Dm, Dn)   EMIT(VSHLR_gen(1, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHLQ_U32(Dd, Dm, Dn)   EMIT(VSHLR_gen(1, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHLQ_U64(Dd, Dm, Dn)   EMIT(VSHLR_gen(1, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHL_S8(Dd, Dm, Dn)     EMIT(VSHLR_gen(0, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHL_S16(Dd, Dm, Dn)    EMIT(VSHLR_gen(0, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHL_S32(Dd, Dm, Dn)    EMIT(VSHLR_gen(0, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHL_S64(Dd, Dm, Dn)    EMIT(VSHLR_gen(0, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSHLQ_S8(Dd, Dm, Dn)    EMIT(VSHLR_gen(0, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHLQ_S16(Dd, Dm, Dn)   EMIT(VSHLR_gen(0, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHLQ_S32(Dd, Dm, Dn)   EMIT(VSHLR_gen(0, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VSHLQ_S64(Dd, Dm, Dn)   EMIT(VSHLR_gen(0, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VSHRN_gen(D, imm6, Vd, M, Vm)   (0b1111<<28 | 0b0010<<24 | 1<<23 | (D)<<22 | (imm6)<<16 | (Vd)<<12 | 1<<11 | (M)<<5 | 1<<4 | (Vm))
#define VSHRN_16(Dd, Dm, imm3)          EMIT(VSHRN_gen(((Dd)>>4)&1, 0b001<<3 | (8-(imm3)), (Dd)&15, ((Dm)>>4)&1, (Dm)&15))
#define VSHRN_32(Dd, Dm, imm4)          EMIT(VSHRN_gen(((Dd)>>4)&1, 0b01<<4  | (16-(imm4)), (Dd)&15, ((Dm)>>4)&1, (Dm)&15))
#define VSHRN_64(Dd, Dm, imm5)          EMIT(VSHRN_gen(((Dd)>>4)&1, 0b1<<5   | (32-(imm5)), (Dd)&15, ((Dm)>>4)&1, (Dm)&15))

#define VTRN_gen(D, size, Vd, Q, M, Vm) (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | 0b10<<16 | (Vd)<<12 | 0b0000<<8 | 1<<7 | (Q)<<6 | (M)<<5 | (Vm))
#define VTRN_32(Dd, Dm)     EMIT(VTRN_gen(((Dd)>>4)&1, 2, (Dd)&15, 0, ((Dm)>>4)&1, (Dm)&15))

#define VZIP_gen(D, size, Vd, Q, M, Vm) (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | 0b10<<16 | (Vd)<<12 | 0b0001<<8 | 1<<7 | (Q)<<6 | (M)<<5 | (Vm))
#define VZIP_8(Dd, Dm)     EMIT(VZIP_gen(((Dd)>>4)&1, 0, (Dd)&15, 0, ((Dm)>>4)&1, (Dm)&15))
#define VZIP_16(Dd, Dm)    EMIT(VZIP_gen(((Dd)>>4)&1, 1, (Dd)&15, 0, ((Dm)>>4)&1, (Dm)&15))
#define VZIP_32(Dd, Dm)    VTRN_32(Dd, Dm)
#define VZIPQ_8(Dd, Dm)    EMIT(VZIP_gen(((Dd)>>4)&1, 0, (Dd)&15, 1, ((Dm)>>4)&1, (Dm)&15))
#define VZIPQ_16(Dd, Dm)   EMIT(VZIP_gen(((Dd)>>4)&1, 1, (Dd)&15, 1, ((Dm)>>4)&1, (Dm)&15))
#define VZIPQ_32(Dd, Dm)   EMIT(VZIP_gen(((Dd)>>4)&1, 2, (Dd)&15, 1, ((Dm)>>4)&1, (Dm)&15))

#define VUZP_gen(D, size, Vd, Q, M, Vm) (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | 0b10<<16 | (Vd)<<12 | 0b0001<<8 | 0<<7 | (Q)<<6 | (M)<<5 | (Vm))
#define VUZP_8(Dd, Dm)     EMIT(VUZP_gen(((Dd)>>4)&1, 0, (Dd)&15, 0, ((Dm)>>4)&1, (Dm)&15))
#define VUZP_16(Dd, Dm)    EMIT(VUZP_gen(((Dd)>>4)&1, 1, (Dd)&15, 0, ((Dm)>>4)&1, (Dm)&15))
#define VUZP_32(Dd, Dm)    VTRN_32(Dd, Dm)
#define VUZPQ_8(Dd, Dm)    EMIT(VUZP_gen(((Dd)>>4)&1, 0, (Dd)&15, 1, ((Dm)>>4)&1, (Dm)&15))
#define VUZPQ_16(Dd, Dm)   EMIT(VUZP_gen(((Dd)>>4)&1, 1, (Dd)&15, 1, ((Dm)>>4)&1, (Dm)&15))
#define VUZPQ_32(Dd, Dm)   EMIT(VUZP_gen(((Dd)>>4)&1, 2, (Dd)&15, 1, ((Dm)>>4)&1, (Dm)&15))

#define VADD_F_gen(D, Vn, Vd, N, Q, M, Vm) (0b1111<<28 | 0b0010<<24 | (D)<<22 | 0<<20 | (Vn)<<16 | (Vd)<<12 | 0b1101<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | (Vm))
#define VADD__F32(Dd, Dn, Dm)   EMIT(VADD_F_gen(((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VADDQ_F32(Dd, Dn, Dm)   EMIT(VADD_F_gen(((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VSUB_F_gen(D, Vn, Vd, N, Q, M, Vm) (0b1111<<28 | 0b0010<<24 | (D)<<22 | 1<<21 | 0<<20 | (Vn)<<16 | (Vd)<<12 | 0b1101<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | (Vm))
#define VSUB__F32(Dd, Dn, Dm)   EMIT(VSUB_F_gen(((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VSUBQ_F32(Dd, Dn, Dm)   EMIT(VSUB_F_gen(((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VMUL_F_gen(D, Vn, Vd, N, Q, M, Vm) (0b1111<<28 | 0b0011<<24 | (D)<<22 | 0<<20 | (Vn)<<16 | (Vd)<<12 | 0b1101<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 1<<4 | (Vm))
#define VMUL__F32(Dd, Dn, Dm)   EMIT(VMUL_F_gen(((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VMULQ_F32(Dd, Dn, Dm)   EMIT(VMUL_F_gen(((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VCVT_NEON_gen(D, size, Vd, op, Q, M, Vm)    (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | 0b11<<16 | (Vd)<<12 | 0b11<<9 | (op)<<7 | (Q)<<6 | (M)<<5 | (Vm))
#define VCVTQ_F32_S32(Dd, Dm)   EMIT(VCVT_NEON_gen(((Dd)>>4)&1, 2, (Dd)&15, 0b00, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCVTQ_S32_F32(Dd, Dm)   EMIT(VCVT_NEON_gen(((Dd)>>4)&1, 2, (Dd)&15, 0b10, 1, ((Dm)>>4)&1, (Dm)&15))
#define VCVTn_F32_S32(Dd, Dm)   EMIT(VCVT_NEON_gen(((Dd)>>4)&1, 2, (Dd)&15, 0b00, 0, ((Dm)>>4)&1, (Dm)&15))
#define VCVTn_S32_F32(Dd, Dm)   EMIT(VCVT_NEON_gen(((Dd)>>4)&1, 2, (Dd)&15, 0b10, 0, ((Dm)>>4)&1, (Dm)&15))

#define VMULL_NEON_gen(U, D, size, Vn, Vd, op, N, M, Vm)    (0b1111<<28 | 0b001<<25 | (U)<<24 | 1<<23 | (D)<<22 | (size)<<20 | (Vn)<<16 | (Vd)<<12 | 0b11<<10 | (op)<<9 | (N)<<7 | (M)<<5 | (Vm))
#define VMULL_S16_S8(Dd, Dn, Dm)    EMIT(VMULL_NEON_gen(0, ((Dd)>>4)&1, 0, (Dn)&15, (Dd)&15, 0, ((Dn)>>4)&1, ((Dm)>>4)&1, (Dm)&15))
#define VMULL_U16_U8(Dd, Dn, Dm)    EMIT(VMULL_NEON_gen(1, ((Dd)>>4)&1, 0, (Dn)&15, (Dd)&15, 0, ((Dn)>>4)&1, ((Dm)>>4)&1, (Dm)&15))
#define VMULL_S32_S16(Dd, Dn, Dm)   EMIT(VMULL_NEON_gen(0, ((Dd)>>4)&1, 1, (Dn)&15, (Dd)&15, 0, ((Dn)>>4)&1, ((Dm)>>4)&1, (Dm)&15))
#define VMULL_U32_U16(Dd, Dn, Dm)   EMIT(VMULL_NEON_gen(1, ((Dd)>>4)&1, 1, (Dn)&15, (Dd)&15, 0, ((Dn)>>4)&1, ((Dm)>>4)&1, (Dm)&15))
#define VMULL_S64_S32(Dd, Dn, Dm)   EMIT(VMULL_NEON_gen(0, ((Dd)>>4)&1, 2, (Dn)&15, (Dd)&15, 0, ((Dn)>>4)&1, ((Dm)>>4)&1, (Dm)&15))
#define VMULL_U64_U32(Dd, Dn, Dm)   EMIT(VMULL_NEON_gen(1, ((Dd)>>4)&1, 2, (Dn)&15, (Dd)&15, 0, ((Dn)>>4)&1, ((Dm)>>4)&1, (Dm)&15))
#define VMULL_P64(Dd, Dn, Dm)       EMIT(VMULL_NEON_gen(0, ((Dd)>>4)&1, 2, (Dn)&15, (Dd)&15, 1, ((Dn)>>4)&1, ((Dm)>>4)&1, (Dm)&15))

#define VMUL_NEON_gen(op, D, size, Vn, Vd, N, Q, M, Vm)    (0b1111<<28 | 0b001<<25 | (op)<<24 | 0<<23 | (D)<<22 | (size)<<20 | (Vn)<<16 | (Vd)<<12 | 0b1001<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 1<<4 | (Vm))
#define VMULQ_32(Dd, Dn, Dm)     EMIT(VMUL_NEON_gen(0, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VMULQ_16(Dd, Dn, Dm)     EMIT(VMUL_NEON_gen(0, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VMUL_32(Dd, Dn, Dm)      EMIT(VMUL_NEON_gen(0, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VMUL_16(Dd, Dn, Dm)      EMIT(VMUL_NEON_gen(0, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))

#define VEXT_gen(D, Vn, Vd, imm4, N, Q, M, Vm)  (0b1111<<28 | 0b0010<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (Vn)<<16 | (Vd)<<12 | (imm4)<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | (Vm))
#define VEXT_8(Dd, Dn, Dm, imm4)    EMIT(VEXT_gen(((Dd)>>4)&1, (Dn)&15, (Dd)&15, (imm4)&0xf, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VEXTQ_8(Dd, Dn, Dm, imm4)   EMIT(VEXT_gen(((Dd)>>4)&1, (Dn)&15, (Dd)&15, (imm4)&0xf, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VMVN_gen(D, size, Vd, Q, M, Vm) (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | (Vd)<<12 | 0b1011<<7 | (Q)<<6 | (M)<<5 | 0<<4 | (Vm))
#define VMVNQ(Dd, Dm)       EMIT(VMVN_gen(((Dd)>>4)&1, 0, (Dd)&15, 1, ((Dm)>>4)&1, (Dm)&15))

#define VQMOVN_gen(D, size, Vd, op, M, Vm) (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | 0b10<<16 | (Vd)<<12 | 0b0010<<8 | (op)<<6 | (M)<<5 | (Vm))
// Vector Saturating Move and Narrow, for 16 bits signed integer (Dm is Qm in fact)
#define VQMOVN_S16(Dd, Dm)  EMIT(VQMOVN_gen(((Dd)>>4)&1, 0b00, (Dd)&15, 0b10, ((Dm)>>4)&1, (Dm)&15))
// Vector Saturating Move and Narrow, for 32 bits signed integer (Dm is Qm in fact)
#define VQMOVN_S32(Dd, Dm)  EMIT(VQMOVN_gen(((Dd)>>4)&1, 0b01, (Dd)&15, 0b10, ((Dm)>>4)&1, (Dm)&15))
// Vector Saturating Move and Narrow, for 64 bits signed integer (Dm is Qm in fact)
#define VQMOVN_S64(Dd, Dm)  EMIT(VQMOVN_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 0b10, ((Dm)>>4)&1, (Dm)&15))
// Vector Saturating Move and Narrow, for 16 bits signed integer (Dm is Qm in fact)
#define VQMOVUN_S16(Dd, Dm) EMIT(VQMOVN_gen(((Dd)>>4)&1, 0b00, (Dd)&15, 0b01, ((Dm)>>4)&1, (Dm)&15))
// Vector Saturating Move and Narrow, for 32 bits signed integer (Dm is Qm in fact)
#define VQMOVUN_S32(Dd, Dm) EMIT(VQMOVN_gen(((Dd)>>4)&1, 0b01, (Dd)&15, 0b01, ((Dm)>>4)&1, (Dm)&15))
// Vector Saturating Move and Narrow, for 64 bits signed integer (Dm is Qm in fact)
#define VQMOVUN_S64(Dd, Dm) EMIT(VQMOVN_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 0b01, ((Dm)>>4)&1, (Dm)&15))
// Vector Saturating Move and Narrow, for 16 bits unsigned integer (Dm is Qm in fact)
#define VQMOVN_U16(Dd, Dm)  EMIT(VQMOVN_gen(((Dd)>>4)&1, 0b00, (Dd)&15, 0b11, ((Dm)>>4)&1, (Dm)&15))
// Vector Saturating Move and Narrow, for 32 bits signed integer (Dm is Qm in fact)
#define VQMOVN_U32(Dd, Dm)  EMIT(VQMOVN_gen(((Dd)>>4)&1, 0b01, (Dd)&15, 0b11, ((Dm)>>4)&1, (Dm)&15))
// Vector Saturating Move and Narrow, for 64 bits signed integer (Dm is Qm in fact)
#define VQMOVN_U64(Dd, Dm)  EMIT(VQMOVN_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 0b11, ((Dm)>>4)&1, (Dm)&15))

#define VTBL_gen(D, Vn, Vd, len, N, op, M, Vm) (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (Vn)<<16 | (Vd)<<12 | 0b10<<10 | (len)<<8 | (N)<<7 | (op)<<6 | (M)<<5 | (Vm))
// Vector Table lookup, take elements from Dn, and using lookup table Dm, put elements in Dd
#define VTBL1_8(Dd, Dn, Dm) EMIT(VTBL_gen(((Dd)>>4)&1, (Dn)&15, (Dd)&15, 0b00, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// Vector Table lookup, take elements from Dn/Dn+1, and using lookup table Dm, put elements in Dd
#define VTBL2_8(Dd, Dn, Dm) EMIT(VTBL_gen(((Dd)>>4)&1, (Dn)&15, (Dd)&15, 0b01, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// Vector Table lookup eXtension, take elements from Dn, and using lookup table Dm, put elements in Dd, outer element untouched
#define VTBLX1_8(Dd, Dn, Dm) EMIT(VTBL_gen(((Dd)>>4)&1, (Dn)&15, (Dd)&15, 0b00, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Vector Table lookup eXtension, take elements from Dn/Dn+1, and using lookup table Dm, put elements in Dd, outer element untouched
#define VTBLX2_8(Dd, Dn, Dm) EMIT(VTBL_gen(((Dd)>>4)&1, (Dn)&15, (Dd)&15, 0b01, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VRECPE_gen(D, size, Vd, F, Q, M, Vm) (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | 0b11<<16 | (Vd)<<12 | 0b010<<9 | (F)<<8 | (Q)<<6 | (M)<<5 | (Vm))
// Vector Reciprocal Estimate of Dm to Dd
#define VRECPEQ_F32(Dd, Dm) EMIT(VRECPE_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VRECPS_gen(D, size, Vn, Vd, N, Q, M, Vm) (0b1111<<28 | 0b0010<<24 | 0<<23 | (D)<<22 | (size)<<20 | (Vn)<<16 | (Vd)<<12 | 0b1111<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 1<<4 | (Vm))
// Vector Reciprocal Step of Dn and Dm to Dd
#define VRECPSQ_F32(Dd, Dn, Dm) EMIT(VRECPS_gen(((Dd)>>4)&1, 0, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VRSQRTE_gen(D, size, Vd, F, Q, M, Vm) (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | 0b11<<16 | (Vd)<<12 | 0b010<<9 | (F)<<8 | 1<<7 | (Q)<<6 | (M)<<5 | (Vm))
// Vector Reciprocal Square Root Estimate of Dm to Dd
#define VRSQRTEQ_F32(Dd, Dm) EMIT(VRSQRTE_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VRSQRTS_gen(D, size, Vn, Vd, N, Q, M, Vm) (0b1111<<28 | 0b0010<<24 | 0<<23 | (D)<<22 | 1<<21 | (size)<<20 | (Vn)<<16 | (Vd)<<12 | 0b1111<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 1<<4 | (Vm))
// Vector Reciprocal Square Root Step of Dn and Dm to Dd
#define VRSQRTSQ_F32(Dd, Dn, Dm) EMIT(VRSQRTS_gen(((Dd)>>4)&1, 0, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))


#define VSWP_gen(D, size, Vd, Q, M, Vm)    (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | 0b10<<16 | (Vd)<<12 | 0b0000<<7 | (Q)<<6 | (M)<<5 | (Vm))
// Swap Double vectors content
#define VSWP(Dd, Dm)   EMIT(VSWP_gen(((Dd)>>4)&1, 0b00, (Dd)&15, 0, ((Dm)>>4)&1, (Dm)&15))
// Swap Quad vectors content
#define VSWPQ(Dd, Dm)  EMIT(VSWP_gen(((Dd)>>4)&1, 0b00, (Dd)&15, 1, ((Dm)>>4)&1, (Dm)&15))

#define VQADD_gen(U, D, size, Vn, Vd, N, Q, M, Vm)  (0b1111<<28 | 0b001<<25 | (U)<<24 | (D)<<22 | (size)<<20 | (Vn)<<16 | (Vd)<<12  | 0b0000<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 1<<4 | (Vm))
// Add with saturation unsigned 8bits
#define VQADDQ_U8(Dd, Dn, Dm)   EMIT(VQADD_gen(1, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Add with saturation unsigned 16bits
#define VQADDQ_U16(Dd, Dn, Dm)  EMIT(VQADD_gen(1, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Add with saturation unsigned 32bits
#define VQADDQ_U32(Dd, Dn, Dm)  EMIT(VQADD_gen(1, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Add with saturation unsigned 64bits
#define VQADDQ_U64(Dd, Dn, Dm)  EMIT(VQADD_gen(1, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Add with saturation signed 8bits
#define VQADDQ_S8(Dd, Dn, Dm)   EMIT(VQADD_gen(0, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Add with saturation signed 16bits
#define VQADDQ_S16(Dd, Dn, Dm)  EMIT(VQADD_gen(0, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Add with saturation signed 32bits
#define VQADDQ_S32(Dd, Dn, Dm)  EMIT(VQADD_gen(0, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Add with saturation signed 64bits
#define VQADDQ_S64(Dd, Dn, Dm)  EMIT(VQADD_gen(0, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Add with saturation unsigned 8bits
#define VQADD_U8(Dd, Dn, Dm)   EMIT(VQADD_gen(1, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// Add with saturation unsigned 16bits
#define VQADD_U16(Dd, Dn, Dm)  EMIT(VQADD_gen(1, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// Add with saturation unsigned 32bits
#define VQADD_U32(Dd, Dn, Dm)  EMIT(VQADD_gen(1, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// Add with saturation unsigned 64bits
#define VQADD_U64(Dd, Dn, Dm)  EMIT(VQADD_gen(1, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// Add with saturation signed 8bits
#define VQADD_S8(Dd, Dn, Dm)   EMIT(VQADD_gen(0, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// Add with saturation signed 16bits
#define VQADD_S16(Dd, Dn, Dm)  EMIT(VQADD_gen(0, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// Add with saturation signed 32bits
#define VQADD_S32(Dd, Dn, Dm)  EMIT(VQADD_gen(0, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// Add with saturation signed 64bits
#define VQADD_S64(Dd, Dn, Dm)  EMIT(VQADD_gen(0, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))

#define VQSUB_gen(U, D, size, Vn, Vd, N, Q, M, Vm)  (0b1111<<28 | 0b001<<25 | (U)<<24 | (D)<<22 | (size)<<20 | (Vn)<<16 | (Vd)<<12 | 0b0010<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 1<<4 | (Vm))
// Substract with saturation unsigned 8bits
#define VQSUBQ_U8(Dd, Dn, Dm)   EMIT(VQSUB_gen(1, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Substract with saturation unsigned 16bits
#define VQSUBQ_U16(Dd, Dn, Dm)  EMIT(VQSUB_gen(1, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Substract with saturation unsigned 32bits
#define VQSUBQ_U32(Dd, Dn, Dm)  EMIT(VQSUB_gen(1, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Substract with saturation unsigned 64bits
#define VQSUBQ_U64(Dd, Dn, Dm)  EMIT(VQSUB_gen(1, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Substract with saturation signed 8bits
#define VQSUBQ_S8(Dd, Dn, Dm)   EMIT(VQSUB_gen(0, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Substract with saturation signed 16bits
#define VQSUBQ_S16(Dd, Dn, Dm)  EMIT(VQSUB_gen(0, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Substract with saturation signed 32bits
#define VQSUBQ_S32(Dd, Dn, Dm)  EMIT(VQSUB_gen(0, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Substract with saturation signed 64bits
#define VQSUBQ_S64(Dd, Dn, Dm)  EMIT(VQSUB_gen(0, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
// Substract with saturation unsigned 8bits
#define VQSUB_U8(Dd, Dn, Dm)   EMIT(VQSUB_gen(1, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// Substract with saturation unsigned 16bits
#define VQSUB_U16(Dd, Dn, Dm)  EMIT(VQSUB_gen(1, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// Substract with saturation unsigned 32bits
#define VQSUB_U32(Dd, Dn, Dm)  EMIT(VQSUB_gen(1, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// Substract with saturation unsigned 64bits
#define VQSUB_U64(Dd, Dn, Dm)  EMIT(VQSUB_gen(1, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// Substract with saturation signed 8bits
#define VQSUB_S8(Dd, Dn, Dm)   EMIT(VQSUB_gen(0, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// Substract with saturation signed 16bits
#define VQSUB_S16(Dd, Dn, Dm)  EMIT(VQSUB_gen(0, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// Substract with saturation signed 32bits
#define VQSUB_S32(Dd, Dn, Dm)  EMIT(VQSUB_gen(0, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// Substract with saturation signed 64bits
#define VQSUB_S64(Dd, Dn, Dm)  EMIT(VQSUB_gen(0, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))

#define VNEGN_gen(D, size, Vd, F, Q, M, Vm) (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | 0b01<<16 | (Vd)<<12 | (F)<<10 | 0b111<<7 | (Q)<<6 | (M)<<5 | (Vm))
#define VNEGN_8(Dd, Dm)     EMIT(VNEGN_gen(((Dd)>>4)&1, 0b00, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VNEGN_16(Dd, Dm)    EMIT(VNEGN_gen(((Dd)>>4)&1, 0b01, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VNEGN_32(Dd, Dm)    EMIT(VNEGN_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
// no VNEGN_64...
#define VNEGNQ_8(Dd, Dm)    EMIT(VNEGN_gen(((Dd)>>4)&1, 0b00, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VNEGNQ_16(Dd, Dm)   EMIT(VNEGN_gen(((Dd)>>4)&1, 0b01, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VNEGNQ_32(Dd, Dm)   EMIT(VNEGN_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VNEGN_F32(Dd, Dm)   EMIT(VNEGN_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VNEGNQ_F32(Dd, Dm)  EMIT(VNEGN_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VMINMAXF_gen(D, op, sz, Vn, Vd, N, Q, M, Vm)    (0b1111<<28 | 0b0010<<24 | (D)<<22 | (op)<<21 | (sz)<<20 | (Vn)<<16 | (Vd)<<12 | 0b1111<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | (Vm))
#define VMINQ_F32(Dd, Dn, Dm)   EMIT(VMINMAXF_gen(((Dd)>>4)&1, 1, 0, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VMAXQ_F32(Dd, Dn, Dm)   EMIT(VMINMAXF_gen(((Dd)>>4)&1, 0, 0, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VABD_gen(U, D, size, Vn, Vd, N, Q, M, Vm)   (0b1111<<28 | 1<<25 | (U)<<24 | 0<<23 | (D)<<22 | (size)<<20 | (Vn)<<16 | (Vd)<<12 | 0b0111<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | (Vm))
// Compute absolute difference of U8
#define VABD_U8(Dd, Dn, Dm)     EMIT(VABD_gen(1, ((Dd)>>4)&1, 0, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VABDQ_U8(Dd, Dn, Dm)    EMIT(VABD_gen(1, ((Dd)>>4)&1, 0, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VPADAL_gen(D, size, Vd, op, Q, M, Vm)   (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | (Vd)<<12 | 0b0110<<8 | (op)<<7 | (Q)<<6 | (M)<<5 | (Vm))
// Add pair of U8 from Dm, Add result in U16 in Dd (some position as U8U8)
#define VPADAL_U8(Dd, Dm)   EMIT(VPADAL_gen(((Dd)>>4)&1, 0b00, (Dd)&15, 1, 0, ((Dm)>>4)&1, (Dm)&15))
// Add pair of U16 from Dm, Add result in U32 in Dd
#define VPADAL_U16(Dd, Dm)  EMIT(VPADAL_gen(((Dd)>>4)&1, 0b01, (Dd)&15, 1, 0, ((Dm)>>4)&1, (Dm)&15))
// Add pair of U32 from Dm, Add result in U64 in Dd
#define VPADAL_U32(Dd, Dm)  EMIT(VPADAL_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 1, 0, ((Dm)>>4)&1, (Dm)&15))
// Add pair of S8 from Dm, Add result in S16 in Dd (some position as S8S8)
#define VPADAL_S8(Dd, Dm)   EMIT(VPADAL_gen(((Dd)>>4)&1, 0b00, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
// Add pair of S16 from Dm, Add result in S32 in Dd
#define VPADAL_S16(Dd, Dm)  EMIT(VPADAL_gen(((Dd)>>4)&1, 0b01, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
// Add pair of S32 from Dm, Add result in S64 in Dd
#define VPADAL_S32(Dd, Dm)  EMIT(VPADAL_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
// Add pair of U8 from Dm, Add result in U16 in Dd (some position as U8U8)
#define VPADALQ_U8(Dd, Dm)  EMIT(VPADAL_gen(((Dd)>>4)&1, 0b00, (Dd)&15, 1, 1, ((Dm)>>4)&1, (Dm)&15))
// Add pair of U16 from Dm, Add result in U32 in Dd
#define VPADALQ_U16(Dd, Dm) EMIT(VPADAL_gen(((Dd)>>4)&1, 0b01, (Dd)&15, 1, 1, ((Dm)>>4)&1, (Dm)&15))
// Add pair of U32 from Dm, Add result in U64 in Dd
#define VPADALQ_U32(Dd, Dm) EMIT(VPADAL_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 1, 1, ((Dm)>>4)&1, (Dm)&15))
// Add pair of S8 from Dm, Add result in S16 in Dd (some position as S8S8)
#define VPADALQ_S8(Dd, Dm)  EMIT(VPADAL_gen(((Dd)>>4)&1, 0b00, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
// Add pair of S16 from Dm, Add result in S32 in Dd
#define VPADALQ_S16(Dd, Dm) EMIT(VPADAL_gen(((Dd)>>4)&1, 0b01, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
// Add pair of S32 from Dm, Add result in S64 in Dd
#define VPADALQ_S32(Dd, Dm) EMIT(VPADAL_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))

#define VPADDL_gen(D, size, Vd, op, Q, M, Vm)   (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | (Vd)<<12 | 0b0010<<8 | (op)<<7 | (Q)<<6 | (M)<<5 | (Vm))
// Add pair of U8, store result in U16
#define VPADDL_U8(Dd, Dm)   EMIT(VPADDL_gen(((Dd)>>4)&1, 0b00, (Dd)&15, 1, 0, ((Dm)>>4)&1, (Dm)&15))
// Add pair of U16, store result in U32
#define VPADDL_U16(Dd, Dm)  EMIT(VPADDL_gen(((Dd)>>4)&1, 0b01, (Dd)&15, 1, 0, ((Dm)>>4)&1, (Dm)&15))
// Add pair of U32, store result in U64
#define VPADDL_U32(Dd, Dm)  EMIT(VPADDL_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 1, 0, ((Dm)>>4)&1, (Dm)&15))
// Add pair of S8, store result in S16
#define VPADDL_S8(Dd, Dm)   EMIT(VPADDL_gen(((Dd)>>4)&1, 0b00, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
// Add pair of S16, store result in S32
#define VPADDL_S16(Dd, Dm)  EMIT(VPADDL_gen(((Dd)>>4)&1, 0b01, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
// Add pair of S32, store result in S64
#define VPADDL_S32(Dd, Dm)  EMIT(VPADDL_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
// Add pair of U8, store result in U16
#define VPADDLQ_U8(Dd, Dm)  EMIT(VPADDL_gen(((Dd)>>4)&1, 0b00, (Dd)&15, 1, 1, ((Dm)>>4)&1, (Dm)&15))
// Add pair of U16, store result in U32
#define VPADDLQ_U16(Dd, Dm) EMIT(VPADDL_gen(((Dd)>>4)&1, 0b01, (Dd)&15, 1, 1, ((Dm)>>4)&1, (Dm)&15))
// Add pair of U32, store result in U64
#define VPADDLQ_U32(Dd, Dm) EMIT(VPADDL_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 1, 1, ((Dm)>>4)&1, (Dm)&15))
// Add pair of S8, store result in S16
#define VPADDLQ_S8(Dd, Dm)  EMIT(VPADDL_gen(((Dd)>>4)&1, 0b00, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
// Add pair of S16, store result in S32
#define VPADDLQ_S16(Dd, Dm) EMIT(VPADDL_gen(((Dd)>>4)&1, 0b01, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
// Add pair of S32, store result in S64
#define VPADDLQ_S32(Dd, Dm) EMIT(VPADDL_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))

#define VPADD_gen(D, size, Vn, Vd, N, Q, M, Vm)    (0b1111<<28 | 0b0010<<24 | (D)<<22 | (size)<<20 | (Vn)<<16 | (Vd)<<12 | 0b1011<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 1<<4 | (Vm))
#define VPADD_8(Dd, Dn, Dm)     EMIT(VPADD_gen(((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VPADD_16(Dd, Dn, Dm)    EMIT(VPADD_gen(((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VPADD_32(Dd, Dn, Dm)    EMIT(VPADD_gen(((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
// NO VPADDQ....

#define VPADD_Fgen(D, size, Vn, Vd, N, Q, M, Vm)   (0b1111<<28 | 0b0011<<24 | (D)<<22 | (size)<<20 | (Vn)<<16 | (Vd)<<12 | 0b1101<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 0<<4 | (Vm))
// Add pair of F32, store result in F32
#define VPADD_F32(Dd, Dn, Dm)   EMIT(VPADD_Fgen(((Dd)>>4)&1, 0, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))

#define VMINMAX_gen(U, D, size, Vn, Vd, N, Q, M, op, Vm)    (0b1111<<28 | 0b001<<25 | (U)<<24 | (D)<<22 | (size)<<20 | (Vn)<<16 | (Vd)<<12 | 0b110<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | (op)<<4 | (Vm))
#define VMIN_U8(Dd, Dn, Dm)     EMIT(VMINMAX_gen(1, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, 1, (Dm)&15))
#define VMIN_U16(Dd, Dn, Dm)    EMIT(VMINMAX_gen(1, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, 1, (Dm)&15))
#define VMIN_U32(Dd, Dn, Dm)    EMIT(VMINMAX_gen(1, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, 1, (Dm)&15))
#define VMIN_U64(Dd, Dn, Dm)    EMIT(VMINMAX_gen(1, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, 1, (Dm)&15))
#define VMIN_S8(Dd, Dn, Dm)     EMIT(VMINMAX_gen(0, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, 1, (Dm)&15))
#define VMIN_S16(Dd, Dn, Dm)    EMIT(VMINMAX_gen(0, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, 1, (Dm)&15))
#define VMIN_S32(Dd, Dn, Dm)    EMIT(VMINMAX_gen(0, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, 1, (Dm)&15))
#define VMIN_S64(Dd, Dn, Dm)    EMIT(VMINMAX_gen(0, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, 1, (Dm)&15))
#define VMINQ_U8(Dd, Dn, Dm)    EMIT(VMINMAX_gen(1, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, 1, (Dm)&15))
#define VMINQ_U16(Dd, Dn, Dm)   EMIT(VMINMAX_gen(1, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, 1, (Dm)&15))
#define VMINQ_U32(Dd, Dn, Dm)   EMIT(VMINMAX_gen(1, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, 1, (Dm)&15))
#define VMINQ_U64(Dd, Dn, Dm)   EMIT(VMINMAX_gen(1, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, 1, (Dm)&15))
#define VMINQ_S8(Dd, Dn, Dm)    EMIT(VMINMAX_gen(0, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, 1, (Dm)&15))
#define VMINQ_S16(Dd, Dn, Dm)   EMIT(VMINMAX_gen(0, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, 1, (Dm)&15))
#define VMINQ_S32(Dd, Dn, Dm)   EMIT(VMINMAX_gen(0, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, 1, (Dm)&15))
#define VMINQ_S64(Dd, Dn, Dm)   EMIT(VMINMAX_gen(0, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, 1, (Dm)&15))
#define VMAX_U8(Dd, Dn, Dm)     EMIT(VMINMAX_gen(1, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, 0, (Dm)&15))
#define VMAX_U16(Dd, Dn, Dm)    EMIT(VMINMAX_gen(1, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, 0, (Dm)&15))
#define VMAX_U32(Dd, Dn, Dm)    EMIT(VMINMAX_gen(1, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, 0, (Dm)&15))
#define VMAX_U64(Dd, Dn, Dm)    EMIT(VMINMAX_gen(1, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, 0, (Dm)&15))
#define VMAX_S8(Dd, Dn, Dm)     EMIT(VMINMAX_gen(0, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, 0, (Dm)&15))
#define VMAX_S16(Dd, Dn, Dm)    EMIT(VMINMAX_gen(0, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, 0, (Dm)&15))
#define VMAX_S32(Dd, Dn, Dm)    EMIT(VMINMAX_gen(0, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, 0, (Dm)&15))
#define VMAX_S64(Dd, Dn, Dm)    EMIT(VMINMAX_gen(0, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, 0, (Dm)&15))
#define VMAXQ_U8(Dd, Dn, Dm)    EMIT(VMINMAX_gen(1, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, 0, (Dm)&15))
#define VMAXQ_U16(Dd, Dn, Dm)   EMIT(VMINMAX_gen(1, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, 0, (Dm)&15))
#define VMAXQ_U32(Dd, Dn, Dm)   EMIT(VMINMAX_gen(1, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, 0, (Dm)&15))
#define VMAXQ_U64(Dd, Dn, Dm)   EMIT(VMINMAX_gen(1, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, 0, (Dm)&15))
#define VMAXQ_S8(Dd, Dn, Dm)    EMIT(VMINMAX_gen(0, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, 0, (Dm)&15))
#define VMAXQ_S16(Dd, Dn, Dm)   EMIT(VMINMAX_gen(0, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, 0, (Dm)&15))
#define VMAXQ_S32(Dd, Dn, Dm)   EMIT(VMINMAX_gen(0, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, 0, (Dm)&15))
#define VMAXQ_S64(Dd, Dn, Dm)   EMIT(VMINMAX_gen(0, ((Dd)>>4)&1, 0b11, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, 0, (Dm)&15))

#define VRHADD_gen(U, D, size, Vn, Vd, N, Q, M, Vm) (0b1111<<28 | 1<<25 | (U)<<24 | (D)<<22 | (size)<<20 | (Vn)<<16 | (Vd)<<12 | 0b0001<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | (Vm))
// Dd <= (Dn + Dm + 1)>>1
#define VRHADD_U8(Dd, Dn, Dm)   EMIT(VRHADD_gen(1, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VRHADD_U16(Dd, Dn, Dm)  EMIT(VRHADD_gen(1, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VRHADD_U32(Dd, Dn, Dm)  EMIT(VRHADD_gen(1, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VRHADD_S8(Dd, Dn, Dm)   EMIT(VRHADD_gen(0, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VRHADD_S16(Dd, Dn, Dm)  EMIT(VRHADD_gen(0, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VRHADD_S32(Dd, Dn, Dm)  EMIT(VRHADD_gen(0, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VRHADDQ_U8(Dd, Dn, Dm)  EMIT(VRHADD_gen(1, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VRHADDQ_U16(Dd, Dn, Dm) EMIT(VRHADD_gen(1, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VRHADDQ_U32(Dd, Dn, Dm) EMIT(VRHADD_gen(1, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VRHADDQ_S8(Dd, Dn, Dm)  EMIT(VRHADD_gen(0, ((Dd)>>4)&1, 0b00, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VRHADDQ_S16(Dd, Dn, Dm) EMIT(VRHADD_gen(0, ((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VRHADDQ_S32(Dd, Dn, Dm) EMIT(VRHADD_gen(0, ((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VQRDMULH_gen(D, size, Vn, Vd, N, Q, M, Vm)  (0b1111<<28 | 0b0011 <<24 | (D)<<22 | (size)<<20 | (Vn)<<16 | (Vd)<<12 | 0b1011<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | (Vm))
// Dd <= (2*Dn*Dm + 1<<(size-1))>>size
#define VQRDMULH_S16(Dd, Dn, Dm)    EMIT(VQRDMULH_gen(((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VQRDMULH_S32(Dd, Dn, Dm)    EMIT(VQRDMULH_gen(((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VQRDMULHQ_S16(Dd, Dn, Dm)   EMIT(VQRDMULH_gen(((Dd)>>4)&1, 0b01, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))
#define VQRDMULHQ_S32(Dd, Dn, Dm)   EMIT(VQRDMULH_gen(((Dd)>>4)&1, 0b10, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#define VDUP_sgen(D, imm4, Vd, Q, M, Vm)    (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (imm4)<<16 | (Vd)<<12 | 0b1100<<8 | (Q)<<6 | (M)<<5 | (Vm))
// Dd <= Dup(Dm[x])
#define VDUP_8(Dd, Dm, x)       EMIT(VDUP_sgen(((Dd)>>4)&1, (((x)<<1)|0b1), (Dd)&15, 0, ((Dm)>>4)&1, (Dm)&15))
#define VDUPQ_8(Dd, Dm, x)      EMIT(VDUP_sgen(((Dd)>>4)&1, (((x)<<1)|0b1), (Dd)&15, 1, ((Dm)>>4)&1, (Dm)&15))
#define VDUP_16(Dd, Dm, x)      EMIT(VDUP_sgen(((Dd)>>4)&1, (((x)<<2)|0b10), (Dd)&15, 0, ((Dm)>>4)&1, (Dm)&15))
#define VDUPQ_16(Dd, Dm, x)     EMIT(VDUP_sgen(((Dd)>>4)&1, (((x)<<2)|0b10), (Dd)&15, 1, ((Dm)>>4)&1, (Dm)&15))
#define VDUP_32(Dd, Dm, x)      EMIT(VDUP_sgen(((Dd)>>4)&1, (((x)<<3)|0b100), (Dd)&15, 0, ((Dm)>>4)&1, (Dm)&15))
#define VDUPQ_32(Dd, Dm, x)     EMIT(VDUP_sgen(((Dd)>>4)&1, (((x)<<3)|0b100), (Dd)&15, 1, ((Dm)>>4)&1, (Dm)&15))
#define VDUPQ_64(Dd, Dm, x)     EMIT(VDUP_sgen(((Dd)>>4)&1, (((x)<<4)|0b1000), (Dd)&15, 1, ((Dm)>>4)&1, (Dm)&15))

#define VREV64_gen(D, size, Vd, Q, M, Vm)   (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | (Vd)<<12 | (Q)<<6 | (M)<<5 | (Vm))
#define VREV64_32(Dd, Dm)       EMIT(VREV64_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 0, ((Dm)>>4)&1, (Dm)&15))
#define VREV64Q_32(Dd, Dm)      EMIT(VREV64_gen(((Dd)>>4)&1, 0b10, (Dd)&15, 1, ((Dm)>>4)&1, (Dm)&15))

#define VABS_vgen(D, size, Vd, F, Q, M, Vm) (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | 0b01<<16 | (Vd)<<12 | (F)<<10 | 0b110<<7 | (Q)<<6 | (M)<<5 | (Vm))
#define VABS_S32(Dd, Dm)        EMIT(VABS_vgen(((Dd)>>4)&1, 2, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VABS_S16(Dd, Dm)        EMIT(VABS_vgen(((Dd)>>4)&1, 1, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VABS_S8(Dd, Dm)         EMIT(VABS_vgen(((Dd)>>4)&1, 0, (Dd)&15, 0, 0, ((Dm)>>4)&1, (Dm)&15))
#define VABS_F(Dd, Dm)          EMIT(VABS_vgen(((Dd)>>4)&1, 2, (Dd)&15, 1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VABSQ_S32(Dd, Dm)       EMIT(VABS_vgen(((Dd)>>4)&1, 2, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VABSQ_S16(Dd, Dm)       EMIT(VABS_vgen(((Dd)>>4)&1, 1, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VABSQ_S8(Dd, Dm)        EMIT(VABS_vgen(((Dd)>>4)&1, 0, (Dd)&15, 0, 1, ((Dm)>>4)&1, (Dm)&15))
#define VABSQ_F(Dd, Dm)         EMIT(VABS_vgen(((Dd)>>4)&1, 2, (Dd)&15, 1, 1, ((Dm)>>4)&1, (Dm)&15))

// AES
#define AESD_gen(D, size, Vd, M, Vm)    (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | (Vd)<<12 | 0b0110<<7 | 1<<6 | (M)<<5 | (Vm))
#define AESD(Dd, Dm)            EMIT(AESD_gen(((Dd)>>4)&1, 0, (Dd)&15, ((Dm)>>4)&1, (Dm)&15))

#define AESE_gen(D, size, Vd, M, Vm)    (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | (Vd)<<12 | 0b0110<<7 | 0<<6 | (M)<<5 | (Vm))
#define AESE(Dd, Dm)            EMIT(AESE_gen(((Dd)>>4)&1, 0, (Dd)&15, ((Dm)>>4)&1, (Dm)&15))

#define AESIMC_gen(D, size, Vd, M, Vm)  (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | (Vd)<<12 | 0b0111<<7 | 1<<6 | (M)<<5 | (Vm))
#define AESIMC(Dd, Dm)          EMIT(AESIMC_gen(((Dd)>>4)&1, 0, (Dd)&15, ((Dm)>>4)&1, (Dm)&15))

#define AESMC_gen(D, size, Vd, M, Vm)   (0b1111<<28 | 0b0011<<24 | 1<<23 | (D)<<22 | 0b11<<20 | (size)<<18 | (Vd)<<12 | 0b0111<<7 | 0<<6 | (M)<<5 | (Vm))
#define AESMC(Dd, Dm)           EMIT(AESMC_gen(((Dd)>>4)&1, 0, (Dd)&15, ((Dm)>>4)&1, (Dm)&15))

#endif  //__ARM_EMITTER_H__
