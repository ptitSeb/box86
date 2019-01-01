#ifndef __WRAPPER_H_
#define __WRAPPER_H_
#include <stdint.h>

typedef struct x86emu_s x86emu_t;

// the generic wrapper pointer functions
typedef void (*wrapper_t)(x86emu_t* emu, uintptr_t fnc);

// list of defined wrapper
// v = void, i = int32, u = uint32, U/I= (u)int64
// p = pointer, P = callback
// f = float, d = double, D = long double, L = fake long double
// V = vaargs, E = current x86emu struct
// 0 = constant 0, 1 = constant 1
// o = stdout
// C = unsigned byte c = char
// W = unsigned short w = short
// Q = ...
// S8 = struct, 8 bytes

#define __PASTER(x, y) x##y
#define _EVALUATOR(x, y) __PASTER(x, y)
#define PREFIX(y) _EVALUATOR(ret, _EVALUATOR(F, y))

#define GO(N, ...) GOW(N, __PASTER(N, _t), __VA_ARGS__)
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc);

#define GO01(b, ...) GO(PREFIX(b), __VA_ARGS__)
#define GO02(b, c, ...) GO01(b##c, __VA_ARGS__)
#define GO03(b, c, ...) GO02(b##c, __VA_ARGS__)
#define GO04(b, c, ...) GO03(b##c, __VA_ARGS__)
#define GO05(b, c, ...) GO04(b##c, __VA_ARGS__)
#define GO06(b, c, ...) GO05(b##c, __VA_ARGS__)
#define GO07(b, c, ...) GO06(b##c, __VA_ARGS__)
#define GO08(b, c, ...) GO07(b##c, __VA_ARGS__)
#define GO09(b, c, ...) GO08(b##c, __VA_ARGS__)
#define GO10(b, c, ...) GO09(b##c, __VA_ARGS__)
#define GO11(b, c, ...) GO10(b##c, __VA_ARGS__)
#define GO12(b, c, ...) GO11(b##c, __VA_ARGS__)
#define GO13(b, c, ...) GO12(b##c, __VA_ARGS__)
#define GO14(b, c, ...) GO13(b##c, __VA_ARGS__)
#define GO15(b, c, ...) GO14(b##c, __VA_ARGS__)

// void...
#define ret v
#include "wrappers.h"
#include "wrapper_v.h"
#undef ret

// uint8....
#define ret C
#include "wrappers.h"
#include "wrapper_u8.h"
#undef ret

// int32...
#define ret i
#include "wrappers.h"
#include "wrapper_i.h"
#undef ret

// uint32
#define ret u
#include "wrappers.h"
#include "wrapper_u.h"
#undef ret

// void*
#define ret p
#include "wrappers.h"
#include "wrapper_p.h"
#undef ret

// float
#define ret f
#include "wrappers.h"
#include "wrapper_f.h"
#undef ret

// double
#define ret d
#include "wrappers.h"
#include "wrapper_d.h"
#undef ret

// long double
#define ret D
#include "wrappers.h"
#include "wrapper_ld.h"
#undef ret

// int64...
#define ret I
#include "wrappers.h"
#include "wrapper_i64.h"
#undef ret

// uint64
#define ret U
#include "wrappers.h"
#include "wrapper_u64.h"
#undef ret

#undef GO15
#undef GO14
#undef GO13
#undef GO12
#undef GO11
#undef GO10
#undef GO09
#undef GO08
#undef GO07
#undef GO06
#undef GO05
#undef GO04
#undef GO03
#undef GO02
#undef GO01

#undef GOW
#undef GO

#undef PREFIX
#undef _EVALUATOR
#undef __PASTER

#endif //__WRAPPER_H_
