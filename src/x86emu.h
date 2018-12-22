#ifndef __X86EMU_H_
#define __X86EMU_H_

typedef struct x86emu_s x86emu_t;
typedef struct box86context_s box86context_t;

x86emu_t *NewX86Emu(box86context_t *context, uintptr_t start, uintptr_t stack, int stacksize);
void FreeX86Emu(x86emu_t **x86emu);

uint32_t GetEAX(x86emu_t *emu);
void SetEAX(x86emu_t *emu, uint32_t v);
void SetEBX(x86emu_t *emu, uint32_t v);
void SetECX(x86emu_t *emu, uint32_t v);
void SetEDX(x86emu_t *emu, uint32_t v);
const char* DumpCPURegs(x86emu_t* emu);

void StopEmu(x86emu_t* emu, const char* reason);

#endif //__X86EMU_H_