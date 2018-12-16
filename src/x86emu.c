#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86emu_private.h"
#include "box86context.h"
#include "x86trace.h"

static uint8_t EndEmuMarker[] = {0xcc, 'S', 'C', 0, 0, 0, 0};

x86emu_t *NewX86Emu(box86context_t *context, uintptr_t start, uintptr_t stack, int stacksize)
{
    printf_debug(DEBUG_DEBUG, "Allocate a new X86 Emu, with EIP=%p and Stack=%p/0x%X\n", start, stack, stacksize);

    x86emu_t *emu = (x86emu_t*)calloc(1, sizeof(x86emu_t));
    // setup cpu helpers
    for (int i=0; i<8; ++i)
        emu->sbiidx[i] = &emu->regs[i];
    emu->sbiidx[4] = &emu->zero;

    // set default value
    R_EIP = start;
    R_ESP = stack + stacksize;
    // push "end emu" marker address
    Push(emu, (uint32_t)&EndEmuMarker);
    // if trace is activated
    if(context->x86trace) {
        emu->dec = InitX86TraceDecoder(context);
        if(!emu->dec)
            printf_debug(DEBUG_INFO, "Failed to initialize Zydis decoder and formater, no trace activated\n");
    }

    return emu;
}

void FreeX86Emu(x86emu_t **x86emu)
{
    if(!x86emu)
        return;
    printf_debug(DEBUG_DEBUG, "Free a X86 Emu (%p)\n", *x86emu);
    if((*x86emu)->dec)
        DeleteX86TraceDecoder(&(*x86emu)->dec);
    free(*x86emu);
    *x86emu = NULL;
}

uint32_t GetEAX(x86emu_t *emu)
{
    return R_EAX;
}
void SetEAX(x86emu_t *emu, uint32_t v)
{
    R_EAX = v;
}
void SetEBX(x86emu_t *emu, uint32_t v)
{
    R_EBX = v;
}
void SetECX(x86emu_t *emu, uint32_t v)
{
    R_ECX = v;
}
void SetEDX(x86emu_t *emu, uint32_t v)
{
    R_EDX = v;
}

const char* DumpCPURegs(x86emu_t* emu)
{
    static char buff[500];
    char* regname[] = {"EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"};
    char tmp[50];
    sprintf(buff, "EIP=%08X ", R_EIP);
    for (int i=_AX; i<=_DI; ++i) {
        sprintf(tmp, "%s=%08X ", regname[i], emu->regs[i].dword[0]);
        strcat(buff, tmp);
    }
    return buff;
}