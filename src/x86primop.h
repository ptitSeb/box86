#ifndef __X86PRIMOP_H_
#define __X86PRIMOP_H_

typedef struct x86emu_s x86emu_t;

// Based on libx86emu

uint16_t     aaa16 (x86emu_t *emu, uint16_t d);
uint16_t     aas16 (x86emu_t *emu, uint16_t d);
uint16_t     aad16 (x86emu_t *emu, uint16_t d, uint8_t base);
uint16_t     aam16 (x86emu_t *emu, uint8_t d, uint8_t base);
uint8_t      adc8  (x86emu_t *emu, uint8_t d, uint8_t s);
uint16_t     adc16 (x86emu_t *emu, uint16_t d, uint16_t s);
uint32_t     adc32 (x86emu_t *emu, uint32_t d, uint32_t s);
/****************************************************************************
REMARKS:
Implements the ADD instruction and side effects.
****************************************************************************/
static inline uint8_t add8(x86emu_t *emu, uint8_t d, uint8_t s)
{
	emu->res = d + s;
	emu->op1 = d;
	emu->op2 = s;
	emu->df = d_add8;
	return (uint8_t)emu->res;
}

/****************************************************************************
REMARKS:
Implements the ADD instruction and side effects.
****************************************************************************/
static inline uint16_t add16(x86emu_t *emu, uint16_t d, uint16_t s)
{
	emu->res = d + s;
	emu->op1 = d;
	emu->op2 = s;
	emu->df = d_add16;
	return (uint16_t)emu->res;
}

/****************************************************************************
REMARKS:
Implements the ADD instruction and side effects.
****************************************************************************/
static inline uint32_t add32(x86emu_t *emu, uint32_t d, uint32_t s)
{
	emu->res = d + s;
	emu->op1 = d;
	emu->op2 = s;
	emu->df = d_add32;
    return emu->res;
}

/****************************************************************************
REMARKS:
Implements the AND instruction and side effects.
****************************************************************************/
static inline uint8_t and8(x86emu_t *emu, uint8_t d, uint8_t s)
{
	emu->res = d & s;
	emu->df = d_and8;

	return emu->res;
}

/****************************************************************************
REMARKS:
Implements the AND instruction and side effects.
****************************************************************************/
static inline uint16_t and16(x86emu_t *emu, uint16_t d, uint16_t s)
{
    emu->res = d & s;
	emu->df = d_and16;

    return emu->res;
}

/****************************************************************************
REMARKS:
Implements the AND instruction and side effects.
****************************************************************************/
static inline uint32_t and32(x86emu_t *emu, uint32_t d, uint32_t s)
{
	register uint32_t res;   /* all operands in native machine order */

	emu->res = d & s;
	emu->df = d_and32;

	return emu->res;
}
uint8_t      cmp8  (x86emu_t *emu, uint8_t d, uint8_t s);
uint16_t     cmp16 (x86emu_t *emu, uint16_t d, uint16_t s);
uint32_t     cmp32 (x86emu_t *emu, uint32_t d, uint32_t s);
uint8_t      daa8  (x86emu_t *emu, uint8_t d);
uint8_t      das8  (x86emu_t *emu, uint8_t d);
/****************************************************************************
REMARKS:
Implements the DEC instruction and side effects.
****************************************************************************/
static inline uint8_t dec8(x86emu_t *emu, uint8_t d)
{
    emu->res = d - 1;
	emu->op1 = d;
	emu->df = d_dec8;
	return (uint8_t)emu->res;
}

/****************************************************************************
REMARKS:
Implements the DEC instruction and side effects.
****************************************************************************/
static inline uint16_t dec16(x86emu_t *emu, uint16_t d)
{
    emu->res = d - 1;
	emu->op1 = d;
	emu->df = d_dec16;
	return (uint16_t)emu->res;

}

/****************************************************************************
REMARKS:
Implements the DEC instruction and side effects.
****************************************************************************/
static inline uint32_t dec32(x86emu_t *emu, uint32_t d)
{
    emu->res = d - 1;
	emu->op1 = d;
	emu->df = d_dec32;

	return emu->res;
}

/****************************************************************************
REMARKS:
Implements the INC instruction and side effects.
****************************************************************************/
static inline uint8_t inc8(x86emu_t *emu, uint8_t d)
{
	emu->res = d + 1;
	emu->op1 = d;
	emu->df = d_inc8;
	return (uint8_t)emu->res;
}

/****************************************************************************
REMARKS:
Implements the INC instruction and side effects.
****************************************************************************/
static inline uint16_t inc16(x86emu_t *emu, uint16_t d)
{
	emu->res = d + 1;
	emu->op1 = d;
	emu->df = d_inc16;
	return (uint16_t)emu->res;
}

/****************************************************************************
REMARKS:
Implements the INC instruction and side effects.
****************************************************************************/
static inline uint32_t inc32(x86emu_t *emu, uint32_t d)
{
	emu->res = d + 1;
	emu->op1 = d;
	emu->df = d_inc32;
	return emu->res;
}

/****************************************************************************
REMARKS:
Implements the OR instruction and side effects.
****************************************************************************/
static inline uint8_t or8(x86emu_t *emu, uint8_t d, uint8_t s)
{
	emu->res = d | s;
	emu->df = d_or8;
	return emu->res;
}

/****************************************************************************
REMARKS:
Implements the OR instruction and side effects.
****************************************************************************/
static inline uint16_t or16(x86emu_t *emu, uint16_t d, uint16_t s)
{
	emu->res = d | s;
	emu->df = d_or16;
	/* set the carry flag to be bit 8 */
	return emu->res;
}

/****************************************************************************
REMARKS:
Implements the OR instruction and side effects.
****************************************************************************/
static inline uint32_t or32(x86emu_t *emu, uint32_t d, uint32_t s)
{
	emu->res = d | s;
	emu->df = d_or32;
	return emu->res;
}

/****************************************************************************
REMARKS:
Implements the OR instruction and side effects.
****************************************************************************/
static inline uint8_t neg8(x86emu_t *emu, uint8_t s)
{
	emu->res = (uint8_t)-s;
	emu->op1 = s;
	emu->df = d_neg8;
	return emu->res;
}

/****************************************************************************
REMARKS:
Implements the OR instruction and side effects.
****************************************************************************/
static inline uint16_t neg16(x86emu_t *emu, uint16_t s)
{
	emu->res = (uint16_t)-s;
	emu->op1 = s;
	emu->df = d_neg16;
	return emu->res;
}

/****************************************************************************
REMARKS:
Implements the OR instruction and side effects.
****************************************************************************/
static inline uint32_t neg32(x86emu_t *emu, uint32_t s)
{
	emu->res = (uint32_t)-s;
	emu->op1 = s;
	emu->df = d_neg32;
	return emu->res;
}

/****************************************************************************
REMARKS:
Implements the NOT instruction and side effects.
****************************************************************************/
static inline uint8_t not8(x86emu_t *emu, uint8_t s)
{
	return ~s;
}

/****************************************************************************
REMARKS:
Implements the NOT instruction and side effects.
****************************************************************************/
static inline uint16_t not16(x86emu_t *emu, uint16_t s)
{
	return ~s;
}

/****************************************************************************
REMARKS:
Implements the NOT instruction and side effects.
****************************************************************************/
static inline uint32_t not32(x86emu_t *emu, uint32_t s)
{
	return ~s;
}

uint8_t      rcl8  (x86emu_t *emu, uint8_t d, uint8_t s);
uint16_t     rcl16 (x86emu_t *emu, uint16_t d, uint8_t s);
uint32_t     rcl32 (x86emu_t *emu, uint32_t d, uint8_t s);
uint8_t      rcr8  (x86emu_t *emu, uint8_t d, uint8_t s);
uint16_t     rcr16 (x86emu_t *emu, uint16_t d, uint8_t s);
uint32_t     rcr32 (x86emu_t *emu, uint32_t d, uint8_t s);
uint8_t      rol8  (x86emu_t *emu, uint8_t d, uint8_t s);
uint16_t     rol16 (x86emu_t *emu, uint16_t d, uint8_t s);
uint32_t     rol32 (x86emu_t *emu, uint32_t d, uint8_t s);
uint8_t      ror8  (x86emu_t *emu, uint8_t d, uint8_t s);
uint16_t     ror16 (x86emu_t *emu, uint16_t d, uint8_t s);
uint32_t     ror32 (x86emu_t *emu, uint32_t d, uint8_t s);
uint8_t      shl8  (x86emu_t *emu, uint8_t d, uint8_t s);
uint16_t     shl16 (x86emu_t *emu, uint16_t d, uint8_t s);
uint32_t     shl32 (x86emu_t *emu, uint32_t d, uint8_t s);
uint8_t      shr8  (x86emu_t *emu, uint8_t d, uint8_t s);
uint16_t     shr16 (x86emu_t *emu, uint16_t d, uint8_t s);
uint32_t     shr32 (x86emu_t *emu, uint32_t d, uint8_t s);
uint8_t      sar8  (x86emu_t *emu, uint8_t d, uint8_t s);
uint16_t     sar16 (x86emu_t *emu, uint16_t d, uint8_t s);
uint32_t     sar32 (x86emu_t *emu, uint32_t d, uint8_t s);
uint16_t     shld16 (x86emu_t *emu, uint16_t d, uint16_t fill, uint8_t s);
uint32_t     shld32 (x86emu_t *emu, uint32_t d, uint32_t fill, uint8_t s);
uint16_t     shrd16 (x86emu_t *emu, uint16_t d, uint16_t fill, uint8_t s);
uint32_t     shrd32 (x86emu_t *emu, uint32_t d, uint32_t fill, uint8_t s);
uint8_t      sbb8  (x86emu_t *emu, uint8_t d, uint8_t s);
uint16_t     sbb16 (x86emu_t *emu, uint16_t d, uint16_t s);
uint32_t     sbb32 (x86emu_t *emu, uint32_t d, uint32_t s);
/****************************************************************************
REMARKS:
Implements the SUB instruction and side effects.
****************************************************************************/
static inline uint8_t sub8(x86emu_t *emu, uint8_t d, uint8_t s)
{
	emu->res = d - s;
	emu->op1 = d;
	emu->op2 = s;
	emu->df = d_sub8;
	return (uint8_t)emu->res;
}

/****************************************************************************
REMARKS:
Implements the SUB instruction and side effects.
****************************************************************************/
static inline uint16_t sub16(x86emu_t *emu, uint16_t d, uint16_t s)
{
    emu->res = d - s;
	emu->op1 = d;
	emu->op2 = s;
	emu->df = d_sub16;
	return (uint16_t)emu->res;
}

/****************************************************************************
REMARKS:
Implements the SUB instruction and side effects.
****************************************************************************/
static inline uint32_t sub32(x86emu_t *emu, uint32_t d, uint32_t s)
{
	emu->res = d - s;
	emu->op1 = d;
	emu->op2 = s;
	emu->df = d_sub32;
	return emu->res;
}

void         test8  (x86emu_t *emu, uint8_t d, uint8_t s);
void         test16 (x86emu_t *emu, uint16_t d, uint16_t s);
void         test32 (x86emu_t *emu, uint32_t d, uint32_t s);
/****************************************************************************
REMARKS:
Implements the XOR instruction and side effects.
****************************************************************************/
static inline uint8_t xor8(x86emu_t *emu, uint8_t d, uint8_t s)
{
	emu->res = d ^ s;
	emu->df = d_xor8;
	return emu->res;
}

/****************************************************************************
REMARKS:
Implements the XOR instruction and side effects.
****************************************************************************/
static inline uint16_t xor16(x86emu_t *emu, uint16_t d, uint16_t s)
{
	emu->res = d ^ s;
	emu->df = d_xor16;
	return emu->res;
}

/****************************************************************************
REMARKS:
Implements the XOR instruction and side effects.
****************************************************************************/
static inline uint32_t xor32(x86emu_t *emu, uint32_t d, uint32_t s)
{
	emu->res = d ^ s;
	emu->df = d_xor32;
	return emu->res;
}

void         imul8  (x86emu_t *emu, uint8_t s);
void         imul16_eax (x86emu_t *emu, uint16_t s);
uint16_t     imul16 (x86emu_t *emu, uint16_t op1, uint16_t op2);
void         imul32_eax (x86emu_t *emu, uint32_t s);
uint32_t     imul32(x86emu_t *emu, uint32_t op1, uint32_t op2);
void 	     imul32_direct(uint32_t *res_lo, uint32_t* res_hi, uint32_t d, uint32_t s);
void         mul8  (x86emu_t *emu, uint8_t s);
void         mul16 (x86emu_t *emu, uint16_t s);
void         mul32_eax (x86emu_t *emu, uint32_t s);
void         idiv8  (x86emu_t *emu, uint8_t s);
void         idiv16 (x86emu_t *emu, uint16_t s);
void         idiv32 (x86emu_t *emu, uint32_t s);
void         div8  (x86emu_t *emu, uint8_t s);
void         div16 (x86emu_t *emu, uint16_t s);
void         div32 (x86emu_t *emu, uint32_t s);
//void         ins (x86emu_t *emu, int size);
//void         outs (x86emu_t *emu, int size);
/*void         push16 (x86emu_t *emu, uint16_t w);
void         push32 (x86emu_t *emu, uint32_t w);
uint16_t     pop16 (x86emu_t *emu);
uint32_t     pop32 (x86emu_t *emu);*/
int eval_condition(x86emu_t *emu, unsigned type);

#endif //__X86PRIMOP_H_