#ifndef __REGS_H_
#define __REGS_H_

enum SegNames { es=0,cs,ss,ds,fs,gs};

enum {
	_AX, _CX, _DX, _BX,
	_SP, _BP, _SI, _DI
};

enum {
    _CS, _DS, _SS, _ES, _FS, _GS
};


typedef union {
	int32_t  sword[1];
	uint32_t dword[1];
	uint16_t word[2];
	uint8_t  byte[4];
} reg32_t;

typedef struct {
    uint32_t m1;
    uint32_t m2;
    uint16_t m3;

    uint16_t d1;
    uint32_t d2;
} fpu_p_reg_t;

typedef enum {
	ROUND_Nearest = 0,		
	ROUND_Down    = 1,
	ROUND_Up      = 2,	
	ROUND_Chop    = 3
} fpu_round_t;

#pragma pack(push, 1)

typedef union {
    double d;
    struct {
        uint32_t lower;
        uint32_t upper;
    } l;
    struct {
        float lower;
        float upper;
    } f;
    int64_t ll;
} fpu_reg_t;

typedef union {
	//long double ld;	// works only if 80bits!
	struct {
		uint64_t lower;
		uint16_t upper;
	} l;
} longdouble_t;

typedef struct {
	#ifdef HAVE_LD80BITS
	long double 	ld;
	#else
	longdouble_t 	ld;
	#endif
	uint64_t		ref;
} fpu_ld_t;

typedef struct {
	uint64_t		ll;
	uint64_t		ref;
} fpu_ll_t;

typedef union {
    struct __attribute__ ((__packed__)) {
        unsigned int F_CF:1;
		unsigned int res1:1;
        unsigned int F_PF:1;
		unsigned int res2:1;
        unsigned int F_AF:1;
		unsigned int res3:1;
        unsigned int F_ZF:1;
        unsigned int F_SF:1;
        unsigned int F_TF:1;
        unsigned int F_IF:1;
        unsigned int F_DF:1;
        unsigned int F_OF:1;
        unsigned int F_IOPL:2;
        unsigned int F_NT:1;
        unsigned int dummy:1;
        unsigned int F_RF:1;
        unsigned int F_VM:1;
        unsigned int F_AC:1;
        unsigned int F_VIF:1; 
        unsigned int F_VIP:1;
        unsigned int F_ID:1;
    } f;
    uint32_t    x32;
} x86flags_t;


typedef union {
    struct __attribute__ ((__packed__)) {
        unsigned int F87_IE:1;
        unsigned int F87_DE:1;
        unsigned int F87_ZE:1;
        unsigned int F87_OE:1;
        unsigned int F87_UE:1;
        unsigned int F87_PE:1;
        unsigned int F87_SF:1;
        unsigned int F87_ES:1;
        unsigned int F87_C0:1;
		unsigned int F87_C1:1;
		unsigned int F87_C2:1;
		unsigned int F87_TOP:3;
		unsigned int F87_C3:1;
		unsigned int F87_B:1;
    } f;
    uint16_t    x16;
} x87flags_t;

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

typedef union {
	uint64_t q[2];
	struct {
		uint32_t d[4];
	} ud;

	struct {
		int32_t d[4];
	} sd;

	struct {
		uint16_t w[8];
	} uw;

	struct {
		int16_t w[8];
	} sw;

	struct {
		uint8_t b[16];
	} ub;

	struct {
		int8_t b[16];
	} sb;
} sse_regs_t;
#pragma pack(pop)

#define R_EIP emu->ip.dword[0]
#define R_EAX emu->regs[_AX].dword[0]
#define R_EBX emu->regs[_BX].dword[0]
#define R_ECX emu->regs[_CX].dword[0]
#define R_EDX emu->regs[_DX].dword[0]
#define R_EDI emu->regs[_DI].dword[0]
#define R_ESI emu->regs[_SI].dword[0]
#define R_ESP emu->regs[_SP].dword[0]
#define R_EBP emu->regs[_BP].dword[0]
#define R_AX emu->regs[_AX].word[0]
#define R_BX emu->regs[_BX].word[0]
#define R_CX emu->regs[_CX].word[0]
#define R_DX emu->regs[_DX].word[0]
#define R_DI emu->regs[_DI].word[0]
#define R_SI emu->regs[_SI].word[0]
#define R_SP emu->regs[_SP].word[0]
#define R_BP emu->regs[_BP].word[0]
#define R_AL emu->regs[_AX].byte[0]
#define R_AH emu->regs[_AX].byte[1]
#define R_CX emu->regs[_CX].word[0]
#define R_CL emu->regs[_CX].byte[0]
#define R_CS emu->segs[_CS]
#define R_DS emu->segs[_DS]
#define R_SS emu->segs[_SS]
#define R_ES emu->segs[_ES]
#define R_FS emu->segs[_FS]
#define R_GS emu->segs[_GS]

#define ACCESS_FLAG(F)  emu->eflags.f.F
#define SET_FLAG(F)     emu->eflags.f.F = 1
#define CLEAR_FLAG(F)   emu->eflags.f.F = 0
#define CONDITIONAL_SET_FLAG(COND, F)   emu->eflags.f.F = (COND)?1:0

#endif //__REGS_H_