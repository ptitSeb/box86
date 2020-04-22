#define _LARGEFILE_SOURCE 1
#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <dlfcn.h>
#include <signal.h>
#include <errno.h>
#include <err.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <glob.h>
#include <ctype.h>
#include <dirent.h>
#include <search.h>
#include <sys/types.h>
#include <poll.h>
#include <sys/epoll.h>
#include <ftw.h>
#include <sys/syscall.h> 
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sys/mman.h>
#include <setjmp.h>

#include "wrappedlibs.h"

#include "box86stack.h"
#include "x86emu.h"
#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "callback.h"
#include "librarian.h"
#include "librarian/library_private.h"
#include "emu/x86emu_private.h"
#include "box86context.h"
#include "myalign.h"
#include "signals.h"
#include "fileutils.h"
#include "auxval.h"
#include "elfloader.h"

#ifdef PANDORA
#ifndef __NR_preadv
#define __NR_preadv                     (__NR_SYSCALL_BASE+361)
#endif
#ifndef __NR_pwritev
#define __NR_pwritev                    (__NR_SYSCALL_BASE+362)
#endif
#ifndef __NR_accept4
#define __NR_accept4                    (__NR_SYSCALL_BASE+366)
#endif
#ifndef __NR_sendmmsg
#define __NR_sendmmsg			        (__NR_SYSCALL_BASE+374)
#endif
#ifndef __NR_prlimit64
#define __NR_prlimit64                  (__NR_SYSCALL_BASE+369)
#endif
#endif

// need to undef all read / read64 stuffs!
#undef pread
#undef pwrite
#undef lseek
#undef fseeko
#undef ftello
#undef fseekpos
#undef fsetpos
#undef fgetpos
#undef fopen
#undef statfs
#undef fstatfs
#undef freopen
#undef truncate
#undef ftruncate
#undef tmpfile
#undef lockf
#undef fscanf
#undef scanf
#undef sscanf
#undef vfscanf
#undef vscanf
#undef vsscanf
#undef getc
#undef putc
#undef mkstemp
#undef mkstemps
#undef mkostemp
#undef mkostemps
#undef pread
#undef pwrite
#undef creat
#undef scandir
#undef mmap
#undef fcntl

#define LIBNAME libc
const char* libcName = "libc.so.6";

typedef int (*iFL_t)(unsigned long);
typedef void (*vFpp_t)(void*, void*);
typedef void (*vFipp_t)(int32_t, void*, void*);
typedef int32_t (*iFpi_t)(void*, int32_t);
typedef int32_t (*iFpp_t)(void*, void*);
typedef int32_t (*iFpup_t)(void*, uint32_t, void*);
typedef int32_t (*iFpuu_t)(void*, uint32_t, uint32_t);
typedef int32_t (*iFippi_t)(int32_t, void*, void*, int32_t);
typedef int32_t (*iFipuu_t)(int32_t, void*, uint32_t, uint32_t);
typedef int32_t (*iFipiI_t)(int32_t, void*, int32_t, int64_t);
typedef int32_t (*iFiiuuuuuu_t)(int32_t, int32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

#define SUPER() \
    GO(_ITM_addUserCommitAction, iFpup_t)   \
    GO(_IO_file_stat, iFpp_t)


typedef struct libc_my_s {
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} libc_my_t;

void* getLIBCMy(library_t* lib)
{
    libc_my_t* my = (libc_my_t*)calloc(1, sizeof(libc_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeLIBCMy(void* lib)
{
    // empty for now
}

void libc1ArgCallback(void *userdata)
{
    x86emu_t *emu = (x86emu_t*) userdata;
    RunCallback(emu);
}

// utility functions
#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \
GO(5)   \
GO(6)   \
GO(7)   \
GO(8)   \
GO(9)   \
GO(10)  \
GO(11)  \
GO(12)  \
GO(13)  \
GO(14)  \
GO(15)

// compare
#define GO(A)   \
static uintptr_t my_compare_fct_##A = 0;        \
static int my_compare_##A(void* a, void* b)     \
{                                               \
    return (int)RunFunctionFast(my_context, my_compare_fct_##A, 2, a, b);\
}
SUPER()
#undef GO
static void* findcompareFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_compare_fct_##A == (uintptr_t)fct) return my_compare_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_compare_fct_##A == 0) {my_compare_fct_##A = (uintptr_t)fct; return my_compare_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc compare callback\n");
    return NULL;
}
// compare_r
#define GO(A)   \
static uintptr_t my_compare_r_fct_##A = 0;                                      \
static int my_compare_r_##A(void* a, void* b, void* data)                       \
{                                                                               \
    return (int)RunFunctionFast(my_context, my_compare_r_fct_##A, 3, a, b, data);   \
}
SUPER()
#undef GO
static void* findcompare_rFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_compare_r_fct_##A == (uintptr_t)fct) return my_compare_r_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_compare_r_fct_##A == 0) {my_compare_r_fct_##A = (uintptr_t)fct; return my_compare_r_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc compare_r callback\n");
    return NULL;
}

// ftw
#define GO(A)   \
static uintptr_t my_ftw_fct_##A = 0;                                      \
static int my_ftw_##A(void* fpath, void* sb, int flag)                       \
{                                                                               \
    return (int)RunFunctionFast(my_context, my_ftw_fct_##A, 3, fpath, sb, flag);   \
}
SUPER()
#undef GO
static void* findftwFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_ftw_fct_##A == (uintptr_t)fct) return my_ftw_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ftw_fct_##A == 0) {my_ftw_fct_##A = (uintptr_t)fct; return my_ftw_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc ftw callback\n");
    return NULL;
}

// ftw64
#define GO(A)   \
static uintptr_t my_ftw64_fct_##A = 0;                      \
static int my_ftw64_##A(void* fpath, void* sb, int flag)    \
{                                                           \
    struct i386_stat64 i386st;                              \
    UnalignStat64(sb, &i386st);                             \
    return (int)RunFunctionFast(my_context, my_ftw64_fct_##A, 3, fpath, &i386st, flag);  \
}
SUPER()
#undef GO
static void* findftw64Fct(void* fct)
{
    if(!fct) return NULL;
    #define GO(A) if(my_ftw64_fct_##A == (uintptr_t)fct) return my_ftw64_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_ftw64_fct_##A == 0) {my_ftw64_fct_##A = (uintptr_t)fct; return my_ftw64_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc ftw64 callback\n");
    return NULL;
}

// nftw
#define GO(A)   \
static uintptr_t my_nftw_fct_##A = 0;                                   \
static int my_nftw_##A(void* fpath, void* sb, int flag, void* ftwbuff)  \
{                                                                       \
    return (int)RunFunctionFast(my_context, my_nftw_fct_##A, 4, fpath, sb, flag, ftwbuff);   \
}
SUPER()
#undef GO
static void* findnftwFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_nftw_fct_##A == (uintptr_t)fct) return my_nftw_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_nftw_fct_##A == 0) {my_nftw_fct_##A = (uintptr_t)fct; return my_nftw_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc nftw callback\n");
    return NULL;
}

// nftw64
#define GO(A)   \
static uintptr_t my_nftw64_fct_##A = 0;                                     \
static int my_nftw64_##A(void* fpath, void* sb, int flag, void* ftwbuff)    \
{                                                                           \
    struct i386_stat64 i386st;                                              \
    UnalignStat64(sb, &i386st);                                             \
    return (int)RunFunctionFast(my_context, my_nftw64_fct_##A, 4, fpath, &i386st, flag, ftwbuff);   \
}
SUPER()
#undef GO
static void* findnftw64Fct(void* fct)
{
    if(!fct) return NULL;
    #define GO(A) if(my_nftw64_fct_##A == (uintptr_t)fct) return my_nftw64_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_nftw64_fct_##A == 0) {my_nftw64_fct_##A = (uintptr_t)fct; return my_nftw64_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc nftw64 callback\n");
    return NULL;
}

// globerr
#define GO(A)   \
static uintptr_t my_globerr_fct_##A = 0;                                        \
static int my_globerr_##A(void* epath, int eerrno)                              \
{                                                                               \
    return (int)RunFunctionFast(my_context, my_globerr_fct_##A, 2, epath, eerrno);  \
}
SUPER()
#undef GO
static void* findgloberrFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_globerr_fct_##A == (uintptr_t)fct) return my_globerr_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_globerr_fct_##A == 0) {my_globerr_fct_##A = (uintptr_t)fct; return my_globerr_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc globerr callback\n");
    return NULL;
}
#undef dirent
// filter_dir
#define GO(A)   \
static uintptr_t my_filter_dir_fct_##A = 0;                               \
static int my_filter_dir_##A(const struct dirent* a)                    \
{                                                                       \
    return (int)RunFunctionFast(my_context, my_filter_dir_fct_##A, 1, a);     \
}
SUPER()
#undef GO
static void* findfilter_dirFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_filter_dir_fct_##A == (uintptr_t)fct) return my_filter_dir_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_filter_dir_fct_##A == 0) {my_filter_dir_fct_##A = (uintptr_t)fct; return my_filter_dir_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc filter_dir callback\n");
    return NULL;
}
// compare_dir
#define GO(A)   \
static uintptr_t my_compare_dir_fct_##A = 0;                                  \
static int my_compare_dir_##A(const struct dirent* a, const struct dirent* b)    \
{                                                                           \
    return (int)RunFunctionFast(my_context, my_compare_dir_fct_##A, 2, a, b);     \
}
SUPER()
#undef GO
static void* findcompare_dirFct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_compare_dir_fct_##A == (uintptr_t)fct) return my_compare_dir_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_compare_dir_fct_##A == 0) {my_compare_dir_fct_##A = (uintptr_t)fct; return my_compare_dir_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc compare_dir callback\n");
    return NULL;
}

// filter64
#define GO(A)   \
static uintptr_t my_filter64_fct_##A = 0;                               \
static int my_filter64_##A(const struct dirent64* a)                    \
{                                                                       \
    return (int)RunFunctionFast(my_context, my_filter64_fct_##A, 1, a);     \
}
SUPER()
#undef GO
static void* findfilter64Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_filter64_fct_##A == (uintptr_t)fct) return my_filter64_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_filter64_fct_##A == 0) {my_filter64_fct_##A = (uintptr_t)fct; return my_filter64_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc filter64 callback\n");
    return NULL;
}
// compare64
#define GO(A)   \
static uintptr_t my_compare64_fct_##A = 0;                                      \
static int my_compare64_##A(const struct dirent64* a, const struct dirent64* b) \
{                                                                               \
    return (int)RunFunctionFast(my_context, my_compare64_fct_##A, 2, a, b);         \
}
SUPER()
#undef GO
static void* findcompare64Fct(void* fct)
{
    if(!fct) return NULL;
    void* p;
    if((p = GetNativeFnc((uintptr_t)fct))) return p;
    #define GO(A) if(my_compare64_fct_##A == (uintptr_t)fct) return my_compare64_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_compare64_fct_##A == 0) {my_compare64_fct_##A = (uintptr_t)fct; return my_compare64_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for libc compare64 callback\n");
    return NULL;
}

#undef SUPER

// some my_XXX declare and defines
int32_t my___libc_start_main(x86emu_t* emu, int *(main) (int, char * *, char * *), 
    int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), 
    void (*rtld_fini) (void), void (* stack_end)); // implemented in x86run_private.c
EXPORT void my___libc_init_first(x86emu_t* emu, int argc, char* arg0, char** b)
{
    // do nothing specific for now
    return;
}
uint32_t my_syscall(x86emu_t *emu); // implemented in x86syscall.c
void EXPORT my___stack_chk_fail(x86emu_t* emu)
{
    char buff[200];
    #ifdef HAVE_TRACE
    sprintf(buff, "%p: Stack is corrupted, aborting (prev IP=%p->%p)\n", (void*)emu->old_ip, (void*)emu->prev2_ip, (void*)emu->prev_ip);
    #else
    sprintf(buff, "%p: Stack is corrupted, aborting\n", (void*)emu->old_ip);
    #endif
    StopEmu(emu, buff);
}
void EXPORT my___gmon_start__(x86emu_t *emu)
{
    printf_log(LOG_DEBUG, "__gmon_start__ called (dummy call)\n");
}
int EXPORT my___cxa_atexit(x86emu_t* emu, void* p, void* a, void* d)
{
    AddCleanup1Arg(emu, p, a);
    return 0;
}
void EXPORT my___cxa_finalize(x86emu_t* emu, void* p)
{
    if(!p) {
        // p is null, call (and remove) all Cleanup functions
        CallAllCleanup(emu);
    } else {
        CallCleanup(emu, p);
    }
}
int EXPORT my_atexit(x86emu_t* emu, void *p)
{
    AddCleanup(emu, p);
    return 0;
}

int my_getcontext(x86emu_t* emu, void* ucp);
int my_setcontext(x86emu_t* emu, void* ucp);
int my_makecontext(x86emu_t* emu, void* ucp, void* fnc, int32_t argc, void* argv);
int my_swapcontext(x86emu_t* emu, void* ucp1, void* ucp2);

// All signal and context functions defined in signals.c

// All fts function defined in myfts.c

// getauxval implemented in auxval.c


// this one is defined in elfloader.c
int my_dl_iterate_phdr(x86emu_t *emu, void* F, void *data);


pid_t EXPORT my_fork(x86emu_t* emu)
{
    #if 1
    emu->quit = 1;
    emu->fork = 1;
    return 0;
    #else
    return 0;
    #endif
}
pid_t EXPORT my___fork(x86emu_t* emu) __attribute__((alias("my_fork")));
pid_t EXPORT my_vfork(x86emu_t* emu)
{
    #if 1
    emu->quit = 1;
    emu->fork = 1;  // use regular fork...
    return 0;
    #else
    return 0;
    #endif
}

int EXPORT my_uname(struct utsname *buf)
{
    // sizeof(struct utsname)==390 on i686, and also on ARM, so this seem safe
    int ret = uname(buf);
    strcpy(buf->machine, /*(box86_steam)?"x86_64":*/"i686");
    return ret;
}


EXPORT void* my__ZGTtnaX (size_t a) { printf("warning _ZGTtnaX called\n"); return NULL; }
EXPORT void my__ZGTtdlPv (void* a) { printf("warning _ZGTtdlPv called\n"); }
EXPORT uint8_t my__ITM_RU1(const uint8_t * a) { printf("warning _ITM_RU1 called\n"); return 0; }
EXPORT uint32_t my__ITM_RU4(const uint32_t * a) { printf("warning _ITM_RU4 called\n"); return 0; }
EXPORT uint64_t my__ITM_RU8(const uint64_t * a) { printf("warning _ITM_RU8 called\n"); return 0; }
EXPORT void my__ITM_memcpyRtWn(void * a, const void * b, size_t c) {printf("warning _ITM_memcpyRtWn called\n");  }
EXPORT void my__ITM_memcpyRnWt(void * a, const void * b, size_t c) {printf("warning _ITM_memcpyRtWn called\n"); }

EXPORT void my_longjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val);
EXPORT void my__longjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val) __attribute__((alias("my_longjmp")));
EXPORT void my_siglongjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val) __attribute__((alias("my_longjmp")));
EXPORT void my___longjmp_chk(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val) __attribute__((alias("my_longjmp")));

EXPORT int32_t my_setjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p);
EXPORT int32_t my__setjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p) __attribute__((alias("my_setjmp")));
EXPORT int32_t my___sigsetjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p) __attribute__((alias("my_setjmp")));
#if 0
EXPORT void my_exit(x86emu_t *emu, int32_t status)
{
    R_EAX = (uint32_t)status;
    emu->quit = 1;
}
EXPORT void my__exit(x86emu_t *emu, int32_t status) __attribute__((alias("my_exit")));
EXPORT void my__Exit(x86emu_t *emu, int32_t status) __attribute__((alias("my_exit")));
#endif
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
    void* f = vprintf;
    return ((iFpp_t)f)(fmt, *(uint32_t**)b);
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
    void* f = vfprintf;
    return ((iFppp_t)f)(F, fmt, *(uint32_t**)b);
    #endif
}
EXPORT int my___vfprintf_chk(x86emu_t *emu, void* F, void* fmt, void* b, va_list V) __attribute__((alias("my_vfprintf")));
EXPORT int my__IO_vfprintf(x86emu_t *emu, void* F, void* fmt, void* b, va_list V) __attribute__((alias("my_vfprintf")));

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

EXPORT int my_wprintf(x86emu_t *emu, void* fmt, void* b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlignW((const char*)fmt, b, emu->scratch);
    void* f = vwprintf;
    return ((iFpp_t)f)(fmt, emu->scratch);
    #else
    // other platform don't need that
    return vwprintf((const wchar_t*)fmt, V);
    #endif
}
EXPORT int my___wprintf_chk(x86emu_t *emu, int flag, void* fmt, void* b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlignW((const char*)fmt, b, emu->scratch);
    void* f = vwprintf;
    return ((iFpp_t)f)(fmt, emu->scratch);
    #else
    // other platform don't need that
    return vwprintf((const wchar_t*)fmt, V);
    #endif
}
EXPORT int my_fwprintf(x86emu_t *emu, void* F, void* fmt, void* b, va_list V)  {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlignW((const char*)fmt, b, emu->scratch);
    void* f = vfwprintf;
    return ((iFppp_t)f)(F, fmt, emu->scratch);
    #else
    // other platform don't need that
    return vfwprintf((FILE*)F, (const wchar_t*)fmt, V);
    #endif
}
EXPORT int my___fwprintf_chk(x86emu_t *emu, void* F, void* fmt, void* b, va_list V) __attribute__((alias("my_fwprintf")));

EXPORT int my_vfwprintf(x86emu_t *emu, void* F, void* fmt, void* b) {
    #ifndef NOALIGN
    myStackAlignW((const char*)fmt, b, emu->scratch);
    void* f = vfwprintf;
    return ((iFppp_t)f)(F, fmt, emu->scratch);
    #else
    void* f = vfwprintf;
    return ((iFppp_t)f)(F, fmt, b);
    #endif
}

EXPORT int my_vwprintf(x86emu_t *emu, void* fmt, void* b) {
    #ifndef NOALIGN
    myStackAlignW((const char*)fmt, b, emu->scratch);
    void* f = vwprintf;
    return ((iFpp_t)f)(fmt, emu->scratch);
    #else
    void* f = vwprintf;
    return ((iFpp_t)f)(fmt, b);
    #endif
}

EXPORT void *my_div(void *result, int numerator, int denominator) {
    *(div_t *)result = div(numerator, denominator);
    return result;
}

EXPORT int my_snprintf(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, b, emu->scratch);
    void* f = vsnprintf;
    return ((iFpupp_t)f)(buff, s, fmt, emu->scratch);
    #else
    return vsnprintf((char*)buff, s, (char*)fmt, V);
    #endif
}
EXPORT int my___snprintf_chk(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) __attribute__((alias("my_snprintf")));
EXPORT int my___snprintf(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) __attribute__((alias("my_snprintf")));

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

EXPORT int my_asprintf(x86emu_t* emu, void** buff, void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, b, emu->scratch);
    void* f = vasprintf;
    return ((iFppp_t)f)(buff, fmt, emu->scratch);
    #else
    return vasprintf((char**)buff, (char*)fmt, V);
    #endif
}
EXPORT int my___asprintf(x86emu_t* emu, void** buff, void * fmt, void * b, va_list V) __attribute__((alias("my_asprintf")));


EXPORT int my_vsprintf(x86emu_t* emu, void* buff,  void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, *(uint32_t**)b, emu->scratch);
    void* f = vsprintf;
    int r = ((iFppp_t)f)(buff, fmt, emu->scratch);
    return r;
    #else
    void* f = vsprintf;
    int r = ((iFppp_t)f)(buff, fmt, *(uint32_t**)b);
    return r;
    #endif
}
EXPORT int my___vsprintf_chk(x86emu_t* emu, void* buff, void * fmt, void * b, va_list V) __attribute__((alias("my_vsprintf")));

EXPORT int my_vfscanf(x86emu_t* emu, void* stream, void* fmt, void* b) // probably uneeded to do a GOM, a simple wrap should enough
{
    void* f = vfscanf;
    return ((iFppp_t)f)(stream, fmt, *(uint32_t**)b);
}
EXPORT int my__IO_vfscanf(x86emu_t* emu, void* stream, void* fmt, void* b) __attribute__((alias("my_vfscanf")));

EXPORT int my_vsnprintf(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, *(uint32_t**)b, emu->scratch);
    void* f = vsnprintf;
    int r = ((iFpupp_t)f)(buff, s, fmt, emu->scratch);
    return r;
    #else
    void* f = vsnprintf;
    int r = ((iFpupp_t)f)(buff, s, fmt, *(uint32_t**)b);
    return r;
    #endif
}
EXPORT int my___vsnprintf(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) __attribute__((alias("my_vsnprintf")));
EXPORT int my___vsnprintf_chk(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) __attribute__((alias("my_vsnprintf")));

EXPORT int my_vasprintf(x86emu_t* emu, void* strp, void* fmt, void* b, va_list V)
{
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, *(uint32_t**)b, emu->scratch);
    void* f = vasprintf;
    int r = ((iFppp_t)f)(strp, fmt, emu->scratch);
    return r;
    #else
    void* f = vasprintf;
    int r = ((iFppp_t)f)(strp, fmt, *(uint32_t**)b);
    return r;
    #endif
}
EXPORT int my___vasprintf_chk(x86emu_t* emu, void* strp, int flags, void* fmt, void* b, va_list V)
{
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, *(uint32_t**)b, emu->scratch);
    void* f = vasprintf;
    int r = ((iFppp_t)f)(strp, fmt, emu->scratch);
    return r;
    #else
    void* f = vasprintf;
    int r = ((iFppp_t)f)(strp, fmt, *(uint32_t**)b);
    return r;
    #endif
}

EXPORT int my___asprintf_chk(x86emu_t* emu, void* result_ptr, int flags, void* fmt, void* b, va_list V)
{
    #ifndef NOALIGN
    myStackAlign((const char*)fmt, b, emu->scratch);
    void* f = vasprintf;
    return ((iFppp_t)f)(result_ptr, fmt, emu->scratch);
    #else
    return vasprintf((char**)result_ptr, (char*)fmt, V);
    #endif
}

EXPORT int32_t my_obstack_vprintf(x86emu_t* emu, void* obstack, void* fmt, void* b, va_list V)
{
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, *(uint32_t**)b, emu->scratch);
    void* f = obstack_vprintf;
    int r = ((iFppp_t)f)(obstack, fmt, emu->scratch);
    return r;
    #else
    void* f = obstack_vprintf;
    int r = ((iFppp_t)f)(obstack, fmt, *(uint32_t**)b);
    return r;
    #endif
}

EXPORT int my_vswprintf(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlignW((const char*)fmt, *(uint32_t**)b, emu->scratch);
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(buff, s, fmt, emu->scratch);
    return r;
    #else
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(buff, s, fmt, *(uint32_t**)b);
    return r;
    #endif
}
EXPORT int my___vswprintf(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) __attribute__((alias("my_vswprintf")));
EXPORT int my___vswprintf_chk(x86emu_t* emu, void* buff, uint32_t s, void * fmt, void * b, va_list V) __attribute__((alias("my_vswprintf")));

EXPORT void my_verr(x86emu_t* emu, int eval, void* fmt, void* b) {
    #ifndef NOALIGN
    myStackAlignW((const char*)fmt, *(uint32_t**)b, emu->scratch);
    void* f = verr;
    ((vFipp_t)f)(eval, fmt, emu->scratch);
    #else
    void* f = verr;
    ((vFipp_t)f)(eval, fmt, *(uint32_t**)b);
    #endif
}

EXPORT void my_vwarn(x86emu_t* emu, void* fmt, void* b) {
    #ifndef NOALIGN
    myStackAlignW((const char*)fmt, *(uint32_t**)b, emu->scratch);
    void* f = vwarn;
    ((vFpp_t)f)(fmt, emu->scratch);
    #else
    void* f = vwarn;
    ((vFpp_t)f)(fmt, *(uint32_t**)b);
    #endif
}

EXPORT int my___swprintf_chk(x86emu_t* emu, void* s, uint32_t n, int32_t flag, uint32_t slen, void* fmt, void * b)
{
    #ifndef NOALIGN
    myStackAlignW((const char*)fmt, b, emu->scratch);
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(s, n, fmt, emu->scratch);
    return r;
    #else
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(s, n, fmt, b);
    return r;
    #endif
}
EXPORT int my_swprintf(x86emu_t* emu, void* s, uint32_t n, void* fmt, void *b)
{
    #ifndef NOALIGN
    myStackAlignW((const char*)fmt, b, emu->scratch);
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(s, n, fmt, emu->scratch);
    return r;
    #else
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(s, n, fmt, b);
    return r;
    #endif
}

EXPORT void my__ITM_addUserCommitAction(x86emu_t* emu, void* cb, uint32_t b, void* c)
{
    // disabled for now... Are all this _ITM_ stuff really mendatory?
    #if 0
    // quick and dirty... Should store the callback to be removed later....
    libc_my_t *my = (libc_my_t *)emu->context->libclib->priv.w.p2;
    x86emu_t *cbemu = AddCallback(emu, (uintptr_t)cb, 1, c, NULL, NULL, NULL);
    my->_ITM_addUserCommitAction(libc1ArgCallback, b, cbemu);
    // should keep track of cbemu to remove at some point...
    #else
    printf("warning _ITM_addUserCommitAction called\n");
    #endif
}
EXPORT void my__ITM_registerTMCloneTable(x86emu_t* emu, void* p, uint32_t s) {}
EXPORT void my__ITM_deregisterTMCloneTable(x86emu_t* emu, void* p) {}


EXPORT int my___fxstat64(x86emu_t *emu, int vers, int fd, void* buf)
{
    struct stat64 st;
    int r = fstat64(fd, &st);
    //int r = syscall(__NR_stat64, fd, &st);
    UnalignStat64(&st, buf);
    return r;
}

EXPORT int my___xstat64(x86emu_t* emu, int v, void* path, void* buf)
{
    struct stat64 st;
    int r = stat64((const char*)path, &st);
    UnalignStat64(&st, buf);
    return r;
}

EXPORT int my___lxstat64(x86emu_t* emu, int v, void* name, void* buf)
{
    struct stat64 st;
    int r = lstat64((const char*)name, &st);
    UnalignStat64(&st, buf);
    return r;
}

EXPORT int my___fxstatat64(x86emu_t* emu, int v, int d, void* path, void* buf, int flags)
{
    struct  stat64 st;
    int r = fstatat64(d, path, &st, flags);
    UnalignStat64(&st, buf);
    return r;
}

EXPORT int my__IO_file_stat(x86emu_t* emu, void* f, void* buf)
{
    struct stat64 st;
    libc_my_t *my = (libc_my_t *)emu->context->libclib->priv.w.p2;
    int r = my->_IO_file_stat(f, &st);
    UnalignStat64(&st, buf);
    return r;
}

EXPORT void my_qsort(x86emu_t* emu, void* base, size_t nmemb, size_t size, void* fnc)
{
    qsort(base, nmemb, size, findcompareFct(fnc));
}
EXPORT void my_qsort_r(x86emu_t* emu, void* base, size_t nmemb, size_t size, void* fnc, void* arg)
{
    qsort_r(base, nmemb, size, findcompare_rFct(fnc), arg);
}

EXPORT void* my_bsearch(x86emu_t* emu, void* key, void* base, size_t nmemb, size_t size, void* fnc)
{
    return bsearch(key, base, nmemb, size, findcompareFct(fnc));
}

EXPORT void* my_lsearch(x86emu_t* emu, void* key, void* base, size_t* nmemb, size_t size, void* fnc)
{
    return lsearch(key, base, nmemb, size, findcompareFct(fnc));
}
EXPORT void* my_lfind(x86emu_t* emu, void* key, void* base, size_t* nmemb, size_t size, void* fnc)
{
    return lfind(key, base, nmemb, size, findcompareFct(fnc));
}

EXPORT int32_t my_readlink(x86emu_t* emu, void* path, void* buf, uint32_t sz)
{
    if(strcmp((const char*)path, "/proc/self/exe")==0) {
        // special case for self...
        return strlen(strncpy((char*)buf, emu->context->fullpath, sz));
    }
    if(strncmp((const char*)path, "/proc/", 6)==0) {
        // check if self checking....
        pid_t pid = getpid();
        char tmp[64];
        sprintf(tmp, "/proc/%d/exe", pid);
        if(strcmp((const char*)path, tmp)==0) {
            return strlen(strncpy((char*)buf, emu->context->fullpath, sz));
        }
    }
    return readlink((const char*)path, (char*)buf, sz);
}
#define TMP_CMDLINE "box86_tmpcmdline"
EXPORT int32_t my_open(x86emu_t* emu, void* pathname, int32_t flags, uint32_t mode)
{
    if(strcmp((const char*)pathname, "/proc/self/cmdline")==0) {
        // special case for self command line...
        #if 0
        char tmpcmdline[200] = {0};
        char tmpbuff[100] = {0};
        sprintf(tmpbuff, "%s/cmdlineXXXXXX", getenv("TMP")?getenv("TMP"):".");
        int tmp = mkstemp(tmpbuff);
        int dummy;
        if(tmp<0) return open(pathname, flags, mode);
        dummy = write(tmp, emu->context->fullpath, strlen(emu->context->fullpath)+1);
        for (int i=1; i<emu->context->argc; ++i)
            dummy = write(tmp, emu->context->argv[i], strlen(emu->context->argv[i])+1);
        lseek(tmp, 0, SEEK_SET);
        #else
        int tmp = shm_open(TMP_CMDLINE, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open(pathname, flags, mode);
        shm_unlink(TMP_CMDLINE);    // remove the shm file, but it will still exist because it's currently in use
        int dummy = write(tmp, emu->context->fullpath, strlen(emu->context->fullpath)+1);
        (void)dummy;
        for (int i=1; i<emu->context->argc; ++i)
            dummy = write(tmp, emu->context->argv[i], strlen(emu->context->argv[i])+1);
        lseek(tmp, 0, SEEK_SET);
        #endif
        return tmp;
    }
    return open(pathname, flags, mode);
}
EXPORT int32_t my___open(x86emu_t* emu, void* pathname, int32_t flags, uint32_t mode) __attribute__((alias("my_open")));

EXPORT int32_t my_open64(x86emu_t* emu, void* pathname, int32_t flags, uint32_t mode)
{
    if(strcmp((const char*)pathname, "/proc/self/cmdline")==0) {
        // special case for self command line...
        #if 0
        char tmpcmdline[200] = {0};
        char tmpbuff[100] = {0};
        sprintf(tmpbuff, "%s/cmdlineXXXXXX", getenv("TMP")?getenv("TMP"):".");
        int tmp = mkstemp64(tmpbuff);
        int dummy;
        if(tmp<0) return open64(pathname, flags, mode);
        dummy = write(tmp, emu->context->fullpath, strlen(emu->context->fullpath)+1);
        for (int i=1; i<emu->context->argc; ++i)
            dummy = write(tmp, emu->context->argv[i], strlen(emu->context->argv[i])+1);
        lseek64(tmp, 0, SEEK_SET);
        #else
        int tmp = shm_open(TMP_CMDLINE, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return open64(pathname, flags, mode);
        shm_unlink(TMP_CMDLINE);    // remove the shm file, but it will still exist because it's currently in use
        int dummy = write(tmp, emu->context->fullpath, strlen(emu->context->fullpath)+1);
        (void)dummy;
        for (int i=1; i<emu->context->argc; ++i)
            dummy = write(tmp, emu->context->argv[i], strlen(emu->context->argv[i])+1);
        lseek(tmp, 0, SEEK_SET);
        #endif
        return tmp;
    }
    return open64(pathname, flags, mode);
}
#define TMP_MEMMAP "box86_tmpmemmap"
EXPORT FILE* my_fopen(x86emu_t* emu, const char* path, const char* mode)
{
    if(strcmp((const char*)path, "/proc/self/maps")==0) {
        // special case for self memory map
        int tmp = shm_open(TMP_MEMMAP, O_RDWR | O_CREAT, S_IRWXU);
        if(tmp<0) return fopen(path, mode); // error fallback
        shm_unlink(TMP_MEMMAP);    // remove the shm file, but it will still exist because it's currently in use
        CreateMemorymapFile(emu->context, tmp);
        lseek(tmp, 0, SEEK_SET);
        return fdopen(tmp, mode);
    }
    return fopen(path, mode);
}


EXPORT int my_mkstemps64(x86emu_t* emu, char* template, int suffixlen)
{
    library_t* lib = GetLib(emu->context->maplib, libcName);
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "mkstemps64");
    if(f)
        return ((iFpi_t)f)(template, suffixlen);
    // implement own version...
    // TODO: check size of template, and if really XXXXXX is there
    char* fname = strdup(template);
    do {
        strcpy(fname, template);
        char num[8];
        sprintf(num, "%06d", rand()%999999);
        memcpy(fname+strlen(fname)-suffixlen-6, num, 6);
    } while(!FileExist(fname, -1));
    int ret = open64(fname, O_EXCL);
    free(fname);
    return ret;
}

#undef ftw
#undef nftw
EXPORT int32_t my_ftw(x86emu_t* emu, void* pathname, void* B, int32_t nopenfd)
{
    return ftw(pathname, findftwFct(B), nopenfd);
}

EXPORT int32_t my_nftw(x86emu_t* emu, void* pathname, void* B, int32_t nopenfd, int32_t flags)
{
    return nftw(pathname, findnftwFct(B), nopenfd, flags);
}

EXPORT void* my_ldiv(x86emu_t* emu, void* p, int32_t num, int32_t den)
{
    *((ldiv_t*)p) = ldiv(num, den);
    return p;
}

#ifndef NOALIGN
EXPORT int32_t my_epoll_ctl(x86emu_t* emu, int32_t epfd, int32_t op, int32_t fd, void* event)
{
    struct epoll_event _event[1] = {0};
    if(event && (op!=EPOLL_CTL_DEL))
        AlignEpollEvent(_event, event, 1);
    return epoll_ctl(epfd, op, fd, _event);
}
EXPORT int32_t my_epoll_wait(x86emu_t* emu, int32_t epfd, void* events, int32_t maxevents, int32_t timeout)
{
    struct epoll_event _events[maxevents];
    int32_t ret = epoll_wait(epfd, _events, maxevents, timeout);
    if(ret>0)
        UnalignEpollEvent(events, _events, ret);
    return ret;
}
#endif

#undef glob
EXPORT int32_t my_glob(x86emu_t *emu, void* pat, int32_t flags, void* errfnc, void* pglob)
{
    return glob(pat, flags, findgloberrFct(errfnc), pglob);
}

EXPORT int32_t my_glob64(x86emu_t *emu, void* pat, int32_t flags, void* errfnc, void* pglob)
{
    return glob64(pat, flags, findgloberrFct(errfnc), pglob);
}

EXPORT int my_scandir64(x86emu_t *emu, void* dir, void* namelist, void* sel, void* comp)
{
    return scandir64(dir, namelist, findfilter64Fct(sel), findcompare64Fct(comp));
}

#undef scandir
EXPORT int my_scandir(x86emu_t *emu, void* dir, void* namelist, void* sel, void* comp)
{
    return scandir(dir, namelist, findfilter_dirFct(sel), findcompare_dirFct(comp));
}

EXPORT int my_ftw64(x86emu_t* emu, void* filename, void* func, int descriptors)
{
    return ftw64(filename, findftw64Fct(func), descriptors);
}

EXPORT int32_t my_nftw64(x86emu_t* emu, void* pathname, void* B, int32_t nopenfd, int32_t flags)
{
    return nftw64(pathname, findnftw64Fct(B), nopenfd, flags);
}

EXPORT int32_t my_execv(x86emu_t* emu, const char* path, char* const argv[])
{
    int x86 = FileIsX86ELF(path);
    printf_log(LOG_DEBUG, "execv(\"%s\", %p), IsX86=%d\n", path, argv, x86);
    if (x86) {
        // count argv...
        int i=0;
        while(argv[i]) ++i;
        char** newargv = (char**)calloc(i+2, sizeof(char*));
        newargv[0] = emu->context->box86path;
        for (int j=0; j<i; ++j)
            newargv[j+1] = argv[j];
        printf_log(LOG_DEBUG, " => execv(\"%s\", %p [\"%s\", \"%s\"...:%d])\n", newargv[0], newargv, newargv[1], i?newargv[2]:"", i);
        int ret = execv(newargv[0], newargv);
        free(newargv);
        return ret;
    }
    return execv(path, argv);
}

EXPORT int32_t my_execvp(x86emu_t* emu, const char* path, char* const argv[])
{
    // need to use BOX86_PATH / PATH here...
    int x86 = FileIsX86ELF(path);
    printf_log(LOG_DEBUG, "execvp(\"%s\", %p), IsX86=%d\n", path, argv, x86);
    if (x86) {
        // count argv...
        int i=0;
        while(argv[i]) ++i;
        char** newargv = (char**)calloc(i+2, sizeof(char*));
        newargv[0] = emu->context->box86path;
        for (int j=0; j<i; ++j)
            newargv[j+1] = argv[j];
        printf_log(LOG_DEBUG, " => execvp(\"%s\", %p [\"%s\", \"%s\"...:%d])\n", newargv[0], newargv, newargv[1], i?newargv[2]:"", i);
        int ret = execvp(newargv[0], argv);
        free(newargv);
        return ret;
    }
    return execvp(path, argv);
}

EXPORT void my__Jv_RegisterClasses() {}

EXPORT int32_t my___cxa_thread_atexit_impl(x86emu_t* emu, void* dtor, void* obj, void* dso)
{
    printf_log(LOG_INFO, "Warning, call to __cxa_thread_atexit_impl(%p, %p, %p) ignored\n", dtor, obj, dso);
    return 0;
}

extern void __chk_fail();
EXPORT unsigned long int my___fdelt_chk (unsigned long int d)
{
  if (d >= FD_SETSIZE)
    __chk_fail ();

  return d / __NFDBITS;
}

EXPORT int32_t my_getrandom(x86emu_t* emu, void* buf, uint32_t buflen, uint32_t flags)
{
    // not always implemented on old linux version...
    library_t* lib = GetLib(emu->context->maplib, libcName);
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "getrandom");
    if(f)
        return ((iFpuu_t)f)(buf, buflen, flags);
    // do what should not be done, but it's better then nothing....
    FILE * rnd = fopen("/dev/random", "rb");
    uint32_t r = fread(buf, 1, buflen, rnd);
    fclose(rnd);
    return r;
}

EXPORT int32_t my___sendmmsg(x86emu_t* emu, int32_t fd, void* msgvec, uint32_t vlen, uint32_t flags)
{
    // Implemented starting glibc 2.14+
    library_t* lib = GetLib(emu->context->maplib, libcName);
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "__sendmmsg");
    if(f)
        return ((iFipuu_t)f)(fd, msgvec, vlen, flags);
    // Use the syscall
    return syscall(__NR_sendmmsg, fd, msgvec, vlen, flags);
}

EXPORT int32_t my___register_atfork(x86emu_t *emu, void* prepare, void* parent, void* child)
{
    // this is partly incorrect, because the emulated funcionts should be executed by actual fork and not by my_atfork...
    if(emu->context->atfork_sz==emu->context->atfork_cap) {
        emu->context->atfork_cap += 4;
        emu->context->atforks = (atfork_fnc_t*)realloc(emu->context->atforks, emu->context->atfork_cap*sizeof(atfork_fnc_t));
    }
    emu->context->atforks[emu->context->atfork_sz].prepare = (uintptr_t)prepare;
    emu->context->atforks[emu->context->atfork_sz].parent = (uintptr_t)parent;
    emu->context->atforks[emu->context->atfork_sz++].child = (uintptr_t)child;
    return 0;
}

EXPORT uint64_t my___umoddi3(uint64_t a, uint64_t b)
{
    return a%b;
}
EXPORT uint64_t my___udivdi3(uint64_t a, uint64_t b)
{
    return a/b;
}
EXPORT int64_t my___divdi3(int64_t a, int64_t b)
{
    return a/b;
}

EXPORT int32_t my___poll_chk(void* a, uint32_t b, int c, int l)
{
    return poll(a, b, c);   // no check...
}

int of_convert(int flag);
EXPORT int32_t my_fcntl64(x86emu_t* emu, int32_t a, int32_t b, uint32_t d1, uint32_t d2, uint32_t d3, uint32_t d4, uint32_t d5, uint32_t d6)
{
    // Implemented starting glibc 2.14+
    library_t* lib = GetLib(emu->context->maplib, libcName);
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "fcntl64");
    if(b==F_SETFL)
        d1 = of_convert(d1);
    if(f)
        return ((iFiiuuuuuu_t)f)(a, b, d1, d2, d3, d4, d5, d6);
    //TODO: check if better to use the syscall or regular fcntl?
    //return syscall(__NR_fcntl64, a, b, d1, d2, d3, d4);   // should be enough
    return fcntl(a, b, d1, d2, d3, d4, d5, d6);
}

EXPORT int32_t my_fcntl(x86emu_t* emu, int32_t a, int32_t b, uint32_t d1, uint32_t d2, uint32_t d3, uint32_t d4, uint32_t d5, uint32_t d6)
{
    if(b==F_SETFL && d1==0xFFFFF7FF) {
        // special case for ~O_NONBLOCK...
        int flags = fcntl(a, F_GETFL);
        if(flags&O_NONBLOCK) {
            flags &= ~O_NONBLOCK;
            return fcntl(a, b, flags);
        }
        return 0;
    }
    if(b==F_SETFL)
        d1 = of_convert(d1);
    return fcntl(a, b, d1, d2, d3, d4, d5, d6);
}
EXPORT int32_t my___fcntl(x86emu_t* emu, int32_t a, int32_t b, uint32_t d1, uint32_t d2, uint32_t d3, uint32_t d4, uint32_t d5, uint32_t d6) __attribute__((alias("my_fcntl")));

EXPORT int32_t my_preadv64(x86emu_t* emu, int32_t fd, void* v, int32_t c, int64_t o)
{
    library_t* lib = GetLib(emu->context->maplib, libcName);
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "preadv64");
    if(f)
        return ((iFipiI_t)f)(fd, v, c, o);
    return syscall(__NR_preadv, fd, v, c,(uint32_t)(o&0xffffffff), (uint32_t)((o>>32)&0xffffffff));
}

EXPORT int32_t my_pwritev64(x86emu_t* emu, int32_t fd, void* v, int32_t c, int64_t o)
{
    library_t* lib = GetLib(emu->context->maplib, libcName);
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "pwritev64");
    if(f)
        return ((iFipiI_t)f)(fd, v, c, o);
    return syscall(__NR_pwritev, fd, v, c,(uint32_t)(o&0xffffffff), (uint32_t)((o>>32)&0xffffffff));
}

EXPORT int32_t my_accept4(x86emu_t* emu, int32_t fd, void* a, void* l, int32_t flags)
{
    library_t* lib = GetLib(emu->context->maplib, libcName);
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "accept4");
    if(f)
        return ((iFippi_t)f)(fd, a, l, flags);
    if(!flags)
        return accept(fd, a, l);
    return syscall(__NR_accept4, fd, a, l, flags);
}

EXPORT struct __processor_model
{
  unsigned int __cpu_vendor;
  unsigned int __cpu_type;
  unsigned int __cpu_subtype;
  unsigned int __cpu_features[1];
} my___cpu_model;

#include "cpu_info.h"
void InitCpuModel()
{
    // some pseudo random cpu info...
    my___cpu_model.__cpu_vendor = VENDOR_INTEL;
    my___cpu_model.__cpu_type = INTEL_PENTIUM_M;
    my___cpu_model.__cpu_subtype = 0; // N/A
    my___cpu_model.__cpu_features[0] = (1<<FEATURE_CMOV) 
                                     | (1<<FEATURE_MMX) 
                                     | (1<<FEATURE_SSE) 
                                     | (1<<FEATURE_SSE2) 
                                     | (1<<FEATURE_SSE3)
                                     | (1<<FEATURE_SSSE3)
                                     | (1<<FEATURE_MOVBE)
                                     | (1<<FEATURE_ADX);
}

EXPORT const unsigned short int *my___ctype_b;
EXPORT const int32_t *my___ctype_tolower;
EXPORT const int32_t *my___ctype_toupper;

void ctSetup()
{
    my___ctype_b = *(__ctype_b_loc());
    my___ctype_toupper = *(__ctype_toupper_loc());
    my___ctype_tolower = *(__ctype_tolower_loc());
}

EXPORT void* my___libc_stack_end;
void stSetup(box86context_t* context)
{
    my___libc_stack_end = context->stack;   // is this the end, or should I add stasz?
}

EXPORT void my___register_frame_info(void* a, void* b)
{
    // nothing
}
EXPORT void* my___deregister_frame_info(void* a)
{
    return NULL;
}

EXPORT void* my____brk_addr = NULL;

// longjmp / setjmp
typedef struct jump_buff_i386_s {
 uint32_t save_ebx;
 uint32_t save_esi;
 uint32_t save_edi;
 uint32_t save_ebp;
 uint32_t save_esp;
 uint32_t save_eip;
} jump_buff_i386_t;

typedef struct __jmp_buf_tag_s {
    jump_buff_i386_t __jmpbuf;
    int              __mask_was_saved;
    __sigset_t       __saved_mask;
} __jmp_buf_tag_t;

void EXPORT my_longjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val)
{
    jump_buff_i386_t *jpbuff = &((__jmp_buf_tag_t*)p)->__jmpbuf;
    //restore  regs
    R_EBX = jpbuff->save_ebx;
    R_ESI = jpbuff->save_esi;
    R_EDI = jpbuff->save_edi;
    R_EBP = jpbuff->save_ebp;
    R_ESP = jpbuff->save_esp;
    // jmp to saved location, plus restore val to eax
    R_EAX = __val;
    R_EIP = jpbuff->save_eip;
    if(emu->quitonlongjmp) {
        emu->longjmp = 1;
        emu->quit = 1;
    }
}

EXPORT int32_t my_setjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p)
{
    jump_buff_i386_t *jpbuff = &((__jmp_buf_tag_t*)p)->__jmpbuf;
    // save the buffer
    jpbuff->save_ebx = R_EBX;
    jpbuff->save_esi = R_ESI;
    jpbuff->save_edi = R_EDI;
    jpbuff->save_ebp = R_EBP;
    jpbuff->save_esp = R_ESP+4; // include "return address"
    jpbuff->save_eip = *(uint32_t*)(R_ESP);
    // and that's it.. Nothing more for now
    return 0;
}

EXPORT void my___explicit_bzero_chk(x86emu_t* emu, void* dst, uint32_t len, uint32_t dstlen)
{
    memset(dst, 0, len);
}

EXPORT void* my_realpath(x86emu_t* emu, void* path, void* resolved_path)
{
    char* ret;
    if(strcmp(path, "/proc/self/exe")==0) {
        ret = realpath(emu->context->fullpath, resolved_path);
    } else  if(strncmp((const char*)path, "/proc/", 6)==0) {
        // check if self checking....
        pid_t pid = getpid();
        char tmp[64];
        sprintf(tmp, "/proc/%d/exe", pid);
        if(strcmp((const char*)path, tmp)==0)
            ret = realpath(emu->context->fullpath, resolved_path);
        else
            ret = realpath(path, resolved_path);
    } else
        ret = realpath(path, resolved_path);
    return ret;
}

EXPORT void* my_mmap(x86emu_t* emu, void *addr, unsigned long length, int prot, int flags, int fd, int offset)
{
    dynarec_log(LOG_DEBUG, "mmap(%p, %lu, 0x%x, 0x%x, %d, %d) =>", addr, length, prot, flags, fd, offset);
    void* ret = mmap(addr, length, prot, flags, fd, offset);
    dynarec_log(LOG_DEBUG, "%p\n", ret);
    #ifdef DYNAREC
    if(prot& PROT_EXEC)
        addDBFromAddressRange((uintptr_t)ret, length);
    else
        cleanDBFromAddressRange((uintptr_t)ret, length);
    #endif
    return ret;
}

EXPORT void* my_mmap64(x86emu_t* emu, void *addr, unsigned long length, int prot, int flags, int fd, int64_t offset)
{
    dynarec_log(LOG_DEBUG, "mmap64(%p, %lu, 0x%x, 0x%x, %d, %lld) =>", addr, length, prot, flags, fd, offset);
    void* ret = mmap64(addr, length, prot, flags, fd, offset);
    dynarec_log(LOG_DEBUG, "%p\n", ret);
    #ifdef DYNAREC
    if(prot& PROT_EXEC)
        addDBFromAddressRange((uintptr_t)ret, length);
    else
        cleanDBFromAddressRange((uintptr_t)ret, length);
    #endif
    return ret;
}

EXPORT int my_munmap(x86emu_t* emu, void* addr, unsigned long length)
{
    dynarec_log(LOG_DEBUG, "munmap(%p, %lu)\n", addr, length);
    #ifdef DYNAREC
    cleanDBFromAddressRange((uintptr_t)addr, length);
    #endif
    return munmap(addr, length);
}

EXPORT int my_mprotect(x86emu_t* emu, void *addr, unsigned long len, int prot)
{
    dynarec_log(LOG_DEBUG, "mprotect(%p, %lu, 0x%x)\n", addr, len, prot);
    #ifdef DYNAREC
    if(prot& PROT_EXEC)
        addDBFromAddressRange((uintptr_t)addr, len);
    else
        cleanDBFromAddressRange((uintptr_t)addr, len);
    #endif
    return mprotect(addr, len, prot);
}

static ssize_t my_cookie_read(void *cookie, char *buf, size_t size)
{
    x86emu_t *emu = (x86emu_t*)cookie;
    SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 3));
    SetCallbackArg(emu, 1, buf);
    SetCallbackArg(emu, 2, (void*)size);
    SetCallbackNArg(emu, 3);
    return RunCallback(emu);
}
static ssize_t my_cookie_write(void *cookie, const char *buf, size_t size)
{
    x86emu_t *emu = (x86emu_t*)cookie;
    SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 4));
    SetCallbackArg(emu, 1, (void*)buf);
    SetCallbackArg(emu, 2, (void*)size);
    SetCallbackNArg(emu, 3);
    return RunCallback(emu);
}
static int my_cookie_seek(void *cookie, off64_t *offset, int whence)
{
    x86emu_t *emu = (x86emu_t*)cookie;
    SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 5));
    SetCallbackArg(emu, 1, offset);
    SetCallbackArg(emu, 2, (void*)whence);
    SetCallbackNArg(emu, 3);
    return RunCallback(emu);

}
static int my_cookie_close(void *cookie)
{
    x86emu_t *emu = (x86emu_t*)cookie;
    void* cl = (void*)GetCallbackArg(emu, 6);
    int ret = 0;
    if(cl) {
        SetCallbackAddress(emu, (uintptr_t)cl);
        SetCallbackNArg(emu, 1);
        ret = (int)RunCallback(emu);
    }
    // free anyway, even if ret is not null?
    FreeCallback(emu);
    return ret;
}
EXPORT void* my_fopencookie(x86emu_t* emu, void* cookie, void* mode, void* read, void* write, void* seek, void* close)
{
    cookie_io_functions_t io_funcs = {read?my_cookie_read:NULL, write?my_cookie_write:NULL, seek?my_cookie_seek:NULL, my_cookie_close};
    x86emu_t *cb = AddCallback(emu, 0, 3, cookie, NULL, NULL, NULL);
    SetCallbackArg(emu, 3, read);
    SetCallbackArg(emu, 4, write);
    SetCallbackArg(emu, 5, seek);
    SetCallbackArg(emu, 6, close);
    return fopencookie(cb, mode, io_funcs);
}

EXPORT long my_prlimit64(void* pid, uint32_t res, void* new_rlim, void* old_rlim)
{
    return syscall(__NR_prlimit64, pid, res, new_rlim, old_rlim);
}

EXPORT void* my_reallocarray(void* ptr, size_t nmemb, size_t size)
{
    return realloc(ptr, nmemb*size);
}

#ifndef __OPEN_NEEDS_MODE
# define __OPEN_NEEDS_MODE(oflag) \
  (((oflag) & O_CREAT) != 0)
// || ((oflag) & __O_TMPFILE) == __O_TMPFILE)
#endif
EXPORT int my___open_nocancel(x86emu_t* emu, void* file, int oflag, int* b)
{
    int mode = 0;
    if (__OPEN_NEEDS_MODE (oflag))
        mode = b[0];
    return openat(AT_FDCWD, file, oflag, mode);
}

EXPORT int my___libc_alloca_cutoff(x86emu_t* emu, size_t size)
{
    // not always implemented on old linux version...
    library_t* lib = GetLib(emu->context->maplib, libcName);
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "__libc_alloca_cutoff");
    if(f)
        return ((iFL_t)f)(size);
    // approximate version but it's better than nothing....
    return (size<=(65536*4));
}

#ifndef NOALIGN
// wrapped malloc using calloc, it seems x86 malloc set alloc'd block to zero somehow
EXPORT void* my_malloc(unsigned long size)
{
    return calloc(1, size);
}
#endif

#define CUSTOM_INIT         \
    InitCpuModel();         \
    ctSetup();              \
    stSetup(box86);         \
    box86->libclib = lib;   \
    lib->priv.w.p2 = getLIBCMy(lib); \
    lib->priv.w.needed = 3; \
    lib->priv.w.neededlibs = (char**)calloc(lib->priv.w.needed, sizeof(char*)); \
    lib->priv.w.neededlibs[0] = strdup("ld-linux.so.2"); \
    lib->priv.w.neededlibs[1] = strdup("libpthread.so.0"); \
    lib->priv.w.neededlibs[2] = strdup("librt.so.1");

#define CUSTOM_FINI \
    freeLIBCMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"
