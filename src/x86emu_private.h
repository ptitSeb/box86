#ifndef __X86EMU_PRIVATE_H_
#define __X86EMU_PRIVATE_H_

#include "regs.h"

typedef struct zydis_dec_s zydis_dec_t;

typedef struct x86emu_s {
    // cpu
	reg32_t     regs[8],ip;
	x86flags_t  eflags;
    // segments
    uint32_t    segs[6];    // only 32bits value?
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
    // cpu helpers
    reg32_t     zero;
    reg32_t     *sbiidx[8];
    // emu control
    int         quit;
    int         divby0;
    // trace
    zydis_dec_t *dec;
    // global stuffs, pointed with GS: segment
    void        *globals;
} x86emu_t;

#define INTR_RAISE_DIV0(emu) {emu->divby0 = 1; emu->quit=1;}

#endif //__X86EMU_PRIVATE_H_