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

static void printf_x86_instruction(zydis_dec_t* dec, instruction_x86_t* inst, const char* name) {
    uint8_t *ip = (uint8_t*)inst->addr;
    if(ip[0]==0xcc && ip[1]=='S' && ip[2]=='C') {
        uint32_t a = *(uint32_t*)(ip+3);
        if(a==0) {
            dynarec_log(LOG_NONE, "0x%p: Exit x86emu\n", (void*)ip);
        } else {
            dynarec_log(LOG_NONE, "0x%p: Native call to %p\n", (void*)ip, (void*)a);
        }
    } else {
        if(dec) {
            dynarec_log(LOG_NONE, "%s\n", DecodeX86Trace(dec, inst->addr));
        } else {
            for(int i=0; i<inst->size; ++i)
                dynarec_log(LOG_NONE, "%02X ", ip[i]);
            dynarec_log(LOG_NONE, " %s\n", name);
        }
    }
}

#define STEP        3
#define NAME_STEP   arm_pass3

#define INIT    
#define FINI
#define EMIT(A)     \
    if(box86_dynarec_log>=LOG_DUMP) dynarec_log(LOG_NONE, "\t%08x\n", (A)); \
    *(uint32_t*)(dyn->block) = A;   \
    dyn->block += 4

#define MESSAGE(A, ...)  dynarec_log(A, __VA_ARGS__);
#define FLAGS(A)
#define NEW_INST    
#define INST_NAME(name) if(box86_dynarec_log>=LOG_DUMP) printf_x86_instruction(dyn->dec, &dyn->insts[ninst].x86, name)
#define DEFAULT         

#include "dynarec_arm_pass.h"
