#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include "debug.h"
#include "box86context.h"
#include "dynarec.h"
#include "emu/x86emu_private.h"
#include "tools/bridge_private.h"
#include "x86run.h"
#include "x86emu.h"
#include "box86stack.h"
#include "callback.h"
#include "emu/x86run_private.h"
#include "emu/x87emu_private.h"
#include "x86trace.h"
#include "signals.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"
#include "dynarec_arm_functions.h"
#include "bridge.h"
#include "arm_printer.h"
#include "custommem.h"

void arm_fstp(x86emu_t* emu, void* p)
{
    if(ST0.q!=STld(0).uref)
        D2LD(&ST0.d, p);
    else
        memcpy(p, &STld(0).ld, 10);
}

void arm_print_armreg(x86emu_t* emu, uintptr_t reg, uintptr_t n)
{
    dynarec_log(LOG_DEBUG, "R%d=0x%x (%d)\n", n, reg, reg);
}

void arm_f2xm1(x86emu_t* emu)
{
    ST0.d = exp2(ST0.d) - 1.0;
}
void arm_fyl2x(x86emu_t* emu)
{
    ST(1).d = log2(ST0.d)*ST(1).d;
}
void arm_ftan(x86emu_t* emu)
{
    ST0.d = tan(ST0.d);
    emu->sw.f.F87_C2 = 0;
}
void arm_fpatan(x86emu_t* emu)
{
    ST1.d = atan2(ST1.d, ST0.d);
}
void arm_fxtract(x86emu_t* emu)
{
    int tmp32s;
    if(isnan(ST1.d)) {
        ST0.d = ST1.d;
    } else if(isinf(ST1.d)) {
        ST0.d = ST1.d;
        ST1.d = INFINITY;
    } else if(ST1.d==0.0) {
        ST0.d = ST1.d;
        ST1.d = -INFINITY;
    } else {
        // LD80bits doesn't have implicit "1" bit, so need to adjust for that
        ST0.d = frexp(ST1.d, &tmp32s)*2;
        ST1.d = tmp32s-1;
    }
}
void arm_fprem(x86emu_t* emu)
{
    int e0, e1;
    int64_t ll;
    frexp(ST0.d, &e0);
    frexp(ST1.d, &e1);
    int32_t tmp32s = e0 - e1;
    if(tmp32s<64)
    {
        ll = (int64_t)floor(ST0.d/ST1.d);
        ST0.d = ST0.d - (ST1.d*ll);
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C1 = (ll&1)?1:0;
        emu->sw.f.F87_C3 = (ll&2)?1:0;
        emu->sw.f.F87_C0 = (ll&4)?1:0;
    } else {
        ll = (int64_t)(floor((ST0.d/ST1.d))/exp2(tmp32s - 32));
        ST0.d = ST0.d - ST1.d*ll*exp2(tmp32s - 32);
        emu->sw.f.F87_C2 = 1;
    }
}
void arm_frndint(x86emu_t* emu)
{
    ST0.d = fpu_round(emu, ST0.d);
}
void arm_fscale(x86emu_t* emu)
{
    if(ST0.d!=0.0)
        ST0.d *= exp2(trunc(ST1.d));
}
void arm_fbld(x86emu_t* emu, uint8_t* ed)
{
    fpu_fbld(emu, ed);
}

void arm_fild64(x86emu_t* emu, int64_t* ed)
{
    int64_t tmp;
    memcpy(&tmp, ed, sizeof(tmp));
    ST0.d = tmp;
    STll(0).sq = tmp;
    STll(0).sref = ST0.sq;
}

void arm_fbstp(x86emu_t* emu, uint8_t* ed)
{
    fpu_fbst(emu, ed);
}

int64_t arm_fist64_0(double d)
{
    int64_t tmp;
    if(isgreater(d, (double)(int64_t)0x7fffffffffffffffLL) || isless(d, -(double)(uint64_t)0x8000000000000000LL) || !isfinite(d))
        tmp = 0x8000000000000000LL;
    else
        tmp = nearbyint(d);
    return tmp;
}
int64_t arm_fist64_1(double d)
{
    int64_t tmp;
    if(isgreater(d, (double)(int64_t)0x7fffffffffffffffLL) || isless(d, -(double)(uint64_t)0x8000000000000000LL) || !isfinite(d))
        tmp = 0x8000000000000000LL;
    else
        tmp = floor(d);
    return tmp;
}
int64_t arm_fist64_2(double d)
{
    int64_t tmp;
    if(isgreater(d, (double)(int64_t)0x7fffffffffffffffLL) || isless(d, -(double)(uint64_t)0x8000000000000000LL) || !isfinite(d))
        tmp = 0x8000000000000000LL;
    else
        tmp = ceil(d);
    return tmp;
}
int64_t arm_fist64_3(double d)
{
    int64_t tmp;
    if(isgreater(d, (double)(int64_t)0x7fffffffffffffffLL) || isless(d, -(double)(uint64_t)0x8000000000000000LL) || !isfinite(d))
        tmp = 0x8000000000000000LL;
    else
        tmp = trunc(d);
    return tmp;
}

void arm_fld(x86emu_t* emu, uint8_t* ed)
{
    memcpy(&STld(0).ld, ed, 10);
    LD2D(&STld(0).ld, &ST(0).d);
    STld(0).uref = ST0.q;
}

void arm_ud(x86emu_t* emu)
{
    emu->test.test = 0;
    emit_signal(emu, SIGILL, (void*)R_EIP, 0);
}

void arm_priv(x86emu_t* emu)
{
    emu->test.test = 0;
    emit_signal(emu, SIGSEGV, (void*)R_EIP, 128);
}

void arm_singlestep(x86emu_t* emu)
{
    emu->test.test = 0;
    emit_signal(emu, SIGTRAP, (void*)R_EIP, 2);
}

void arm_fsave(x86emu_t* emu, uint8_t* ed)
{
    fpu_savenv(emu, (char*)ed, 0);

    uint8_t* p = ed;
    p += 28;
    for (int i=0; i<8; ++i) {
        D2LD(&ST(i).d, p);
        p+=10;
    }
}
void arm_frstor(x86emu_t* emu, uint8_t* ed)
{
    fpu_loadenv(emu, (char*)ed, 0);

    uint8_t* p = ed;
    p += 28;
    for (int i=0; i<8; ++i) {
        LD2D(p, &ST(i).d);
        p+=10;
    }

}

void arm_fprem1(x86emu_t* emu)
{
    int e0, e1;
    int64_t ll;
    frexp(ST0.d, &e0);
    frexp(ST1.d, &e1);
    int32_t tmp32s = e0 - e1;
    if(tmp32s<64)
    {
        ll = (int64_t)round(ST0.d/ST1.d);
        ST0.d = ST0.d - (ST1.d*ll);
        emu->sw.f.F87_C2 = 0;
        emu->sw.f.F87_C1 = (ll&1)?1:0;
        emu->sw.f.F87_C3 = (ll&2)?1:0;
        emu->sw.f.F87_C0 = (ll&4)?1:0;
    } else {
        ll = (int64_t)(trunc((ST0.d/ST1.d))/exp2(tmp32s - 32));
        ST0.d = ST0.d - ST1.d*ll*exp2(tmp32s - 32);
        emu->sw.f.F87_C2 = 1;
    }
}

static uint8_t ff_mult(uint8_t a, uint8_t b)
{
	int retval = 0;

	for(int i = 0; i < 8; i++) {
		if((b & 1) == 1)
			retval ^= a;

		if((a & 0x80)) {
			a <<= 1;
			a  ^= 0x1b;
		} else {
			a <<= 1;
		}

		b >>= 1;
	}

	return retval;
}

void arm_aesimc(x86emu_t* emu, int xmm)
{
    sse_regs_t eax1 = emu->xmm[xmm];

    for(int j=0; j<4; ++j) {
        emu->xmm[xmm].ub[0+j*4] = ff_mult(0x0E, eax1.ub[0+j*4]) ^ ff_mult(0x0B, eax1.ub[1+j*4]) ^ ff_mult(0x0D, eax1.ub[2+j*4]) ^ ff_mult(0x09, eax1.ub[3+j*4]);
        emu->xmm[xmm].ub[1+j*4] = ff_mult(0x09, eax1.ub[0+j*4]) ^ ff_mult(0x0E, eax1.ub[1+j*4]) ^ ff_mult(0x0B, eax1.ub[2+j*4]) ^ ff_mult(0x0D, eax1.ub[3+j*4]);
        emu->xmm[xmm].ub[2+j*4] = ff_mult(0x0D, eax1.ub[0+j*4]) ^ ff_mult(0x09, eax1.ub[1+j*4]) ^ ff_mult(0x0E, eax1.ub[2+j*4]) ^ ff_mult(0x0B, eax1.ub[3+j*4]);
        emu->xmm[xmm].ub[3+j*4] = ff_mult(0x0B, eax1.ub[0+j*4]) ^ ff_mult(0x0D, eax1.ub[1+j*4]) ^ ff_mult(0x09, eax1.ub[2+j*4]) ^ ff_mult(0x0E, eax1.ub[3+j*4]);
    }
}
void arm_aesmc(x86emu_t* emu, int xmm)
{
    sse_regs_t eax1 = emu->xmm[xmm];

    for(int j=0; j<4; ++j) {
        emu->xmm[xmm].ub[0+j*4] = ff_mult(0x02, eax1.ub[0+j*4]) ^ ff_mult(0x03, eax1.ub[1+j*4]) ^               eax1.ub[2+j*4]  ^               eax1.ub[3+j*4] ;
        emu->xmm[xmm].ub[1+j*4] =               eax1.ub[0+j*4]  ^ ff_mult(0x02, eax1.ub[1+j*4]) ^ ff_mult(0x03, eax1.ub[2+j*4]) ^               eax1.ub[3+j*4] ;
        emu->xmm[xmm].ub[2+j*4] =               eax1.ub[0+j*4]  ^               eax1.ub[1+j*4]  ^ ff_mult(0x02, eax1.ub[2+j*4]) ^ ff_mult(0x03, eax1.ub[3+j*4]);
        emu->xmm[xmm].ub[3+j*4] = ff_mult(0x03, eax1.ub[0+j*4]) ^               eax1.ub[1+j*4]  ^               eax1.ub[2+j*4]  ^ ff_mult(0x02, eax1.ub[3+j*4]);
    }
}
void arm_aesdlast(x86emu_t* emu, int xmm)
{
                            //   A0 B1 C2 D3 E4 F5 G6 H7 I8 J9 Ka Lb Mc Nd Oe Pf
                            //   A  N  K  H  E  B  O  L  I  F  C  P  M  J  G  D
    const uint8_t invshiftrows[] = {0,13,10, 7, 4, 1,14,11, 8, 5, 2,15,12, 9, 6, 3};
    const uint8_t invsubbytes[256] = {
        0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
        0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
        0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
        0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
        0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
        0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
        0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
        0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
        0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
        0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
        0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
        0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
        0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
        0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
        0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
        0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d,
    };

    sse_regs_t eax1;
    for(int i=0; i<16; ++i)
        eax1.ub[i] = emu->xmm[xmm].ub[invshiftrows[i]];
    //STATE ← InvSubBytes( STATE );
    for(int i=0; i<16; ++i)
        emu->xmm[xmm].ub[i] = invsubbytes[eax1.ub[i]];

}
static const uint8_t shiftrows[] = {0, 5,10,15, 4, 9,14, 3, 8,13, 2, 7,12, 1, 6,11};
static const uint8_t subbytes[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16,
};
void arm_aeselast(x86emu_t* emu, int xmm)
{
                            //   A0 B1 C2 D3 E4 F5 G6 H7 I8 J9 Ka Lb Mc Nd Oe Pf
                            //   A  F  K  P  E  J  O  D  I  N  C  H  M  B  G  L
    sse_regs_t eax1;
    for(int i=0; i<16; ++i)
        eax1.ub[i] = emu->xmm[xmm].ub[shiftrows[i]];
    //STATE ← SubBytes( STATE );
    for(int i=0; i<16; ++i)
        emu->xmm[xmm].ub[i] = subbytes[eax1.ub[i]];
}
void arm_aesd(x86emu_t* emu, int xmm)
{
    arm_aesdlast(emu, xmm);
    arm_aesimc(emu, xmm);
}
void arm_aese(x86emu_t* emu, int xmm)
{
    arm_aeselast(emu, xmm);
    arm_aesmc(emu, xmm);
}
void arm_aeskeygenassist(x86emu_t* emu, int gx, void* p, uint32_t u8)
{
    sse_regs_t *EX = ((uintptr_t)p>8)?((sse_regs_t*)p):&emu->xmm[(uintptr_t)p];
    sse_regs_t *GX = &emu->xmm[gx];
    for (int i = 4; i < 8; ++i)
        GX->ub[i] = subbytes[EX->ub[i]];
    for (int i = 12; i < 16; ++i)
        GX->ub[i] = subbytes[EX->ub[i]];
    GX->ud[0] = GX->ud[1];
    uint8_t tmp8u = GX->ub[4];
    GX->ud[1] = GX->ud[1] >> 8;
    GX->ub[7] = tmp8u;
    GX->ud[1] ^= u8;
    GX->ud[2] = GX->ud[3];
    tmp8u = GX->ub[12];
    GX->ud[3] = GX->ud[3] >> 8;
    GX->ub[15] = tmp8u;
    GX->ud[3] ^= u8;
}

void arm_pclmul(x86emu_t* emu, int gx, void* p, uint32_t u8)
{
    int ex = (((uintptr_t)p)<8)?((uintptr_t)p):-1;
    sse_regs_t *EX = (ex==-1)?((sse_regs_t*)p):&emu->xmm[ex];
    sse_regs_t *GX = &emu->xmm[gx];
    int g = (u8&1)?1:0;
    int e = (u8&0b10000)?1:0;
    uint64_t result[2] = {0};
    uint64_t op2 = EX->q[e];
    for (int i=0; i<64; ++i)
        if(GX->q[g]&(1LL<<i)) {
            result[0] ^= (EX->q[e]<<i);
            if(i)
                result[1] ^= (EX->q[e]>>(64-i));

        }
    GX->q[0] = result[0];
    GX->q[1] = result[1];
}

void arm_clflush(x86emu_t* emu, void* p)
{
    cleanDBFromAddressRange((uintptr_t)p, 8, 0);
}

// Get a FPU single scratch reg
int fpu_get_scratch_single(dynarec_arm_t* dyn)
{
    return dyn->n.fpu_scratch++;  // return an Sx
}
// Get a FPU double scratch reg
int fpu_get_scratch_double(dynarec_arm_t* dyn)
{
    int i = (dyn->n.fpu_scratch+1)&(~1);
    dyn->n.fpu_scratch = i+2;
    return i/2; // return a Dx
}
// Get a FPU quad scratch reg
int fpu_get_scratch_quad(dynarec_arm_t* dyn)
{
    if(dyn->n.fpu_scratch>4) {
        if(dyn->n.fpu_extra_qscratch) {
            dynarec_log(LOG_NONE, "Warning, Extra QScratch slot taken and need another one!\n");
        } else
            dyn->n.fpu_extra_qscratch = fpu_get_reg_quad(dyn, NEON_CACHE_SCR, 0);
        return dyn->n.fpu_extra_qscratch;
    }
    int i = (dyn->n.fpu_scratch+3)&(~3);
    dyn->n.fpu_scratch = i+4;
    return i/2; // Return a Dx, not a Qx
}
// Reset scratch regs counter
void fpu_reset_scratch(dynarec_arm_t* dyn)
{
    dyn->n.fpu_scratch = 0;
    if(dyn->n.fpu_extra_qscratch) {
        fpu_free_reg_quad(dyn, dyn->n.fpu_extra_qscratch);
        dyn->n.fpu_extra_qscratch = 0;
    }
}
// Get a FPU double reg
int fpu_get_reg_double(dynarec_arm_t* dyn, unsigned int t, unsigned int n)
{
    // TODO: check upper limit?
    int i=0;
    while (dyn->n.fpuused[i] && (i<24)) ++i;
    assert(i<24);

    dyn->n.fpuused[i] = 1;
    dyn->n.neoncache[i].n = n;
    dyn->n.neoncache[i].t = t;
    dyn->n.news |= (1<<i);
    return i+FPUFIRST; // return a Dx
}
// Free a FPU double reg
void fpu_free_reg_double(dynarec_arm_t* dyn, int reg)
{
    // TODO: check upper limit?
    int i=reg-FPUFIRST;
    dyn->n.fpuused[i] = 0;
    if(dyn->n.neoncache[i].t!=NEON_CACHE_ST_F && dyn->n.neoncache[i].t!=NEON_CACHE_ST_D)
        dyn->n.neoncache[i].v = 0;
}
// Get a FPU quad reg
int fpu_get_reg_quad(dynarec_arm_t* dyn, unsigned int t, unsigned int n)
{
    int i=0;
    while ((dyn->n.fpuused[i] || dyn->n.fpuused[i+1]) && (i<24)) i+=2;
    assert(i<24);
    dyn->n.fpuused[i] = dyn->n.fpuused[i+1] = 1;
    dyn->n.neoncache[i].t = t;
    dyn->n.neoncache[i].n = n;
    dyn->n.neoncache[i+1].t = t;
    dyn->n.neoncache[i+1].n = n;
    dyn->n.news |= (3<<i);
    return i+FPUFIRST; // Return a Dx, not a Qx
}
// Free a FPU quad reg
void fpu_free_reg_quad(dynarec_arm_t* dyn, int reg)
{
    int i=reg-FPUFIRST;
    dyn->n.fpuused[i] = dyn->n.fpuused[i+1] = 0;
    dyn->n.neoncache[i].v = 0;
    dyn->n.neoncache[i+1].v = 0;
}
// Reset fpu regs counter
static void fpu_reset_reg_neoncache(neoncache_t* n)
{
    n->fpu_reg = 0;
    for (int i=0; i<24; ++i) {
        n->fpuused[i]=0;
        n->neoncache[i].v = 0;
    }

}
void fpu_reset_reg(dynarec_arm_t* dyn)
{
    fpu_reset_reg_neoncache(&dyn->n);
}

int neoncache_get_st(dynarec_arm_t* dyn, int ninst, int a)
{
    if (dyn->insts[ninst].n.swapped) {
        if(dyn->insts[ninst].n.combined1 == a)
            a = dyn->insts[ninst].n.combined2;
        else if(dyn->insts[ninst].n.combined2 == a)
            a = dyn->insts[ninst].n.combined1;
    }
    for(int i=0; i<24; ++i)
        if((dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_F
         || dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_D)
         && dyn->insts[ninst].n.neoncache[i].n==a)
            return dyn->insts[ninst].n.neoncache[i].t;
    // not in the cache yet, so will be fetched...
    return NEON_CACHE_ST_D;
}

int neoncache_get_current_st(dynarec_arm_t* dyn, int ninst, int a)
{
    (void)ninst;
    if(!dyn->insts)
        return NEON_CACHE_ST_D;
    for(int i=0; i<24; ++i)
        if((dyn->n.neoncache[i].t==NEON_CACHE_ST_F
         || dyn->n.neoncache[i].t==NEON_CACHE_ST_D)
         && dyn->n.neoncache[i].n==a)
            return dyn->n.neoncache[i].t;
    // not in the cache yet, so will be fetched...
    return NEON_CACHE_ST_D;
}

int neoncache_get_st_f(dynarec_arm_t* dyn, int ninst, int a)
{
    /*if(a+dyn->insts[ninst].n.stack_next-st<0)
        // The STx has been pushed at the end of instructon, so stop going back
        return -1;*/
    for(int i=0; i<24; ++i)
        if(dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_F
         && dyn->insts[ninst].n.neoncache[i].n==a)
            return i;
    return -1;
} 
int neoncache_get_st_f_noback(dynarec_arm_t* dyn, int ninst, int a)
{
    for(int i=0; i<24; ++i)
        if(dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_F
         && dyn->insts[ninst].n.neoncache[i].n==a)
            return i;
    return -1;
} 
int neoncache_get_current_st_f(dynarec_arm_t* dyn, int a)
{
    for(int i=0; i<24; ++i)
        if(dyn->n.neoncache[i].t==NEON_CACHE_ST_F
         && dyn->n.neoncache[i].n==a)
            return i;
    return -1;
} 
static void neoncache_promote_double_forward(dynarec_arm_t* dyn, int ninst, int maxinst, int a);
static void neoncache_promote_double_internal(dynarec_arm_t* dyn, int ninst, int maxinst, int a);
static void neoncache_promote_double_combined(dynarec_arm_t* dyn, int ninst, int maxinst, int a)
{
    if(a == dyn->insts[ninst].n.combined1 || a == dyn->insts[ninst].n.combined2) {
        if(a == dyn->insts[ninst].n.combined1) {
            a = dyn->insts[ninst].n.combined2;
        } else 
            a = dyn->insts[ninst].n.combined1;
        int i = neoncache_get_st_f_noback(dyn, ninst, a);
        //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_combined, ninst=%d combined%c %d i=%d (stack:%d/%d)\n", ninst, (a == dyn->insts[ninst].n.combined2)?'2':'1', a ,i, dyn->insts[ninst].n.stack_push, -dyn->insts[ninst].n.stack_pop);
        if(i>=0) {
            dyn->insts[ninst].n.neoncache[i].t = NEON_CACHE_ST_D;
            if(!dyn->insts[ninst].n.barrier)
                neoncache_promote_double_internal(dyn, ninst-1, maxinst, a-dyn->insts[ninst].n.stack_push);
            // go forward is combined is not pop'd
            if(a-dyn->insts[ninst].n.stack_pop>=0)
                if(!dyn->insts[ninst+1].n.barrier)
                    neoncache_promote_double_forward(dyn, ninst+1, maxinst, a-dyn->insts[ninst].n.stack_pop);
        }
    }
}
static void neoncache_promote_double_internal(dynarec_arm_t* dyn, int ninst, int maxinst, int a)
{
    if(dyn->insts[ninst+1].n.barrier)
        return;
    while(ninst>=0) {
        a+=dyn->insts[ninst].n.stack_pop;    // adjust Stack depth: add pop'd ST (going backward)
        int i = neoncache_get_st_f(dyn, ninst, a);
        //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_internal, ninst=%d, a=%d st=%d:%d, i=%d\n", ninst, a, dyn->insts[ninst].n.stack, dyn->insts[ninst].n.stack_next, i);
        if(i<0) return;
        dyn->insts[ninst].n.neoncache[i].t = NEON_CACHE_ST_D;
        // check combined propagation too
        if(dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2) {
            if(dyn->insts[ninst].n.swapped) {
                //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_internal, ninst=%d swapped %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].n.combined1 ,dyn->insts[ninst].n.combined2, a, dyn->insts[ninst].n.stack);
                if (a==dyn->insts[ninst].n.combined1)
                    a = dyn->insts[ninst].n.combined2;
                else if (a==dyn->insts[ninst].n.combined2)
                    a = dyn->insts[ninst].n.combined1;
            } else {
                //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_internal, ninst=%d combined %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].n.combined1 ,dyn->insts[ninst].n.combined2, a, dyn->insts[ninst].n.stack);
                neoncache_promote_double_combined(dyn, ninst, maxinst, a);
            }
        }
        a-=dyn->insts[ninst].n.stack_push;  // // adjust Stack depth: remove push'd ST (going backward)
        --ninst;
        if(ninst<0 || a<0 || dyn->insts[ninst].n.barrier)
            return;
    }
}

static void neoncache_promote_double_forward(dynarec_arm_t* dyn, int ninst, int maxinst, int a)
{
    while((ninst!=-1) && (ninst<maxinst) && (a>=0)) {
        a+=dyn->insts[ninst].n.stack_push;  // // adjust Stack depth: add push'd ST (going forward)
        if((dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2) && dyn->insts[ninst].n.swapped) {
            //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_forward, ninst=%d swapped %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].n.combined1 ,dyn->insts[ninst].n.combined2, a, dyn->insts[ninst].n.stack);
            if (a==dyn->insts[ninst].n.combined1)
                a = dyn->insts[ninst].n.combined2;
            else if (a==dyn->insts[ninst].n.combined2)
                a = dyn->insts[ninst].n.combined1;
        }
        int i = neoncache_get_st_f_noback(dyn, ninst, a);
        //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_forward, ninst=%d, a=%d st=%d:%d(%d/%d), i=%d\n", ninst, a, dyn->insts[ninst].n.stack, dyn->insts[ninst].n.stack_next, dyn->insts[ninst].n.stack_push, -dyn->insts[ninst].n.stack_pop, i);
        if(i<0) return;
        dyn->insts[ninst].n.neoncache[i].t = NEON_CACHE_ST_D;
        // check combined propagation too
        if((dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2) && !dyn->insts[ninst].n.swapped) {
            //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double_forward, ninst=%d combined %d/%d vs %d with st %d\n", ninst, dyn->insts[ninst].n.combined1 ,dyn->insts[ninst].n.combined2, a, dyn->insts[ninst].n.stack);
            neoncache_promote_double_combined(dyn, ninst, maxinst, a);
        }
        a-=dyn->insts[ninst].n.stack_pop;    // adjust Stack depth: remove pop'd ST (going forward)
        if(dyn->insts[ninst].x86.has_next && !dyn->insts[ninst].n.barrier)
            ++ninst;
        else
            ninst=-1;
    }
    if(ninst==maxinst)
        neoncache_promote_double(dyn, ninst, a);
}

void neoncache_promote_double(dynarec_arm_t* dyn, int ninst, int a)
{
    int i = neoncache_get_current_st_f(dyn, a);
    //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double, ninst=%d a=%d st=%d i=%d\n", ninst, a, dyn->n.stack, i);
    if(i<0) return;
    dyn->n.neoncache[i].t = NEON_CACHE_ST_D;
    dyn->insts[ninst].n.neoncache[i].t = NEON_CACHE_ST_D;
    // check combined propagation too
    if(dyn->n.combined1 || dyn->n.combined2) {
        if(dyn->n.swapped) {
            //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double, ninst=%d swapped! %d/%d vs %d\n", ninst, dyn->n.combined1 ,dyn->n.combined2, a);
            if(dyn->n.combined1 == a)
                a = dyn->n.combined2;
            else if(dyn->n.combined2 == a)
                a = dyn->n.combined1;
        } else {
            //if(box86_dynarec_dump) dynarec_log(LOG_NONE, "neoncache_promote_double, ninst=%d combined! %d/%d vs %d\n", ninst, dyn->n.combined1 ,dyn->n.combined2, a);
            if(dyn->n.combined1 == a)
                neoncache_promote_double(dyn, ninst, dyn->n.combined2);
            else if(dyn->n.combined2 == a)
                neoncache_promote_double(dyn, ninst, dyn->n.combined1);
        }
    }
    a-=dyn->insts[ninst].n.stack_push;  // // adjust Stack depth: remove push'd ST (going backward)
    if(!ninst || a<0) return;
    neoncache_promote_double_internal(dyn, ninst-1, ninst, a);
}

int neoncache_combine_st(dynarec_arm_t* dyn, int ninst, int a, int b)
{
    dyn->n.combined1=a;
    dyn->n.combined2=b;
    if( neoncache_get_current_st(dyn, ninst, a)==NEON_CACHE_ST_F
     && neoncache_get_current_st(dyn, ninst, b)==NEON_CACHE_ST_F )
        return NEON_CACHE_ST_F;
    return NEON_CACHE_ST_D;
}

int isPred(dynarec_arm_t* dyn, int ninst, int pred) {
    for(int i=0; i<dyn->insts[ninst].pred_sz; ++i)
        if(dyn->insts[ninst].pred[i]==pred)
            return pred;
    return -1;
}
int getNominalPred(dynarec_arm_t* dyn, int ninst) {
    if((ninst<=0) || !dyn->insts[ninst].pred_sz)
        return -1;
    if(isPred(dyn, ninst, ninst-1)!=-1)
        return ninst-1;
    return dyn->insts[ninst].pred[0];
}

static int fpuCacheNeedsTransform(dynarec_arm_t* dyn, int ninst) {
    int i2 = dyn->insts[ninst].x86.jmp_insts;
    if(i2<0)
        return 1;
    if((dyn->insts[i2].x86.barrier&BARRIER_FLOAT))
        return ((dyn->insts[ninst].x86.barrier&BARRIER_FLOAT))?0:1; // if the barrier as already been apply, no transform needed
    int ret = 0;
    if(!i2) { // just purge
        if(dyn->insts[ninst].n.stack_next)  {
            return 1;
        }
        for(int i=0; i<24 && !ret; ++i)
            if(dyn->insts[ninst].n.neoncache[i].v) {       // there is something at ninst for i
                if(!(
                (dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_F || dyn->insts[ninst].n.neoncache[i].t==NEON_CACHE_ST_D)
                && dyn->insts[ninst].n.neoncache[i].n<dyn->insts[ninst].n.stack_pop))
                    ret = 1;
            }
        return ret;
    }
    // Check if ninst can be compatible to i2
    if(dyn->insts[ninst].n.stack_next != dyn->insts[i2].n.stack-dyn->insts[i2].n.stack_push) {
        return 1;
    }
    neoncache_t cache_i2 = dyn->insts[i2].n;
    neoncacheUnwind(&cache_i2);

    for(int i=0; i<24; ++i) {
        if(dyn->insts[ninst].n.neoncache[i].v) {       // there is something at ninst for i
            if(!cache_i2.neoncache[i].v) {    // but there is nothing at i2 for i
                ret = 1;
            } else if(dyn->insts[ninst].n.neoncache[i].v!=cache_i2.neoncache[i].v) {  // there is something different
                if(dyn->insts[ninst].n.neoncache[i].n!=cache_i2.neoncache[i].n) {   // not the same x86 reg
                    ret = 1;
                }
                else if(dyn->insts[ninst].n.neoncache[i].t == NEON_CACHE_XMMR && cache_i2.neoncache[i].t == NEON_CACHE_XMMW)
                    {/* nothing */ }
                else
                    ret = 1;
            }
        } else if(cache_i2.neoncache[i].v)
            ret = 1;
    }
    return ret;
}

void neoncacheUnwind(neoncache_t* cache)
{
    if(cache->swapped) {
        // unswap
        int a = -1; 
        int b = -1;
        // in neoncache
        for(int j=0; j<24 && ((a==-1) || (b==-1)); ++j)
            if((cache->neoncache[j].t == NEON_CACHE_ST_D || cache->neoncache[j].t == NEON_CACHE_ST_F)) {
                if(cache->neoncache[j].n == cache->combined1)
                    a = j;
                else if(cache->neoncache[j].n == cache->combined2)
                    b = j;
            }
        if(a!=-1 && b!=-1) {
            int tmp = cache->neoncache[a].n;
            cache->neoncache[a].n = cache->neoncache[b].n;
            cache->neoncache[b].n = tmp;
        }
        cache->swapped = 0;
        cache->combined1 = cache->combined2 = 0;
    }
    if(cache->news) {
        // remove the newly created neoncache
        for(int i=0; i<24; ++i)
            if(cache->news&(1<<i))
                cache->neoncache[i].v = 0;
        cache->news = 0;
    }
    if(cache->stack_push) {
        // unpush
        for(int j=0; j<24; ++j) {
            if((cache->neoncache[j].t == NEON_CACHE_ST_D || cache->neoncache[j].t == NEON_CACHE_ST_F)) {
                if(cache->neoncache[j].n<cache->stack_push)
                    cache->neoncache[j].v = 0;
                else
                    cache->neoncache[j].n-=cache->stack_push;
            }
        }
        cache->x87stack-=cache->stack_push;
        cache->tags>>=(cache->stack_push*2);
        cache->stack-=cache->stack_push;
        if(cache->pushed>=cache->stack_push)
            cache->pushed-=cache->stack_push;
        else
            cache->pushed = 0;
        cache->stack_push = 0;
    }
    cache->x87stack+=cache->stack_pop;
    cache->stack_next = cache->stack;
    if(cache->stack_pop) {
        if(cache->poped>=cache->stack_pop)
            cache->poped-=cache->stack_pop;
        else
            cache->poped = 0;
        cache->tags<<=(cache->stack_pop*2);
    }
    cache->stack_pop = 0;
    cache->barrier = 0;
    // And now, rebuild the x87cache info with neoncache
    cache->mmxcount = 0;
    cache->fpu_scratch = 0;
    cache->fpu_extra_qscratch = 0;
    cache->fpu_reg = 0;
    for(int i=0; i<8; ++i) {
        cache->x87cache[i] = -1;
        cache->mmxcache[i] = -1;
        cache->ssecache[i].v = -1;
        cache->x87reg[i] = 0;
    }
    int x87reg = 0;
    for(int i=0; i<24; ++i) {
        if(cache->neoncache[i].v) {
            cache->fpuused[i] = 1;
            switch (cache->neoncache[i].t) {
                case NEON_CACHE_MM:
                    cache->mmxcache[cache->neoncache[i].n] = i+FPUFIRST;
                    ++cache->mmxcount;
                    ++cache->fpu_reg;
                    break;
                case NEON_CACHE_XMMR:
                case NEON_CACHE_XMMW:
                    cache->ssecache[cache->neoncache[i].n].reg = i+FPUFIRST;
                    cache->ssecache[cache->neoncache[i].n].write = (cache->neoncache[i].t==NEON_CACHE_XMMW)?1:0;
                    cache->neoncache[i] = cache->neoncache[i+1];
                    ++cache->fpu_reg;
                    ++i;    // next next
                    break;
                case NEON_CACHE_ST_F:
                case NEON_CACHE_ST_D:
                    cache->x87cache[x87reg] = cache->neoncache[i].n;
                    cache->x87reg[x87reg] = i+FPUFIRST;
                    ++x87reg;
                    ++cache->fpu_reg;
                    break;
                case NEON_CACHE_SCR:
                    cache->fpuused[i] = 0;
                    cache->neoncache[i].v = 0;
                    break;
            }
        } else {
            cache->fpuused[i] = 0;
        }
    }
}

#define F8      *(uint8_t*)(addr++)
#define F32     *(uint32_t*)(addr+=4, addr-4)
// Get if ED will have the correct parity. Not emiting anything. Parity is 2 for DWORD or 3 for QWORD
int getedparity(dynarec_arm_t* dyn, int ninst, uintptr_t addr, uint8_t nextop, int parity)
{

    uint32_t tested = (1<<parity)-1;
    if((nextop&0xC0)==0xC0)
        return 0;   // direct register, no parity...
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            int sib_reg = (sib>>3)&7;
            if((sib&0x7)==5) {
                uint32_t tmp = F32;
                if (sib_reg!=4) {
                    // if XXXXXX+reg<<N then check parity of XXXXX and N should be enough
                    return ((tmp&tested)==0 && (sib>>6)>=parity)?1:0;
                } else {
                    // just a constant...
                    return (tmp&tested)?0:1;
                }
            } else {
                if(sib_reg==4 && parity<3)
                    return 0;   // simple [reg]
                // don't try [reg1 + reg2<<N], unless reg1 is ESP
                return ((sib&0x7)==4 && (sib>>6)>=parity)?1:0;
            }
        } else if((nextop&7)==5) {
            uint32_t tmp = F32;
            return (tmp&tested)?0:1;
        } else {
            return 0;
        }
    } else {
        return 0; //Form [reg1 + reg2<<N + XXXXXX]
    }
}

// Do the GETED, but don't emit anything...
uintptr_t fakeed(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop) 
{
    if((nextop&0xC0)==0xC0)
        return addr;
    if(!(nextop&0xC0)) {
        if((nextop&7)==4) {
            uint8_t sib = F8;
            if((sib&0x7)==5) {
                addr+=4;
            }
        } else if((nextop&7)==5) {
            addr+=4;
        }
    } else {
        if((nextop&7)==4) {
            ++addr;
        }
        if(nextop&0x80) {
            addr+=4;
        } else {
            ++addr;
        }
    }
    return addr;
}
// return Ib on a mod/rm opcode without emiting anything
uint8_t geted_ib(dynarec_arm_t* dyn, uintptr_t addr, int ninst, uint8_t nextop)
{
    addr = fakeed(dyn, addr, ninst, nextop);
    return F8;
}
#undef F8
#undef F32

int isNativeCall(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t* calladdress, int* retn)
{
#define PK(a)       *(uint8_t*)(addr+a)
#define PK32(a)     *(uint32_t*)(addr+a)

    if(!addr || !getProtection(addr))
        return 0;
    if(PK(0)==0xff && PK(1)==0x25) {  // absolute jump, maybe the GOT
        uintptr_t a1 = (PK32(2));   // need to add a check to see if the address is from the GOT !
        addr = (uintptr_t)getAlternate(*(void**)a1); 
    }
    if(addr<0x10000 || !getProtection(addr))    // too low, that is suspicious
        return 0;
    onebridge_t *b = (onebridge_t*)(addr);
    if(b->CC==0xCC && b->S=='S' && b->C=='C' && b->w!=(wrapper_t)0 && b->f!=(uintptr_t)PltResolver) {
        // found !
        if(retn) *retn = (b->C3==0xC2)?b->N:0;
        if(calladdress) *calladdress = addr+1;
        return 1;
    }
    return 0;
#undef PK32
#undef PK
}

const char* getCacheName(int t, int n)
{
    static char buff[20];
    switch(t) {
        case NEON_CACHE_ST_D: sprintf(buff, "ST%d", n); break;
        case NEON_CACHE_ST_F: sprintf(buff, "st%d", n); break;
        case NEON_CACHE_MM: sprintf(buff, "MM%d", n); break;
        case NEON_CACHE_XMMW: sprintf(buff, "XMM%d", n); break;
        case NEON_CACHE_XMMR: sprintf(buff, "xmm%d", n); break;
        case NEON_CACHE_SCR: sprintf(buff, "Scratch"); break;
        case NEON_CACHE_NONE: buff[0]='\0'; break;
    }
    return buff;
}

static int flagsCacheNeedsTransform(dynarec_arm_t* dyn, int ninst) {
    int jmp = dyn->insts[ninst].x86.jmp_insts;
    if(jmp<0)
        return 0;
    if(dyn->insts[ninst].f_exit.dfnone)  // flags are fully known, nothing we can do more
        return 0;
    if(dyn->insts[jmp].f_entry.dfnone && !dyn->insts[ninst].f_exit.dfnone)
        return 1;
    switch (dyn->insts[jmp].f_entry.pending) {
        case SF_UNKNOWN: return 0;
        case SF_SET:
            if(dyn->insts[ninst].f_exit.pending!=SF_SET && dyn->insts[ninst].f_exit.pending!=SF_SET_PENDING)
                return 1;
            else
                return 0;
        case SF_SET_PENDING:
            if(dyn->insts[ninst].f_exit.pending==SF_SET_PENDING)
                return 0;
            return 1;
        case SF_PENDING:
            if(dyn->insts[ninst].f_exit.pending==SF_PENDING || dyn->insts[ninst].f_exit.pending==SF_SET_PENDING)
                return 0;
            return (dyn->insts[jmp].f_entry.dfnone  == dyn->insts[ninst].f_exit.dfnone)?0:1;
    }
    return 0;
}

int CacheNeedsTransform(dynarec_arm_t* dyn, int ninst) {
    int ret = 0;
    if (fpuCacheNeedsTransform(dyn, ninst)) ret|=1;
    if (flagsCacheNeedsTransform(dyn, ninst)) ret|=2;
    return ret;
}

void inst_name_pass3(dynarec_arm_t* dyn, int ninst, const char* name)
{
    if(box86_dynarec_dump) {
        printf_x86_instruction(my_context->dec, &dyn->insts[ninst].x86, name);
        dynarec_log(LOG_NONE, "%s%p: %d emited opcodes, inst=%d, barrier=%d state=%d/%d(%d), %s=%X/%X, use=%X, need=%X/%X sm=%d/%d",
            (box86_dynarec_dump>1)?"\e[32m":"",
            (void*)(dyn->arm_start+dyn->insts[ninst].address),
            dyn->insts[ninst].size/4,
            ninst,
            dyn->insts[ninst].x86.barrier,
            dyn->insts[ninst].x86.state_flags,
            dyn->f.pending,
            dyn->f.dfnone,
            dyn->insts[ninst].x86.may_set?"may":"set",
            dyn->insts[ninst].x86.set_flags,
            dyn->insts[ninst].x86.gen_flags,
            dyn->insts[ninst].x86.use_flags,
            dyn->insts[ninst].x86.need_before,
            dyn->insts[ninst].x86.need_after,
            dyn->smread, dyn->smwrite);
        if(dyn->insts[ninst].pred_sz) {
            dynarec_log(LOG_NONE, ", pred=");
            for(int ii=0; ii<dyn->insts[ninst].pred_sz; ++ii)
                dynarec_log(LOG_NONE, "%s%d", ii?"/":"", dyn->insts[ninst].pred[ii]);
        }
        if(dyn->insts[ninst].x86.jmp && dyn->insts[ninst].x86.jmp_insts>=0)
            dynarec_log(LOG_NONE, ", jmp=%d", dyn->insts[ninst].x86.jmp_insts);
        if(dyn->insts[ninst].x86.jmp && dyn->insts[ninst].x86.jmp_insts==-1)
            dynarec_log(LOG_NONE, ", jmp=out");
        for(int ii=0; ii<24; ++ii) {
            switch(dyn->insts[ninst].n.neoncache[ii].t) {
                case NEON_CACHE_ST_D: dynarec_log(LOG_NONE, " D%d:%s", ii+8, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
                case NEON_CACHE_ST_F: dynarec_log(LOG_NONE, " S%d:%s", (ii+8)*2, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
                case NEON_CACHE_MM: dynarec_log(LOG_NONE, " D%d:%s", ii+8, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
                case NEON_CACHE_XMMW: dynarec_log(LOG_NONE, " Q%d:%s", (ii+8)/2, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); ++ii; break;
                case NEON_CACHE_XMMR: dynarec_log(LOG_NONE, " Q%d:%s", (ii+8)/2, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); ++ii; break;
                case NEON_CACHE_SCR: dynarec_log(LOG_NONE, " D%d:%s", ii+8, getCacheName(dyn->insts[ninst].n.neoncache[ii].t, dyn->insts[ninst].n.neoncache[ii].n)); break;
                case NEON_CACHE_NONE:
                default:    break;
            }
        }
        if(dyn->n.stack || dyn->insts[ninst].n.stack_next || dyn->insts[ninst].n.x87stack)
            dynarec_log(LOG_NONE, " X87:%d/%d(+%d/-%d)%d", dyn->n.stack, dyn->insts[ninst].n.stack_next, dyn->insts[ninst].n.stack_push, dyn->insts[ninst].n.stack_pop, dyn->insts[ninst].n.x87stack);
        if(dyn->insts[ninst].n.combined1 || dyn->insts[ninst].n.combined2)
            dynarec_log(LOG_NONE, " %s:%d/%d", dyn->insts[ninst].n.swapped?"SWP":"CMB", dyn->insts[ninst].n.combined1, dyn->insts[ninst].n.combined2);
        dynarec_log(LOG_NONE, "%s\n", (box86_dynarec_dump>1)?"\e[m":"");
    }
}

void print_opcode(dynarec_arm_t* dyn, int ninst, uint32_t opcode)
{
    dynarec_log(LOG_NONE, "\t%08x\t%s\n", opcode, arm_print(opcode));
}

static void x87_reset(neoncache_t* n)
{
    for (int i=0; i<8; ++i)
        n->x87cache[i] = -1;
    n->x87stack = 0;
    n->stack = 0;
    n->stack_next = 0;
    n->stack_pop = 0;
    n->stack_push = 0;
    n->combined1 = n->combined2 = 0;
    n->swapped = 0;
    n->barrier = 0;
    n->pushed = 0;
    n->poped = 0;

    for(int i=0; i<24; ++i)
        if(n->neoncache[i].t == NEON_CACHE_ST_F || n->neoncache[i].t == NEON_CACHE_ST_D)
            n->neoncache[i].v = 0;
}
static void mmx_reset(neoncache_t* n)
{
    n->mmxcount = 0;
    for (int i=0; i<8; ++i)
        n->mmxcache[i] = -1;
}
static void sse_reset(neoncache_t* n)
{
    for (int i=0; i<8; ++i)
        n->ssecache[i].v = -1;
}


void fpu_reset(dynarec_arm_t* dyn)
{
    x87_reset(&dyn->n);
    mmx_reset(&dyn->n);
    sse_reset(&dyn->n);
    fpu_reset_reg(dyn);
}

void fpu_reset_ninst(dynarec_arm_t* dyn, int ninst)
{
    x87_reset(&dyn->insts[ninst].n);
    mmx_reset(&dyn->insts[ninst].n);
    sse_reset(&dyn->insts[ninst].n);
    fpu_reset_reg_neoncache(&dyn->insts[ninst].n);
}

int fpu_is_st_freed(dynarec_arm_t* dyn, int ninst, int st)
{
    return (dyn->n.tags&(0b11<<(st*2)))?1:0;
}