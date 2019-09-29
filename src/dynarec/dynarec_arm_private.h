#ifndef __DYNAREC_ARM_PRIVATE_H_
#define __DYNAREC_ARM_PRIVATE_H_

#include "dynarec_private.h"

typedef struct zydis_dec_s zydis_dec_t;

typedef struct instruction_arm_s {
    instruction_x86_t   x86;
    int                 size;       // size of the arm emited instruction
} instruction_arm_t;

typedef struct dynarec_arm_s {
    instruction_arm_t   *insts;
    int32_t             size;
    int32_t             cap;
    uintptr_t           start;      // start of the block
    uint32_t            isize;      // size in byte of x86 instructions included
    void*               block;      // memory pointer where next instruction is emited
    int                 arm_size;   // size of emitted arm code
    zydis_dec_t         *dec;
} dynarec_arm_t;


#endif //__DYNAREC_ARM_PRIVATE_H_