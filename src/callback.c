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

typedef struct onecallback_s {
    x86emu_t    *emu;
    uintptr_t   fnc;
    int         nb_args;
    void*       arg[4];
    void*       stack;
} onecallback_t;

typedef struct callbacklist_s {
    onecallback_t**     list;   //TODO: use a khmap instead?
    int                 cap;
    int                 sz;
} callbacklist_t;

x86emu_t* AddCallback(x86emu_t* emu, uintptr_t fnc, int nb_args, void* arg1, void* arg2, void* arg3, void* arg4)
{
    callbacklist_t *callbacks = emu->context->callbacks;
    int idx = 0;
    if(callbacks->cap)
        while(idx<callbacks->sz && callbacks->list[idx])
            ++idx;
    if(idx==callbacks->cap) {
        callbacks->cap += 16;
        callbacks->list = (onecallback_t**)realloc(callbacks->list, callbacks->cap*sizeof(onecallback_t*));
    }
    int stsize = 1024*1024;
    callbacks->list[idx]->stack = malloc(stsize);
    x86emu_t * newemu = NewX86Emu(emu->context, fnc, (uintptr_t)callbacks->list[idx]->stack, stsize);
	SetupX86Emu(newemu, emu->shared_global, emu->globals);
    callbacks->list[idx]->emu = newemu;
    callbacks->list[idx]->fnc = fnc;
    callbacks->list[idx]->nb_args = nb_args;
    callbacks->list[idx]->arg[0] = arg1;
    callbacks->list[idx]->arg[1] = arg2;
    callbacks->list[idx]->arg[2] = arg3;
    callbacks->list[idx]->arg[3] = arg4;

    if(idx==callbacks->sz)
        ++callbacks->sz;

    return newemu;
}

onecallback_t* FindCallback(x86emu_t* emu)
{
    // find the callback first
    callbacklist_t *callbacks = emu->context->callbacks;
    if(!callbacks->cap)
        return NULL;
    int idx = 0;
    while(idx<callbacks->sz && callbacks->list[idx]) {
        if(callbacks->list[idx]->emu == emu) {
            return callbacks->list[idx];
        }
    }
    return NULL;
}

void FreeCallback(x86emu_t* emu)
{
    // find the callback first
    callbacklist_t *callbacks = emu->context->callbacks;
    if(!callbacks->cap)
        return;
    int idx = 0;
    while(idx<callbacks->sz && callbacks->list[idx]) {
        if(callbacks->list[idx]->emu == emu) {
            FreeX86Emu(&callbacks->list[idx]->emu);
            free(callbacks->list[idx]->stack);
            if(idx==callbacks->sz)
                --callbacks->sz;
            return;
        }
        ++idx;
    }
}

uint32_t RunCallback(x86emu_t* emu)
{
    onecallback_t *cb = FindCallback(emu);
    if(cb) {
        for (int i=0; i<cb->nb_args; ++i)
            Push(emu, (uint32_t)cb->arg[i]);
        PushExit(emu);
        R_EIP = cb->fnc;
        Run(emu);
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


callbacklist_t* NewCallbackList()
{
    callbacklist_t* callbacks = (callbacklist_t*)calloc(1, sizeof(callbacklist_t));
    return callbacks;
}

void FreeCallbackList(callbacklist_t** callbacks)
{
    if(!callbacks)
        return;
    if(!*callbacks)
        return;
    for (int i=0; i<(*callbacks)->sz; ++i)
        if((*callbacks)->list[i]) {
            FreeX86Emu(&(*callbacks)->list[i]->emu);
            free((*callbacks)->list[i]->stack);
        }
    free((*callbacks)->list);
    free(*callbacks);
    *callbacks = NULL;
}
