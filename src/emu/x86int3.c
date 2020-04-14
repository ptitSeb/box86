#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <pthread.h>

#include "debug.h"
#include "box86stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x87emu_private.h"
#include "x86primop.h"
#include "x86trace.h"
#include "wrapper.h"
#include "box86context.h"
#include "librarian.h"

typedef int32_t (*iFpppp_t)(void*, void*, void*, void*);

x86emu_t* x86emu_fork(x86emu_t* emu, int forktype)
{
    // execute atforks prepare functions, in reverse order
    for (int i=emu->context->atfork_sz-1; i>=0; --i)
        EmuCall(emu, emu->context->atforks[i].prepare);
    int v;
    if(forktype==2) {
        iFpppp_t forkpty = (iFpppp_t)emu->forkpty_info->f;
        v = forkpty(emu->forkpty_info->amaster, emu->forkpty_info->name, emu->forkpty_info->termp, emu->forkpty_info->winp);
        emu->forkpty_info = NULL;
    } else
        v = fork();
    if(v==EAGAIN || v==ENOMEM) {
        // error...
    } else if(v!=0) {  
        // execute atforks parent functions
        for (int i=0; i<emu->context->atfork_sz; --i)
            EmuCall(emu, emu->context->atforks[i].parent);

    } else if(v==0) {
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
            //printf_log(LOG_INFO, "%p:Exit x86 emu (emu=%p)\n", *(void**)(R_ESP), emu);
            emu->quit=1; // normal quit
        } else {
            RESET_FLAGS(emu);
            wrapper_t w = (wrapper_t)addr;
            addr = Fetch32(emu);
            if(box86_log>=LOG_DEBUG /*&& emu->trace_end==0 && !emu->context->x86trace*/) {
                pthread_mutex_lock(&emu->context->mutex_trace);
                int tid = syscall(SYS_gettid);
                char buff[256] = "\0";
                char buff2[64] = "\0";
                char buff3[64] = "\0";
                char *tmp;
                int post = 0;
                int perr = 0;
                uint32_t *pu32 = NULL;
                const char *s = GetNativeName(emu, (void*)addr);
                if(addr==(uintptr_t)PltResolver) {
                    snprintf(buff, 256, "%s", " ... ");
                } else
                if(strstr(s, "SDL_RWFromFile")==s || strstr(s, "SDL_RWFromFile")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%s, %s)", tid, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(char**)(R_ESP+8));
                } else  if(strstr(s, "glColor4f")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f, %f, %f)", tid, *(void**)(R_ESP), "glColor4f", *(float*)(R_ESP+4), *(float*)(R_ESP+8), *(float*)(R_ESP+12), *(float*)(R_ESP+16));
                } else  if(strstr(s, "glTexCoord2f")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f)", tid, *(void**)(R_ESP), "glTexCoord2f", *(float*)(R_ESP+4), *(float*)(R_ESP+8));
                } else  if(strstr(s, "glVertex2f")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f)", tid, *(void**)(R_ESP), "glVertex2f", *(float*)(R_ESP+4), *(float*)(R_ESP+8));
                } else  if(strstr(s, "glVertex3f")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f, %f)", tid, *(void**)(R_ESP), "glVertex3f", *(float*)(R_ESP+4), *(float*)(R_ESP+8), *(float*)(R_ESP+12));
                } else  if(strstr(s, "__open64")==s || strstr(s, "open64")==s || strstr(s, "open64")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", %d, %d)", tid, *(void**)(R_ESP), "open64", *(char**)(R_ESP+4), *(int*)(R_ESP+8), *(int*)(R_ESP+12));
                    perr = 1;
                } else  if(strstr(s, "__open")==s || strstr(s, "open")==s || strstr(s, "open")==s) {
                    tmp = *(char**)(R_ESP+4);
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", %d)", tid, *(void**)(R_ESP), s, (tmp)?tmp:"(nil)", *(int*)(R_ESP+8));
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
                } else  if(strstr(s, "__xstat64")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p(\"%s\"), %p)", tid, *(void**)(R_ESP), "__xstat64", *(int32_t*)(R_ESP+4), *(char**)(R_ESP+8), *(char**)(R_ESP+8), *(void**)(R_ESP+12));
                    perr = 1;
                } else  if(strstr(s, "__lxstat64")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%d, %p(\"%s\"), %p)", tid, *(void**)(R_ESP), "__lxstat64", *(int32_t*)(R_ESP+4), *(char**)(R_ESP+8), *(char**)(R_ESP+8), *(void**)(R_ESP+12));
                    perr = 1;
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
                } else  if(strstr(s, "gtk_signal_connect_full")) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\", %p, %p, %p, %p, %d, %d)", tid, *(void**)(R_ESP), "gtk_signal_connect_full", *(void**)(R_ESP+4), *(char**)(R_ESP+8), *(void**)(R_ESP+12), *(void**)(R_ESP+16), *(void**)(R_ESP+20), *(void**)(R_ESP+24), *(int32_t*)(R_ESP+28), *(int32_t*)(R_ESP+32));
                } else  if(strstr(s, "strncasecmp")==s || strstr(s, "__strncasecmp")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\", %u)", tid, *(void**)(R_ESP), "strcasecmp", *(char**)(R_ESP+4), *(char**)(R_ESP+8), *(uint32_t*)(R_ESP+12));
                } else  if(strstr(s, "strcmp")==s || strstr(s, "__strcmp")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\")", tid, *(void**)(R_ESP), "strcmp", *(char**)(R_ESP+4), *(char**)(R_ESP+8));
                } else  if(strstr(s, "strstr")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%.127s\", \"%.127s\")", tid, *(void**)(R_ESP), "strstr", *(char**)(R_ESP+4), *(char**)(R_ESP+8));
                } else  if(strstr(s, "strlen")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p(\"%s\"))", tid, *(void**)(R_ESP), "strlen", *(char**)(R_ESP+4), ((R_ESP+4))?(*(char**)(R_ESP+4)):"nil");
                } else  if(strstr(s, "vsnprintf")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%08X, %u, %08X...)", tid, *(void**)(R_ESP), "vsnprintf", *(uint32_t*)(R_ESP+4), *(uint32_t*)(R_ESP+8), *(uint32_t*)(R_ESP+12));
                    pu32 = *(uint32_t**)(R_ESP+4);
                    post = 3;
                } else  if(strstr(s, "vsprintf")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%08X, \"%s\"...)", tid, *(void**)(R_ESP), "vsprintf", *(uint32_t*)(R_ESP+4), *(char**)(R_ESP+8));
                    pu32 = *(uint32_t**)(R_ESP+4);
                    post = 3;
                } else  if(strstr(s, "snprintf")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%08X, %u, %08X...)", tid, *(void**)(R_ESP), "snprintf", *(uint32_t*)(R_ESP+4), *(uint32_t*)(R_ESP+8), *(uint32_t*)(R_ESP+12));
                    pu32 = *(uint32_t**)(R_ESP+4);
                    post = 3;
                } else  if(strstr(s, "sprintf")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%08X, %08X...)", tid, *(void**)(R_ESP), "sprintf", *(uint32_t*)(R_ESP+4), *(uint32_t*)(R_ESP+8));
                    pu32 = *(uint32_t**)(R_ESP+4);
                    post = 3;
                } else  if(strstr(s, "fprintf")==s) {
                    pu32 = *(uint32_t**)(R_ESP+8);
                    if(((uintptr_t)pu32)<0x5) // probably a __fprint_chk
                        pu32 = *(uint32_t**)(R_ESP+12);
                    snprintf(buff, 255, "%04d|%p: Calling %s(%08X, \"%s\", ...)", tid, *(void**)(R_ESP), "fprintf", *(uint32_t*)(R_ESP+4), pu32?((char*)(pu32)):"nil");
                } else  if(strstr(s, "sscanf")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(\"%s\", \"%s\", ...)", tid, *(void**)(R_ESP), "sscanf", *(char**)(R_ESP+4), *(char**)(R_ESP+8));
                } else if(strstr(s, "XCreateWindow")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, %p, %d, %d, %u, %u, %u, %d, %u, %p, %u, %p)", tid, *(void**)(R_ESP), "XCreateWindow", *(void**)(R_ESP+4), *(void**)(R_ESP+8), *(int*)(R_ESP+12), *(int*)(R_ESP+16), *(uint32_t*)(R_ESP+20), *(uint32_t*)(R_ESP+24), *(uint32_t*)(R_ESP+28), *(int32_t*)(R_ESP+32), *(uint32_t*)(R_ESP+36), *(void**)(R_ESP+40), *(uint32_t*)(R_ESP+44), *(void**)(R_ESP+48));
                } else if(strstr(s, "XLoadQueryFont")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\")", tid, *(void**)(R_ESP), "XLoadQueryFont", *(void**)(R_ESP+4), *(char**)(R_ESP+8));
                } else if(strstr(s, "pthread_mutex_lock")==s) {
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p)", tid, *(void**)(R_ESP), "pthread_mutex_lock", *(void**)(R_ESP+4));
                } else if(strstr(s, "fmodf")==s) {
                    post = 4;
                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f)", tid, *(void**)(R_ESP), "fmodf", *(float*)(R_ESP+4), *(float*)(R_ESP+8));
                } else if(strstr(s, "fmod")==s) {
                    post = 4;
                    snprintf(buff, 255, "%04d|%p: Calling %s(%f, %f)", tid, *(void**)(R_ESP), "fmod", *(double*)(R_ESP+4), *(double*)(R_ESP+12));
                } else if(strstr(s, "SDL_GetWindowSurface")==s) {
                    post = 5;
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p)", tid, *(void**)(R_ESP), "SDL_GetWindowSurface", *(void**)(R_ESP+4));
                } else if(strstr(s, "udev_monitor_new_from_netlink")==s) {
                    post = 5;
                    snprintf(buff, 255, "%04d|%p: Calling %s(%p, \"%s\")", tid, *(void**)(R_ESP), "udev_monitor_new_from_netlink", *(void**)(R_ESP+4), *(char**)(R_ESP+8));
                } else {
                    snprintf(buff, 255, "%04d|%p: Calling %s (%08X, %08X, %08X...)", tid, *(void**)(R_ESP), s, *(uint32_t*)(R_ESP+4), *(uint32_t*)(R_ESP+8), *(uint32_t*)(R_ESP+12));
                }
                printf_log(LOG_DEBUG, "%s =>", buff);
                pthread_mutex_unlock(&emu->context->mutex_trace);
                w(emu, addr);   // some function never come back, so unlock the mutex first!
                pthread_mutex_lock(&emu->context->mutex_trace);
                if(post)
                    switch(post) {
                    case 1: snprintf(buff2, 63, " [%d sec %d nsec]", pu32?pu32[0]:-1, pu32?pu32[1]:-1);
                            break;
                    case 2: snprintf(buff2, 63, "(%s)", R_EAX?((char*)R_EAX):"nil");
                            break;
                    case 3: snprintf(buff2, 63, "(%s)", pu32?((char*)pu32):"nil");
                            break;
                    #ifdef USE_FLOAT
                    case 4: snprintf(buff2, 63, " (%f)", ST0.f);
                    #else
                    case 4: snprintf(buff2, 63, " (%f)", ST0.d);
                    #endif
                            break;
                    case 5: {
                            uint32_t* p = (uint32_t*)R_EAX;
                            if(p)
                                snprintf(buff2, 63, " size=%dx%d, pitch=%d, pixels=%p", p[2], p[3], p[4], p+5);
                            else
                                snprintf(buff2, 63, "NULL Surface");
                            }
                            break;
                }
                if(perr && ((int)R_EAX)<0)
                    snprintf(buff3, 63, " (errno=%d)", errno);
                printf_log(LOG_DEBUG, " return 0x%08X%s%s\n", R_EAX, buff2, buff3);
                pthread_mutex_unlock(&emu->context->mutex_trace);
            } else
                w(emu, addr);
        }
        return;
    }
    printf_log(LOG_INFO, "Warning, ignoring unsupported Int 3 call\n");
    //emu->quit = 1;
}
