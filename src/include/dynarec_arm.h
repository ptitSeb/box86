#ifndef __DYNAREC_ARM_H_
#define __DYNAREC_ARM_H_

typedef struct dynablock_s dynablock_t;
typedef struct x86emu_s x86emu_t;
typedef struct instsize_s instsize_t;

#define MAX_INSTS   32760

void addInst(instsize_t* insts, size_t* size, int x86_size, int arm_size);

void CancelBlock();
void* FillBlock(dynablock_t* block, uintptr_t addr);

#endif //__DYNAREC_ARM_H_