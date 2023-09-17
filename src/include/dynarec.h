#ifndef __DYNAREC_H_
#define __DYNAREC_H_

typedef struct x86emu_s x86emu_t;

void DynaCall(x86emu_t* emu, uintptr_t addr); // try to use DynaRec... Fallback to EmuCall if no dynarec available
void x86test_init(x86emu_t* ref, uintptr_t ip);

#endif // __DYNAREC_H_