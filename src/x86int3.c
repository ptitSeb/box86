#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
            addr = Fetch32(emu);
            if(box86_log>=LOG_DEBUG /*&& emu->trace_end==0 && !emu->context->x86trace*/) {
                const char *s = GetNativeName((void*)addr);
                if(strstr(s, "SDL_RWFromFile")) {
                    printf_log(LOG_INFO, "%p: (%p) Calling %s(%s, %s)", (void*)R_EIP, *(void**)(R_ESP), s, *(char**)(R_ESP+4), *(char**)(R_ESP+8));
                } else  if(strstr(s, "glColor4f")) {
                    printf_log(LOG_INFO, "%p: (%p) Calling %s(%f, %f, %f, %f)", (void*)R_EIP, *(void**)(R_ESP), "glColor4f", *(float*)(R_ESP+4), *(float*)(R_ESP+8), *(float*)(R_ESP+12), *(float*)(R_ESP+16));
/*                    if(emu->trace_end==0xfb00) {
                        emu->trace_start = (*(uintptr_t*)(R_ESP)) - 0x100;
                        emu->trace_end = emu->trace_start + 0x100;
                    }*/
                } else  if(strstr(s, "glTexCoord2f")) {
                    printf_log(LOG_INFO, "%p: (%p) Calling %s(%f, %f)", (void*)R_EIP, *(void**)(R_ESP), "glTexCoord2f", *(float*)(R_ESP+4), *(float*)(R_ESP+8));
                } else  if(strstr(s, "glVertex3f")) {
                    printf_log(LOG_INFO, "%p: (%p) Calling %s(%f, %f, %f)", (void*)R_EIP, *(void**)(R_ESP), "glVertex3f", *(float*)(R_ESP+4), *(float*)(R_ESP+8), *(float*)(R_ESP+12));
                } else {
                    printf_log(LOG_INFO, " Calling %s", s);
                }
            }
            w(emu, addr);
            printf_log(LOG_DEBUG, " => return 0x%08X\n", R_EAX);
        }
        return;
    }
    printf_log(LOG_NONE, "Unsupported Int 3 call\n");
    emu->quit = 1;
}