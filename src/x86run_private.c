#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86run_private.h"
#include "x86emu_private.h"
#include "box86context.h"


int32_t EXPORT my___libc_start_main(x86emu_t* emu, int *(main) (int, char * *, char * *), int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), void (*rtld_fini) (void), void (* stack_end))
{
    //TODO: register rtld_fini
    //TODO: register fini
    if(init) {
        PushExit(emu);
        R_EIP=(uint32_t)*init;
        printf_log(LOG_DEBUG, "Calling init(%p) from __libc_start_main\n", *init);
        Run(emu);
        if(emu->error)  // any error, don't bother with more
            return 0;
        emu->quit = 0;
    }
    // let's cheat and set all args...
    // call main and finish
    Push(emu, (uint32_t)emu->context->envv);
    Push(emu, (uint32_t)emu->context->argv);
    Push(emu, (uint32_t)emu->context->argc);
    PushExit(emu);
    R_EIP=(uint32_t)main;
    printf_log(LOG_DEBUG, "Calling main(=>%p) from __libc_start_main\n", main);
}

const char* GetNativeName(void* p)
{
    static char buff[500] = {0};
    Dl_info info;
    if(dladdr(p, &info)==0)
        strcpy(buff, "???");
    else {
        if(info.dli_sname) {
            strcpy(buff, info.dli_sname);
            if(info.dli_fname) {
                strcat(buff, " ("); strcat(buff, info.dli_fname); strcat(buff, ")");
            }
        } else
            strcpy(buff, "unknown");
    }
    return buff;
}

uintptr_t pltResolver = ~0;
void PltResolver(x86emu_t* emu, uint32_t id, uintptr_t ofs)
{
    printf("PltResolver: Ofs=%p, Id=%d (IP=%p)", (void*)ofs, id, *(void**)(R_ESP));
    emu->quit=1;
}
