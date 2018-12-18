#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"

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
        default:
            printf_debug(DEBUG_INFO, "Error: Unsupported Syscall %02Xh\n", s);
            emu->quit = 1;
    }
}