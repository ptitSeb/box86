#ifndef __WRAPPER_H_
#define __WRAPPER_H_
#include <stdint.h>

typedef struct x86emu_s x86emu_t;

// the generic wrapper pointer functions
typedef void (*wrapper_t)(x86emu_t* emu, uintptr_t fnc);

// list of defined wrapper
// v = void, i = int32, u = uint32, U/I= (u)int64
// p = pointer
// f = float, d = double, D = long double
// V = vaargs, E = current x86emu struct
// 0 = constant 0, 1 = constant 1
// o = stdout

#define GO(N, ...) GOW(N, N##_t, __VA_ARGS__)
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc);

// void...
#include "wrapper_v.h"

// int32...
#include "wrapper_i.h"

// uint32
#include "wrapper_u.h"

// void*
#include "wrapper_p.h"

#undef GOW
#undef GO

#endif //__WRAPPER_H_