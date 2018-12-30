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

#define GO(N, ...) GOW(N, N##_t, __VA_ARGS__)
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc);

#define GO01(a, b, ...) GO(a##F##b, __VA_ARGS__)
#define GO02(a, b, c, ...) GO(a##F##b##c, __VA_ARGS__)
#define GO03(a, b, c, d, ...) GO(a##F##b##c##d, __VA_ARGS__)
#define GO04(a, b, c, d, e, ...) GO(a##F##b##c##d##e, __VA_ARGS__)
#define GO05(a, b, c, d, e, f, ...) GO(a##F##b##c##d##e##f, __VA_ARGS__)
#define GO06(a, b, c, d, e, f, g, ...) GO(a##F##b##c##d##e##f##g, __VA_ARGS__)
#define GO07(a, b, c, d, e, f, g, h, ...) GO(a##F##b##c##d##e##f##g##h, __VA_ARGS__)
#define GO08(a, b, c, d, e, f, g, h, i, ...) GO(a##F##b##c##d##e##f##g##h##i, __VA_ARGS__)
#define GO09(a, b, c, d, e, f, g, h, i, j, ...) GO(a##F##b##c##d##e##f##g##h##i##j, __VA_ARGS__)
#define GO10(a, b, c, d, e, f, g, h, i, j, k, ...) GO(a##F##b##c##d##e##f##g##h##i##j##k, __VA_ARGS__)
#define GO11(a, b, c, d, e, f, g, h, i, j, k, l, ...) GO(a##F##b##c##d##e##f##g##h##i##j##k##l, __VA_ARGS__)
#define GO12(a, b, c, d, e, f, g, h, i, j, k, l, m, ...) GO(a##F##b##c##d##e##f##g##h##i##j##k##l##m, __VA_ARGS__)

// void...
#include "wrapper_v.h"

// uint8....
#include "wrapper_u8.h"

// int32...
#include "wrapper_i.h"

// uint32
#include "wrapper_u.h"

// void*
#include "wrapper_p.h"

// float
#include "wrapper_f.h"

// double
#include "wrapper_d.h"

// long double
#include "wrapper_ld.h"

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

#endif //__WRAPPER_H_
