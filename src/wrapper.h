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

void    vFv(x86emu_t *emu, uintptr_t fnc);
void    vFp(x86emu_t *emu, uintptr_t fnc);
void    vFE(x86emu_t *emu, uintptr_t fnc);
void    uFE(x86emu_t *emu, uintptr_t fnc);
void    iFv(x86emu_t *emu, uintptr_t fnc);
void    uFv(x86emu_t *emu, uintptr_t fnc);
void    pFv(x86emu_t *emu, uintptr_t fnc);
void    vFi(x86emu_t *emu, uintptr_t fnc);
void    iFi(x86emu_t *emu, uintptr_t fnc);
void    iFp(x86emu_t *emu, uintptr_t fnc);
void    pFp(x86emu_t *emu, uintptr_t fnc);
void    iFpp(x86emu_t *emu, uintptr_t fnc);
void    iFppi(x86emu_t *emu, uintptr_t fnc);
void    pFuu(x86emu_t *emu, uintptr_t fnc);
void    iFii(x86emu_t *emu, uintptr_t fnc);
void    iFip(x86emu_t *emu, uintptr_t fnc);
void    iFuu(x86emu_t *emu, uintptr_t fnc);
void    iFup(x86emu_t *emu, uintptr_t fnc);
void    iFpv(x86emu_t *emu, uintptr_t fnc);
void    iF1pV(x86emu_t *emu, uintptr_t fnc);
void    iFopV(x86emu_t *emu, uintptr_t fnc);
void    iFvopV(x86emu_t *emu, uintptr_t fnc);
void	iFEpppp(x86emu_t *emu, uintptr_t fnc);
void    iFEpippppp(x86emu_t *emu, uintptr_t fnc);   // this is __libc_start_main basically


#endif //__WRAPPER_H_