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
#include "khash.h"

typedef struct onecallback_s onecallback_t;

typedef struct onecallback_s {
    x86emu_t    *emu;
    uintptr_t   fnc;
    int         nb_args;
    void*       arg[10];
    int         shared;
    onecallback_t *chain;
} onecallback_t;

KHASH_MAP_INIT_INT(callbacks, onecallback_t*)

typedef struct callbacklist_s {
    kh_callbacks_t      *list;
    int                 cap;
    int                 sz;
} callbacklist_t;

onecallback_t* FindCallback(x86emu_t* emu)
{
    if(!emu)
        return NULL;
    // find the callback first
    callbacklist_t *callbacks = emu->context->callbacks;
    khint_t k = kh_get(callbacks, callbacks->list, (uintptr_t)emu);
    if(k==kh_end(callbacks->list) || !kh_exist(callbacks->list, k))
        return NULL;
    return kh_value(callbacks->list, k);
}

x86emu_t* FindCallbackFnc1Arg(x86emu_t* emu, uintptr_t fnc, int argn, void* arg)
{
    if(!emu)
        return NULL;
    // find the callback first
    callbacklist_t *callbacks = emu->context->callbacks;
    onecallback_t* cb;
    kh_foreach_value(callbacks->list, cb, 
        if(cb->fnc==fnc && cb->arg[argn]==arg)
            return cb->emu;
    );
    return NULL;
}

int IsCallback(box86context_t* context, x86emu_t* cb)
{
    if(!cb)
        return 0;
    callbacklist_t *callbacks = context->callbacks;
    khint_t k = kh_get(callbacks, callbacks->list, (uintptr_t)cb);
    if(k==kh_end(callbacks->list) || !kh_exist(callbacks->list, k))
        return 0;
    return 1;
}

x86emu_t* AddVariableCallback(x86emu_t* emu, int stsize, uintptr_t fnc, int nb_args, void* arg1, void* arg2, void* arg3, void* arg4)
{
    callbacklist_t *callbacks = emu->context->callbacks;
    void* stack = calloc(1, stsize);
    if(!stack) {
        printf_log(LOG_NONE, "BOX86: Error, cannot allocate %d KB Stack for callback\n", stsize/1024);
    }
    x86emu_t * newemu = NewX86Emu(emu->context, fnc, (uintptr_t)stack, stsize, 1);
	SetupX86Emu(newemu);
    newemu->trace_start = emu->trace_start;
    newemu->trace_end = emu->trace_end;

    onecallback_t * cb;
    int ret;
    khint_t k = kh_put(callbacks, callbacks->list, (uintptr_t)newemu, &ret);
    cb = kh_value(callbacks->list, k) = (onecallback_t*)calloc(1, sizeof(onecallback_t));

    cb->emu = newemu;
    cb->fnc = fnc;
    cb->nb_args = nb_args;
    cb->arg[0] = arg1;
    cb->arg[1] = arg2;
    cb->arg[2] = arg3;
    cb->arg[3] = arg4;

    cb->shared = 0;

    return newemu;
}

x86emu_t* AddCallback(x86emu_t* emu, uintptr_t fnc, int nb_args, void* arg1, void* arg2, void* arg3, void* arg4)
{
    // 2MB stack (1MB is not enough for Xenonauts)
    return AddVariableCallback(emu, 2*1024*1024, fnc, nb_args, arg1, arg2, arg3, arg4);
}


x86emu_t* AddSmallCallback(x86emu_t* emu, uintptr_t fnc, int nb_args, void* arg1, void* arg2, void* arg3, void* arg4)
{
    // 64KB stack
    return AddVariableCallback(emu, 64*1024, fnc, nb_args, arg1, arg2, arg3, arg4);
}

x86emu_t* AddSharedCallback(x86emu_t* emu, uintptr_t fnc, int nb_args, void* arg1, void* arg2, void* arg3, void* arg4)
{
    callbacklist_t *callbacks = emu->context->callbacks;
    x86emu_t * newemu = emu;

    onecallback_t * cb;

    onecallback_t * old = FindCallback(emu);
    
    int ret;
    khint_t k = kh_put(callbacks, callbacks->list, (uintptr_t)newemu, &ret);
    cb = kh_value(callbacks->list, k) = (onecallback_t*)calloc(1, sizeof(onecallback_t));

    cb->emu = newemu;
    cb->fnc = fnc;
    cb->nb_args = nb_args;
    cb->arg[0] = arg1;
    cb->arg[1] = arg2;
    cb->arg[2] = arg3;
    cb->arg[3] = arg4;
    cb->shared = 1;
    cb->chain = old;

    return newemu;
}

x86emu_t* GetCallback1Arg(x86emu_t* emu, uintptr_t fnc, int nb_args, void* arg1)
{
    callbacklist_t *callbacks = emu->context->callbacks;

    onecallback_t * cb;
    kh_foreach_value(callbacks->list, cb, 
        if(cb->fnc==fnc && cb->nb_args==nb_args && cb->arg[0]==arg1) {
            return cb->emu;
        }
    );
    return NULL;
}

x86emu_t* FreeCallback(x86emu_t* emu)
{
    // find the callback first
    callbacklist_t *callbacks = emu->context->callbacks;
    khint_t k = kh_get(callbacks, callbacks->list, (uintptr_t)emu);
    if(k==kh_end(callbacks->list))
        return emu;
    onecallback_t* cb = kh_value(callbacks->list, k);
    if(!cb->shared)
        FreeX86Emu(&cb->emu);
    x86emu_t* ret = NULL;
    if(cb->chain) {
        kh_value(callbacks->list, k) = cb->chain;   // unchain, in case of shared callback inside callback
        ret = cb->chain->emu;
    } else {
        kh_del(callbacks, callbacks->list, k);
    }
    free(cb);
    return ret;
}

uint32_t RunCallback(x86emu_t* emu)
{
    onecallback_t *cb = FindCallback(emu);
    if(cb) {
        for (int i=cb->nb_args-1; i>=0; --i)    // reverse order
            Push(emu, (uint32_t)cb->arg[i]);
        DynaCall(emu, cb->fnc);
        R_ESP+=(cb->nb_args*4);
        return R_EAX;
    }
    printf_log(LOG_INFO, "Warning, Callback not found?!\n");
    return 0;
}

void SetCallbackArg(x86emu_t* emu, int arg, void* val)
{
    onecallback_t *cb = FindCallback(emu);
    if(cb) {
        cb->arg[arg] = val;
    }
}

void SetCallbackNArg(x86emu_t* emu, int narg)
{
    onecallback_t *cb = FindCallback(emu);
    if(cb) {
        cb->nb_args = narg;
    }
}

void* GetCallbackArg(x86emu_t* emu, int arg)
{
    onecallback_t *cb = FindCallback(emu);
    if(cb) {
        return cb->arg[arg];
    }
    return NULL;
}

void SetCallbackAddress(x86emu_t* emu, uintptr_t address)
{
    onecallback_t *cb = FindCallback(emu);
    if(cb) {
        cb->fnc = address;
    }
}

uintptr_t GetCallbackAddress(x86emu_t* emu)
{
    onecallback_t *cb = FindCallback(emu);
    if(cb) {
        return cb->fnc;
    }
    return 0;
}

callbacklist_t* NewCallbackList()
{
    callbacklist_t* callbacks = (callbacklist_t*)calloc(1, sizeof(callbacklist_t));
    callbacks->list = kh_init(callbacks);
    return callbacks;
}

void FreeCallbackList(callbacklist_t** callbacks)
{
    if(!callbacks)
        return;
    if(!*callbacks)
        return;
    onecallback_t* cb;
    kh_foreach_value((*callbacks)->list, cb,
        FreeX86Emu(&cb->emu);
        free(cb);
    );
    kh_destroy(callbacks, (*callbacks)->list);
    free(*callbacks);
    *callbacks = NULL;
}

uint32_t RunFunction(box86context_t *context, uintptr_t fnc, int nargs, ...)
{
    uint32_t mystack[60*1024] = {0};  // there is a limit at 256K (and even less on not main thread) for object on the stack
    x86emu_t myemu = {0};
    x86emu_t *emu = NewX86EmuFromStack(&myemu, context, fnc, (uintptr_t)&mystack, 60*1024*4, 0);
    SetupX86Emu(emu);
    SetTraceEmu(emu, context->emu->trace_start, context->emu->trace_end);

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
    FreeX86EmuFromStack(&emu);

    return ret;
}

uint32_t RunFunctionFast(box86context_t *context, uintptr_t fnc, int nargs, ...)
{
    uint32_t mystack[30*1024];
    x86emu_t myemu = {0};
    x86emu_t *emu = NewX86EmuFromStack(&myemu, context, fnc, (uintptr_t)&mystack, 30*1024*4, 0);
    SetupX86Emu(emu);
    SetTraceEmu(emu, context->emu->trace_start, context->emu->trace_end);

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
    FreeX86EmuFromStack(&emu);

    return ret;
}

void SetCallbackArgs(x86emu_t* emu, int nargs, ...)
{
    onecallback_t *cb = FindCallback(emu);
    if(cb) {
        va_list va;
        va_start (va, nargs);
        for (int i=0; i<nargs; ++i) {
            cb->arg[i] = va_arg(va, void*);
        }
        va_end (va);
    }
}

void SetCallbackNArgs(x86emu_t* emu, int N, int nargs, ...)
{
    onecallback_t *cb = FindCallback(emu);
    if(cb) {
        va_list va;
        va_start (va, nargs);
        for (int i=0; i<nargs; ++i) {
            cb->arg[N+i] = va_arg(va, void*);
        }
        va_end (va);
    }
}

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

    uint32_t ret = R_EAX;

    return ret;
}
