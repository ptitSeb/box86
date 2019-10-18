#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

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
#include "emu/x87emu_private.h"
#include "x86trace.h"
#include "dynablock.h"
#include "dynablock_private.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"
#include "dynarec_arm_functions.h"

void arm_popf(x86emu_t* emu, uint32_t f)
{
    emu->packed_eflags.x32 = ((f & 0x3F7FD7) & (0xffff-40) ) | 0x2; // mask off res2 and res3 and on res1
    UnpackFlags(emu);
    RESET_FLAGS(emu);
}

void arm_fstp(x86emu_t* emu, void* p)
{
    if(ST0.ll!=STld(0).ref)
        D2LD(&ST0.d, p);
    else
        memcpy(p, &STld(0).ld, 10);
}

void arm_print_armreg(x86emu_t* emu, uintptr_t reg, uintptr_t n)
{
    dynarec_log(LOG_DEBUG, "R%d=0x%x (%d)\n", n, reg, reg);
}