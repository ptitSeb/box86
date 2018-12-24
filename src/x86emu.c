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
void PushExit(x86emu_t* emu)
{
    Push(emu, (uint32_t)&EndEmuMarker);
}
/// maxval not inclusive
int getrand(int maxval)
{
    if(maxval<1024) {
        return ((random()&0x7fff)*maxval)/0x7fff;
    } else {
        uint64_t r = random();
        r = (r*maxval) / RAND_MAX;
        return r;
    }
}

x86emu_t *NewX86Emu(box86context_t *context, uintptr_t start, uintptr_t stack, int stacksize)
{
    printf_log(LOG_DEBUG, "Allocate a new X86 Emu, with EIP=%p and Stack=%p/0x%X\n", start, stack, stacksize);

    x86emu_t *emu = (x86emu_t*)calloc(1, sizeof(x86emu_t));
    emu->context = context;
    // setup cpu helpers
    for (int i=0; i<8; ++i)
        emu->sbiidx[i] = &emu->regs[i];
    emu->sbiidx[4] = &emu->zero;
    // set default value
    R_EIP = start;
    R_ESP = stack + stacksize;

    // if trace is activated
    if(context->x86trace) {
        emu->dec = InitX86TraceDecoder(context);
        if(!emu->dec)
            printf_log(LOG_INFO, "Failed to initialize Zydis decoder and formater, no trace activated\n");
    }
    
    return emu;
}

void SetupX86Emu(x86emu_t *emu, int* shared_global, void* globals)
{
    printf_log(LOG_DEBUG, "Setup X86 Emu\n");

    // push "end emu" marker address
    PushExit(emu);
    // Setup the GS segment:
    if(shared_global) {
        emu->globals = globals;
        emu->shared_global = shared_global;
    } else {
        emu->globals = calloc(1, 256);  // arbitrary 256 byte size?
        // calc canary...
        uint8_t canary[4];
        for (int i=0; i<4; ++i) canary[i] = 1 +  getrand(255);
        canary[getrand(4)] = 0;
        memcpy(emu->globals+0x14, canary, sizeof(canary));  // put canary in place
        printf_log(LOG_DEBUG, "Setting up canary (for Stack protector) at GS:0x14, value:%08X\n", *(uint32_t*)canary);
        emu->shared_global = (int*)calloc(1, sizeof(int));
    }
    (*emu->shared_global)++;
}

void SetTraceEmu(x86emu_t *emu, uintptr_t trace_start, uintptr_t trace_end)
{
    printf_log(LOG_INFO, "Setting trace only between %p and %p\n", trace_start, trace_end);
    emu->trace_start = trace_start;
    emu->trace_end = trace_end;
}

void FreeX86Emu(x86emu_t **x86emu)
{
    if(!x86emu)
        return;
    printf_log(LOG_DEBUG, "Free a X86 Emu (%p)\n", *x86emu);
    if((*x86emu)->dec)
        DeleteX86TraceDecoder(&(*x86emu)->dec);
    if((*x86emu)->shared_global && !(*(*x86emu)->shared_global)--) {
        if((*x86emu)->globals)
            free((*x86emu)->globals);
        free((*x86emu)->shared_global);
    }
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
    buff[0] = '\0';
    for (int i=_AX; i<=_DI; ++i) {
        sprintf(tmp, "%s=%08X ", regname[i], emu->regs[i].dword[0]);
        strcat(buff, tmp);

        if (i==3) {
#define FLAG_CHAR(f) (ACCESS_FLAG(F_##f##F)) ? #f : "-"
            sprintf(tmp, "FLAGS=%s%s%s%s%s%s\n", FLAG_CHAR(O), FLAG_CHAR(C), FLAG_CHAR(P), FLAG_CHAR(A), FLAG_CHAR(Z), FLAG_CHAR(S));
            strcat(buff, tmp);
#undef FLAG_CHAR
        }
    }
    sprintf(tmp, "EIP=%08X ", R_EIP);
    strcat(buff, tmp);
    return buff;
}

void StopEmu(x86emu_t* emu, const char* reason)
{
    emu->quit = 1;
    printf_log(LOG_NONE, reason);
    // dump stuff...
    printf_log(LOG_NONE, "CPU Regs=%s\n", DumpCPURegs(emu));
    // TODO: stack, memory/instruction around EIP, etc..
}