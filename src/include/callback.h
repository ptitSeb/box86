#ifndef __CALLBACK_H__
#define __CALLBACK_H__

#include <stdint.h>

typedef struct x86emu_s x86emu_t;

uint32_t RunFunction(box86context_t *context, uintptr_t fnc, int nargs, ...);
// use thread local emu to run the function (context is unused now)
uint64_t RunFunction64(box86context_t *context, uintptr_t fnc, int nargs, ...);
// use emu state to run function
uint32_t RunFunctionWithEmu(x86emu_t *emu, int QuitOnLongJump, uintptr_t fnc, int nargs, ...);

#endif //__CALLBACK_H__