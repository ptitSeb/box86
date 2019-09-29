#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "debug.h"
#include "box86context.h"
#include "dynarec.h"
#include "emu/x86emu_private.h"
#include "tools/bridge_private.h"
#include "x86run.h"
#include "x86emu.h"
#include "box86stack.h"
#include "callback.h"
#include "emu/x86run_private.h"
#include "x86trace.h"
#include "dynablock.h"
#include "dynablock_private.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"

void arm_epilog();

void arm_pass0(dynarec_arm_t* dyn, uintptr_t addr);
void arm_pass1(dynarec_arm_t* dyn, uintptr_t addr);
void arm_pass2(dynarec_arm_t* dyn, uintptr_t addr);

void FillBlock(x86emu_t* emu, dynablock_t* block, uintptr_t addr) {
    // init the helper
    dynarec_arm_t helper = {0};
    arm_pass0(&helper, addr);
    if(!helper.size) {
        dynarec_log(LOG_DEBUG, "Warning, null-sized dynarec block (%p)\n", (void*)addr);
        return;
    }
    helper.cap = helper.size+1;
    helper.insts = (instruction_arm_t*)calloc(helper.cap, sizeof(instruction_arm_t));
    helper.dec = emu->dec;
    // pass 1, addresses, instruction size, x86 jump addresses
    arm_pass1(&helper, addr);
    // pass 2, logging
    arm_pass2(&helper, addr);


    // all done...
    free(helper.insts);
}