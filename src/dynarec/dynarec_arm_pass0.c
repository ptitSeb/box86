#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>

#include "debug.h"
#include "box86context.h"
#include "dynarec.h"
#include "emu/x86emu_private.h"
#include "emu/x86run_private.h"
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

#define STEP        0
#define NAME_STEP   arm_pass0

#define INIT    uintptr_t sav_addr=addr
#define FINI    dyn->isize = addr-sav_addr
#define MESSAGE(A, ...)  
#define EMIT(A)     
#define UFLAGS          {}
#define NEW_INST        ++dyn->size
#define INST_NAME(name) 
#define DEFAULT         \
        --dyn->size;    \
        dynarec_log(LOG_INFO, "%p: Dynarec stopped because of Opcode %02X %02X %02X %02X %02X %02X %02X %02X\n", \
        addr-1, opcode,                 \
        PK(0), PK(1), PK(2), PK(3),     \
        PK(4), PK(5), PK(6))

#include "dynarec_arm_pass.h"
