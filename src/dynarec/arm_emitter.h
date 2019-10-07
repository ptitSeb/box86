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
    EMIT(0xe2500000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// add.s dst, src, #(imm8)

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
#define TSTS_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe1100000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// tst.s dst, src1, #imm
#define TSTS_IMM8(dst, src, imm8) \
    EMIT(0xe3100000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// orr dst, src1, src2, lsl #imm
#define ORR_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe1800000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// orr.s dst, src1, src2, lsl #imm
#define ORRS_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe1900000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// orr dst, src1, #imm8
#define ORR_IMM8(dst, src, imm8, rot) \
    EMIT(0xe3800000 | ((dst) << 12) | ((src) << 16) | ((rot)<<8) | imm8 )
// xor dst, src1, src2, lsl #imm
#define XOR_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0200000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// xor.s dst, src1, src2, lsl #imm
#define XORS_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe0300000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// xor dst, src, #(imm8)
#define XOR_IMM8(dst, src, imm8) \
    EMIT(0xe2200000 | ((dst) << 12) | ((src) << 16) | brIMM(imm8) )
// bic dst, src1, src2, lsl #imm
#define BIC_REG_LSL_IMM8(dst, src1, src2, imm8) \
    EMIT(0xe1c00000 | ((dst) << 12) | ((src1) << 16) | brLSL(imm8, src2) )
// bic dst, src, IMM8
#define BIC_IMM8(dst, src, imm8, rot) \
    EMIT(0xe3c00000 | ((dst) << 12) | ((src) << 16) | ((rot)<<8) | imm8 )
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
// ldr reg, [addr, rm lsl imm5]
#define LDR_REG_LSL_IMM5(reg, addr, rm, imm5) EMIT(0xe5900000 | ((reg) << 12) | ((addr) << 16) | (1<<25) | brLSL(imm5, rm) )

// str reg, [addr, #imm9]
#define STR_IMM9(reg, addr, imm9) EMIT(0xe5800000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// strb reg, [addr, #imm9]
#define STRB_IMM9(reg, addr, imm9) EMIT(0xe5c00000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// str reg, [addr], #imm9
#define STRAI_IMM9(reg, addr, imm9) EMIT(0xe4800000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// str reg, [addr, #-(imm9)]!
#define STR_NIMM9_W(reg, addr, imm9) EMIT(0xe5200000 | ((reg) << 12) | ((addr) << 16) | brIMM(imm9) )
// str reg, [addr, rm lsl imm5]
#define STR_REG_LSL_IMM5(reg, addr, rm, imm5) EMIT(0xe5800000 | ((reg) << 12) | ((addr) << 16) | (1<<25) | brLSL(imm5, rm) )

// bx reg
#define BX(reg) EMIT(0xe12fff10 | (reg) )

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
#define HWS_REG(Cond, P, U, W, L, Rn, Rd, S, H, Rm)     (Cond | (0b000<<25) | (P<<24) | (0<<22) | (U<<23) | (W<<21) | (L<<20) | (Rn<<16) | (Rd<<12) | (1<<7) | (S<<6) | (H<<5) | (1<<4) | Rm)
#define HWS_OFF(Cond, P, U, W, L, Rn, Rd, S, H, Imm8)   (Cond | (0b000<<25) | (P<<24) | (1<<22) | (U<<23) | (W<<21) | (L<<20) | (Rn<<16) | (Rd<<12) | ((Imm8&0xf0)<<(8-4)) | (1<<7) | (S<<6) | (H<<5) | (1<<4) | (Imm8&0x0f))

#define LDRSB_IMM8(reg, addr, imm8) EMIT(HWS_OFF(c__, 1, 1, 0, 1, addr, reg, 1, 0, imm8))
#define LDRSH_IMM8(reg, addr, imm8) EMIT(HWS_OFF(c__, 1, 1, 0, 1, addr, reg, 1, 1, imm8))
#define LDRH_IMM8(reg, addr, imm8) EMIT(HWS_OFF(c__, 1, 1, 0, 1, addr, reg, 0, 1, imm8))
#define STRH_IMM8(reg, addr, imm8) EMIT(HWS_OFF(c__, 1, 1, 0, 0, addr, reg, 0, 1, imm8))

// Mul Long construction
#define MULLONG(Cond, U, A, S, RdHi, RdLo, Rs, Rm)     (Cond | (0b00001<<23) | (U<<22) | (A<<21) | (S<<20) | (RdHi<<16) | (RdLo<<12) | (Rs<<8) | (0b1001<<4) | (Rm))

#define UMULL(RdHi, RdLo, Rs, Rm)   EMIT(MULLONG(c__, 0, 0, 0, RdHi, RdLo, Rs, Rm))
#define SMULL(RdHi, RdLo, Rs, Rm)   EMIT(MULLONG(c__, 1, 0, 0, RdHi, RdLo, Rs, Rm))

// Mul and MulA
#define MULMULA(Cond, A, S, Rd, Rn, Rs, Rm)     (Cond | (0b000000<<22) | (A<<21) | (S<<20) | (Rd<<16) | (Rn<<12) | (Rs<<8) | (0b1001<<4) | (Rm))
#define MUL(Rd, Rm, Rs)     EMIT(MULMULA(c__, 0, 0, Rd, Rd, ((Rd==Rm)?Rm:Rs), ((Rd==Rm)?Rs:Rm)))

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

#endif  //__ARM_EMITTER_H__