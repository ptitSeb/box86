#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/syscall.h>   /* For SYS_xxx definitions */
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>
// for getrlimit()
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"

// Syscall table for x86 can be found here: http://shell-storm.org/shellcode/files/syscalls.html
typedef struct scwrap_s {
    int x86s;
    int nats;
    int nbpars;
} scwrap_t;

scwrap_t syscallwrap[] = {
    { 3, __NR_read, 3 },
    { 4, __NR_write, 3 },
    { 5, __NR_open, 3 },
    { 6, __NR_close, 1 },
#ifdef __NR_time
    { 13, __NR_time, 1 },
#endif
    { 54, __NR_ioctl, 3 },
    { 85, __NR_readlink, 3 },
    { 195, __NR_stat64, 2 },
};

struct mmap_arg_struct {
    unsigned long addr;
    unsigned long len;
    unsigned long prot;
    unsigned long flags;
    unsigned long fd;
    unsigned long offset;
};

void EXPORT x86Syscall(x86emu_t *emu)
{
    uint32_t s = R_EAX;
    // check wrapper first
    int cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    for (int i=0; i<cnt; i++) {
        if(syscallwrap[i].x86s == s) {
            int sc = syscallwrap[i].nats;
            switch(syscallwrap[i].nbpars) {
                case 0: R_EAX = syscall(sc); return;
                case 1: R_EAX = syscall(sc, R_EBX); return;
                case 2: R_EAX = syscall(sc, R_EBX, R_ECX); return;
                case 3: R_EAX = syscall(sc, R_EBX, R_ECX, R_EDX); return;
                case 4: R_EAX = syscall(sc, R_EBX, R_ECX, R_EDX, R_ESI); return;
                case 5: R_EAX = syscall(sc, R_EBX, R_ECX, R_EDX, R_ESI, R_EDI); return;
                default:
                   printf_log(LOG_NONE, "ERROR, Unimplemented syscall wrapper (%d, %d)\n", s, syscallwrap[i].nbpars); 
                   emu->quit = 1;
                   return;
            }
        }
    }
    switch (s) {
        case 1: // sys_exit
            emu->quit = 1;
            R_EAX = R_EBX; // faking the syscall here, we don't want to really terminate the program now
            break;
#ifndef __NR_time
        case 13:    // sys_time (it's deprecated and remove on ARM EABI it seems)
            R_EAX = time(NULL);
            break;
#endif
        case 90:    // old_mmap
            {
                struct mmap_arg_struct *st = (struct mmap_arg_struct*)R_EBX;
                R_EAX = (uintptr_t)mmap((void*)st->addr, st->len, st->prot, st->flags, st->fd, st->offset);
            }
            break;
        case 174: // sys_rt_sigaction
            printf_log(LOG_NONE, "Warning, Ignoring sys_rt_sigaction(0x%02X, %p, %p)\n", R_EBX, (void*)R_ECX, (void*)R_EDX);
            R_EAX = 0;
            break;
        case 191: // sys_getrlimit
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

uint32_t EXPORT my_syscall(x86emu_t *emu)
{
    uint32_t s = u32(0);
    // check wrapper first
    int cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    for (int i=0; i<cnt; i++) {
        if(syscallwrap[i].x86s == s) {
            int sc = syscallwrap[i].nats;
            switch(syscallwrap[i].nbpars) {
                case 0: return syscall(sc);
                case 1: return syscall(sc, u32(4));
                case 2: return syscall(sc, u32(4), u32(8));
                case 3: return syscall(sc, u32(4), u32(8), u32(12));
                case 4: return syscall(sc, u32(4), u32(8), u32(12), u32(16));
                case 5: return syscall(sc, u32(4), u32(8), u32(12), u32(16), u32(20));
                default:
                   printf_log(LOG_NONE, "ERROR, Unimplemented syscall wrapper (%d, %d)\n", s, syscallwrap[i].nbpars); 
                   emu->quit = 1;
                   return 0;
            }
        }
    }
    switch (s) {
        case 1: // __NR_exit
            emu->quit = 1;
            return u32(4); // faking the syscall here, we don't want to really terminate the program now
            break;
        default:
            printf_log(LOG_INFO, "Error: Unsupported libc Syscall 0x%02X (%d)\n", s, s);
            emu->quit = 1;
            emu->error |= ERR_UNIMPL;
    }
    return 0;
}
