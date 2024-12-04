#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/mman.h>

#include "debug.h"
#include "box86stack.h"
#include "x86emu.h"
#include "x86emu_private.h"
#include "x87emu_private.h"
#include "box86context.h"
#include "x86run.h"
#include "x86run_private.h"
#include "callback.h"
#include "bridge.h"
#ifdef HAVE_TRACE
#include "x86trace.h"
#endif
#ifdef DYNAREC
#include "custommem.h"
#endif
// for the applyFlushTo0
#ifdef __i386__
#include <immintrin.h>
#elif defined(__arm__)
#else
#warning Architecture cannot follow SSE Flush to 0 flag
#endif

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

static void internalX86Setup(x86emu_t* emu, box86context_t *context, uintptr_t start, uintptr_t stack, int stacksize, int ownstack)
{
    emu->context = context;
    // setup cpu helpers
    for (int i=0; i<8; ++i)
        emu->sbiidx[i] = &emu->regs[i];
    emu->sbiidx[4] = &emu->zero;
    emu->x86emu_parity_tab = x86emu_parity_tab;
    emu->eflags.x32 = 0x202; // default flags?
    // own stack?
    emu->stack2free = (ownstack)?(void*)stack:NULL;
    emu->init_stack = (void*)stack;
    emu->size_stack = stacksize;
    // set default value
    R_EIP = start;
    R_ESP = (stack + stacksize) & ~7;   // align stack start, always
    // fake init of segments...
    emu->segs[_CS] = 0x73;
    emu->segs[_DS] = emu->segs[_ES] = emu->segs[_SS] = 0x7b;
    emu->segs[_FS] = default_fs;
    emu->segs[_GS] = 0x33;
    // setup fpu regs
    reset_fpu(emu);
    emu->mxcsr.x32 = 0x1f80;  
}

EXPORTDYN
x86emu_t *NewX86Emu(box86context_t *context, uintptr_t start, uintptr_t stack, int stacksize, int ownstack)
{
    printf_log(LOG_DEBUG, "%04d|Allocate a new X86 Emu, with EIP=%p and Stack=%p/0x%X\n", GetTID(), (void*)start, (void*)stack, stacksize);

    x86emu_t *emu = (x86emu_t*)box_calloc(1, sizeof(x86emu_t));

    internalX86Setup(emu, context, start, stack, stacksize, ownstack);

    return emu;
}

x86emu_t *NewX86EmuFromStack(x86emu_t* emu, box86context_t *context, uintptr_t start, uintptr_t stack, int stacksize, int ownstack)
{
    printf_log(LOG_DEBUG, "%04d|New X86 Emu from stack, with EIP=%p and Stack=%p/0x%X\n", GetTID(), (void*)start, (void*)stack, stacksize);

    internalX86Setup(emu, context, start, stack, stacksize, ownstack);
    
    return emu;
}

EXPORTDYN
void SetupX86Emu(x86emu_t *emu)
{
    (void)emu;
    printf_log(LOG_DEBUG, "Setup X86 Emu\n");
}

void SetTraceEmu(uintptr_t start, uintptr_t end)
{
    if(my_context->zydis) {
        if (end == 0) {
            printf_log(LOG_INFO, "Setting trace\n");
        } else {
            if(end!=1) {  // 0-1 is basically no trace, so don't printf it...
                printf_log(LOG_INFO, "Setting trace only between %p and %p\n", (void*)start, (void*)end);
            }
        }
    }
    trace_start = start;
    trace_end = end;
}

void AddCleanup(x86emu_t *emu, void *p)
{
    (void)emu;
    if(my_context->clean_sz == my_context->clean_cap) {
        my_context->clean_cap += 4;
        my_context->cleanups = (cleanup_t*)box_realloc(my_context->cleanups, sizeof(cleanup_t)*my_context->clean_cap);
    }
    my_context->cleanups[my_context->clean_sz].arg = 0;
    my_context->cleanups[my_context->clean_sz].a = NULL;
    my_context->cleanups[my_context->clean_sz++].f = p;
}

void AddCleanup1Arg(x86emu_t *emu, void *p, void* a)
{
    (void)emu;
    if(my_context->clean_sz == my_context->clean_cap) {
        my_context->clean_cap += 4;
        my_context->cleanups = (cleanup_t*)box_realloc(my_context->cleanups, sizeof(cleanup_t)*my_context->clean_cap);
    }
    my_context->cleanups[my_context->clean_sz].arg = 1;
    my_context->cleanups[my_context->clean_sz].a = a;
    my_context->cleanups[my_context->clean_sz++].f = p;
}

void CallCleanup(x86emu_t *emu, void* p)
{
    printf_log(LOG_DEBUG, "Calling atexit registered functions for %p mask\n", p);
    for(int i=my_context->clean_sz-1; i>=0; --i) {
        if(p==my_context->cleanups[i].f) {
            printf_log(LOG_DEBUG, "Call cleanup #%d\n", i);
            RunFunctionWithEmu(emu, 0, (uintptr_t)(my_context->cleanups[i].f), my_context->cleanups[i].arg, my_context->cleanups[i].a );
            // now remove the cleanup
            if(i!=my_context->clean_sz-1)
                memmove(my_context->cleanups+i, my_context->cleanups+i+1, (my_context->clean_sz-i-1)*sizeof(cleanup_t));
            --my_context->clean_sz;
        }
    }
}

void CallAllCleanup(x86emu_t *emu)
{
    printf_log(LOG_DEBUG, "Calling atexit registered functions\n");
    for(int i=my_context->clean_sz-1; i>=0; --i) {
        printf_log(LOG_DEBUG, "Call cleanup #%d\n", i);
        RunFunctionWithEmu(emu, 0, (uintptr_t)(my_context->cleanups[i].f), my_context->cleanups[i].arg, my_context->cleanups[i].a );
    }
    my_context->clean_sz = 0;
    box_free(my_context->cleanups);
    my_context->cleanups = NULL;
}

static void internalFreeX86(x86emu_t* emu)
{
    if(emu->stack2free)
        munmap(emu->stack2free, emu->size_stack);
}

EXPORTDYN
void FreeX86Emu(x86emu_t **emu)
{
    if(!emu)
        return;
    printf_log(LOG_DEBUG, "%04d|Free a X86 Emu (%p)\n", GetTID(), *emu);

    if((*emu)->test.emu) {
        internalFreeX86((*emu)->test.emu);
        box_free((*emu)->test.emu);
        (*emu)->test.emu = NULL;
    }
    internalFreeX86(*emu);

    box_free(*emu);
    *emu = NULL;
}

void FreeX86EmuFromStack(x86emu_t **emu)
{
    if(!emu)
        return;
    printf_log(LOG_DEBUG, "%04d|Free a X86 Emu from stack (%p)\n", GetTID(), *emu);

    internalFreeX86(*emu);
}

void CloneEmu(x86emu_t *newemu, const x86emu_t* emu)
{
	memcpy(newemu->regs, emu->regs, sizeof(emu->regs));
    memcpy(&newemu->ip, &emu->ip, sizeof(emu->ip));
	memcpy(&newemu->eflags, &emu->eflags, sizeof(emu->eflags));
    newemu->old_ip = emu->old_ip;
    memcpy(newemu->segs, emu->segs, sizeof(emu->segs));
    memset(newemu->segs_serial, 0, sizeof(newemu->segs_serial));
	memcpy(newemu->x87, emu->x87, sizeof(emu->x87));
	memcpy(newemu->mmx, emu->mmx, sizeof(emu->mmx));
    memcpy(newemu->fpu_ld, emu->fpu_ld, sizeof(emu->fpu_ld));
    memcpy(newemu->fpu_ll, emu->fpu_ll, sizeof(emu->fpu_ll));
	newemu->fpu_tags = emu->fpu_tags;
	newemu->cw = emu->cw;
	newemu->sw = emu->sw;
	newemu->top = emu->top;
    newemu->fpu_stack = emu->fpu_stack;
    memcpy(newemu->xmm, emu->xmm, sizeof(emu->xmm));
    newemu->df = emu->df;
    newemu->op1 = emu->op1;
    newemu->op2 = emu->op2;
    newemu->res = emu->res;
    newemu->mxcsr = emu->mxcsr;
    newemu->quit = emu->quit;
    newemu->error = emu->error;
}

void CopyEmu(x86emu_t *newemu, const x86emu_t* emu)
{
	memcpy(newemu->regs, emu->regs, sizeof(emu->regs));
    memcpy(&newemu->ip, &emu->ip, sizeof(emu->ip));
	memcpy(&newemu->eflags, &emu->eflags, sizeof(emu->eflags));
    newemu->old_ip = emu->old_ip;
    memcpy(newemu->segs, emu->segs, sizeof(emu->segs));
    memcpy(newemu->segs_serial, emu->segs_serial, sizeof(emu->segs_serial));
    memcpy(newemu->segs_offs, emu->segs_offs, sizeof(emu->segs_offs));
	memcpy(newemu->x87, emu->x87, sizeof(emu->x87));
	memcpy(newemu->mmx, emu->mmx, sizeof(emu->mmx));
    memcpy(newemu->xmm, emu->xmm, sizeof(emu->xmm));
    memcpy(newemu->fpu_ld, emu->fpu_ld, sizeof(emu->fpu_ld));
    memcpy(newemu->fpu_ll, emu->fpu_ll, sizeof(emu->fpu_ll));
	newemu->fpu_tags = emu->fpu_tags;
	newemu->cw = emu->cw;
	newemu->sw = emu->sw;
	newemu->top = emu->top;
    newemu->fpu_stack = emu->fpu_stack;
    newemu->df = emu->df;
    newemu->op1 = emu->op1;
    newemu->op2 = emu->op2;
    newemu->res = emu->res;
    newemu->mxcsr = emu->mxcsr;
    newemu->quit = emu->quit;
    newemu->error = emu->error;
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
void SetFS(x86emu_t *emu, uint16_t v)
{
    emu->segs[_FS] = v;
}
uint16_t GetFS(x86emu_t *emu)
{
    return emu->segs[_FS];
}


void ResetFlags(x86emu_t *emu)
{
    emu->df = d_none;
}

const char* DumpCPURegs(x86emu_t* emu, uintptr_t ip)
{
    static char buff[800];
    static const char* regname[] = {"EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"};
    static const char* segname[] = {"ES", "CS", "SS", "DS", "FS", "GS"};
    char tmp[80];
    buff[0] = '\0';
    if(trace_emm) {
        // do emm reg is needed
        for(int i=0; i<8; ++i) {
            sprintf(tmp, "mm%d:%016llx", i, emu->mmx[i].q);
            strcat(buff, tmp);
            if ((i&3)==3) strcat(buff, "\n"); else strcat(buff, " ");
        }
    }
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
        int stack = emu->fpu_stack;
        if(stack>8) stack = 8;
        for (int i=0; i<stack; i++) {
            sprintf(tmp, "ST%d=%f", i, ST(i).d);
            strcat(buff, tmp);
            int c = 10-strlen(tmp);
            if(c<1) c=1;
            while(c--) strcat(buff, " ");
            if(i==3) strcat(buff, "\n");
        }
        sprintf(tmp, " C3210 = %d%d%d%d", emu->sw.f.F87_C3, emu->sw.f.F87_C2, emu->sw.f.F87_C1, emu->sw.f.F87_C0);
        strcat(buff, tmp);
        strcat(buff, "\n");
    }
    for (int i=0; i<6; ++i) {
            sprintf(tmp, "%s=0x%04x", segname[i], emu->segs[i]);
            strcat(buff, tmp);
            if(i!=_GS)
                strcat(buff, " ");
    }
    strcat(buff, "\n");
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
    printf_log(LOG_NONE, "\n==== CPU Registers ====\n%s\n", DumpCPURegs(emu, R_EIP));
    printf_log(LOG_NONE, "\n======== Stack ========\nStack is from 0x%x to 0x%0x\n", R_EBP, R_ESP);
    if (R_EBP == R_ESP) {
        printf_log(LOG_NONE, "EBP = ESP: leaf function detected; next 128 bytes should be either data or random.\n");
    } else {
        // TODO: display stack if operation should be allowed (to avoid crashes)
        /* for (uint32_t *sp = R_EBP; sp >= R_ESP; --sp) {
        } */
    }
    printf_log(LOG_NONE, "\n====== Past data ======\nOld IP: %tX\n", emu->old_ip);
#ifdef HAVE_TRACE
    printf_log(LOG_NONE, "%s\n", DecodeX86Trace(my_context->dec, emu->old_ip));
#endif
}

void UnimpOpcode(x86emu_t* emu)
{
    R_EIP = emu->old_ip;

    int tid = syscall(SYS_gettid);
    printf_log(LOG_NONE, "%04d|%p: Unimplemented Opcode (%02X) %02X %02X %02X %02X %02X %02X %02X %02X\n", 
        tid, (void*)emu->old_ip, Peek(emu, -1),
        Peek(emu, 0), Peek(emu, 1), Peek(emu, 2), Peek(emu, 3),
        Peek(emu, 4), Peek(emu, 5), Peek(emu, 6), Peek(emu, 7));
}

void EmuCall(x86emu_t* emu, uintptr_t addr)
{
    uint32_t old_esp = R_ESP;
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
    if(emu->flags.quitonlongjmp && emu->flags.longjmp) {
        if(emu->flags.quitonlongjmp==1)
            emu->flags.longjmp = 0;   // don't change anything because of the longjmp
    } else {
        R_EBX = old_ebx;
        R_EDI = old_edi;
        R_ESI = old_esi;
        R_EBP = old_ebp;
        R_ESP = old_esp;
        R_EIP = old_eip;  // and set back instruction pointer
    }
}

#if defined(__ARM_ARCH)
#if (__ARM_ARCH >= 6)
static inline unsigned int arm_perf (void)
{
  unsigned int value;
  // Read CCNT Register
  asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(value));
  return value;
}
static inline void init_perfcounters (int32_t do_reset, int32_t enable_divider)
{
  // in general enable all counters (including cycle counter)
  int32_t value = 1;

  // peform reset:
  if (do_reset)
  {
    value |= 2;     // reset all counters to zero.
    value |= 4;     // reset cycle counter to zero.
  }

  if (enable_divider)
    value |= 8;     // enable "by 64" divider for CCNT.

  value |= 16;

  // program the performance-counter control-register:
  asm volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(value));

  // enable all counters:
  asm volatile ("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000000f));

  // clear overflows:
  asm volatile ("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x8000000f));
}

#endif
#endif

uint64_t ReadTSC(x86emu_t* emu)
{
    (void)emu;
    // Read the TimeStamp Counter as 64bits.
    // this is supposed to be the number of instructions executed since last reset
#if defined(__i386__)
    uint64_t ret;
    __asm__ volatile("rdtsc" : "=A"(ret));
    return ret;
#elif defined(__ARM_ARCH)
#if 0
#if (__ARM_ARCH >= 6)
    static int init = 1;
    if(init) {
        init_perfcounters(1, 1);
        init = 0;
    }
    uint32_t perf = arm_perf();
    if(perf) {
        return ((uint64_t)perf)*64;
    }
#endif
#endif
#endif
    // fall back to gettime...
#ifndef NOGETCLOCK
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    return (uint64_t)(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000000 + tv.tv_usec;
#endif
}

void ResetSegmentsCache(x86emu_t *emu)
{
    if(!emu)
        return;
    memset(emu->segs_serial, 0, sizeof(emu->segs_serial));
}

void applyFlushTo0(x86emu_t* emu)
{
#ifdef __i386__
    _mm_setcsr(_mm_getcsr() | (emu->mxcsr.x32&0x8040));
#elif defined(__arm__)
    uint64_t fpscr = __builtin_arm_get_fpscr();
    fpscr &= ~(1<<24);                     // clear bit FZ (24)
    fpscr |= (emu->mxcsr.f.MXCSR_FZ)<<24;  // set FZ as mxcsr FZ
    __builtin_arm_set_fpscr(fpscr);
#else
    (void)emu;
#endif
}
