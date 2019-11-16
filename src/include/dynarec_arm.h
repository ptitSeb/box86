#ifndef __DYNAREC_ARM_H_
#define __DYNAREC_ARM_H_

typedef struct dynablock_s dynablock_t;
typedef struct x86emu_s x86emu_t;

void FillBlock(x86emu_t* emu, dynablock_t* block, uintptr_t addr);

#endif //__DYNAREC_ARM_H_