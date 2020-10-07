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
void fini_pthread_helper();

#endif //_THREADS_H_