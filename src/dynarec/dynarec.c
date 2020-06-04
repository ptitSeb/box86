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
void arm_prolog(x86emu_t* emu, void* addr) EXPORTDYN;
void arm_epilog() EXPORTDYN;
void arm_linker() EXPORTDYN;
int arm_tableupdate(void* jump, uintptr_t addr, void** table) EXPORTDYN;
#endif

void tableupdate(void* jumpto, uintptr_t ref, void** table)
{
    #ifdef ARM
    if(arm_tableupdate(jumpto, ref, table))
    #endif
    {
        table[0] = jumpto;
        table[1] = (void*)ref;
    }
}

void resettable(void** table)
{
    void* p = table[1];
    #ifdef ARM
    tableupdate(arm_linker, (uintptr_t)p, table);
    #endif
    //table[2] = own_dynablock // unchanged
    table[3] = NULL;    // removed "linked" information
}

#ifdef DYNAREC
void* UpdateLinkTable(x86emu_t* emu, void** table, uintptr_t addr)
{
    dynablock_t* current = (dynablock_t*)table[2];
    if(current->father)
        current = current->father;
    dynablock_t* block = DBGetBlock(emu, addr, 1, &current);
    if(!current)  {  // current has been invalidated, stop running it...
        //dynarec_log(LOG_DEBUG, "--- Current invalidated while linking.\n");
        return arm_epilog;
    }
    if(table[3]) {
        //dynarec_log(LOG_DEBUG, "--- Remove mark from %p (%p)\n", current, table[3]);
        RemoveMark(table);
    }
    if(!block) {
        // no block, don't try again, ever
        tableupdate(arm_epilog, addr, table);
        return arm_epilog;
    }
    if(!block->done) {
        // not finished yet... leave linker
        //tableupdate(arm_linker, addr, table);
        return arm_epilog;
    }
    if(!block->block) {
        // null block, but done: go to epilog, no linker here
        tableupdate(arm_epilog, addr, table);
        return arm_epilog;
    }
    dynablock_t *father = block->father?block->father:block;
    if(!block->parent->nolinker || (current && father->marks)) {
        //dynarec_log(LOG_DEBUG, "--- Linking %p/%p to %p (table=%p[%p/%p/%p/%p])\n", block, block->block, current, table, table[0], table[1], table[2], table[3]);
        // only update block if linker is allowed or if marks is possibe
        if(current && father->marks && current!=father)
            AddMark(current, father, table);
        tableupdate(block->block, addr, table);
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
        dynablock_t* current = NULL;
        while(!emu->quit) {
            block = DBGetBlock(emu, R_EIP, 1, &current);
            current = (block && !block->parent->nolinker)?block:NULL;
            if(!block || !block->block || !block->done) {
                // no block, of block doesn't have DynaRec content (yet, temp is not null)
                // Use interpreter (should use single instruction step...)
                dynarec_log(LOG_DEBUG, "Calling Interpretor @%p, emu=%p\n", (void*)R_EIP, emu);
                Run(emu, 1);
            } else {
                dynarec_log(LOG_DEBUG, "Calling DynaRec Block @%p (%p) of %d x86 instructions (nolinker=%d, father=%p) emu=%p\n", (void*)R_EIP, block->block, block->isize ,block->parent->nolinker, block->father, emu);
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
        dynablock_t* current = NULL;
        while(!emu->quit) {
            block = DBGetBlock(emu, R_EIP, 1, &current);
            current = (block && !block->parent->nolinker)?block:NULL;
            if(!block || !block->block || !block->done) {
                // no block, of block doesn't have DynaRec content (yet, temp is not null)
                // Use interpreter (should use single instruction step...)
                dynarec_log(LOG_DEBUG, "Running Interpretor @%p, emu=%p\n", (void*)R_EIP, emu);
                Run(emu, 1);
            } else {
                dynarec_log(LOG_DEBUG, "Running DynaRec Block @%p (%p) of %d x86 insts (nolinker=%d, father=%p) emu=%p\n", (void*)R_EIP, block->block, block->isize, block->parent->nolinker, block->father, emu);
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