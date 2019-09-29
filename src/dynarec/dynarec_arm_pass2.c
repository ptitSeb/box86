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

#define STEP        2
#define NAME_STEP   arm_pass2

#define INIT        int arm_size = 0
#define FINI                    \
    dyn->arm_size = arm_size

#define EMIT(A)     dyn->insts[ninst].size+=4; arm_size+=4
#define FLAGS(A)
#define NEW_INST    
#define INST_NAME(name) 
#define DEFAULT         

#include "dynarec_arm_pass.h"
