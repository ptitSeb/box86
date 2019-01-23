#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"
#include "wrapper.h"
#include "box86context.h"

x86emu_t* x86emu_fork(x86emu_t* e)
{
    x86emu_t *emu = e;
    // lets duplicate the emu
    void* newstack = 0;
    posix_memalign(&newstack, emu->context->stackalign, emu->context->stacksz);
    memcpy(newstack, emu->context->stack, emu->context->stacksz);
    x86emu_t* newemu = NewX86Emu(emu->context, R_EIP, (uintptr_t)newstack, emu->context->stacksz);
    SetupX86Emu(newemu, emu->shared_global, emu->globals);
    CloneEmu(newemu, emu);
    emu->stack = newstack;
    // ready to fork
    ++emu->context->forked;
    int v = fork();
    if(!v) {  
        emu = newemu;
    }
    if(v==EAGAIN || v==ENOMEM) {
        --emu->context->forked;
        FreeX86Emu(&newemu);    // fork failed, free the new emu
    }
    R_EAX = v;
    return emu;
}

void x86Int3(x86emu_t* emu)
{
    if(Peek(emu, 0)=='S' && Peek(emu, 1)=='C') // Signature for "Out of x86 door"
    {
        R_EIP += 2;
        uint32_t addr = Fetch32(emu);
        if(addr==0) {
            //printf_log(LOG_INFO, "Exit\n");
            emu->quit=1; // normal quit
        } else {
            wrapper_t w = (wrapper_t)addr;
            int tid = syscall(SYS_gettid);
            addr = Fetch32(emu);
            if(box86_log>=LOG_DEBUG /*&& emu->trace_end==0 && !emu->context->x86trace*/) {
                const char *s = GetNativeName((void*)addr);
                if(addr==(uintptr_t)PltResolver) {
                    printf(" ... ");
                } else
                if(strstr(s, "SDL_RWFromFile")) {
                    printf_log(LOG_INFO, "%04d|%p: Calling %s(%s, %s)", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(char**)(R_ESP+8));
                } else  if(strstr(s, "glColor4f")) {
                    printf_log(LOG_INFO, "%04d|%p: Calling %s(%f, %f, %f, %f)", tid, *(void**)(R_ESP), "glColor4f", *(float*)(R_ESP+4), *(float*)(R_ESP+8), *(float*)(R_ESP+12), *(float*)(R_ESP+16));
                } else  if(strstr(s, "glTexCoord2f")) {
                    printf_log(LOG_INFO, "%04d|%p: Calling %s(%f, %f)", tid, *(void**)(R_ESP), "glTexCoord2f", *(float*)(R_ESP+4), *(float*)(R_ESP+8));
                } else  if(strstr(s, "glVertex3f")) {
                    printf_log(LOG_INFO, "%04d|%p: Calling %s(%f, %f, %f)", tid, *(void**)(R_ESP), "glVertex3f", *(float*)(R_ESP+4), *(float*)(R_ESP+8), *(float*)(R_ESP+12));
                } else  if(strstr(s, "__open")==s) {
                    printf_log(LOG_INFO, "%04d|%p: Calling %s(\"%s\", %d)", tid, *(void**)(R_ESP), "open", *(char**)(R_ESP+4), *(int*)(R_ESP+8));
                } else  if(strstr(s, "fopen")==s) {
                    printf_log(LOG_INFO, "%04d|%p: Calling %s(\"%s\", %s)", tid, *(void**)(R_ESP), "fopen", *(char**)(R_ESP+4), *(char**)(R_ESP+8));
                } else {
                    printf_log(LOG_INFO, "%04d|%p: Calling %s (%08X, %08X, %08X...)", tid, *(void**)(R_ESP), s, *(uint32_t*)(R_ESP+4), *(uint32_t*)(R_ESP+8), *(uint32_t*)(R_ESP+12));
                }
                fflush(stdout);
            }
            w(emu, addr);
            printf_log(LOG_DEBUG, " => return 0x%08X\n", R_EAX);
        }
        return;
    }
    printf_log(LOG_NONE, "Unsupported Int 3 call\n");
    emu->quit = 1;
}