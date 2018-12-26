#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "box86context.h"
#include "threads.h"
#include "x86emu_private.h"
#include "bridge_private.h"
#include "x86run.h"
#include "x86emu.h"
#include "stack.h"

// memory handling to be perfected...
// keep a hash thread_t -> emu to set emu->quit to 1 on pthread_cancel

typedef struct emuthread_s {
	x86emu_t 	*emu;
	onebridge_t	routine;
	int			stacksize;
	void		*stack;
} emuthread_t;

static void pthread_clean_routine(void* p)
{
	emuthread_t *et = (emuthread_t*)p;

	FreeX86Emu(&et->emu);
	free(et->stack);
	free(et);
}

static void* pthread_routine(void* p)
{
	void* r = NULL;
	
	pthread_cleanup_push(pthread_clean_routine, p);
	emuthread_t *et = (emuthread_t*)p;
	Run(et->emu);
	r = (void*)GetEAX(et->emu);
	pthread_cleanup_pop(1);

	return r;
}

int my_pthread_create(x86emu_t *emu, void* t, void* attr, void* start_routine, void* arg)
{
	emuthread_t *emuthread = (emuthread_t*)calloc(1, sizeof(emuthread_t));
	emuthread->routine.CC = 0xCC;
	emuthread->routine.S = 'S'; emuthread->routine.C = 'C';
	emuthread->routine.w = pFp;
	emuthread->routine.f = (uintptr_t)start_routine;
	emuthread->routine.C3 = 0xC3;

	emuthread->stacksize = 2*1024*1024;	//default stack size is 2Mo
	// TODO: get stack size inside attr
	emuthread->stack = calloc(1, emuthread->stacksize);
	emuthread->emu = NewX86Emu(emu->context, (uintptr_t)start_routine, (uintptr_t)emuthread->stack, 
		emuthread->stacksize);
	SetupX86Emu(emuthread->emu, emu->shared_global, emu->globals);
	emuthread->emu->trace_start = emu->trace_start;
	emuthread->emu->trace_end = emu->trace_end;
	Push(emuthread->emu, (uintptr_t)arg);
	PushExit(emuthread->emu);

	// create thread
	return pthread_create((pthread_t*)t, (const pthread_attr_t *)attr, 
		pthread_routine, emuthread);
}