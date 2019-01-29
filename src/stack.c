#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "stack.h"
#include "box86context.h"
#include "elfloader.h"
#include "debug.h"
#include "x86emu_private.h"
#include "x86run_private.h"

int CalcStackSize(box86context_t *context)
{
    printf_log(LOG_DEBUG, "Calc stack size, based on %d elf(s)\n", context->elfsize);
    context->stacksz = 1024; context->stackalign=4;
    for (int i=0; i<context->elfsize; ++i)
        CalcStack(context->elfs[i], &context->stacksz, &context->stackalign);

    if (posix_memalign((void**)&context->stack, context->stackalign, context->stacksz)) {
        printf_log(LOG_NONE, "Cannot allocate aligned memory (0x%x/0x%x) for stack\n", context->stacksz, context->stackalign);
        return 1;
    }
    printf_log(LOG_DEBUG, "Stack is @%p size=0x%x align=0x%x\n", context->stack, context->stacksz, context->stackalign);

    return 0;
}

uint16_t Pop16(x86emu_t *emu)
{
    uint16_t* st = ((uint16_t*)(R_ESP));
    R_ESP += 2;
    return *st;
}

void Push16(x86emu_t *emu, uint16_t v)
{
    R_ESP -= 2;
    *((uint16_t*)R_ESP) = v;
}

void PushString(x86emu_t *emu, const char* s)
{
    int sz = strlen(s) + 1;
    // round to 4 bytes boundary
    R_ESP -= sz;
    memcpy((void*)R_ESP, s, sz);
}


void SetupInitialStack(box86context_t *context)
{
    // setup the stack...
    x86emu_t *emu = context->emu;
    // start with 0
    Push(emu, 0);
    // push program executed
    PushString(emu, context->argv[0]);
    uintptr_t p_arg0 = R_ESP;
    // push envs
    uintptr_t p_envv[context->envc];
    for (int i=context->envc-1; i>=0; --i) {
        PushString(emu, context->envv[i]);
        p_envv[i] = R_ESP;
    }
    // push args
    uintptr_t p_argv[context->argc];
    for (int i=context->argc-1; i>=0; --i) {
        PushString(emu, context->argv[i]);
        p_argv[i] = R_ESP;
    }
    // align
    uintptr_t tmp = (R_ESP)&~(context->stackalign-1);
    memset((void*)tmp, 0, R_ESP-tmp);
    R_ESP=tmp;

    // push some AuxVector stuffs
    PushString(emu, "i686");
    uintptr_t p_386 = R_ESP;
    for (int i=0; i<4; ++i)
        Push(emu, random());
    uintptr_t p_random = R_ESP;
    // align
    tmp = (R_ESP)&~(context->stackalign-1);
    memset((void*)tmp, 0, R_ESP-tmp);
    R_ESP=tmp;

    // push the AuxVector themselves
    Push(emu, 0); Push(emu, 0);         //AT_NULL(0)=0
    Push(emu, p_386); Push(emu, 15);    //AT_PLATFORM(15)=p_386
    Push(emu, p_arg0); Push(emu, 31);   //AT_EXECFN(31)=p_arg0
    Push(emu, p_random); Push(emu, 25); //AT_RANDOM(25)=p_random
    Push(emu, 0); Push(emu, 23);        //AT_SECURE(23)=0
    Push(emu, 1000); Push(emu, 14);     //AT_EGID(14)=1000 TODO
    Push(emu, 1000); Push(emu, 13);     //AT_GID(13)=1000 TODO
    Push(emu, 1000); Push(emu, 12);     //AT_EUID(12)=1000 TODO
    Push(emu, 1000); Push(emu, 11);     //AT_UID(11)=1000 TODO
    Push(emu, R_EIP); Push(emu, 9);     //AT_ENTRY(9)=entrypoint
    Push(emu, 0/*context->vsyscall*/); Push(emu, 32);      //AT_SYSINFO(32)=vsyscall
    // TODO: continue

    // push nil / envs / nil / args / argc
    Push(emu, 0);
    for (int i=context->envc-1; i>=0; --i)
        Push(emu, p_envv[i]);
    Push(emu, 0);
    for (int i=context->argc-1; i>=0; --i)
        Push(emu, p_argv[i]);
    Push(emu, context->argc);
}