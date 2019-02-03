#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"
#include "x87emu_private.h"
#include "box86context.h"

int Run(x86emu_t *emu)
{
    uint8_t opcode;
    uint8_t nextop;
    reg32_t *op1, *op2, *op3, *op4;
    reg32_t ea1, ea2, ea3, ea4;
    uint8_t tmp8u, tmp8u2;
    int8_t tmp8s;
    uint16_t tmp16u, tmp16u2;
    int16_t tmp16s;
    uint32_t tmp32u, tmp32u2, tmp32u3;
    int32_t tmp32s;
    uint64_t tmp64u;
    int64_t tmp64s;
    double d;
    float f;
    int64_t ll;
    sse_regs_t *opx1, *opx2, eax1;
    mmx_regs_t *opm1, *opm2;
    //ref opcode: http://ref.x86asm.net/geek32.html#xA1
    printf_log(LOG_DEBUG, "Run X86, EIP=%p, Stack=%p\n", (void*)R_EIP, emu->context->stack);
#define F8      Fetch8(emu)
#define F8S     Fetch8s(emu)
#define F16     Fetch16(emu)
#define F32     Fetch32(emu)
#define F32S    Fetch32s(emu)
#define PK(a)   (*(uint8_t*(R_EIP+a)))

x86emurun:
    emu->quit = 0;
    static const void* baseopcodes[256] ={
    &&_0x00_0,  &&_0x00_1,  &&_0x00_2,  &&_0x00_3,  &&_0x00_4,  &&_0x00_5,  &&_default, &&_default,   //0x00-0x07
    &&_0x08_0,  &&_0x08_1,  &&_0x08_2,  &&_0x08_3,  &&_0x08_4,  &&_0x08_5,  &&_default, &&_0x0F,      //0x08-0x0F
    &&_0x10_0,  &&_0x10_1,  &&_0x10_2,  &&_0x10_3,  &&_0x10_4,  &&_0x10_5,  &&_default, &&_default,   //0x10-0x17
    &&_0x18_0,  &&_0x18_1,  &&_0x18_2,  &&_0x18_3,  &&_0x18_4,  &&_0x18_5,  &&_default, &&_default,   //0x18-0x1F
    &&_0x20_0,  &&_0x20_1,  &&_0x20_2,  &&_0x20_3,  &&_0x20_4,  &&_0x20_5,  &&_default, &&_0x27,      //0x20-0x27
    &&_0x28_0,  &&_0x28_1,  &&_0x28_2,  &&_0x28_3,  &&_0x28_4,  &&_0x28_5,  &&_default, &&_0x2F,      //0x28-0x2F
    &&_0x30_0,  &&_0x30_1,  &&_0x30_2,  &&_0x30_3,  &&_0x30_4,  &&_0x30_5,  &&_default, &&_0x37,      //0x30-0x37
    &&_0x38,    &&_0x39,    &&_0x3A,    &&_0x3B,    &&_0x3C,    &&_0x3D,    &&_default, &&_0x3F,      //0x38-0x3F
    &&_0x40,    &&_0x41,    &&_0x42,    &&_0x43,    &&_0x44,    &&_0x45,    &&_0x46,    &&_0x47, 
    &&_0x48,    &&_0x49,    &&_0x4A,    &&_0x4B,    &&_0x4C,    &&_0x4D,    &&_0x4E,    &&_0x4F,     
    &&_0x50,    &&_0x51,    &&_0x52,    &&_0x53,    &&_0x54,    &&_0x55,    &&_0x56,    &&_0x57, 
    &&_0x58,    &&_0x59,    &&_0x5A,    &&_0x5B,    &&_0x5C,    &&_0x5D,    &&_0x5E,    &&_0x5F, 
    &&_default, &&_default, &&_default, &&_default, &&_default, &&_0x65,    &&_0x66,    &&_0x67,
    &&_0x68,    &&_0x69,    &&_0x6A,    &&_0x6B,    &&_default, &&_default, &&_default, &&_default,   //0x68-0x6F
    &&_0x70_0,  &&_0x70_1,  &&_0x70_2,  &&_0x70_3,  &&_0x70_4,  &&_0x70_5,  &&_0x70_6,  &&_0x70_7,    //0x70-0x77
    &&_0x70_8,  &&_0x70_9,  &&_0x70_A,  &&_0x70_B,  &&_0x70_C,  &&_0x70_D,  &&_0x70_E,  &&_0x70_F,    //0x78-0x7F
    &&_0x80,    &&_0x81,    &&_default, &&_0x83,    &&_0x84,    &&_0x85,    &&_0x86,    &&_0x87,     
    &&_0x88,    &&_0x89,    &&_0x8A,    &&_0x8B,    &&_default, &&_0x8D,    &&_default, &&_0x8F,     
    &&_0x90,    &&_0x91,    &&_0x92,    &&_0x93,    &&_0x94,    &&_0x95,    &&_0x96,    &&_0x97, 
    &&_0x98,    &&_0x99,    &&_default, &&_0x9B,    &&_0x9C,    &&_0x9D,    &&_0x9E,    &&_default,
    &&_0xA0,    &&_0xA1,    &&_0xA2,    &&_0xA3,    &&_0xA4,    &&_0xA5,    &&_0xA6,    &&_0xA7, 
    &&_0xA8,    &&_0xA9,    &&_0xAA,    &&_0xAB,    &&_0xAC,    &&_0xAD,    &&_0xAE,    &&_0xAF,     
    &&_0xB0,    &&_0xB1,    &&_0xB2,    &&_0xB3,    &&_0xB4,    &&_0xB5,    &&_0xB6,    &&_0xB7, 
    &&_0xB8,    &&_0xB9,    &&_0xBA,    &&_0xBB,    &&_0xBC,    &&_0xBD,    &&_0xBE,    &&_0xBF, 
    &&_0xC0,    &&_0xC1,    &&_0xC2,    &&_0xC3,    &&_default, &&_default, &&_0xC6,    &&_0xC7, 
    &&_default, &&_0xC9,    &&_default, &&_default, &&_0xCC,    &&_0xCD,    &&_default, &&_default,  //0xC8-0xCF
    &&_0xD0,    &&_0xD1,    &&_0xD2,    &&_0xD3,    &&_0xD4,    &&_0xD5,    &&_default, &&_0xD7, 
    &&_0xD8,    &&_0xD9,    &&_0xDA,    &&_0xDB,    &&_0xDC,    &&_0xDD,    &&_0xDE,    &&_0xDF, 
    &&_0xE0,    &&_0xE1,    &&_0xE2,    &&_0xE3,    &&_default, &&_default, &&_default, &&_default,
    &&_0xE8,    &&_0xE9,    &&_default, &&_0xEB,    &&_default, &&_default, &&_default, &&_default,
    &&_0xF0,    &&_default, &&_0xF2,    &&_0xF3,    &&_default, &&_default, &&_0xF6,    &&_0xF7, 
    &&_0xF8,    &&_0xF9,    &&_default, &&_default, &&_0xFC,    &&_0xFD,    &&_0xFE,    &&_0xFF
    };

    static const void* opcodes0f[256] = {
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x00-0x07
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x08-0x0F
    &&_0f_0x10, &&_0f_0x11, &&_0f_0x12, &&_0f_0x13, &&_0f_0x14, &&_0f_0x15, &&_0f_0x16, &&_0f_0x17, //0x10-0x17 
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_0f_0x1F, //0x18-0x1F
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x20-0x27
    &&_0f_0x28, &&_0f_0x29, &&_0f_0x2A, &&_default, &&_0f_0x2C, &&_0f_0x2D, &&_0f_0x2E, &&_0f_0x2F, 
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x30-0x37
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x38-0x3F
    &&_0f_0x40_0, &&_0f_0x40_1, &&_0f_0x40_2, &&_0f_0x40_3, &&_0f_0x40_4, &&_0f_0x40_5, &&_0f_0x40_6, &&_0f_0x40_7,
    &&_0f_0x40_8, &&_0f_0x40_9, &&_0f_0x40_A, &&_0f_0x40_B, &&_0f_0x40_C, &&_0f_0x40_D, &&_0f_0x40_E, &&_0f_0x40_F,
    &&_0f_0x50, &&_0f_0x51, &&_default, &&_default, &&_0f_0x54, &&_0f_0x55, &&_0f_0x56, &&_0f_0x57, //0x50-0x57
    &&_0f_0x58, &&_0f_0x59, &&_0f_0x5A, &&_0f_0x5B, &&_0f_0x5C, &&_0f_0x5D, &&_0f_0x5E, &&_0f_0x5F, 
    &&_0f_0x60, &&_0f_0x61, &&_0f_0x62, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x60-0x67
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_0f_0x6F, //0x68-0x6F
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_0f_0x77, //0x70-0x77
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_0f_0x7F, //0x78-0x7F
    &&_0f_0x80_0, &&_0f_0x80_1, &&_0f_0x80_2, &&_0f_0x80_3, &&_0f_0x80_4, &&_0f_0x80_5, &&_0f_0x80_6, &&_0f_0x80_7,
    &&_0f_0x80_8, &&_0f_0x80_9, &&_0f_0x80_A, &&_0f_0x80_B, &&_0f_0x80_C, &&_0f_0x80_D, &&_0f_0x80_E, &&_0f_0x80_F,
    &&_0f_0x90_0, &&_0f_0x90_1, &&_0f_0x90_2, &&_0f_0x90_3, &&_0f_0x90_4, &&_0f_0x90_5, &&_0f_0x90_6, &&_0f_0x90_7,
    &&_0f_0x90_8, &&_0f_0x90_9, &&_0f_0x90_A, &&_0f_0x90_B, &&_0f_0x90_C, &&_0f_0x90_D, &&_0f_0x90_E, &&_0f_0x90_F,
    &&_default, &&_default, &&_0f_0xA2, &&_0f_0xA3, &&_0f_0xA4, &&_0f_0xA5, &&_default, &&_default, //0xA0-0xA7
    &&_default, &&_default, &&_default, &&_0f_0xAB, &&_0f_0xAC, &&_0f_0xAD, &&_0f_0xAE, &&_0f_0xAF, 
    &&_0f_0xB0, &&_0f_0xB1, &&_default, &&_0f_0xB3, &&_default, &&_default, &&_0f_0xB6, &&_0f_0xB7, 
    &&_default, &&_default, &&_0f_0xBA, &&_0f_0xBB, &&_0f_0xBC, &&_0f_0xBD, &&_0f_0xBE, &&_0f_0xBF, 
    &&_0f_0xC0, &&_0f_0xC1, &&_0f_0xC2, &&_default, &&_default, &&_default, &&_0f_0xC6, &&_0f_0xC7, 
    &&_0f_0xC8, &&_0f_0xC9, &&_0f_0xCA, &&_0f_0xCB, &&_0f_0xCC, &&_0f_0xCD, &&_0f_0xCE, &&_0f_0xCF, //0xC8-0xCF
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0xD0-0xD7
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0xD8-0xDF
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0xE0-0xE7
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0xE8-0xEF
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0xF0-0xF7
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default  //0xF8-0xFF
    };

    static const void* opcodes66[256] = {
    &&_default, &&_66_0x00_1, &&_default, &&_66_0x00_3, &&_default ,&&_66_0x00_5, &&_default, &&_default, //0x00-0x07
    &&_default, &&_66_0x08_1, &&_default, &&_66_0x08_3, &&_default ,&&_66_0x08_5, &&_default, &&_default, //0x08-0x0F
    &&_default, &&_66_0x10_1, &&_default, &&_66_0x10_3, &&_default ,&&_66_0x10_5, &&_default, &&_default, //0x10-0x17
    &&_default, &&_66_0x18_1, &&_default, &&_66_0x18_3, &&_default ,&&_66_0x18_5, &&_default, &&_default, //0x18-0x1F
    &&_default, &&_66_0x20_1, &&_default, &&_66_0x20_3, &&_default ,&&_66_0x20_5, &&_default, &&_default, //0x20-0x27
    &&_default, &&_66_0x28_1, &&_default, &&_66_0x28_3, &&_default ,&&_66_0x28_5, &&_66_0x2E, &&_default, //0x28-0x2F
    &&_default, &&_66_0x30_1, &&_default, &&_66_0x30_3, &&_default ,&&_66_0x30_5, &&_default, &&_default, //0x30-0x37
    &&_default, &&_66_0x39, &&_default, &&_66_0x3B, &&_default, &&_66_0x3D, &&_default, &&_default, //0x38-0x3F
    &&_66_0x40, &&_66_0x41, &&_66_0x42, &&_66_0x43, &&_66_0x44, &&_66_0x45, &&_66_0x46, &&_66_0x47, 
    &&_66_0x48, &&_66_0x49, &&_66_0x4A, &&_66_0x4B, &&_66_0x4C, &&_66_0x4D, &&_66_0x4E, &&_66_0x4F, 
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x50-0x57
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x58-0x5F
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x60-0x67
    &&_default, &&_66_0x69, &&_default, &&_66_0x6B, &&_default, &&_default, &&_default, &&_default, //0x68-0x6F
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x70-0x77
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x78-0x7F
    &&_default, &&_66_0x81, &&_default, &&_66_0x83, &&_default, &&_66_0x85, &&_default, &&_66_0x87, 
    &&_default, &&_66_0x89, &&_default, &&_66_0x8B, &&_default, &&_default, &&_default, &&_66_0x8F, 
    &&_66_0x90, &&_default, &&_66_0x92, &&_default, &&_default, &&_default, &&_default, &&_default, 
    &&_66_0x98, &&_66_0x99, &&_default, &&_default, &&_default, &&_default, &&_default, &&_default, //0x98-0x9F
    &&_default, &&_66_0xA1, &&_default, &&_66_0xA3, &&_default, &&_66_0xA5, &&_default, &&_66_0xA7, 
    &&_default, &&_default, &&_default, &&_66_0xAB, &&_default, &&_66_0xAD, &&_default, &&_66_0xAF, //0xA8-0xAF
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0xB0-0xB7
    &&_66_0xB8, &&_66_0xB9, &&_66_0xBA, &&_66_0xBB, &&_66_0xBC, &&_66_0xBD, &&_66_0xBE, &&_66_0xBF, 
    &&_default, &&_66_0xC1, &&_default, &&_default, &&_default, &&_default, &&_default, &&_66_0xC7, 
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0xC8-0xCF
    &&_default, &&_66_0xD1, &&_default, &&_66_0xD3, &&_default, &&_default, &&_default, &&_default, //0xD0-0xD8
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0xD8-0xDF
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0xE0-0xE7
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0xE8-0xEF
    &&_default, &&_default, &&_66_0xF2, &&_66_0xF3, &&_default, &&_default, &&_default, &&_66_0xF7, 
    &&_default, &&_default, &&_default, &&_default, &&_default, &&_default, &&_default, &&_66_0xFF
    };

    static const void* opcodes660f[256] = {
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x00-0x07
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x08-0x0F
    &&_6f_0x10, &&_6f_0x11, &&_default, &&_default, &&_6f_0x14, &&_6f_0x15, &&_6f_0x16, &&_6f_0x17, 
    &&_default, &&_default, &&_default, &&_default, &&_default, &&_default, &&_default, &&_6f_0x1F, 
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x20-0x27
    &&_6f_0x28, &&_6f_0x29, &&_6f_0x2A, &&_default, &&_6f_0x2C, &&_6f_0x2D, &&_6f_0x2E, &&_6f_0x2F, 
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x30-0x37
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x38-0x3F
    &&_6f_0x40_0, &&_6f_0x40_1, &&_6f_0x40_2, &&_6f_0x40_3, &&_6f_0x40_4, &&_6f_0x40_5, &&_6f_0x40_6, &&_6f_0x40_7, 
    &&_6f_0x40_8, &&_6f_0x40_9, &&_6f_0x40_A, &&_6f_0x40_B, &&_6f_0x40_C, &&_6f_0x40_D, &&_6f_0x40_E, &&_6f_0x40_F, 
    &&_6f_0x50, &&_6f_0x51, &&_default, &&_default, &&_6f_0x54, &&_6f_0x55, &&_6f_0x56, &&_6f_0x57, 
    &&_6f_0x58, &&_6f_0x59, &&_6f_0x5A, &&_6f_0x5B, &&_6f_0x5C, &&_6f_0x5D, &&_6f_0x5E, &&_6f_0x5F, 
    &&_6f_0x60, &&_6f_0x61, &&_6f_0x62, &&_default, &&_6f_0x64, &&_6f_0x65, &&_6f_0x66, &&_6f_0x67, 
    &&_6f_0x68, &&_6f_0x69, &&_6f_0x6A, &&_default, &&_6f_0x6C, &&_6f_0x6D, &&_6f_0x6E, &&_6f_0x6F,     
    &&_6f_0x70, &&_6f_0x71, &&_6f_0x72, &&_6f_0x73, &&_6f_0x74, &&_6f_0x75, &&_6f_0x76, &&_default, 
    &&_default, &&_default, &&_default, &&_default, &&_default, &&_default, &&_6f_0x7E, &&_6f_0x7F, //0x78-0x7F
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x80-0x87
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x88-0x8F
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x90-0x97
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x98-0x9F
    &&_default, &&_default, &&_default, &&_6f_0xA3, &&_6f_0xA4, &&_6f_0xA5, &&_default, &&_default, 
    &&_default, &&_default, &&_default, &&_6f_0xAB, &&_6f_0xAC, &&_6f_0xAD, &&_default, &&_6f_0xAF, 
    &&_default, &&_6f_0xB1, &&_default, &&_6f_0xB3, &&_default, &&_default, &&_6f_0xB6, &&_default, 
    &&_default, &&_default, &&_default, &&_6f_0xBB, &&_6f_0xBC, &&_6f_0xBD, &&_6f_0xBE, &&_default, //0xB8-0xBF
    &&_default, &&_6f_0xC1, &&_6f_0xC2, &&_default, &&_6f_0xC4, &&_6f_0xC5, &&_6f_0xC6, &&_default, 
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0xC8-0xCF
    &&_default, &&_6f_0xD1, &&_6f_0xD2, &&_6f_0xD3, &&_6f_0xD4, &&_default, &&_6f_0xD6, &&_default, 
    &&_default, &&_default, &&_default, &&_6f_0xDB, &&_default, &&_default, &&_default, &&_6f_0xDF, 
    &&_default, &&_6f_0xE1, &&_6f_0xE2, &&_default, &&_default, &&_default, &&_6f_0xE6, &&_default, 
    &&_default, &&_default, &&_default, &&_6f_0xEB, &&_6f_0xEC, &&_6f_0xED, &&_6f_0xEE, &&_6f_0xEF, 
    &&_default, &&_6f_0xF1, &&_6f_0xF2, &&_6f_0xF3, &&_6f_0xF4, &&_default, &&_default, &&_default, 
    &&_default, &&_default, &&_6f_0xFA, &&_6f_0xFB, &&_6f_0xFC, &&_default, &&_6f_0xFE, &&_default
    };

    while (1)
    {
#ifdef HAVE_TRACE
_trace:
        emu->old_ip = R_EIP;
        if(emu->dec && (
                (emu->trace_end == 0) 
             || ((R_EIP >= emu->trace_start) && (R_EIP < emu->trace_end))) ) {
            printf_log(LOG_NONE, "%s", DumpCPURegs(emu));
            if(PK(0)==0xcc && PK(1)=='S' && PK(2)=='C') {
                uint32_t a = *(uint32_t*)(R_EIP+3);
                if(a==0) {
                    printf_log(LOG_NONE, "0x%p: Exit x86emu\n", (void*)R_EIP);
                } else {
                    printf_log(LOG_NONE, "0x%p: Native call to %p => %s\n", (void*)R_EIP, (void*)a, GetNativeName(*(void**)(R_EIP+7)));
                }
            } else {
                printf_log(LOG_NONE, "%s", DecodeX86Trace(emu->dec, R_EIP));
                uint8_t peek = PK(0);
                if(peek==0xC3 || peek==0xC2) {
                    printf_log(LOG_NONE, " => %p", *(void**)(R_ESP));
                } else if(peek==0x55) {
                    printf_log(LOG_NONE, " => STACK_TOP: %p", *(void**)(R_ESP));
                }
                printf_log(LOG_NONE, "\n");
            }
        }
    #define NEXT    __builtin_prefetch((void*)R_EIP, 0, 2); goto _trace;
#else
    #define NEXT    emu->old_ip = R_EIP; __builtin_prefetch((void*)R_EIP, 0, 2); goto *baseopcodes[(opcode=F8)];
#endif
        opcode = F8;
        goto *baseopcodes[opcode];

            #define GO(B, OP)                      \
            _##B##_0: \
                nextop = F8;               \
                op1=GetEb(emu, nextop);             \
                op1->byte[0] = OP##8(emu, op1->byte[0], GetGb(emu, nextop)->byte[0]);  \
                NEXT;                              \
            _##B##_1: \
                nextop = F8;               \
                op1=GetEd(emu, nextop);             \
                op1->dword[0] = OP##32(emu, op1->dword[0], GetG(emu, nextop)->dword[0]); \
                NEXT;                              \
            _##B##_2: \
                nextop = F8;               \
                op2=GetEb(emu, nextop);             \
                op1=GetGb(emu, nextop);             \
                op1->byte[0] = OP##8(emu, op1->byte[0], op2->byte[0]); \
                NEXT;                              \
            _##B##_3: \
                nextop = F8;               \
                op2=GetEd(emu, nextop);     \
                op1=GetG(emu, nextop);            \
                op1->dword[0] = OP##32(emu, op1->dword[0], op2->dword[0]); \
                NEXT;                              \
            _##B##_4: \
                R_AL = OP##8(emu, R_AL, F8); \
                NEXT;                              \
            _##B##_5: \
                R_EAX = OP##32(emu, R_EAX, F32); \
                NEXT;


            GO(0x00, add)                   /* ADD 0x00 -> 0x05 */
            GO(0x08, or)                    /*  OR 0x08 -> 0x0D */
            GO(0x10, adc)                   /* ADC 0x10 -> 0x15 */
            GO(0x18, sbb)                   /* SBB 0x18 -> 0x1D */
            GO(0x20, and)                   /* AND 0x20 -> 0x25 */
            GO(0x28, sub)                   /* SUB 0x28 -> 0x2D */
            GO(0x30, xor)                   /* XOR 0x30 -> 0x35 */
            //GO(0x38, cmp)                   /* CMP 0x38 -> 0x3D */    avoid affectation

            #undef GO
            _0x38:
                nextop = F8;
                op1=GetEb(emu, nextop);
                cmp8(emu, op1->byte[0], GetGb(emu, nextop)->byte[0]);
                NEXT;
            _0x39:
                nextop = F8;
                op1=GetEd(emu, nextop);
                cmp32(emu, op1->dword[0], GetG(emu, nextop)->dword[0]);
                NEXT;
            _0x3A:
                nextop = F8;
                op2=GetEb(emu, nextop);
                op1=GetGb(emu, nextop);
                cmp8(emu, op1->byte[0], op2->byte[0]);
                NEXT;
            _0x3B:
                nextop = F8;
                op2=GetEd(emu, nextop);
                op1=GetG(emu, nextop);
                cmp32(emu, op1->dword[0], op2->dword[0]);
                NEXT;
            _0x3C:
                cmp8(emu, R_AL, F8);
                NEXT;
            _0x3D:
                cmp32(emu, R_EAX, F32);
                NEXT;

            _0x0F:                      /* More instructions */
                #include "run0f.h"
                NEXT;

            _0x27:                      /* DAA */
                R_AL = daa8(emu, R_AL);
                NEXT;
            _0x2F:                      /* DAS */
                R_AL = das8(emu, R_AL);
                NEXT;
            _0x37:                      /* AAA */
                R_AX = aaa16(emu, R_AX);
                NEXT;
            _0x3F:                      /* AAS */
                R_AX = aas16(emu, R_AX);
                NEXT;


            _0x40:
            _0x41:
            _0x42:
            _0x43:
            _0x44:
            _0x45:
            _0x46:
            _0x47:                      /* INC Reg */
                tmp8u = opcode&7;
                emu->regs[tmp8u].dword[0] = inc32(emu, emu->regs[tmp8u].dword[0]);
                NEXT;
            _0x48:
            _0x49:
            _0x4A:
            _0x4B:
            _0x4C:
            _0x4D:
            _0x4E:
            _0x4F:                      /* DEC Reg */
                tmp8u = opcode&7;
                emu->regs[tmp8u].dword[0] = dec32(emu, emu->regs[tmp8u].dword[0]);
                NEXT;
            _0x54:                      /* PUSH ESP */
                tmp32u = R_ESP;
                Push(emu, tmp32u);
                NEXT;
            _0x50:
            _0x51:
            _0x52:
            _0x53:
            _0x55:
            _0x56:
            _0x57:                      /* PUSH Reg */
                tmp8u = opcode&7;
                Push(emu, emu->regs[tmp8u].dword[0]);
                NEXT;
            _0x5C:                      /* POP ESP */
                R_ESP += 4;
                NEXT;
            _0x58:
            _0x59:
            _0x5A:
            _0x5B:
            _0x5D:
            _0x5E:
            _0x5F:                      /* POP Reg */
                tmp8u = opcode&7;
                emu->regs[tmp8u].dword[0] = Pop(emu);
                NEXT;

            _0x65:                      /* GS: */
                // TODO: set a new decoder function?
                opcode = F8;
                switch(opcode) {
                    case 0x33:              /* XOR Gd,Ed */
                        nextop = F8;
                        op2=GetEd(emu, nextop);
                        op2 = (reg32_t*)(((char*)op2) + (uintptr_t)emu->globals);
                        op1 = GetG(emu, nextop);
                        op1->dword[0] = xor32(emu, op1->dword[0], op2->dword[0]);
                        break;
                    case 0x8B:              /* MOV Gd,Ed */
                        nextop = F8;
                        op2=GetEd(emu, nextop);
                        op2 = (reg32_t*)(((char*)op2) + (uintptr_t)emu->globals);
                        op1=GetG(emu, nextop);
                        op1->dword[0] = op2->dword[0];
                        break;
                    case 0xA1:              /* MOV EAX,Ov */
                        tmp32u = F32;
                        R_EAX = *(uint32_t*)(((uintptr_t)emu->globals) + tmp32u);
                        break;

                    default:
                        UnimpOpcode(emu);
                        goto fini;
                }
                NEXT;
            _0x66:                      /* Prefix to change width of intructions, so here, down to 16bits */
                nextop = F8;
                if(nextop==0x0F) {
                    #include "run660f.h"
                } else if(nextop==0xD9)
                    Run66D9(emu);
                else if(nextop==0xDD)
                    Run66DD(emu);
                else if(nextop==0x66) {
                    goto _0x66; // 0x66 0x66 => can remove one 0x66
                } else {
                    #include "run66.h"
                }
                if(emu->quit) goto fini;
                NEXT;
            _0x67:                      /* Prefix to change width of intructions */
                Run67(emu); // implemented in Run66.c
                if(emu->quit) goto fini;
                NEXT;

            _0x68:                      /* Push Id */
                Push(emu, F32);
                NEXT;
            _0x69:                      /* IMUL Gd,Ed,Id */
                nextop = F8;
                op1=GetEd(emu, nextop);
                tmp32u = F32;
                GetG(emu, nextop)->dword[0] = imul32(emu, op1->dword[0], tmp32u);
                NEXT;
            _0x6A:                      /* Push Ib */
                tmp32s = F8S;
                Push(emu, (uint32_t)tmp32s);
                NEXT;
            _0x6B:                      /* IMUL Gd,Ed,Ib */
                nextop = F8;
                op1=GetEd(emu, nextop);
                tmp32s = F8S;
                GetG(emu, nextop)->dword[0] = imul32(emu, op1->dword[0], (uint32_t)tmp32s);
                NEXT;

            #define GOCOND(BASE, PREFIX, CONDITIONAL) \
            _##BASE##_0:                            \
                PREFIX                              \
                if(ACCESS_FLAG(F_OF))               \
                    CONDITIONAL                     \
                NEXT;                              \
            _##BASE##_1:                            \
                PREFIX                              \
                if(!ACCESS_FLAG(F_OF))              \
                    CONDITIONAL                     \
                NEXT;                              \
            _##BASE##_2:                            \
                PREFIX                              \
                if(ACCESS_FLAG(F_CF))               \
                    CONDITIONAL                     \
                NEXT;                              \
            _##BASE##_3:                            \
                PREFIX                              \
                if(!ACCESS_FLAG(F_CF))              \
                    CONDITIONAL                     \
                NEXT;                              \
            _##BASE##_4:                            \
                PREFIX                              \
                if(ACCESS_FLAG(F_ZF))               \
                    CONDITIONAL                     \
                NEXT;                              \
            _##BASE##_5:                            \
                PREFIX                              \
                if(!ACCESS_FLAG(F_ZF))              \
                    CONDITIONAL                     \
                NEXT;                              \
            _##BASE##_6:                            \
                PREFIX                              \
                if((ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF)))  \
                    CONDITIONAL                     \
                NEXT;                              \
            _##BASE##_7:                            \
                PREFIX                              \
                if(!(ACCESS_FLAG(F_ZF) || ACCESS_FLAG(F_CF))) \
                    CONDITIONAL                     \
                NEXT;                              \
            _##BASE##_8:                            \
                PREFIX                              \
                if(ACCESS_FLAG(F_SF))               \
                    CONDITIONAL                     \
                NEXT;                              \
            _##BASE##_9:                            \
                PREFIX                              \
                if(!ACCESS_FLAG(F_SF))              \
                    CONDITIONAL                     \
                NEXT;                              \
            _##BASE##_A:                            \
                PREFIX                              \
                if(ACCESS_FLAG(F_PF))               \
                    CONDITIONAL                     \
                NEXT;                              \
            _##BASE##_B:                            \
                PREFIX                              \
                if(!ACCESS_FLAG(F_PF))              \
                    CONDITIONAL                     \
                NEXT;                              \
            _##BASE##_C:                            \
                PREFIX                              \
                if(ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))  \
                    CONDITIONAL                     \
                NEXT;                              \
            _##BASE##_D:                            \
                PREFIX                              \
                if(ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF)) \
                    CONDITIONAL                     \
                NEXT;                              \
            _##BASE##_E:                            \
                PREFIX                              \
                if(ACCESS_FLAG(F_ZF) || (ACCESS_FLAG(F_SF) != ACCESS_FLAG(F_OF))) \
                    CONDITIONAL                     \
                NEXT;                              \
            _##BASE##_F:                            \
                PREFIX                              \
                if(!ACCESS_FLAG(F_ZF) && (ACCESS_FLAG(F_SF) == ACCESS_FLAG(F_OF))) \
                    CONDITIONAL                     \
                NEXT;
            GOCOND(0x70
                ,   tmp8s = F8S; CHECK_FLAGS(emu);
                ,   R_EIP += (int8_t)tmp8s;
                )                           /* Jxx Ib */
            #undef GOCOND

            
            _0x80:                      /* GRP Eb,Ib */
                nextop = F8;
                op1=GetEb(emu, nextop);
                tmp8u = F8;
                switch((nextop>>3)&7) {
                    case 0: op1->byte[0] = add8(emu, op1->byte[0], tmp8u); break;
                    case 1: op1->byte[0] =  or8(emu, op1->byte[0], tmp8u); break;
                    case 2: op1->byte[0] = adc8(emu, op1->byte[0], tmp8u); break;
                    case 3: op1->byte[0] = sbb8(emu, op1->byte[0], tmp8u); break;
                    case 4: op1->byte[0] = and8(emu, op1->byte[0], tmp8u); break;
                    case 5: op1->byte[0] = sub8(emu, op1->byte[0], tmp8u); break;
                    case 6: op1->byte[0] = xor8(emu, op1->byte[0], tmp8u); break;
                    case 7:                cmp8(emu, op1->byte[0], tmp8u); break;
                }
                NEXT;
            _0x81:                      /* GRP Ed,Id */
            _0x83:                      /* GRP Ed,Ib */
                nextop = F8;
                op1=GetEd(emu, nextop);
                if(opcode==0x81) 
                    tmp32u = F32;
                else {
                    tmp32s = F8S;
                    tmp32u = *(uint32_t*)&tmp32s;
                }
                switch((nextop>>3)&7) {
                    case 0: op1->dword[0] = add32(emu, op1->dword[0], tmp32u); break;
                    case 1: op1->dword[0] =  or32(emu, op1->dword[0], tmp32u); break;
                    case 2: op1->dword[0] = adc32(emu, op1->dword[0], tmp32u); break;
                    case 3: op1->dword[0] = sbb32(emu, op1->dword[0], tmp32u); break;
                    case 4: op1->dword[0] = and32(emu, op1->dword[0], tmp32u); break;
                    case 5: op1->dword[0] = sub32(emu, op1->dword[0], tmp32u); break;
                    case 6: op1->dword[0] = xor32(emu, op1->dword[0], tmp32u); break;
                    case 7:                 cmp32(emu, op1->dword[0], tmp32u); break;
                }
                NEXT;
            _0x84:                      /* TEST Eb,Gb */
                nextop = F8;
                op1=GetEb(emu, nextop);
                test8(emu, op1->byte[0], GetGb(emu, nextop)->byte[0]);
                NEXT;
            _0x85:                      /* TEST Ed,Gd */
                nextop = F8;
                op1=GetEd(emu, nextop);
                test32(emu, op1->dword[0], GetG(emu, nextop)->dword[0]);
                NEXT;
            _0x86:                      /* XCHG Eb,Gb */
                nextop = F8;
                op1=GetEb(emu, nextop);
                op2=GetGb(emu, nextop);
                tmp8u = op1->byte[0];
                op1->byte[0] = op2->byte[0];
                op2->byte[0] = tmp8u;
                NEXT;
            _0x87:                      /* XCHG Ed,Gd */
                nextop = F8;
                op1=GetEd(emu, nextop);
                op2=GetG(emu, nextop);
                tmp32u = op1->dword[0];
                op1->dword[0] = op2->dword[0];
                op2->dword[0] = tmp32u;
                NEXT;
            _0x88:                      /* MOV Eb,Gb */
                nextop = F8;
                op1=GetEb(emu, nextop);
                op2=GetGb(emu, nextop);
                op1->byte[0] = op2->byte[0];
                NEXT;
            _0x89:                      /* MOV Ed,Gd */
                nextop = F8;
                op1=GetEd(emu, nextop);
                op2=GetG(emu, nextop);
                op1->dword[0] = op2->dword[0];
                NEXT;
            _0x8A:                      /* MOV Gb,Eb */
                nextop = F8;
                op2=GetEb(emu, nextop);
                op1=GetGb(emu, nextop);
                op1->byte[0] = op2->byte[0];
                NEXT;
            _0x8B:                      /* MOV Gd,Ed */
                nextop = F8;
                op2=GetEd(emu, nextop);
                op1=GetG(emu, nextop);
                op1->dword[0] = op2->dword[0];
                NEXT;

            _0x8D:                      /* LEA Gd,M */
                nextop = F8;
                op1=GetEd(emu, nextop);
                op2=GetG(emu, nextop);
                op2->dword[0] = (uint32_t)op1;
                NEXT;

            _0x8F:                      /* POP Ed */
                nextop = F8;
                op1=GetEd(emu, nextop);
                op1->dword[0] = Pop(emu);
                NEXT;
            _0x90:                      /* NOP */
                NEXT;
            _0x91:
            _0x92:
            _0x93:
            _0x94:
            _0x95:
            _0x96:
            _0x97:                      /* XCHG reg,EAX */
                tmp32u = R_EAX;
                R_EAX = emu->regs[opcode&7].dword[0];
                emu->regs[opcode&7].dword[0] = tmp32u;
                NEXT;

            _0x98:                      /* CWDE */
                *(int32_t*)&R_EAX = (int16_t)R_AX;
                NEXT;
            _0x99:                      /* CDQ */
                if(R_EAX & 0x80000000)
                    R_EDX=0xFFFFFFFF;
                else
                    R_EDX=0x00000000;
                NEXT;

            _0x9B:                      /* FWAIT */
                NEXT;
            _0x9C:                      /* PUSHF */
                CHECK_FLAGS(emu);
                PackFlags(emu);
                Push(emu, emu->packed_eflags.x32);
                NEXT;
            _0x9D:                      /* POPF */
                emu->packed_eflags.x32 = ((Pop(emu) & 0x3F7FD7) & (0xffff-40) ) | 0x2; // mask off res2 and res3 and on res1
                UnpackFlags(emu);
                RESET_FLAGS(emu);
                NEXT;
            _0x9E:                      /* SAHF */
                CHECK_FLAGS(emu);
                PackFlags(emu);
                emu->regs[_AX].byte[1] = emu->packed_eflags.x32&0xff;
                NEXT;

            _0xA0:                      /* MOV AL,Ob */
                R_AL = *(uint8_t*)F32;
                NEXT;
            _0xA1:                      /* MOV EAX,Od */
                R_EAX = *(uint32_t*)F32;
                NEXT;
            _0xA2:                      /* MOV Ob,AL */
                *(uint8_t*)F32 = R_AL;
                NEXT;
            _0xA3:                      /* MOV Od,EAX */
                *(uint32_t*)F32 = R_EAX;
                NEXT;
            _0xA4:                      /* MOVSB */
                tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
                *(uint8_t*)R_EDI = *(uint8_t*)R_ESI;
                R_EDI += tmp8s;
                R_ESI += tmp8s;
                NEXT;
            _0xA5:                      /* MOVSD */
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
                *(uint32_t*)R_EDI = *(uint32_t*)R_ESI;
                R_EDI += tmp8s;
                R_ESI += tmp8s;
                NEXT;
            _0xA6:                      /* CMPSB */
                tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
                tmp8u  = *(uint8_t*)R_EDI;
                tmp8u2 = *(uint8_t*)R_ESI;
                R_EDI += tmp8s;
                R_ESI += tmp8s;
                cmp8(emu, tmp8u2, tmp8u);
                NEXT;
            _0xA7:                      /* CMPSD */
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
                tmp32u  = *(uint32_t*)R_EDI;
                tmp32u2 = *(uint32_t*)R_ESI;
                R_EDI += tmp8s;
                R_ESI += tmp8s;
                cmp32(emu, tmp32u2, tmp32u);
                NEXT;
            _0xA8:                      /* TEST AL, Ib */
                test8(emu, R_AL, F8);
                NEXT;
            _0xA9:                      /* TEST EAX, Id */
                test32(emu, R_EAX, F32);
                NEXT;
            _0xAA:                      /* STOSB */
                tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
                *(uint8_t*)R_EDI = R_AL;
                R_EDI += tmp8s;
                NEXT;
            _0xAB:                      /* STOSD */
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
                *(uint32_t*)R_EDI = R_EAX;
                R_EDI += tmp8s;
                NEXT;
            _0xAC:                      /* LODSB */
                tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
                R_AL = *(uint8_t*)R_ESI;
                R_ESI += tmp8s;
                NEXT;
            _0xAD:                      /* LODSD */
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
                R_EAX = *(uint32_t*)R_ESI;
                R_ESI += tmp8s;
                NEXT;
            _0xAE:                      /* SCASB */
                tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
                cmp8(emu, R_AL, *(uint8_t*)R_EDI);
                R_EDI += tmp8s;
                NEXT;
            _0xAF:                      /* SCASD */
                tmp8s = ACCESS_FLAG(F_DF)?-4:+4;
                cmp32(emu, R_EAX, *(uint32_t*)R_EDI);
                R_EDI += tmp8s;
                NEXT;
            
            _0xB0:                      /* MOV AL,Ib */
            _0xB1:                      /* MOV CL,Ib */
            _0xB2:                      /* MOV DL,Ib */
            _0xB3:                      /* MOV BL,Ib */
            _0xB4:                      /* MOV AH,Ib */
            _0xB5:                      /*    ...    */
            _0xB6:
            _0xB7:
                tmp8u = opcode&0x07;
                emu->regs[tmp8u&3].byte[tmp8u>>2] = F8;
                NEXT;
            _0xB8:                      /* MOV EAX,Id */
            _0xB9:                      /* MOV ECX,Id */
            _0xBA:                      /* MOV EDX,Id */
            _0xBB:                      /* MOV EBX,Id */
            _0xBC:                      /*    ...     */
            _0xBD:
            _0xBE:
            _0xBF:
                emu->regs[opcode-0xB8].dword[0] = F32;
                NEXT;

            _0xC0:                      /* GRP2 Eb,Ib */
                nextop = F8;
                op1=GetEb(emu, nextop);
                tmp8u = F8/* & 0x1f*/; // masking done in each functions
                switch((nextop>>3)&7) {
                    case 0: op1->byte[0] = rol8(emu, op1->byte[0], tmp8u); break;
                    case 1: op1->byte[0] = ror8(emu, op1->byte[0], tmp8u); break;
                    case 2: op1->byte[0] = rcl8(emu, op1->byte[0], tmp8u); break;
                    case 3: op1->byte[0] = rcr8(emu, op1->byte[0], tmp8u); break;
                    case 4:
                    case 6: op1->byte[0] = shl8(emu, op1->byte[0], tmp8u); break;
                    case 5: op1->byte[0] = shr8(emu, op1->byte[0], tmp8u); break;
                    case 7: op1->byte[0] = sar8(emu, op1->byte[0], tmp8u); break;
                }
                NEXT;
            _0xC1:                      /* GRP2 Ed,Ib */
                nextop = F8;
                op1=GetEd(emu, nextop);
                tmp8u = F8/* & 0x1f*/; // masking done in each functions
                switch((nextop>>3)&7) {
                    case 0: op1->dword[0] = rol32(emu, op1->dword[0], tmp8u); break;
                    case 1: op1->dword[0] = ror32(emu, op1->dword[0], tmp8u); break;
                    case 2: op1->dword[0] = rcl32(emu, op1->dword[0], tmp8u); break;
                    case 3: op1->dword[0] = rcr32(emu, op1->dword[0], tmp8u); break;
                    case 4:
                    case 6: op1->dword[0] = shl32(emu, op1->dword[0], tmp8u); break;
                    case 5: op1->dword[0] = shr32(emu, op1->dword[0], tmp8u); break;
                    case 7: op1->dword[0] = sar32(emu, op1->dword[0], tmp8u); break;
                }
                NEXT;
            _0xC2:                      /* RETN Iw */
                tmp16u = F16;
                R_EIP = Pop(emu);
                R_ESP += tmp16u;
                NEXT;
            _0xC3:                      /* RET */
                R_EIP = Pop(emu);
                NEXT;

            _0xC6:                      /* MOV Eb,Ib */
                nextop = F8;
                op1=GetEb(emu, nextop);
                op1->byte[0] = F8;
                NEXT;
            _0xC7:                      /* MOV Ed,Id */
                nextop = F8;
                op1=GetEd(emu, nextop);
                op1->dword[0] = F32;
                NEXT;

            _0xC9:                      /* LEAVE */
                R_ESP = R_EBP;
                R_EBP = Pop(emu);
                NEXT;

            _0xCC:                      /* INT 3 */
                x86Int3(emu);
                if(emu->quit) goto fini;
                NEXT;
            _0xCD:                      /* INT Ib */
                nextop = F8;
                if(nextop == 0x80) {
                    x86Syscall(emu);
                    if(emu->quit) goto fini;
                } else {
                    printf_log(LOG_NONE, "Unsupported Int %02Xh\n", nextop);
                    emu->quit = 1;
                    emu->error |= ERR_UNIMPL;
                    goto fini;
                }
                NEXT;

            _0xD1:                      /* GRP2 Ed,1 */
            _0xD3:                      /* GRP2 Ed,CL */
                nextop = F8;
                op1=GetEd(emu, nextop);
                tmp8u = (opcode==0xD1)?1:R_CL;
                switch((nextop>>3)&7) {
                    case 0: op1->dword[0] = rol32(emu, op1->dword[0], tmp8u); break;
                    case 1: op1->dword[0] = ror32(emu, op1->dword[0], tmp8u); break;
                    case 2: op1->dword[0] = rcl32(emu, op1->dword[0], tmp8u); break;
                    case 3: op1->dword[0] = rcr32(emu, op1->dword[0], tmp8u); break;
                    case 4: 
                    case 6: op1->dword[0] = shl32(emu, op1->dword[0], tmp8u); break;
                    case 5: op1->dword[0] = shr32(emu, op1->dword[0], tmp8u); break;
                    case 7: op1->dword[0] = sar32(emu, op1->dword[0], tmp8u); break;
                }
                NEXT;
            _0xD0:                      /* GRP2 Eb,1 */
            _0xD2:                      /* GRP2 Eb,CL */
                nextop = F8;
                op1=GetEb(emu, nextop);
                tmp8u = (opcode==0xD0)?1:R_CL;
                switch((nextop>>3)&7) {
                    case 0: op1->byte[0] = rol8(emu, op1->byte[0], tmp8u); break;
                    case 1: op1->byte[0] = ror8(emu, op1->byte[0], tmp8u); break;
                    case 2: op1->byte[0] = rcl8(emu, op1->byte[0], tmp8u); break;
                    case 3: op1->byte[0] = rcr8(emu, op1->byte[0], tmp8u); break;
                    case 4: 
                    case 6: op1->byte[0] = shl8(emu, op1->byte[0], tmp8u); break;
                    case 5: op1->byte[0] = shr8(emu, op1->byte[0], tmp8u); break;
                    case 7: op1->byte[0] = sar8(emu, op1->byte[0], tmp8u); break;
                }
                NEXT;
            _0xD4:                      /* AAM Ib */
                nextop = F8;
                R_AX = aam16(emu, R_AL, nextop);
                NEXT;
            _0xD5:                      /* AAD Ib */
                nextop = F8;
                R_AX = aad16(emu, R_AX, nextop);
                NEXT;
            
            _0xD7:                      /* XLAT */
                R_AL = *(uint8_t*)(R_EBX + R_AL);
                NEXT;
            
            _0xD8:                      /* x87 */
                #include "rund8.h"
                NEXT;
            _0xD9:                      /* x87 */
                #include "rund9.h"
                NEXT;
            _0xDA:                      /* x87 */
                #include "runda.h"
                NEXT;
            _0xDB:                      /* x87 */
                #include "rundb.h"
                NEXT;
            _0xDC:                      /* x87 */
                #include "rundc.h"
                NEXT;
            _0xDD:                      /* x87 */
                #include "rundd.h"
                NEXT;
            _0xDE:                      /* x87 */
                #include "runde.h"
                NEXT;
            _0xDF:                      /* x87 */
                #include "rundf.h"
                NEXT;

            _0xE0:                      /* LOOPNZ */
                CHECK_FLAGS(emu);
                tmp8s = F8S;
                --R_ECX; // don't update flags
                if(R_ECX && !ACCESS_FLAG(F_ZF))
                    R_EIP += tmp8s;
                NEXT;
            _0xE1:                      /* LOOPZ */
                CHECK_FLAGS(emu);
                tmp8s = F8S;
                --R_ECX; // don't update flags
                if(R_ECX && ACCESS_FLAG(F_ZF))
                    R_EIP += tmp8s;
                NEXT;
            _0xE2:                      /* LOOP */
                tmp8s = F8S;
                --R_ECX; // don't update flags
                if(R_ECX)
                    R_EIP += tmp8s;
                NEXT;
            _0xE3:                      /* JECXZ */
                tmp8s = F8S;
                if(!R_ECX)
                    R_EIP += tmp8s;
                NEXT;

            _0xE8:                      /* CALL Id */
                tmp32s = F32S; // call is relative
                Push(emu, R_EIP);
                R_EIP += tmp32s;
                NEXT;
            _0xE9:                      /* JMP Id */
                tmp32s = F32S; // jmp is relative
                R_EIP += tmp32s;
                NEXT;

            _0xEB:                      /* JMP Ib */
                tmp32s = F8S; // jump is relative
                R_EIP += tmp32s;
                NEXT;

            _0xF0:                      /* LOCK */
                NEXT;

            _0xF2:                      /* REPNZ prefix */
            _0xF3:                      /* REPZ prefix */
                nextop = F8;
                if(nextop==0x0F) {
                    if(opcode==0xF3) {
                        #include "runf30f.h"
                    } else {
                        #include "runf20f.h"
                    }
                    NEXT;
                } else {
                    tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
                    tmp32u = R_ECX;
                    switch(nextop) {
                        case 0xC3:              /* REPZ RET... yup */
                            R_EIP = Pop(emu);
                            break;
                        case 0xA4:              /* REP MOVSB */
                            while(tmp32u) {
                                --tmp32u;
                                *(uint8_t*)R_EDI = *(uint8_t*)R_ESI;
                                R_EDI += tmp8s;
                                R_ESI += tmp8s;
                            }
                            break;
                        case 0xA5:              /* REP MOVSD */
                            tmp8s *= 4;
                            while(tmp32u) {
                                --tmp32u;
                                *(uint32_t*)R_EDI = *(uint32_t*)R_ESI;
                                R_EDI += tmp8s;
                                R_ESI += tmp8s;
                            }
                            break;
                        case 0xA6:              /* REP(N)Z CMPSB */
                            if(opcode==0xF2) {
                                while(tmp32u) {
                                    --tmp32u;
                                    tmp8u  = *(uint8_t*)R_EDI;
                                    tmp8u2 = *(uint8_t*)R_ESI;
                                    R_EDI += tmp8s;
                                    R_ESI += tmp8s;
                                    if(tmp8u==tmp8u2)
                                        break;
                                }
                            } else {
                                while(tmp32u) {
                                    --tmp32u;
                                    tmp8u  = *(uint8_t*)R_EDI;
                                    tmp8u2 = *(uint8_t*)R_ESI;
                                    R_EDI += tmp8s;
                                    R_ESI += tmp8s;
                                    if(tmp8u!=tmp8u2)
                                        break;
                                }
                            }
                            cmp8(emu, tmp8u2, tmp8u);
                            break;
                        case 0xA7:              /* REP(N)Z CMPSD */
                            tmp8s *= 4;
                            if(opcode==0xF2) {
                                while(tmp32u) {
                                    --tmp32u;
                                    tmp32u3 = *(uint32_t*)R_EDI;
                                    tmp32u2 = *(uint32_t*)R_ESI;
                                    R_EDI += tmp8s;
                                    R_ESI += tmp8s;
                                    if(tmp32u3==tmp32u2)
                                        break;
                                }
                            } else {
                                while(tmp32u) {
                                    --tmp32u;
                                    tmp32u3 = *(uint32_t*)R_EDI;
                                    tmp32u2 = *(uint32_t*)R_ESI;
                                    R_EDI += tmp8s;
                                    R_ESI += tmp8s;
                                    if(tmp32u3!=tmp32u2)
                                        break;
                                }
                            }
                            cmp32(emu, tmp32u2, tmp32u3);
                            break;
                        case 0xAA:              /* REP STOSB */
                            while(tmp32u) {
                                --tmp32u;
                                *(uint8_t*)R_EDI = R_AL;
                                R_EDI += tmp8s;
                            }
                            break;
                        case 0xAB:              /* REP STOSD */
                            tmp8s *= 4;
                            while(tmp32u) {
                                --tmp32u;
                                *(uint32_t*)R_EDI = R_EAX;
                                R_EDI += tmp8s;
                            }
                            break;
                        case 0xAC:              /* REP LODSB */
                            while(tmp32u) {
                                --tmp32u;
                                R_AL = *(uint8_t*)R_ESI;
                                R_ESI += tmp8s;
                            }
                            break;
                        case 0xAD:              /* REP LODSD */
                            tmp8s *= 4;
                            while(tmp32u) {
                                --tmp32u;
                                R_EAX = *(uint32_t*)R_ESI;
                                R_ESI += tmp8s;
                            }
                            break;
                        case 0xAE:              /* REP(N)Z SCASB */
                            if(opcode==0xF2) {
                                while(tmp32u) {
                                    --tmp32u;
                                    tmp8u = *(uint8_t*)R_EDI;
                                    R_EDI += tmp8s;
                                    if(R_AL==tmp8u)
                                        break;
                                }
                            } else {
                                while(tmp32u) {
                                    --tmp32u;
                                    tmp8u = *(uint8_t*)R_EDI;
                                    R_EDI += tmp8s;
                                    if(R_AL!=tmp8u)
                                        break;
                                }
                            }
                            cmp8(emu, R_AL, tmp8u);
                            break;
                        case 0xAF:              /* REP(N)Z SCASD */
                            tmp8s *= 4;
                            if(opcode==0xF2) {
                                while(tmp32u) {
                                    --tmp32u;
                                    tmp32u2 = *(uint32_t*)R_EDI;
                                    R_EDI += tmp8s;
                                    if(R_EAX==tmp32u2)
                                        break;
                                }
                            } else {
                                while(tmp32u) {
                                    --tmp32u;
                                    tmp32u2 = *(uint32_t*)R_EDI;
                                    R_EDI += tmp8s;
                                    if(R_EAX!=tmp32u2)
                                        break;
                                }
                            }
                            cmp32(emu, R_EAX, tmp32u2);
                            break;
                        default:
                            UnimpOpcode(emu);
                            goto fini;
                    }
                    R_ECX = tmp32u;
                }   // else(nextop==0x0F)
                NEXT;
            
            _0xF6:                      /* GRP3 Eb(,Ib) */
                nextop = F8;
                op1=GetEb(emu, nextop);
                switch((nextop>>3)&7) {
                    case 0: 
                    case 1:                 /* TEST Eb,Ib */
                        test8(emu, op1->byte[0], F8);
                        break;
                    case 2:                 /* NOT Eb */
                        op1->byte[0] = not8(emu, op1->byte[0]);
                        break;
                    case 3:                 /* NEG Eb */
                        op1->byte[0] = neg8(emu, op1->byte[0]);
                        break;
                    case 4:                 /* MUL EAX,Eb */
                        mul8(emu, op1->byte[0]);
                        break;
                    case 5:                 /* IMUL EAX,Eb */
                        imul8(emu, op1->byte[0]);
                        break;
                    case 6:                 /* DIV Eb */
                        div8(emu, op1->byte[0]);
                        break;
                    case 7:                 /* IDIV Eb */
                        idiv8(emu, op1->byte[0]);
                        break;
                }
                NEXT;
            _0xF7:                      /* GRP3 Ed(,Id) */
                nextop = F8;
                op1=GetEd(emu, nextop);
                switch((nextop>>3)&7) {
                    case 0: 
                    case 1:                 /* TEST Ed,Id */
                        test32(emu, op1->dword[0], F32);
                        break;
                    case 2:                 /* NOT Ed */
                        op1->dword[0] = not32(emu, op1->dword[0]);
                        break;
                    case 3:                 /* NEG Ed */
                        op1->dword[0] = neg32(emu, op1->dword[0]);
                        break;
                    case 4:                 /* MUL EAX,Ed */
                        mul32_eax(emu, op1->dword[0]);
                        break;
                    case 5:                 /* IMUL EAX,Ed */
                        imul32_eax(emu, op1->dword[0]);
                        break;
                    case 6:                 /* DIV Ed */
                        div32(emu, op1->dword[0]);
                        break;
                    case 7:                 /* IDIV Ed */
                        idiv32(emu, op1->dword[0]);
                        break;
                }
                NEXT;

            _0xF8:                      /* CLC */
                CHECK_FLAGS(emu);
                CLEAR_FLAG(F_CF);
                NEXT;
            _0xF9:                      /* STC */
                CHECK_FLAGS(emu);
                SET_FLAG(F_CF);
                NEXT;

            _0xFC:                      /* CLD */
                CLEAR_FLAG(F_DF);
                NEXT;
            _0xFD:                      /* STD */
                SET_FLAG(F_DF);
                NEXT;
            _0xFE:                      /* GRP 5 Eb */
                nextop = F8;
                op1=GetEb(emu, nextop);
                switch((nextop>>3)&7) {
                    case 0:                 /* INC Eb */
                        op1->byte[0] = inc8(emu, op1->byte[0]);
                        break;
                    case 1:                 /* DEC Ed */
                        op1->byte[0] = dec8(emu, op1->byte[0]);
                        break;
                    default:
                        printf_log(LOG_NONE, "Illegal Opcode %02X %02X\n", opcode, nextop);
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                        goto fini;
                }
                NEXT;
            _0xFF:                      /* GRP 5 Ed */
                nextop = F8;
                op1=GetEd(emu, nextop);
                switch((nextop>>3)&7) {
                    case 0:                 /* INC Ed */
                        op1->dword[0] = inc32(emu, op1->dword[0]);
                        break;
                    case 1:                 /* DEC Ed */
                        op1->dword[0] = dec32(emu, op1->dword[0]);
                        break;
                    case 2:                 /* CALL NEAR Ed */
                        Push(emu, R_EIP);
                        R_EIP = op1->dword[0];
                        break;
                    case 3:                 /* CALL FAR Ed */
                        if(nextop>0xc0) {
                            printf_log(LOG_NONE, "Illegal Opcode %02X %02X\n", opcode, nextop);
                            emu->quit=1;
                            emu->error |= ERR_ILLEGAL;
                            goto fini;
                        } else {
                            Push16(emu, R_CS);
                            Push(emu, R_EIP);
                            R_EIP = op1->dword[0];
                            R_CS = (op1+1)->word[0];
                        }
                        break;
                    case 4:                 /* JMP NEAR Ed */
                        R_EIP = op1->dword[0];
                        break;
                    case 5:                 /* JMP FAR Ed */
                        if(nextop>0xc0) {
                            printf_log(LOG_NONE, "Illegal Opcode 0x%02X 0x%02X\n", opcode, nextop);
                            emu->quit=1;
                            emu->error |= ERR_ILLEGAL;
                            goto fini;
                        } else {
                            R_EIP = op1->dword[0];
                            R_CS = (op1+1)->word[0];
                        }
                        break;
                    case 6:                 /* Push Ed */
                        Push(emu, op1->dword[0]);
                        break;
                    default:
                        printf_log(LOG_NONE, "Illegal Opcode 0x%02X 0x%02X\n", opcode, nextop);
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                        goto fini;
                }
                NEXT;

            _default:
                UnimpOpcode(emu);
                goto fini;
        }
fini:
    if(emu->fork) {
        emu->fork = 0;
        emu = x86emu_fork(emu);
        goto x86emurun;
    }
    return 0;
}
