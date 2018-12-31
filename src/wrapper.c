#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "wrapper.h"
#include "x86emu_private.h"
#include "x87emu_private.h"
#include "regs.h"

// the stack as the return address 1st thing, then the args...

#define v() void
#define i() int32_t
#define u() uint32_t
#define I() int64_t
#define U() uint64_t
#define p() void*
#define f() float
#define d() double
#define D() long double
#define E() x86emu_t*
#define C() unsigned char
#define c() char
#define W() unsigned short
#define w() short

#define __PASTER(x, y) x##y
#define _EVALUATOR(x, y) __PASTER(x, y)
#define PREFIX(y) _EVALUATOR(ret, _EVALUATOR(F, y))

#define GOW(...)
#define _GO(name, ...) typedef ret() (*_EVALUATOR(name, _t))(__VA_ARGS__);
#define GO(name, ...) _GO(name, __VA_ARGS__)
#define GO01(type01) GO(PREFIX(type01), type01())
#define GO02(type01, type02) GO(PREFIX(type01##type02), type01(), type02())
#define GO03(type01, type02, type03) GO(PREFIX(type01##type02##type03), type01(), type02(), type03())
#define GO04(type01, type02, type03, type04) GO(PREFIX(type01##type02##type03##type04), type01(), type02(), type03(), type04())
#define GO05(type01, type02, type03, type04, type05) GO(PREFIX(type01##type02##type03##type04##type05), type01(), type02(), type03(), type04(), type05())
#define GO06(type01, type02, type03, type04, type05, type06) GO(PREFIX(type01##type02##type03##type04##type05##type06), type01(), type02(), type03(), type04(), type05(), type06())
#define GO07(type01, type02, type03, type04, type05, type06, type07) GO(PREFIX(type01##type02##type03##type04##type05##type06##type07), type01(), type02(), type03(), type04(), type05(), type06(), type07())
#define GO08(type01, type02, type03, type04, type05, type06, type07, type08) GO(PREFIX(type01##type02##type03##type04##type05##type06##type07##type08), type01(), type02(), type03(), type04(), type05(), type06(), type07(), type08())
#define GO09(type01, type02, type03, type04, type05, type06, type07, type08, type09) GO(PREFIX(type01##type02##type03##type04##type05##type06##type07##type08##type09), type01(), type02(), type03(), type04(), type05(), type06(), type07(), type08(), type09())
#define GO10(type01, type02, type03, type04, type05, type06, type07, type08, type09, type10) GO(PREFIX(type01##type02##type03##type04##type05##type06##type07##type08##type09##type10), type01(), type02(), type03(), type04(), type05(), type06(), type07(), type08(), type09(), type10())
#define GO11(type01, type02, type03, type04, type05, type06, type07, type08, type09, type10, type11) GO(PREFIX(type01##type02##type03##type04##type05##type06##type07##type08##type09##type10##type11), type01(), type02(), type03(), type04(), type05(), type06(), type07(), type08(), type09(), type10(), type11())
#define GO12(type01, type02, type03, type04, type05, type06, type07, type08, type09, type10, type11, type12) GO(PREFIX(type01##type02##type03##type04##type05##type06##type07##type08##type09##type10##type11##type12), type01(), type02(), type03(), type04(), type05(), type06(), type07(), type08(), type09(), type10(), type11(), type12())
#define GO13(type01, type02, type03, type04, type05, type06, type07, type08, type09, type10, type11, type12, type13) GO(PREFIX(type01##type02##type03##type04##type05##type06##type07##type08##type09##type10##type11##type12##type13), type01(), type02(), type03(), type04(), type05(), type06(), type07(), type08(), type09(), type10(), type11(), type12(), type13())
#define GO14(type01, type02, type03, type04, type05, type06, type07, type08, type09, type10, type11, type12, type13, type14) GO(PREFIX(type01##type02##type03##type04##type05##type06##type07##type08##type09##type10##type11##type12##type13##type14), type01(), type02(), type03(), type04(), type05(), type06(), type07(), type08(), type09(), type10(), type11(), type12(), type13(), type14())
#define GO15(type01, type02, type03, type04, type05, type06, type07, type08, type09, type10, type11, type12, type13, type14, type15) GO(PREFIX(type01##type02##type03##type04##type05##type06##type07##type08##type09##type10##type11##type12##type13##type14##type15), type01(), type02(), type03(), type04(), type05(), type06(), type07(), type08(), type09(), type10(), type11(), type12(), type13(), type14(), type15())

// void...
#define ret v
#include "wrappers.h"
#undef ret

// uint8...
#define ret C
#include "wrappers.h"
#undef ret

// int32...
#define ret i
#include "wrappers.h"
#undef ret

// uint32....
#define ret u
#include "wrappers.h"
#undef ret

// void*....
#define ret p
#include "wrappers.h"
#undef ret

// float....
#define ret f
#include "wrappers.h"
#undef ret

// double....
#define ret d
#include "wrappers.h"
#undef ret

// long double....
#define ret D
#include "wrappers.h"
#undef ret

#undef v
#undef i
#undef u
#undef I
#undef U
#undef p
#undef f
#undef d
#undef D
#undef E
#undef C
#undef c
#undef W
#undef w
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
#undef GO
#undef GOW

#define DEF(A) A fn = (A)fnc

#define vd() 4
#define id() 4
#define ud() 4
#define Id() 8
#define Ud() 8
#define pd() 4
#define fd() 4
#define dd() 8
#define Dd() 12
#define Ed() 0
#define Cd() 4
#define cd() 4
#define Wd() 4
#define wd() 4

#define stack(n) (R_ESP+4+n)
#define v(p)
#define c(p)    *(int8_t*)stack(p)
#define w(p)    *(int16_t*)stack(p)
#define i(p)    *(int32_t*)stack(p)
#define C(p)    *(uint8_t*)stack(p)
#define W(p)    *(uint16_t*)stack(p)
#define u(p)    *(uint32_t*)stack(p)
// to check vvvv
#define I(p)    *(int64_t*)stack(p)
#define U(p)    *(uint64_t*)stack(p)
//          ^^^^
#define p(p)    *(void**)stack(p)
#define f(p)    *(float*)stack(p)
#define d(p)    *(double*)stack(p)
#define D(p)    *(long double*)stack(p)
#define E(p)    emu

#define GO(N, ...) GOW(PREFIX(N), _EVALUATOR(PREFIX(N), _t), __VA_ARGS__)

#define GO01(type01) GO(type01, type01(0))
#define GO02(type01, type02) GO(type01##type02, type01(0), type02(type01##d()))
#define GO03(type01, type02, type03) GO(type01##type02##type03, type01(0), type02(type01##d()), type03(type01##d() + type02##d()))
#define GO04(type01, type02, type03, type04) GO(type01##type02##type03##type04, type01(0), type02(type01##d()), type03(type01##d() + type02##d()), type04(type01##d() + type02##d() + type03##d()))
#define GO05(type01, type02, type03, type04, type05) GO(type01##type02##type03##type04##type05, type01(0), type02(type01##d()), type03(type01##d() + type02##d()), type04(type01##d() + type02##d() + type03##d()), type05(type01##d() + type02##d() + type03##d() + type04##d()))
#define GO06(type01, type02, type03, type04, type05, type06) GO(type01##type02##type03##type04##type05##type06, type01(0), type02(type01##d()), type03(type01##d() + type02##d()), type04(type01##d() + type02##d() + type03##d()), type05(type01##d() + type02##d() + type03##d() + type04##d()), type06(type01##d() + type02##d() + type03##d() + type04##d() + type05##d()))
#define GO07(type01, type02, type03, type04, type05, type06, type07) GO(type01##type02##type03##type04##type05##type06##type07, type01(0), type02(type01##d()), type03(type01##d() + type02##d()), type04(type01##d() + type02##d() + type03##d()), type05(type01##d() + type02##d() + type03##d() + type04##d()), type06(type01##d() + type02##d() + type03##d() + type04##d() + type05##d()), type07(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d()))
#define GO08(type01, type02, type03, type04, type05, type06, type07, type08) GO(type01##type02##type03##type04##type05##type06##type07##type08, type01(0), type02(type01##d()), type03(type01##d() + type02##d()), type04(type01##d() + type02##d() + type03##d()), type05(type01##d() + type02##d() + type03##d() + type04##d()), type06(type01##d() + type02##d() + type03##d() + type04##d() + type05##d()), type07(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d()), type08(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d()))
#define GO09(type01, type02, type03, type04, type05, type06, type07, type08, type09) GO(type01##type02##type03##type04##type05##type06##type07##type08##type09, type01(0), type02(type01##d()), type03(type01##d() + type02##d()), type04(type01##d() + type02##d() + type03##d()), type05(type01##d() + type02##d() + type03##d() + type04##d()), type06(type01##d() + type02##d() + type03##d() + type04##d() + type05##d()), type07(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d()), type08(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d()), type09(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d()))
#define GO10(type01, type02, type03, type04, type05, type06, type07, type08, type09, type10) GO(type01##type02##type03##type04##type05##type06##type07##type08##type09##type10, type01(0), type02(type01##d()), type03(type01##d() + type02##d()), type04(type01##d() + type02##d() + type03##d()), type05(type01##d() + type02##d() + type03##d() + type04##d()), type06(type01##d() + type02##d() + type03##d() + type04##d() + type05##d()), type07(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d()), type08(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d()), type09(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d()), type10(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d()))
#define GO11(type01, type02, type03, type04, type05, type06, type07, type08, type09, type10, type11) GO(type01##type02##type03##type04##type05##type06##type07##type08##type09##type10##type11, type01(0), type02(type01##d()), type03(type01##d() + type02##d()), type04(type01##d() + type02##d() + type03##d()), type05(type01##d() + type02##d() + type03##d() + type04##d()), type06(type01##d() + type02##d() + type03##d() + type04##d() + type05##d()), type07(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d()), type08(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d()), type09(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d()), type10(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d()), type11(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d() + type10##d()))
#define GO12(type01, type02, type03, type04, type05, type06, type07, type08, type09, type10, type11, type12) GO(type01##type02##type03##type04##type05##type06##type07##type08##type09##type10##type11##type12, type01(0), type02(type01##d()), type03(type01##d() + type02##d()), type04(type01##d() + type02##d() + type03##d()), type05(type01##d() + type02##d() + type03##d() + type04##d()), type06(type01##d() + type02##d() + type03##d() + type04##d() + type05##d()), type07(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d()), type08(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d()), type09(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d()), type10(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d()), type11(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d() + type10##d()), type12(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d() + type10##d() + type11##d()))
#define GO13(type01, type02, type03, type04, type05, type06, type07, type08, type09, type10, type11, type12, type13) GO(type01##type02##type03##type04##type05##type06##type07##type08##type09##type10##type11##type12##type13, type01(0), type02(type01##d()), type03(type01##d() + type02##d()), type04(type01##d() + type02##d() + type03##d()), type05(type01##d() + type02##d() + type03##d() + type04##d()), type06(type01##d() + type02##d() + type03##d() + type04##d() + type05##d()), type07(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d()), type08(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d()), type09(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d()), type10(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d()), type11(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d() + type10##d()), type12(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d() + type10##d() + type11##d()), type13(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d() + type10##d() + type11##d() + type12##d()))
#define GO14(type01, type02, type03, type04, type05, type06, type07, type08, type09, type10, type11, type12, type13, type14) GO(type01##type02##type03##type04##type05##type06##type07##type08##type09##type10##type11##type12##type13##type14, type01(0), type02(type01##d()), type03(type01##d() + type02##d()), type04(type01##d() + type02##d() + type03##d()), type05(type01##d() + type02##d() + type03##d() + type04##d()), type06(type01##d() + type02##d() + type03##d() + type04##d() + type05##d()), type07(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d()), type08(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d()), type09(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d()), type10(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d()), type11(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d() + type10##d()), type12(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d() + type10##d() + type11##d()), type13(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d() + type10##d() + type11##d() + type12##d()), type14(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d() + type10##d() + type11##d() + type12##d() + type13##d()))
#define GO15(type01, type02, type03, type04, type05, type06, type07, type08, type09, type10, type11, type12, type13, type14, type15) GO(type01##type02##type03##type04##type05##type06##type07##type08##type09##type10##type11##type12##type13##type14##type15, type01(0), type02(type01##d()), type03(type01##d() + type02##d()), type04(type01##d() + type02##d() + type03##d()), type05(type01##d() + type02##d() + type03##d() + type04##d()), type06(type01##d() + type02##d() + type03##d() + type04##d() + type05##d()), type07(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d()), type08(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d()), type09(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d()), type10(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d()), type11(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d() + type10##d()), type12(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d() + type10##d() + type11##d()), type13(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d() + type10##d() + type11##d() + type12##d()), type14(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d() + type10##d() + type11##d() + type12##d() + type13##d()), type15(type01##d() + type02##d() + type03##d() + type04##d() + type05##d() + type06##d() + type07##d() + type08##d() + type09##d() + type10##d() + type11##d() + type12##d() + type13##d() + type14##d()))

// void...
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); fn(__VA_ARGS__); }
#define ret v
#include "wrappers.h"
#include "wrapper_v.h"
#undef ret
#undef GOW

// uint8....
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); R_EAX=fn(__VA_ARGS__); }
#define ret C
#include "wrappers.h"
#include "wrapper_u8.h"
#undef ret
#undef GOW

// int32....
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); R_EAX=fn(__VA_ARGS__); }
#define ret i
#include "wrappers.h"
#include "wrapper_i.h"
#undef ret
#undef GOW

// uint32....
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); R_EAX=(uint32_t)fn(__VA_ARGS__); }
#define ret u
#include "wrappers.h"
#include "wrapper_u.h"
#undef ret
#undef GOW

// void*....
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); R_EAX=(uintptr_t)fn(__VA_ARGS__); }
#define ret p
#include "wrappers.h"
#include "wrapper_p.h"
#undef ret
#undef GOW

// float
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); float fl=fn(__VA_ARGS__); fpu_do_push(emu); ST0.d = fl;}
#define ret f
#include "wrappers.h"
#include "wrapper_f.h"
#undef ret
#undef GOW

// double
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); double db=fn(__VA_ARGS__); fpu_do_push(emu); ST0.d = db;}
#define ret d
#include "wrappers.h"
#include "wrapper_d.h"
#undef ret
#undef GOW

// long double
#define GOW(N, W, ...) void N(x86emu_t *emu, uintptr_t fnc){ DEF(W); long double ld=fn(__VA_ARGS__); fpu_do_push(emu); ST0.d = ld;}
#define ret D
#include "wrappers.h"
#include "wrapper_ld.h"
#undef ret
#undef GOW

#undef v
#undef i
#undef u
#undef I
#undef U
#undef p
#undef f
#undef d
#undef D
#undef E
#undef C
#undef c
#undef W
#undef w

#undef vd
#undef id
#undef ud
#undef Id
#undef Ud
#undef pd
#undef fd
#undef dd
#undef Dd
#undef Ed
#undef Cd
#undef cd
#undef Wd
#undef wd

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
#undef GO

#undef PREFIX
#undef _EVALUATOR
#undef __PASTER
