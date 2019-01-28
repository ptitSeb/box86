#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "debug.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "box86context.h"
#include "stack.h"
#include "khash.h"

typedef struct onecallback_s {
    x86emu_t    *emu;
    uintptr_t   fnc;
    int         nb_args;
    void*       arg[4];
    void*       stack;
} onecallback_t;

KHASH_MAP_INIT_INT(callbacks, onecallback_t*)

typedef struct callbacklist_s {
    kh_callbacks_t      *list;
    int                 cap;
    int                 sz;
} callbacklist_t;

x86emu_t* AddCallback(x86emu_t* emu, uintptr_t fnc, int nb_args, void* arg1, void* arg2, void* arg3, void* arg4)
{
    callbacklist_t *callbacks = emu->context->callbacks;
    int stsize = 1024*1024;
    void* stack = malloc(stsize);
    x86emu_t * newemu = NewX86Emu(emu->context, fnc, (uintptr_t)stack, stsize);
	SetupX86Emu(newemu, emu->shared_global, emu->globals);
    newemu->trace_start = emu->trace_start;
    newemu->trace_end = emu->trace_end;

    onecallback_t * cb;
    int ret;
    khint_t k = kh_put(callbacks, callbacks->list, (uintptr_t)newemu, &ret);
    cb = kh_value(callbacks->list, k) = (onecallback_t*)calloc(1, sizeof(onecallback_t));

    cb->stack = stack;
    cb->emu = newemu;
    cb->fnc = fnc;
    cb->nb_args = nb_args;
    cb->arg[0] = arg1;
    cb->arg[1] = arg2;
    cb->arg[2] = arg3;
    cb->arg[3] = arg4;

    return newemu;
}

onecallback_t* FindCallback(x86emu_t* emu)
{
    // find the callback first
    callbacklist_t *callbacks = emu->context->callbacks;
    khint_t k = kh_get(callbacks, callbacks->list, (uintptr_t)emu);
    if(k==kh_end(callbacks->list))
        return NULL;
    return kh_value(callbacks->list, k);
}

void FreeCallback(x86emu_t* emu)
{
    // find the callback first
    callbacklist_t *callbacks = emu->context->callbacks;
    khint_t k = kh_get(callbacks, callbacks->list, (uintptr_t)emu);
    if(k==kh_end(callbacks->list))
        return;
    onecallback_t* cb = kh_value(callbacks->list, k);
    FreeX86Emu(&cb->emu);
    free(cb->stack);
    free(cb);
    kh_del(callbacks, callbacks->list, k);
}

uint32_t RunCallback(x86emu_t* emu)
{
    onecallback_t *cb = FindCallback(emu);
    if(cb) {
        for (int i=cb->nb_args-1; i>=0; --i)    // reverse order
            Push(emu, (uint32_t)cb->arg[i]);
        EmuCall(emu, cb->fnc);
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
        free(cb->stack);
        free(cb);
    );
    kh_destroy(callbacks, (*callbacks)->list);
    free(*callbacks);
    *callbacks = NULL;
}
