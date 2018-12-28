#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "wrapper.h"
#include "x86emu_private.h"
#include "x87emu_private.h"
#include "regs.h"

// the stack as the return address 1st thing, then the args...
#define stack(n) (R_ESP+4+n)
#define i8(n)   *(int8_t*)stack(n)
#define i16(n)  *(int16_t*)stack(n)
#define i32(n)  *(int32_t*)stack(n)
#define u8(n)   *(uint8_t*)stack(n)
#define u16(n)  *(uint16_t*)stack(n)
#define u32(n)  *(uint32_t*)stack(n)
#define p(n)    *(void**)stack(n)
// to check vvvv
#define i64(n)  *(int64_t*)stack(n)
#define u64(n)  *(uint64_t*)stack(n)
//          ^^^^
#define f32(n)  *(float*)stack(n)
#define d64(n)  *(double*)stack(n)
#define D80(n)  *(long double*)stack(n)

// void...
typedef void        (*vFv_t)();
typedef void        (*vFi_t)(int32_t);
typedef void        (*vFp_t)(void*);
typedef void        (*vFu_t)(uint32_t);
typedef void        (*vFE_t)(x86emu_t*);
typedef void        (*vFpp_t)(void*, void*);
typedef void        (*vFpii_t)(void*, int32_t, int32_t);

// uint8...
typedef uint8_t     (*CFui_t)(uint32_t, int32_t);

// int32...
typedef int32_t     (*iFv_t)();
typedef int32_t     (*iFi_t)(int32_t);
typedef int32_t     (*iFp_t)(void*);
typedef int32_t     (*iFu_t)(uint32_t);
typedef int32_t     (*iFEp_t)(x86emu_t*, void*);
typedef int32_t     (*iFii_t)(int32_t, int32_t);
typedef int32_t     (*iFip_t)(int32_t, void*);
typedef int32_t     (*iFpi_t)(void*, int32_t);
typedef int32_t     (*iFpp_t)(void*, void*);
typedef int32_t     (*iFpu_t)(void*, uint32_t);
typedef int32_t     (*iFui_t)(uint32_t, int32_t);
typedef int32_t     (*iFup_t)(uint32_t, void*);
typedef int32_t     (*iFuu_t)(uint32_t, uint32_t);
typedef int32_t     (*iFipp_t)(int32_t, void*, void*);
typedef int32_t     (*iFpii_t)(void*, int32_t, int32_t);
typedef int32_t     (*iFppi_t)(void*, void*, int32_t);
typedef int32_t     (*iFppu_t)(void*, void*, uint32_t);
typedef int32_t     (*iFpip_t)(void*, int32_t, void*);
typedef int32_t     (*iFppp_t)(void*, void*, void*);
typedef int32_t     (*iFEpp_t)(x86emu_t*, void*, void*);
typedef int32_t     (*iFipii_t)(int32_t, void*, int32_t, int32_t);
typedef int32_t     (*iFiWii_t)(int32_t, uint16_t, int32_t, int32_t);
typedef int32_t     (*iFpipp_t)(void*, int32_t, void*, void*);
typedef int32_t     (*iFpppi_t)(void*, void*, void*, int32_t);
typedef int32_t     (*iFpuup_t)(void*, uint32_t, uint32_t, void*);
typedef int32_t     (*iFuipp_t)(uint32_t, int32_t, void*, void*);
typedef int32_t     (*iFEpipp_t)(x86emu_t*, void*, int32_t, void*, void*);
typedef int32_t     (*iFEpppp_t)(x86emu_t*, void*, void*, void*, void*);
typedef int32_t     (*iFipiii_t)(int32_t, void*, int32_t, int32_t, int32_t);
typedef int32_t     (*iFipppi_t)(int32_t, void*, void*, void*, int32_t);
typedef int32_t     (*iFpppii_t)(void*, void*, void*, int32_t, int32_t);
typedef int32_t     (*iFppppp_t)(void*, void*, void*, void*, void*);
typedef int32_t     (*iFpiuuuu_t)(void*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef int32_t     (*iFppuiiu_t)(void*, void*, uint32_t, int32_t, int32_t, uint32_t);
typedef int32_t     (*iFppiiuui_t)(void*, void*, int32_t, int32_t, uint32_t, uint32_t, int32_t);
typedef int32_t     (*iFEpippppp_t)(x86emu_t*, void*, int32_t, void*, void*, void*, void*, void*);
typedef int32_t     (*iFppiuiippu_t)(void*, void*, int32_t, uint32_t, int32_t, int32_t, void*, void*, uint32_t);
typedef int32_t     (*iFppppiiiiuu_t)(void*, void*, void*, void*, int32_t, int32_t, int32_t, int32_t, uint32_t, uint32_t);
typedef int32_t     (*iFpppiiipppppp_t)(void*, void*, void*, int32_t, int32_t, int32_t, void*, void*, void*, void*, void*, void*);

// uint32....
typedef uint32_t    (*uFv_t)();
typedef uint32_t    (*uFE_t)(x86emu_t*);
typedef uint32_t    (*uFp_t)(void*);
typedef uint32_t    (*uFpp_t)(void*, void*);
typedef uint32_t    (*uFpuup_t)(void*, uint32_t, uint32_t, void*);
typedef uint32_t    (*uFpupp_t)(void*, uint32_t, void*, void*);
typedef uint32_t    (*uFppiip_t)(void*, void*, int32_t, int32_t, void*);
typedef uint32_t    (*uFpuppu_t)(void*, uint32_t, void*, void*, uint32_t);

// void*....
typedef void*       (*pFv_t)();
typedef void*       (*pFE_t)(x86emu_t*);
typedef void*       (*pFp_t)(void*);
typedef void*       (*pFEp_t)(x86emu_t*, void*);
typedef void*       (*pFu_t)(uint32_t);
typedef void*       (*pFip_t)(int32_t, void*);
typedef void*       (*pFpi_t)(void*, int32_t);
typedef void*       (*pFpp_t)(void*, void*);
typedef void*       (*pFuu_t)(uint32_t, uint32_t);
typedef void*       (*pFEpi_t)(x86emu_t*, void*, int32_t);
typedef void*       (*pFEpp_t)(x86emu_t*, void*, void*);
typedef void*       (*pFppi_t)(void*, void*, int32_t);
typedef void*       (*pFppu_t)(void*, void*, uint32_t);
typedef void*       (*pFppp_t)(void*, void*, void*);
typedef void*       (*pFEppp_t)(x86emu_t*, void*, void*, void*);
typedef void*       (*pFpippp_t)(void*, int32_t, void*, void*, void*);
typedef void*       (*pFpiiiiu_t)(void*, int32_t, int32_t, int32_t, int32_t, uint32_t);
typedef void*       (*pFppiiuuui_t)(void*, void*, int32_t, int32_t, uint32_t, uint32_t, uint32_t, int32_t);
typedef void*       (*pFppiiuuuipii_t)(void*, void*, int32_t, int32_t, uint32_t, uint32_t, uint32_t, int32_t, void*, int32_t, int32_t);

// float....
typedef float       (*fFf_t)(float);
typedef float       (*fFff_t)(float, float);
typedef float       (*fFppu_t)(void*, void*, uint32_t);

// double....
typedef double      (*dFd_t)(double);
typedef double      (*dFdd_t)(double, double);
typedef double      (*dFppu_t)(void*, void*, uint32_t);

// long double....
typedef long double (*DFppu_t)(void*, void*, uint32_t);

#define DEF(A) A f = (A)fnc

#define GO(N, ...) GOW(N, N##_t, __VA_ARGS__)
// void...
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); f(__VA_ARGS__); }
#include "wrapper_v.h"

// uint8....
#undef GOW
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); R_EAX=f(__VA_ARGS__); }
#include "wrapper_u8.h"

// int32....
#undef GOW
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); R_EAX=(uint32_t)f(__VA_ARGS__); }
#include "wrapper_i.h"

// uint32....
#undef GOW
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); R_EAX=f(__VA_ARGS__); }
#include "wrapper_u.h"

// void*....
#undef GOW
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); R_EAX=(uintptr_t)f(__VA_ARGS__); }
#include "wrapper_p.h"

// float
#undef GOW
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); float fl=f(__VA_ARGS__); fpu_do_push(emu); ST0.d = fl;}
#include "wrapper_f.h"

// double
#undef GOW
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); double db=f(__VA_ARGS__); fpu_do_push(emu); ST0.d = db;}
#include "wrapper_d.h"

// long double
#undef GOW
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); long double ld=f(__VA_ARGS__); fpu_do_push(emu); ST0.d = ld;}
#include "wrapper_ld.h"

#undef GOW
#undef GO