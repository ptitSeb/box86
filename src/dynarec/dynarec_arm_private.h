#ifndef __DYNAREC_ARM_PRIVATE_H_
#define __DYNAREC_ARM_PRIVATE_H_

#include "dynarec_private.h"

typedef struct x86emu_s x86emu_t;

typedef struct instruction_arm_s {
    instruction_x86_t   x86;
    uintptr_t           address;    // (start) address of the arm emited instruction
    uintptr_t           epilog;     // epilog of current instruction (can be start of next, of barrier stuff)
    int                 size;       // size of the arm emited instruction
    uintptr_t           mark, mark2, mark3;
    uintptr_t           markf;
} instruction_arm_t;

typedef struct dynarec_arm_s {
    instruction_arm_t   *insts;
    int32_t             size;
    int32_t             cap;
    uintptr_t           start;      // start of the block
    uint32_t            isize;      // size in byte of x86 instructions included
    void*               block;      // memory pointer where next instruction is emited
    int                 arm_size;   // size of emitted arm code
    x86emu_t*           emu;
    uintptr_t*          table;      // jump table
    int                 tablesz;    // size of the jump table
    int                 tablei;     // index
    int                 cleanflags; // flags are clean (CMP or TST have been just executed)
    int                 x87cache[8];// cache status for the 8 x87 register behind the fpu stack
    int                 x87stack;   // cache stack counter
    int                 fpu_scratch;// scratch counter
} dynarec_arm_t;


#endif //__DYNAREC_ARM_PRIVATE_H_