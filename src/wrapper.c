#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "wrapper.h"
#include "x86emu_private.h"
#include "regs.h"

// the stack as the return address 1st thing, then the args...
#define stack(n) (R_ESP+4+n)
#define i32(n)  *(int32_t*)stack(n)
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
typedef void        (*vFp_t)(void*);
typedef void        (*vFE_t)(x86emu_t*);

// int32...
typedef int32_t     (*iFv_t)();
typedef uint32_t    (*uFv_t)();
typedef int32_t     (*iFi_t)(int32_t);
typedef int32_t     (*iFp_t)(void*);
typedef int32_t     (*iFu_t)(uint32_t);
typedef int32_t     (*iFpp_t)(void*, void*);
typedef int32_t     (*iFii_t)(int32_t, int32_t);
typedef int32_t     (*iFip_t)(int32_t, void*);
typedef int32_t     (*iFuu_t)(uint32_t, uint32_t);
typedef int32_t     (*iFup_t)(uint32_t, void*);
typedef int32_t     (*iFppi_t)(void*, void*, int32_t);
typedef int32_t     (*iFpip_t)(void*, int32_t, void*);
typedef int32_t     (*iFipp_t)(int, void*, void*);
typedef int32_t     (*iFppp_t)(void*, void*, void*);
typedef int32_t     (*iFpuup_t)(void*, uint32_t, uint32_t, void*);
typedef int32_t     (*iFuipp_t)(uint32_t, int32_t, void*, void*);
typedef int32_t     (*iFEpppp_t)(x86emu_t*, void*, void*, void*, void*);
typedef int32_t     (*iFEpippppp_t)(x86emu_t*, void*, int32_t, void*, void*, void*, void*, void*);

// uint32....
typedef uint32_t    (*uFE_t)(x86emu_t*);
typedef uint32_t    (*uFp_t)(void*);
typedef uint32_t    (*uFpupp_t)(void*, uint32_t, void*, void*);

// void*....
typedef void*       (*pFv_t)();
typedef void        (*vFi_t)(int32_t);
typedef void*       (*pFp_t)(void*);
typedef void*       (*pFpp_t)(void*, void*);
typedef void*       (*pFuu_t)(uint32_t, uint32_t);
typedef void*       (*pFppp_t)(void*, void*, void*);

#define DEF(A) A f = (A)fnc

#define GO(N, ...) GOW(N, N##_t, __VA_ARGS__)
// void...
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); f(__VA_ARGS__); }
GO(vFv, )
GO(vFp, p(0))
GO(vFE, emu)
GO(vFi, i32(0))

// int32....
#undef GOW
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); R_EAX=(uint32_t)f(__VA_ARGS__); }
GO(iFv, )
GO(iFpp, p(0), p(4))
GO(iFppi, p(0), p(4), i32(8))
GO(iFi, i32(0))
GO(iFp, p(0))
GO(iFu, u32(0))
GO(iFii, i32(0), i32(4))
GO(iFip, i32(0), p(4))
GO(iFuu, u32(0), u32(4))
GO(iFup, u32(0), p(4))
GOW(iFpV, iFpp_t, p(0), (void*)stack(4))
GO(iFpip, p(0), i32(4), p(8))
GOW(iF1pV, iFipp_t, 1, p(0), (void*)stack(4))
GO(iFuipp, u32(0), i32(4), p(8), p(12))
GO(iFpuup, p(0), u32(4), u32(8), p(12))
GOW(iFopV, iFppp_t, (void*)stdout, p(0), (void*)stack(4))
GOW(iFvopV, iFppp_t, (void*)stdout, p(4), (void*)stack(8))
GO(iFEpppp, emu, p(0), p(4), p(8), p(12))
GO(iFEpippppp, emu, p(0), i32(4), p(8), p(12), p(16), p(20), p(24))

// uint32....
#undef GOW
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); R_EAX=f(__VA_ARGS__); }
GO(uFv, )
GO(uFE, emu)
GO(uFp, p(0))
GO(uFpupp, p(0), u32(4), p(8), p(12))

// void*....
#undef GOW
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); R_EAX=(uintptr_t)f(__VA_ARGS__); }
GO(pFv, )
GO(pFp, p(0))
GO(pFuu, u32(0), u32(4))
GO(pFpp, p(0), p(4))
GO(pFppp, p(0), p(4), p(8))
