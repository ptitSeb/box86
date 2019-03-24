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
#include <sys/epoll.h>

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
#include "signals.h"


#define LIBNAME libc
const char* libcName = "libc.so.6";

typedef void (*vFipp_t)(int32_t, void*, void*);
typedef int32_t (*iFpup_t)(void*, uint32_t, void*);
typedef int32_t (*iFpuu_t)(void*, uint32_t, uint32_t);

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
// All signal functions defined in signals.c

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

EXPORT void *my_div(void *result, int numerator, int denominator) {
    *(div_t *)result = div(numerator, denominator);
    return result;
}

EXPORT int my_dl_iterate_phdr(x86emu_t *emu, void* F, void *data) {
    printf_log(LOG_NONE, "Error: unimplemented dl_iterate_phdr(%p, %p) used\n", F, data);
    emu->quit = 1;
    return 0;
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

EXPORT int my_vsprintf(x86emu_t* emu, void* buff,  void * fmt, void * b, va_list V) {
    #ifndef NOALIGN
    // need to align on arm
    myStackAlign((const char*)fmt, *(uint32_t**)b, emu->scratch);
    void* f = vsprintf;
    int r = ((iFppp_t)f)(buff, fmt, emu->scratch);
    return r;
    #else
    return vsprintf(buff, fmt, V);
    #endif
}
EXPORT int my___vsprintf_chk(x86emu_t* emu, void* buff, void * fmt, void * b, va_list V) __attribute__((alias("my_vsprintf")));

EXPORT int my_vfscanf(x86emu_t* emu, void* stream, void* fmt, void* b) // probably uneeded to do a GOM, a simple wrap should enough
{
    void* f = vfscanf;
    return ((iFppp_t)f)(stream, fmt, *(uint32_t**)b);
}

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

EXPORT int my___swprintf_chk(x86emu_t* emu, void* s, uint32_t n, int32_t flag, uint32_t slen, void* fmt, void * b)
{
    #ifndef NOALIGN
    myStackAlignW((const char*)fmt, b, emu->scratch);
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(s, n, fmt, emu->scratch);
    #else
    void* f = vswprintf;
    ((iFpupp_t)f)(s, n, fmt, b);
    #endif
}
EXPORT int my_swprintf(x86emu_t* emu, void* s, uint32_t n, void* fmt, void *b)
{
    #ifndef NOALIGN
    myStackAlignW((const char*)fmt, b, emu->scratch);
    void* f = vswprintf;
    int r = ((iFpupp_t)f)(s, n, fmt, emu->scratch);
    #else
    void* f = vswprintf;
    ((iFpupp_t)f)(s, n, fmt, b);
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

static int qsort_cmp(const void* a, const void* b, void* e)
{
    x86emu_t* emu = (x86emu_t*)e;
    SetCallbackArg(emu, 0, (void*)a);
    SetCallbackArg(emu, 1, (void*)b);
    return (int)RunCallback(emu);
}

EXPORT void my_qsort(x86emu_t* emu, void* base, size_t nmemb, size_t size, void* fnc)
{
    // use a temporary callback
    x86emu_t *cbemu = AddSharedCallback(emu, (uintptr_t)fnc, 2, NULL, NULL, NULL, NULL);
    qsort_r(base, nmemb, size, qsort_cmp, cbemu);
    FreeCallback(cbemu);
}
EXPORT void my_qsort_r(x86emu_t* emu, void* base, size_t nmemb, size_t size, void* fnc, void* arg)
{
    // use a temporary callback
    x86emu_t *cbemu = AddSharedCallback(emu, (uintptr_t)fnc, 3, NULL, NULL, arg, NULL);
    qsort_r(base, nmemb, size, qsort_cmp, cbemu);
    FreeCallback(cbemu);
}

static x86emu_t *bsearch_emu = NULL;
static int bsearch_cmp(const void* a, const void* b)
{
    SetCallbackArg(bsearch_emu, 0, (void*)a);
    SetCallbackArg(bsearch_emu, 1, (void*)b);
    return (int)RunCallback(bsearch_emu);
}

EXPORT void* my_bsearch(x86emu_t* emu, void* key, void* base, size_t nmemb, size_t size, void* fnc)
{
    // use a temporary callback, but global because there is no bsearch_r...
    bsearch_emu = AddSharedCallback(emu, (uintptr_t)fnc, 2, NULL, NULL, NULL, NULL);
    void* ret = bsearch(key, base, nmemb, size, bsearch_cmp);
    bsearch_emu = FreeCallback(bsearch_emu);
    return ret;
}

EXPORT void* my_lsearch(x86emu_t* emu, void* key, void* base, size_t* nmemb, size_t size, void* fnc)
{
    // use a temporary callback, but global because there is no bsearch_r...
    bsearch_emu = AddSharedCallback(emu, (uintptr_t)fnc, 2, NULL, NULL, NULL, NULL);
    void* ret = lsearch(key, base, nmemb, size, bsearch_cmp);
    bsearch_emu = FreeCallback(bsearch_emu);
    return ret;
}
EXPORT void* my_lfind(x86emu_t* emu, void* key, void* base, size_t* nmemb, size_t size, void* fnc)
{
    // use a temporary callback, but global because there is no bsearch_r...
    bsearch_emu = AddSharedCallback(emu, (uintptr_t)fnc, 2, NULL, NULL, NULL, NULL);
    void* ret = lfind(key, base, nmemb, size, bsearch_cmp);
    bsearch_emu = FreeCallback(bsearch_emu);
    return ret;
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

EXPORT int32_t my_open(x86emu_t* emu, void* pathname, int32_t flags, uint32_t mode)
{
    if(strcmp((const char*)pathname, "/proc/self/cmdline")==0) {
        // special case for self command line...
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
        return tmp;
    }
    return open(pathname, flags, mode);
}
EXPORT int32_t my___open(x86emu_t* emu, void* pathname, int32_t flags, uint32_t mode) __attribute__((alias("my_open")));

EXPORT int32_t my_open64(x86emu_t* emu, void* pathname, int32_t flags, uint32_t mode)
{
    if(strcmp((const char*)pathname, "/proc/self/cmdline")==0) {
        // special case for self command line...
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
        return tmp;
    }
    return open64(pathname, flags, mode);
}

EXPORT void* my_ldiv(x86emu_t* emu, void* p, int32_t num, int32_t den)
{
    *((ldiv_t*)p) = ldiv(num, den);
    return p;
}

#ifndef NOALIGN
EXPORT int32_t my_epoll_ctl(x86emu_t* emu, int32_t epfd, int32_t op, int32_t fd, void* event)
{
    struct epoll_event _event[1];
    if(event && op!=EPOLL_CTL_DEL)
        AlignEpollEvent(_event, event, 1);
    return epoll_ctl(epfd, op, fd, (event && op!=EPOLL_CTL_DEL)?_event:NULL);
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

x86emu_t *globemu = NULL;   // issue with multi threads...
static int glob_errfnccallback(const char* epath, int no)
{
    if(globemu) {
        SetCallbackArg(globemu, 0, (void*)epath);
        SetCallbackArg(globemu, 1, (void*)no);
        return (int32_t)RunCallback(globemu);
    }
    return 0;
}
EXPORT int32_t my_glob(x86emu_t *emu, void* pat, int32_t flags, void* errfnc, void* pblog)
{
    if(errfnc)
        globemu = AddSharedCallback(emu, (uintptr_t)errfnc, 2, NULL, NULL, NULL, NULL);
    int32_t r = glob((const char*)pat, flags, globemu?glob_errfnccallback:NULL, (glob_t*)pblog);
    if(globemu)
        globemu = FreeCallback(globemu);
    return r;
}

x86emu_t *scandir64emu1 = NULL;
static int scandir64_selcb1(const struct dirent64* dir)
{
    if(scandir64emu1) {
        SetCallbackNArg(scandir64emu1, 1);
        SetCallbackAddress(scandir64emu1, (uintptr_t)GetCallbackArg(scandir64emu1, 3));
        SetCallbackArg(scandir64emu1, 0, (void*)dir);
        return (int32_t)RunCallback(scandir64emu1);
    }
    return 0;
}
#ifdef PANDORA
static int scandir64_compcb1(const void* a, const void* b)
#else
static int scandir64_compcb1(const struct dirent64** a, const struct dirent64** b)
#endif
{
    if(scandir64emu1) {
        SetCallbackNArg(scandir64emu1, 2);
        SetCallbackAddress(scandir64emu1, (uintptr_t)GetCallbackArg(scandir64emu1, 4));
        SetCallbackArg(scandir64emu1, 0, (void*)a);
        SetCallbackArg(scandir64emu1, 1, (void*)b);
        return (int32_t)RunCallback(scandir64emu1);
    }
    return 0;
}
x86emu_t *scandir64emu2 = NULL;
static int scandir64_selcb2(const struct dirent64* dir)
{
    if(scandir64emu2) {
        SetCallbackNArg(scandir64emu2, 1);
        SetCallbackAddress(scandir64emu2, (uintptr_t)GetCallbackArg(scandir64emu2, 3));
        SetCallbackArg(scandir64emu2, 0, (void*)dir);
        return (int32_t)RunCallback(scandir64emu2);
    }
    return 0;
}
#ifdef PANDORA
static int scandir64_compcb2(const void* a, const void* b)
#else
static int scandir64_compcb2(const struct dirent64** a, const struct dirent64** b)
#endif
{
    if(scandir64emu2) {
        SetCallbackNArg(scandir64emu2, 2);
        SetCallbackAddress(scandir64emu2, (uintptr_t)GetCallbackArg(scandir64emu2, 4));
        SetCallbackArg(scandir64emu2, 0, (void*)a);
        SetCallbackArg(scandir64emu2, 1, (void*)b);
        return (int32_t)RunCallback(scandir64emu2);
    }
    return 0;
}

EXPORT int my_scandir64(x86emu_t *emu, void* dir, void* namelist, void* sel, void* comp)
{
    int ret = 0;
    // cleanup first
    if(scandir64emu1 && !IsCallback(emu, scandir64emu1))
        scandir64emu1 = NULL;
    if(scandir64emu2 && !IsCallback(emu, scandir64emu2))
        scandir64emu2 = NULL;
    
    if(scandir64emu1 && (scandir64emu1!=emu)) {
        // in case there are 2 concurent call!
        if(scandir64emu2 && (scandir64emu2!=emu)) {
                printf_log(LOG_NONE, "Warning, more than 2 concurent call to scandir64\n");
        }
        scandir64emu2 = AddSharedCallback(emu, (uintptr_t)sel, 1, NULL, NULL, NULL, NULL);
        SetCallbackArg(scandir64emu2, 3, sel);
        SetCallbackArg(scandir64emu2, 4, comp);
        SetCallbackArg(scandir64emu2, 5, my_scandir64);
        ret = scandir64(dir, namelist, scandir64_selcb2, scandir64_compcb2);
        scandir64emu2 = FreeCallback(scandir64emu2);
        if(scandir64emu2 && GetCallbackArg(scandir64emu2, 5)!=my_scandir64)
            scandir64emu2 = NULL;
    } else {
        scandir64emu1 = AddSharedCallback(emu, (uintptr_t)sel, 1, NULL, NULL, NULL, NULL);
        SetCallbackArg(scandir64emu1, 3, sel);
        SetCallbackArg(scandir64emu1, 4, comp);
        SetCallbackArg(scandir64emu1, 5, my_scandir64);
        ret = scandir64(dir, namelist, scandir64_selcb1, scandir64_compcb1);
        scandir64emu1 = FreeCallback(scandir64emu1);    // this chain stuff is in case a scandir64 is called inside one of the 2 callback
        if(scandir64emu1 && GetCallbackArg(scandir64emu1, 5)!=my_scandir64)
                scandir64emu1 = NULL;
    }
    return ret;
}

x86emu_t *scandiremu = NULL;
static int scandir_selcb(const struct dirent* dir)
{
    if(scandiremu) {
        SetCallbackNArg(scandiremu, 1);
        SetCallbackAddress(scandiremu, (uintptr_t)GetCallbackArg(scandiremu, 3));
        SetCallbackArg(scandiremu, 0, (void*)dir);
        return (int32_t)RunCallback(scandiremu);
    }
    return 0;
}
#ifdef PANDORA
static int scandir_compcb(const void* a, const void* b)
#else
static int scandir_compcb(const struct dirent** a, const struct dirent** b)
#endif
{
    if(scandiremu) {
        SetCallbackNArg(scandiremu, 1);
        SetCallbackAddress(scandiremu, (uintptr_t)GetCallbackArg(scandiremu, 3));
        SetCallbackArg(scandiremu, 0, (void*)a);
        SetCallbackArg(scandiremu, 1, (void*)b);
        return (int32_t)RunCallback(scandiremu);
    }
    return 0;
}
EXPORT int my_scandir(x86emu_t *emu, void* dir, void* namelist, void* sel, void* comp)
{
    scandiremu = AddSharedCallback(emu, (uintptr_t)sel, 1, NULL, NULL, NULL, NULL);
    SetCallbackArg(scandiremu, 3, sel);
    SetCallbackArg(scandiremu, 4, comp);
    int ret = scandir(dir, namelist, scandir_selcb, scandir_compcb);
    scandiremu = FreeCallback(scandiremu);
    return ret;
}

EXPORT int32_t my_execvp(x86emu_t* emu, void* a, void* b, va_list v)
{
    return execvp(a, b);
}
EXPORT int32_t my_execlp(x86emu_t* emu, void* a, void* b, va_list v) __attribute__((alias("my_execvp")));

EXPORT int32_t my_execl(x86emu_t* emu, void* a, void* b, void* c, va_list v)
{
    int n=1;
    if(b) {
        ++n;
        void** cnt = (void**)c;
        while(cnt[n]) ++n;
    }
    void** params = (void**)calloc(n, sizeof(void*));
    params[0] = b;
    memcpy(params+4, c, n*sizeof(void*));
    int32_t r = execv(a, (char* const*)params);
    free(params);
    return r;
}

EXPORT int32_t my_execle(x86emu_t* emu, void* a, void* b, void* c, va_list v)
{
    int n=1;
    if(b) {
        ++n;
        void** cnt = (void**)c;
        while(cnt[n]) ++n;
    }
    void** params = (void**)calloc(n, sizeof(void*));
    params[0] = b;
    memcpy(params+4, c, n*sizeof(void*));
    int32_t r = execve(a, (char* const*)params, *((void**)c+(n+1)));
    free(params);
    return r;
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
    jpbuff->save_esp = R_ESP+4; // include "return address"
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

EXPORT void my_getcontext(x86emu_t* emu, void* ucp)
{
    printf_log(LOG_NONE, "Warning: call to unimplemented getcontext\n");
}

EXPORT void my_makecontext(x86emu_t* emu, void* ucp, void* fnc, int32_t argc, void* argv)
{
    printf_log(LOG_NONE, "Warning: call to unimplemented makecontext\n");
}


#define CUSTOM_INIT \
    InitCpuModel(); \
    ctSetup(); \
    stSetup(box86); \
    box86->libclib = lib; \
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
