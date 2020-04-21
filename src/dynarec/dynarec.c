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
#ifdef DYNAREC
#include "dynablock.h"
#include "dynablock_private.h"
#endif

#ifdef ARM
void arm_prolog(x86emu_t* emu, void* addr);
void arm_epilog();
void arm_linker();
int arm_tableupdate(void* jump, uintptr_t addr, void** table);
#endif

#ifdef DYNAREC
void* UpdateLinkTable(x86emu_t* emu, void** table, uintptr_t addr)
{
    dynablock_t* block = DBGetBlock(emu, addr, 1, NULL);    // keep a copy of parent block?
    int r;
    if(block==0) {
        // no block, don't try again, ever
        #ifdef ARM
        r = arm_tableupdate(arm_epilog, addr, table);
        if(r) dynarec_log(LOG_DEBUG, "Linker: failed to set table data @%p, for emu=%p\n", table, emu);
        #else
        table[0] = arm_epilog;
        table[1] = addr;
        #endif
        return arm_epilog;
    }
    if(!block->done) {
        // not finished yet... leave linker
        #ifdef ARM
        r = arm_tableupdate(arm_linker, addr, table);
        if(r) dynarec_log(LOG_DEBUG, "Linker: failed to set table data @%p, for emu=%p\n", table, emu);
        #else
        table[0] = arm_linker;
        table[1] = addr;
        #endif
        return arm_epilog;
    }
    if(!block->block) {
        // null block, but done: go to epilog, no linker here
        #ifdef ARM
        r = arm_tableupdate(arm_epilog, addr, table);
        if(r) dynarec_log(LOG_DEBUG, "Linker: failed to set table data @%p, for emu=%p\n", table, emu);
        #else
        table[0] = arm_epilog;
        table[1] = addr;
        #endif
        return arm_epilog;
    }
    if(!block->parent->nolinker) {
        // only update block if linker is allowed
        #ifdef ARM
        r = arm_tableupdate(block->block, addr, table);
        if(r) dynarec_log(LOG_DEBUG, "Linker: failed to set table data @%p, for emu=%p\n", table, emu);
        #else
        table[0] = block->block;
        table[1] = addr;
        #endif
    }
    return block->block;
}
#endif

void DynaCall(x86emu_t* emu, uintptr_t addr)
{
#ifdef DYNAREC
    if(!box86_dynarec)
#endif
        EmuCall(emu, addr);
#ifdef DYNAREC
    else {
        uint32_t old_esp = R_ESP;
        uint32_t old_ebx = R_EBX;
        uint32_t old_edi = R_EDI;
        uint32_t old_esi = R_ESI;
        uint32_t old_ebp = R_EBP;
        uint32_t old_eip = R_EIP;
        PushExit(emu);
        R_EIP = addr;
        emu->df = d_none;
        dynablock_t* block = NULL;
        while(!emu->quit) {
            block = DBGetBlock(emu, R_EIP, 1, block);
            if(!block || !block->block || !block->done) {
                // no block, of block doesn't have DynaRec content (yet, temp is not null)
                // Use interpreter (should use single instruction step...)
                dynarec_log(LOG_DEBUG, "Calling Interpretor @%p, emu=%p\n", (void*)R_EIP, emu);
                Run(emu, 1);
            } else {
                dynarec_log(LOG_DEBUG, "Calling DynaRec Block @%p (%p) emu=%p\n", (void*)R_EIP, block->block, emu);
                CHECK_FLAGS(emu);
                // block is here, let's run it!
                #ifdef ARM
                arm_prolog(emu, block->block);
                #endif
            }
            if(emu->fork) {
                int forktype = emu->fork;
                emu->quit = 0;
                emu->fork = 0;
                emu = x86emu_fork(emu, forktype);
            }
        }
        emu->quit = 0;  // reset Quit flags...
        emu->df = d_none;
        if(emu->quitonlongjmp && emu->longjmp) {
            emu->longjmp = 0;   // don't change anything because of the longjmp
        } else {
            R_EBX = old_ebx;
            R_EDI = old_edi;
            R_ESI = old_esi;
            R_EBP = old_ebp;
            R_ESP = old_esp;
            R_EIP = old_eip;  // and set back instruction pointer
        }
    }
#endif
}

int DynaRun(x86emu_t* emu)
{
#ifdef DYNAREC
    if(!box86_dynarec)
#endif
        return Run(emu, 0);
#ifdef DYNAREC
    else {
        dynablock_t* block = NULL;
        while(!emu->quit) {
            block = DBGetBlock(emu, R_EIP, 1, block);
            if(!block || !block->block || !block->done) {
                // no block, of block doesn't have DynaRec content (yet, temp is not null)
                // Use interpreter (should use single instruction step...)
                dynarec_log(LOG_DEBUG, "Running Interpretor @%p, emu=%p\n", (void*)R_EIP, emu);
                Run(emu, 1);
            } else {
                dynarec_log(LOG_DEBUG, "Running DynaRec Block @%p (%p) emu=%p\n", (void*)R_EIP, block->block, emu);
                // block is here, let's run it!
                #ifdef ARM
                arm_prolog(emu, block->block);
                #endif
            }
            if(emu->fork) {
                int forktype = emu->fork;
                emu->quit = 0;
                emu->fork = 0;
                emu = x86emu_fork(emu, forktype);
            }
        }
    }
    return 0;
#endif
}