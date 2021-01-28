#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <syscall.h>
#include <stddef.h>
#include <stdarg.h>
#include <ucontext.h>
#include <setjmp.h>
#include <sys/mman.h>

#include "box86context.h"
#include "debug.h"
#include "x86emu.h"
#include "emu/x86emu_private.h"
#include "emu/x86run_private.h"
#include "signals.h"
#include "box86stack.h"
#include "dynarec.h"
#include "callback.h"
#include "x86run.h"
#include "elfloader.h"
#include "threads.h"
#include "emu/x87emu_private.h"
#include "custommem.h"
#ifdef DYNAREC
#include "dynablock.h"
#include "../dynarec/dynablock_private.h"
#endif


/* Definitions taken from the kernel headers.  */
enum
{
  REG_GS = 0,
# define REG_GS         REG_GS
  REG_FS,
# define REG_FS         REG_FS
  REG_ES,
# define REG_ES         REG_ES
  REG_DS,
# define REG_DS         REG_DS
  REG_EDI,
# define REG_EDI        REG_EDI
  REG_ESI,
# define REG_ESI        REG_ESI
  REG_EBP,
# define REG_EBP        REG_EBP
  REG_ESP,
# define REG_ESP        REG_ESP
  REG_EBX,
# define REG_EBX        REG_EBX
  REG_EDX,
# define REG_EDX        REG_EDX
  REG_ECX,
# define REG_ECX        REG_ECX
  REG_EAX,
# define REG_EAX        REG_EAX
  REG_TRAPNO,
# define REG_TRAPNO        REG_TRAPNO
  REG_ERR,
# define REG_ERR        REG_ERR
  REG_EIP,
# define REG_EIP        REG_EIP
  REG_CS,
# define REG_CS                REG_CS
  REG_EFL,
# define REG_EFL        REG_EFL
  REG_UESP,
# define REG_UESP        REG_UESP
  REG_SS
# define REG_SS        REG_SS
};

typedef uint32_t i386_gregset_t[19];
struct i386_fpreg
{
  uint16_t significand[4];
  uint16_t exponent;
}__attribute__((packed));

struct i386_fpxreg
{
  unsigned short significand[4];
  unsigned short exponent;
  unsigned short padding[3];
}__attribute__((packed));

struct i386_xmmreg
{
  uint32_t          element[4];
}__attribute__((packed));

struct i386_fpstate
{
  /* Regular FPU environment.  */
  uint32_t          cw;
  uint32_t          sw;
  uint32_t          tag;
  uint32_t          ipoff;
  uint32_t          cssel;
  uint32_t          dataoff;
  uint32_t          datasel;
  struct i386_fpreg _st[8];
  unsigned short    status;
  unsigned short    magic;
  /* FXSR FPU environment.  */
  uint32_t          _fxsr_env[6];
  uint32_t          mxcsr;
  uint32_t          reserved;
  struct i386_fpxreg _fxsr_st[8];
  struct i386_xmmreg _xmm[8];
  uint32_t          padding[56];
}__attribute__((packed));

typedef struct i386_fpstate *i386_fpregset_t;

typedef struct
  {
    void *ss_sp;
    int ss_flags;
    size_t ss_size;
  } i386_stack_t;

/*
another way to see the sigcontext
struct sigcontext
{
  unsigned short gs, __gsh;
  unsigned short fs, __fsh;
  unsigned short es, __esh;
  unsigned short ds, __dsh;
  unsigned long edi;
  unsigned long esi;
  unsigned long ebp;
  unsigned long esp;
  unsigned long ebx;
  unsigned long edx;
  unsigned long ecx;
  unsigned long eax;
  unsigned long trapno;
  unsigned long err;
  unsigned long eip;
  unsigned short cs, __csh;
  unsigned long eflags;
  unsigned long esp_at_signal;
  unsigned short ss, __ssh;
  struct _fpstate * fpstate;
  unsigned long oldmask;
  unsigned long cr2;
};
*/
typedef struct
  {
    i386_gregset_t gregs;
    i386_fpregset_t fpregs;
    uint32_t oldmask;
    uint32_t cr2;
  } i386_mcontext_t;

// /!\ signal sig_set is different than glibc __sig_set
#define _NSIG_WORDS (64 / 32)
typedef unsigned long i386_old_sigset_t;
typedef struct {
    unsigned long sig[_NSIG_WORDS];
} i386_sigset_t;

struct i386_xsave_hdr_struct {
 	uint64_t xstate_bv;
 	uint64_t reserved1[2];
 	uint64_t reserved2[5];
};

struct i386_xstate {
	/*
	 * Applications need to refer to fpstate through fpstate pointer
	 * in sigcontext. Not here directly.
	 */
 	struct i386_fpstate fpstate;
 	struct i386_xsave_hdr_struct xsave_hdr;
 	/* new processor state extensions will go here */
} __attribute__ ((aligned (64)));

struct i386_xstate_cntxt {
	struct  i386_xstate *xstate;
	uint32_t	        size;
	uint32_t 	        lmask;
	uint32_t	        hmask;
};

typedef struct i386_ucontext_s
{
    uint32_t uc_flags;
    struct i386_ucontext_s *uc_link;
    i386_stack_t uc_stack;
    i386_mcontext_t uc_mcontext;
    i386_sigset_t uc_sigmask;
	/* Allow for uc_sigmask growth.  Glibc uses a 1024-bit sigset_t.  */
	int		  __unused[32 - (sizeof (sigset_t) / sizeof (int))];
	//struct i386_xstate_cntxt  uc_xstate;
    struct i386_xstate  xstate;
} i386_ucontext_t;

typedef struct i386_sigframe_s {
    uintptr_t       pretcode;   // pointer to retcode
    int             sig;
    i386_mcontext_t cpustate;
    struct i386_xstate fpstate;
    uintptr_t       extramask[64-1];
    char            retcode[8];
} i386_sigframe_t;

struct kernel_sigaction {
        void (*k_sa_handler) (int);
        unsigned long sa_flags;
        void (*sa_restorer) (void);
        unsigned long sa_mask;
        unsigned long sa_mask2;
};

static void sigstack_destroy(void* p)
{
	i386_stack_t *ss = (i386_stack_t*)p;
    free(ss);
}

static void free_signal_emu(void* p)
{
    if(p)
        FreeX86Emu((x86emu_t**)&p);
}

static pthread_key_t sigstack_key;
static pthread_once_t sigstack_key_once = PTHREAD_ONCE_INIT;

static void sigstack_key_alloc() {
	pthread_key_create(&sigstack_key, sigstack_destroy);
}

static pthread_key_t sigemu_key;
static pthread_once_t sigemu_key_once = PTHREAD_ONCE_INIT;

static void sigemu_key_alloc() {
	pthread_key_create(&sigemu_key, free_signal_emu);
}

static x86emu_t* get_signal_emu()
{
    x86emu_t *emu = (x86emu_t*)pthread_getspecific(sigemu_key);
    if(!emu) {
        const int stsize = 8*1024;  // small stack for signal handler
        void* stack = calloc(1, stsize);
        emu = NewX86Emu(my_context, 0, (uintptr_t)stack, stsize, 1);
        emu->type = EMUTYPE_SIGNAL;
        pthread_setspecific(sigemu_key, emu);
    }
    return emu;
}


uint32_t RunFunctionHandler(int* exit, uintptr_t fnc, int nargs, ...)
{
    if(fnc==0 || fnc==1) {
        printf_log(LOG_NONE, "BOX86: Warning, calling Signal function handler %s\n", fnc?"SIG_DFL":"SIG_IGN");
        return 0;
    }
    uintptr_t old_start = trace_start, old_end = trace_end;
//    trace_start = 0; trace_end = 1; // disabling trace, globably for now...

    x86emu_t *emu = get_signal_emu();
    printf_log(LOG_DEBUG, "%04d|signal function handler %p called, ESP=%p\n", GetTID(), (void*)fnc, (void*)R_ESP);
    
    /*SetFS(emu, default_fs);*/
    for (int i=0; i<6; ++i)
        emu->segs_serial[i] = 0;
        
    R_ESP -= nargs*4;   // need to push in reverse order

    uint32_t *p = (uint32_t*)R_ESP;

    va_list va;
    va_start (va, nargs);
    for (int i=0; i<nargs; ++i) {
        uint32_t v = va_arg(va, uint32_t);
        *p = v;
        p++;
    }
    va_end (va);

    EmuCall(emu, fnc);  // avoid DynaCall for now
    R_ESP+=(nargs*4);

    if(exit)
        *exit = emu->exit;

    uint32_t ret = R_EAX;

    trace_start = old_start; trace_end = old_end;

    return ret;
}

EXPORT int my_sigaltstack(x86emu_t* emu, const i386_stack_t* ss, i386_stack_t* oss)
{
    if(!ss && !oss) {   // this is not true, ss can be NULL to retreive oss info only
        errno = EFAULT;
        return -1;
    }
    x86emu_t *sigemu = get_signal_emu();
	i386_stack_t *new_ss = (i386_stack_t*)pthread_getspecific(sigstack_key);
    if(!ss) {
        if(!new_ss) {
            oss->ss_flags = SS_DISABLE;
            oss->ss_sp = sigemu->init_stack;
            oss->ss_size = sigemu->size_stack;
        } else {
            oss->ss_flags = new_ss->ss_flags;
            oss->ss_sp = new_ss->ss_sp;
            oss->ss_size = new_ss->ss_size;
        }
        return 0;
    }
    printf_log(LOG_DEBUG, "%04d|sigaltstack called ss=%p[flags=0x%x, sp=%p, ss=0x%x], oss=%p\n", GetTID(), ss, ss->ss_flags, ss->ss_sp, ss->ss_size, oss);
    if(ss->ss_flags && ss->ss_flags!=SS_DISABLE && ss->ss_flags!=SS_ONSTACK) {
        errno = EINVAL;
        return -1;
    }

    if(ss->ss_flags==SS_DISABLE) {
        if(new_ss)
            free(new_ss);
        pthread_setspecific(sigstack_key, NULL);

        sigemu->regs[_SP].dword[0] = ((uintptr_t)sigemu->init_stack + sigemu->size_stack) & ~7;
        
        return 0;
    }
    if(oss) {
        if(!new_ss) {
            oss->ss_flags = SS_DISABLE;
            oss->ss_sp = sigemu->init_stack;
            oss->ss_size = sigemu->size_stack;
        } else {
            oss->ss_flags = new_ss->ss_flags;
            oss->ss_sp = new_ss->ss_sp;
            oss->ss_size = new_ss->ss_size;
        }
    }
    if(!new_ss)
        new_ss = (i386_stack_t*)calloc(1, sizeof(i386_stack_t));
    new_ss->ss_sp = ss->ss_sp;
    new_ss->ss_size = ss->ss_size;

	pthread_setspecific(sigstack_key, new_ss);

    sigemu->regs[_SP].dword[0] = ((uintptr_t)new_ss->ss_sp + new_ss->ss_size - 16) & ~7;

    return 0;
}


void my_sighandler(int32_t sig)
{
    pthread_mutex_unlock(&my_context->mutex_trace);   // just in case
    printf_log(LOG_DEBUG, "Sighanlder for signal #%d called (jump to %p)\n", sig, (void*)my_context->signals[sig]);
    uintptr_t restorer = my_context->restorer[sig];
    int exits = 0;
    int ret = RunFunctionHandler(&exits, my_context->signals[sig], 1, sig);
    if(exits)
        exit(ret);
    if(restorer)
        RunFunctionHandler(&exits, restorer, 0);
}

#ifdef DYNAREC
uintptr_t getX86Address(dynablock_t* db, uintptr_t arm_addr)
{
    uintptr_t x86addr = (uintptr_t)db->x86_addr;
    uintptr_t armaddr = (uintptr_t)db->block;
    int i = 0;
    do {
        int x86sz = 0;
        int armsz = 0;
        do {
            x86sz+=db->instsize[i].x86;
            armsz+=db->instsize[i].nat*4;
        }
        while(!db->instsize[++i].x86);
        if(arm_addr>=armaddr && arm_addr<(armaddr+armsz))
            return x86addr;
        armaddr+=armsz;
        x86addr+=x86sz;
        if(arm_addr==armaddr)
            return x86addr;
    } while(db->instsize[i].x86 || db->instsize[i].nat);
    return x86addr;
}
#endif

void my_sigactionhandler_oldcode(int32_t sig, siginfo_t* info, void * ucntx, int* old_code)
{
    // need to create some x86_ucontext????
    pthread_mutex_unlock(&my_context->mutex_trace);   // just in case
    printf_log(LOG_DEBUG, "Sigactionhanlder for signal #%d called (jump to %p/%s)\n", sig, (void*)my_context->signals[sig], GetNativeName((void*)my_context->signals[sig]));

    uintptr_t restorer = my_context->restorer[sig];
    // get that actual ESP first!
    x86emu_t *emu = thread_get_emu();
    uint32_t *frame = (uint32_t*)R_ESP;
#if defined(DYNAREC) && defined(__arm__)
    ucontext_t *p = (ucontext_t *)ucntx;
    void * pc = (void*)p->uc_mcontext.arm_pc;
    dynablock_t* db = FindDynablockFromNativeAddress(pc);
    if(db) {
        frame = (uint32_t*)p->uc_mcontext.arm_r8;
    }
#endif
    // stack tracking
    x86emu_t *sigemu = get_signal_emu();
	i386_stack_t *new_ss = my_context->onstack[sig]?(i386_stack_t*)pthread_getspecific(sigstack_key):NULL;
    int used_stack = 0;
    if(new_ss) {
        if(new_ss->ss_flags == SS_ONSTACK) { // already using it!
            frame = (uint32_t*)sigemu->regs[_SP].dword[0];
        } else {
            frame = (uint32_t*)(((uintptr_t)new_ss->ss_sp + new_ss->ss_size - 16) & ~7);
            used_stack = 1;
            new_ss->ss_flags = SS_ONSTACK;
        }
    }

    // TODO: do I need to really setup 2 stack frame? That doesn't seems right!
    // setup stack frame
    // try to fill some sigcontext....
    frame -= sizeof(i386_ucontext_t)/4;
    i386_ucontext_t   *sigcontext = (i386_ucontext_t*)frame;
    // get general register
    sigcontext->uc_mcontext.gregs[REG_EAX] = R_EAX;
    sigcontext->uc_mcontext.gregs[REG_ECX] = R_ECX;
    sigcontext->uc_mcontext.gregs[REG_EDX] = R_EDX;
    sigcontext->uc_mcontext.gregs[REG_EDI] = R_EDI;
    sigcontext->uc_mcontext.gregs[REG_ESI] = R_ESI;
    sigcontext->uc_mcontext.gregs[REG_EBP] = R_EBP;
    sigcontext->uc_mcontext.gregs[REG_EIP] = R_EIP;
    sigcontext->uc_mcontext.gregs[REG_ESP] = R_ESP;
    sigcontext->uc_mcontext.gregs[REG_EBX] = R_EBX;
    // flags
    sigcontext->uc_mcontext.gregs[REG_EFL] = emu->eflags.x32;
    // get segments
    sigcontext->uc_mcontext.gregs[REG_GS] = R_GS;
    sigcontext->uc_mcontext.gregs[REG_FS] = R_FS;
    sigcontext->uc_mcontext.gregs[REG_ES] = R_ES;
    sigcontext->uc_mcontext.gregs[REG_DS] = R_DS;
    sigcontext->uc_mcontext.gregs[REG_CS] = R_CS;
    sigcontext->uc_mcontext.gregs[REG_SS] = R_SS;
#if defined(DYNAREC) && defined(__arm__)
    if(db) {
        sigcontext->uc_mcontext.gregs[REG_EAX] = p->uc_mcontext.arm_r4;
        sigcontext->uc_mcontext.gregs[REG_ECX] = p->uc_mcontext.arm_r5;
        sigcontext->uc_mcontext.gregs[REG_EDX] = p->uc_mcontext.arm_r6;
        sigcontext->uc_mcontext.gregs[REG_EBX] = p->uc_mcontext.arm_r7;
        sigcontext->uc_mcontext.gregs[REG_ESP] = p->uc_mcontext.arm_r8;
        sigcontext->uc_mcontext.gregs[REG_EBP] = p->uc_mcontext.arm_r9;
        sigcontext->uc_mcontext.gregs[REG_ESI] = p->uc_mcontext.arm_r10;
        sigcontext->uc_mcontext.gregs[REG_EDI] = p->uc_mcontext.arm_fp;
        sigcontext->uc_mcontext.gregs[REG_EIP] = getX86Address(db, (uintptr_t)pc);
    }
#endif
    // get FloatPoint status
    sigcontext->uc_mcontext.fpregs = (i386_fpregset_t)&sigcontext->xstate;
    fpu_fxsave(emu, &sigcontext->xstate);
    // add custom SIGN in reserved area
    //((unsigned int *)(&sigcontext.xstate.fpstate.padding))[8*4+12] = 0x46505853;  // not yet, when XSAVE / XRSTR will be ready
    // get signal mask

    if(!new_ss) {
        sigcontext->uc_stack.ss_sp = sigemu->init_stack;
        sigcontext->uc_stack.ss_size = sigemu->size_stack;
        sigcontext->uc_stack.ss_flags = SS_DISABLE;
    } else {
        sigcontext->uc_stack.ss_sp = new_ss->ss_sp;
        sigcontext->uc_stack.ss_size = new_ss->ss_size;
        sigcontext->uc_stack.ss_flags = new_ss->ss_flags;
    }
    // Try to guess some REG_TRAPNO
    /*
    TRAP_x86_DIVIDE     = 0,   // Division by zero exception
    TRAP_x86_TRCTRAP    = 1,   // Single-step exception
    TRAP_x86_NMI        = 2,   // NMI interrupt
    TRAP_x86_BPTFLT     = 3,   // Breakpoint exception
    TRAP_x86_OFLOW      = 4,   // Overflow exception
    TRAP_x86_BOUND      = 5,   // Bound range exception
    TRAP_x86_PRIVINFLT  = 6,   // Invalid opcode exception
    TRAP_x86_DNA        = 7,   // Device not available exception
    TRAP_x86_DOUBLEFLT  = 8,   // Double fault exception
    TRAP_x86_FPOPFLT    = 9,   // Coprocessor segment overrun
    TRAP_x86_TSSFLT     = 10,  // Invalid TSS exception
    TRAP_x86_SEGNPFLT   = 11,  // Segment not present exception
    TRAP_x86_STKFLT     = 12,  // Stack fault
    TRAP_x86_PROTFLT    = 13,  // General protection fault
    TRAP_x86_PAGEFLT    = 14,  // Page fault
    TRAP_x86_ARITHTRAP  = 16,  // Floating point exception
    TRAP_x86_ALIGNFLT   = 17,  // Alignment check exception
    TRAP_x86_MCHK       = 18,  // Machine check exception
    TRAP_x86_CACHEFLT   = 19   // SIMD exception (via SIGFPE) if CPU is SSE capable otherwise Cache flush exception (via SIGSEV)
    */
    uint32_t prot = getProtection((uintptr_t)info->si_addr);
    if(sig==SIGBUS)
        sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 17;
    else if(sig==SIGSEGV) {
        if((uintptr_t)info->si_addr == sigcontext->uc_mcontext.gregs[REG_EIP]) {
            sigcontext->uc_mcontext.gregs[REG_ERR] = 0x0010;    // execution flag issue (probably)
            sigcontext->uc_mcontext.gregs[REG_TRAPNO] = (info->si_code == SEGV_ACCERR)?13:14;
        } else if(info->si_code==SEGV_ACCERR && !(prot&PROT_WRITE)) {
            sigcontext->uc_mcontext.gregs[REG_ERR] = 0x0002;    // write flag issue
            if(abs((intptr_t)info->si_addr-(intptr_t)sigcontext->uc_mcontext.gregs[REG_ESP])<16)
                sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 12; // stack overflow probably
            else
                sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 14; // PAGE_FAULT
        } else {
            sigcontext->uc_mcontext.gregs[REG_TRAPNO] = (info->si_code == SEGV_ACCERR)?13:14;
            //REG_ERR seems to be INT:8 CODE:8. So for write access segfault it's 0x0002 For a read it's 0x0004 (and 8 for exec). For an int 2d it could be 0x2D01 for example
            sigcontext->uc_mcontext.gregs[REG_ERR] = 0x0004;    // read error? there is no execute control in box86 anyway
        }
        if(info->si_code == SEGV_ACCERR && old_code)
            *old_code = -1;
    } else if(sig==SIGFPE)
        sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 19;
    else if(sig==SIGILL)
        sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 6;
    // call the signal handler
    i386_ucontext_t sigcontext_copy = *sigcontext;

    // set stack pointer
    sigemu->regs[_SP].dword[0] = (uintptr_t)frame;

    // set segments
    memcpy(sigemu->segs, emu->segs, sizeof(sigemu->segs));

    int exits = 0;
    int ret = RunFunctionHandler(&exits, my_context->signals[sig], 3, sig, info, sigcontext);

    if(memcmp(sigcontext, &sigcontext_copy, sizeof(i386_ucontext_t))) {
        emu_jmpbuf_t* ejb = GetJmpBuf();
        if(ejb->jmpbuf_ok) {
            #define GO(R)   if(sigcontext->uc_mcontext.gregs[REG_E##R]!=sigcontext_copy.uc_mcontext.gregs[REG_E##R]) ejb->emu->regs[_##R].dword[0]=sigcontext->uc_mcontext.gregs[REG_E##R]
            GO(AX);
            GO(CX);
            GO(DX);
            GO(DI);
            GO(SI);
            GO(BP);
            GO(SP);
            GO(BX);
            #undef GO
            if(sigcontext->uc_mcontext.gregs[REG_EIP]!=sigcontext_copy.uc_mcontext.gregs[REG_EIP]) ejb->emu->ip.dword[0]=sigcontext->uc_mcontext.gregs[REG_EIP];
            sigcontext->uc_mcontext.gregs[REG_EIP] = R_EIP;
            // flags
            if(sigcontext->uc_mcontext.gregs[REG_EFL]!=sigcontext_copy.uc_mcontext.gregs[REG_EFL]) ejb->emu->eflags.x32=sigcontext->uc_mcontext.gregs[REG_EFL];
            // get segments
            #define GO(S)   if(sigcontext->uc_mcontext.gregs[REG_##S]!=sigcontext_copy.uc_mcontext.gregs[REG_##S]) {ejb->emu->segs[_##S]=sigcontext->uc_mcontext.gregs[REG_##S]; ejb->emu->segs_serial[_##S] = 0;}
            GO(GS);
            GO(FS);
            GO(ES);
            GO(DS);
            GO(CS);
            GO(SS);
            #undef GO
            printf_log(LOG_DEBUG, "Context has been changed in Sigactionhanlder, doing longjmp to resume emu\n");
            if(old_code)
                *old_code = -1;    // re-init the value to allow another segfault at the same place
            if(used_stack)  // release stack
                new_ss->ss_flags = 0;
            longjmp(ejb->jmpbuf, 1);
        }
        printf_log(LOG_INFO, "Warning, context has been changed in Sigactionhanlder%s\n", (sigcontext->uc_mcontext.gregs[REG_EIP]!=sigcontext_copy.uc_mcontext.gregs[REG_EIP])?" (EIP changed)":"");
    }
    printf_log(LOG_DEBUG, "Sigactionhanlder main function returned (exit=%d, restorer=%p)\n", exits, (void*)restorer);
    if(exits)
        exit(ret);
    if(restorer)
        RunFunctionHandler(&exits, restorer, 0);
    if(used_stack)  // release stack
        new_ss->ss_flags = 0;
}

void my_box86signalhandler(int32_t sig, siginfo_t* info, void * ucntx)
{
    // sig==SIGSEGV || sig==SIGBUS || sig==SIGILL here!
    int log_minimum = (my_context->is_sigaction[sig] && sig==SIGSEGV)?LOG_INFO:LOG_NONE;
    ucontext_t *p = (ucontext_t *)ucntx;
    void* addr = (void*)info->si_addr;  // address that triggered the issue
    void* esp = NULL;
#ifdef __arm__
    void * pc = (void*)p->uc_mcontext.arm_pc;
#elif defined __i386
    void * pc = (void*)p->uc_mcontext.gregs[REG_EIP];
#elif defined PPC
    void * pc = (void*)p->uc_mcontext.uc_regs->gregs[PT_NIP];
#else
    void * pc = NULL;    // unknow arch...
    #warning Unhandled architecture
#endif
#ifdef DYNAREC
    uint32_t prot = getProtection((uintptr_t)addr);
    if ((sig==SIGSEGV) && (addr) && (info->si_code == SEGV_ACCERR) && (prot&PROT_DYNAREC)) {
        if(box86_dynarec_smc) {
            dynablock_t* db_pc = NULL;
            dynablock_t* db_addr = NULL;
            db_pc = FindDynablockFromNativeAddress(pc);
            if(db_pc)
                db_addr = FindDynablockFromNativeAddress(addr);
            if(db_pc && db_addr) {
                if (db_pc == db_addr) {
                    dynarec_log(LOG_NONE, "Warning: Access to protected %p from %p, inside same dynablock\n", addr, pc);            
                }
            }
        }
        dynarec_log(LOG_DEBUG, "Access to protected %p from %p, unprotecting memory (prot=%x)\n", addr, pc, prot);
        // access error, unprotect the block (and mark them dirty)
        if(prot&PROT_DYNAREC)   // on heavy multi-thread program, the protection can already be gone...
            unprotectDB((uintptr_t)addr, 1);    // unprotect 1 byte... But then, the whole page will be unprotected
        // done
        if(prot&PROT_WRITE) return; // if there is no write permission, don't return and continue to program signal handling
    }
#endif
    static int old_code = -1;
    static void* old_pc = 0;
    static void* old_addr = 0;
    const char* signame = (sig==SIGSEGV)?"SIGSEGV":((sig==SIGBUS)?"SIGBUS":"SIGILL");
    if(old_code==info->si_code && old_pc==pc && old_addr==addr) {
        printf_log(log_minimum, "%04d|Double %s (code=%d, pc=%p, addr=%p)!\n", GetTID(), signame, old_code, old_pc, old_addr);
exit(-1);
    } else {
#ifdef DYNAREC
        dynablock_t* db = FindDynablockFromNativeAddress(pc);
#endif
        old_code = info->si_code;
        old_pc = pc;
        old_addr = addr;
        const char* name = GetNativeName(pc);
        uintptr_t x86pc = (uintptr_t)-1;
        const char* x86name = NULL;
        const char* elfname = NULL;
        x86emu_t* emu = thread_get_emu();
        x86pc = R_EIP;
        esp = (void*)R_ESP;
#if defined(__arm__) && defined(DYNAREC)
        if(db && p->uc_mcontext.arm_r0>0x10000) {
            emu = (x86emu_t*)p->uc_mcontext.arm_r0;
        }
        if(db) {
            x86pc = getX86Address(db, (uintptr_t)pc);
            esp = (void*)p->uc_mcontext.arm_r8;
        }
#endif
        x86name = getAddrFunctionName(x86pc);
        elfheader_t* elf = FindElfAddress(my_context, x86pc);
        if(elf)
            elfname = ElfName(elf);
        if(jit_gdb) {
            pid_t pid = getpid();
            int v = fork(); // is this ok in a signal handler???
            if(v) {
                // parent process, the one that have the segfault
                volatile int waiting = 1;
                printf_log(LOG_NONE, "Waiting for %s (pid %d)...\n", (jit_gdb==2)?"gdbserver":"gdb", pid);
                while(waiting) {
                    // using gdb, use "set waiting=0" to stop waiting...
                    usleep(1000);
                }
            } else {
                char myarg[50] = {0};
                sprintf(myarg, "%d", pid);
                if(jit_gdb==2)
                    execlp("gdbserver", "gdbserver", "127.0.0.1:1234", "--attach", myarg, (char*)NULL);
                else
                    execlp("gdb", "gdb", "-pid", myarg, (char*)NULL);
                exit(-1);
            }
        }
#ifdef DYNAREC
        printf_log(log_minimum, "%04d|%s @%p (%s) (x86pc=%p/%s:\"%s\", esp=%p), for accessing %p (code=%d/prot=%x), db=%p(%p:%p/%p:%p/%s:%s)", 
            GetTID(), signame, pc, name, (void*)x86pc, elfname?elfname:"???", x86name?x86name:"???", esp, addr, info->si_code, 
            prot, db, db?db->block:0, db?(db->block+db->size):0, 
            db?db->x86_addr:0, db?(db->x86_addr+db->x86_size):0, 
            getAddrFunctionName((uintptr_t)(db?db->x86_addr:0)), (db?db->need_test:0)?"need_stest":"clean");
#else
        printf_log(log_minimum, "%04d|%s @%p (%s) (x86pc=%p/%s:\"%s\", esp=%p), for accessing %p (code=%d)", GetTID(), signame, pc, name, (void*)x86pc, elfname?elfname:"???", x86name?x86name:"???", esp, addr, info->si_code);
#endif
        if(sig==SIGILL)
            printf_log(log_minimum, " opcode=%02X %02X %02X %02X %02X %02X %02X %02X\n", ((uint8_t*)pc)[0], ((uint8_t*)pc)[1], ((uint8_t*)pc)[2], ((uint8_t*)pc)[3], ((uint8_t*)pc)[4], ((uint8_t*)pc)[5], ((uint8_t*)pc)[6], ((uint8_t*)pc)[7]);
        else if(sig==SIGBUS)
            printf_log(log_minimum, " x86opcode=%02X %02X %02X %02X %02X %02X %02X %02X\n", ((uint8_t*)x86pc)[0], ((uint8_t*)x86pc)[1], ((uint8_t*)x86pc)[2], ((uint8_t*)x86pc)[3], ((uint8_t*)x86pc)[4], ((uint8_t*)x86pc)[5], ((uint8_t*)x86pc)[6], ((uint8_t*)x86pc)[7]);
        else
            printf_log(log_minimum, "\n");
    }
    if(my_context->signals[sig] && my_context->signals[sig]!=1) {
        if(my_context->is_sigaction[sig])
            my_sigactionhandler_oldcode(sig, info, ucntx, &old_code);
        else
            my_sighandler(sig);
        return;
    }
    // no handler (or double identical segfault)
    // set default and that's it, instruction will restart and default segfault handler will be called...
    if(my_context->signals[sig]!=1)
        signal(sig, SIG_DFL);
}

void my_sigactionhandler(int32_t sig, siginfo_t* info, void * ucntx)
{
    my_sigactionhandler_oldcode(sig, info, ucntx, NULL);
}

EXPORT sighandler_t my_signal(x86emu_t* emu, int signum, sighandler_t handler)
{
    if(signum<0 || signum>=MAX_SIGNAL)
        return SIG_ERR;

    if(signum==SIGSEGV && emu->context->no_sigsegv)
        return 0;

    // create a new handler
    my_context->signals[signum] = (uintptr_t)handler;
    my_context->is_sigaction[signum] = 0;
    my_context->restorer[signum] = 0;
    if(handler!=NULL && handler!=(sighandler_t)1) {
        handler = my_sighandler;
    }

    if(signum==SIGSEGV || signum==SIGBUS || signum==SIGILL)
        return 0;

    return signal(signum, handler);
}
EXPORT sighandler_t my___sysv_signal(x86emu_t* emu, int signum, sighandler_t handler) __attribute__((alias("my_signal")));
EXPORT sighandler_t my_sysv_signal(x86emu_t* emu, int signum, sighandler_t handler) __attribute__((alias("my_signal")));    // not completly exact

int EXPORT my_sigaction(x86emu_t* emu, int signum, const x86_sigaction_t *act, x86_sigaction_t *oldact)
{
    if(signum<0 || signum>=MAX_SIGNAL)
        return -1;
    
    if(signum==SIGSEGV && emu->context->no_sigsegv)
        return 0;

    if(signum==SIGILL && emu->context->no_sigill)
        return 0;
    struct sigaction newact = {0};
    struct sigaction old = {0};
    if(act) {
        newact.sa_mask = act->sa_mask;
        newact.sa_flags = act->sa_flags&~0x04000000;  // No sa_restorer...
        if(act->sa_flags&0x04) {
            my_context->signals[signum] = (uintptr_t)act->_u._sa_sigaction;
            my_context->is_sigaction[signum] = 1;
            if(act->_u._sa_handler!=NULL && act->_u._sa_handler!=(sighandler_t)1) {
                newact.sa_sigaction = my_sigactionhandler;
            } else
                newact.sa_sigaction = act->_u._sa_sigaction;
        } else {
            my_context->signals[signum] = (uintptr_t)act->_u._sa_handler;
            my_context->is_sigaction[signum] = 0;
            if(act->_u._sa_handler!=NULL && act->_u._sa_handler!=(sighandler_t)1) {
                newact.sa_handler = my_sighandler;
            } else
                newact.sa_handler = act->_u._sa_handler;
        }
        my_context->restorer[signum] = (act->sa_flags&0x04000000)?(uintptr_t)act->sa_restorer:0;
        my_context->onstack[signum] = (act->sa_flags&SA_ONSTACK)?1:0;
    }
    int ret = 0;
    if(signum!=SIGSEGV && signum!=SIGBUS && signum!=SIGILL)
        sigaction(signum, act?&newact:NULL, oldact?&old:NULL);
    if(oldact) {
        oldact->sa_flags = old.sa_flags;
        oldact->sa_mask = old.sa_mask;
        if(old.sa_flags & 0x04)
            oldact->_u._sa_sigaction = old.sa_sigaction; //TODO should wrap...
        else
            oldact->_u._sa_handler = old.sa_handler;  //TODO should wrap...
        oldact->sa_restorer = NULL; // no handling for now...
    }
    return ret;
}
int EXPORT my___sigaction(x86emu_t* emu, int signum, const x86_sigaction_t *act, x86_sigaction_t *oldact)
__attribute__((alias("my_sigaction")));

int EXPORT my_syscall_sigaction(x86emu_t* emu, int signum, const x86_sigaction_restorer_t *act, x86_sigaction_restorer_t *oldact, int sigsetsize)
{
    printf_log(LOG_DEBUG, "Syscall/Sigaction(signum=%d, act=%p, old=%p, size=%d)\n", signum, act, oldact, sigsetsize);
    if(signum<0 || signum>=MAX_SIGNAL)
        return -1;
    
    if(signum==SIGSEGV && emu->context->no_sigsegv)
        return 0;
    // TODO, how to handle sigsetsize>4?!
    if(signum==32 || signum==33) {
        // cannot use libc sigaction, need to use syscall!
        struct kernel_sigaction newact = {0};
        struct kernel_sigaction old = {0};
        if(act) {
            printf_log(LOG_DEBUG, " New (kernel) action flags=0x%x mask=0x%x\n", act->sa_flags, *(uint32_t*)&act->sa_mask);
            memcpy(&newact.sa_mask, &act->sa_mask, (sigsetsize>8)?8:sigsetsize);
            newact.sa_flags = act->sa_flags&~0x04000000;  // No sa_restorer...
            if(act->sa_flags&0x04) {
                my_context->signals[signum] = (uintptr_t)act->_u._sa_sigaction;
                my_context->is_sigaction[signum] = 1;
                if(act->_u._sa_handler!=NULL && act->_u._sa_handler!=(sighandler_t)1) {
                    newact.k_sa_handler = (void*)my_sigactionhandler;
                } else {
                    newact.k_sa_handler = (void*)act->_u._sa_sigaction;
                }
            } else {
                my_context->signals[signum] = (uintptr_t)act->_u._sa_handler;
                my_context->is_sigaction[signum] = 0;
                if(act->_u._sa_handler!=NULL && act->_u._sa_handler!=(sighandler_t)1) {
                    newact.k_sa_handler = my_sighandler;
                } else {
                    newact.k_sa_handler = act->_u._sa_handler;
                }
            }
            my_context->restorer[signum] = (act->sa_flags&0x04000000)?(uintptr_t)act->sa_restorer:0;
        }

        if(oldact) {
            old.sa_flags = oldact->sa_flags;
            memcpy(&old.sa_mask, &oldact->sa_mask, (sigsetsize>8)?8:sigsetsize);
        }

        int ret = syscall(__NR_sigaction, signum, act?&newact:NULL, oldact?&old:NULL, (sigsetsize>8)?8:sigsetsize);
        if(oldact && ret==0) {
            oldact->sa_flags = old.sa_flags;
            memcpy(&oldact->sa_mask, &old.sa_mask, (sigsetsize>8)?8:sigsetsize);
            if(old.sa_flags & 0x04)
                oldact->_u._sa_sigaction = (void*)old.k_sa_handler; //TODO should wrap...
            else
                oldact->_u._sa_handler = old.k_sa_handler;  //TODO should wrap...
        }
        return ret;
    } else {
        // using libc sigaction
        struct sigaction newact = {0};
        struct sigaction old = {0};
        if(act) {
            printf_log(LOG_DEBUG, " New action flags=0x%x mask=0x%x\n", act->sa_flags, *(uint32_t*)&act->sa_mask);
            newact.sa_mask = act->sa_mask;
            newact.sa_flags = act->sa_flags&~0x04000000;  // No sa_restorer...
            if(act->sa_flags&0x04) {
                if(act->_u._sa_handler!=NULL && act->_u._sa_handler!=(sighandler_t)1) {
                    my_context->signals[signum] = (uintptr_t)act->_u._sa_sigaction;
                    newact.sa_sigaction = my_sigactionhandler;
                } else {
                    newact.sa_sigaction = act->_u._sa_sigaction;
                }
            } else {
                if(act->_u._sa_handler!=NULL && act->_u._sa_handler!=(sighandler_t)1) {
                    my_context->signals[signum] = (uintptr_t)act->_u._sa_handler;
                    my_context->is_sigaction[signum] = 0;
                    newact.sa_handler = my_sighandler;
                } else {
                    newact.sa_handler = act->_u._sa_handler;
                }
            }
            my_context->restorer[signum] = (act->sa_flags&0x04000000)?(uintptr_t)act->sa_restorer:0;
        }

        if(oldact) {
            old.sa_flags = oldact->sa_flags;
            old.sa_mask = oldact->sa_mask;
        }
        int ret = 0;

        if(signum!=SIGSEGV && signum!=SIGBUS && signum!=SIGILL)
            ret = sigaction(signum, act?&newact:NULL, oldact?&old:NULL);
        if(oldact && ret==0) {
            oldact->sa_flags = old.sa_flags;
            oldact->sa_mask = old.sa_mask;
            if(old.sa_flags & 0x04)
                oldact->_u._sa_sigaction = old.sa_sigaction; //TODO should wrap...
            else
                oldact->_u._sa_handler = old.sa_handler;  //TODO should wrap...
        }
        return ret;
    }
}

EXPORT sighandler_t my_sigset(x86emu_t* emu, int signum, sighandler_t handler)
{
    // emulated SIG_HOLD
    if(handler == (sighandler_t)2) {
        x86_sigaction_t oact;
        sigset_t nset;
        sigset_t oset;
        if (sigemptyset (&nset) < 0)
            return (sighandler_t)-1;
        if (sigaddset (&nset, signum) < 0)
            return (sighandler_t)-1;
        if (sigprocmask (SIG_BLOCK, &nset, &oset) < 0)
            return (sighandler_t)-1;
        if (sigismember (&oset, signum))
            return (sighandler_t)2;
        if (my_sigaction (emu, signum, NULL, &oact) < 0)
            return (sighandler_t)-1;
        return oact._u._sa_handler;
    }
    return my_signal(emu, signum, handler);
}

EXPORT int my_getcontext(x86emu_t* emu, void* ucp)
{
//    printf_log(LOG_NONE, "Warning: call to partially implemented getcontext\n");
    i386_ucontext_t *u = (i386_ucontext_t*)ucp;
    // stack traking
    u->uc_stack.ss_sp = NULL;
    u->uc_stack.ss_size = 0;    // this need to filled
    // get general register
    u->uc_mcontext.gregs[REG_EAX] = R_EAX;
    u->uc_mcontext.gregs[REG_ECX] = R_ECX;
    u->uc_mcontext.gregs[REG_EDX] = R_EDX;
    u->uc_mcontext.gregs[REG_EDI] = R_EDI;
    u->uc_mcontext.gregs[REG_ESI] = R_ESI;
    u->uc_mcontext.gregs[REG_EBP] = R_EBP;
    u->uc_mcontext.gregs[REG_EIP] = *(uint32_t*)R_ESP;
    u->uc_mcontext.gregs[REG_ESP] = R_ESP+4;
    u->uc_mcontext.gregs[REG_EBX] = R_EBX;
    // get segments
    u->uc_mcontext.gregs[REG_GS] = R_GS;
    u->uc_mcontext.gregs[REG_FS] = R_FS;
    u->uc_mcontext.gregs[REG_ES] = R_ES;
    u->uc_mcontext.gregs[REG_DS] = R_DS;
    u->uc_mcontext.gregs[REG_CS] = R_CS;
    u->uc_mcontext.gregs[REG_SS] = R_SS;
    // get FloatPoint status
    // get signal mask
    sigprocmask(SIG_SETMASK, NULL, (sigset_t*)&u->uc_sigmask);

    return 0;
}

EXPORT int my_setcontext(x86emu_t* emu, void* ucp)
{
//    printf_log(LOG_NONE, "Warning: call to partially implemented setcontext\n");
    i386_ucontext_t *u = (i386_ucontext_t*)ucp;
    // stack tracking
    emu->init_stack = u->uc_stack.ss_sp;
    emu->size_stack = u->uc_stack.ss_size;
    // set general register
    R_EAX = u->uc_mcontext.gregs[REG_EAX];
    R_ECX = u->uc_mcontext.gregs[REG_ECX];
    R_EDX = u->uc_mcontext.gregs[REG_EDX];
    R_EDI = u->uc_mcontext.gregs[REG_EDI];
    R_ESI = u->uc_mcontext.gregs[REG_ESI];
    R_EBP = u->uc_mcontext.gregs[REG_EBP];
    R_EIP = u->uc_mcontext.gregs[REG_EIP];
    R_ESP = u->uc_mcontext.gregs[REG_ESP];
    R_EBX = u->uc_mcontext.gregs[REG_EBX];
    // get segments
    R_GS = u->uc_mcontext.gregs[REG_GS];
    R_FS = u->uc_mcontext.gregs[REG_FS];
    R_ES = u->uc_mcontext.gregs[REG_ES];
    R_DS = u->uc_mcontext.gregs[REG_DS];
    R_CS = u->uc_mcontext.gregs[REG_CS];
    R_SS = u->uc_mcontext.gregs[REG_SS];
    // set FloatPoint status
    // set signal mask
    //sigprocmask(SIG_SETMASK, NULL, (sigset_t*)&u->uc_sigmask);
    // set uc_link
    emu->uc_link = u->uc_link;

    return R_EAX;
}

EXPORT int my_makecontext(x86emu_t* emu, void* ucp, void* fnc, int32_t argc, int32_t* argv)
{
//    printf_log(LOG_NONE, "Warning: call to unimplemented makecontext\n");
    i386_ucontext_t *u = (i386_ucontext_t*)ucp;
    // setup stack
    u->uc_mcontext.gregs[REG_ESP] = (uintptr_t)u->uc_stack.ss_sp + u->uc_stack.ss_size - 4;
    // setup the function
    u->uc_mcontext.gregs[REG_EIP] = (intptr_t)fnc;
    // setup args
    uint32_t* esp = (uint32_t*)u->uc_mcontext.gregs[REG_ESP];
    for (int i=0; i<argc; ++i) {
        // push value
        --esp;
        *esp = argv[(argc-1)-i];
    }
    // push the return value
    --esp;
    *esp = (uintptr_t)GetExit();
    u->uc_mcontext.gregs[REG_ESP] = (uintptr_t)esp;
    
    return 0;
}

EXPORT int my_swapcontext(x86emu_t* emu, void* ucp1, void* ucp2)
{
//    printf_log(LOG_NONE, "Warning: call to unimplemented swapcontext\n");
    // grab current context in ucp1
    my_getcontext(emu, ucp1);
    // activate ucp2
    my_setcontext(emu, ucp2);
    return 0;
}

void init_signal_helper(box86context_t* context)
{
    // setup signal handling
    for(int i=0; i<MAX_SIGNAL; ++i) {
        context->signals[i] = 1;    // SIG_DFL
    }
	struct sigaction action;
	action.sa_flags = SA_SIGINFO | SA_RESTART | SA_NODEFER;
	action.sa_sigaction = my_box86signalhandler;
    sigaction(SIGSEGV, &action, NULL);
	action.sa_flags = SA_SIGINFO | SA_RESTART | SA_NODEFER;
	action.sa_sigaction = my_box86signalhandler;
    sigaction(SIGBUS, &action, NULL);
	action.sa_flags = SA_SIGINFO | SA_RESTART | SA_NODEFER;
	action.sa_sigaction = my_box86signalhandler;
    sigaction(SIGILL, &action, NULL);

	pthread_once(&sigstack_key_once, sigstack_key_alloc);
	pthread_once(&sigemu_key_once, sigemu_key_alloc);
}

void fini_signal_helper()
{
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGILL, SIG_DFL);
}
