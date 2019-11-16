#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>

#include "box86context.h"
#include "debug.h"
#include "x86emu.h"
#include "emu/x86emu_private.h"
#include "signals.h"
#include "box86stack.h"


static box86context_t *context = NULL;  // global context, because signals are globals?

/* Definitions taken from the kernel headers.  */
enum
{
  REG_GS = 0,
# define REG_GS                REG_GS
  REG_FS,
# define REG_FS                REG_FS
  REG_ES,
# define REG_ES                REG_ES
  REG_DS,
# define REG_DS                REG_DS
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
struct i386_libc_fpreg
{
  uint16_t significand[4];
  uint16_t exponent;
};
struct i386_libc_fpstate
{
  uint32_t cw;
  uint32_t sw;
  uint32_t tag;
  uint32_t ipoff;
  uint32_t cssel;
  uint32_t dataoff;
  uint32_t datasel;
  struct i386_libc_fpreg _st[8];
  uint32_t status;
};

typedef struct i386_libc_fpstate *i386_fpregset_t;

typedef struct
  {
    void *ss_sp;
    int32_t ss_flags;
    uint32_t ss_size;
  } i386_stack_t;

typedef struct
  {
    i386_gregset_t gregs;
    i386_fpregset_t fpregs;
    uint32_t oldmask;
    uint32_t cr2;
  } i386_mcontext_t;

typedef uint32_t i386_sigset_t;
  
typedef struct i386_ucontext_s
  {
    uint32_t uc_flags;
    struct i386_ucontext_s *uc_link;
    i386_stack_t uc_stack;
    i386_mcontext_t uc_mcontext;
    i386_sigset_t uc_sigmask;
    struct i386_libc_fpstate __fpregs_mem;
    uint32_t __ssp[4];
  } i386_ucontext_t;



void my_sighandler(int32_t sig)
{
    Push32(context->signal_emu, sig);
    EmuCall(context->signal_emu, context->signals[sig]);
    Pop32(context->signal_emu);
}

void my_sigactionhandler(int32_t sig, siginfo_t* sigi, void * ucntx)
{
    // need to creat some x86_ucontext????
    Push32(context->signal_emu, (uintptr_t)ucntx);  // WRONG!!!
    Push32(context->signal_emu, (uintptr_t)sigi);
    Push32(context->signal_emu, sig);
    EmuCall(context->signal_emu, context->signals[sig]);
    Pop32(context->signal_emu);
    Pop32(context->signal_emu);
    Pop32(context->signal_emu);
}

static void CheckSignalContext(x86emu_t* emu)
{
    if(!context) {
        context = emu->context;
        if(!context->signal_emu) {
            context->signal_emu = NewX86Emu(context, 0, (uintptr_t)malloc(1024*1024*2), 2*1024*1024, 1);
            SetupX86Emu(context->signal_emu);
            SetTraceEmu(context->signal_emu, /*emu->trace_start, emu->trace_end*/0, 0);
        }
    }
}

EXPORT sighandler_t my_signal(x86emu_t* emu, int signum, sighandler_t handler)
{
    if(signum<0 || signum>=MAX_SIGNAL)
        return SIG_ERR;

    if(signum==SIGSEGV && emu->context->no_sigsegv)
        return 0;

    CheckSignalContext(emu);

    sighandler_t ret = NULL;
    if(handler!=NULL && handler!=(sighandler_t)1) {
        // create a new handler
        context->signals[signum] = (uintptr_t)handler;
        handler = my_sighandler;
    }
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

    CheckSignalContext(emu);

    struct sigaction newact;
    struct sigaction old;
    if(act) {
        newact.sa_mask = act->sa_mask;
        newact.sa_flags = act->sa_flags;
        if(act->sa_flags&0x04) {
            if(act->_u._sa_handler!=NULL && act->_u._sa_handler!=(sighandler_t)1) {
                context->signals[signum] = (uintptr_t)act->_u._sa_sigaction;
                newact.sa_sigaction = my_sigactionhandler;
            }
        } else {
            if(act->_u._sa_handler!=NULL && act->_u._sa_handler!=(sighandler_t)1) {
                context->signals[signum] = (uintptr_t)act->_u._sa_handler;
                newact.sa_handler = my_sighandler;
            }
        }
    }
    int ret = sigaction(signum, act?&newact:NULL, oldact?&old:NULL);
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
    // get segments (only FS)
    u->uc_mcontext.gregs[REG_FS] = R_FS;
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
    // get segments (only FS)
    R_FS = u->uc_mcontext.gregs[REG_FS];
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