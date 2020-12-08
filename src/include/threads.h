#ifndef _THREADS_H_
#define _THREADS_H_

typedef struct box86context_s box86context_t;
typedef struct x86emu_s x86emu_t;

typedef struct emu_jmpbuf_s {
	void* 	    jmpbuf;
	int 	    jmpbuf_ok;
    x86emu_t*   emu;
} emu_jmpbuf_t;

void CleanStackSize(box86context_t* context);

emu_jmpbuf_t* GetJmpBuf();

void init_pthread_helper();
void fini_pthread_helper(box86context_t* context);

// prepare an "emuthread structure" in pet and return address of function pointer for a "thread creation routine"
void* my_prepare_thread(x86emu_t *emu, void* f, void* arg, int ssize, void** pet);

#endif //_THREADS_H_