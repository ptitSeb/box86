// __USE_UNIX98 is needed for sttype / gettype definition
#define __USE_UNIX98
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include "debug.h"
#include "box86context.h"
#include "threads.h"
#include "emu/x86emu_private.h"
#include "tools/bridge_private.h"
#include "x86run.h"
#include "x86emu.h"
#include "box86stack.h"
#include "callback.h"
#include "khash.h"
#include "emu/x86run_private.h"
#include "x86trace.h"
#include "dynarec.h"
#include "bridge.h"

void _pthread_cleanup_push_defer(void* buffer, void* routine, void* arg);	// declare hidden functions
void _pthread_cleanup_pop_restore(void* buffer, int exec);

typedef struct emuthread_s {
	x86emu_t *emu;
	uintptr_t fnc;
	void*	arg;
} emuthread_t;

typedef struct threadstack_s {
	void* 	stack;
	size_t 	stacksize;
} threadstack_t;

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

typedef struct x86_unwind_buff_s {
	struct {
		jump_buff_i386_t	__cancel_jmp_buf;	
		int					__mask_was_saved;
	} __cancel_jmp_buf[1];
	void *__pad[4];
} x86_unwind_buff_t __attribute__((__aligned__));

KHASH_MAP_INIT_INT(threadstack, threadstack_t*)
KHASH_MAP_INIT_INT(cancelthread, __pthread_unwind_buf_t*)

void CleanStackSize()
{
	threadstack_t *ts;
	if(!my_context->stacksizes)
		return;
	kh_foreach_value(my_context->stacksizes, ts, free(ts));
	kh_destroy(threadstack, my_context->stacksizes);
	my_context->stacksizes = NULL;
}

void FreeStackSize(kh_threadstack_t* map, uintptr_t attr)
{
	khint_t k = kh_get(threadstack, map, attr);
	if(k==kh_end(map))
		return;
	free(kh_value(map, k));
	kh_del(threadstack, map, k);
}

void AddStackSize(kh_threadstack_t* map, uintptr_t attr, void* stack, size_t stacksize)
{
	khint_t k;
	int ret;
	k = kh_put(threadstack, map, attr, &ret);
	threadstack_t* ts = kh_value(map, k) = (threadstack_t*)calloc(1, sizeof(threadstack_t));
	ts->stack = stack;
	ts->stacksize = stacksize;
}

// return stack from attr (or from current emu if attr is not found..., wich is wrong but approximate enough?)
int GetStackSize(x86emu_t* emu, uintptr_t attr, void** stack, size_t* stacksize)
{
	if(emu->context->stacksizes && attr) {
		khint_t k = kh_get(threadstack, emu->context->stacksizes, attr);
		if(k!=kh_end(emu->context->stacksizes)) {
			threadstack_t* ts = kh_value(emu->context->stacksizes, k);
			*stack = ts->stack;
			*stacksize = ts->stacksize;
			return 1;
		}
	}
	// should a Warning be emited?
	*stack = emu->init_stack;
	*stacksize = emu->size_stack;
	return 0;
}

static void InitCancelThread()
{
	my_context->cancelthread = kh_init(cancelthread);
}

static void FreeCancelThread()
{
	__pthread_unwind_buf_t* buff;
	kh_foreach_value(my_context->cancelthread, buff, free(buff))
	kh_destroy(cancelthread, my_context->cancelthread);
	my_context->cancelthread = NULL;
}
static __pthread_unwind_buf_t* AddCancelThread(uintptr_t buff)
{
	int ret;
	khint_t k = kh_put(cancelthread, my_context->cancelthread, buff, &ret);
	if(ret)
		kh_value(my_context->cancelthread, k) = (__pthread_unwind_buf_t*)calloc(1, sizeof(__pthread_unwind_buf_t));
	return kh_value(my_context->cancelthread, k);
}

static void DelCancelThread(uintptr_t buff)
{
	khint_t k = kh_get(cancelthread, my_context->cancelthread, buff);
	if(k==kh_end(my_context->cancelthread))
		return;
	free(kh_value(my_context->cancelthread, k));
	kh_del(cancelthread, my_context->cancelthread, k);
}

static void pthread_clean_routine(void* p)
{
	emuthread_t *et = (emuthread_t*)p;
	FreeX86Emu(&et->emu);
	free(et);
}

static void* pthread_routine(void* p)
{
	void* r = NULL;

	emuthread_t *et = (emuthread_t*)p;
	
	pthread_cleanup_push_defer_np(pthread_clean_routine, p);

	x86emu_t *emu = et->emu;
	r = (void*)RunFunctionWithEmu(emu, 0, et->fnc, 1, et->arg);

	pthread_cleanup_pop_restore_np(1);

	return r;
}

EXPORT int my_pthread_attr_destroy(x86emu_t* emu, void* attr)
{
	if(emu->context->stacksizes)
		FreeStackSize(emu->context->stacksizes, (uintptr_t)attr);
	return pthread_attr_destroy(attr);
}

EXPORT int my_pthread_attr_getstack(x86emu_t* emu, void* attr, void* stackaddr, size_t* stacksize)
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
	pthread_mutex_lock(&emu->context->mutex_lock);
	int stacksize = 2*1024*1024;	//default stack size is 2Mo
	if(attr) {
		size_t stsize;
		if(pthread_attr_getstacksize(attr, &stsize)==0)
			stacksize = stsize;
	}
	void* attr_stack;
	size_t attr_stacksize;
	int own;
	void* stack;
	if(GetStackSize(emu, (uintptr_t)attr, &attr_stack, &attr_stacksize))
	{
		stack = attr_stack;
		stacksize = attr_stacksize;
		own = 0;
	} else {
		stack = malloc(stacksize);
		own = 1;
	}
	emuthread_t *et = (emuthread_t*)calloc(1, sizeof(emuthread_t));
    x86emu_t *emuthread = NewX86Emu(emu->context, (uintptr_t)start_routine, (uintptr_t)stack, stacksize, own);
	SetupX86Emu(emuthread);
    emuthread->trace_start = emu->trace_start;
    emuthread->trace_end = emu->trace_end;
	et->emu = emuthread;
	et->fnc = (uintptr_t)start_routine;
	et->arg = arg;
	pthread_mutex_unlock(&emu->context->mutex_lock);
	// create thread
	return pthread_create((pthread_t*)t, (const pthread_attr_t *)attr, 
		pthread_routine, et);
}

void my_longjmp(x86emu_t* emu, /*struct __jmp_buf_tag __env[1]*/void *p, int32_t __val);

#define CANCEL_MAX 8
static __thread x86emu_t* cancel_emu[CANCEL_MAX] = {0};
static __thread x86_unwind_buff_t* cancel_buff[CANCEL_MAX] = {0};
static __thread int cancel_deep = 0;
EXPORT void my___pthread_register_cancel(void* E, void* B)
{
	// get a stack local copy of the args, as may be live in some register depending the architecture (like ARM)
	if(cancel_deep<0) {
		printf_log(LOG_INFO, "BOX86: Warning, inconsistant value in __pthread_register_cancel (%d)\n", cancel_deep);
		cancel_deep = 0;
	}
	if(cancel_deep!=CANCEL_MAX-1) 
		++cancel_deep;
	else
		{printf_log(LOG_INFO, "BOX86: Warning, calling __pthread_register_cancel(...) too many time\n");}
		
	cancel_emu[cancel_deep] = (x86emu_t*)E;
	// on i386, the function as __cleanup_fct_attribute attribute: so 1st parameter is in register
	x86_unwind_buff_t* buff = cancel_buff[cancel_deep] = (x86_unwind_buff_t*)((x86emu_t*)E)->regs[_AX].dword[0];
	__pthread_unwind_buf_t * pbuff = AddCancelThread((uintptr_t)buff);
	if(__sigsetjmp((struct __jmp_buf_tag*)(void*)pbuff->__cancel_jmp_buf, 0)) {
		//DelCancelThread((uintptr_t)cancel_buff);	// no del here, it will be delete by unwind_next...
		int i = cancel_deep--;
		x86emu_t* emu = cancel_emu[i];
		my_longjmp(emu, cancel_buff[i]->__cancel_jmp_buf, 1);
		DynaRun(emu);	// resume execution
		return;
	}

	__pthread_register_cancel(pbuff);
}

EXPORT void my___pthread_unregister_cancel(x86emu_t* emu, x86_unwind_buff_t* buff)
{
	// on i386, the function as __cleanup_fct_attribute attribute: so 1st parameter is in register
	buff = (x86_unwind_buff_t*)R_EAX;
	__pthread_unwind_buf_t * pbuff = AddCancelThread((uintptr_t)buff);
	__pthread_unregister_cancel(pbuff);

	--cancel_deep;
	DelCancelThread((uintptr_t)buff);
}

EXPORT void my___pthread_unwind_next(x86emu_t* emu, void* p)
{
	// on i386, the function as __cleanup_fct_attribute attribute: so 1st parameter is in register
	x86_unwind_buff_t* buff = (x86_unwind_buff_t*)R_EAX;
	__pthread_unwind_buf_t *pbuff = AddCancelThread((uintptr_t)buff);
	// function is noreturn, so putting stuff on the stack is not a good idea
	__pthread_unwind_next(pbuff);
	// just in case it does return
	// no clear way to clean up this, unless it does return...
	DelCancelThread((uintptr_t)buff);
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
GO(15)

// once_callback
#define GO(A)   \
static uintptr_t my_once_callback_fct_##A = 0;  \
static void my_once_callback_##A()    			\
{                                       		\
    RunFunction(my_context, my_once_callback_fct_##A, 0, 0);\
}
SUPER()
#undef GO
static void* findonce_callbackFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_once_callback_fct_##A == (uintptr_t)fct) return my_once_callback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_once_callback_fct_##A == 0) {my_once_callback_fct_##A = (uintptr_t)fct; return my_once_callback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pthread once_callback callback\n");
    return NULL;
}
// key_destructor
#define GO(A)   \
static uintptr_t my_key_destructor_fct_##A = 0;  \
static void my_key_destructor_##A(void* a)    			\
{                                       		\
    RunFunction(my_context, my_key_destructor_fct_##A, 1, a);\
}
SUPER()
#undef GO
static void* findkey_destructorFct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_key_destructor_fct_##A == (uintptr_t)fct) return my_key_destructor_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_key_destructor_fct_##A == 0) {my_key_destructor_fct_##A = (uintptr_t)fct; return my_key_destructor_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for pthread key_destructor callback\n");
    return NULL;
}
// cleanup_routine
#define GO(A)   \
static uintptr_t my_cleanup_routine_fct_##A = 0;  \
static void my_cleanup_routine_##A(void* a)    			\
{                                       		\
    RunFunction(my_context, my_cleanup_routine_fct_##A, 1, a);\
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

#undef SUPER

int EXPORT my_pthread_once(x86emu_t* emu, void* once, void* cb)
{
	return pthread_once(once, findonce_callbackFct(cb));
}
EXPORT int my___pthread_once(x86emu_t* emu, void* once, void* cb) __attribute__((alias("my_pthread_once")));

EXPORT int my_pthread_key_create(x86emu_t* emu, void* key, void* dtor)
{
	return pthread_key_create(key, findkey_destructorFct(dtor));
}
EXPORT int my___pthread_key_create(x86emu_t* emu, void* key, void* dtor) __attribute__((alias("my_pthread_key_create")));

// phtread_cond_init with null attr seems to only write 1 (NULL) dword on x86, while it 48 bytes on ARM. 
// Not sure why as sizeof(pthread_cond_init) is 48 on both platform... But Neverwinter Night init seems to rely on that
// What about cond that are statically initialized? It's a versionned function, corresponding to an old behaviour

KHASH_MAP_INIT_INT(mapcond, pthread_cond_t*);

// should all access to that map be behind a mutex?
kh_mapcond_t *mapcond = NULL;

static pthread_cond_t* add_cond(void* cond)
{
	if(!mapcond)
		mapcond = kh_init(mapcond);
	khint_t k;
	int ret;
	k = kh_put(mapcond, mapcond, (uintptr_t)cond, &ret);
	pthread_cond_t *c = kh_value(mapcond, k) = (pthread_cond_t*)calloc(1, sizeof(pthread_cond_t));
	*(uint32_t*)cond = 0;
	return c;
}
static pthread_cond_t* get_cond(void* cond)
{
	if(!mapcond)
		return (pthread_cond_t*)cond;
	khint_t k = kh_get(mapcond, mapcond, (uintptr_t)cond);
	if(k==kh_end(mapcond))
		return (pthread_cond_t*)cond;
	return kh_value(mapcond, k);
}
static void del_cond(void* cond)
{
	if(!mapcond)
		return;
	khint_t k = kh_get(mapcond, mapcond, (uintptr_t)cond);
	if(k!=kh_end(mapcond)) {
		free(kh_value(mapcond, k));
		kh_del(mapcond, mapcond, k);
		if(kh_size(mapcond)==0) {
			kh_destroy(mapcond, mapcond);
			mapcond = NULL;
		}
	}
}

EXPORT int my_pthread_cond_broadcast(x86emu_t* emu, void* cond)
{
	pthread_cond_t * c = get_cond(cond);
	sched_yield();
	return pthread_cond_broadcast(c);
}
EXPORT int my_pthread_cond_destroy(x86emu_t* emu, void* cond)
{
	pthread_cond_t * c = get_cond(cond);
	int ret = pthread_cond_destroy(c);
	if(c!=cond) del_cond(cond);
	return ret;
}
EXPORT int my_pthread_cond_init(x86emu_t* emu, void* cond, void* attr)
{
	pthread_cond_t *c = (pthread_cond_t*)cond;
	if(attr==NULL)
		c = add_cond(cond);
	return pthread_cond_init(c, (const pthread_condattr_t*)attr);
}
EXPORT int my_pthread_cond_signal(x86emu_t* emu, void* cond)
{
	pthread_cond_t * c = get_cond(cond);
	return pthread_cond_signal(c);
}
EXPORT int my_pthread_cond_timedwait(x86emu_t* emu, void* cond, void* mutex, void* abstime)
{
	pthread_cond_t * c = get_cond(cond);
	return pthread_cond_timedwait(c, (pthread_mutex_t*)mutex, (const struct timespec*)abstime);
}
EXPORT int my_pthread_cond_wait(x86emu_t* emu, void* cond, void* mutex)
{
	pthread_cond_t * c = get_cond(cond);
	return pthread_cond_wait(c, (pthread_mutex_t*)mutex);
}

EXPORT int my_pthread_mutexattr_setkind_np(x86emu_t* emu, void* t, int kind)
{
    // does "kind" needs some type of translation?
    return pthread_mutexattr_settype(t, kind);
}

EXPORT int my_pthread_attr_setscope(x86emu_t* emu, void* attr, int scope)
{
    if(scope!=PTHREAD_SCOPE_SYSTEM) printf_log(LOG_INFO, "Warning, call to pthread_attr_setaffinity_np(...) changed\n");
	return pthread_attr_setscope(attr, PTHREAD_SCOPE_SYSTEM);
    //The scope is either PTHREAD_SCOPE_SYSTEM or PTHREAD_SCOPE_PROCESS
    // but PTHREAD_SCOPE_PROCESS doesn't seem supported on ARM linux, and PTHREAD_SCOPE_SYSTEM is default
}

EXPORT void my__pthread_cleanup_push_defer(x86emu_t* emu, void* buffer, void* routine, void* arg)
{
	_pthread_cleanup_push_defer(buffer, findcleanup_routineFct(routine), arg);
}

EXPORT void my__pthread_cleanup_pop_restore(x86emu_t* emu, void* buffer, int exec)
{
	_pthread_cleanup_pop_restore(buffer, exec);
}


EXPORT int my_pthread_kill(x86emu_t* emu, void* thread, int sig)
{
    // check for old "is everything ok?"
    if(thread==NULL && sig==0)
        return pthread_kill(pthread_self(), 0);
    return pthread_kill((pthread_t)thread, sig);
}

void init_pthread_helper()
{
	InitCancelThread();
}

void fini_pthread_helper()
{
	FreeCancelThread();
	CleanStackSize();
}
