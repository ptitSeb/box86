#ifndef __DYNAREC_ARM_FUNCTIONS_H__
#define __DYNAREC_ARM_FUNCTIONS_H__

typedef struct x86emu_s x86emu_t;

void arm_popf(x86emu_t* emu, uint32_t f);

#endif //__DYNAREC_ARM_FUNCTIONS_H__