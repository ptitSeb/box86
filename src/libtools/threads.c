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

typedef struct emuthread_s {
	x86emu_t *emu;
	uintptr_t fnc;
	void*	arg;
} emuthread_t;

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

int EXPORT my_pthread_create(x86emu_t *emu, void* t, void* attr, void* start_routine, void* arg)
{
	pthread_mutex_lock(&emu->context->mutex_lock);
	int stacksize = 2*1024*1024;	//default stack size is 2Mo
	if(attr) {
		size_t stsize;
		if(pthread_attr_getstacksize(attr, &stsize)==0)
			if(stacksize<stsize) stacksize = stsize;
	}
	void* stack = malloc(stacksize);
	emuthread_t *et = (emuthread_t*)calloc(1, sizeof(emuthread_t));
    x86emu_t *emuthread = NewX86Emu(emu->context, (uintptr_t)start_routine, (uintptr_t)stack, stacksize, 1);
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




KHASH_MAP_INIT_INT(once, int)

#define nb_dtor	16
typedef void(*key_dtor)(void*);
static x86emu_t *dtor_emu[nb_dtor] = {0};
static void key_dtor_callback(int n, void* a)
{
	if(dtor_emu[n]) {
		SetCallbackArg(dtor_emu[n], 0, a);
		RunCallback(dtor_emu[n]);
	}
}
#define GO(N) \
void key_dtor_callback_##N(void* a) \
{ \
	key_dtor_callback(N, a); \
}
GO(0)
GO(1)
GO(2)
GO(3)
GO(4)
GO(5)
GO(6)
GO(7)
GO(8)
GO(9)
GO(10)
GO(11)
GO(12)
GO(13)
GO(14)
GO(15)
#undef GO
static const key_dtor dtor_cb[nb_dtor] = {
	 key_dtor_callback_0, key_dtor_callback_1, key_dtor_callback_2, key_dtor_callback_3
	,key_dtor_callback_4, key_dtor_callback_5, key_dtor_callback_6, key_dtor_callback_7
	,key_dtor_callback_8, key_dtor_callback_9, key_dtor_callback_10,key_dtor_callback_11
	,key_dtor_callback_12,key_dtor_callback_13,key_dtor_callback_14,key_dtor_callback_15
};
// TODO: put all this in libpthread private stuff...
static __thread x86emu_t *once_emu = NULL;
static __thread uintptr_t once_fnc = 0;
static void my_thread_once_callback()
{
	if(!once_emu)
		return;
	EmuCall(once_emu, once_fnc);
}
static __thread x86emu_t *once2_emu = NULL;
static __thread uintptr_t once2_fnc = 0;
static void my_thread_once2_callback()
{
	if(!once2_emu)
		return;
	EmuCall(once2_emu, once2_fnc);
}

int EXPORT my_pthread_once(x86emu_t* emu, void* once, void* cb)
{
	if(pthread_mutex_trylock(&emu->context->mutex_once)==EBUSY)
	{
		// 2nd level...
		pthread_mutex_lock(&emu->context->mutex_once2);
		once2_emu = emu;
		once2_fnc = (uintptr_t)cb;
		int ret = pthread_once(once, my_thread_once2_callback);
		once_emu = NULL;
		pthread_mutex_unlock(&emu->context->mutex_once2);
		return ret;
	} else {
		once_emu = emu;
		once_fnc = (uintptr_t)cb;
		int ret = pthread_once(once, my_thread_once_callback);
		once_emu = NULL;

		pthread_mutex_unlock(&emu->context->mutex_once);
		return ret;
	}
}
EXPORT int my___pthread_once(x86emu_t* emu, void* once, void* cb) __attribute__((alias("my_pthread_once")));

EXPORT int my_pthread_key_create(x86emu_t* emu, void* key, void* dtor)
{
	if(!dtor)
		return pthread_key_create((pthread_key_t*)key, NULL);
	int n = 0;
	while (n<nb_dtor) {
		if(!dtor_emu[n] || GetCallbackAddress(dtor_emu[n])==((uintptr_t)dtor)) {
			if(!dtor_emu[n]) 
				dtor_emu[n] = AddSmallCallback(emu, (uintptr_t)dtor, 1, NULL, NULL, NULL, NULL);
			return pthread_key_create((pthread_key_t*)key, dtor_cb[n]);
		}
		++n;
	}
	printf_log(LOG_NONE, "Error: pthread_key_create with destructor: no more slot!\n");
	emu->quit = 1;
	return -1;
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
