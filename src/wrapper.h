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
// v = vaargs, E = current x86emu struct

void    vFv(x86emu_t *emu, uintptr_t fnc);
void    vFE(x86emu_t *emu, uintptr_t fnc);
void    uFE(x86emu_t *emu, uintptr_t fnc);
void    iFv(x86emu_t *emu, uintptr_t fnc);
void    vFi(x86emu_t *emu, uintptr_t fnc);
void    iFi(x86emu_t *emu, uintptr_t fnc);
void    iFp(x86emu_t *emu, uintptr_t fnc);
void    iFpp(x86emu_t *emu, uintptr_t fnc);
void    iFpv(x86emu_t *emu, uintptr_t fnc);


#endif //__WRAPPER_H_