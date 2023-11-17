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
uint32_t RunSafeFunction(uintptr_t fnc, int nargs, ...)
{
    x86emu_t *emu = thread_get_emu();

    Push(emu, R_EBP); // push rbp
    uintptr_t old_ebp = R_EBP = R_ESP;      // mov rbp, rsp

    Push(emu, R_EDI);
    Push(emu, R_ESI);
    Push(emu, R_EDX);
    Push(emu, R_ECX);
    Push(emu, R_EBX);
    Push(emu, R_EAX);

    R_ESP -= nargs*4;   // need to push in reverse order

    uint32_t *p = (uint32_t*)R_ESP;

    va_list va;
    va_start (va, nargs);
    for (int i=0; i<nargs; ++i) {
        *p = va_arg(va, uint32_t);
        p++;
    }
    va_end (va);

    // save defered flags
    defered_flags_t old_df = emu->df;
    uint32_t old_op1 = emu->op1;
    uint32_t old_op2 = emu->op2;
    uint32_t old_res = emu->res;

    uintptr_t oldip = R_EIP;
    DynaCall(emu, fnc);
    R_ESP+=(nargs*4);

    // restore defered flags
    emu->df = old_df;
    emu->op1 = old_op1;
    emu->op2 = old_op2;
    emu->res = old_res;

    uint32_t ret = R_EAX;
    R_EIP = oldip;

    R_EAX = Pop(emu);
    R_EBX = Pop(emu);
    R_ECX = Pop(emu);
    R_EDX = Pop(emu);
    R_ESI = Pop(emu);
    R_EDI = Pop(emu);

    R_ESP = old_ebp;      // mov rsp, rbp
    R_EBP = Pop(emu);     // pop rbp

    return ret;
}

EXPORTDYN
uint32_t RunFunction(uintptr_t fnc, int nargs, ...)
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
uint64_t RunFunction64(uintptr_t fnc, int nargs, ...)
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
uint32_t RunFunctionFmt(uintptr_t fnc, const char* fmt, ...)
{
    x86emu_t *emu = thread_get_emu();
    int nargs = 0;
    int ni = 0;
    int ndf = 0;
    for (int i=0; fmt[i]; ++i) {
        switch(fmt[i]) {
            case 'd': 
            case 'I': 
            case 'U': nargs+=2; break;
            case 'f': 
            case 'p': 
            case 'i': 
            case 'u': 
            case 'L': 
            case 'l': 
            case 'w': 
            case 'W': 
            case 'c': 
            case 'C': ++nargs; break;
            default:
                ++nargs; break;
        }
    }
    int align = nargs&1;
    int stackn = align + nargs;

    Push(emu, R_EBP);   // push ebp
    R_EBP = R_ESP;      // mov ebp, esp

    R_ESP -= stackn*sizeof(void*);   // need to push in reverse order

    uint32_t *p = (uint32_t*)R_ESP;

    #define GO(c, B, B2, N) case c: *((B*)p) = va_arg(va, B2); p+=N; break
    va_list va;
    va_start (va, fmt);
    for (int i=0; fmt[i]; ++i) {
        switch(fmt[i]) {
            GO('f', float, double, 1);
            GO('d', double, double, 2);
            GO('p', void*, void*, 1);
            GO('i', int, int, 1);
            GO('u', uint32_t, uint32_t, 1);
            GO('I', int64_t, int64_t, 2);
            GO('U', uint64_t, uint64_t, 2);
            GO('L', uint32_t, uint32_t, 1);
            GO('l', int32_t, int32_t, 1);
            GO('w', int16_t, int, 1);
            GO('W', uint16_t, int, 1);
            GO('c', int8_t, int, 1);
            GO('C', uint8_t, int, 1);
            default:
                printf_log(LOG_NONE, "Error, unhandled arg %d: '%c' in RunFunctionFmt\n", i, fmt[i]);
                *p = va_arg(va, uint32_t);
                ++p; 
                break;
        }
    }
    va_end (va);

    uintptr_t oldip = R_EIP;
    DynaCall(emu, fnc);
    R_ESP+=(nargs*4);

    if(oldip==R_EIP) {
        R_ESP = R_EBP;      // mov esp, ebp
        R_EBP = Pop(emu);   // pop ebp
    }

    uint32_t ret = R_EAX;

    return ret;
}

EXPORTDYN
uint64_t RunFunctionFmt64(uintptr_t fnc, const char* fmt, ...)
{
    x86emu_t *emu = thread_get_emu();
    int nargs = 0;
    int ni = 0;
    int ndf = 0;
    for (int i=0; fmt[i]; ++i) {
        switch(fmt[i]) {
            case 'd': 
            case 'I': 
            case 'U': nargs+=2; break;
            case 'f': 
            case 'p': 
            case 'i': 
            case 'u': 
            case 'L': 
            case 'l': 
            case 'w': 
            case 'W': 
            case 'c': 
            case 'C': ++nargs; break;
            default:
                ++nargs; break;
        }
    }
    int align = nargs&1;
    int stackn = align + nargs;

    Push(emu, R_EBP);   // push ebp
    R_EBP = R_ESP;      // mov ebp, esp

    R_ESP -= stackn*sizeof(void*);   // need to push in reverse order

    uint32_t *p = (uint32_t*)R_ESP;

    #define GO(c, B, B2, N) case c: *((B*)p) = va_arg(va, B2); p+=N; break
    va_list va;
    va_start (va, fmt);
    for (int i=0; fmt[i]; ++i) {
        switch(fmt[i]) {
            GO('f', float, double, 1);
            GO('d', double, double, 2);
            GO('p', void*, void*, 1);
            GO('i', int, int, 1);
            GO('u', uint32_t, uint32_t, 1);
            GO('I', int64_t, int64_t, 2);
            GO('U', uint64_t, uint64_t, 2);
            GO('L', uint32_t, uint32_t, 1);
            GO('l', int32_t, int32_t, 1);
            GO('w', int16_t, int, 1);
            GO('W', uint16_t, int, 1);
            GO('c', int8_t, int, 1);
            GO('C', uint8_t, int, 1);
            default:
                printf_log(LOG_NONE, "Error, unhandled arg %d: '%c' in RunFunctionFmt\n", i, fmt[i]);
                *p = va_arg(va, uint32_t);
                ++p; 
                break;
        }
    }
    va_end (va);

    uintptr_t oldip = R_EIP;
    DynaCall(emu, fnc);
    R_ESP+=(nargs*4);

    if(oldip==R_EIP) {
        R_ESP = R_EBP;      // mov esp, ebp
        R_EBP = Pop(emu);   // pop ebp
    }

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
    int oldlong = emu->flags.quitonlongjmp;

    emu->quit = 0;
    emu->flags.quitonlongjmp = QuitOnLongJump;

    DynaCall(emu, fnc);

    if(oldip==R_EIP)
        R_ESP+=(nargs*4);   // restore stack only if EIP is the one expected (else, it means return value is not the one expected)

    emu->quit = old_quit;
    emu->flags.quitonlongjmp = oldlong;


    return R_EAX;
}
