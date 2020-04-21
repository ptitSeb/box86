#ifndef _THREADS_H_
#define _THREADS_H_

typedef struct box86context_s box86context_t;

void CleanStackSize();

void init_pthread_helper();
void fini_pthread_helper();

#endif //_THREADS_H_