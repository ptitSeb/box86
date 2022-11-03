#ifndef __DYNAREC_NEXT_H__
#define __DYNAREC_NEXT_H__

void arm_next(void) EXPORTDYN;
void arm_prolog(x86emu_t* emu, void* addr) EXPORTDYN;
void arm_epilog() EXPORTDYN;

#endif //__DYNAREC_NEXT_H__