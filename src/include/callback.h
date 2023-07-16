#ifndef __CALLBACK_H__
#define __CALLBACK_H__

#include <stdint.h>

typedef struct x86emu_s x86emu_t;

uint32_t RunFunction(uintptr_t fnc, int nargs, ...);
// using a fmt description
uint32_t RunFunctionFmt(uintptr_t fnc, const char* fmt, ...);
// use thread local emu to run the function (context is unused now)
uint64_t RunFunction64(uintptr_t fnc, int nargs, ...);
// using a fmt description and returning a 64bit value
uint64_t RunFunctionFmt64(uintptr_t fnc, const char* fmt, ...);
// use emu state to run function
uint32_t RunFunctionWithEmu(x86emu_t *emu, int QuitOnLongJump, uintptr_t fnc, int nargs, ...);

uint32_t RunSafeFunction(uintptr_t fnc, int nargs, ...);
#endif //__CALLBACK_H__