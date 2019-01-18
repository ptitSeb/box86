#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <signal.h>
typedef void (*sighandler_t)(int);
#include <errno.h>
#include <stdarg.h>

#include "wrappedlibs.h"

#include "stack.h"
#include "x86emu.h"
#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "callback.h"
#include "librarian.h"
#include "library_private.h"
#include "x86emu_private.h"
#include "box86context.h"
#include "myalign.h"


typedef int32_t (*iFpup_t)(void*, uint32_t, void*);

typedef struct libc_my_s {
    iFpup_t         _ITM_addUserCommitAction;
} libc_my_t;

void* getLIBCMy(library_t* lib)
{
    libc_my_t* my = (libc_my_t*)calloc(1, sizeof(libc_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(_ITM_addUserCommitAction, iFpup_t)
    #undef GO
    return my;
}

void freeLIBCMy(void* lib)
{
    // empty for now
}

void libc1ArgCallback(void *userdata)
{
    x86emu_t *emu = (x86emu_t*) userdata;
    RunCallback(emu);
}


// some my_XXX declare and defines
int32_t my___libc_start_main(x86emu_t* emu, int *(main) (int, char * *, char * *), 
    int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), 
    void (*rtld_fini) (void), void (* stack_end)); // implemented in x86run_private.c
uint32_t my_syscall(x86emu_t *emu); // implemented in x86syscall.c
void EXPORT my___stack_chk_fail(x86emu_t* emu)
{
    StopEmu(emu, "Stack is corrupted, abborting\n");
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

EXPORT void my_longjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val);
EXPORT void my__longjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val) __attribute__((alias("my_longjmp")));
EXPORT void my_siglongjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val) __attribute__((alias("my_longjmp")));
EXPORT void my___longjmp_chk(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val) __attribute__((alias("my_longjmp")));

EXPORT int32_t my_setjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p);
EXPORT int32_t my__setjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p) __attribute__((alias("my_setjmp")));
EXPORT int32_t my___sigsetjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p) __attribute__((alias("my_setjmp")));

EXPORT void my_exit(x86emu_t *emu, int32_t status)
{
    R_EAX = (uint32_t)status;
    emu->quit = 1;
}
EXPORT void my__exit(x86emu_t *emu, int32_t status) __attribute__((alias("my_exit")));

void myStackAlign(const char* fmt, uint32_t* st, uint32_t* mystack); // align st into mystack according to fmt (for v(f)printf(...))
typedef int (*iFpp_t)(void*, void*);
typedef int (*iFppp_t)(void*, void*, void*);
typedef int (*iFpupp_t)(void*, uint32_t, void*, void*);
EXPORT int my_printf(x86emu_t *emu, void* fmt, void* b, va_list V) {
    #ifndef NOALIGN
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

EXPORT int my_vprintf(x86emu_t *emu, void* fmt, void* b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, *(uint32_t**)b, emu->scratch);
    void* f = vprintf;
    return ((iFpp_t)f)(fmt, emu->scratch);
    #else
    // other platform don't need that
    return vprintf((const char*)fmt, V);
    #endif
}
EXPORT int my___vprintf_chk(x86emu_t *emu, void* fmt, void* b, va_list V) __attribute__((alias("my_vprintf")));

EXPORT int my_vfprintf(x86emu_t *emu, void* F, void* fmt, void* b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, *(uint32_t**)b, emu->scratch);
    void* f = vfprintf;
    return ((iFppp_t)f)(F, fmt, emu->scratch);
    #else
    // other platform don't need that
    return vfprintf((FILE*)F, (const char*)fmt, V);
    #endif
}
EXPORT int my___vfprintf_chk(x86emu_t *emu, void* F, void* fmt, void* b, va_list V) __attribute__((alias("my_vfprintf")));

EXPORT int my_fprintf(x86emu_t *emu, void* F, void* fmt, void* b, va_list V)  {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, b, emu->scratch);
    void* f = vfprintf;
    return ((iFppp_t)f)(F, fmt, emu->scratch);
    #else
    // other platform don't need that
    return vfprintf((FILE*)F, (const char*)fmt, V);
    #endif
}
EXPORT int my___fprintf_chk(x86emu_t *emu, void* F, void* fmt, void* b, va_list V) __attribute__((alias("my_fprintf")));

EXPORT int my_dl_iterate_phdr(x86emu_t *emu, void* F, void *data) {
    printf_log(LOG_NONE, "Error: unimplemented dl_iterate_phdr(%p, %p) used\n", F, data);
    emu->quit = 1;
}

EXPORT int my_snprintf(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, b, emu->scratch);
    void* f = vsnprintf;
    return ((iFpupp_t)f)(buff, s, fmt, emu->scratch);
    #else
    return vsnprintf((char*)buff, s, (char*)vsnprintf, V);
    #endif
}
EXPORT int my___snprintf_chk(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) __attribute__((alias("my_snprintf")));

EXPORT int my_sprintf(x86emu_t* emu, void* buff, void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, b, emu->scratch);
    void* f = vsprintf;
    return ((iFppp_t)f)(buff, fmt, emu->scratch);
    #else
    return vsprintf((char*)buff, (char*)fmt, V);
    #endif
}
EXPORT int my___sprintf_chk(x86emu_t* emu, void* buff, void * fmt, void * b, va_list V) __attribute__((alias("my_sprintf")));

EXPORT int my_vsnprintf(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, *(uint32_t**)b, emu->scratch);
    void* f = vsnprintf;
    int r = ((iFppp_t)f)(buff, n, fmt, emu->scratch);
    return r;
    #else
    return vsnprintf((char*)buff, s, (char*)fmt, V);
    #endif
}
EXPORT int my___vsnprintf(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) __attribute__((alias("my_vsnprintf")));
EXPORT int my___vsnprintf_chk(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) __attribute__((alias("my_vsnprintf")));

EXPORT int my_vswprintf(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, *(uint32_t**)b, emu->scratch);
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(buff, s, fmt, emu->scratch);
    return r;
    #else
    return vswprintf((wchar_t*)buff, s, (wchar_t*)fmt, V);
    #endif
}
EXPORT int my___vswprintf(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) __attribute__((alias("my_vswprintf")));
EXPORT int my___vswprintf_chk(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) __attribute__((alias("my_vswprintf")));

EXPORT void my__ITM_addUserCommitAction(x86emu_t* emu, void* cb, uint32_t b, void* c)
{
    // disabled for now... Are all this _ITM_ stuff really mendatory?
    #if 0
    // quick and dirty... Should store the callback to be removed later....
    libc_my_t *my = (libc_my_t *)emu->context->libclib->priv.w.p2;
    x86emu_t *cbemu = AddCallback(emu, (uintptr_t)cb, 1, c, NULL, NULL, NULL);
    my->_ITM_addUserCommitAction(libc1ArgCallback, b, cbemu);
    // should keep track of cbemu to remove at some point...
    #endif
}

#define LIBNAME libc
const char* libcName = "libc.so.6";

#define CUSTOM_INIT \
    box86->libclib = lib; \
    lib->priv.w.p2 = getLIBCMy(lib); \
    lib->priv.w.needed = 1; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("libpthread.so.0");

#define CUSTOM_FINI \
    freeLIBCMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);


// define all standard library functions
#include "wrappedlib_init.h"

// longjmp / setjmp
typedef struct jump_buff_i386_s {
 uint32_t save_ebx;
 uint32_t save_esi;
 uint32_t save_edi;
 uint32_t save_ebp;
 uint32_t save_esp;
 uint32_t save_eip;
} jump_buff_i386_t;

int32_t my_setjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p)
{
    jump_buff_i386_t *jpbuff = (jump_buff_i386_t*)p;
    // save the buffer
    jpbuff->save_ebx = R_EBX;
    jpbuff->save_esi = R_ESI;
    jpbuff->save_edi = R_EDI;
    jpbuff->save_ebp = R_EBP;
    jpbuff->save_esp = R_ESP;
    jpbuff->save_eip = *(uint32_t*)(R_ESP);
    // and that's it.. Nothing more for now
    return 0;
}

void my_longjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val)
{
    jump_buff_i386_t *jpbuff = (jump_buff_i386_t*)p;
    //restore  regs
    R_EBX = jpbuff->save_ebx;
    R_ESI = jpbuff->save_esi;
    R_EDI = jpbuff->save_edi;
    R_EBP = jpbuff->save_ebp;
    R_ESP = jpbuff->save_esp;
    // jmp to saved location, plus restore val to eax
    R_EAX = __val;
    R_EIP = jpbuff->save_eip;
}
