#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "box86stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"
#include "x87emu_private.h"
#include "box86context.h"
#include "my_cpuid.h"
#include "bridge.h"
#ifdef DYNAREC
#include "../dynarec/arm_lock_helper.h"
#endif

int my_setcontext(x86emu_t* emu, void* ucp);

int Run(x86emu_t *emu, int step)
{
    uint8_t opcode;
    uint8_t nextop;
    reg32_t *oped;
    uint8_t tmp8u, tmp8u2;
    int8_t tmp8s;
    uint16_t tmp16u, tmp16u2;
    int16_t tmp16s;
    uint32_t tmp32u, tmp32u2, tmp32u3;
    int32_t tmp32s, tmp32s2;
    uint64_t tmp64u;
    int64_t tmp64s;
    uintptr_t ip;
    double d;
    float f;
    int64_t ll;
    sse_regs_t *opex, eax1;
    mmx_regs_t *opem, eam1;

    if(emu->quit)
        return 0;

    //ref opcode: http://ref.x86asm.net/geek32.html#xA1
    printf_log(LOG_DEBUG, "Run X86 (%p), EIP=%p, Stack=%p\n", emu, (void*)R_EIP, emu->context->stack);
#define F8      *(uint8_t*)(ip++)
#define F8S     *(int8_t*)(ip++)
#define F16     *(uint16_t*)(ip+=2, ip-2)
#define F32     *(uint32_t*)(ip+=4, ip-4)
#define F32S    *(int32_t*)(ip+=4, ip-4)
#define PK(a)   *(uint8_t*)(ip+a)
#ifdef DYNAREC
#define STEP if(step) goto stepout;
#else
#define STEP
#endif

    static const void* baseopcodes[256] ={
    &&_0x00_0,  &&_0x00_1,  &&_0x00_2,  &&_0x00_3,  &&_0x00_4,  &&_0x00_5,  &&_0x06,    &&_0x07,      //0x00-0x07
    &&_0x08_0,  &&_0x08_1,  &&_0x08_2,  &&_0x08_3,  &&_0x08_4,  &&_0x08_5,  &&_0x0E,    &&_0x0F,      //0x08-0x0F
    &&_0x10_0,  &&_0x10_1,  &&_0x10_2,  &&_0x10_3,  &&_0x10_4,  &&_0x10_5,  &&_0x16,    &&_0x17,      //0x10-0x17
    &&_0x18_0,  &&_0x18_1,  &&_0x18_2,  &&_0x18_3,  &&_0x18_4,  &&_0x18_5,  &&_0x1E,    &&_0x1F,      //0x18-0x1F
    &&_0x20_0,  &&_0x20_1,  &&_0x20_2,  &&_0x20_3,  &&_0x20_4,  &&_0x20_5,  &&_0x26,    &&_0x27,      //0x20-0x27
    &&_0x28_0,  &&_0x28_1,  &&_0x28_2,  &&_0x28_3,  &&_0x28_4,  &&_0x28_5,  &&_0x2E,    &&_0x2F,      //0x28-0x2F
    &&_0x30_0,  &&_0x30_1,  &&_0x30_2,  &&_0x30_3,  &&_0x30_4,  &&_0x30_5,  &&_0x36,    &&_0x37,      //0x30-0x37
    &&_0x38,    &&_0x39,    &&_0x3A,    &&_0x3B,    &&_0x3C,    &&_0x3D,    &&_0x3E,    &&_0x3F,      //0x38-0x3F
    &&_0x40,    &&_0x41,    &&_0x42,    &&_0x43,    &&_0x44,    &&_0x45,    &&_0x46,    &&_0x47, 
    &&_0x48,    &&_0x49,    &&_0x4A,    &&_0x4B,    &&_0x4C,    &&_0x4D,    &&_0x4E,    &&_0x4F,     
    &&_0x50,    &&_0x51,    &&_0x52,    &&_0x53,    &&_0x54,    &&_0x55,    &&_0x56,    &&_0x57, 
    &&_0x58,    &&_0x59,    &&_0x5A,    &&_0x5B,    &&_0x5C,    &&_0x5D,    &&_0x5E,    &&_0x5F, 
    &&_0x60,    &&_0x61,    &&_0x62,    &&_0x63,    &&_0x64,    &&_0x65,    &&_0x66,    &&_0x67,
    &&_0x68,    &&_0x69,    &&_0x6A,    &&_0x6B,    &&_0x6C,    &&_0x6D,    &&_0x6E,    &&_0x6F,      //0x68-0x6F
    &&_0x70_0,  &&_0x70_1,  &&_0x70_2,  &&_0x70_3,  &&_0x70_4,  &&_0x70_5,  &&_0x70_6,  &&_0x70_7,    //0x70-0x77
    &&_0x70_8,  &&_0x70_9,  &&_0x70_A,  &&_0x70_B,  &&_0x70_C,  &&_0x70_D,  &&_0x70_E,  &&_0x70_F,    //0x78-0x7F
    &&_0x80,    &&_0x81,    &&_0x82,    &&_0x83,    &&_0x84,    &&_0x85,    &&_0x86,    &&_0x87,     
    &&_0x88,    &&_0x89,    &&_0x8A,    &&_0x8B,    &&_0x8C,    &&_0x8D,    &&_0x8E,    &&_0x8F,     
    &&_0x90,    &&_0x91,    &&_0x92,    &&_0x93,    &&_0x94,    &&_0x95,    &&_0x96,    &&_0x97, 
    &&_0x98,    &&_0x99,    &&_default, &&_0x9B,    &&_0x9C,    &&_0x9D,    &&_0x9E,    &&_0x9F,
    &&_0xA0,    &&_0xA1,    &&_0xA2,    &&_0xA3,    &&_0xA4,    &&_0xA5,    &&_0xA6,    &&_0xA7, 
    &&_0xA8,    &&_0xA9,    &&_0xAA,    &&_0xAB,    &&_0xAC,    &&_0xAD,    &&_0xAE,    &&_0xAF,     
    &&_0xB0,    &&_0xB1,    &&_0xB2,    &&_0xB3,    &&_0xB4,    &&_0xB5,    &&_0xB6,    &&_0xB7, 
    &&_0xB8,    &&_0xB9,    &&_0xBA,    &&_0xBB,    &&_0xBC,    &&_0xBD,    &&_0xBE,    &&_0xBF, 
    &&_0xC0,    &&_0xC1,    &&_0xC2,    &&_0xC3,    &&_0xC4,    &&_0xC5,    &&_0xC6,    &&_0xC7, 
    &&_0xC8,    &&_0xC9,    &&_default, &&_0xCB,    &&_0xCC,    &&_0xCD,    &&_default, &&_0xCF,      //0xC8-0xCF
    &&_0xD0,    &&_0xD1,    &&_0xD2,    &&_0xD3,    &&_0xD4,    &&_0xD5,    &&_0xD6,    &&_0xD7, 
    &&_0xD8,    &&_0xD9,    &&_0xDA,    &&_0xDB,    &&_0xDC,    &&_0xDD,    &&_0xDE,    &&_0xDF, 
    &&_0xE0,    &&_0xE1,    &&_0xE2,    &&_0xE3,    &&_0xE4,    &&_0xE5,    &&_0xE6,    &&_0xE7,
    &&_0xE8,    &&_0xE9,    &&_default, &&_0xEB,    &&_0xEC,    &&_0xED,    &&_default, &&_default,
    &&_0xF0,    &&_0xF1,    &&_0xF2,    &&_0xF3,    &&_0xF4,    &&_0xF5,    &&_0xF6,    &&_0xF7, 
    &&_0xF8,    &&_0xF9,    &&_0xFA,    &&_0xFB,    &&_0xFC,    &&_0xFD,    &&_0xFE,    &&_0xFF
    };

    static const void* opcodes0f[256] = {
    &&_0f_0x00, &&_0f_0x01, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x00-0x07
    &&_default, &&_default, &&_default, &&_0f_0x0B, &&_default ,&&_default, &&_default, &&_default, //0x08-0x0F
    &&_0f_0x10, &&_0f_0x11, &&_0f_0x12, &&_0f_0x13, &&_0f_0x14, &&_0f_0x15, &&_0f_0x16, &&_0f_0x17, //0x10-0x17 
    &&_0f_0x18, &&_default, &&_0f_0x1A, &&_0f_0x1B, &&_default ,&&_default, &&_default, &&_0f_0x1F, //0x18-0x1F
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x20-0x27
    &&_0f_0x28, &&_0f_0x29, &&_0f_0x2A, &&_0f_0x2B, &&_0f_0x2C, &&_0f_0x2D, &&_0f_0x2E, &&_0f_0x2F, 
    &&_default, &&_0f_0x31, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x30-0x37
    &&_0f_0x38, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x38-0x3F
    &&_0f_0x40_0, &&_0f_0x40_1, &&_0f_0x40_2, &&_0f_0x40_3, &&_0f_0x40_4, &&_0f_0x40_5, &&_0f_0x40_6, &&_0f_0x40_7,
    &&_0f_0x40_8, &&_0f_0x40_9, &&_0f_0x40_A, &&_0f_0x40_B, &&_0f_0x40_C, &&_0f_0x40_D, &&_0f_0x40_E, &&_0f_0x40_F,
    &&_0f_0x50, &&_0f_0x51, &&_0f_0x52, &&_0f_0x53, &&_0f_0x54, &&_0f_0x55, &&_0f_0x56, &&_0f_0x57, //0x50-0x57
    &&_0f_0x58, &&_0f_0x59, &&_0f_0x5A, &&_0f_0x5B, &&_0f_0x5C, &&_0f_0x5D, &&_0f_0x5E, &&_0f_0x5F, 
    &&_0f_0x60, &&_0f_0x61, &&_0f_0x62, &&_0f_0x63, &&_0f_0x64 ,&&_0f_0x65, &&_0f_0x66, &&_0f_0x67, //0x60-0x67
    &&_0f_0x68, &&_0f_0x69, &&_0f_0x6A, &&_0f_0x6B, &&_default ,&&_default, &&_0f_0x6E, &&_0f_0x6F, //0x68-0x6F
    &&_0f_0x70, &&_0f_0x71, &&_0f_0x72, &&_0f_0x73, &&_0f_0x74 ,&&_0f_0x75, &&_0f_0x76, &&_0f_0x77, //0x70-0x77
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_0f_0x7E, &&_0f_0x7F, //0x78-0x7F
    &&_0f_0x80_0, &&_0f_0x80_1, &&_0f_0x80_2, &&_0f_0x80_3, &&_0f_0x80_4, &&_0f_0x80_5, &&_0f_0x80_6, &&_0f_0x80_7,
    &&_0f_0x80_8, &&_0f_0x80_9, &&_0f_0x80_A, &&_0f_0x80_B, &&_0f_0x80_C, &&_0f_0x80_D, &&_0f_0x80_E, &&_0f_0x80_F,
    &&_0f_0x90_0, &&_0f_0x90_1, &&_0f_0x90_2, &&_0f_0x90_3, &&_0f_0x90_4, &&_0f_0x90_5, &&_0f_0x90_6, &&_0f_0x90_7,
    &&_0f_0x90_8, &&_0f_0x90_9, &&_0f_0x90_A, &&_0f_0x90_B, &&_0f_0x90_C, &&_0f_0x90_D, &&_0f_0x90_E, &&_0f_0x90_F,
    &&_0f_0xA0, &&_0f_0xA1, &&_0f_0xA2, &&_0f_0xA3, &&_0f_0xA4, &&_0f_0xA5, &&_default, &&_default, //0xA0-0xA7
    &&_0f_0xA8, &&_0f_0xA9, &&_default, &&_0f_0xAB, &&_0f_0xAC, &&_0f_0xAD, &&_0f_0xAE, &&_0f_0xAF, 
    &&_0f_0xB0, &&_0f_0xB1, &&_default, &&_0f_0xB3, &&_default, &&_default, &&_0f_0xB6, &&_0f_0xB7, 
    &&_default, &&_default, &&_0f_0xBA, &&_0f_0xBB, &&_0f_0xBC, &&_0f_0xBD, &&_0f_0xBE, &&_0f_0xBF, 
    &&_0f_0xC0, &&_0f_0xC1, &&_0f_0xC2, &&_0f_0xC3, &&_0f_0xC4, &&_0f_0xC5, &&_0f_0xC6, &&_0f_0xC7, 
    &&_0f_0xC8, &&_0f_0xC9, &&_0f_0xCA, &&_0f_0xCB, &&_0f_0xCC, &&_0f_0xCD, &&_0f_0xCE, &&_0f_0xCF, //0xC8-0xCF
    &&_default, &&_0f_0xD1, &&_0f_0xD2, &&_0f_0xD3, &&_0f_0xD4 ,&&_0f_0xD5, &&_default, &&_0f_0xD7, //0xD0-0xD7
    &&_0f_0xD8, &&_0f_0xD9, &&_0f_0xDA, &&_0f_0xDB, &&_0f_0xDC ,&&_0f_0xDD, &&_0f_0xDE, &&_0f_0xDF, //0xD8-0xDF
    &&_0f_0xE0, &&_0f_0xE1, &&_0f_0xE2, &&_0f_0xE3, &&_0f_0xE4 ,&&_0f_0xE5, &&_default, &&_0f_0xE7, //0xE0-0xE7
    &&_0f_0xE8, &&_0f_0xE9, &&_0f_0xEA, &&_0f_0xEB, &&_0f_0xEC ,&&_0f_0xED, &&_0f_0xEE, &&_0f_0xEF, //0xE8-0xEF
    &&_default, &&_0f_0xF1, &&_0f_0xF2, &&_0f_0xF3, &&_0f_0xF4 ,&&_0f_0xF5, &&_0f_0xF6, &&_0f_0xF7, //0xF0-0xF7
    &&_0f_0xF8, &&_0f_0xF9, &&_0f_0xFA, &&_default, &&_0f_0xFC ,&&_0f_0xFD, &&_0f_0xFE, &&_default  //0xF8-0xFF
    };

    static const void* opcodes66[256] = {
    &&_66_0x00_0, &&_66_0x00_1, &&_66_0x00_2, &&_66_0x00_3, &&_66_0x00_4 ,&&_66_0x00_5, &&_66_0x06, &&_66_0x07, //0x00-0x07
    &&_66_0x08_0, &&_66_0x08_1, &&_66_0x08_2, &&_66_0x08_3, &&_66_0x08_4 ,&&_66_0x08_5, &&_default, &&_66_0x0F, //0x08-0x0F
    &&_66_0x10_0, &&_66_0x10_1, &&_66_0x10_2, &&_66_0x10_3, &&_66_0x10_4 ,&&_66_0x10_5, &&_default, &&_default, //0x10-0x17
    &&_66_0x18_0, &&_66_0x18_1, &&_66_0x18_2, &&_66_0x18_3, &&_66_0x18_4 ,&&_66_0x18_5, &&_default, &&_66_0x1F, //0x18-0x1F
    &&_66_0x20_0, &&_66_0x20_1, &&_66_0x20_2, &&_66_0x20_3, &&_66_0x20_4 ,&&_66_0x20_5, &&_66_0x26, &&_default, //0x20-0x27
    &&_66_0x28_0, &&_66_0x28_1, &&_66_0x28_2, &&_66_0x28_3, &&_66_0x28_4 ,&&_66_0x28_5, &&_66_0x2E, &&_default, //0x28-0x2F
    &&_66_0x30_0, &&_66_0x30_1, &&_66_0x30_2, &&_66_0x30_3, &&_66_0x30_4 ,&&_66_0x30_5, &&_66_0x36, &&_default, //0x30-0x37
    &&_default, &&_66_0x39, &&_default, &&_66_0x3B, &&_default, &&_66_0x3D, &&_default, &&_default, //0x38-0x3F
    &&_66_0x40, &&_66_0x41, &&_66_0x42, &&_66_0x43, &&_66_0x44, &&_66_0x45, &&_66_0x46, &&_66_0x47, 
    &&_66_0x48, &&_66_0x49, &&_66_0x4A, &&_66_0x4B, &&_66_0x4C, &&_66_0x4D, &&_66_0x4E, &&_66_0x4F, 
    &&_66_0x50, &&_66_0x51, &&_66_0x52, &&_66_0x53, &&_66_0x54, &&_66_0x55, &&_66_0x56, &&_66_0x57, //0x50-0x57
    &&_66_0x58, &&_66_0x59, &&_66_0x5A, &&_66_0x5B, &&_66_0x5C, &&_66_0x5D, &&_66_0x5E, &&_66_0x5F, //0x58-0x5F
    &&_66_0x60, &&_66_0x61, &&_default, &&_default, &&_66_0x64 ,&&_default, &&_66_0x66, &&_default, //0x60-0x67
    &&_66_0x68, &&_66_0x69, &&_66_0x6A, &&_66_0x6B, &&_default, &&_default, &&_default, &&_default, //0x68-0x6F
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x70-0x77
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0x78-0x7F
    &&_default, &&_66_0x81, &&_default, &&_66_0x83, &&_default, &&_66_0x85, &&_default, &&_66_0x87, 
    &&_default, &&_66_0x89, &&_default, &&_66_0x8B, &&_66_0x8C, &&_default, &&_66_0x8E, &&_66_0x8F, 
    &&_66_0x90, &&_66_0x91, &&_66_0x92, &&_66_0x93, &&_66_0x94, &&_66_0x95, &&_66_0x96, &&_66_0x97, 
    &&_66_0x98, &&_66_0x99, &&_default, &&_default, &&_66_0x9C, &&_66_0x9D, &&_default, &&_default, //0x98-0x9F
    &&_default, &&_66_0xA1, &&_default, &&_66_0xA3, &&_default, &&_66_0xA5, &&_default, &&_66_0xA7, 
    &&_default, &&_66_0xA9, &&_default, &&_66_0xAB, &&_default, &&_66_0xAD, &&_default, &&_66_0xAF, //0xA8-0xAF
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0xB0-0xB7
    &&_66_0xB8, &&_66_0xB9, &&_66_0xBA, &&_66_0xBB, &&_66_0xBC, &&_66_0xBD, &&_66_0xBE, &&_66_0xBF, 
    &&_default, &&_66_0xC1, &&_default, &&_default, &&_default, &&_default, &&_default, &&_66_0xC7, 
    &&_default, &&_default, &&_default, &&_66_0xCB, &&_66_0xCC ,&&_default, &&_default, &&_default, //0xC8-0xCF
    &&_default, &&_66_0xD1, &&_default, &&_66_0xD3, &&_default, &&_default, &&_default, &&_default, //0xD0-0xD7
    &&_default, &&_66_0xD9, &&_default, &&_default, &&_default ,&&_66_0xDD, &&_default, &&_default, //0xD8-0xDF
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0xE0-0xE7
    &&_default, &&_default, &&_default, &&_default, &&_default ,&&_default, &&_default, &&_default, //0xE8-0xEF
    &&_66_0xF0, &&_default, &&_66_0xF2, &&_66_0xF3, &&_default, &&_default, &&_default, &&_66_0xF7, 
    &&_66_0xF8, &&_66_0xF9, &&_default, &&_default, &&_default, &&_default, &&_default, &&_66_0xFF
    };

x86emurun:
    ip = R_EIP;
#ifdef HAVE_TRACE
_trace:
    __builtin_prefetch((void*)ip, 0, 0); 
    emu->prev2_ip = emu->prev_ip;
    emu->prev_ip = R_EIP;
    R_EIP=ip;
    if(my_context->dec && (
        (trace_end == 0) 
        || ((ip >= trace_start) && (ip < trace_end))) )
            PrintTrace(emu, ip, 0);

    #define NEXT    goto _trace
#else
    #define NEXT    goto *baseopcodes[(R_EIP=ip, opcode=F8)]
#endif

#include "modrm.h"

    opcode = F8;
    goto *baseopcodes[opcode];

        #define GO(B, OP)                      \
        _##B##_0: \
            nextop = F8;               \
            GET_EB;             \
            EB->byte[0] = OP##8(emu, EB->byte[0], GB);  \
            NEXT;                              \
        _##B##_1: \
            nextop = F8;               \
            GET_ED;             \
            ED->dword[0] = OP##32(emu, ED->dword[0], GD.dword[0]); \
            NEXT;                              \
        _##B##_2: \
            nextop = F8;               \
            GET_EB;                   \
            GB = OP##8(emu, GB, EB->byte[0]); \
            NEXT;                              \
        _##B##_3: \
            nextop = F8;               \
            GET_ED;         \
            GD.dword[0] = OP##32(emu, GD.dword[0], ED->dword[0]); \
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
            GET_EB;
            cmp8(emu, EB->byte[0], GB);
            NEXT;
        _0x39:
            nextop = F8;
            GET_ED;
            cmp32(emu, ED->dword[0], GD.dword[0]);
            NEXT;
        _0x3A:
            nextop = F8;
            GET_EB;
            cmp8(emu, GB, EB->byte[0]);
            NEXT;
        _0x3B:
            nextop = F8;
            GET_ED;
            cmp32(emu, GD.dword[0], ED->dword[0]);
            NEXT;
        _0x3C:
            cmp8(emu, R_AL, F8);
            NEXT;
        _0x3D:
            cmp32(emu, R_EAX, F32);
            NEXT;

        _0x06:                      /* PUSH ES */
            Push(emu, emu->segs[_ES]);    // even if a segment is a 16bits, a 32bits push/pop is done
            NEXT;
        _0x07:                      /* POP ES */
            emu->segs[_ES] = Pop(emu);    // no check, no use....
            emu->segs_serial[_ES] = 0;
            NEXT;

        _0x0E:                      /* PUSH CS */
            Push(emu, emu->segs[_CS]);
            NEXT;
        _0x0F:                      /* More instructions */
            #include "run0f.h"
            NEXT;

        _0x16:                      /* PUSH SS */
            Push(emu, emu->segs[_SS]);    // even if a segment is a 16bits, a 32bits push/pop is done
            NEXT;
        _0x17:                      /* POP SS */
            emu->segs[_SS] = Pop(emu);    // no check, no use....
            emu->segs_serial[_SS] = 0;
            NEXT;

        _0x1E:                      /* PUSH DS */
            Push(emu, emu->segs[_DS]);  // even if a segment is a 16bits, a 32bits push/pop is done
            NEXT;
        _0x1F:                      /* POP DS */
            emu->segs[_DS] = Pop(emu);    // no check, no use....
            emu->segs_serial[_DS] = 0;
            NEXT;

        _0x26:                      /* ES: */
            NEXT;  //ignored...
        _0x27:                      /* DAA */
            R_AL = daa8(emu, R_AL);
            NEXT;

        _0x2E:                      /* CS: */
            // Can also happens before a condition branch: Branch Not Taken Hint
            NEXT;  //ignored...
        _0x2F:                      /* DAS */
            R_AL = das8(emu, R_AL);
            NEXT;

        _0x36:                      /* SS: */
            NEXT;
        _0x37:                      /* AAA */
            R_AX = aaa16(emu, R_AX);
            NEXT;

        _0x3E:                      /* DS: */
            // Can also happens before a condition branch: Branch Taken Hint
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
        _0x58:
        _0x59:
        _0x5A:
        _0x5B:
        _0x5C:                      /* POP ESP */
        _0x5D:
        _0x5E:
        _0x5F:                      /* POP Reg */
            tmp8u = opcode&7;
            emu->regs[tmp8u].dword[0] = Pop(emu);
            NEXT;
        _0x60:                      /* PUSHAD */
            tmp32u = R_ESP;
            Push(emu, R_EAX);
            Push(emu, R_ECX);
            Push(emu, R_EDX);
            Push(emu, R_EBX);
            Push(emu, tmp32u);
            Push(emu, R_EBP);
            Push(emu, R_ESI);
            Push(emu, R_EDI);
            NEXT;
        _0x61:                      /* POPAD */
            R_EDI = Pop(emu);
            R_ESI = Pop(emu);
            R_EBP = Pop(emu);
            R_ESP+=4;   // POP ESP
            R_EBX = Pop(emu);
            R_EDX = Pop(emu);
            R_ECX = Pop(emu);
            R_EAX = Pop(emu);
            NEXT;
        _0x62:                      /* BOUND Gd, Ed */
            nextop = F8;
            GET_ED;
            // ignoring the test for now
            NEXT;
        _0x63:                      /* ARPL Ew, Gw */
            nextop = F8;
            GET_EW;
            // faking to always happy...
            SET_FLAG(F_ZF);
            NEXT;
        _0x64:                      /* FS: */
            emu->old_ip = R_EIP;
            R_EIP = ip-1;
            RunFS(emu); // implemented in Run66.c
            ip = R_EIP;
            if(emu->quit) goto fini;
            STEP
            NEXT;
        _0x65:                      /* GS: */
            emu->old_ip = R_EIP;
            R_EIP = ip-1;
            RunGS(emu); // implemented in Run66.c
            ip = R_EIP;
            if(emu->quit) goto fini;
            STEP
            NEXT;

        _0x66:                      /* Prefix to change width of intructions, so here, down to 16bits */
            #include "run66.h"
        _0x67:                      /* Prefix to change width of registers */
            emu->old_ip = R_EIP;
            R_EIP = ip-1;   // don't count 0x67 yet
            Run67(emu); // implemented in Run66.c
            ip = R_EIP;
            if(emu->quit) goto fini;
            STEP
            NEXT;

        _0x68:                      /* Push Id */
            Push(emu, F32);
            NEXT;
        _0x69:                      /* IMUL Gd,Ed,Id */
            nextop = F8;
            GET_ED;
            tmp32u = F32;
            GD.dword[0] = imul32(emu, ED->dword[0], tmp32u);
            NEXT;
        _0x6A:                      /* Push Ib */
            tmp32s = F8S;
            Push(emu, (uint32_t)tmp32s);
            NEXT;
        _0x6B:                      /* IMUL Gd,Ed,Ib */
            nextop = F8;
            GET_ED;
            tmp32s = F8S;
            GD.dword[0] = imul32(emu, ED->dword[0], (uint32_t)tmp32s);
            NEXT;
        _0x6C:                      /* INSB */
            *(int8_t*)(R_EDI+GetESBaseEmu(emu)) = 0;   // faking port read, using explicit ES segment
            if(ACCESS_FLAG(F_DF))
                R_EDI-=1;
            else
                R_EDI+=1;
            NEXT;
        _0x6D:                      /* INSD */
            *(int32_t*)(R_EDI+GetESBaseEmu(emu)) = 0;   // faking port read, using explicit ES segment
            if(ACCESS_FLAG(F_DF))
                R_EDI-=4;
            else
                R_EDI+=4;
            NEXT;
        _0x6E:                      /* OUTSB */
            // faking port write, using explicit ES segment
            if(ACCESS_FLAG(F_DF))
                R_ESI-=1;
            else
                R_ESI+=1;
            NEXT;
        _0x6F:                      /* OUTSD */
            // faking port write, using explicit ES segment
            if(ACCESS_FLAG(F_DF))
                R_ESI-=4;
            else
                R_ESI+=4;
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
            ,   ip += tmp8s;
            )                           /* Jxx Ib */
        #undef GOCOND

        
        _0x80:                      /* GRP Eb,Ib */
        _0x82:                      // 0x82 and 0x80 are the same opcodes it seems?
            nextop = F8;
            GET_EB;
            tmp8u = F8;
            switch((nextop>>3)&7) {
                case 0: EB->byte[0] = add8(emu, EB->byte[0], tmp8u); break;
                case 1: EB->byte[0] =  or8(emu, EB->byte[0], tmp8u); break;
                case 2: EB->byte[0] = adc8(emu, EB->byte[0], tmp8u); break;
                case 3: EB->byte[0] = sbb8(emu, EB->byte[0], tmp8u); break;
                case 4: EB->byte[0] = and8(emu, EB->byte[0], tmp8u); break;
                case 5: EB->byte[0] = sub8(emu, EB->byte[0], tmp8u); break;
                case 6: EB->byte[0] = xor8(emu, EB->byte[0], tmp8u); break;
                case 7:               cmp8(emu, EB->byte[0], tmp8u); break;
            }
            NEXT;
        _0x81:                      /* GRP Ed,Id */
            nextop = F8;
            GET_ED;
            tmp32u = F32;
            goto _0x81_common;
        _0x83:                      /* GRP Ed,Ib */
            nextop = F8;
            GET_ED;
            tmp32s = F8S;
            tmp32u = (uint32_t)tmp32s;
        _0x81_common:
            switch((nextop>>3)&7) {
                case 0: ED->dword[0] = add32(emu, ED->dword[0], tmp32u); break;
                case 1: ED->dword[0] =  or32(emu, ED->dword[0], tmp32u); break;
                case 2: ED->dword[0] = adc32(emu, ED->dword[0], tmp32u); break;
                case 3: ED->dword[0] = sbb32(emu, ED->dword[0], tmp32u); break;
                case 4: ED->dword[0] = and32(emu, ED->dword[0], tmp32u); break;
                case 5: ED->dword[0] = sub32(emu, ED->dword[0], tmp32u); break;
                case 6: ED->dword[0] = xor32(emu, ED->dword[0], tmp32u); break;
                case 7:                cmp32(emu, ED->dword[0], tmp32u); break;
            }
            NEXT;
        _0x84:                      /* TEST Eb,Gb */
            nextop = F8;
            GET_EB;
            test8(emu, EB->byte[0], GB);
            NEXT;
        _0x85:                      /* TEST Ed,Gd */
            nextop = F8;
            GET_ED;
            test32(emu, ED->dword[0], GD.dword[0]);
            NEXT;
        _0x86:                      /* XCHG Eb,Gb */
            nextop = F8;
#ifdef DYNAREC
            GET_EB;
            if((nextop&0xC0)==0xC0) { // reg / reg: no lock
                tmp8u = GB;
                GB = EB->byte[0];
                EB->byte[0] = tmp8u;
            } else {
                do {
                    tmp8u = arm_lock_read_b(EB);
                } while(arm_lock_write_b(EB, GB));
                GB = tmp8u;
            }
            // dynarec use need it's own mecanism
#else
            GET_EB;
            if((nextop&0xC0)!=0xC0)
                pthread_mutex_lock(&emu->context->mutex_lock); // XCHG always LOCK (but when accessing memory only)
            tmp8u = GB;
            GB = EB->byte[0];
            EB->byte[0] = tmp8u;
            if((nextop&0xC0)!=0xC0)
                pthread_mutex_unlock(&emu->context->mutex_lock);
#endif                
            NEXT;
        _0x87:                      /* XCHG Ed,Gd */
            nextop = F8;
#ifdef DYNAREC
            GET_ED;
            if((nextop&0xC0)==0xC0) {
                tmp32u = GD.dword[0];
                GD.dword[0] = ED->dword[0];
                ED->dword[0] = tmp32u;
            } else {
                if(((uintptr_t)ED)&3)
                {
                    // not aligned, dont't try to "LOCK"
                    tmp32u = ED->dword[0];
                    ED->dword[0] = GD.dword[0];
                    GD.dword[0] = tmp32u;
                } else {
                    // XCHG is supposed to automaticaly LOCK memory bus
                    GD.dword[0] = arm_lock_xchg(ED, GD.dword[0]);
                }
            }
#else
            GET_ED;
            if((nextop&0xC0)!=0xC0)
                pthread_mutex_lock(&emu->context->mutex_lock); // XCHG always LOCK (but when accessing memory only)
            tmp32u = GD.dword[0];
            GD.dword[0] = ED->dword[0];
            ED->dword[0] = tmp32u;
            if((nextop&0xC0)!=0xC0)
                pthread_mutex_unlock(&emu->context->mutex_lock);
#endif
            NEXT;
        _0x88:                      /* MOV Eb,Gb */
            nextop = F8;
            GET_EB;
            EB->byte[0] = GB;
            NEXT;
        _0x89:                      /* MOV Ed,Gd */
            nextop = F8;
            GET_ED;
            ED->dword[0] = GD.dword[0];
            NEXT;
        _0x8A:                      /* MOV Gb,Eb */
            nextop = F8;
            GET_EB;
            GB = EB->byte[0];
            NEXT;
        _0x8B:                      /* MOV Gd,Ed */
            nextop = F8;
            GET_ED;
            GD.dword[0] = ED->dword[0];
            NEXT;
        _0x8C:                      /* MOV Ed, Seg */
            nextop = F8;
            GET_ED;
            ED->dword[0] = emu->segs[((nextop&0x38)>>3)];
            NEXT;
        _0x8D:                      /* LEA Gd,M */
            nextop = F8;
            GET_ED;
            GD.dword[0] = (uint32_t)ED;
            NEXT;
        _0x8E:                      /* MOV Seg,Ew */
            nextop = F8;
            GET_EW;
            emu->segs[((nextop&0x38)>>3)] = EW->word[0];
            emu->segs_serial[((nextop&0x38)>>3)] = 0;
            if(((nextop&0x38)>>3)==_FS)
                default_fs = EW->word[0];
            NEXT;
        _0x8F:                      /* POP Ed */
            nextop = F8;
            if((nextop&0xC0)==0xC0) {
                emu->regs[(nextop&7)].dword[0] = Pop(emu);
            } else {
                tmp32u = Pop(emu);  // this order allows handling POP [ESP] and variant
                GET_ED;
                R_ESP -= 4; // to prevent issue with SEGFAULT
                ED->dword[0] = tmp32u;
                R_ESP += 4;
            }
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
            emu->regs[_AX].sdword[0] = emu->regs[_AX].sword[0];
            NEXT;
        _0x99:                      /* CDQ */
            R_EDX=(R_EAX & 0x80000000)?0xFFFFFFFF:0x00000000;
            NEXT;

        _0x9B:                      /* FWAIT */
            NEXT;
        _0x9C:                      /* PUSHF */
            CHECK_FLAGS(emu);
            Push(emu, emu->eflags.x32);
            NEXT;
        _0x9D:                      /* POPF */
            emu->eflags.x32 = ((Pop(emu) & 0x3F7FD7)/* & (0xffff-40)*/ ) | 0x2; // mask off res2 and res3 and on res1
            RESET_FLAGS(emu);
            NEXT;
        _0x9E:                      /* SAHF */
            tmp8u = emu->regs[_AX].byte[1];
            CONDITIONAL_SET_FLAG(tmp8u&0x01, F_CF);
            CONDITIONAL_SET_FLAG(tmp8u&0x04, F_PF);
            CONDITIONAL_SET_FLAG(tmp8u&0x10, F_AF);
            CONDITIONAL_SET_FLAG(tmp8u&0x40, F_ZF);
            CONDITIONAL_SET_FLAG(tmp8u&0x80, F_SF);
            RESET_FLAGS(emu);
            NEXT;
        _0x9F:                      /* LAHF */
            CHECK_FLAGS(emu);
            R_AH = (uint8_t)emu->eflags.x32;
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
            emu->regs[opcode&3].byte[0] = F8;
            NEXT;
        _0xB4:                      /* MOV AH,Ib */
        _0xB5:                      /*    ...    */
        _0xB6:
        _0xB7:
            emu->regs[opcode&3].byte[1] = F8;
            NEXT;
        _0xB8:                      /* MOV EAX,Id */
        _0xB9:                      /* MOV ECX,Id */
        _0xBA:                      /* MOV EDX,Id */
        _0xBB:                      /* MOV EBX,Id */
        _0xBC:                      /*    ...     */
        _0xBD:
        _0xBE:
        _0xBF:
            emu->regs[opcode&7].dword[0] = F32;
            NEXT;

        _0xC0:                      /* GRP2 Eb,Ib */
            nextop = F8;
            GET_EB;
            tmp8u = F8/* & 0x1f*/; // masking done in each functions
            switch((nextop>>3)&7) {
                case 0: EB->byte[0] = rol8(emu, EB->byte[0], tmp8u); break;
                case 1: EB->byte[0] = ror8(emu, EB->byte[0], tmp8u); break;
                case 2: EB->byte[0] = rcl8(emu, EB->byte[0], tmp8u); break;
                case 3: EB->byte[0] = rcr8(emu, EB->byte[0], tmp8u); break;
                case 4:
                case 6: EB->byte[0] = shl8(emu, EB->byte[0], tmp8u); break;
                case 5: EB->byte[0] = shr8(emu, EB->byte[0], tmp8u); break;
                case 7: EB->byte[0] = sar8(emu, EB->byte[0], tmp8u); break;
            }
            NEXT;
        _0xC1:                      /* GRP2 Ed,Ib */
            nextop = F8;
            GET_ED;
            tmp8u = F8/* & 0x1f*/; // masking done in each functions
            switch((nextop>>3)&7) {
                case 0: ED->dword[0] = rol32(emu, ED->dword[0], tmp8u); break;
                case 1: ED->dword[0] = ror32(emu, ED->dword[0], tmp8u); break;
                case 2: ED->dword[0] = rcl32(emu, ED->dword[0], tmp8u); break;
                case 3: ED->dword[0] = rcr32(emu, ED->dword[0], tmp8u); break;
                case 4:
                case 6: ED->dword[0] = shl32(emu, ED->dword[0], tmp8u); break;
                case 5: ED->dword[0] = shr32(emu, ED->dword[0], tmp8u); break;
                case 7: ED->dword[0] = sar32(emu, ED->dword[0], tmp8u); break;
            }
            NEXT;
        _0xC2:                      /* RETN Iw */
            tmp16u = F16;
            ip = Pop(emu);
            R_ESP += tmp16u;
            STEP
            NEXT;
        _0xC3:                      /* RET */
            ip = Pop(emu);
            STEP
            NEXT;
        _0xC4:                      /* LES Gd,Ed */
            nextop = F8;
            GET_ED;
            emu->segs[_ES] = ED->word[0];
            emu->segs_serial[_ES] = 0;
            GD.dword[0] = *(uint32_t*)(((void*)ED)+2);
            NEXT;
        _0xC5:                      /* LDS Gd,Ed */
            nextop = F8;
            GET_ED;
            emu->segs[_DS] = ED->word[0];
            emu->segs_serial[_DS] = 0;
            GD.dword[0] = *(uint32_t*)(((void*)ED)+2);
            NEXT;
        _0xC6:                      /* MOV Eb,Ib */
            nextop = F8;
            GET_EB;
            EB->byte[0] = F8;
            NEXT;
        _0xC7:                      /* MOV Ed,Id */
            nextop = F8;
            GET_ED;
            ED->dword[0] = F32;
            NEXT;

        _0xC8:                      /* ENTER Iw,Ib */
            tmp16u = F16;
            tmp8u = (F8) & 0x1f;
            tmp32u = R_EBP;
            Push(emu, R_EBP);
            R_EBP = R_ESP;
            if (tmp8u) {
                for (tmp8u2 = 1; tmp8u2 < tmp8u; tmp8u2++) {
                    tmp32u -= 4;
                    Push(emu, *((uint32_t*)tmp32u));
                }
                Push(emu, R_EBP);
            }
            R_ESP -= tmp16u;
            NEXT;

        _0xC9:                      /* LEAVE */
            R_ESP = R_EBP;
            R_EBP = Pop(emu);
            NEXT;

        _0xCB:                      /* FAR RET */
            ip = Pop(emu);
            emu->segs[_CS] = Pop(emu);    // no check, no use....
            emu->segs_serial[_CS] = 0;
            // need to check status of CS register!
            STEP
            NEXT;
        _0xCC:                      /* INT 3 */
            emu->old_ip = R_EIP;
            R_EIP = ip;
            x86Int3(emu);
            ip = R_EIP;
            if(emu->quit) goto fini;
            NEXT;
        _0xCD:                      /* INT Ib */
            nextop = F8;
            if(nextop == 0x80) {
                emu->old_ip = R_EIP;
                R_EIP = ip;
                x86Syscall(emu);
                ip = R_EIP;
                if(emu->quit) goto fini;
            } else {
                int tid = GetTID();
                printf_log(LOG_NONE, "%04d|%p: Ignoring Unsupported Int %02Xh\n", tid, (void*)ip, nextop);
                emu->old_ip = R_EIP;
                R_EIP = ip;
                emu->quit = 1;
                emu->error |= ERR_UNIMPL;
                goto fini;
            }
            NEXT;

        _0xCF:                      /* IRET */
            ip = Pop(emu);
            emu->segs[_CS] = Pop(emu);
            emu->segs_serial[_CS] = 0;
            emu->eflags.x32 = ((Pop(emu) & 0x3F7FD7)/* & (0xffff-40)*/ ) | 0x2; // mask off res2 and res3 and on res1
            RESET_FLAGS(emu);
            NEXT;
        _0xD0:                      /* GRP2 Eb,1 */
        _0xD2:                      /* GRP2 Eb,CL */
            nextop = F8;
            GET_EB;
            tmp8u = (opcode==0xD0)?1:R_CL;
            switch((nextop>>3)&7) {
                case 0: EB->byte[0] = rol8(emu, EB->byte[0], tmp8u); break;
                case 1: EB->byte[0] = ror8(emu, EB->byte[0], tmp8u); break;
                case 2: EB->byte[0] = rcl8(emu, EB->byte[0], tmp8u); break;
                case 3: EB->byte[0] = rcr8(emu, EB->byte[0], tmp8u); break;
                case 4: 
                case 6: EB->byte[0] = shl8(emu, EB->byte[0], tmp8u); break;
                case 5: EB->byte[0] = shr8(emu, EB->byte[0], tmp8u); break;
                case 7: EB->byte[0] = sar8(emu, EB->byte[0], tmp8u); break;
            }
            NEXT;
        _0xD1:                      /* GRP2 Ed,1 */
        _0xD3:                      /* GRP2 Ed,CL */
            nextop = F8;
            GET_ED;
            tmp8u = (opcode==0xD1)?1:R_CL;
            switch((nextop>>3)&7) {
                case 0: ED->dword[0] = rol32(emu, ED->dword[0], tmp8u); break;
                case 1: ED->dword[0] = ror32(emu, ED->dword[0], tmp8u); break;
                case 2: ED->dword[0] = rcl32(emu, ED->dword[0], tmp8u); break;
                case 3: ED->dword[0] = rcr32(emu, ED->dword[0], tmp8u); break;
                case 4: 
                case 6: ED->dword[0] = shl32(emu, ED->dword[0], tmp8u); break;
                case 5: ED->dword[0] = shr32(emu, ED->dword[0], tmp8u); break;
                case 7: ED->dword[0] = sar32(emu, ED->dword[0], tmp8u); break;
            }
            NEXT;
        _0xD4:                      /* AAM Ib */
            R_AX = aam16(emu, R_AL, F8);
            NEXT;
        _0xD5:                      /* AAD Ib */
            R_AX = aad16(emu, R_AX, F8);
            NEXT;
        _0xD6:
            CHECK_FLAGS(emu);
            R_AL = ACCESS_FLAG(F_CF)?0xFF:0x00;
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
                ip += tmp8s;
            STEP
            NEXT;
        _0xE1:                      /* LOOPZ */
            CHECK_FLAGS(emu);
            tmp8s = F8S;
            --R_ECX; // don't update flags
            if(R_ECX && ACCESS_FLAG(F_ZF))
                ip += tmp8s;
            STEP
            NEXT;
        _0xE2:                      /* LOOP */
            tmp8s = F8S;
            --R_ECX; // don't update flags
            if(R_ECX)
                ip += tmp8s;
            STEP
            NEXT;
        _0xE3:                      /* JECXZ */
            tmp8s = F8S;
            if(!R_ECX)
                ip += tmp8s;
            STEP
            NEXT;
        _0xE4:                      /* IN AL, Ib */
            tmp32s = F8;   // port address
            R_AL = 0;  // nothing... should do a warning maybe?
            NEXT;
        _0xE5:                      /* IN EAX, Ib */
            tmp32s = F8;   // port address
            R_EAX = 0;  // nothing... should do a warning maybe?
            NEXT;
        _0xE6:                      /* OUT Ib, AL */
            tmp32s = F8;    // port address
            //nothing goes out...
            NEXT;
        _0xE7:                      /* OUT Ib, EAX */
            tmp32s = F8;    // port address
            //nothing goes out...
            NEXT;
        _0xE8:                      /* CALL Id */
            tmp32s = F32S; // call is relative
            Push(emu, ip);
            ip += tmp32s;
            STEP
            NEXT;
        _0xE9:                      /* JMP Id */
            tmp32s = F32S; // jmp is relative
            ip += tmp32s;
            STEP
            NEXT;

        _0xEB:                      /* JMP Ib */
            tmp32s = F8S; // jump is relative
            ip += tmp32s;
            STEP
            NEXT;
        _0xEC:                      /* IN AL, DX */
            R_AL = 0;  // nothing... should do a warning maybe?
            NEXT;
        _0xED:                      /* IN EAX, DX */
            R_EAX = 0;  // nothing... should do a warning maybe?
            NEXT;

        _0xF0:                      /* LOCK */
            emu->old_ip = R_EIP;
            R_EIP = ip-1;   // dont't count F0 yet
            RunLock(emu); // implemented in Run66.c
            ip = R_EIP;
            if(emu->quit) goto fini;
            NEXT;
        _0xF1:                      /* INT1 */
            // does nothing for now
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
                STEP
                NEXT;
            } else if(nextop==0x66) {
                nextop = F8;
                tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
                tmp32u = R_ECX;
                switch(nextop) {
                    case 0xA5:              /* REP MOVSW */
                        tmp8s *= 2;
                        while(tmp32u) {
                            --tmp32u;
                            *(uint16_t*)R_EDI = *(uint16_t*)R_ESI;
                            R_EDI += tmp8s;
                            R_ESI += tmp8s;
                        }
                        break;
                    case 0xAB:              /* REP STOSW */
                        tmp8s *= 2;
                        while(tmp32u) {
                            --tmp32u;
                            *(uint16_t*)R_EDI = R_AX;
                            R_EDI += tmp8s;
                        }
                        break;
                    case 0xA7:              /* REP(N)Z CMPSW */
                        tmp16u = 0;
                        tmp16u2 = 0;
                        tmp8s *= 2;
                        if(opcode==0xF2) {
                            while(tmp32u) {
                                --tmp32u;
                                tmp16u2 = *(uint16_t*)R_EDI;
                                tmp16u  = *(uint16_t*)R_ESI;
                                R_EDI += tmp8s;
                                R_ESI += tmp8s;
                                if(tmp16u2==tmp16u)
                                    break;
                            }
                        } else {
                            while(tmp32u) {
                                --tmp32u;
                                tmp16u2 = *(uint16_t*)R_EDI;
                                tmp16u  = *(uint16_t*)R_ESI;
                                R_EDI += tmp8s;
                                R_ESI += tmp8s;
                                if(tmp16u2!=tmp16u)
                                    break;
                            }
                        }
                        if(R_ECX) cmp16(emu, tmp16u, tmp16u2);
                        break;
                    case 0xAF:              /* REP(N)Z SCASW */
                        tmp16u = 0;
                        tmp8s *= 2;
                        if(opcode==0xF2) {
                            while(tmp32u) {
                                --tmp32u;
                                tmp16u = *(uint16_t*)R_EDI;
                                R_EDI += tmp8s;
                                if(R_AX==tmp16u)
                                    break;
                            }
                        } else {
                            while(tmp32u) {
                                --tmp32u;
                                tmp16u = *(uint16_t*)R_EDI;
                                R_EDI += tmp8s;
                                if(R_AX!=tmp16u)
                                    break;
                            }
                        }
                        if(R_ECX) cmp16(emu, R_AX, tmp16u);
                        break;
                    default:
                        goto _default;
                }
                R_ECX = tmp32u;
            } else {
                tmp8s = ACCESS_FLAG(F_DF)?-1:+1;
                tmp32u = R_ECX;
                switch(nextop) {
                    case 0x18:
                    case 0x19:
                    case 0x1A:
                    case 0x1B:
                    case 0x1C:
                    case 0x1D:
                    case 0x25:
                    case 0x40:
                    case 0x41:
                    case 0x42:
                    case 0x43:
                    case 0x44:
                    case 0x45:
                    case 0x46:
                    case 0x47:
                    case 0x48:
                    case 0x49:
                    case 0x4A:
                    case 0x4B:
                    case 0x4C:
                    case 0x4D:
                    case 0x4E:
                    case 0x4F:
                    case 0x50:
                    case 0x51:
                    case 0x52:
                    case 0x53:
                    case 0x54:
                    case 0x55:
                    case 0x56:
                    case 0x57:
                    case 0x58:
                    case 0x59:
                    case 0x5A:
                    case 0x5B:
                    case 0x5C:
                    case 0x5D:
                    case 0x5E:
                    case 0x5F:
                    case 0x60:
                    case 0x61:
                    case 0x70:
                    case 0x71:
                    case 0x72:
                    case 0x73:
                    case 0x74:
                    case 0x75:
                    case 0x76:
                    case 0x77:
                    case 0x78:
                    case 0x79:
                    case 0x7A:
                    case 0x7B:
                    case 0x7C:
                    case 0x7D:
                    case 0x7E:
                    case 0x7F:              /* BND / NOP */
                    case 0x86:              /* XACQUIRE/XRELEASE XCHG */
                    case 0x87:              /* XACQUIRE/XRELEASE XCHG */
                    case 0x88:              /* XRELEASE MOV... */
                    case 0x89:              /* XRELEASE MOV... ignoring acquire thingy */
                    case 0x9C:
                    case 0xC3:              /* yup, repz ret is valid */
                    case 0xE8:
                    case 0xE9:
                    case 0xEB:
                    case 0xF0:              /* XACQUIRE/XRELEASE LOCK ... ignoring acquire thingy */
                        --ip;
                        NEXT;
                    case 0x90:              /* PAUSE */
                        NEXT;
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
                        tmp8u = 0;
                        tmp8u2 = 0;
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
                        if(R_ECX) cmp8(emu, tmp8u2, tmp8u);
                        break;
                    case 0xA7:              /* REP(N)Z CMPSD */
                        tmp32u2 = 0;
                        tmp32u3 = 0;
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
                        if(R_ECX) cmp32(emu, tmp32u2, tmp32u3);
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
                        tmp8u = 0;
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
                        if(R_ECX) cmp8(emu, R_AL, tmp8u);
                        break;
                    case 0xAF:              /* REP(N)Z SCASD */
                        tmp32u2 = 0;
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
                        if(R_ECX) cmp32(emu, R_EAX, tmp32u2);
                        break;
                    default:
                        goto _default;
                }
                R_ECX = tmp32u;
            }   // else(nextop==0x0F)
            NEXT;
        _0xF4:                      /* HLT */
            // this is a privilege opcode... should an error be called instead?
            sched_yield();
            STEP;
            NEXT;
        _0xF5:                      /* CMC */
            CHECK_FLAGS(emu);
            CONDITIONAL_SET_FLAG(!ACCESS_FLAG(F_CF), F_CF);
            NEXT;

        _0xF6:                      /* GRP3 Eb(,Ib) */
            nextop = F8;
            GET_EB;
            switch((nextop>>3)&7) {
                case 0: 
                case 1:                 /* TEST Eb,Ib */
                    tmp8u = F8;
                    test8(emu, EB->byte[0], tmp8u);
                    break;
                case 2:                 /* NOT Eb */
                    EB->byte[0] = not8(emu, EB->byte[0]);
                    break;
                case 3:                 /* NEG Eb */
                    EB->byte[0] = neg8(emu, EB->byte[0]);
                    break;
                case 4:                 /* MUL AL,Eb */
                    mul8(emu, EB->byte[0]);
                    break;
                case 5:                 /* IMUL AL,Eb */
                    imul8(emu, EB->byte[0]);
                    break;
                case 6:                 /* DIV Eb */
                    div8(emu, EB->byte[0]);
                    break;
                case 7:                 /* IDIV Eb */
                    idiv8(emu, EB->byte[0]);
                    break;
            }
            NEXT;
        _0xF7:                      /* GRP3 Ed(,Id) */
            nextop = F8;
            GET_ED;
            switch((nextop>>3)&7) {
                case 0: 
                case 1:                 /* TEST Ed,Id */
                    tmp32u = F32;
                    test32(emu, ED->dword[0], tmp32u);
                    break;
                case 2:                 /* NOT Ed */
                    ED->dword[0] = not32(emu, ED->dword[0]);
                    break;
                case 3:                 /* NEG Ed */
                    ED->dword[0] = neg32(emu, ED->dword[0]);
                    break;
                case 4:                 /* MUL EAX,Ed */
                    mul32_eax(emu, ED->dword[0]);
                    break;
                case 5:                 /* IMUL EAX,Ed */
                    imul32_eax(emu, ED->dword[0]);
                    break;
                case 6:                 /* DIV Ed */
                    div32(emu, ED->dword[0]);
                    break;
                case 7:                 /* IDIV Ed */
                    idiv32(emu, ED->dword[0]);
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
        _0xFA:                      /* CLI */
            CLEAR_FLAG(F_IF);   //not really handled...
            NEXT;
        _0xFB:                      /* STI */
            SET_FLAG(F_IF);
            NEXT;
        _0xFC:                      /* CLD */
            CLEAR_FLAG(F_DF);
            NEXT;
        _0xFD:                      /* STD */
            SET_FLAG(F_DF);
            NEXT;
        _0xFE:                      /* GRP 5 Eb */
            nextop = F8;
            tmp32u2 = ip;
            GET_EB;
            switch((nextop>>3)&7) {
                case 0:                 /* INC Eb */
                    EB->byte[0] = inc8(emu, EB->byte[0]);
                    break;
                case 1:                 /* DEC Eb */
                    EB->byte[0] = dec8(emu, EB->byte[0]);
                    break;
                default:
                    emu->old_ip = R_EIP;
                    R_EIP = ip = tmp32u2;
                    printf_log(LOG_NONE, "Illegal Opcode %p: %02X %02X %02X %02X\n", (void*)ip, opcode, nextop, PK(2), PK(3));
                    emu->quit=1;
                    emu->error |= ERR_ILLEGAL;
                    goto fini;
            }
            NEXT;
        _0xFF:                      /* GRP 5 Ed */
            nextop = F8;
            tmp32u2 = ip;
            GET_ED;
            switch((nextop>>3)&7) {
                case 0:                 /* INC Ed */
                    ED->dword[0] = inc32(emu, ED->dword[0]);
                    break;
                case 1:                 /* DEC Ed */
                    ED->dword[0] = dec32(emu, ED->dword[0]);
                    break;
                case 2:                 /* CALL NEAR Ed */
                    tmp32u = (uintptr_t)getAlternate((void*)ED->dword[0]);
                    Push(emu, ip);
                    ip = tmp32u;
                    STEP
                    break;
                case 3:                 /* CALL FAR Ed */
                    if(nextop>0xc0) {
                        emu->old_ip = R_EIP;
                        R_EIP = ip = tmp32u2;
                        printf_log(LOG_NONE, "Illegal Opcode %p: %02X %02X %02X %02X\n", (void*)ip, opcode, nextop, PK(2), PK(3));
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                        goto fini;
                    } else {
                        Push16(emu, R_CS);
                        Push(emu, ip);
                        ip = ED->dword[0];
                        R_CS = (ED+1)->word[0];
                        STEP
                    }
                    break;
                case 4:                 /* JMP NEAR Ed */
                    ip = (uintptr_t)getAlternate((void*)ED->dword[0]);
                    STEP
                    break;
                case 5:                 /* JMP FAR Ed */
                    if(nextop>0xc0) {
                        emu->old_ip = R_EIP;
                        R_EIP = ip = tmp32u2;
                        printf_log(LOG_NONE, "Illegal Opcode %p: 0x%02X 0x%02X %02X %02X\n", (void*)ip, opcode, nextop, PK(2), PK(3));
                        emu->quit=1;
                        emu->error |= ERR_ILLEGAL;
                        goto fini;
                    } else {
                        ip = ED->dword[0];
                        R_CS = (ED+1)->word[0];
                        STEP
                    }
                    break;
                case 6:                 /* Push Ed */
                    tmp32u = ED->dword[0];
                    Push(emu, tmp32u);  // avoid potential issue with push [esp+...]
                    break;
                default:
                    emu->old_ip = R_EIP;
                    R_EIP = ip = tmp32u2;
                    printf_log(LOG_NONE, "Illegal Opcode %p: %02X %02X %02X %02X %02X %02X\n",(void*)ip, opcode, nextop, PK(2), PK(3), PK(4), PK(5));
                    emu->quit=1;
                    emu->error |= ERR_ILLEGAL;
                    goto fini;
            }
            NEXT;

        _default:
            emu->old_ip = R_EIP;
            R_EIP = ip;
            UnimpOpcode(emu);
            goto fini;
#ifdef DYNAREC
stepout:
    emu->old_ip = R_EIP;
    R_EIP = ip;
    return 0;
#endif

fini:
//    PackFlags(emu);
    // fork handling
    if(emu->fork) {
        if(step)
            return 0;
        int forktype = emu->fork;
        emu->quit = 0;
        emu->fork = 0;
        emu = x86emu_fork(emu, forktype);
        goto x86emurun;
    }
    // setcontext handling
    else if(emu->uc_link) {
        emu->quit = 0;
        my_setcontext(emu, emu->uc_link);
        goto x86emurun;
    }
    return 0;
}
