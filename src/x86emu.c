#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86emu_private.h"

static uint8_t EndEmuMarker[] = {0xcc, 'S', 'C', 0, 0, 0, 0};

x86emu_t *NewX86Emu(uintptr_t start, uintptr_t stack, int stacksize)
{
    printf_debug(DEBUG_DEBUG, "Allocate a new X86 Emu, with EIP=%p and Stack=%p/0x%X\n", start, stack, stacksize);

    x86emu_t *emu = (x86emu_t*)calloc(1, sizeof(x86emu_t));
    // set default value
    _EIP(emu->regs) = start;
    _ESP(emu->regs) = stack + stacksize - 4;
    // push "end emu" marker address
    Push(emu, (uint32_t)&EndEmuMarker);

    return emu;
}

void FreeX86Emu(x86emu_t **x86emu)
{
    if(!x86emu)
        return;
    printf_debug(DEBUG_DEBUG, "Free a X86 Emu (%p)\n", *x86emu);
    free(*x86emu);
    *x86emu = NULL;
}

uint32_t GetEAX(x86emu_t *emu)
{
    return _EAX(emu->regs);
}
void SetEAX(x86emu_t *emu, uint32_t v)
{
    _EAX(emu->regs) = v;
}
void SetEBX(x86emu_t *emu, uint32_t v)
{
    _EBX(emu->regs) = v;
}
void SetECX(x86emu_t *emu, uint32_t v)
{
    _ECX(emu->regs) = v;
}
void SetEDX(x86emu_t *emu, uint32_t v)
{
    _EDX(emu->regs) = v;
}

