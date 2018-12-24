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
    uint32_t    x32;
    struct {
        int F_CF:1;
        int F_PF:1;
        int F_AF:1;
        int F_ZF:1;
        int F_SF:1;
        int F_TF:1;
        int F_IF:1;
        int F_DF:1;
        int F_OF:1;
        unsigned int F_IOPL:2;
        int F_NT:1;
        int dummy:1;
        int F_RF:1;
        int F_VM:1;
        int F_AC:1;
        int F_VIF:1; 
        int F_VIP:1;
        int F_ID:1;
    } f;
} x86flags_t;

/*typedef struct {
	uint16_t    val[8];
	uintptr_t   phys[8];
} x86segment_t;*/

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
#define R_DX emu->regs[_DX].word[0]
#define R_AL emu->regs[_AX].byte[0]
#define R_AH emu->regs[_AX].byte[1]
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