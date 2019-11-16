#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>

#include "debug.h"
#include "box86stack.h"
#include "x86emu.h"
#include "x86emu_private.h"
#include "x87emu_private.h"
#include "box86context.h"
#include "x86trace.h"
#include "x86run.h"
#include "x86run_private.h"

typedef struct cleanup_s {
    void*       f;
    int         arg;
    void*       a;
} cleanup_t;

static uint32_t x86emu_parity_tab[8] =
{
	0x96696996,
	0x69969669,
	0x69969669,
	0x96696996,
	0x69969669,
	0x96696996,
	0x96696996,
	0x69969669,
};

static uint8_t EndEmuMarker[] = {0xcc, 'S', 'C', 0, 0, 0, 0};
void PushExit(x86emu_t* emu)
{
    Push(emu, (uint32_t)&EndEmuMarker);
}

void* GetExit()
{
    return &EndEmuMarker;
}

x86emu_t *NewX86Emu(box86context_t *context, uintptr_t start, uintptr_t stack, int stacksize, int ownstack)
{
    printf_log(LOG_DEBUG, "Allocate a new X86 Emu, with EIP=%p and Stack=%p/0x%X\n", (void*)start, (void*)stack, stacksize);

    x86emu_t *emu = (x86emu_t*)calloc(1, sizeof(x86emu_t));
    emu->context = context;
    // setup cpu helpers
    for (int i=0; i<8; ++i)
        emu->sbiidx[i] = &emu->regs[i];
    emu->sbiidx[4] = &emu->zero;
    emu->x86emu_parity_tab = x86emu_parity_tab;
    emu->packed_eflags.x32 = 0x202; // default flags?
    UnpackFlags(emu);
    // own stack?
    if(ownstack)
        emu->stack = (void*)stack;
    emu->init_stack = (void*)stack;
    emu->size_stack = stacksize;
    // set default value
    R_EIP = start;
    R_ESP = stack + stacksize;
    // fake init of segments...
    emu->segs[_CS] = 0x73;
    emu->segs[_DS] = emu->segs[_ES] = emu->segs[_SS] = 0x7b;
    emu->segs[_FS] = 0;
    emu->segs[_GS] = 0x33;
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

void SetupX86Emu(x86emu_t *emu)
{
    printf_log(LOG_DEBUG, "Setup X86 Emu\n");
}

void SetTraceEmu(x86emu_t *emu, uintptr_t trace_start, uintptr_t trace_end)
{
    if(emu->dec) {
        if (trace_end == 0) {
            printf_log(LOG_INFO, "Setting trace\n");
        } else {
            if(trace_end!=1) {  // 0-1 is basically no trace, so don't printf it...
                printf_log(LOG_INFO, "Setting trace only between %p and %p\n", (void*)trace_start, (void*)trace_end);
            }
        }
    }
    emu->trace_start = trace_start;
    emu->trace_end = trace_end;
}

void AddCleanup(x86emu_t *emu, void *p)
{
    if(emu->clean_sz == emu->clean_cap) {
        emu->clean_cap += 4;
        emu->cleanups = (cleanup_t*)realloc(emu->cleanups, sizeof(cleanup_t)*emu->clean_cap);
    }
    emu->cleanups[emu->clean_sz].arg = 0;
    emu->cleanups[emu->clean_sz++].f = p;
}

void AddCleanup1Arg(x86emu_t *emu, void *p, void* a)
{
    if(emu->clean_sz == emu->clean_cap) {
        emu->clean_cap += 4;
        emu->cleanups = (cleanup_t*)realloc(emu->cleanups, sizeof(cleanup_t)*emu->clean_cap);
    }
    emu->cleanups[emu->clean_sz].arg = 1;
    emu->cleanups[emu->clean_sz].a = a;
    emu->cleanups[emu->clean_sz++].f = p;
}

void CallCleanup(x86emu_t *emu, void* p)
{
    for(int i=emu->clean_sz-1; i>=0; --i) {
        if(p==emu->cleanups[i].f) {
            printf_log(LOG_DEBUG, "Call cleanup #%d\n", i);
            if(emu->cleanups[i].arg)
                Push(emu, (uintptr_t)emu->cleanups[i].a);
            PushExit(emu);
            emu->ip.dword[0] = (uintptr_t)(emu->cleanups[i].f);
            Run(emu, 0);
            emu->quit = 0;
            // now remove the cleanup
            if(i!=emu->clean_sz-1)
                memmove(emu->cleanups+i, emu->cleanups+i+1, (emu->clean_sz-i-1)*sizeof(cleanup_t));
            --emu->clean_sz;
            return;
        }
    }
}

void CallAllCleanup(x86emu_t *emu)
{
    /* disabling for now...
    for(int i=emu->clean_sz-1; i>=0; --i) {
        printf_log(LOG_DEBUG, "Call cleanup #%d\n", i);
        if(emu->cleanups[i].arg)
            Push(emu, (uintptr_t)emu->cleanups[i].a);
        PushExit(emu);
        emu->ip.dword[0] = (uintptr_t)(emu->cleanups[i].f);
        Run(emu);
        emu->quit = 0;
    }
    */
    emu->clean_sz = 0;
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
    CallAllCleanup(*emu);
    free((*emu)->cleanups);

    free((*emu)->scratch);

    free((*emu)->stack);

    free(*emu);
    *emu = NULL;
}

void CloneEmu(x86emu_t *newemu, const x86emu_t* emu)
{
	memcpy(newemu->regs, emu->regs, sizeof(emu->regs));
    memcpy(&newemu->ip, &emu->ip, sizeof(emu->ip));
	memcpy(&newemu->packed_eflags, &emu->packed_eflags, sizeof(emu->packed_eflags));
    memcpy(&newemu->flags, &emu->flags, sizeof(emu->flags));
    newemu->old_ip = emu->old_ip;
    memcpy(newemu->segs, emu->segs, sizeof(emu->segs));
	memcpy(newemu->fpu, emu->fpu, sizeof(emu->fpu));
    memcpy(newemu->fpu_ld, emu->fpu_ld, sizeof(emu->fpu_ld));
    memcpy(newemu->fpu_ll, emu->fpu_ll, sizeof(emu->fpu_ll));
	memcpy(newemu->p_regs, emu->p_regs, sizeof(emu->p_regs));
	newemu->cw = emu->cw;
    newemu->cw_mask_all = emu->cw_mask_all;
	memcpy(&newemu->sw, &emu->sw, sizeof(emu->sw));
	newemu->top = emu->top;
    newemu->fpu_stack = emu->fpu_stack;
	memcpy(&newemu->round, &emu->round, sizeof(emu->round));
    memcpy(newemu->mmx, emu->mmx, sizeof(emu->mmx));
    memcpy(newemu->xmm, emu->xmm, sizeof(emu->xmm));
    newemu->mxcsr = emu->mxcsr;
    memcpy(&newemu->zero, &emu->zero, sizeof(emu->zero));
    memcpy(newemu->sbiidx, emu->sbiidx, sizeof(emu->sbiidx));
    newemu->quit = emu->quit;
    newemu->error = emu->error;
    // addapt R_ESP to new stack frame
    uintptr_t oldst = (uintptr_t)((emu->stack)?emu->stack:emu->context->stack);
    uintptr_t newst = (uintptr_t)((newemu->stack)?newemu->stack:newemu->context->stack);
    newemu->regs[_SP].dword[0] = emu->regs[_SP].dword[0] + (intptr_t)(newst - oldst);
}

box86context_t* GetEmuContext(x86emu_t* emu)
{
    return emu->context;
}

uint32_t GetEAX(x86emu_t *emu)
{
    return R_EAX;
}
uint64_t GetEDXEAX(x86emu_t *emu)
{
    return ((uint64_t)R_EAX)|(((uint64_t)R_EDX)<<32);
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
void SetEIP(x86emu_t *emu, uint32_t v)
{
    R_EIP = v;
}
void SetESI(x86emu_t *emu, uint32_t v)
{
    R_ESI = v;
}
void SetEDI(x86emu_t *emu, uint32_t v)
{
    R_EDI = v;
}
void SetEBP(x86emu_t *emu, uint32_t v)
{
    R_EBP = v;
}
void SetESP(x86emu_t *emu, uint32_t v)
{
    R_ESP = v;
}
uint32_t GetESP(x86emu_t *emu)
{
    return R_ESP;
}


void ResetFlags(x86emu_t *emu)
{
    emu->df = d_none;
}

const char* DumpCPURegs(x86emu_t* emu, uintptr_t ip)
{
    static char buff[800];
    char* regname[] = {"EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"};
    char tmp[80];
    buff[0] = '\0';
    if(trace_xmm) {
        // do xmm reg is needed
        for(int i=0; i<8; ++i) {
            sprintf(tmp, "%d:%016llx%016llx", i, emu->xmm[i].q[1], emu->xmm[i].q[0]);
            strcat(buff, tmp);
            if ((i&3)==3) strcat(buff, "\n"); else strcat(buff, " ");
        }
    }
    // start with FPU regs...
    if(emu->fpu_stack) {
        for (int i=0; i<emu->fpu_stack; i++) {
            #ifdef USE_FLOAT
            sprintf(tmp, "ST%d=%f", i, emu->fpu[(emu->top+i)&7].f);
            #else
            sprintf(tmp, "ST%d=%f", i, emu->fpu[(emu->top+i)&7].d);
            #endif
            strcat(buff, tmp);
            int c = 10-strlen(tmp);
            if(c<1) c=1;
            while(c--) strcat(buff, " ");
            if(i==3) strcat(buff, "\n");
        }
        strcat(buff, "\n");
    }
    for (int i=_AX; i<=_DI; ++i) {
        sprintf(tmp, "%s=%08x ", regname[i], emu->regs[i].dword[0]);
        strcat(buff, tmp);

        if (i==3) {
            if(emu->df) {
                strcat(buff, "FLAGS=??????\n");
            } else {
#define FLAG_CHAR(f) (ACCESS_FLAG(F_##f##F)) ? #f : "-"
                sprintf(tmp, "FLAGS=%s%s%s%s%s%s\n", FLAG_CHAR(O), FLAG_CHAR(C), FLAG_CHAR(P), FLAG_CHAR(A), FLAG_CHAR(Z), FLAG_CHAR(S));
                strcat(buff, tmp);
#undef FLAG_CHAR
            }
        }
    }
    sprintf(tmp, "EIP=%08x ", ip);
    strcat(buff, tmp);
    return buff;
}

void StopEmu(x86emu_t* emu, const char* reason)
{
    emu->quit = 1;
    printf_log(LOG_NONE, "%s", reason);
    // dump stuff...
    printf_log(LOG_NONE, "CPU Regs=%s\n", DumpCPURegs(emu, R_EIP));
    // TODO: stack, memory/instruction around EIP, etc..
}

void UnimpOpcode(x86emu_t* emu)
{
    R_EIP = emu->old_ip;

    int tid = syscall(SYS_gettid);
    printf_log(LOG_NONE, "%04d|%p: Unimplemented Opcode %02X %02X %02X %02X %02X %02X %02X %02X\n", 
        tid, (void*)emu->old_ip,
        Peek(emu, 0), Peek(emu, 1), Peek(emu, 2), Peek(emu, 3),
        Peek(emu, 4), Peek(emu, 5), Peek(emu, 6), Peek(emu, 7));
    emu->quit=1;
    emu->error |= ERR_UNIMPL;
}

void EmuCall(x86emu_t* emu, uintptr_t addr)
{
    uint32_t old_ebx = R_EBX;
    uint32_t old_edi = R_EDI;
    uint32_t old_esi = R_ESI;
    uint32_t old_ebp = R_EBP;
    uint32_t old_eip = R_EIP;
    PushExit(emu);
    R_EIP = addr;
    emu->df = d_none;
    Run(emu, 0);
    emu->quit = 0;  // reset Quit flags...
    emu->df = d_none;
    R_EBX = old_ebx;
    R_EDI = old_edi;
    R_ESI = old_esi;
    R_EBP = old_ebp;
    R_EIP = old_eip;  // and set back instruction pointer
}

uint64_t ReadTSC(x86emu_t* emu)
{
    // Read the TimeStamp Counter as 64bits.
    // this is supposed to be the number of instrunctions executed since last reset
#if defined(__i386__)
  uint64_t ret;
  __asm__ volatile("rdtsc" : "=A"(ret));
  return ret;
#elif defined(__ARM_ARCH)
#if (__ARM_ARCH >= 6)
  uint32_t pmccntr;
  uint32_t pmuseren;
  uint32_t pmcntenset;
  // Read the user mode perf monitor counter access permissions.
  asm volatile("mrc p15, 0, %0, c9, c14, 0" : "=r"(pmuseren));
  if (pmuseren & 1) {  // Allows reading perfmon counters for user mode code.
    asm volatile("mrc p15, 0, %0, c9, c12, 1" : "=r"(pmcntenset));
    if (pmcntenset & 0x80000000ul) {  // Is it counting?
      asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(pmccntr));
      // The counter is set up to count every 64th cycle, but it's x86 we want...,so *64 is probably too much
      return (uint64_t)(pmccntr) * 32;
    }
  }
#endif
#endif
// fall back to gettime...
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint64_t)(tv.tv_sec) * 1000000 + tv.tv_usec;
}
