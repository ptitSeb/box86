#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

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
#include "custommem.h"

#ifndef STEP
#error No STEP defined
#endif

uintptr_t arm_pass(dynarec_arm_t* dyn, uintptr_t addr)
{
    int ok = 1;
    int ninst = 0;
    uintptr_t ip = addr;
    uintptr_t init_addr = addr;
    MAYUSE(init_addr);
    int need_epilog = 1;
    dyn->sons_size = 0;
    // Clean up (because there are multiple passes)
    dyn->state_flags = 0;
    dyn->dfnone = 0;
    fpu_reset(dyn, ninst);
    // ok, go now
    INIT;
    while(ok) {
        ip = addr;
        if((dyn->insts[ninst].x86.barrier==BARRIER_FULL)) {
            NEW_BARRIER_INST;
        }
        // propagate ST stack state, especial stack pop that are defered
        if(dyn->n.stack_pop) {
            for(int j=0; j<24; ++j)
                if((dyn->n.neoncache[j].t == NEON_CACHE_ST_D || dyn->n.neoncache[j].t == NEON_CACHE_ST_F)) {
                    if(dyn->n.neoncache[j].n<dyn->n.stack_pop)
                        dyn->n.neoncache[j].v = 0;
                    else
                        dyn->n.neoncache[j].n-=dyn->n.stack_pop;
                }
            dyn->n.stack_pop = 0;
        }
        dyn->n.stack = dyn->n.stack_next;
        NEW_INST;
        fpu_reset_scratch(dyn);
#ifdef HAVE_TRACE
        if(my_context->dec && box86_dynarec_trace) {
        if((trace_end == 0) 
            || ((ip >= trace_start) && (ip < trace_end)))  {
                MESSAGE(LOG_DUMP, "TRACE ----\n");
                fpu_reflectcache(dyn, ninst, x1, x2, x3);
                MOV32(x1, ip);
                STM(xEmu, (1<<xEAX)|(1<<xEBX)|(1<<xECX)|(1<<xEDX)|(1<<xESI)|(1<<xEDI)|(1<<xESP)|(1<<xEBP));
                STR_IMM9(x1, xEmu, offsetof(x86emu_t, ip));
                MOVW(x2, 1);
                CALL(PrintTrace, -1, 0);
                MESSAGE(LOG_DUMP, "----------\n");
            }
        }
#endif

        addr = dynarec00(dyn, addr, ip, ninst, &ok, &need_epilog);

        INST_EPILOG;

        if(dyn->insts[ninst+1].x86.barrier) {
            if(dyn->insts[ninst+1].x86.barrier&BARRIER_FLOAT)
                fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
            if(dyn->insts[ninst+1].x86.barrier&BARRIER_FLAGS) {
                dyn->state_flags = 0;
                dyn->dfnone = 0;
            }
        }
        if(!ok && !need_epilog && (addr < (dyn->start+dyn->isize))) {
            ok = 1;
        }
        #if STEP == 0
        if(!ok && !need_epilog && box86_dynarec_bigblock && getProtection(addr+3))
            if(*(uint32_t*)addr!=0) {   // check if need to continue (but is next 4 bytes are 0, stop)
                uintptr_t next = get_closest_next(dyn, addr);
                if(next && (
                    (((next-addr)<15) && is_nops(dyn, addr, next-addr)) 
                    ||(((next-addr)<30) && is_instructions(dyn, addr, next-addr)) ))
                {
                    dynarec_log(LOG_DEBUG, "Extend block %p, %p -> %p (ninst=%d)\n", dyn, (void*)addr, (void*)next, ninst);
                    ok = 1;
                } else if(next && (next-addr)<30) {
                    dynarec_log(LOG_DEBUG, "Cannot extend block %p -> %p (%02X %02X %02X %02X %02X %02X %02X %02x)\n", (void*)addr, (void*)next, PK(0), PK(1), PK(2), PK(3), PK(4), PK(5), PK(6), PK(7));
                }
            }
        #endif
        if(ok<0)  {ok = 0; need_epilog=1;}
        ++ninst;
        #if STEP == 0
        if(ok && !isJumpTableDefault((void*)addr))
        #else
        if(ok && (ninst==dyn->size))
        #endif
        {
            #if STEP == 3
            dynarec_log(LOG_DEBUG, "Stopping block %p (%d / %d)\n",(void*)init_addr, ninst, dyn->size); 
            #endif
            BARRIER(BARRIER_FLOAT);
            fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
            jump_to_next(dyn, addr, 0, ninst);
            ok=0; need_epilog=0;
        }
    }
    if(need_epilog) {
        fpu_purgecache(dyn, ninst, 0, x1, x2, x3);
        jump_to_epilog(dyn, ip, 0, ninst);  // no linker here, it's an unknow instruction
    }
    FINI;
    MESSAGE(LOG_DUMP, "---- END OF BLOCK ---- (%d, %d sons)\n", dyn->size, dyn->sons_size);
    return addr;
}