#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "stack.h"
#include "box86context.h"
#include "elfloader.h"
#include "debug.h"
#include "x86emu_private.h"

int CalcStackSize(box86context_t *context)
{
    printf_debug(DEBUG_DEBUG, "Calc stack size, based on %d elf(s)\n", context->elfsize);
    context->stacksz = 1024; context->stackalign=4;
    for (int i=0; i<context->elfsize; ++i)
        CalcStack(context->elfs[i], &context->stacksz, &context->stackalign);

    if (posix_memalign((void**)&context->stack, context->stackalign, context->stacksz)) {
        printf_debug(DEBUG_NONE, "Cannot allocate aligned memory (0x%x/0x%x) for stack\n", context->stacksz, context->stackalign);
        return 1;
    }
    printf_debug(DEBUG_DEBUG, "Stack is @%p size=0x%x align=0x%x\n", context->stack, context->stacksz, context->stackalign);

    return 0;
}

uint32_t Pop(x86emu_t *emu)
{
    uint32_t* st = ((uint32_t*)(R_ESP));
    R_ESP += 4;
    return *st;
}

void Push(x86emu_t *emu, uint32_t v)
{
    R_ESP -= 4;
    *((uint32_t*)R_ESP) = v;
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

