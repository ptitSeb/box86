#ifndef __DYNAREC_ARM_FUNCTIONS_H__
#define __DYNAREC_ARM_FUNCTIONS_H__

typedef struct x86emu_s x86emu_t;

void arm_popf(x86emu_t* emu, uint32_t f);
void arm_fstp(x86emu_t* emu, void* p);

void arm_print_armreg(x86emu_t* emu, uintptr_t reg, uintptr_t n);

void arm_f2xm1(x86emu_t* emu);
void arm_fyl2x(x86emu_t* emu);
void arm_ftan(x86emu_t* emu);
void arm_fpatan(x86emu_t* emu);
void arm_fxtract(x86emu_t* emu);
void arm_fprem(x86emu_t* emu);
void arm_fyl2xp1(x86emu_t* emu);
void arm_fsincos(x86emu_t* emu);
void arm_frndint(x86emu_t* emu);
void arm_fscale(x86emu_t* emu);
void arm_fsin(x86emu_t* emu);
void arm_fcos(x86emu_t* emu);
void arm_fbld(x86emu_t* emu, uint8_t* ed);
void arm_fild64(x86emu_t* emu, int64_t* ed);
void arm_fbstp(x86emu_t* emu, uint8_t* ed);
void arm_fistp64(x86emu_t* emu, int64_t* ed);
void arm_fld(x86emu_t* emu, uint8_t* ed);

#endif //__DYNAREC_ARM_FUNCTIONS_H__