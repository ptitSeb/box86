#include <stdint.h>
#include <stdlib.h>

#include "arm_emitter.h"

#define EMIT(A) *block = (A); ++block
void CreateJmpNextTo(void* addr, void* tonext)
{
    uint32_t* block = (uint32_t*)addr;
    MOV32(x3, (uintptr_t)tonext);
    BX(x3);
}