#ifndef __BRIDGE_H_
#define __BRIDGE_H_
#include <stdint.h>

#include "wrapper.h"

typedef struct x86emu_s x86emu_t;
typedef struct bridge_s bridge_t;

bridge_t *NewBridge();
void FreeBridge(bridge_t** bridge);

uintptr_t AddBridge(bridge_t* bridge, wrapper_t w, void* fnc);
uintptr_t CheckBridged(bridge_t* bridge, void* fnc);

#endif //__BRIDGE_H_