#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>

#include "box86context.h"
#include "debug.h"
#include "x86emu.h"
#include "x86emu_private.h"
#include "signals.h"
#include "stack.h"


static box86context_t *context = NULL;  // global context, because signals are globals?

#if 0
/* Definitions taken from the kernel headers.  */
# define __ctx(fld) fld
struct x86_libc_fpreg
{
  unsigned short int __ctx(significand)[4];
  unsigned short int __ctx(exponent);
};
struct x86_libc_fpstate
{
  unsigned long int __ctx(cw);
  unsigned long int __ctx(sw);
  unsigned long int __ctx(tag);
  unsigned long int __ctx(ipoff);
  unsigned long int __ctx(cssel);
  unsigned long int __ctx(dataoff);
  unsigned long int __ctx(datasel);
  struct x86_libc_fpreg _st[8];
  unsigned long int __ctx(status);
};
/* Structure to describe FPU registers.  */
typedef struct x86_libc_fpstate *fpregset_t;
/* Context to describe whole processor state.  */
typedef struct
  {
    gregset_t __ctx(gregs);
    /* Due to Linux's history we have to use a pointer here.  The SysV/i386
       ABI requires a struct with the values.  */
    fpregset_t __ctx(fpregs);
    unsigned long int __ctx(oldmask);
    unsigned long int __ctx(cr2);
  } mcontext_t;
/* Userlevel context.  */
typedef struct x86_ucontext_t
  {
    unsigned long int __ctx(uc_flags);
    struct x86_ucontext_t *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    sigset_t uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
    unsigned long int __ssp[4];
  } ucontext_t;
#endif

void my_sighandler(int32_t sig)
{
    Push32(context->signal_emu, sig);
    EmuCall(context->signal_emu, context->signals[sig]);
    Pop32(context->signal_emu);
}

void my_sigactionhandler(int32_t sig, struct siginfo * sigi, void * ucntx)
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
            SetupX86Emu(context->signal_emu, emu->shared_global, emu->globals);
            SetTraceEmu(context->signal_emu, /*emu->trace_start, emu->trace_end*/0, 0);
        }
    }
}

EXPORT sighandler_t my_signal(x86emu_t* emu, int signum, sighandler_t handler)
{
    if(signum<0 || signum>=MAX_SIGNAL)
        return SIG_ERR;

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
    int ret = sigaction(signum, &newact, oldact?&old:NULL);
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

