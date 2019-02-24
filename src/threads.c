#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "debug.h"
#include "box86context.h"
#include "threads.h"
#include "x86emu_private.h"
#include "bridge_private.h"
#include "x86run.h"
#include "x86emu.h"
#include "stack.h"
#include "callback.h"
#include "khash.h"
#include "x86run_private.h"
#include "x86trace.h"

// memory handling to be perfected...
// keep a hash thread_t -> emu to set emu->quit to 1 on pthread_cancel

static void pthread_clean_routine(void* p)
{
	x86emu_t *emu = (x86emu_t*)p;

	FreeX86Emu(&emu);
}

static void* pthread_routine(void* p)
{
	void* r = NULL;
	
	pthread_cleanup_push(pthread_clean_routine, p);
	x86emu_t *emu = (x86emu_t*)p;
	Run(emu);
	r = (void*)GetEAX(emu);
	pthread_cleanup_pop(1);

	return r;
}

int EXPORT my_pthread_create(x86emu_t *emu, void* t, void* attr, void* start_routine, void* arg)
{
	int stacksize = 2*1024*1024;	//default stack size is 2Mo
	// TODO: get stack size inside attr
	void* stack = calloc(1, stacksize);
	x86emu_t *emuthread = NewX86Emu(emu->context, (uintptr_t)start_routine, (uintptr_t)stack, 
		stacksize);
	SetupX86Emu(emuthread, emu->shared_global, emu->globals);
	emuthread->trace_start = emu->trace_start;
	emuthread->trace_end = emu->trace_end;
	Push(emuthread, (uintptr_t)arg);
	PushExit(emuthread);
	// create thread
	return pthread_create((pthread_t*)t, (const pthread_attr_t *)attr, 
		pthread_routine, emuthread);
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
static x86emu_t *once_emu = NULL;
static uintptr_t once_fnc = 0;
static void my_thread_once_callback()
{
	if(!once_emu)
		return;
	EmuCall(once_emu, once_fnc);
}

int EXPORT my_pthread_once(x86emu_t* emu, void* once, void* cb)
{
	pthread_mutex_lock(&emu->context->mutex_once);	// what if it deadlock here?

	once_emu = emu;
	once_fnc = (uintptr_t)cb;
	int ret = pthread_once(once, my_thread_once_callback);
	once_emu = NULL;

	pthread_mutex_unlock(&emu->context->mutex_once);
	return ret;
}

EXPORT int my_pthread_key_create(x86emu_t* emu, void* key, void* dtor)
{
	if(!dtor)
		return pthread_key_create((pthread_key_t*)key, NULL);
	int n = 0;
	while (n<nb_dtor) {
		if(!dtor_emu[n] || GetCallbackAddress(dtor_emu[n])==((uintptr_t)dtor)) {
			if(!dtor_emu[n]) 
				dtor_emu[n] = AddCallback(emu, (uintptr_t)dtor, 1, NULL, NULL, NULL, NULL);
			return pthread_key_create((pthread_key_t*)key, dtor_cb[n]);
		}
		++n;
	}
	printf_log(LOG_NONE, "Error: pthread_key_create with destructor: no more slot!\n");
	emu->quit = 1;
	return -1;
}
EXPORT int my___pthread_key_create(x86emu_t* emu, void* key, void* dtor) __attribute__((alias("my_pthread_key_create")));

// phtread_cond_init with null attr seems to only take 1 dword on x86, while it 48 bytes on ARM. 
// Not sure why as sizeof(pthread_cond_init) is 48 on both platform... But Neverwinter Night init seems to rely on that

EXPORT int my_pthread_cond_broadcast(x86emu_t* emu, void* cond)
{
	pthread_cond_t * c = *(pthread_cond_t **)cond;
	return pthread_cond_broadcast(c);
}
EXPORT int my_pthread_cond_destroy(x86emu_t* emu, void* cond)
{
	pthread_cond_t * c = *(pthread_cond_t **)cond;
	int ret = pthread_cond_destroy(c);
	free(c);
	return ret;
}
EXPORT int my_pthread_cond_init(x86emu_t* emu, void* cond, void* attr)
{
	pthread_cond_t * c = *(pthread_cond_t **)cond = (pthread_cond_t *)calloc(1, sizeof(pthread_cond_t));
	return pthread_cond_init(c, (const pthread_condattr_t*)attr);
}
EXPORT int my_pthread_cond_signal(x86emu_t* emu, void* cond)
{
	pthread_cond_t * c = *(pthread_cond_t **)cond;
	return pthread_cond_signal(c);
}
EXPORT int my_pthread_cond_timedwait(x86emu_t* emu, void* cond, void* mutex, void* abstime)
{
	pthread_cond_t * c = *(pthread_cond_t **)cond;
	return pthread_cond_timedwait(c, (pthread_mutex_t*)mutex, (const struct timespec*)abstime);
}
EXPORT int my_pthread_cond_wait(x86emu_t* emu, void* cond, void* mutex)
{
	pthread_cond_t * c = *(pthread_cond_t **)cond;
	return pthread_cond_wait(c, (pthread_mutex_t*)mutex);
}
