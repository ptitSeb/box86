#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x86emu.h"
#include "emu/x86emu_private.h"
#include "box86context.h"
#include "librarian.h"

const char* libpthreadName = "libpthread.so.0";
#define LIBNAME libpthread


int my_pthread_create(x86emu_t *emu, void* t, void* attr, void* start_routine, void* arg); //implemented in thread.c
int my_pthread_key_create(x86emu_t* emu, void* key, void* dtor);
int my___pthread_key_create(x86emu_t* emu, void* key, void* dtor);
int my_pthread_once(x86emu_t* emu, void* once, void* cb);
int my___pthread_once(x86emu_t* emu, void* once, void* cb);
int my_pthread_cond_broadcast(x86emu_t* emu, void* cond);
int my_pthread_cond_destroy(x86emu_t* emu, void* cond);
int my_pthread_cond_init(x86emu_t* emu, void* cond, void* attr);
int my_pthread_cond_signal(x86emu_t* emu, void* cond);
int my_pthread_cond_timedwait(x86emu_t* emu, void* cond, void* mutex, void* abstime);
int my_pthread_cond_wait(x86emu_t* emu, void* cond, void* mutex);
int my_pthread_mutexattr_setkind_np(x86emu_t* emu, void* t, int kind);
int my_pthread_attr_setscope(x86emu_t* emu, void* attr, int scope);
void my__pthread_cleanup_push_defer(x86emu_t* emu, void* buffer, void* routine, void* arg);
void my__pthread_cleanup_pop_restore(x86emu_t* emu, void* buffer, int exec);
int my_pthread_kill(x86emu_t* emu, void* thread, int sig);

typedef int (*iFpp_t)(void*, void*);
typedef int (*iFppu_t)(void*, void*, uint32_t);
EXPORT int my_pthread_setname_np(x86emu_t* emu, void* t, void* n)
{
    library_t* lib = GetLib(emu->context->maplib, libpthreadName);
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "pthread_setname_np");
    if(f)
        return ((iFpp_t)f)(t, n);
    return 0;
}
EXPORT int my_pthread_getname_np(x86emu_t* emu, void* t, void* n, uint32_t s)
{
    library_t* lib = GetLib(emu->context->maplib, libpthreadName);
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "pthread_getname_np");
    if(f)
        return ((iFppu_t)f)(t, n, s);
    else 
        strncpy((char*)n, "dummy", s);
    return 0;
}

EXPORT int my_pthread_setaffinity_np(x86emu_t* emu, void* thread, int cpusetsize, void* cpuset)
{
    printf_log(LOG_INFO, "Warning, call to pthread_setaffinity_np(...) ignored\n");
    //the cpuset needs transformation, from i386 to current architecture
    // ignoring for now
    return 0;   // faking success
}

EXPORT int my_pthread_attr_setaffinity_np(x86emu_t* emu, void* attr, uint32_t cpusetsize, void* cpuset)
{
    printf_log(LOG_INFO, "Warning, call to pthread_attr_setaffinity_np(...) ignored\n");
    //the cpuset needs transformation, from i386 to current architecture
    // ignoring for now
    return 0;   // faking success
}

EXPORT int my_pthread_attr_setschedparam(x86emu_t* emu, void* attr, void* param)
{
    int policy;
    pthread_attr_getschedpolicy(attr, &policy);
    int pmin = sched_get_priority_min(policy);
    int pmax = sched_get_priority_max(policy);
    if(param) {
        int p = *(int*)param;
        if(p>=pmin && p<=pmax)
            return pthread_attr_setschedparam(attr, param);
    }
    printf_log(LOG_INFO, "Warning, call to pthread_attr_setschedparam(%p, %p[%d]) ignored\n", attr, param, param?(*(int*)param):-1);
    return 0;   // faking success
}

EXPORT int32_t my_pthread_atfork(x86emu_t *emu, void* prepare, void* parent, void* child)
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
EXPORT int32_t my___pthread_atfork(x86emu_t *emu, void* prepare, void* parent, void* child) __attribute__((alias("my_pthread_atfork")));

EXPORT void my___pthread_initialize()
{
    // nothing, the lib initialize itself now
}

#include "wrappedlib_init.h"
