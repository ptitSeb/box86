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
#include "dynablock.h"
#include "dynablock_private.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"

#define STEP        1
#define NAME_STEP   arm_pass1

#define NEW_INST \
    dyn->insts[ninst].x86.addr = ip; \
    if(ninst) dyn->insts[ninst-1].x86.size = dyn->insts[ninst].x86.addr - dyn->insts[ninst-1].x86.addr;
#define INST_NAME(name) 
#define DEFAULT         

#include "dynarec_arm_pass.h"
