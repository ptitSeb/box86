#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <signal.h>
typedef void (*sighandler_t)(int);
#include <errno.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "library_private.h"
#include "x86emu.h"
#include "x86emu_private.h"

int32_t my___libc_start_main(x86emu_t* emu, int *(main) (int, char * *, char * *), 
    int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), 
    void (*rtld_fini) (void), void (* stack_end)); // implemented in x86run_private.c
uint32_t my_syscall(x86emu_t *emu); // implemented in x86syscall.c
void EXPORT my___stack_chk_fail(x86emu_t* emu)
{
    StopEmu(emu, "Stack is corrupted, abborting");
}
void EXPORT my___gmon_start__(x86emu_t *emu)
{
    printf_log(LOG_DEBUG, "__gmon_start__ called (dummy call)\n");
}
int EXPORT my___cxa_atexit(x86emu_t* emu, void* p)
{
    AddCleanup(emu, p);
}
int EXPORT my_atexit(x86emu_t* emu, void *p)
{
    AddCleanup(emu, p);
}
int EXPORT my_sigaction(x86emu_t* emu, int signum, const struct sigaction *act, struct sigaction *oldact)
{
    printf_log(LOG_NONE, "Warning, Ignoring sigaction(0x%02X, %p, %p)\n", signum, act, oldact);
    return 0;
}
int EXPORT my___sigaction(x86emu_t* emu, int signum, const struct sigaction *act, struct sigaction *oldact)
__attribute__((alias("my_sigaction")));

sighandler_t EXPORT my_signal(x86emu_t* emu, int signum, sighandler_t handler)
{
    printf_log(LOG_NONE, "Warning, Ignoring signal(0x%02X, %p)\n", signum, handler);
    return SIG_ERR;
}
sighandler_t EXPORT my___sysv_signal(x86emu_t* emu, int signum, sighandler_t handler) __attribute__((alias("my_signal")));
sighandler_t EXPORT my_sysv_signal(x86emu_t* emu, int signum, sighandler_t handler) __attribute__((alias("my_signal")));
pid_t EXPORT my_fork()
{
    // should be doable, but the x86emu stack has to be dedup for the child
    printf_log(LOG_NONE, "Warning, Ignoring fork()\n");
    return EAGAIN;
}
pid_t EXPORT my___fork() __attribute__((alias("my_fork")));

EXPORT void* my__ZGTtnaX (size_t a) { return NULL; }
EXPORT void my__ZGTtdlPv (void* a) { }
EXPORT uint8_t my__ITM_RU1(const uint8_t * a) { return 0; }
EXPORT uint32_t my__ITM_RU4(const uint32_t * a) { return 0; }
EXPORT uint64_t my__ITM_RU8(const uint64_t * a) { return 0; }
EXPORT void my__ITM_memcpyRtWn(void * a, const void * b, size_t c) { }
EXPORT void my__ITM_memcpyRnWt(void * a, const void * b, size_t c) { }
EXPORT void my__ITM_addUserCommitAction(void (*a)(void *), uint64_t b, void * c) { };

void my___longjmp_chk(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int __val);


int wrappedlibc_init(library_t* lib)
{
    lib->priv.w.lib = dlopen("libc.so.6", RTLD_NOW);
    if(!lib->priv.w.lib) {
        return -1;
    }
    lib->priv.w.bridge = NewBridge();
    return 0;
}
void wrappedlibc_fini(library_t* lib)
{
    if(lib->priv.w.lib)
        dlclose(lib->priv.w.lib);
    lib->priv.w.lib = NULL;
    FreeBridge(&lib->priv.w.bridge);
}
int wrappedlibc_get(library_t* lib, const char* name, uintptr_t *offs, uint32_t *sz)
{
    uintptr_t addr = 0;
    uint32_t size = 0;
    void* symbol = NULL;

#include "wrappedlib_defines.h"
#include "wrappedlibc_private.h"
#include "wrappedlib_undefs.h"


    if(!addr)
        return 0;
    *offs = addr;
    *sz = size;
    return 1;
}

void my___longjmp_chk(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int __val)
{
    printf_log(LOG_NONE, "Error: longjmp used\n");
    emu->quit = 1;
}