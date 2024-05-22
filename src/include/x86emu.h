#ifndef __X86EMU_H_
#define __X86EMU_H_

typedef struct x86emu_s x86emu_t;
typedef struct box86context_s box86context_t;

x86emu_t *NewX86Emu(box86context_t *context, uintptr_t start, uintptr_t stack, int stacksize, int ownstack);
x86emu_t *NewX86EmuFromStack(x86emu_t* emu, box86context_t *context, uintptr_t start, uintptr_t stack, int stacksize, int ownstack);
void SetupX86Emu(x86emu_t *emu);
void FreeX86Emu(x86emu_t **x86emu);
void FreeX86EmuFromStack(x86emu_t **emu);
void CloneEmu(x86emu_t *newemu, const x86emu_t* emu);
void CopyEmu(x86emu_t *newemu, const x86emu_t* emu);
void SetTraceEmu(uintptr_t trace_start, uintptr_t trace_end);

box86context_t* GetEmuContext(x86emu_t* emu);

uint32_t GetEAX(x86emu_t *emu);
uint64_t GetEDXEAX(x86emu_t *emu);
void SetEAX(x86emu_t *emu, uint32_t v);
void SetEBX(x86emu_t *emu, uint32_t v);
void SetECX(x86emu_t *emu, uint32_t v);
void SetEDX(x86emu_t *emu, uint32_t v);
void SetEDI(x86emu_t *emu, uint32_t v);
void SetESI(x86emu_t *emu, uint32_t v);
void SetEBP(x86emu_t *emu, uint32_t v);
void SetESP(x86emu_t *emu, uint32_t v);
void SetEIP(x86emu_t *emu, uint32_t v);
void SetFS(x86emu_t *emu, uint16_t v);
uint16_t GetFS(x86emu_t *emu);
uint32_t GetESP(x86emu_t *emu);
void ResetFlags(x86emu_t *emu);
void ResetSegmentsCache(x86emu_t *emu);
const char* DumpCPURegs(x86emu_t* emu, uintptr_t ip);

void StopEmu(x86emu_t* emu, const char* reason);
void EmuCall(x86emu_t* emu, uintptr_t addr);
void AddCleanup(x86emu_t *emu, void *p);
void AddCleanup1Arg(x86emu_t *emu, void *p, void* a);
void CallCleanup(x86emu_t *emu, void* p);
void CallAllCleanup(x86emu_t *emu);
void UnimpOpcode(x86emu_t* emu);

uint64_t ReadTSC(x86emu_t* emu);

double FromLD(void* ld);        // long double (80bits pointer) -> double
long double LD2localLD(void* ld);        // long double (80bits pointer) -> long double (80 or 128 or 64)
void LD2D(void* ld, void* d);   // long double (80bits) -> double (64bits)
void D2LD(void* d, void* ld);   // double (64bits) -> long double (64bits)

void printFunctionAddr(uintptr_t nextaddr, const char* text);
const char* getAddrFunctionName(uintptr_t addr);

#endif //__X86EMU_H_