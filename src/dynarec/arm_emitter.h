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
#define xEIP    12
// scratch registers
#define x1      1
#define x2      2
#define x3      3
#define x12     12
// emu is r0
#define xEmu    0
// ARM SP is r13
#define xSP     13      

// barel roll operations (4 possibles)
#define brLSL(i, r) (0<<4 | 0<<5 | ((i&31)<<7) | r)
#define brLSR(i, r) (0<<4 | 1<<5 | ((i&31)<<7) | r)
#define brASR(i, r) (0<<4 | 2<<5 | ((i&31)<<7) | r)
#define brROR(i, r) (0<<4 | 3<<5 | ((i&31)<<7) | r)
#define brIMM(r)    (r)
// barel roll with a register
#define brRLSL(i, r) (1<<4 | 0<<5 | ((i&15)<<8) | r)
#define brRLSR(i, r) (1<<4 | 1<<5 | ((i&15)<<8) | r)
#define brRASR(i, r) (1<<4 | 2<<5 | ((i&15)<<8) | r)
#define brRROR(i, r) (1<<4 | 3<<5 | ((i&15)<<8) | r)


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
// movxx dst, src
#define MOV_REG_COND(cond, dst, src) EMIT( cond | 0x01a00000 | ((dst) << 12) | (src) )

// movw dst, #imm16
#define MOVW(dst, imm16) EMIT(0xe3000000 | ((dst) << 12) | (((imm16) & 0xf000) << 4) | brIMM((imm16) & 0x0fff) )
// movt dst, #imm16
#define MOVT(dst, imm16) EMIT(0xe3400000 | ((dst) << 12) | (((imm16) & 0xf000) << 4) | brIMM((imm16) & 0x0fff) )
// pseudo insruction: mov reg, #imm with imm a 32bits value
#define MOV32(dst, imm32)                   \
    MOVW(dst, ((uint32_t)imm32)&0xffff);    \
    if (((uint32_t)imm32)>>16) {            \
        MOVT(dst, (((uint32_t)imm32)>>16)); }
// pseudo insruction: mov reg, #imm with imm a 32bits value, fixed size
#define MOV32_(dst, imm32)                   \
    MOVW(dst, ((uint32_t)imm32)&0xffff);    \
    MOVT(dst, (((uint32_t)imm32)>>16))
// movw.cond dst, #imm16
#define MOVW_COND(cond, dst, imm16) EMIT(cond | 0x03000000 | ((dst) << 12) | (((imm16) & 0xf000) << 4) | brIMM((imm16) & 0x0fff) )
// mov dst #imm8 ror imm4*2
#define MOV_IMM(dst, imm8, rimm4) EMIT(0xe3a00000 | ((dst) << 12) | (imm8) | ((rimm4) << 7) )

// mov dst, src lsl imm5
#define MOV_REG_LSL_IMM5(dst, src, imm5) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (0<<4) | (0<<5) | (imm5<<7))
// mov dst, src lsr imm5
#define MOV_REG_LSR_IMM5(dst, src, imm5) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (0<<4) | (1<<5) | (imm5<<7))
// mov dst, src asr imm5
#define MOV_REG_ASR_IMM5(dst, src, imm5) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (0<<4) | (2<<5) | (imm5<<7))
// mov dst, src ror imm5
#define MOV_REG_ROR_IMM5(dst, src, imm5) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (0<<4) | (3<<5) | (imm5<<7))

// mov dst, src lsl rs
#define MOV_REG_LSL_REG(dst, src, rs) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (1<<4) | (0<<5) | (rs<<8))
// mov dst, src lsr rs
#define MOV_REG_LSR_REG(dst, src, rs) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (1<<4) | (1<<5) | (rs<<8))
// mov dst, src asr rs
#define MOV_REG_ASR_REG(dst, src, rs) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (1<<4) | (2<<5) | (rs<<8))
// mov dst, src ror rs
#define MOV_REG_ROR_REG(dst, src, rs) EMIT(0xe1a00000 | ((dst) << 12) | (src) | (1<<4) | (3<<5) | (rs<<8))

// sub dst, src, #(imm8)
#define SUB_IMM8(dst, src, imm8) \
    EMIT(0xe2400000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// sub cond dst, src, #(imm8)
#define SUB_COND_IMM8(dst, src, imm8) \
    EMIT((cond) | 0x02400000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// add.s dst, src, #(imm8)
#define SUBS_IMM8(dst, src, imm8) \
    EMIT(0xe2500000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// add dst, src1, src2, lsl #imm
#define SUB_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0400000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// add.s dst, src1, src2, lsl #imm
#define SUBS_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0500000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )

// rsb dst, src, #(imm8)
#define RSB_IMM8(dst, src, imm8) \
    EMIT(0xe2600000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// rsb cond dst, src, #(imm8)
#define RSB_COND_IMM8(cond, dst, src, imm8) \
    EMIT((cond) | 0x02600000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )

// and dst, src1, src2, lsl #imm
#define AND_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0000000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// and.s dst, src1, src2, lsl #imm
#define ANDS_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0100000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// and dst, src1, src2, lsr #imm
#define AND_REG_LSR_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0000000 | ((dst) << 12) | ((src1) << 16) | brLSR(imm8, src2) )
// and dst, src, #(imm8)
#define AND_IMM8(dst, src, imm8) \
    EMIT(0xe2000000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// and.s dst, src, #(imm8)
#define ANDS_IMM8(dst, src, imm8) \
    EMIT(0xe2100000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// add dst, src, #(imm8)
#define ADD_IMM8(dst, src, imm8) \
    EMIT(0xe2800000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// add.s dst, src, #(imm8)
#define ADDS_IMM8(dst, src, imm8) \
    EMIT(0xe2900000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// add dst, src1, src2, lsl #imm
#define ADD_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0800000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// add.s dst, src1, src2, lsl #imm
#define ADDS_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0900000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// add dst, src1, src2, lsr #imm
#define ADD_REG_LSR_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0800000 | ((dst) << 12) | ((src1) << 16) | brLSR(imm8, src2) )
// cmp.s dst, src1, src2, lsl #imm
#define CMPS_REG_LSL_IMM8(src1, src2, imm8) \
    EMIT(0xe1500000 | ((0) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// cmp.s dst, src, #imm
#define CMPS_IMM8(src, imm8) \
    EMIT(0xe3500000 | ((0) << 12) | ((src) << 16) | brIMM(imm8) )
// tst.s dst, src1, src2, lsl #imm
#define TSTS_REG_LSL_IMM8(src1, src2, imm8) \
    EMIT(0xe1100000 | ((0) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// tst.s dst, src1, #imm
#define TSTS_IMM8(src, imm8) \
    EMIT(0xe3100000 | ((0) << 12) | ((src) << 16) | brIMM(imm8) )
// tst.s dst, src1, #imm ror rot*2
#define TSTS_IMM8_ROR(src, imm8, rot) \
    EMIT(0xe3100000 | ((0) << 12) | ((src) << 16) | ((rot)<<8) | brIMM(imm8) )
// orr dst, src1, src2, lsl #imm
#define ORR_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe1800000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// orr.s dst, src1, src2, lsl #imm
#define ORRS_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe1900000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// orr dst, src1, #imm8
#define ORR_IMM8(dst, src, imm8, rot) \
    EMIT(0xe3800000 | ((dst) << 12) | ((src) << 16) | ((rot)<<8) | imm8 )
// orr dst, src1, src2, lsl #rs
#define ORR_REG_LSL_REG(dst, src1, src2, rs) \
    EMIT(0xe1800000 | ((dst) << 12) | ((src1) << 16) | brRLSL(rs, src2) )
// xor dst, src1, src2, lsl #imm
#define XOR_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0200000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// xor.s dst, src1, src2, lsl #imm
#define XORS_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0300000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// xor dst, src, #(imm8)
#define XOR_IMM8(dst, src, imm8) \
    EMIT(0xe2200000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// xor dst, src1, src2, lsl #rs
#define XOR_REG_LSL_REG(dst, src1, src2, rs) \
    EMIT(0xe0200000 | ((dst) << 12) | ((src1) << 16) | brRLSL(rs, src2) )
// xor dst, src1, src2, lsl #imm
#define XOR_REG_LSR_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0200000 | ((dst) << 12) | ((src1) << 16) | brLSR(imm8, src2) )
// bic dst, src1, src2, lsl #imm
#define BIC_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe1c00000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// bic dst, src, IMM8
#define BIC_IMM8(dst, src, imm8, rot) \
    EMIT(0xe3c00000 | ((dst) << 12) | ((src) << 16) | ((rot)<<8) | imm8 )
// bic dst, src1, src2, lsl #imm
#define BIC_REG_LSL_REG(dst, src1, src2, rs) \
    EMIT(0xe1c00000 | ((dst) << 12) | ((src1) << 16) | brRLSL(rs, src2) )
// mvn dst, src1, src2, lsl #imm
#define MVN_REG_LSL_IMM8(dst, rm, imm8) \
    EMIT(0xe1e00000 | ((dst) << 12) | (0 << 16) | brLSL(imm8, rm) )
// mvn dst, src, IMM8
#define MVN_IMM8(dst, src, imm8, rot) \
    EMIT(0xe3e00000 | ((dst) << 12) | ((src) << 16) | ((rot)<<8) | imm8 )

// Single data transfert construction
#define SDT_REG(Cond, P, U, B, W, L, Rn, Rd, ShiftRm) (Cond | (0b00<<26) | (1<<25) | (P<<24) | (U<<23) | (B<<22) | (U<<23) | (W<<21) | (L<<20) | (Rn<<16) | (Rd<<12) | ShiftRm)
#define SDT_OFF(Cond, P, U, B, W, L, Rn, Rd, Imm12)   (Cond | (0b00<<26) | (0<<25) | (P<<24) | (U<<23) | (B<<22) | (U<<23) | (W<<21) | (L<<20) | (Rn<<16) | (Rd<<12) | Imm12)
// ldr reg, [addr, #imm9]
#define LDR_IMM9(reg, addr, imm9) EMIT(0xe5900000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// ldrxx reg, [addr, #imm9]
#define LDR_IMM9_COND(cond, reg, addr, imm9) EMIT(cond | 0x05900000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// ldrb reg, [addr, #imm9]
#define LDRB_IMM9(reg, addr, imm9) EMIT(0xe5d00000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// ldr reg, [addr, #imm9]!
#define LDR_IMM9_W(reg, addr, imm9) EMIT(0xe5b00000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// ldr reg, [addr, #-imm9]!
#define LDR_NIMM9_W(reg, addr, imm9) EMIT(0xe5300000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// ldr reg, [addr, rm lsl imm5]
#define LDR_REG_LSL_IMM5(reg, addr, rm, imm5) EMIT(c__ | (0b011<<25) | (1<<24) | (1<<23) | (0<<21) | (1<<20) | ((reg) << 12) | ((addr) << 16) | brLSL(imm5, rm) )
// ldrb reg, [addr, rm lsl imm5]
#define LDRB_REG_LSL_IMM5(reg, addr, rm, imm5) EMIT(0xe5d00000 | ((reg) << 12) | ((addr) << 16) | (1<<25) | brLSL(imm5, rm) )
// ldr reg, [addr], #imm9
#define LDRAI_IMM9_W(reg, addr, imm9)   EMIT(0xe4900000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// ldr reg, [addr], #-imm9
#define LDRAI_NIMM9_W(reg, addr, imm9)  EMIT(0xe4100000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// ldrb reg, [addr], #imm9
#define LDRBAI_IMM9_W(reg, addr, imm9)  EMIT(0xe4900000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// ldrb reg, [addr], #-imm9
#define LDRBAI_NIMM9_W(reg, addr, imm9) EMIT(0xe4100000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// ldr reg, [addr], rm lsl imm5
#define LDRAI_REG_LSL_IMM5(reg, addr, rm, imm5)  EMIT(0xe6900000 | ((reg) << 12) | ((addr) << 16) | (1<<25) | brLSL(imm5, rm) )
// ldrb reg, [addr], rm lsl imm5
#define LDRBAI_REG_LSL_IMM5(reg, addr, rm, imm5) EMIT(0xe6d00000 | ((reg) << 12) | ((addr) << 16) | (1<<25) | brLSL(imm5, rm) )
// ldrd reg, reg+1, [addr, #imm9], reg must be even, reg+1 is implicit
#define LDRD_IMM8(reg, addr, imm8) EMIT(c__ | 0b000<<25 | 1<<24 | 1<<23 | 1<<22 | 0<<21 | 0<<20 | ((reg) << 12) | ((addr) << 16) | ((imm8)&0xf0)<<(8-4) | (0b1101<<4) | ((imm8)&0x0f) )

// str reg, [addr, #imm9]
#define STR_IMM9(reg, addr, imm9) EMIT(0xe5800000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// strb reg, [addr, #imm9]
#define STRB_IMM9(reg, addr, imm9) EMIT(0xe5c00000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// str reg, [addr], #imm9
#define STRAI_IMM9_W(reg, addr, imm9)  EMIT(0xe4800000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// str reg, [addr], #-imm9
#define STRAI_NIMM9_W(reg, addr, imm9) EMIT(0xe4000000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// str reg, [addr, #-(imm9)]!
#define STR_NIMM9_W(reg, addr, imm9) EMIT(0xe5200000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// str reg, [addr, rm lsl imm5]
#define STR_REG_LSL_IMM5(reg, addr, rm, imm5) EMIT(0xe5800000 | ((reg) << 12) | ((addr) << 16) | (1<<25) | brLSL(imm5, rm) )
// strb reg, [addr, rm lsl imm5]
#define STRB_REG_LSL_IMM5(reg, addr, rm, imm5) EMIT(0xe5c00000 | ((reg) << 12) | ((addr) << 16) | (1<<25) | brLSL(imm5, rm) )
// strb reg, [addr], #imm9
#define STRBAI_IMM9_W(reg, addr, imm9)  EMIT(0xe4c00000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// strb reg, [addr], #-imm9
#define STRBAI_NIMM9_W(reg, addr, imm9) EMIT(0xe4400000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// str reg, [addr], rm lsl imm5
#define STRAI_REG_LSL_IMM5(reg, addr, rm, imm5)  EMIT(0xe6800000 | ((reg) << 12) | ((addr) << 16) | (1<<25) | brLSL(imm5, rm) )
// strb reg, [addr], rm lsl imm5
#define STRBAI_REG_LSL_IMM5(reg, addr, rm, imm5) EMIT(0xe6c00000 | ((reg) << 12) | ((addr) << 16) | (1<<25) | brLSL(imm5, rm) )
// strd reg, reg+1, [addr, #imm8], reg must be even, reg+1 is implicit
#define STRD_IMM8(reg, addr, imm8) EMIT(c__ | 0b000<<25 | 1<<24 | 1<<23 | 1<<22 | 0<<21 | 0<<20 | ((reg) << 12) | ((addr) << 16) | ((imm8)&0xf0)<<(8-4) | (0b1111<<4) | ((imm8)&0x0f) )

// bx reg
#define BX(reg) EMIT(0xe12fff10 | (reg) )
// bx cond reg
#define BXcond(C, reg) EMIT(C | 0x012fff10 | (reg) )

// blx reg
#define BLX(reg) EMIT(0xe12fff30 | (reg) )

// b cond offset
#define Bcond(C, O) EMIT(C | (0b101<<25) | (0<<24) | ((O)>>2)&0xffffff)

// bl cond offset
#define BLcond(C, O) EMIT(C | (0b101<<25) | (1<<24) | ((O)>>2)&0xffffff)

// push reg!, {list}
//                           all |    const    |pre-index| subs    | no PSR  |writeback| store   |   base    |reg list
#define PUSH(reg, list) EMIT(c__ | (0b100<<25) | (1<<24) | (0<<23) | (0<<22) | (1<<21) | (0<<20) | (reg<<16) | (list))

// pop reg!, {list}
//                           all |    const    |postindex|  add    | no PSR  |writeback|  load   |   base    |reg list
#define POP(reg, list)  EMIT(c__ | (0b100<<25) | (0<<24) | (1<<23) | (0<<22) | (1<<21) | (1<<20) | (reg<<16) | (list))

// STMDB reg, {list}
//                            all |    const    |pre-index| subs    | no PSR  |  no wb  | store   |   base    |reg list
#define STMDB(reg, list) EMIT(c__ | (0b100<<25) | (1<<24) | (0<<23) | (0<<22) | (0<<21) | (0<<20) | (reg<<16) | (list))
// STMia reg, {list}
//                          all |    const    |postindex|   add   | no PSR  |  no wb  |  store  |   base    |reg list
#define STM(reg, list) EMIT(c__ | (0b100<<25) | (0<<24) | (1<<23) | (0<<22) | (0<<21) | (0<<20) | (reg<<16) | (list))
// LDMia reg, {list}
//                          all |    const    |postindex|   add   | no PSR  |  no wb  |  load   |   base    |reg list
#define LDM(reg, list) EMIT(c__ | (0b100<<25) | (0<<24) | (1<<23) | (0<<22) | (0<<21) | (1<<20) | (reg<<16) | (list))


// Half Word and signed data transfert construction
#define HWS_REG(Cond, P, U, W, L, Rn, Rd, S, H, Rm)     (Cond | (0b000<<25) | (P<<24) | (U<<23) | (0<<22) | (W<<21) | (L<<20) | (Rn<<16) | (Rd<<12) | (1<<7) | (S<<6) | (H<<5) | (1<<4) | Rm)
#define HWS_OFF(Cond, P, U, W, L, Rn, Rd, S, H, Imm8)   (Cond | (0b000<<25) | (P<<24) | (U<<23) | (1<<22) | (W<<21) | (L<<20) | (Rn<<16) | (Rd<<12) | ((Imm8&0xf0)<<(8-4)) | (1<<7) | (S<<6) | (H<<5) | (1<<4) | (Imm8&0x0f))

#define LDRSB_IMM8(reg, addr, imm8) EMIT(HWS_OFF(c__, 1, 1, 0, 1, addr, reg, 1, 0, imm8))
#define LDRSH_IMM8(reg, addr, imm8) EMIT(HWS_OFF(c__, 1, 1, 0, 1, addr, reg, 1, 1, imm8))
#define LDRH_IMM8(reg, addr, imm8) EMIT(HWS_OFF(c__, 1, 1, 0, 1, addr, reg, 0, 1, imm8))
#define LDRH_IMM8_COND(cond, reg, addr, imm8) EMIT(HWS_OFF(cond, 1, 1, 0, 1, addr, reg, 0, 1, imm8))
#define STRH_IMM8(reg, addr, imm8) EMIT(HWS_OFF(c__, 1, 1, 0, 0, addr, reg, 0, 1, imm8))
#define STRSH_IMM8(reg, addr, imm8) EMIT(HWS_OFF(c__, 1, 1, 0, 0, addr, reg, 1, 1, imm8))

#define LDRHAI_REG_LSL_IMM5(reg, addr, rm) EMIT(HWS_REG(c__, 0, 1, 0, 1, addr, reg, 0, 1, rm))
#define STRHAI_REG_LSL_IMM5(reg, addr, rm) EMIT(HWS_REG(c__, 0, 1, 0, 0, addr, reg, 0, 1, rm))

// Mul Long construction
#define MULLONG(Cond, U, A, S, RdHi, RdLo, Rs, Rm)     (Cond | (0b00001<<23) | (U<<22) | (A<<21) | (S<<20) | (RdHi<<16) | (RdLo<<12) | (Rs<<8) | (0b1001<<4) | (Rm))

#define UMULL(RdHi, RdLo, Rs, Rm)   EMIT(MULLONG(c__, 0, 0, 0, RdHi, RdLo, Rs, Rm))
#define SMULL(RdHi, RdLo, Rs, Rm)   EMIT(MULLONG(c__, 1, 0, 0, RdHi, RdLo, Rs, Rm))

// Mul and MulA
#define MULMULA(Cond, A, S, Rd, Rn, Rs, Rm)     (Cond | (0b000000<<22) | (A<<21) | (S<<20) | (Rd<<16) | (Rn<<12) | (Rs<<8) | (0b1001<<4) | (Rm))
#define MUL(Rd, Rm, Rn)     EMIT(MULMULA(c__, 0, 0, (Rd), 0, (Rm), (Rn)))

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

// BFI: Bit Field Insert: copy any number of low order bit from Rn to any position of Rd
#define BFI(rd, rn, lsb, width) EMIT(c__ | (0b0111110<<21) | (((lsb)+(width)-1)<<16) | ((rd)<<12) | ((lsb)<<7) | (0b001<<4) | (rn))
// BFI_COND: Bit Field Insert with condition: copy any number of low order bit from Rn to any position of Rd
#define BFI_COND(cond, rd, rn, lsb, width) EMIT(cond | (0b0111110<<21) | (((lsb)+(width)-1)<<16) | ((rd)<<12) | ((lsb)<<7) | (0b001<<4) | (rn))

// REV: Reverse byte of a 32bits word
#define REV(rd, rm) EMIT(c__ | (0b01101<<23) | (0<<22) | (0b11<<20) | (0b1111<<16) | ((rd)<<12) | (0b1111<<8) | (0b0011<<4) | (rm))


// VFPU
#define TRANSFERT64(C, op) ((0b1100<<24) | (0b010<<21) | (0b101<<9) | ((C)<<8) | ((op)<<4))

// Move from FPSCR to Arm register
#define VMRS(Rt)    EMIT(c__ | (0b1110<<24) | (0b1111<<20) | (0b0001<<16) | ((Rt)<<12) | (0b1010<<8) | (0b0001<<4) | (0b0000))
// Move to FPSCR from Arm register
#define VMSR(Rt)    EMIT(c__ | (0b1110<<24) | (0b1110<<20) | (0b0001<<16) | ((Rt)<<12) | (0b1010<<8) | (0b0001<<4) | (0b0000))
// Move to FPSCR from Arm flags APSR
#define VMRS_APSR()    VMRS(15)

// Move between Rt to Sm
#define VMOVtoV(Sm, Rt) EMIT(c__ | (0b1110<<24) | (0b000<<21) | (0<<20) | ((((Sm)&0b11110)>>1)<<16) | ((Rt)<<12) | (0b1010<<8) | (((Sm)&1)<<7) |(0b00<<6) | (1<<4))
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

// Load from memory to double  VLDR Dd, [Rn, #imm8]
#define VLDR_64(Dd, Rn, Imm8)    EMIT(c__ | (0b1101<<24) | (1<<23) | ((((Dd)>>4)&1)<<22) | (1<<20) | ((Rn)<<16) | (((Dd)&15)<<12) | (0b1011<<8) | ((Imm8)&255))
// Load from memory to single  VLDR Sd, [Rn, #imm8]
#define VLDR_32(Sd, Rn, Imm8)    EMIT(c__ | (0b1101<<24) | (1<<23) | ((((Sd)>>4)&1)<<22) | (1<<20) | ((Rn)<<16) | (((Sd)&15)<<12) | (0b1010<<8) | ((Imm8)&255))

// Store to memory to double  VSTR Dd, [Rn, #imm8]
#define VSTR_64(Dd, Rn, Imm8)    EMIT(c__ | (0b1101<<24) | (1<<23) | ((((Dd)>>4)&1)<<22) | (0<<20) | ((Rn)<<16) | (((Dd)&15)<<12) | (0b1011<<8) | ((Imm8)&255))
// Store to memory to single  VSTR Sd, [Rn, #imm8]
#define VSTR_32(Sd, Rn, Imm8)    EMIT(c__ | (0b1101<<24) | (1<<23) | ((((Sd)>>4)&1)<<22) | (0<<20) | ((Rn)<<16) | (((Sd)&15)<<12) | (0b1010<<8) | ((Imm8)&255))

// Convert from single Sm to double Dd
#define VCVT_F64_F32(Dd, Sm)    EMIT(c__ | (0b1110<<24) | (1<<23) |  ((((Dd)>>4)&1)<<22) | (0b11<<20) | (0b0111<<16) | (((Dd)&15)<<12) | (0b101<<9) | (0<<8) | (0b11<<6) | (((Sm)&1)<<5) | (0<<4) | (((Sm)>>1)&15))
// Convert from double Dm to single Sd
#define VCVT_F32_F64(Sd, Dm)    EMIT(c__ | (0b1110<<24) | (1<<23) |  (((Sd)&1)<<22) | (0b11<<20) | (0b0111<<16) | ((((Sd)>>4)&15)<<12) | (0b101<<9) | (1<<8) | (0b11<<6) | ((((Dm)>>4)&1)<<5) | (0<<4) | ((Dm)&15))

// Convert from double Dm to int32 Sd, with Round toward Zero mode
#define VCVT_S32_F64(Sd, Dm)    EMIT(c__ | (0b1110<<24) | (1<<23) | (((Sd)&1)<<22) | (0b111<<19) | (0b101<<16) | ((((Sd)>>4)&15)<<12) | (0b101<<9) | (1<<8) | (1<<7) | (1<<6) | ((((Dm)>>4)&1)<<5) | ((Dm)&15) )
// Convert from double Dm to int32 Sd, with Round selection from FPSCR
#define VCVTR_S32_F64(Sd, Dm)    EMIT(c__ | (0b1110<<24) | (1<<23) | (((Sd)&1)<<22) | (0b111<<19) | (0b101<<16) | ((((Sd)>>4)&15)<<12) | (0b101<<9) | (1<<8) | (0<<7) | (1<<6) | ((((Dm)>>4)&1)<<5) | ((Dm)&15) )
// Convert from int32 Sm to double Dd
#define VCVT_F64_S32(Dd, Sm)    EMIT(c__ | (0b1110<<24) | (1<<23) | ((((Dd)>>4)&1)<<22) | (0b111<<19) | (0b000<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (1<<7) | (1<<6) | (((Sm)&1)<<5) | (((Sm)>>1)&15) )

// Mutiply F64 Dd = Dn*Dm
#define VMUL_F64(Dd, Dn, Dm)    EMIT(c__ | (0b1110<<24) | (0<<23) | ((((Dd)>>4)&1)<<22) | (0b10<<20) | (((Dn)&15)<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (((Dn>>4)&1)<<7) | (((Dm>>4)&1)<<5) | ((Dm)&15) )

// Divide F64 Dd = Dn/Dm
#define VDIV_F64(Dd, Dn, Dm)    EMIT(c__ | (0b1110<<24) | (1<<23) | ((((Dd)>>4)&1)<<22) | (0b00<<20) | (((Dn)&15)<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (((Dn>>4)&1)<<7) | (((Dm>>4)&1)<<5) | ((Dm)&15) )

// Add F64 Dd = Dn + Dm
#define VADD_F64(Dd, Dn, Dm)    EMIT(c__ | (0b1110<<24) | (0<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (((Dn)&15)<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (((Dn>>4)&1)<<7) | (0<<6) | (((Dm>>4)&1)<<5) | ((Dm)&15) )

// Sub F64 Dd = Dn + Dm
#define VSUB_F64(Dd, Dn, Dm)    EMIT(c__ | (0b1110<<24) | (0<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (((Dn)&15)<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (((Dn>>4)&1)<<7) | (1<<6) | (((Dm>>4)&1)<<5) | ((Dm)&15) )

// Cmp between 2 double Dd and Dm
#define VCMP_F64(Dd, Dm)    EMIT(c__ | (0b1110<<24) | (1<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (0b0100<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (0<<7) | (1<<6) | (((Dm>>4)&1)<<5) | ((Dm)&15) )
// Cmp between 1 double Dd and 0.0
#define VCMP_F64_0(Dd)      EMIT(c__ | (0b1110<<24) | (1<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (0b0101<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (0<<7) | (1<<6) | (0<<5) | (0) )

// Neg F64 Dd = - Dm
#define VNEG_F64(Dd, Dm)    EMIT(c__ | (0b1110<<24) | (1<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (0b0001<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (0b01<<6) | (((Dm>>4)&1)<<5) | ((Dm)&15) )

// Sqrt F64 Dd = - Dm
#define VSQRT_F64(Dd, Dm)    EMIT(c__ | (0b1110<<24) | (1<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (0b0001<<16) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (0b11<<6) | (((Dm>>4)&1)<<5) | ((Dm)&15) )

// Abs Dd = |Dm|
#define VABS_F64(Dd, Dm)     EMIT(c__ | (0b11101<<23) | ((((Dd)>>4)&1)<<22) | (0b11<<20) | (((Dd)&15)<<12) | (0b101<<9) | (1<<8) | (0b11<<6) | ((((Dm)>>4)&1)<<5) | ((Dm)&15))

// NEON
// Move between Dd <- Dm and Dd+1 <- Dm+1 (2 instructions)
#define VMOVQ(Dd, Dm)   VMOV_64(Dd, Dm); VMOV_64(Dd+1, Dm+1)

// L is 1 for VLD1, 0 for VST1 Dd is V:Vd, type:0b0111=64, 0b1010=128, 0b0110=192, 0b0010=256, size:0=8,1=16,2=32,3=64, align:"4<<align", wback=rm!=15, reg_index:rm!=13&&rm!=15
#define Vxx1gen(L, D, Rn, Vd, type, size, align, Rm) (0b1111<<28 | 0b0100<<24 | 0<<23 | (D)<<22 | (L)<<21 | 0<<20 | (Rn)<<16 | (Vd)<<12 | (type)<<8 | (size)<<6 | (align)<<4 | (Rm))
// Load [Rn] => Dd/Dd+1/Dd+2/Dd+3. Align is 4
#define VLD1Q_32(Dd, Rn) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 2, 0, 15))
// Load [Rn]! => Dd/Dd+1/Dd+2/Dd+3. Align is 4
#define VLD1Q_32_W(Dd, Rn) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 2, 0, 13))
// Load [Rn, Rm]! => Dd/Dd+1/Dd+2/Dd+3. If Rm==15, no writeback, Rm ignored, else writeback Rn <- Rn+Rm. Align is 4
#define VLD1Q_32_REG(Dd, Rn, Rm) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 2, 0, Rm))
// Load [Rn] => Dd/Dd+1/Dd+2/Dd+3. Align is 4
#define VLD1Q_8(Dd, Rn) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 0, 0, 15))
// Load [Rn] => Dd/Dd+1/Dd+2/Dd+3. Align is 4
#define VLD1Q_16(Dd, Rn) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 1, 0, 15))
// Load [Rn] => Dd/Dd+1/Dd+2/Dd+3. Align is 4
#define VLD1Q_64(Dd, Rn) EMIT(Vxx1gen(1, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 3, 0, 15))

// Store [Rn] => Dd/Dd+1/Dd+2/Dd+3.Align is 4
#define VST1Q_32(Dd, Rn) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 2, 0, 15))
// Store [Rn]! => Dd/Dd+1/Dd+2/Dd+3.Align is 4
#define VST1Q_32_W(Dd, Rn) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 2, 0, 13))
// Store [Rn, Rm]! => Dd/Dd+1/Dd+2/Dd+3. If Rm==15, no writeback, Rm ignored, else writeback Rn <- Rn+Rm. Align is 4
#define VST1Q_32_REG(Dd, Rn, Rm) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 2, 0, Rm))
// Store [Rn] => Dd/Dd+1/Dd+2/Dd+3.Align is 4
#define VST1Q_8(Dd, Rn) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 0, 0, 15))
// Store [Rn] => Dd/Dd+1/Dd+2/Dd+3.Align is 4
#define VST1Q_16(Dd, Rn) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 1, 0, 15))
// Store [Rn] => Dd/Dd+1/Dd+2/Dd+3.Align is 4
#define VST1Q_64(Dd, Rn) EMIT(Vxx1gen(0, ((Dd)>>4)&1, Rn, ((Dd)&0x0f), 0b1010, 3, 0, 15))

#define VEOR_gen(D, Vn, Vd, N, Q, M, Vm) (0b1111<<28 | 0b0011<<24 | 0<<23 | (D)<<22 | 0b00<<20 | (Vn)<<16 | (Vd)<<12 | 0b0001<<8 | (N)<<7 | (Q)<<6 | (M)<<5 | 1<<4 | (Vm))

#define VEOR(Dd, Dn, Dm)    EMIT(VEOR_gen(((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 0, ((Dm)>>4)&1, (Dm)&15))
#define VEORQ(Dd, Dn, Dm)   EMIT(VEOR_gen(((Dd)>>4)&1, (Dn)&15, (Dd)&15, ((Dn)>>4)&1, 1, ((Dm)>>4)&1, (Dm)&15))

#endif  //__ARM_EMITTER_H__