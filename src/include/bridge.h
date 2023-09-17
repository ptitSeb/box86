#ifndef __BRIDGE_H_
#define __BRIDGE_H_
#include <stdint.h>

typedef struct x86emu_s x86emu_t;
typedef struct bridge_s bridge_t;
typedef struct box86context_s box86context_t;
typedef void (*wrapper_t)(x86emu_t* emu, uintptr_t fnc);

bridge_t *NewBridge();
void FreeBridge(bridge_t** bridge);

uintptr_t AddBridge(bridge_t* bridge, wrapper_t w, void* fnc, int N, const char* name);
uintptr_t CheckBridged(bridge_t* bridge, void* fnc);
int IsBridge(void* func);
uintptr_t AddCheckBridge(bridge_t* bridge, wrapper_t w, void* fnc, int N, const char* name);
uintptr_t AddAutomaticBridge(x86emu_t* emu, bridge_t* bridge, wrapper_t w, void* fnc, int N, const char* name);
void* GetNativeFnc(uintptr_t fnc);
void* GetNativeFncOrFnc(uintptr_t fnc);

int hasAlternate(void* addr);
void* getAlternate(void* addr);
void addAlternate(void* addr, void* alt);
void cleanAlternate();

#ifdef HAVE_TRACE
const char* getBridgeName(void* addr);
#endif

void init_bridge_helper();
void fini_bridge_helper();

#endif //__BRIDGE_H_