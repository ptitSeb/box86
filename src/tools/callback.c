#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "debug.h"
#include "x86emu.h"
#include "x86run.h"
#include "emu/x86emu_private.h"
#include "emu/x86run_private.h"
#include "box86context.h"
#include "box86stack.h"
#include "dynarec.h"

EXPORTDYN
uint32_t RunFunction(box86context_t *context, uintptr_t fnc, int nargs, ...)
{
    x86emu_t *emu = thread_get_emu();

    R_ESP -= nargs*4;   // need to push in reverse order

    uint32_t *p = (uint32_t*)R_ESP;

    va_list va;
    va_start (va, nargs);
    for (int i=0; i<nargs; ++i) {
        *p = va_arg(va, uint32_t);
        p++;
    }
    va_end (va);

    DynaCall(emu, fnc);
    R_ESP+=(nargs*4);

    uint32_t ret = R_EAX;

    return ret;
}

EXPORTDYN
uint64_t RunFunction64(box86context_t *context, uintptr_t fnc, int nargs, ...)
{
    x86emu_t *emu = thread_get_emu();

    R_ESP -= nargs*4;   // need to push in reverse order

    uint32_t *p = (uint32_t*)R_ESP;

    va_list va;
    va_start (va, nargs);
    for (int i=0; i<nargs; ++i) {
        *p = va_arg(va, uint32_t);
        p++;
    }
    va_end (va);

    DynaCall(emu, fnc);
    R_ESP+=(nargs*4);

    uint64_t ret = (uint64_t)R_EAX | ((uint64_t)R_EDX)<<32;

    return ret;
}

EXPORTDYN
uint32_t RunFunctionWithEmu(x86emu_t *emu, int QuitOnLongJump, uintptr_t fnc, int nargs, ...)
{
    R_ESP -= nargs*4;   // need to push in reverse order

    uint32_t *p = (uint32_t*)R_ESP;

    va_list va;
    va_start (va, nargs);
    for (int i=0; i<nargs; ++i) {
        *p = va_arg(va, uint32_t);
        p++;
    }
    va_end (va);

    uint32_t oldip = R_EIP;
    int old_quit = emu->quit;
    int oldlong = emu->quitonlongjmp;

    emu->quit = 0;
    emu->quitonlongjmp = QuitOnLongJump;

    DynaCall(emu, fnc);

    if(oldip==R_EIP)
        R_ESP+=(nargs*4);   // restore stack only if EIP is the one expected (else, it means return value is not the one expected)

    emu->quit = old_quit;
    emu->quitonlongjmp = oldlong;


    return R_EAX;
}
