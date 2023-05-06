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
    dynablock_t* block = DBGetBlock(emu, addr, 1);
    if(!block) {
        // no block, let link table as is...
        if(hasAlternate((void*)addr)) {
            printf_log(LOG_INFO, "Jmp address has alternate: %p", (void*)addr);
            addr = (uintptr_t)getAlternate((void*)addr);
            R_EIP = addr;
            printf_log(LOG_INFO, " -> %p\n", (void*)addr);
            block = DBGetBlock(emu, addr, 1);
        }
        if(!block) {
            #ifdef HAVE_TRACE
            if(LOG_INFO<=box86_dynarec_log) {
                dynablock_t* db = FindDynablockFromNativeAddress(x2);
                elfheader_t* h = FindElfAddress(my_context, (uintptr_t)x2);
                dynarec_log(LOG_INFO, "Warning, jumping to a no-block address %p from %p (db=%p, x64addr=%p(elf=%s))\n", (void*)addr, x2, db, db?(void*)getX86Address(db, (uintptr_t)x2-4):NULL, h?ElfName(h):"(none)");
            }
            #endif
            //tableupdate(arm_epilog, addr, table);
            return arm_epilog;
        }
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

void DynaCall(x86emu_t* emu, uintptr_t addr)
{
    // prepare setjump for signal handling
    emu_jmpbuf_t *ejb = NULL;
    int jmpbuf_reset = 0;
    int skip = 0;
    if(emu->type == EMUTYPE_MAIN) {
        ejb = GetJmpBuf();
        if(!ejb->jmpbuf_ok) {
            ejb->emu = emu;
            ejb->jmpbuf_ok = 1;
            jmpbuf_reset = 1;
            if((skip=sigsetjmp((struct __jmp_buf_tag*)ejb->jmpbuf, 1))) {
                printf_log(LOG_DEBUG, "Setjmp DynaCall %d, fs=0x%x\n", skip, ejb->emu->segs[_FS]);
                addr = R_EIP;   // not sure if it should still be inside DynaCall!
                #ifdef DYNAREC
                if(box86_dynarec_test) {
                    if(emu->test.clean)
                        x86test_check(emu, R_EIP);
                    emu->test.clean = 0;
                }
                #endif
                if(skip!=2)
                    skip = 0;
            }
        }
    }
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
            block = (skip==2)?NULL:DBGetBlock(emu, R_EIP, 1);
            current = block;
            if(!block || !block->block || !block->done) {
                skip = 0;
                // no block, of block doesn't have DynaRec content (yet, temp is not null)
                // Use interpreter (should use single instruction step...)
                dynarec_log(LOG_DEBUG, "%04d|Calling Interpretor @%p, emu=%p\n", GetTID(), (void*)R_EIP, emu);
                if(box86_dynarec_test)
                    emu->test.clean = 0;
                Run(emu, 1);
            } else {
                dynarec_log(LOG_DEBUG, "%04d|Calling DynaRec Block @%p (%p) of %d x86 instructions emu=%p\n", GetTID(), (void*)R_EIP, block->block, block->isize ,emu);
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
                if(emu->type == EMUTYPE_MAIN) {
                    ejb = GetJmpBuf();
                    ejb->emu = emu;
                    ejb->jmpbuf_ok = 1;
                    jmpbuf_reset = 1;
                    if(sigsetjmp((struct __jmp_buf_tag*)ejb->jmpbuf, 1)) {
                        printf_log(LOG_DEBUG, "Setjmp inner DynaCall, fs=0x%x\n", ejb->emu->segs[_FS]);
                        addr = R_EIP;
                    }
                }
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
    // clear the setjmp
    if(ejb && jmpbuf_reset)
        ejb->jmpbuf_ok = 0;
}

int my_setcontext(x86emu_t* emu, void* ucp);
int DynaRun(x86emu_t* emu)
{
    // prepare setjump for signal handling
    emu_jmpbuf_t *ejb = NULL;
    int skip = 0;
#ifdef DYNAREC
    int jmpbuf_reset = 1;
#endif
    if(emu->type == EMUTYPE_MAIN) {
        ejb = GetJmpBuf();
        if(!ejb->jmpbuf_ok) {
            ejb->emu = emu;
            ejb->jmpbuf_ok = 1;
            int a;
#ifdef DYNAREC
            jmpbuf_reset = 1;
#endif
            if((skip=sigsetjmp((struct __jmp_buf_tag*)ejb->jmpbuf, 1))) {
                printf_log(LOG_DEBUG, "Setjmp DynaRun %d, fs=0x%x\n", skip, ejb->emu->segs[_FS]);
                #ifdef DYNAREC
                if(box86_dynarec_test) {
                    if(emu->test.clean)
                        x86test_check(emu, R_EIP);
                    emu->test.clean = 0;
                }
                #endif
                if(skip!=2)
                    skip = 0;
            }
        }
    }
#ifdef DYNAREC
    if(!box86_dynarec)
#endif
        return Run(emu, 0);
#ifdef DYNAREC
    else {
        dynablock_t* block = NULL;
        dynablock_t* current = NULL;
        while(!emu->quit) {
            block = (skip==2)?NULL:DBGetBlock(emu, R_EIP, 1);
            current = block;
            if(!block || !block->block || !block->done) {
                skip = 0;
                // no block, of block doesn't have DynaRec content (yet, temp is not null)
                // Use interpreter (should use single instruction step...)
                dynarec_log(LOG_DEBUG, "%04d|Running Interpretor @%p, emu=%p\n", GetTID(), (void*)R_EIP, emu);
                if(box86_dynarec_test)
                    emu->test.clean = 0;
                Run(emu, 1);
            } else {
                dynarec_log(LOG_DEBUG, "%04d|Running DynaRec Block @%p (%p) of %d x86 insts emu=%p\n", GetTID(), (void*)R_EIP, block->block, block->isize, emu);
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
                if(emu->type == EMUTYPE_MAIN) {
                    ejb = GetJmpBuf();
                    ejb->emu = emu;
                    ejb->jmpbuf_ok = 1;
                    jmpbuf_reset = 1;
                    if(sigsetjmp((struct __jmp_buf_tag*)ejb->jmpbuf, 1))
                        printf_log(LOG_DEBUG, "Setjmp inner DynaRun, fs=0x%x\n", ejb->emu->segs[_FS]);
                }
            }
            else if(emu->quit && emu->uc_link) {
                emu->quit = 0;
                my_setcontext(emu, emu->uc_link);
            }
        }
    }
    // clear the setjmp
    if(ejb && jmpbuf_reset)
        ejb->jmpbuf_ok = 0;
    return 0;
#endif
}