#ifndef __SIGNALS_H__
#define __SIGNALS_H__
#include <signal.h>

typedef void (*sighandler_t)(int);
//typedef uint32_t x86_sigset_t;

typedef struct x86_sigaction_s {
	union {
	  __sighandler_t _sa_handler;
	  void (*_sa_sigaction)(int, siginfo_t *, void *);
	} _u;
	sigset_t sa_mask;
	uint32_t sa_flags;
	void (*sa_restorer)(void);
} x86_sigaction_t;

typedef struct x86_sigaction_restorer_s {
	union {
	  __sighandler_t _sa_handler;
	  void (*_sa_sigaction)(int, siginfo_t *, void *);
	} _u;
	uint32_t sa_flags;
	void (*sa_restorer)(void);
	sigset_t sa_mask;
} x86_sigaction_restorer_t;

sighandler_t my_signal(x86emu_t* emu, int signum, sighandler_t handler);
sighandler_t my___sysv_signal(x86emu_t* emu, int signum, sighandler_t handler);
sighandler_t my_sysv_signal(x86emu_t* emu, int signum, sighandler_t handler);

int my_sigaction(x86emu_t* emu, int signum, const x86_sigaction_t *act, x86_sigaction_t *oldact);
int my___sigaction(x86emu_t* emu, int signum, const x86_sigaction_t *act, x86_sigaction_t *oldact);

int my_syscall_sigaction(x86emu_t* emu, int signum, const x86_sigaction_restorer_t *act, x86_sigaction_restorer_t *oldact, int sigsetsize);

void init_signal_helper(box86context_t* context);
void fini_signal_helper();

void emit_signal(x86emu_t* emu, int sig, void* addr, int code);
void emit_div0(x86emu_t* emu, void* addr, int code);

#endif //__SIGNALS_H__