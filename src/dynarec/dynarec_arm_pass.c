#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>

#include "debug.h"
#include "box86context.h"
#include "dynarec.h"
#include "emu/x86emu_private.h"
#include "emu/x86run_private.h"
#include "x86run.h"
#include "x86emu.h"
#include "box86stack.h"
#include "emu/x86run_private.h"
#include "x86trace.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"
#include "arm_printer.h"
#include "dynarec_arm_functions.h"
#include "dynarec_arm_helper.h"

#ifndef STEP
#error No STEP defined
#endif

void arm_pass(dynarec_arm_t* dyn, uintptr_t addr)
{
    int ok = 1;
    int ninst = 0;
    uintptr_t ip = addr;
    int need_epilog = 1;
    dyn->tablei = 0;
    // Clean up (because there are multiple passes)
    dyn->state_flags = 0;
    fpu_reset(dyn, ninst);
    // ok, go now
    INIT;
    while(ok) {
if(dyn->insts && (ninst>dyn->size)) {dynarec_log(LOG_NONE, "Warning, too many inst treated (%d / %d)\n",ninst, dyn->size);}
        ip = addr;
        NEW_INST;
        fpu_reset_scratch(dyn);
#ifdef HAVE_TRACE
        if(dyn->emu->dec && box86_dynarec_trace) {
        if((dyn->emu->trace_end == 0) 
            || ((ip >= dyn->emu->trace_start) && (ip <= dyn->emu->trace_end)))  {
                MESSAGE(LOG_DUMP, "TRACE ----\n");
                fpu_reflectcache(dyn, ninst, x1, x2, x3);
                MOV32(1, ip);
                STM(0, (1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)|(1<<11));
                STR_IMM9(1, 0, offsetof(x86emu_t, ip));
                MOV32(2, 1);
                CALL(PrintTrace, -1, 0);
                MESSAGE(LOG_DUMP, "----------\n");
            }
        }
#endif

        addr = dynarec00(dyn, addr, ip, ninst, &ok, &need_epilog);

        INST_EPILOG;

        if(dyn->insts && dyn->insts[ninst+1].x86.barrier) {
            fpu_purgecache(dyn, ninst, x1, x2, x3);
            if(dyn->insts[ninst+1].x86.barrier!=2)
                dyn->state_flags = 0;
        }
        if(!ok && !need_epilog && dyn->insts && (addr < (dyn->start+dyn->isize))) {
            ok = 1;
        }
        if(!ok && !need_epilog && !dyn->insts) {   // check if need to continue
            uintptr_t next = get_closest_next(dyn, addr);
            if(next && ((next-addr)<15) && is_nops(dyn, addr, next-addr)) {
                dynarec_log(LOG_DEBUG, "Extend block %p, %p -> %p (ninst=%d)\n", dyn, (void*)addr, (void*)next, ninst);
                ok = 1;
            }
        }
        ++ninst;
    }
    if(need_epilog) {
        fpu_purgecache(dyn, ninst, x1, x2, x3);
        jump_to_epilog(dyn, ip, 0, ninst);  // no linker here, it's an unknow instruction
    }
    FINI;
}