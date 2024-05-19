#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <setjmp.h>

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
#include "threads.h"
#ifdef DYNAREC
#include "dynablock.h"
#include "dynablock_private.h"
#include "bridge.h"
#include "custommem.h"
void x86test_check(x86emu_t* ref, uintptr_t ip);
#endif

#ifdef ARM
void arm_prolog(x86emu_t* emu, void* addr) EXPORTDYN;
void arm_epilog() EXPORTDYN;
void arm_epilog_fast() EXPORTDYN;
#endif

#ifdef DYNAREC
#ifdef HAVE_TRACE
#include "elfloader.h"
uintptr_t getX86Address(dynablock_t* db, uintptr_t arm_addr);
#endif
void* LinkNext(x86emu_t* emu, uintptr_t addr, void* x2)
{
    #ifdef HAVE_TRACE
    if(!addr) {
        x2-=8;  // actual PC is 2 instructions ahead
        dynablock_t* db = FindDynablockFromNativeAddress(x2);
        void* x86addr = db?(void*)getX86Address(db, (uintptr_t)x2):NULL;
        const char* pcname = getAddrFunctionName((uintptr_t)x86addr);
        printf_log(LOG_NONE, "Warning, jumping to NULL address from %p (db=%p, x86addr=%p/%s)\n", x2, db, x86addr, pcname);
    }
    #endif
    void * jblock;
    dynablock_t* block = NULL;
    if(hasAlternate((void*)addr)) {
        printf_log(LOG_INFO, "Jmp address has alternate: %p", (void*)addr);
        addr = (uintptr_t)getAlternate((void*)addr);
        R_EIP = addr;
        printf_log(LOG_INFO, " -> %p\n", (void*)addr);
        block = DBGetBlock(emu, addr, 1);
    } else
        block = DBGetBlock(emu, addr, 1);
    if(!block) {
        // no block, let link table as is...
        #ifdef HAVE_TRACE
        if(LOG_INFO<=box86_dynarec_log) {
            if(checkInHotPage(addr)) {
                dynarec_log(LOG_INFO, "Not trying to run a block from a Hotpage at %p\n", (void*)addr);
            } else {
                dynablock_t* db = FindDynablockFromNativeAddress(x2-4);
                elfheader_t* h = FindElfAddress(my_context, (uintptr_t)x2-4);
                dynarec_log(LOG_INFO, "Warning, jumping to a no-block address %p from %p (db=%p, x86addr=%p(elf=%s))\n", (void*)addr, x2-4, db, db?(void*)getX86Address(db, (uintptr_t)x2-4):NULL, h?ElfName(h):"(none)");
            }
        }
        #endif
        //tableupdate(arm_epilog, addr, table);
        return arm_epilog;
    }
    if(!block->done) {
        // not finished yet... leave linker
        //tableupdate(arm_linker, addr, table);
        return arm_epilog;
    }
    if(!(jblock=block->block)) {
        // null block, but done: go to epilog, no linker here
        return arm_epilog;
    }
    //dynablock_t *father = block->father?block->father:block;
    return jblock;
}
#endif

#ifdef ANDROID
#define JUMPBUFF sigjmp_buf
#else
#define JUMPBUFF struct __jmp_buf_tag
#endif

void DynaCall(x86emu_t* emu, uintptr_t addr)
{
    uint32_t old_esp = R_ESP;
    uint32_t old_ebx = R_EBX;
    uint32_t old_edi = R_EDI;
    uint32_t old_esi = R_ESI;
    uint32_t old_ebp = R_EBP;
    uint32_t old_eip = R_EIP;
    // save defered flags
    defered_flags_t old_df = emu->df;
    uint32_t old_op1 = emu->op1;
    uint32_t old_op2 = emu->op2;
    uint32_t old_res = emu->res;
    // uc_link
    i386_ucontext_t* old_uc_link = emu->uc_link;
    emu->uc_link = NULL;

    PushExit(emu);
    R_EIP = addr;
    emu->df = d_none;
    DynaRun(emu);
    emu->quit = 0;  // reset Quit flags...
    emu->df = d_none;
    emu->uc_link = old_uc_link;
    if(emu->flags.quitonlongjmp && emu->flags.longjmp) {
        if(emu->flags.quitonlongjmp==1)
            emu->flags.longjmp = 0;   // don't change anything because of the longjmp
    } else {
        // restore defered flags
        emu->df = old_df;
        emu->op1 = old_op1;
        emu->op2 = old_op2;
        emu->res = old_res;
        // and the old registers
        R_EBX = old_ebx;
        R_EDI = old_edi;
        R_ESI = old_esi;
        R_EBP = old_ebp;
        R_ESP = old_esp;
        R_EIP = old_eip;  // and set back instruction pointer
    }
}

int my_setcontext(x86emu_t* emu, void* ucp);
int DynaRun(x86emu_t* emu)
{
    // prepare setjump for signal handling
    JUMPBUFF jmpbuf[1] = {0};
    int skip = 0;
    JUMPBUFF *old_jmpbuf = emu->jmpbuf;
    uintptr_t old_savesp = emu->xSPSave;
    emu->flags.jmpbuf_ready = 0;

    while(!(emu->quit)) {
        if(!emu->jmpbuf || (emu->flags.need_jmpbuf && emu->jmpbuf!=jmpbuf)) {
            emu->jmpbuf = jmpbuf;
            emu->old_savedsp = emu->xSPSave;
            emu->flags.jmpbuf_ready = 1;
            #ifdef ANDROID
            if((skip=sigsetjmp(*(JUMPBUFF*)emu->jmpbuf, 1))) 
            #else
            if((skip=sigsetjmp(emu->jmpbuf, 1))) 
            #endif
            {
                printf_log(LOG_DEBUG, "Setjmp DynaRun\n");
                #ifdef DYNAREC
                if(box86_dynarec_test) {
                    if(emu->test.clean)
                        x86test_check(emu, R_EIP);
                    emu->test.clean = 0;
                }
                #endif
            }
        }
        if(emu->flags.need_jmpbuf)
            emu->flags.need_jmpbuf = 0;

#ifndef DYNAREC
        Run(emu, 0);
#else
        if(!box86_dynarec)
            Run(emu, 0);
        else {
            dynablock_t* block = (skip || ACCESS_FLAG(F_TF))?NULL:DBGetBlock(emu, R_EIP, 1);
            if(!block || !block->block || !block->done) {
                skip = 0;
                // no block, of block doesn't have DynaRec content (yet, temp is not null)
                // Use interpreter (should use single instruction step...)
                dynarec_log(LOG_DEBUG, "%04d|Running Interpreter @%p, emu=%p\n", GetTID(), (void*)R_EIP, emu);
                if(box86_dynarec_test)
                    emu->test.clean = 0;
                Run(emu, 1);
            } else {
                dynarec_log(LOG_DEBUG, "%04d|Running DynaRec Block @%p (%p) of %d x86 insts (hash=0x%x) emu=%p\n", GetTID(), (void*)R_EIP, block->block, block->isize, block->hash, emu);
                // block is here, let's run it!
                arm_prolog(emu, block->block);
            }
            if(emu->fork) {
                int forktype = emu->fork;
                emu->quit = 0;
                emu->fork = 0;
                emu = x86emu_fork(emu, forktype);
            }
            if(emu->quit && emu->uc_link) {
                emu->quit = 0;
                my_setcontext(emu, emu->uc_link);
            }
        }
#endif
        if(emu->flags.need_jmpbuf)
            emu->quit = 0;
    }
    // clear the setjmp
    emu->jmpbuf = old_jmpbuf;
    emu->xSPSave = old_savesp;
}