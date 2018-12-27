#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "x86emu_private.h"
#include "x87emu_private.h"

void fpu_do_push(x86emu_t* emu)
{
    ++emu->fpu_stack;
    if(emu->fpu_stack == 8) {// overflow
        printf_log(LOG_NONE, "Error: FPU Stack overflow\n");    // probably better to raise something
        emu->quit = 1;
        return;
    }
    emu->top = (emu->top-1)&7;
}

void fpu_do_pop(x86emu_t* emu)
{
    emu->top = (emu->top+1)&7;
    --emu->fpu_stack;
    if(emu->fpu_stack < 0) {// underflow
        printf_log(LOG_NONE, "Error: FPU Stack uderflow\n");    // probably better to raise something
        emu->quit = 1;
        return;
    }

}