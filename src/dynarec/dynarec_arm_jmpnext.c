#include <stdint.h>
#include <stdlib.h>

#include "arm_emitter.h"

#define EMIT(A) *block = (A); ++block
void CreateJmpNext(void* addr, void* next)
{
    uint32_t* block = (uint32_t*)addr;
    LDR_literal(x2, ((intptr_t)next - (intptr_t)addr)-8);
    BX(x2);
}