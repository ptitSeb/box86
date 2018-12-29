#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
// for getrlimit()
#include <sys/time.h>
#include <sys/resource.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"

// Syscall table for x86 can be found here: http://shell-storm.org/shellcode/files/syscalls.html

void x86Syscall(x86emu_t *emu)
{
    uint32_t s = R_EAX;
    switch (s) {
        case 1: // __NR_exit
            emu->quit = 1;
            R_EAX = R_EBX; // faking the syscall here, we don't want to really terminate the program now
            break;
        case 4: // __NR_write
            R_EAX = syscall(__NR_write, R_EBX, R_ECX, R_EDX);
            break;
        case 174: // sys_rt_sigaction
            printf_log(LOG_NONE, "Warning, Ignoring sys_rt_sigaction(%X, %p, %p)\n", R_EBX, (void*)R_ECX, (void*)R_EDX);
            R_EAX = 0;
            break;
        case 191: // __NR_getrlimit sys_getrlimit
            //R_EAX = syscall(__NR_getrlimit, R_EBX, R_ECX);
            R_EAX = (uint32_t)getrlimit((int32_t)R_EBX, (struct rlimit*)R_ECX);
            break;
        default:
            printf_log(LOG_INFO, "Error: Unsupported Syscall 0x%02Xh (%d)\n", s, s);
            emu->quit = 1;
            emu->error |= ERR_UNIMPL;
    }
}

#define stack(n) (R_ESP+4+n)
#define i32(n)  *(int32_t*)stack(n)
#define u32(n)  *(uint32_t*)stack(n)
#define p(n)    *(void**)stack(n)

uint32_t my_syscall(x86emu_t *emu)
{
    uint32_t s = u32(0);
    switch (s) {
        case 1: // __NR_exit
            emu->quit = 1;
            return u32(4); // faking the syscall here, we don't want to really terminate the program now
            break;
        case 4: // __NR_write
            return syscall(__NR_write, u32(4), u32(8), u32(12));
            break;
        default:
            printf_log(LOG_INFO, "Error: Unsupported libc Syscall %02Xh (%d)\n", s, s);
            emu->quit = 1;
            emu->error |= ERR_UNIMPL;
    }
    return 0;
}
