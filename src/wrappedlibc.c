#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <signal.h>
typedef void (*sighandler_t)(int);
#include <errno.h>
#include <stdarg.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "library_private.h"
#include "x86emu.h"
#include "x86emu_private.h"

// some my_XXX declare and defines
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

EXPORT void my_exit(x86emu_t *emu, int32_t status);
EXPORT void my__exit(x86emu_t *emu, int32_t status) __attribute__((alias("my_exit")));

void myStackAlign(const char* fmt, uint32_t* st, uint32_t* mystack); // align st into mystack according to fmt (for v(f)printf(...))
typedef int (*iFpp_t)(void*, void*);
typedef int (*iFppp_t)(void*, void*, void*);
EXPORT int my_printf(x86emu_t *emu, void* fmt, void* b, va_list V) {
    #ifdef __arm__
    // need to align on arm
    myStackAlign((const char*)fmt, b, emu->scratch);
    void* f = vprintf;
    return ((iFpp_t)f)(fmt, emu->scratch);
    #else
    // other platform don't need that
    return vprintf((const char*)fmt, V);
    #endif
}
EXPORT int my___printf_chk(x86emu_t *emu, void* fmt, void* b, va_list V) __attribute__((alias("my_printf")));
EXPORT int my_vprintf(x86emu_t *emu, void* fmt, void* b, va_list V) __attribute__((alias("my_printf")));
EXPORT int my___vprintf_chk(x86emu_t *emu, void* fmt, void* b, va_list V) __attribute__((alias("my_printf")));
EXPORT int my_vfprintf(x86emu_t *emu, void* F, void* fmt, void* b, va_list V) {
    #ifdef __arm__
    // need to align on arm
    myStackAlign((const char*)fmt, b, emu->scratch);
    void* f = vfprintf;
    return ((iFppp_t)f)(F, fmt, emu->scratch);
    #else
    // other platform don't need that
    return vfprintf((FILE*)F, (const char*)fmt, V);
    #endif
}
EXPORT int my___vfprintf_chk(x86emu_t *emu, void* F, void* fmt, void* b, va_list V) __attribute__((alias("my_vfprintf")));


#define LIBNAME libc
const char* libcName = "libc.so.6";

// define all standard library functions
#include "wrappedlib_init.h"

// my_XXXX Implementations
void my___longjmp_chk(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int __val)
{
    printf_log(LOG_NONE, "Error: longjmp used\n");
    emu->quit = 1;
}

void my_exit(x86emu_t *emu, int32_t status)
{
    R_EAX = (uint32_t)status;
    emu->quit = 1;
}

void myStackAlign(const char* fmt, uint32_t* st, uint32_t* mystack)
{
    // loop...
    const char* p = fmt;
    int state = 0;
    while(*p)
    {
        switch(state) {
            case 0:
                switch(*p) {
                    case '%': state = 1; ++p; break;
                    default:
                        ++p;
                }
                break;
            case 1: // normal
            case 2: // l
            case 3: // ll
            case 4: // L
                switch(*p) {
                    case '%': state = 0;  ++p; break; //%% = back to 0
                    case 'l': ++state; if (state>3) state=3; ++p; break;
                    case 'L': state = 4; ++p; break;
                    case 'a':
                    case 'A':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'F':
                    case 'f': state += 10; break;    //  float
                    case 'd':
                    case 'i':
                    case 'o':
                    case 'u':
                    case 'x':
                    case 'X': state += 20; break;   // int
                    case 'h': ++p; break;  // ignored...
                    case '\'':
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                    case '.': ++p; break; // formating, ignored
                    case 'm': state = 0; ++p; break; // no argument
                    case 'n':
                    case 'p':
                    case 's': state = 30; break; // pointers
                    case '$':
                    case '*': ++p; break; // should issue a warning, it's not handled...
                    case ' ': state=0; ++p; break;
                    default:
                        state=20; // other stuff, put an int...
                }
                break;
            case 11:    //double
            case 12:    //%lg, still double
            case 13:    //%llg, still double
            case 23:    // 64bits int
                if((((uint32_t)mystack)&0x7)!=0)
                    mystack++;
                *(uint64_t*)mystack = *(uint64_t*)st;
                st+=2; mystack+=2;
                state = 0;
                ++p;
                break;
            case 14:    //%LG long double
                *mystack = (uint32_t)st;    // put the address of the long double
                ++mystack;
                st+=3;
                state = 0;
                ++p;
                break;
            case 20:    // fallback
            case 21:
            case 22:
            case 24:    // normal int / pointer
            case 30:
                *mystack = *st;
                ++mystack;
                ++st;
                state = 0;
                ++p;
                break;
            default:
                // whattt?
                state = 0;
        }
    }
}