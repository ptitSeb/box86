#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
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

#define INIT        dyn->arm_size = 0
#define FINI        if(ninst) {dyn->insts[ninst].address = (dyn->insts[ninst-1].address+dyn->insts[ninst-1].size);}

#define MESSAGE(A, ...)  
#define EMIT(A)     dyn->insts[ninst].size+=4; dyn->arm_size+=4
#define NEW_INST    if(ninst) {dyn->insts[ninst].address = (dyn->insts[ninst-1].address+dyn->insts[ninst-1].size);}
#define INST_NAME(name) 
#define DEFAULT         

#include "dynarec_arm_pass.h"
