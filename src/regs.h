#ifndef __REGS_H_
#define __REGS_H_

enum SegNames { es=0,cs,ss,ds,fs,gs};

enum {
	REGI_AX, REGI_CX, REGI_DX, REGI_BX,
	REGI_SP, REGI_BP, REGI_SI, REGI_DI
};

enum {
    FLAG_CF = 0,
    FLAG_PF, FLAG_AF, FLAG_ZF, FLAG_SF, FLAG_TF, FLAG_IF, FLAG_DF, FLAG_OF,
    FLAG_IOPL1, FLAG_IOPL2, FLAG_NT, 
    FLAG_RF = 16, FLAG_VM, FLAG_AC, FLAG_VIF, 
    FLAG_VIP, FLAG_ID
};

typedef struct {
	uint16_t    val[8];
	uintptr_t   phys[8];
} x86segment_t;

typedef union {
	uint32_t dword[1];
	uint16_t word[2];
	uint8_t  byte[4];
} reg32_t;

typedef union {
    double d;
    struct {
        int32_t  upper;
        uint32_t lower;
    } l;
    struct {
        float upper;
        float lower;
    } f;
    int64_t ll;
} fpu_reg_t;

typedef struct {
    uint32_t m1;
    uint32_t m2;
    uint16_t m3;

    uint16_t d1;
    uint32_t d2;
} fpu_p_reg_t;

typedef enum {
	TAG_Valid = 0,
	TAG_Zero  = 1,
	TAG_Weird = 2,
	TAG_Empty = 3
} fpu_tag_t;

typedef enum {
	ROUND_Nearest = 0,		
	ROUND_Down    = 1,
	ROUND_Up      = 2,	
	ROUND_Chop    = 3
} fpu_round_t;

typedef union {
	uint64_t q;
	struct {
		uint32_t d0,d1;
	} ud;

	struct {
		int32_t d0,d1;
	} sd;

	struct {
		uint16_t w0,w1,w2,w3;
	} uw;

	struct {
		int16_t w0,w1,w2,w3;
	} sw;

	struct {
		uint8_t b0,b1,b2,b3,b4,b5,b6,b7;
	} ub;

	struct {
		int8_t b0,b1,b2,b3,b4,b5,b6,b7;
	} sb;
} mmx_regs_t;

typedef struct {
    // cpu
	reg32_t     regs[8],ip;
	uint32_t    eflags;
    // segments
    x86segment_t segs[6];
    // fpu
	fpu_reg_t   fpu[9];
	fpu_p_reg_t p_regs[9];
	fpu_tag_t   tags[9];
	uint16_t    cw,cw_mask_all;
	uint16_t    sw;
	uint32_t    top;
	fpu_round_t round;
    // mmx
    mmx_regs_t  mmx[8];
} x86regs_t;

#define _EIP(reg) reg.ip.dword[0]
#define _EAX(reg) reg.regs[REGI_AX].dword[0]
#define _EBX(reg) reg.regs[REGI_BX].dword[0]
#define _ECX(reg) reg.regs[REGI_CX].dword[0]
#define _EDX(reg) reg.regs[REGI_DX].dword[0]
#define _EDI(reg) reg.regs[REGI_DI].dword[0]
#define _ESI(reg) reg.regs[REGI_SI].dword[0]
#define _ESP(reg) reg.regs[REGI_SP].dword[0]
#define _EBP(reg) reg.regs[REGI_BP].dword[0]

#endif //__REGS_H_