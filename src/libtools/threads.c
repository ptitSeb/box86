// __USE_UNIX98 is needed for sttype / gettype definition
#define __USE_UNIX98
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <setjmp.h>
#include <sys/mman.h>
#include <dlfcn.h>

#include "debug.h"
#include "box86context.h"
#include "threads.h"
#include "emu/x86emu_private.h"
#include "tools/bridge_private.h"
#include "x86run.h"
#include "x86emu.h"
#include "box86stack.h"
#include "callback.h"
#include "custommem.h"
#include "khash.h"
#include "emu/x86run_private.h"
#include "x86trace.h"
#include "dynarec.h"
#include "bridge.h"
#include "myalign.h"
#ifdef DYNAREC
#include "dynablock.h"
#include "dynarec/arm_lock_helper.h"
#endif

typedef void (*vFppp_t)(void*, void*, void*);
typedef void (*vFpi_t)(void*, int);
typedef int (*iFli_t)(long unsigned int, int);
//starting with glibc 2.34+, those 2 functions are in libc.so as versionned symbol only
// So use dlsym to get the symbol unversionned, as simple link will not work.
static vFppp_t real_pthread_cleanup_push_defer = NULL;
static vFpi_t real_pthread_cleanup_pop_restore = NULL;
// with glibc 2.34+, pthread_kill changed behaviour and might break some program, so using old version if possible
// it will be pthread_kill@GLIBC_2.4 on arm, but it's GLIBC_2.0 on i386
static iFli_t real_phtread_kill_old = NULL;
// those function can be used simply
void _pthread_cleanup_push(void* buffer, void* routine, void* arg);	// declare hidden functions
void _pthread_cleanup_pop(void* buffer, int exec);

typedef struct threadstack_s {
	void* 	stack;
	size_t 	stacksize;
} threadstack_t;

typedef struct x86_unwind_buff_s {
	struct {
		jump_buff_i386_t	__cancel_jmp_buf;	
		int					__mask_was_saved;
	} __cancel_jmp_buf[1];
	void *__pad[4];
} x86_unwind_buff_t __attribute__((__aligned__));

typedef void(*vFv_t)();

KHASH_MAP_INIT_INT(threadstack, threadstack_t*)
#ifndef ANDROID
KHASH_MAP_INIT_INT(cancelthread, __pthread_unwind_buf_t*)
#endif

void CleanStackSize(box86context_t* context)
{
	threadstack_t *ts;
	if(!context || !context->stacksizes)
		return;
	mutex_lock(&context->mutex_thread);
	kh_foreach_value(context->stacksizes, ts, box_free(ts));
	kh_destroy(threadstack, context->stacksizes);
	context->stacksizes = NULL;
	mutex_unlock(&context->mutex_thread);
}

void FreeStackSize(kh_threadstack_t* map, uintptr_t attr)
{
	mutex_lock(&my_context->mutex_thread);
	khint_t k = kh_get(threadstack, map, attr);
	if(k!=kh_end(map)) {
		box_free(kh_value(map, k));
		kh_del(threadstack, map, k);
	}
	mutex_unlock(&my_context->mutex_thread);
}

void AddStackSize(kh_threadstack_t* map, uintptr_t attr, void* stack, size_t stacksize)
{
	khint_t k;
	int ret;
	mutex_lock(&my_context->mutex_thread);
	k = kh_put(threadstack, map, attr, &ret);
	threadstack_t* ts = kh_value(map, k) = (threadstack_t*)box_calloc(1, sizeof(threadstack_t));
	ts->stack = stack;
	ts->stacksize = stacksize;
	mutex_unlock(&my_context->mutex_thread);
}

// return stack from attr (or from current emu if attr is not found..., wich is wrong but approximate enough?)
int GetStackSize(x86emu_t* emu, uintptr_t attr, void** stack, size_t* stacksize)
{
	if(emu->context->stacksizes && attr) {
		mutex_lock(&my_context->mutex_thread);
		khint_t k = kh_get(threadstack, emu->context->stacksizes, attr);
		if(k!=kh_end(emu->context->stacksizes)) {
			threadstack_t* ts = kh_value(emu->context->stacksizes, k);
			*stack = ts->stack;
			*stacksize = ts->stacksize;
			mutex_unlock(&my_context->mutex_thread);
			return 1;
		}
		mutex_unlock(&my_context->mutex_thread);
	}
	// should a Warning be emited?
	*stack = emu->init_stack;
	*stacksize = emu->size_stack;
	return 0;
}

void my_longjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val);

typedef struct emuthread_s {
	uintptr_t 	fnc;
	void*		arg;
	x86emu_t*	emu;
	int			cancel_cap, cancel_size;
	x86_unwind_buff_t **cancels;
} emuthread_t;

static pthread_key_t thread_key;

static void emuthread_destroy(void* p)
{
	emuthread_t *et = (emuthread_t*)p;
	if(!et)
		return;
	void* ptr;
	// free x86emu
	if(et) {
		FreeX86Emu(&et->emu);
		box_free(et);
	}
	pthread_setspecific(thread_key, NULL);
}

static void emuthread_cancel(void* p)
{
	emuthread_t *et = (emuthread_t*)p;
	if(!et)
		return;
	// check cancels threads
	for(int i=et->cancel_size-1; i>=0; --i) {
		et->emu->flags.quitonlongjmp = 0;
		my_longjmp(et->emu, et->cancels[i]->__cancel_jmp_buf, 1);
		DynaRun(et->emu);	// will return after a __pthread_unwind_next()
	}
	box_free(et->cancels);
	et->cancels=NULL;
	et->cancel_size = et->cancel_cap = 0;
}

void thread_set_emu(x86emu_t* emu)
{
	emuthread_t *et = (emuthread_t*)pthread_getspecific(thread_key);
	if(!et) {
		et = (emuthread_t*)box_calloc(1, sizeof(emuthread_t));
	} else {
		if(et->emu != emu)
			FreeX86Emu(&et->emu);
	}
	et->emu = emu;
	et->emu->type = EMUTYPE_MAIN;
	pthread_setspecific(thread_key, et);
}

x86emu_t* thread_get_emu()
{
	emuthread_t *et = (emuthread_t*)pthread_getspecific(thread_key);
	if(!et) {
		int stacksize = 2*1024*1024;
		// try to get stack size of the thread
		pthread_attr_t attr;
		if(!pthread_getattr_np(pthread_self(), &attr)) {
			size_t stack_size;
        	void *stack_addr;
			if(!pthread_attr_getstack(&attr, &stack_addr, &stack_size))
				stacksize = stack_size;
			pthread_attr_destroy(&attr);
		}
		//void* stack = box_calloc(1, stacksize);
		void* stack = mmap(NULL, stacksize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_GROWSDOWN, -1, 0);
		x86emu_t *emu = NewX86Emu(my_context, 0, (uintptr_t)stack, stacksize, 1);
		SetupX86Emu(emu);
		thread_set_emu(emu);
		return emu;
	}
	return et->emu;
}

static void* pthread_routine(void* p)
{
	// free current emuthread if it exist
	{
		void* t = pthread_getspecific(thread_key);
		if(t) {
			// not sure how this could happens
			printf_log(LOG_INFO, "Clean of an existing ET for Thread %04d\n", GetTID());
			emuthread_destroy(t);
		}
	}
	pthread_setspecific(thread_key, p);
	// call the function
	emuthread_t *et = (emuthread_t*)p;
	et->emu->type = EMUTYPE_MAIN;
	// setup callstack and run...
	x86emu_t* emu = et->emu;
    R_ESP -= 32;	// guard area
	Push32(emu, (uintptr_t)et->arg);
	PushExit(emu);
	R_EIP = et->fnc;
	pthread_cleanup_push(emuthread_cancel, p);
	DynaRun(et->emu);
	pthread_cleanup_pop(0);
	void* ret = (void*)R_EAX;
	//void* ret = (void*)RunFunctionWithEmu(et->emu, 0, et->fnc, 1, et->arg);
	return ret;
}

EXPORT int my_pthread_attr_destroy(x86emu_t* emu, void* attr)
{
	if(emu->context->stacksizes)
		FreeStackSize(emu->context->stacksizes, (uintptr_t)attr);
	return pthread_attr_destroy(attr);
}

EXPORT int my_pthread_attr_getstack(x86emu_t* emu, void* attr, void** stackaddr, size_t* stacksize)
{
	int ret = pthread_attr_getstack(attr, stackaddr, stacksize);
	if (ret==0)
		GetStackSize(emu, (uintptr_t)attr, stackaddr, stacksize);
	return ret;
}

EXPORT int my_pthread_attr_setstack(x86emu_t* emu, void* attr, void* stackaddr, size_t stacksize)
{
	if(!emu->context->stacksizes) {
		emu->context->stacksizes = kh_init(threadstack);
	}
	AddStackSize(emu->context->stacksizes, (uintptr_t)attr, stackaddr, stacksize);
	//Don't call actual setstack...
	//return pthread_attr_setstack(attr, stackaddr, stacksize);
	return pthread_attr_setstacksize(attr, stacksize);
}

EXPORT int my_pthread_create(x86emu_t *emu, void* t, void* attr, void* start_routine, void* arg)
{
	int stacksize = 2*1024*1024;	//default stack size is 2Mo
	void* attr_stack;
	size_t attr_stacksize;
	int own;
	void* stack;

	if(attr) {
		size_t stsize;
		if(pthread_attr_getstacksize(attr, &stsize)==0)
			stacksize = stsize;
		if(stsize<512*1024)	// emu and all needs some stack space, don't go too low
			pthread_attr_setstacksize(attr, 512*1024);
	}
	if(GetStackSize(emu, (uintptr_t)attr, &attr_stack, &attr_stacksize))
	{
		stack = attr_stack;
		stacksize = attr_stacksize;
		own = 0;
	} else {
		//stack = box_malloc(stacksize);
		stack = mmap(NULL, stacksize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_GROWSDOWN, -1, 0);
		own = 1;
	}

	emuthread_t *et = (emuthread_t*)box_calloc(1, sizeof(emuthread_t));
    x86emu_t *emuthread = NewX86Emu(my_context, (uintptr_t)start_routine, (uintptr_t)stack, stacksize, own);
	SetupX86Emu(emuthread);
	SetFS(emuthread, GetFS(emu));
	et->emu = emuthread;
	et->fnc = (uintptr_t)start_routine;
	et->arg = arg;
	#ifdef DYNAREC
	if(box86_dynarec) {
		// pre-creation of the JIT code for the entry point of the thread
		dynablock_t *current = NULL;
		DBGetBlock(emu, (uintptr_t)start_routine, 1);
	}
	#endif
	// create thread
	return pthread_create((pthread_t*)t, (const pthread_attr_t *)attr, 
		pthread_routine, et);
}

void* my_prepare_thread(x86emu_t *emu, void* f, void* arg, int ssize, void** pet)
{
	int stacksize = (ssize)?ssize:(2*1024*1024);	//default stack size is 2Mo
	//void* stack = box_malloc(stacksize);
	void* stack = mmap(NULL, stacksize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_GROWSDOWN, -1, 0);
	emuthread_t *et = (emuthread_t*)box_calloc(1, sizeof(emuthread_t));
    x86emu_t *emuthread = NewX86Emu(emu->context, (uintptr_t)f, (uintptr_t)stack, stacksize, 1);
	SetupX86Emu(emuthread);
	SetFS(emuthread, GetFS(emu));
	et->emu = emuthread;
	et->fnc = (uintptr_t)f;
	et->arg = arg;
	#ifdef DYNAREC
	if(box86_dynarec) {
		// pre-creation of the JIT code for the entry point of the thread
		dynablock_t *current = NULL;
		DBGetBlock(emu, (uintptr_t)f, 1);
	}
	#endif
	*pet =  et;
	return pthread_routine;
}

EXPORT void my___pthread_register_cancel(x86emu_t* emu, void* buff)
{
	// on i386, the function as __cleanup_fct_attribute attribute: so 1st parameter is in register
	emuthread_t *et = (emuthread_t*)pthread_getspecific(thread_key);
	if(et->cancel_cap == et->cancel_size) {
		et->cancel_cap+=8;
		et->cancels = box_realloc(et->cancels, sizeof(x86_unwind_buff_t*)*et->cancel_cap);
	}
	et->cancels[et->cancel_size++] = (void*)R_EAX;
}

EXPORT void my___pthread_unregister_cancel(x86emu_t* emu, x86_unwind_buff_t* buff)
{
	// on i386, the function as __cleanup_fct_attribute attribute: so 1st parameter is in register
	emuthread_t *et = (emuthread_t*)pthread_getspecific(thread_key);
	for (int i=et->cancel_size-1; i>=0; --i) {
		if(et->cancels[i] == (x86_unwind_buff_t*)R_EAX) {
			if(i!=et->cancel_size-1)
				memmove(et->cancels+i, et->cancels+i+1, sizeof(x86_unwind_buff_t*)*(et->cancel_size-i-1));
			et->cancel_size--;
		}
	}
}

EXPORT void my___pthread_unwind_next(x86emu_t* emu, void* p)
{
	// on i386, the function as __cleanup_fct_attribute attribute: so 1st parameter is in register
	emu->quit = 1;
}

KHASH_MAP_INIT_INT(once, int)

#define SUPER() \
GO(0)			\
GO(1)			\
GO(2)			\
GO(3)			\
GO(4)			\
GO(5)			\
GO(6)			\
GO(7)			\
GO(8)			\
GO(9)			\
GO(10)			\
GO(11)			\
GO(12)			\
GO(13)			\
GO(14)			\
GO(15)			\
GO(16)			\
GO(17)			\
GO(18)			\
GO(19)			\
GO(20)			\
GO(21)			\
GO(22)			\
GO(23)			\
GO(24)			\
GO(25)			\
GO(26)			\
GO(27)			\
GO(28)			\
GO(29)			

// cleanup_routine
#define GO(A)   \
static uintptr_t my_cleanup_routine_fct_##A = 0;  					\
static void my_cleanup_routine_##A(void* a)    						\
{                                       							\
    RunFunctionFmt(my_cleanup_routine_fct_##A, "p", a);	\
}
SUPER()
#undef GO
static void* findcleanup_routineFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_cleanup_routine_fct_##A == (uintptr_t)fct) return my_cleanup_routine_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_cleanup_routine_fct_##A == 0) {my_cleanup_routine_fct_##A = (uintptr_t)fct; return my_cleanup_routine_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pthread cleanup_routine callback\n");
    return NULL;
}

// key_dtor
#define GO(A)   \
static uintptr_t my_key_dtor_fct_##A = 0;  					\
static void my_key_dtor_##A(void* a)    						\
{                                       							\
    RunFunctionFmt(my_key_dtor_fct_##A, "p", a);	\
}
SUPER()
#undef GO
static void* findkey_dtorFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_key_dtor_fct_##A == (uintptr_t)fct) return my_key_dtor_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_key_dtor_fct_##A == 0) {my_key_dtor_fct_##A = (uintptr_t)fct; return my_key_dtor_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pthread key_dtor callback\n");
    return NULL;
}
#undef SUPER


// custom implementation of pthread_once...
int EXPORT my_pthread_once(x86emu_t* emu, int* once, void* cb)
{
	#ifdef DYNAREC
	#ifdef ARM
	int old = arm_lock_xchg(once, 1);
	#else
	#error Not implemented
	#endif
	#else
	int old = *once;	// outside of the mutex in case once is badly formed
	pthread_mutex_lock(&my_context->mutex_lock);
	old = *once;
	*once = 1;
	pthread_mutex_unlock(&my_context->mutex_lock);
	#endif
	if(old)
		return 0;
    // make some room and align R_RSP before doing the call (maybe it would be simpler to just use Callback functions)
    Push32(emu, R_EBP); // push rbp
    R_EBP = R_ESP;      // mov rbp, rsp
    R_ESP -= 0x200;
    R_ESP &= ~63LL;
	DynaCall(emu, (uintptr_t)cb);  // using DynaCall now, speedup wine 7.21 (proabbly other too) init
	R_ESP = R_EBP;          // mov rsp, rbp
	R_EBP = Pop32(emu);     // pop rbp
	return 0;
}
EXPORT int my___pthread_once(x86emu_t* emu, void* once, void* cb) __attribute__((alias("my_pthread_once")));

EXPORT int my_pthread_key_create(x86emu_t* emu, pthread_key_t* key, void* dtor)
{
	int ret = pthread_key_create(key, findkey_dtorFct(dtor));
	return ret;
}
EXPORT int my___pthread_key_create(x86emu_t* emu, pthread_key_t* key, void* dtor) __attribute__((alias("my_pthread_key_create")));

EXPORT int my_pthread_key_delete(x86emu_t* emu, pthread_key_t key)
{
	int ret = pthread_key_delete(key);
	return ret;
}
typedef struct pthread_cond_old_s {
	pthread_cond_t* cond;
} pthread_cond_old_t;

static pthread_cond_t* get_cond(pthread_cond_old_t* cond) {
	if(!cond->cond) {
		pthread_cond_t* newcond = box_calloc(1, sizeof(pthread_cond_t));
		#ifdef DYNAREC
		if(arm_lock_storeifnull(&cond->cond, newcond))
			box_free(newcond);
		#else
		static pthread_mutex_t mutex_cond = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_lock(&mutex_cond);
		if(!cond->cond)
			cond->cond = newcond;
		else
			box_free(newcond);
		#endif
	}
	return cond->cond;
}

static pthread_mutex_t* getAlignedMutex(pthread_mutex_t* m);

EXPORT int my_pthread_cond_broadcast_old(x86emu_t* emu, pthread_cond_old_t* cond)
{
    (void)emu;
	pthread_cond_t * c = get_cond(cond);
	return pthread_cond_broadcast(c);
}
EXPORT int my_pthread_cond_destroy_old(x86emu_t* emu, pthread_cond_old_t* cond)
{
    (void)emu;
	pthread_cond_t * c = get_cond(cond);
	int ret = pthread_cond_destroy(c);
	box_free(cond->cond);
	return ret;
}
EXPORT int my_pthread_cond_init_old(x86emu_t* emu, pthread_cond_old_t* cond, void* attr)
{
    (void)emu;
	pthread_cond_t *c = get_cond(cond);
	return pthread_cond_init(c, (const pthread_condattr_t*)attr);
}
EXPORT int my_pthread_cond_signal_old(x86emu_t* emu, pthread_cond_old_t* cond)
{
    (void)emu;
	pthread_cond_t * c = get_cond(cond);
	return pthread_cond_signal(c);
}
EXPORT int my_pthread_cond_timedwait_old(x86emu_t* emu, pthread_cond_old_t* cond, void* mutex, void* abstime)
{
    (void)emu;
	pthread_cond_t * c = get_cond(cond);
	#ifdef __USE_TIME64_REDIRECTS
    struct timespec t1;
    Timespec2Timespec64(&t1, abstime);
	#define T abstime?(&t1):NULL
	#else
	#define T (const struct timespec*)abstime
	#endif
	return pthread_cond_timedwait(c, getAlignedMutex((pthread_mutex_t*)mutex), T);
	#undef T
}
EXPORT int my_pthread_cond_wait_old(x86emu_t* emu, pthread_cond_old_t* cond, void* mutex)
{
    (void)emu;
	pthread_cond_t * c = get_cond(cond);
	return pthread_cond_wait(c, getAlignedMutex((pthread_mutex_t*)mutex));
}

EXPORT int my_pthread_cond_timedwait(x86emu_t* emu, void* cond, void* mutex, void* abstime)
{
    (void)emu;
	#ifdef __USE_TIME64_REDIRECTS
    struct timespec t1;
    Timespec2Timespec64(&t1, abstime);
	#define T abstime?(&t1):NULL
	#else
	#define T (const struct timespec*)abstime
	#endif
	return pthread_cond_timedwait((pthread_cond_t*)cond, getAlignedMutex((pthread_mutex_t*)mutex), T);
	#undef T
}
EXPORT int my_pthread_cond_wait(x86emu_t* emu, void* cond, void* mutex)
{
    (void)emu;
	return pthread_cond_wait((pthread_cond_t*)cond, getAlignedMutex((pthread_mutex_t*)mutex));
}

EXPORT int my_pthread_mutexattr_setkind_np(x86emu_t* emu, void* t, int kind)
{
    (void)emu;
    // does "kind" needs some type of translation?
    return pthread_mutexattr_settype(t, kind);
}

EXPORT int my_pthread_attr_setscope(x86emu_t* emu, void* attr, int scope)
{
    (void)emu;
    if(scope!=PTHREAD_SCOPE_SYSTEM) printf_log(LOG_INFO, "Warning, scope of call to pthread_attr_setscope(...) changed from %d to PTHREAD_SCOPE_SYSTEM\n", scope);
	return pthread_attr_setscope(attr, PTHREAD_SCOPE_SYSTEM);
    //The scope is either PTHREAD_SCOPE_SYSTEM or PTHREAD_SCOPE_PROCESS
    // but PTHREAD_SCOPE_PROCESS doesn't seem supported on ARM linux, and PTHREAD_SCOPE_SYSTEM is default
}

#ifndef ANDROID
EXPORT void my__pthread_cleanup_push_defer(x86emu_t* emu, void* buffer, void* routine, void* arg)
{
    (void)emu;
	real_pthread_cleanup_push_defer(buffer, findcleanup_routineFct(routine), arg);
}

EXPORT void my__pthread_cleanup_push(x86emu_t* emu, void* buffer, void* routine, void* arg)
{
    (void)emu;
	_pthread_cleanup_push(buffer, findcleanup_routineFct(routine), arg);
}

EXPORT void my__pthread_cleanup_pop_restore(x86emu_t* emu, void* buffer, int exec)
{
    (void)emu;
	real_pthread_cleanup_pop_restore(buffer, exec);
}

EXPORT void my__pthread_cleanup_pop(x86emu_t* emu, void* buffer, int exec)
{
    (void)emu;
	_pthread_cleanup_pop(buffer, exec);
}

// getaffinity_np (pthread or attr) hav an "old" version (glibc-2.3.3) that only have 2 args, cpusetsize is omited
EXPORT int my_pthread_getaffinity_np(x86emu_t* emu, pthread_t thread, int cpusetsize, void* cpuset)
{
    (void)emu;
	if(cpusetsize>0x1000) {
		// probably old version of the function, that didn't have cpusetsize....
		cpuset = (void*)cpusetsize;
		cpusetsize = sizeof(cpu_set_t);
	} 

	int ret = pthread_getaffinity_np(thread, cpusetsize, cpuset);
	if(ret<0) {
		printf_log(LOG_INFO, "Warning, pthread_getaffinity_np(%p, %d, %p) errored, with errno=%d\n", (void*)thread, cpusetsize, cpuset, errno);
	}

    return ret;
}

EXPORT int my_pthread_setaffinity_np(x86emu_t* emu, pthread_t thread, int cpusetsize, void* cpuset)
{
    (void)emu;
	if(cpusetsize>0x1000) {
		// probably old version of the function, that didn't have cpusetsize....
		cpuset = (void*)cpusetsize;
		cpusetsize = sizeof(cpu_set_t);
	} 

	int ret = pthread_setaffinity_np(thread, cpusetsize, cpuset);
	if(ret<0) {
		printf_log(LOG_INFO, "Warning, pthread_setaffinity_np(%p, %d, %p) errored, with errno=%d\n", (void*)thread, cpusetsize, cpuset, errno);
	}

    return ret;
}

EXPORT int my_pthread_attr_setaffinity_np(x86emu_t* emu, void* attr, uint32_t cpusetsize, void* cpuset)
{
    (void)emu;
	if(cpusetsize>0x1000) {
		// probably old version of the function, that didn't have cpusetsize....
		cpuset = (void*)cpusetsize;
		cpusetsize = sizeof(cpu_set_t);
	} 

	int ret = pthread_attr_setaffinity_np(attr, cpusetsize, cpuset);
	if(ret<0) {
		printf_log(LOG_INFO, "Warning, pthread_attr_setaffinity_np(%p, %d, %p) errored, with errno=%d\n", attr, cpusetsize, cpuset, errno);
	}

    return ret;
}
#endif

EXPORT int my_pthread_kill(x86emu_t* emu, void* thread, int sig)
{
    (void)emu;
	// should ESCHR result be filtered, as this is expected to be the 2.34 behaviour?
    // check for old "is everything ok?"
    if((thread==NULL) && (sig==0))
        return pthread_kill(pthread_self(), 0);
    return pthread_kill((pthread_t)thread, sig);
}

EXPORT int my_pthread_kill_old(x86emu_t* emu, void* thread, int sig)
{
    (void)emu;
    // check for old "is everything ok?"
	if(!thread)
		thread = (void*)pthread_self();
    return real_phtread_kill_old((pthread_t)thread, sig);
}

EXPORT int my_pthread_join(x86emu_t* emu, void* thread, void** ret)
{
    (void)emu;
	if(!thread)
		return ESRCH;
	return pthread_join((pthread_t)thread, ret);
}

//EXPORT void my_pthread_exit(x86emu_t* emu, void* retval)
//{
//  (void)emu;
//	emu->quit = 1;	// to be safe
//	pthread_exit(retval);
//}

#ifdef NOALIGN
pthread_mutex_t* getAlignedMutex(pthread_mutex_t* m) {
	return m;
}
#else
#ifdef ANDROID
// On android, mutex might be defined diferently, so keep the old method in place there for now
KHASH_MAP_INIT_INT(mutex, pthread_mutex_t*)
static kh_mutex_t* unaligned_mutex = NULL;
pthread_mutex_t* getAlignedMutex(pthread_mutex_t* m)
{
	if(!(((uintptr_t)m)&3))
		return m;
	khint_t k = kh_get(mutex, unaligned_mutex, (uintptr_t)m);
	if(k!=kh_end(unaligned_mutex))
		return kh_value(unaligned_mutex, k);
	int r;
	k = kh_put(mutex, unaligned_mutex, (uintptr_t)m, &r);
	pthread_mutex_t* ret = kh_value(unaligned_mutex, k) = (pthread_mutex_t*)box_malloc(sizeof(pthread_mutex_t));
	memcpy(ret, m, sizeof(pthread_mutex_t));
	return ret;
}
EXPORT int my_pthread_mutex_destroy(pthread_mutex_t *m)
{
	if(!(((uintptr_t)m)&3))
		return pthread_mutex_destroy(m);
	khint_t k = kh_get(mutex, unaligned_mutex, (uintptr_t)m);
	if(k!=kh_end(unaligned_mutex)) {
		pthread_mutex_t *n = kh_value(unaligned_mutex, k);
		kh_del(mutex, unaligned_mutex, k);
		int ret = pthread_mutex_destroy(n);
		box_free(n);
		return ret;
	}
	return pthread_mutex_destroy(m);
}
#define getAlignedMutexWithInit(A, B)	getAlignedMutex(A)
#else
#define MUTEXES_SIZE	64
typedef struct mutexes_block_s {
	pthread_mutex_t mutexes[MUTEXES_SIZE];
	uint8_t	taken[MUTEXES_SIZE];
	struct mutexes_block_s* next;
	int n_free;
} mutexes_block_t;

static mutexes_block_t *mutexes = NULL;
#ifndef USE_CUSTOM_MUTEX
static pthread_mutex_t mutex_mutexes = PTHREAD_MUTEX_INITIALIZER;
#else
static uint32_t mutex_mutexes = 0;
#endif

static mutexes_block_t* NewMutexesBlock()
{
	mutexes_block_t* ret = (mutexes_block_t*)box_calloc(1, sizeof(mutexes_block_t));
	ret->n_free = MUTEXES_SIZE;
	return ret;
}

static int NewMutex() {
	mutex_lock(&mutex_mutexes);
	if(!mutexes) {
		mutexes = NewMutexesBlock();
	}
	int j = 0;
	mutexes_block_t* m = mutexes;
	while(!m->n_free) {
		if(!m->next) {
			m->next = NewMutexesBlock();
		}
		++j;
		m = m->next;
	}
	--m->n_free;
	for (int i=0; i<MUTEXES_SIZE; ++i)
		if(!m->taken[i]) {
			m->taken[i] = 1;
			mutex_unlock(&mutex_mutexes);
			return j*MUTEXES_SIZE + i;
		}
	mutex_unlock(&mutex_mutexes);
	printf_log(LOG_NONE, "Error: NewMutex unreachable part reached\n");
	return (int)-1;	// error!!!!
}

void FreeMutex(int k)
{
	if(!mutexes)
		return;	//???
	mutex_lock(&mutex_mutexes);
	mutexes_block_t* m = mutexes;
	for(int i=0; i<k/MUTEXES_SIZE; ++i)
		m = m->next;
	m->taken[k%MUTEXES_SIZE] = 0;
	++m->n_free;
	mutex_unlock(&mutex_mutexes);
}

void FreeAllMutexes(mutexes_block_t* m)
{
	if(!m)
		return;
	FreeAllMutexes(m->next);
	// should destroy all mutexes also?
	box_free(m);
}

pthread_mutex_t* GetMutex(int k)
{
	if(!mutexes)
		return NULL;	//???
	mutexes_block_t* m = mutexes;
	for(int i=0; i<k/MUTEXES_SIZE; ++i)
		m = m->next;
	return &m->mutexes[k%MUTEXES_SIZE];
}

// x86 pthread_mutex_t is 24 bytes (ARM32 too)
typedef struct aligned_mutex_s {
	struct aligned_mutex_s *self;
	uint32_t	dummy;
	int k;
	int kind;	// kind position on x86
	pthread_mutex_t* m;
	uint32_t sign;
} aligned_mutex_t;
#define SIGNMTX *(uint32_t*)"MUTX"

static pthread_mutex_t* getAlignedMutexWithInit(pthread_mutex_t* m, int init)
{
	if(box86_mutex_aligned)
		return m;
	if(!m)
		return NULL;
	aligned_mutex_t* am = (aligned_mutex_t*)m;
	if(init && (am->sign==SIGNMTX && am->self==am))
		return am->m;
	int k = NewMutex();
	// check again, it might be created now becayse NewMutex is under mutex
	if(init && (am->sign==SIGNMTX && am->self==am)) {
		FreeMutex(k);
		return am->m;
	}
	pthread_mutex_t* ret = GetMutex(k);

	#ifndef __PTHREAD_MUTEX_HAVE_PREV
	#define __PTHREAD_MUTEX_HAVE_PREV 0
	#endif
	if(init) {
		if(am->sign == SIGNMTX) {
			int kind = ((int*)am->m)[3+__PTHREAD_MUTEX_HAVE_PREV];	// extract kind from original mutex
			((int*)ret)[3+__PTHREAD_MUTEX_HAVE_PREV] = kind;		// inject in new one (i.e. "init" it)
		} else {
			int kind = am->kind;	// extract kind from original mutex
			((int*)ret)[3+__PTHREAD_MUTEX_HAVE_PREV] = kind;		// inject in new one (i.e. "init" it)
		}
	}
	am->self = am;
	am->sign = SIGNMTX;
	am->k = k;
	am->m = ret;
	return ret;
}
static pthread_mutex_t* getAlignedMutex(pthread_mutex_t* m)
{
	return getAlignedMutexWithInit(m, 1);
}

EXPORT int my_pthread_mutex_destroy(pthread_mutex_t *m)
{
	if(box86_mutex_aligned)
		return pthread_mutex_destroy(m);
	aligned_mutex_t* am = (aligned_mutex_t*)m;
	if(am->sign!=SIGNMTX) {
		return 1;	//???
	}
	int ret = pthread_mutex_destroy(am->m);
	FreeMutex(am->k);
	am->sign = 0;
	return ret;
}
#endif	//ANDROID
EXPORT int my___pthread_mutex_destroy(pthread_mutex_t *m) __attribute__((alias("my_pthread_mutex_destroy")));

EXPORT int my_pthread_mutex_init(pthread_mutex_t *m, pthread_mutexattr_t *att)
{
	return pthread_mutex_init(getAlignedMutexWithInit(m, 0), att);
}
EXPORT int my___pthread_mutex_init(pthread_mutex_t *m, pthread_mutexattr_t *att) __attribute__((alias("my_pthread_mutex_init")));

EXPORT int my_pthread_mutex_lock(pthread_mutex_t *m)
{
	return pthread_mutex_lock(getAlignedMutex(m));
}
EXPORT int my___pthread_mutex_lock(pthread_mutex_t *m) __attribute__((alias("my_pthread_mutex_lock")));

EXPORT int my_pthread_mutex_timedlock(pthread_mutex_t *m, void* abstime)
{
	#ifdef __USE_TIME64_REDIRECTS
    struct timespec t1;
    Timespec2Timespec64(&t1, abstime);
	#define T abstime?(&t1):NULL
	#else
	#define T (const struct timespec*)abstime
	#endif
	return pthread_mutex_timedlock(getAlignedMutex(m), T);
	#undef T
}

EXPORT int my_pthread_mutex_trylock(pthread_mutex_t *m)
{
	return pthread_mutex_trylock(getAlignedMutex(m));
}
EXPORT int my___pthread_mutex_trylock(pthread_mutex_t *m) __attribute__((alias("my_pthread_mutex_trylock")));

EXPORT int my_pthread_mutex_unlock(pthread_mutex_t *m)
{
	return pthread_mutex_unlock(getAlignedMutex(m));
}
EXPORT int my___pthread_mutex_unlock(pthread_mutex_t *m) __attribute__((alias("my_pthread_mutex_unlock")));

#endif

void init_pthread_helper()
{
	real_pthread_cleanup_push_defer = (vFppp_t)dlsym(NULL, "_pthread_cleanup_push_defer");
	real_pthread_cleanup_pop_restore = (vFpi_t)dlsym(NULL, "_pthread_cleanup_pop_restore");

	// search for older symbol for pthread_kill
	{
		char buff[50];
		for(int i=0; i<34 && !real_phtread_kill_old; ++i) {
			snprintf(buff, 50, "GLIBC_2.%d", i);
			real_phtread_kill_old = (iFli_t)dlvsym(NULL, "pthread_kill", buff);
		}
	}
	if(!real_phtread_kill_old) {
		printf_log(LOG_INFO, "Warning, older than 2.34 pthread_kill not found, using current one\n");
		real_phtread_kill_old = (iFli_t)pthread_kill;
	}

	pthread_key_create(&thread_key, emuthread_destroy);
	pthread_setspecific(thread_key, NULL);
#if !defined(NOALIGN) && defined(ANDROID)
	unaligned_mutex = kh_init(mutex);
#endif
}

void clean_current_emuthread()
{
	emuthread_t *et = (emuthread_t*)pthread_getspecific(thread_key);
	if(et) {
		pthread_setspecific(thread_key, NULL);
		emuthread_destroy(et);
	}
}

void fini_pthread_helper(box86context_t* context)
{
	CleanStackSize(context);
#ifndef NOALIGN
#ifdef ANDROID
	pthread_mutex_t *m;
	kh_foreach_value(unaligned_mutex, m, 
		pthread_mutex_destroy(m);
		box_free(m);
	);
	kh_destroy(mutex, unaligned_mutex);
#else
	FreeAllMutexes(mutexes);
#endif //!ANDROID
#endif
	emuthread_t *et = (emuthread_t*)pthread_getspecific(thread_key);
	if(et) {
		pthread_setspecific(thread_key, NULL);
		emuthread_destroy(et);
	}
}

int checkUnlockMutex(void* m)
{
	pthread_mutex_t* mutex = (pthread_mutex_t*)m;
	int ret = pthread_mutex_unlock(mutex);
	if(ret==0) {
		return 1;
	}
	return 0;
}
