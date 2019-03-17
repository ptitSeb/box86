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
    // execute atforks prepare functions, in reverse order
    for (int i=e->context->atfork_sz-1; i>=0; --i)
        EmuCall(e, e->context->atforks[i].prepare);
    x86emu_t *emu = e;
    // lets duplicate the emu
    void* newstack = 0;
    if(posix_memalign(&newstack, emu->context->stackalign, emu->context->stacksz)) {
        printf_log(LOG_NONE, "Warning, posix_memalign failed, using regular malloc...\n");
        newstack = malloc(emu->context->stacksz);
    }
    memcpy(newstack, emu->context->stack, emu->context->stacksz);
    x86emu_t* newemu = NewX86Emu(emu->context, R_EIP, (uintptr_t)newstack, emu->context->stacksz, 1);
    SetupX86Emu(newemu, emu->shared_global, emu->globals);
    CloneEmu(newemu, emu);
    // ready to fork
    ++emu->context->forked;
    int v = fork();
    if(v==EAGAIN || v==ENOMEM) {
        --emu->context->forked;
        FreeX86Emu(&newemu);    // fork failed, free the new emu
    } else if(!v) {  
        // execute atforks parent functions
        for (int i=0; i<emu->context->atfork_sz; --i)
            EmuCall(emu, emu->context->atforks[i].parent);

    } else if(v==0) {
        emu = newemu;
        // execute atforks child functions
        for (int i=0; i<emu->context->atfork_sz; --i)
            EmuCall(emu, emu->context->atforks[i].child);
    }
    R_EAX = v;
    return emu;
}

extern int errno;
void x86Int3(x86emu_t* emu)
{
    if(Peek(emu, 0)=='S' && Peek(emu, 1)=='C') // Signature for "Out of x86 door"
    {
        R_EIP += 2;
        uint32_t addr = Fetch32(emu);
        if(addr==0) {
            //printf_log(LOG_INFO, "%p:Exit x86 emu\n", *(void**)(R_ESP));
            emu->quit=1; // normal quit
        } else {
            RESET_FLAGS(emu);
            wrapper_t w = (wrapper_t)addr;
            int tid = syscall(SYS_gettid);
            addr = Fetch32(emu);
            if(box86_log>=LOG_DEBUG /*&& emu->trace_end==0 && !emu->context->x86trace*/) {
                char buff[256] = "\0";
                char buff2[64]= "\0";
                char buff3[64]= "\0";
                int post = 0;
                int perr = 0;
                uint32_t *pu32;
                const char *s = GetNativeName((void*)addr);
                if(addr==(uintptr_t)PltResolver) {
                    snprintf(buff, 256, "%s", " ... ");
                } else
                if(strstr(s, "my_SDL_RWFromFile")==s || strstr(s, "my2_SDL_RWFromFile")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%s, %s)", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(char**)(R_ESP+8));
                } else  if(strstr(s, "glColor4f")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f, %f, %f)", tid, *(void**)(R_ESP), "glColor4f", *(float*)(R_ESP+4), *(float*)(R_ESP+8), *(float*)(R_ESP+12), *(float*)(R_ESP+16));
                } else  if(strstr(s, "glTexCoord2f")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f)", tid, *(void**)(R_ESP), "glTexCoord2f", *(float*)(R_ESP+4), *(float*)(R_ESP+8));
                } else  if(strstr(s, "glVertex2f")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f)", tid, *(void**)(R_ESP), "glVertex2f", *(float*)(R_ESP+4), *(float*)(R_ESP+8));
                } else  if(strstr(s, "glVertex3f")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f, %f)", tid, *(void**)(R_ESP), "glVertex3f", *(float*)(R_ESP+4), *(float*)(R_ESP+8), *(float*)(R_ESP+12));
                } else  if(strstr(s, "__open64")==s || strstr(s, "open64")==s || strstr(s, "my_open64")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", %d)", tid, *(void**)(R_ESP), "open64", *(char**)(R_ESP+4), *(int*)(R_ESP+8));
                    perr = 1;
                } else  if(strstr(s, "__open")==s || strstr(s, "open")==s || strstr(s, "my_open")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", %d)", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(int*)(R_ESP+8));
                    perr = 1;
                } else  if(strstr(s, "fopen")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\")", tid, *(void**)(R_ESP), "fopen", *(char**)(R_ESP+4), *(char**)(R_ESP+8));
                    perr = 1;
                } else  if(strstr(s, "chdir")==s) {
                    pu32=*(uint32_t**)(R_ESP+4);
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\")", tid, *(void**)(R_ESP), "chdir", pu32?((pu32==(uint32_t*)1)?"/1/":(char*)pu32):"/0/");
                } else  if(strstr(s, "getenv")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\")", tid, *(void**)(R_ESP), "getenv", *(char**)(R_ESP+4));
                    post = 2;
                } else  if(strstr(s, "pread")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p, %u, %i)", tid, *(void**)(R_ESP), "pread", *(int32_t*)(R_ESP+4), *(void**)(R_ESP+8), *(uint32_t*)(R_ESP+12), *(int32_t*)(R_ESP+16));
                } else  if(strstr(s, "statvfs64")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p(\"%s\"), %p)", tid, *(void**)(R_ESP), "statvfs64", *(void**)(R_ESP+4), *(char**)(R_ESP+4), *(void**)(R_ESP+8));
                } else  if(strstr(s, "index")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p(\"%s\"), %i(%c))", tid, *(void**)(R_ESP), "index", *(char**)(R_ESP+4), *(char**)(R_ESP+4), *(int32_t*)(R_ESP+8), *(int32_t*)(R_ESP+8));
                } else  if(strstr(s, "rindex")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p(\"%s\"), %i(%c))", tid, *(void**)(R_ESP), "rindex", *(char**)(R_ESP+4), *(char**)(R_ESP+4), *(int32_t*)(R_ESP+8), *(int32_t*)(R_ESP+8));
                } else  if(strstr(s, "my___xstat64")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p(\"%s\"), %p)", tid, *(void**)(R_ESP), "__xstat64", *(int32_t*)(R_ESP+4), *(char**)(R_ESP+8), *(char**)(R_ESP+8), *(void**)(R_ESP+12));
                } else  if(strstr(s, "my___lxstat64")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p(\"%s\"), %p)", tid, *(void**)(R_ESP), "__lxstat64", *(int32_t*)(R_ESP+4), *(char**)(R_ESP+8), *(char**)(R_ESP+8), *(void**)(R_ESP+12));
                } else  if(strstr(s, "sem_timedwait")==s) {
                    pu32 = *(uint32_t**)(R_ESP+8);
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %p[%d sec %d ns])", tid, *(void**)(R_ESP), "sem_timedwait", *(void**)(R_ESP+4), *(void**)(R_ESP+8), pu32?pu32[0]:-1, pu32?pu32[1]:-1);
                    perr = 1;
                } else  if(strstr(s, "clock_gettime")==s || strstr(s, "__clock_gettime")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p)", tid, *(void**)(R_ESP), "clock_gettime", *(uint32_t*)(R_ESP+4), *(void**)(R_ESP+8));
                    post = 1;
                    pu32 = *(uint32_t**)(R_ESP+8);
                } else  if(strstr(s, "strcasecmp")==s || strstr(s, "__strcasecmp")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\")", tid, *(void**)(R_ESP), "strcasecmp", *(char**)(R_ESP+4), *(char**)(R_ESP+8));
                } else  if(strstr(s, "strncasecmp")==s || strstr(s, "__strncasecmp")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\", %u)", tid, *(void**)(R_ESP), "strcasecmp", *(char**)(R_ESP+4), *(char**)(R_ESP+8), *(uint32_t*)(R_ESP+12));
                } else  if(strstr(s, "strcmp")==s || strstr(s, "__strcmp")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\")", tid, *(void**)(R_ESP), "strcmp", *(char**)(R_ESP+4), *(char**)(R_ESP+8));
                } else  if(strstr(s, "strstr")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\")", tid, *(void**)(R_ESP), "strstr", *(char**)(R_ESP+4), *(char**)(R_ESP+8));
                } else  if(strstr(s, "strlen")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p(\"%s\"))", tid, *(void**)(R_ESP), "strlen", *(char**)(R_ESP+4), ((R_ESP+4))?(*(char**)(R_ESP+4)):"nil");
                } else  if(strstr(s, "my_vsnprintf")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%08X, %u, %08X...)", tid, *(void**)(R_ESP), "vsnprintf", *(uint32_t*)(R_ESP+4), *(uint32_t*)(R_ESP+8), *(uint32_t*)(R_ESP+12));
                    pu32 = *(uint32_t**)(R_ESP+4);
                    post = 3;
                } else {
                    snprintf(buff, 255, "%04d|%p: Calling %s (%08X, %08X, %08X...)", tid, *(void**)(R_ESP), s, *(uint32_t*)(R_ESP+4), *(uint32_t*)(R_ESP+8), *(uint32_t*)(R_ESP+12));
                }
                printf_log(LOG_DEBUG, "%s =>", buff);
                w(emu, addr);
                if(post)
                    switch(post) {
                    case 1: snprintf(buff2, 63, " [%d sec %d nsec]", pu32?pu32[0]:-1, pu32?pu32[1]:-1);
                            break;
                    case 2: snprintf(buff2, 63, "(%s)", R_EAX?((char*)R_EAX):"nil");
                            break;
                    case 3: snprintf(buff2, 63, "(%s)", pu32?((char*)pu32):"nil");
                            break;
                    
                }
                if(perr && errno)
                    snprintf(buff3, 63, " (errno=%d)", errno);
                printf_log(LOG_DEBUG, " return 0x%08X%s%s\n", R_EAX, buff2, buff3);
            } else
                w(emu, addr);
        }
        return;
    }
    printf_log(LOG_NONE, "Unsupported Int 3 call\n");
    emu->quit = 1;
}