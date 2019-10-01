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

void arm_prolog(x86emu_t* emu, void* addr);

void DynaCall(x86emu_t* emu, uintptr_t addr)
{
#ifdef DYNAREC
    if(!box86_dynarec)
#endif
        EmuCall(emu, addr);
#ifdef DYNAREC
    else {
        uint32_t old_ebx = R_EBX;
        uint32_t old_edi = R_EDI;
        uint32_t old_esi = R_ESI;
        uint32_t old_ebp = R_EBP;
        uint32_t old_eip = R_EIP;
        PushExit(emu);
        R_EIP = addr;
        emu->df = d_none;
        while(!emu->quit) {
            volatile dynablock_t* block = DBGetBlock(emu, R_EIP, 1);
            if(!block || !block->block || !block->done) {
                // no block, of block doesn't have DynaRec content (yet, temp is not null)
                // Use interpreter (should use single instruction step...)
                dynarec_log(LOG_DEBUG, "Calling Interpretor @%p, emu=%p\n", R_EIP, emu);
                Run(emu);
            } else {
                dynarec_log(LOG_DEBUG, "Calling DynaRec Block @%p (%p) emu=%p\n", R_EIP, block->block, emu);
                // block is here, let's run it!
                arm_prolog(emu, block->block);
            }
        }
        emu->quit = 0;  // reset Quit flags...
        emu->df = d_none;
        R_EBX = old_ebx;
        R_EDI = old_edi;
        R_ESI = old_esi;
        R_EBP = old_ebp;
        R_EIP = old_eip;  // and set back instruction pointer
    }
#endif
}