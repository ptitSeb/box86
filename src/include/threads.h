#ifndef _THREADS_H_
#define _THREADS_H_

typedef struct box86context_s box86context_t;
typedef struct x86emu_s x86emu_t;

void CleanStackSize(box86context_t* context);

void init_pthread_helper();
void fini_pthread_helper(box86context_t* context);
void clean_current_emuthread(void);

// prepare an "emuthread structure" in pet and return address of function pointer for a "thread creation routine"
void* my_prepare_thread(x86emu_t *emu, void* f, void* arg, int ssize, void** pet);

#ifndef USE_CUSTOM_MUTEX
//check and unlock if a mutex is locked by current thread (works only for PTHREAD_MUTEX_ERRORCHECK typed mutex)
int checkUnlockMutex(void* m);
#endif

#endif //_THREADS_H_