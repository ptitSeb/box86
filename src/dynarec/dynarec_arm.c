#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <sys/mman.h>

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
void arm_pass3(dynarec_arm_t* dyn, uintptr_t addr);

void FillBlock(x86emu_t* emu, dynablock_t* block, uintptr_t addr) {
    // init the helper
    dynarec_arm_t helper = {0};
    arm_pass0(&helper, addr);
    if(!helper.size) {
        dynarec_log(LOG_DEBUG, "Warning, null-sized dynarec block (%p)\n", (void*)addr);
        return;
    }
    helper.cap = helper.size+2; // needs epilog handling
    helper.insts = (instruction_arm_t*)calloc(helper.cap, sizeof(instruction_arm_t));
    helper.dec = emu->dec;
    // pass 1, addresses, x86 jump addresses, flags
    arm_pass1(&helper, addr);
    // calculate barriers
    uintptr_t start = helper.insts[0].x86.addr;
    uintptr_t end = helper.insts[helper.size].x86.addr+helper.insts[helper.size].x86.size;
    for(int i=0; i<helper.size; ++i)
        if(helper.insts[i].x86.jmp) {
            uintptr_t j = helper.insts[i].x86.jmp;
            if(j<start || j>=end)
                helper.insts[i].x86.jmp_is_out = 1;
            else {
                // find jump address instruction
                int k=-1;
                for(int i2=0; i2<helper.size && k==-1; ++i2) {
                    if(helper.insts[i2].x86.addr==j)
                        k=i2;
                }
                if(k==-1)   // not found, mmm, probably wrong, exit anyway
                    helper.insts[i].x86.jmp_is_out = 1;
                else
                    helper.insts[k].x86.barrier = 1;
            }
        }
    // pass 2, instruction size
    arm_pass2(&helper, addr);
    // ok, now alocate mapped memory, with executable flag on
    void* p = mmap(NULL, helper.arm_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(p==MAP_FAILED) {
        dynarec_log(LOG_INFO, "Cannot create memory map of %d byte for dynarec block\n", helper.arm_size);
        free(helper.insts);
        return;
    }
    block->size = helper.arm_size;
    helper.block = p;
    // pass 3, emit (log emit arm opcode)
    dynarec_log(LOG_DEBUG, "Emitting %d bytes for %d x86 bytes\n", helper.arm_size, helper.isize);
    arm_pass3(&helper, addr);
    // all done...
    free(helper.insts);
    block->block = p;
}