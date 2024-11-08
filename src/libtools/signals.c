#define _GNU_SOURCE
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
#ifndef ANDROID
#include <execinfo.h>
#endif

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
#include "dynarec_arm.h"
#endif


/* Definitions taken from the kernel headers.  */
#ifndef __i386__
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
#endif

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
  uint16_t ControlWord;        /* 000 */
  uint16_t StatusWord;         /* 002 */
  uint8_t  TagWord;            /* 004 */
  uint8_t  Reserved1;          /* 005 */
  uint16_t ErrorOpcode;        /* 006 */
  uint32_t ErrorOffset;        /* 008 */
  uint16_t ErrorSelector;      /* 00c */
  uint16_t Reserved2;          /* 00e */
  uint32_t DataOffset;         /* 010 */
  uint16_t DataSelector;       /* 014 */
  uint16_t Reserved3;          /* 016 */
  uint32_t MxCsr;              /* 018 */
  uint32_t MxCsr_Mask;         /* 01c */
  sse_regs_t FloatRegisters[8];/* 020 */  // fpu/mmx are store in 128bits here
  sse_regs_t XmmRegisters[16]; /* 0a0 */
  uint8_t  Reserved4[96];      /* 1a0 */
}__attribute__((packed));

static void save_fpreg(x86emu_t* emu, struct i386_fpstate* state)
{
    emu->sw.f.F87_TOP = emu->top&7;
    state->sw = emu->sw.x16;
    state->cw = emu->cw.x16;
    // save SSE and MMX regs
    fpu_fxsave(emu, &state->ControlWord);
}
static void load_fpreg(x86emu_t* emu, struct i386_fpstate* state)
{
    // copy SSE and MMX regs
    fpu_fxrstor(emu, &state->ControlWord);
    emu->cw.x16 = state->cw;
    emu->sw.x16 = state->sw;
    emu->top = emu->sw.f.F87_TOP&7;
}

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
	int		  unused[32 - (sizeof (sigset_t) / sizeof (int))];
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
    box_free(ss);
}

static pthread_key_t sigstack_key;
static pthread_once_t sigstack_key_once = PTHREAD_ONCE_INIT;

static void sigstack_key_alloc() {
	pthread_key_create(&sigstack_key, sigstack_destroy);
}

//1<<8 is mutex_dyndump
#define is_dyndump_locked (1<<8)
uint32_t RunFunctionHandler(int* exit, int dynarec, i386_ucontext_t* sigcontext, uintptr_t fnc, int nargs, ...)
{
    if(fnc==0 || fnc==1) {
        printf_log(LOG_NONE, "BOX86: Warning, calling Signal function handler %s with %d args \n", fnc?"SIG_DFL":"SIG_IGN", nargs);
        return 0;
    }
    uintptr_t old_start = trace_start, old_end = trace_end;
    //trace_start = 0; trace_end = 1; // disabling trace, globably for now...

    x86emu_t *emu = thread_get_emu();
    #ifdef DYNAREC
    if(box86_dynarec_test)
        emu->test.test = 0;
    #endif

    printf_log(LOG_DEBUG, "%04d|signal function handler %p called, ESP=%p, emu=%p\n", GetTID(), (void*)fnc, (void*)R_ESP, emu);
    
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

    int oldquitonlongjmp = emu->flags.quitonlongjmp;
    emu->flags.quitonlongjmp = 2;

    emu->eflags.x32 &= ~(1<<F_TF); // this one needs to cleared

    if(dynarec)
        DynaCall(emu, fnc);
    else
        EmuCall(emu, fnc);

    uint32_t ret = R_EAX;
    emu->flags.quitonlongjmp = oldquitonlongjmp;

    #ifdef DYNAREC
    if(box86_dynarec_test) {
        emu->test.test = 0;
        emu->test.clean = 0;
    }
    #endif

    if(emu->flags.longjmp) {
        // longjmp inside signal handler, lets grab all relevent value and do the actual longjmp in the signal handler
        emu->flags.longjmp = 0;
        if(sigcontext) {
            printf_log(LOG_DEBUG, "Longjmp in signal\n");
            #define GO(R) sigcontext->uc_mcontext.gregs[REG_E##R] = emu->regs[_##R].dword[0]
            GO(AX);
            GO(CX);
            GO(DX);
            GO(DI);
            GO(SI);
            GO(BP);
            GO(SP);
            GO(BX);
            #undef GO
            sigcontext->uc_mcontext.gregs[REG_EIP] = emu->ip.dword[0];
            // flags
            sigcontext->uc_mcontext.gregs[REG_EFL] = emu->eflags.x32;
            // segments
            #define GO(S) sigcontext->uc_mcontext.gregs[REG_##S] = emu->segs[_##S]; emu->segs_serial[_##S] = 0
            GO(GS);
            GO(FS);
            GO(ES);
            GO(DS);
            GO(CS);
            GO(SS);
            #undef GO
        } else {
            printf_log(LOG_NONE, "Warning, longjmp in signal but no sigcontext to change\n");
        }
    }
    R_ESP+=(nargs*4);

    if(exit)
        *exit = emu->exit;

    trace_start = old_start; trace_end = old_end;

    return ret;
}

EXPORT int my_sigaltstack(x86emu_t* emu, const i386_stack_t* ss, i386_stack_t* oss)
{
    if(!ss && !oss) {   // this is not true, ss can be NULL to retreive oss info only
        errno = EFAULT;
        return -1;
    }
	i386_stack_t *new_ss = (i386_stack_t*)pthread_getspecific(sigstack_key);
    if(oss) {
        if(!new_ss) {
            oss->ss_flags = SS_DISABLE;
            oss->ss_sp = emu->init_stack;
            oss->ss_size = emu->size_stack;
        } else {
            oss->ss_flags = new_ss->ss_flags;
            oss->ss_sp = new_ss->ss_sp;
            oss->ss_size = new_ss->ss_size;
        }
    }
    if(!ss) {
        return 0;
    }
    printf_log(LOG_DEBUG, "%04d|sigaltstack called ss=%p[flags=0x%x, sp=%p, ss=0x%x], oss=%p\n", GetTID(), ss, ss->ss_flags, ss->ss_sp, ss->ss_size, oss);
    if(ss->ss_flags && ss->ss_flags!=SS_DISABLE && ss->ss_flags!=SS_ONSTACK) {
        errno = EINVAL;
        return -1;
    }

    if(ss->ss_flags==SS_DISABLE) {
        if(new_ss)
            box_free(new_ss);
        pthread_setspecific(sigstack_key, NULL);

        return 0;
    }
    if(!new_ss)
        new_ss = (i386_stack_t*)box_calloc(1, sizeof(i386_stack_t));
    new_ss->ss_flags = 0;
    new_ss->ss_sp = ss->ss_sp;
    new_ss->ss_size = ss->ss_size;

	pthread_setspecific(sigstack_key, new_ss);

    return 0;
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
            ++i;
        }
        while(db->instsize[i-1].x86==15 || db->instsize[i-1].nat==15);
        if(arm_addr>=armaddr && arm_addr<(armaddr+armsz))
            return x86addr;
        armaddr+=armsz;
        x86addr+=x86sz;
        if(arm_addr==armaddr)
            return x86addr;
    } while(db->instsize[i].x86 || db->instsize[i].nat);
    return x86addr;
}

static int getDBX86N(dynablock_t* db, uintptr_t arm_addr)
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
            ++i;
        }
        while(db->instsize[i-1].x86==15 || db->instsize[i-1].nat==15);
        if(arm_addr>=armaddr && arm_addr<(armaddr+armsz))
            return (arm_addr-armaddr)>>2;
        armaddr+=armsz;
        x86addr+=x86sz;
        if(arm_addr==armaddr)
            return 0;
    } while(db->instsize[i].x86 || db->instsize[i].nat);
    return (arm_addr-armaddr)>>2;
}

#define CASE_MOVS   1
int isSpecialCases(uintptr_t x86pc, int n)
{
    if(*(uint8_t*)x86pc==0xF2 || *(uint8_t*)(x86pc)==0xF3) {
        if(*(uint8_t*)(x86pc+1)>=0xA4 && *(uint8_t*)(x86pc+1)<=0xA7) {
            if(n==6)    // special case if REP MOVS(B/D) happens on STR (so after LDR): 
                        // or the second LDR on CMPS
                        // need to adjust ESI to undo the read
                return CASE_MOVS;
            return 0;
        }
        return 0;
    }
    return 0;
}
#endif

// my_sigactionhandler_oldcode will relock mutex if needed
void my_sigactionhandler_oldcode(int32_t sig, int simple, int Locks, siginfo_t* info, void * ucntx, int* old_code, void* cur_db)
{
    // need to create some x86_ucontext????

    printf_log(LOG_DEBUG, "Sigactionhanlder for signal #%d called (jump to %p/%s), simple=%d\n", sig, (void*)my_context->signals[sig], GetNativeName((void*)my_context->signals[sig]), simple);

    uintptr_t restorer = my_context->restorer[sig];
    // get that actual ESP first!
    x86emu_t *emu = thread_get_emu();
    uintptr_t *frame = (uintptr_t*)R_ESP;
#if defined(DYNAREC) && defined(__arm__)
    dynablock_t* db = (dynablock_t*)cur_db;//FindDynablockFromNativeAddress(pc);
    ucontext_t *p = (ucontext_t *)ucntx;
    void* pc = NULL;
    if(ucntx) {
        pc = (void*)p->uc_mcontext.arm_pc;
        if(db) {
            frame = (uintptr_t*)p->uc_mcontext.arm_r8;
        }
    }
#else
    (void)ucntx; (void)cur_db;
#endif
    // stack tracking
	i386_stack_t *new_ss = my_context->onstack[sig]?(i386_stack_t*)pthread_getspecific(sigstack_key):NULL;
    int used_stack = 0;
    if(new_ss) {
        if(new_ss->ss_flags == SS_ONSTACK) { // already using it!
            frame = (uintptr_t*)emu->regs[_SP].dword[0];
        } else {
            frame = (uintptr_t*)(((uintptr_t)new_ss->ss_sp + new_ss->ss_size - 16) & ~7);
            used_stack = 1;
            new_ss->ss_flags = SS_ONSTACK;
        }
    } else {
        frame -= 0x200/sizeof(uintptr_t); // redzone
    }

    // TODO: do I need to really setup 2 stack frame? That doesn't seems right!
    // setup stack frame
    frame -= sizeof(siginfo_t)/sizeof(uintptr_t);
    siginfo_t* info2 = (siginfo_t*)frame;
    memcpy(info2, info, sizeof(siginfo_t));
    // try to fill some sigcontext....
    frame -= sizeof(i386_ucontext_t)/sizeof(uintptr_t);
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
    if(db && p) {
        sigcontext->uc_mcontext.gregs[REG_EAX] = p->uc_mcontext.arm_r4;
        sigcontext->uc_mcontext.gregs[REG_ECX] = p->uc_mcontext.arm_r5;
        sigcontext->uc_mcontext.gregs[REG_EDX] = p->uc_mcontext.arm_r6;
        sigcontext->uc_mcontext.gregs[REG_EBX] = p->uc_mcontext.arm_r7;
        sigcontext->uc_mcontext.gregs[REG_ESP] = p->uc_mcontext.arm_r8;
        sigcontext->uc_mcontext.gregs[REG_EBP] = p->uc_mcontext.arm_r9;
        sigcontext->uc_mcontext.gregs[REG_ESI] = p->uc_mcontext.arm_r10;
        sigcontext->uc_mcontext.gregs[REG_EDI] = p->uc_mcontext.arm_fp;
        sigcontext->uc_mcontext.gregs[REG_EIP] = getX86Address(db, (uintptr_t)pc);
        sigcontext->uc_mcontext.gregs[REG_EFL] = p->uc_mcontext.arm_ip;
        int special = isSpecialCases(sigcontext->uc_mcontext.gregs[REG_EIP], getDBX86N(db, (uintptr_t)pc));
        switch(special) {
            case CASE_MOVS:
                sigcontext->uc_mcontext.gregs[REG_ESI] -= p->uc_mcontext.arm_r3;
                break;
        }
    }
#endif
    // get FloatPoint status
    sigcontext->uc_mcontext.fpregs = (i386_fpregset_t)&sigcontext->xstate;
    save_fpreg(emu, sigcontext->uc_mcontext.fpregs);
    // add custom SIGN in reserved area
    //((unsigned int *)(&sigcontext.xstate.fpstate.padding))[8*4+12] = 0x46505853;  // not yet, when XSAVE / XRSTR will be ready
    // get signal mask

    if(new_ss) {
        sigcontext->uc_stack.ss_sp = new_ss->ss_sp;
        sigcontext->uc_stack.ss_size = new_ss->ss_size;
        sigcontext->uc_stack.ss_flags = new_ss->ss_flags;
    } else
        sigcontext->uc_stack.ss_flags = SS_DISABLE;
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
            sigcontext->uc_mcontext.gregs[REG_ERR] = (info->si_errno==0x1234)?0:0x0010;    // execution flag issue (probably), unless it's a #GP(0)
            sigcontext->uc_mcontext.gregs[REG_TRAPNO] = ((info->si_code==SEGV_ACCERR) || (info->si_errno==0x1234) || ((uintptr_t)info->si_addr==0))?13:14;
        } else if((info->si_code==SEGV_ACCERR) && !(prot&PROT_WRITE)) {
            sigcontext->uc_mcontext.gregs[REG_ERR] = 0x0002;    // write flag issue
            sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 14;
        } else {
            if((info->si_code!=SEGV_ACCERR) && abs((intptr_t)info->si_addr-(intptr_t)sigcontext->uc_mcontext.gregs[REG_ESP])<8)
                sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 12; // stack overflow probably
            else
                sigcontext->uc_mcontext.gregs[REG_TRAPNO] = (info->si_code==SEGV_ACCERR)?13:14;
            //REG_ERR seems to be INT:8 CODE:8. So for write access segfault it's 0x0002 For a read it's 0x0004 (and 8 for exec). For an int 2d it could be 0x2D01 for example
            sigcontext->uc_mcontext.gregs[REG_ERR] = 0x0004;    // read error? there is no execute control in box86 anyway, and no easy way to see if it's a write error
        }
        if(info->si_code == SEGV_ACCERR && old_code)
            *old_code = -1;
        if(info->si_errno == 0x1234) {
            info2->si_errno = 0;
            if(*(uint8_t*)info->si_addr == 0xCD) {
                sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 13;
                info2->si_addr = NULL;
                uint8_t int_n = *(uint8_t*)(info->si_addr+1);
                sigcontext->uc_mcontext.gregs[REG_ERR] = 0x2 | (int_n<<3);
                // some special cases...
                if(int_n==3) {
                    info2->si_signo = SIGTRAP;
                    sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 3;
                    sigcontext->uc_mcontext.gregs[REG_ERR] = 0;
                    sigcontext->uc_mcontext.gregs[REG_EIP]+=2;   // segfault after the INT
                } else if(int_n==0x04) {
                    sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 4;
                    sigcontext->uc_mcontext.gregs[REG_ERR] = 0;
                    sigcontext->uc_mcontext.gregs[REG_EIP]+=2;   // segfault after the INT
                }
            } else if(*(uint8_t*)info->si_addr == 0xCC) {
                info2->si_signo = SIGTRAP;
                sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 3;
                sigcontext->uc_mcontext.gregs[REG_ERR] = 0;
                sigcontext->uc_mcontext.gregs[REG_EIP]+=1;   // segfault after the INT
            } else if(*(uint8_t*)info->si_addr == 0xCE) {
                sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 4;
                sigcontext->uc_mcontext.gregs[REG_ERR] = 0;
                sigcontext->uc_mcontext.gregs[REG_EIP]+=1;   // segfault after the INTO
                info2->si_addr = NULL;
            } else {
                info2->si_code = 128;
                sigcontext->uc_mcontext.gregs[REG_ERR] = 0;
                sigcontext->uc_mcontext.gregs[REG_EIP] = (uintptr_t)info2->si_addr;
                info2->si_addr = NULL;
            }
        } else if(info->si_errno==0xcafe) {
            info2->si_errno = 0;
            sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 0;
            info2->si_signo = SIGFPE;
        }
    } else if(sig==SIGFPE) {
        if (info->si_code == FPE_INTOVF) {
            sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 4;
        } else {
            sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 19;
        }
    }
    else if(sig==SIGILL) {
        sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 6;
        sigcontext->uc_mcontext.gregs[REG_ERR] = 0x0000;
        info2->si_addr = (void*)sigcontext->uc_mcontext.gregs[REG_EIP];
        info2->si_code = 2;
    }
    else if(sig==SIGTRAP) {
        sigcontext->uc_mcontext.gregs[REG_TRAPNO] = 3;
        sigcontext->uc_mcontext.gregs[REG_ERR] = 0;
        sigcontext->uc_mcontext.gregs[REG_EIP] = (uintptr_t)info2->si_addr;
        info2->si_addr = NULL;
    }
    // call the signal handler
    i386_mcontext_t sigmcontext_copy = sigcontext->uc_mcontext;
    // save old value from emu
    #define GO(R) uint32_t old_##R = R_##R
    GO(EAX);
    GO(ECX);
    GO(EDX);
    #undef GO
    // set stack pointer
    R_ESP = (uintptr_t)frame;
    // set frame pointer
    R_EBP = sigcontext->uc_mcontext.gregs[REG_EBP];

    int exits = 0;
    int ret;
    int dynarec = 0;
    #ifdef DYNAREC
    if(sig!=SIGSEGV && !(Locks&is_dyndump_locked))
        dynarec = 1;
    #endif
    if(simple)
        ret = RunFunctionHandler(&exits, dynarec, sigcontext, my_context->signals[info2->si_signo], 1, info2->si_signo);
    else
        ret = RunFunctionHandler(&exits, dynarec, sigcontext, my_context->signals[info2->si_signo], 3, info2->si_signo, info2, sigcontext);
    // restore old value from emu
    if(used_stack)  // release stack
        new_ss->ss_flags = 0;
    #define GO(R) R_##R = old_##R
    GO(EAX);
    GO(ECX);
    GO(EDX);
    #undef GO

    if(memcmp(&sigcontext->uc_mcontext, &sigmcontext_copy, sizeof(i386_mcontext_t))) {
        if(emu->jmpbuf) {
            #define GO(R) emu->regs[_##R].dword[0]=sigcontext->uc_mcontext.gregs[REG_E##R]
            GO(AX);
            GO(CX);
            GO(DX);
            GO(DI);
            GO(SI);
            GO(BP);
            GO(SP);
            GO(BX);
            #undef GO
            emu->ip.dword[0]=sigcontext->uc_mcontext.gregs[REG_EIP];
            sigcontext->uc_mcontext.gregs[REG_EIP] = R_EIP;
            // flags
            emu->eflags.x32=sigcontext->uc_mcontext.gregs[REG_EFL];
            // get segments
            #define GO(S) emu->segs[_##S]=sigcontext->uc_mcontext.gregs[REG_##S]; emu->segs_serial[_##S] = 0
            GO(GS);
            GO(FS);
            GO(ES);
            GO(DS);
            GO(CS);
            GO(SS);
            #undef GO
            load_fpreg(emu, &sigcontext->xstate.fpstate);
            printf_log(LOG_DEBUG, "Context has been changed in Sigactionhanlder, doing siglongjmp to resume emu\n");
            if(old_code)
                *old_code = -1;    // re-init the value to allow another segfault at the same place
            if(used_stack)  // release stack
                new_ss->ss_flags = 0;
            //relockMutex(Locks);   // do not relock mutex, because of the siglongjmp, whatever was running is canceled
            #ifdef DYNAREC
            if(Locks & is_dyndump_locked)
                CancelBlock(1);
            #endif
            emu->xSPSave = emu->old_savedsp;
            #ifdef ANDROID
            siglongjmp(*emu->jmpbuf, 1);
            #else
            siglongjmp(emu->jmpbuf, 1);
            #endif
        }
        printf_log(LOG_INFO, "Warning, context has been changed in Sigactionhanlder%s\n", (sigcontext->uc_mcontext.gregs[REG_EIP]!=sigmcontext_copy.gregs[REG_EIP])?" (EIP changed)":"");
    }
    // restore regs...
    #define GO(R) emu->regs[_##R].dword[0] = sigcontext->uc_mcontext.gregs[REG_E##R]
    GO(AX);
    GO(CX);
    GO(DX);
    GO(DI);
    GO(SI);
    GO(BP);
    GO(SP);
    GO(BX);
    #undef GO
    emu->ip.dword[0] = sigcontext->uc_mcontext.gregs[REG_EIP];
    emu->eflags.x32 = sigcontext->uc_mcontext.gregs[REG_EFL];
    #define GO(S) emu->segs[_##S] = sigcontext->uc_mcontext.gregs[REG_##S]; emu->segs_serial[_##S] = 0
    GO(GS);
    GO(FS);
    GO(ES);
    GO(DS);
    GO(CS);
    GO(SS);
    #undef GO
    printf_log(LOG_DEBUG, "Sigactionhanlder main function returned (exit=%d, restorer=%p)\n", exits, (void*)restorer);
    if(exits) {
        //relockMutex(Locks);   // the thread will exit, so no relock there
        #ifdef DYNAREC
        if(Locks & is_dyndump_locked)
            CancelBlock();
        #endif
        exit(ret);
    }
    if(restorer)
        RunFunctionHandler(&exits, 0, NULL, restorer, 0);
    relockMutex(Locks);
}

extern void* current_helper;
#define USE_SIGNAL_MUTEX
#ifdef USE_SIGNAL_MUTEX
#ifdef USE_CUSTOM_MUTEX
static uint32_t mutex_dynarec_prot = 0;
#else
static pthread_mutex_t mutex_dynarec_prot = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
#endif
#define lock_signal()     mutex_lock(&mutex_dynarec_prot)
#define unlock_signal()   mutex_unlock(&mutex_dynarec_prot)
#else   // USE_SIGNAL_MUTEX
#define lock_signal()     
#define unlock_signal()   
#endif

extern int box86_quit;
extern int box86_exit_code;

void my_box86signalhandler(int32_t sig, siginfo_t* info, void * ucntx)
{
    // sig==SIGSEGV || sig==SIGBUS || sig==SIGILL here!
    int log_minimum = (box86_showsegv)?LOG_NONE:((my_context->is_sigaction[sig] && sig==SIGSEGV)?LOG_DEBUG:LOG_INFO);
    if((sig==SIGSEGV || sig==SIGBUS) && box86_quit) {
        printf_log(LOG_INFO, "Sigfault/Segbus while quitting, exiting silently\n");
        _exit(box86_exit_code);    // Hack, segfault while quiting, exit silently
    }
    ucontext_t *p = (ucontext_t *)ucntx;
    void* addr = (void*)info->si_addr;  // address that triggered the issue
    uintptr_t x86pc = (uintptr_t)-1;
    void* esp = NULL;
    x86emu_t* emu = thread_get_emu();
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
    int Locks = unlockMutex();
    uint32_t prot = getProtection((uintptr_t)addr);
    #ifdef BAD_SIGNAL
    // try to see if the si_code makes sense
    // the RK3588 tend to need a special Kernel that seems to have a weird behaviour sometimes
    if((sig==SIGSEGV) && (addr) && (info->si_code == 1) && prot&(PROT_READ|PROT_WRITE|PROT_EXEC)) {
        printf_log(LOG_DEBUG, "Workaround for suspicious si_code for %p / prot=0x%x\n", addr, prot);
        info->si_code = 2;
    }
    #endif
#ifdef DYNAREC
    if((Locks & is_dyndump_locked) && (sig==SIGSEGV) && current_helper) {
        CancelBlock(0);
        relockMutex(Locks);
        cancelFillBlock();  // Segfault inside a Fillblock
    }
    dynablock_t* db = NULL;
    int db_searched = 0;
    if ((sig==SIGSEGV) && (addr) && (info->si_code == SEGV_ACCERR) && (prot&PROT_CUSTOM)) {
        lock_signal();
        // check if SMC inside block
        db = FindDynablockFromNativeAddress(pc);
        db_searched = 1;
        static uintptr_t repeated_page = 0;
        dynarec_log(LOG_DEBUG, "SIGSEGV with Access error on %p for %p , db=%p(%p), prot=0x%hhx (old page=%p)\n", pc, addr, db, db?((void*)db->x86_addr):NULL, prot, (void*)repeated_page);
        static int repeated_count = 0;
        if(repeated_page == ((uintptr_t)addr&~(box86_pagesize-1))) {
            ++repeated_count;   // Access eoor multiple time on same page, disable dynarec on this page a few time...
            dynarec_log(LOG_DEBUG, "Detecting a Hotpage at %p (%d)\n", (void*)repeated_page, repeated_count);
            SetHotPage(repeated_page);
        } else {
            repeated_page = (uintptr_t)addr&~(box86_pagesize-1);
            repeated_count = 0;
        }
        // access error, unprotect the block (and mark them dirty)
        unprotectDB((uintptr_t)addr, 1, 1);    // unprotect 1 byte... But then, the whole page will be unprotected
        int db_need_test = (db && !box86_dynarec_fastpage)?getNeedTest((uintptr_t)db->x86_addr):0;
        dynarec_log(LOG_INFO/*LOG_DEBUG*/, "SIGSEGV with Access error on %p for %p , db=%p(%p)\n", pc, addr, db, db?((void*)db->x86_addr):NULL);
        if(db && ((addr>=db->x86_addr && addr<(db->x86_addr+db->x86_size)) || db_need_test)) {
            if(db && p->uc_mcontext.arm_r0>0x10000)
                emu = (x86emu_t*)p->uc_mcontext.arm_r0;
            // dynablock got auto-dirty! need to get out of it!!!
            if(emu->jmpbuf) {
                emu->regs[_AX].dword[0] = p->uc_mcontext.arm_r4;
                emu->regs[_CX].dword[0] = p->uc_mcontext.arm_r5;
                emu->regs[_DX].dword[0] = p->uc_mcontext.arm_r6;
                emu->regs[_BX].dword[0] = p->uc_mcontext.arm_r7;
                emu->regs[_SP].dword[0] = p->uc_mcontext.arm_r8;
                emu->regs[_BP].dword[0] = p->uc_mcontext.arm_r9;
                emu->regs[_SI].dword[0] = p->uc_mcontext.arm_r10;
                emu->regs[_DI].dword[0] = p->uc_mcontext.arm_fp;
                emu->ip.dword[0] = getX86Address(db, (uintptr_t)pc);
                emu->eflags.x32 = p->uc_mcontext.arm_ip;
                // check special case opcodes, to adjust regs
                int special = isSpecialCases(emu->ip.dword[0], getDBX86N(db, (uintptr_t)pc));
                switch(special) {
                    case CASE_MOVS:
                        emu->regs[_SI].dword[0] -= p->uc_mcontext.arm_r3;
                        break;
                }
                if(addr>=db->x86_addr && addr<(db->x86_addr+db->x86_size)) {
                    dynarec_log(LOG_INFO, "Auto-SMC detected, getting out of current Dynablock (speical=%d)!\n", special);
                } else {
                    dynarec_log(LOG_INFO, "Dynablock %p(%p) unprotected, getting out (arm pc=%p, x86_pc=%p, special=%d)!\n", db, db->x86_addr, pc, (void*)emu->ip.dword[0], special);
                }
                //relockMutex(Locks);   // do not relock because of he siglongjmp
                unlock_signal();
                if(Locks & is_dyndump_locked)
                    CancelBlock(1);
                emu->test.clean = 0;
                #ifdef ANDROID
                siglongjmp(*(JUMPBUFF*)emu->jmpbuf, 2);
                #else
                siglongjmp(emu->jmpbuf, 2);
                #endif
            }
            dynarec_log(LOG_INFO, "Warning, Auto-SMC (%p for db %p/%p) detected, but jmpbuffer not ready!\n", (void*)addr, db, (void*)db->x86_addr);
        }
        // done
        if(prot&PROT_WRITE) {
            unlock_signal();
            // if there is no write permission, don't return and continue to program signal handling
            relockMutex(Locks);
            return;
        }
        unlock_signal();
    } else if ((sig==SIGSEGV) && (addr) && (info->si_code == SEGV_ACCERR) && (prot&PROT_DYNAREC_R)) {
        // unprotect and continue to signal handler, because Write is not there on purpose
        unprotectDB((uintptr_t)addr, 1, 1);    // unprotect 1 byte... But then, the whole page will be unprotected
    } else if ((sig==SIGSEGV) && (addr) && (info->si_code == SEGV_ACCERR) && (prot&(PROT_READ|PROT_WRITE))) {
        lock_signal();
        db = FindDynablockFromNativeAddress(pc);
        db_searched = 1;
        if(db && db->x86_addr>= addr && (db->x86_addr+db->x86_size)<addr) {
            dynarec_log(LOG_INFO, "Warning, addr inside current dynablock!\n");
        }
        // mark stuff as unclean
        cleanDBFromAddressRange(((uintptr_t)addr)&~(box86_pagesize-1), box86_pagesize, 0);
        static void* glitch_pc = NULL;
        static void* glitch_addr = NULL;
        static int glitch_prot = 0;
        if(addr && pc /*&& db*/) {
            if((glitch_pc!=pc || glitch_addr!=addr || glitch_prot!=prot)) {
                // probably a glitch due to intensive multitask...
                dynarec_log(/*LOG_DEBUG*/LOG_INFO, "SIGSEGV with Access error on %p for %p, db=%p, prot=0x%x, retrying\n", pc, addr, db, prot);
                glitch_pc = pc;
                glitch_addr = addr;
                glitch_prot = prot;
                relockMutex(Locks);
                unlock_signal();
                return; // try again
            }
dynarec_log(/*LOG_DEBUG*/LOG_INFO, "Repeated SIGSEGV with Access error on %p for %p, db=%p, prot=0x%x\n", pc, addr, db, prot);
            glitch_pc = NULL;
            glitch_addr = NULL;
            glitch_prot = 0;
        }
        if(addr && pc && ((prot&(PROT_READ|PROT_WRITE))==(PROT_READ|PROT_WRITE))) {
            static void* glitch2_pc = NULL;
            static void* glitch2_addr = NULL;
            static int glitch2_prot = 0;
            if((glitch2_pc!=pc || glitch2_addr!=addr || glitch2_prot!=prot)) {
                dynarec_log(LOG_INFO, "Is that a multi process glitch too?\n");
                //printf_log(LOG_INFO, "Is that a multi process glitch too?\n");
                glitch2_pc = pc;
                glitch2_addr = addr;
                glitch2_prot = prot;
                sched_yield();  // give time to the other process
                refreshProtection((uintptr_t)addr);
                relockMutex(Locks);
                sched_yield();  // give time to the other process
                unlock_signal();
                return; // try again
            }
            glitch2_pc = NULL;
            glitch2_addr = NULL;
            glitch2_prot = 0;
        }
        unlock_signal();
    }
    if(!db_searched)
        db = FindDynablockFromNativeAddress(pc);
#else
    void* db = NULL;
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
        if(!db_searched)
            db = FindDynablockFromNativeAddress(pc);
#endif
        old_code = info->si_code;
        old_pc = pc;
        old_addr = addr;
        const char* name = GetNativeName(pc);
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
        elfheader_t* elf = FindElfAddress(my_context, x86pc);
        if(elf)
            x86name = getAddrFunctionName(x86pc);
        if(jit_gdb) {
            pid_t pid = getpid();
            int v = fork(); // is this ok in a signal handler???
            if(v) {
                // parent process, the one that have the segfault
                volatile int waiting = 1;
                printf("Waiting for %s (pid %d)...\n", (jit_gdb==2)?"gdbserver":"gdb", pid);
                while(waiting) {
                    // using gdb, use "set waiting=0" to stop waiting...
                    usleep(1000);
                }
            } else {
                char myarg[50] = {0};
                sprintf(myarg, "%d", pid);
                if(jit_gdb==2)
                    execlp("gdbserver", "gdbserver", "127.0.0.1:1234", "--attach", myarg, (char*)NULL);
                else if(jit_gdb==3)
                    execlp("lldb", "lldb", "-p", myarg, (char*)NULL);
                else
                    execlp("gdb", "gdb", "-pid", myarg, (char*)NULL);
                exit(-1);
            }
        }
        if(cycle_log) {
            print_cycle_log(log_minimum);
        }
#ifdef DYNAREC
        uint32_t hash = 0;
        if(db && ((addr<db->x86_addr || addr>(db->x86_addr+db->x86_size)) || (prot&PROT_READ)))
            hash = X31_hash_code(db->x86_addr, db->x86_size);
        printf_log(log_minimum, "%04d|%s @%p (%s) (x86pc=%p/\"%s\", esp=%p, stack=%p:%p own=%p fp=%p), for accessing %p (code=%d/prot=%x), db=%p(%p:%p/%p:%p/%s:%s, hash:%x/%x)", 
            GetTID(), signame, pc, name, (void*)x86pc, x86name?x86name:"???", esp, 
            emu->init_stack, emu->init_stack+emu->size_stack, emu->stack2free, (void*)R_EBP, 
            addr, info->si_code, prot, db, db?db->block:0, db?(db->block+db->size):0, 
            db?db->x86_addr:0, db?(db->x86_addr+db->x86_size):0, 
            getAddrFunctionName((uintptr_t)(db?db->x86_addr:0)), (db?getNeedTest((uintptr_t)db->x86_addr):0)?"need_stest":"clean", db?db->hash:0, hash);
#if defined(ARM)
        static const char* reg_name[] = {"EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"};
        if(db)
            for (int i=0; i<8; ++i) {
                if(!(i%4)) printf_log(log_minimum, "\n");
                printf_log(log_minimum, "%s:0x%08x ", reg_name[i], ((uint32_t*)&p->uc_mcontext.arm_r4)[i]);
            }
        if(esp!=addr)
            for (int i=-4; i<4; ++i) {
                printf_log(log_minimum, "%sESP%c0x%02x:0x%08x", (i%4)?" ":"\n", i<0?'-':'+', abs(i)*4, *(uintptr_t*)(esp+i*4));
            }
#else
        #warning TODO
#endif
#else
        printf_log(log_minimum, "%04d|%s @%p (%s) (x86pc=%p/\"%s\", esp=%p), for accessing %p (code=%d)", GetTID(), signame, pc, name, (void*)x86pc, x86name?x86name:"???", esp, addr, info->si_code);
#endif
        if(sig==SIGILL) {
            printf_log(log_minimum, " opcode=%02X %02X %02X %02X %02X %02X %02X %02X", ((uint8_t*)pc)[0], ((uint8_t*)pc)[1], ((uint8_t*)pc)[2], ((uint8_t*)pc)[3], ((uint8_t*)pc)[4], ((uint8_t*)pc)[5], ((uint8_t*)pc)[6], ((uint8_t*)pc)[7]);
            if(x86pc)
                printf_log(log_minimum, " x86opcode=%02X %02X %02X %02X %02X %02X %02X %02X\n", ((uint8_t*)x86pc)[0], ((uint8_t*)x86pc)[1], ((uint8_t*)x86pc)[2], ((uint8_t*)x86pc)[3], ((uint8_t*)x86pc)[4], ((uint8_t*)x86pc)[5], ((uint8_t*)x86pc)[6], ((uint8_t*)x86pc)[7]);
            else
                printf_log(log_minimum, "\n");
        } else if(sig==SIGBUS)
            printf_log(log_minimum, " x86opcode=%02X %02X %02X %02X %02X %02X %02X %02X\n", ((uint8_t*)x86pc)[0], ((uint8_t*)x86pc)[1], ((uint8_t*)x86pc)[2], ((uint8_t*)x86pc)[3], ((uint8_t*)x86pc)[4], ((uint8_t*)x86pc)[5], ((uint8_t*)x86pc)[6], ((uint8_t*)x86pc)[7]);
        else
            printf_log(log_minimum, "\n");
#ifndef ANDROID
        if(box86_showbt && (log_minimum<=box86_log)) {
            int nptrs;
            void *buffer[200];
            char **strings;

            printf_log(LOG_NONE, "Native bactrace:\n");
            nptrs = backtrace(buffer, 200);
            strings = backtrace_symbols(buffer, nptrs);
            for(int j=0; j<nptrs; j++)
                printf_log(LOG_NONE, "\t%s\n", strings[j]);
            box_free(strings);

        }
#endif
    }
    #if 0
    if(sig==SIGSEGV && (info->si_code==2 && ((prot&~PROT_CUSTOM)==7 || (prot&~PROT_CUSTOM)==5))) {
        /* there are some strange things happening with Terraria, where a segfault auccurs but the memory is perfectly accessible
        (probably some timing issue) */
        //box86_log=2;
        relockMutex(Locks);
        return;//why?
    }
    #endif
    if(my_context->signals[sig] && my_context->signals[sig]!=1) {
        my_sigactionhandler_oldcode(sig, my_context->is_sigaction[sig]?0:1, Locks, info, ucntx, &old_code, db);
        return;
    }
    // no handler (or double identical segfault)
    // set default and that's it, instruction will restart and default segfault handler will be called...
    if(my_context->signals[sig]!=1)
        signal(sig, SIG_DFL);
}

void my_sigactionhandler(int32_t sig, siginfo_t* info, void * ucntx)
{
    int Locks = unlockMutex();
    #ifdef DYNAREC
    ucontext_t *p = (ucontext_t *)ucntx;
    void * pc = (void*)p->uc_mcontext.arm_pc;
    dynablock_t* db = FindDynablockFromNativeAddress(pc);
    #else
    void* db = NULL;
    #endif

    my_sigactionhandler_oldcode(sig, 0, Locks, info, ucntx, NULL, db);
}

void emit_signal(x86emu_t* emu, int sig, void* addr, int code)
{
    int Locks = unlockMutex();
    void* db = NULL;
    siginfo_t info = {0};
    info.si_signo = sig;
    info.si_errno = (sig==SIGSEGV)?0x1234:0;    // Mark as a sign this is a #GP(0) (like privileged instruction)
    info.si_code = code;
    info.si_addr = addr;
    printf_log(LOG_INFO, "Emit Signal %d at IP=%p / addr=%p, code=%d\n", sig, (void*)R_EIP, addr, code);
    my_sigactionhandler_oldcode(sig, 0, Locks, &info, NULL, NULL, db);
}

void emit_div0(x86emu_t* emu, void* addr, int code)
{
    int Locks = unlockMutex();
    siginfo_t info = {0};
    info.si_signo = SIGSEGV;
    info.si_errno = 0xcafe;
    info.si_code = code;
    info.si_addr = addr;
    const char* x86name = NULL;
    const char* elfname = NULL;
    printf_log(LOG_INFO, "Emit Divide by 0 at IP=%p, addr=%p\n", (void*)R_EIP, addr);
    my_sigactionhandler_oldcode(SIGSEGV, 0, Locks, &info, NULL, NULL, NULL);
}

EXPORT sighandler_t my_signal(x86emu_t* emu, int signum, sighandler_t handler)
{
    if(signum<0 || signum>MAX_SIGNAL) {
        errno = EINVAL;
        return SIG_ERR;
    }

    if(signum==SIGSEGV && emu->context->no_sigsegv)
        return 0;

    // create a new handler
    my_context->signals[signum] = (uintptr_t)handler;
    my_context->is_sigaction[signum] = 0;
    my_context->restorer[signum] = 0;
    my_context->onstack[signum] = 0;
    if(signum==SIGSEGV || signum==SIGBUS || signum==SIGILL)
        return 0;
    if(handler!=NULL && handler!=(sighandler_t)1) {
        struct sigaction newact = {0};
        struct sigaction oldact = {0};
        newact.sa_flags = 0x04;
        newact.sa_sigaction = my_sigactionhandler;
        sigaction(signum, &newact, &oldact);
        return oldact.sa_handler;
    } else 
        return signal(signum, handler);
}
EXPORT sighandler_t my___sysv_signal(x86emu_t* emu, int signum, sighandler_t handler) __attribute__((alias("my_signal")));
EXPORT sighandler_t my_sysv_signal(x86emu_t* emu, int signum, sighandler_t handler) __attribute__((alias("my_signal")));    // not completly exact

int EXPORT my_sigaction(x86emu_t* emu, int signum, const x86_sigaction_t *act, x86_sigaction_t *oldact)
{
    if(signum<0 || signum>MAX_SIGNAL) {
        errno = EINVAL;
        return -1;
    }
    
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
                newact.sa_flags|=0x04;
                newact.sa_sigaction = my_sigactionhandler;
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
    if(signum<0 || signum>MAX_SIGNAL) {
        errno = EINVAL;
        return -1;
    }
    
    if(signum==SIGSEGV && emu->context->no_sigsegv)
        return 0;
    // TODO, how to handle sigsetsize>4?!
    if(signum==32 || signum==33) {
        // cannot use libc sigaction, need to use syscall!
        struct kernel_sigaction newact = {0};
        struct kernel_sigaction old = {0};
        if(act) {
            printf_log(LOG_DEBUG, " New (kernel) action flags=0x%x mask=0x%llx\n", act->sa_flags, *(uint64_t*)&act->sa_mask);
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
                    newact.sa_flags|=0x4;
                    newact.k_sa_handler = (void*)my_sigactionhandler;
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
            printf_log(LOG_DEBUG, " New action flags=0x%x mask=0x%llx\n", act->sa_flags, *(uint64_t*)&act->sa_mask);
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
                    newact.sa_sigaction = my_sigactionhandler;
                    newact.sa_flags|=0x4;
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
            memcpy(&oldact->sa_mask, &old.sa_mask, (sigsetsize>8)?8:sigsetsize);
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
    u->uc_mcontext.fpregs = ucp + 236;    // magic offset of fpregs in an actual i386 u_context
    fpu_savenv(emu, (void*)u->uc_mcontext.fpregs, 1);   // it seems getcontext only save fpu env, not fpu regs
    // get signal mask
    sigprocmask(SIG_SETMASK, NULL, (sigset_t*)&u->uc_sigmask);
    // ensure uc_link is properly initialized
    u->uc_link = emu->uc_link;

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
    if(u->uc_mcontext.fpregs)
        fpu_loadenv(emu, (void*)u->uc_mcontext.fpregs, 1);
    // set signal mask
    sigprocmask(SIG_SETMASK, (sigset_t*)&u->uc_sigmask, NULL);
    // set uc_link
    emu->uc_link = u->uc_link;
    errno = 0;

    return R_EAX;
}

EXPORT int my_makecontext(x86emu_t* emu, void* ucp, void* fnc, int32_t argc, int32_t* argv)
{
    (void)emu;
//    printf_log(LOG_NONE, "Warning: call to unimplemented makecontext\n");
    i386_ucontext_t *u = (i386_ucontext_t*)ucp;
    // setup stack
    uint32_t* esp = (uint32_t*)((uintptr_t)u->uc_stack.ss_sp + u->uc_stack.ss_size - 4);
    // setup the function
    u->uc_mcontext.gregs[REG_EIP] = (intptr_t)fnc;
    // setup args
    for (int i=0; i<argc; ++i) {
        // push value
        --esp;
        *esp = argv[(argc-1)-i];
    }
    // push the return value
    --esp;
    *esp = my_context->exit_bridge;
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
#ifdef USE_SIGNAL_MUTEX
static void atfork_child_dynarec_prot(void)
{
    #ifdef USE_CUSTOM_MUTEX
    native_lock_store(&mutex_dynarec_prot, 0);
    #else
    pthread_mutex_t tmp = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP; 
    memcpy(&mutex_dynarec_prot, &tmp, sizeof(mutex_dynarec_prot));
    #endif
}
#endif
void init_signal_helper(box86context_t* context)
{
    // setup signal handling
    for(int i=0; i<=MAX_SIGNAL; ++i) {
        context->signals[i] = 1;    // SIG_DFL
    }
	struct sigaction action = {0};
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
#ifdef USE_SIGNAL_MUTEX
    atfork_child_dynarec_prot();
    pthread_atfork(NULL, NULL, atfork_child_dynarec_prot);
#endif
}

void fini_signal_helper()
{
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGILL, SIG_DFL);
}
