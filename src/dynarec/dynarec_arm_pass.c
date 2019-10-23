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

#include "dynarec_arm_helper.h"

void arm_pass(dynarec_arm_t* dyn, uintptr_t addr)
{
    uint8_t nextop, opcode;
    int ok = 1;
    int ninst = 0;
    uintptr_t ip = addr;
    uintptr_t natcall;
    int retn;
    uint8_t gd, ed;
    int8_t i8;
    int32_t i32, tmp;
    uint8_t u8;
    uint8_t gb1, gb2, eb1, eb2;
    uint32_t u32;
    int need_epilog = 1;
    uint8_t wback, wb1, wb2;
    int fixedaddress;
    dyn->tablei = 0;
    // Clean up (because there are multiple passes)
    dyn->cleanflags = 0;
    x87_reset(dyn, ninst);
    // ok, go now
    INIT;
    while(ok) {
        ip = addr;
        NEW_INST;
        fpu_reset_scratch(dyn);
#ifdef HAVE_TRACE
        if(dyn->emu->dec && box86_dynarec_trace) {
        if((dyn->emu->trace_end == 0) 
            || ((ip >= dyn->emu->trace_start) && (ip <= dyn->emu->trace_end)))  {
                MESSAGE(LOG_DUMP, "TRACE ----\n");
                if(trace_xmm)
                    fpu_reflectcache(dyn, ninst, x1, x2, x3);
                else
                    x87_reflectcache(dyn, ninst, x1, x2, x3);
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
                dyn->cleanflags = 0;
        }
        ++ninst;
    }
    if(need_epilog) {
        fpu_purgecache(dyn, ninst, x1, x2, x3);
        jump_to_epilog(dyn, ip, 0, ninst);  // no linker here, it's an unknow instruction
    }
    FINI;
}