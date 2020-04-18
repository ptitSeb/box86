#include <stdio.h>
#include <stdlib.h>
// __USE_UNIX98 is needed for sttype / gettype definition
#define __USE_UNIX98
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

void CleanStackSize(box86context_t* context)
{
	threadstack_t *ts;
	if(!context->stacksizes)
		return;
	kh_foreach_value(context->stacksizes, ts, free(ts));
	kh_destroy(threadstack, context->stacksizes);
	context->stacksizes = NULL;
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

void InitCancelThread(box86context_t* context)
{
	context->cancelthread = kh_init(cancelthread);
}

void FreeCancelThread(box86context_t* context)
{
	__pthread_unwind_buf_t* buff;
	kh_foreach_value(context->cancelthread, buff, free(buff))
	kh_destroy(cancelthread, context->cancelthread);
	context->cancelthread = NULL;
}
__pthread_unwind_buf_t* AddCancelThread(box86context_t* context, uintptr_t buff)
{
	int ret;
	khint_t k = kh_put(cancelthread, context->cancelthread, buff, &ret);
	if(ret)
		kh_value(context->cancelthread, k) = (__pthread_unwind_buf_t*)calloc(1, sizeof(__pthread_unwind_buf_t));
	return kh_value(context->cancelthread, k);
}

void DelCancelThread(box86context_t* context, uintptr_t buff)
{
	khint_t k = kh_get(cancelthread, context->cancelthread, buff);
	if(k==kh_end(context->cancelthread))
		return;
	free(kh_value(context->cancelthread, k));
	kh_del(cancelthread, context->cancelthread, k);
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
	
	pthread_cleanup_push(pthread_clean_routine, p);

	x86emu_t *emu = et->emu;
	Push(emu, (uint32_t)et->arg);
	DynaCall(emu, et->fnc);
	R_ESP+=4;
	r = (void*)R_EAX;

	pthread_cleanup_pop(1);

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
	return pthread_attr_setstack(attr, stackaddr, stacksize);
}

EXPORT int my_pthread_create(x86emu_t *emu, void* t, void* attr, void* start_routine, void* arg)
{
	pthread_mutex_lock(&emu->context->mutex_lock);
	int stacksize = 2*1024*1024;	//default stack size is 2Mo
	if(attr) {
		size_t stsize;
		if(pthread_attr_getstacksize(attr, &stsize)==0)
			if(stacksize<stsize) stacksize = stsize;
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
EXPORT void my___pthread_register_cancel(x86emu_t* emu, x86_unwind_buff_t* buff)
{
	__pthread_unwind_buf_t * pbuff = AddCancelThread(emu->context, (uintptr_t)buff);

	int not_first_call = __sigsetjmp((struct __jmp_buf_tag*)(void*)pbuff->__cancel_jmp_buf, 0);
	if(not_first_call) {
		my_longjmp(emu, pbuff, not_first_call);
		return;
	}

	__pthread_register_cancel(pbuff);
}

EXPORT void my___pthread_unregister_cancel(x86emu_t* emu, x86_unwind_buff_t* buff)
{
	__pthread_unwind_buf_t * pbuff = AddCancelThread(emu->context, (uintptr_t)buff);

	__pthread_unregister_cancel(pbuff);

	DelCancelThread(emu->context, (uintptr_t)buff);
}



KHASH_MAP_INIT_INT(once, int)

static box86context_t *my_context = NULL;

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

static void my_cleanup_routine(void* arg)
{
    if(!arg)
        return;
    x86emu_t *emu = (x86emu_t*)arg;
    RunCallback(emu);
	FreeCallback(emu);
}
void _pthread_cleanup_push_defer(void* buffer, void* routine, void* arg);	// declare the function that should be in pthread lib
void _pthread_cleanup_pop_restore(void* buffer, int exec);

EXPORT void my__pthread_cleanup_push_defer(x86emu_t* emu, void* buffer, void* routine, void* arg)
{
    // create a callback...
    x86emu_t* cbemu = AddCallback(emu, (uintptr_t)routine, 1, arg, NULL, NULL, NULL);
	_pthread_cleanup_push_defer(buffer, my_cleanup_routine, cbemu);
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

void init_pthread_helper(box86context_t* context)
{
	my_context = context;
}

void fini_pthread_helper()
{
	my_context = NULL;
}