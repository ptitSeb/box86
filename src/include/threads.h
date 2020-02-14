#ifndef _THREADS_H_
#define _THREADS_H_

#include "bridge.h"

void CleanStackSize(box86context_t* context);
void InitCancelThread(box86context_t* context);
void FreeCancelThread(box86context_t* context);

#endif //_THREADS_H_