#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86emu_private.h"
#include "x87emu_private.h"
#include "box86context.h"
#include "x86trace.h"
#include "x86run.h"

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
    printf_log(LOG_DEBUG, "Allocate a new X86 Emu, with EIP=%p and Stack=%p/0x%X\n", (void*)start, (void*)stack, stacksize);

    x86emu_t *emu = (x86emu_t*)calloc(1, sizeof(x86emu_t));
    emu->context = context;
    // setup cpu helpers
    for (int i=0; i<8; ++i)
        emu->sbiidx[i] = &emu->regs[i];
    emu->sbiidx[4] = &emu->zero;
    emu->eflags.x32 = 0x02; // default flags?
    // set default value
    R_EIP = start;
    R_ESP = stack + stacksize;
    // setup fpu regs
    reset_fpu(emu);
    // that should be enough
    emu->scratch = (uint32_t*)calloc(200, sizeof(uint32_t));


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
    if (trace_end == 0) {
        printf_log(LOG_INFO, "Setting trace\n");
    } else {
        printf_log(LOG_INFO, "Setting trace only between %p and %p\n", (void*)trace_start, (void*)trace_end);
    }
    emu->trace_start = trace_start;
    emu->trace_end = trace_end;
}

void AddCleanup(x86emu_t *emu, void *p)
{
    if(emu->clean_sz == emu->clean_cap) {
        emu->clean_cap += 4;
        emu->cleanups = (void**)realloc(emu->cleanups, sizeof(void*)*emu->clean_cap);
    }
    emu->cleanups[emu->clean_sz++] = p;
}

void FreeX86Emu(x86emu_t **emu)
{
    if(!emu)
        return;
    printf_log(LOG_DEBUG, "Free a X86 Emu (%p)\n", *emu);
    // stop trace now
    if((*emu)->dec)
        DeleteX86TraceDecoder(&(*emu)->dec);
    (*emu)->dec = NULL;
    // call atexit and fini first!
    for(int i=0; i<(*emu)->clean_sz; ++i) {
        printf_log(LOG_DEBUG, "Call cleanup #%d\n", i);
        PushExit(*emu);
        (*emu)->ip.dword[0] = (uintptr_t)((*emu)->cleanups[i]);
        Run(*emu);
    }
    free((*emu)->cleanups);
    if((*emu)->shared_global && !(*(*emu)->shared_global)--) {
        if((*emu)->globals)
            free((*emu)->globals);
        free((*emu)->shared_global);
    }

    free((*emu)->scratch);

    free(*emu);
    *emu = NULL;
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
    // start with FPU regs...
    if(emu->fpu_stack) {
        for (int i=0; i<emu->fpu_stack; i++) {
            sprintf(tmp, "ST%d=%f", i, emu->fpu[(emu->top+i)&7].d);
            strcat(buff, tmp);
            int c = 10-strlen(tmp);
            if(c<1) c=1;
            while(c--) strcat(buff, " ");
            if(i==3) strcat(buff, "\n");
        }
        strcat(buff, "\n");
    }
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
    printf_log(LOG_NONE, "%s", reason);
    // dump stuff...
    printf_log(LOG_NONE, "CPU Regs=%s\n", DumpCPURegs(emu));
    // TODO: stack, memory/instruction around EIP, etc..
}

void UnimpOpcode(x86emu_t* emu)
{
    R_EIP = emu->old_ip;

    printf_log(LOG_NONE, "%p: Unimplemented Opcode %02X %02X %02X %02X %02X %02X %02X %02X\n", 
        (void*)emu->old_ip,
        Peek(emu, 0), Peek(emu, 1), Peek(emu, 2), Peek(emu, 3),
        Peek(emu, 4), Peek(emu, 5), Peek(emu, 6), Peek(emu, 7));
    emu->quit=1;
    emu->error |= ERR_UNIMPL;
}
