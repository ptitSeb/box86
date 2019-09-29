#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <stdatomic.h>

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
#ifdef ARM
#include "dynarec_arm.h"
#else
#error Unsupported architecture!
#endif

#include "khash.h"

KHASH_MAP_INIT_INT(dynablocks, dynablock_t*)

static _Atomic(int) dynalock;
static int lock_inited = 0;


dynablocklist_t* NewDynablockList()
{
    dynablocklist_t* ret = (dynablocklist_t*)calloc(1, sizeof(dynablocklist_t));
    ret->blocks = kh_init(dynablocks);

    if(!lock_inited) {
        atomic_store(&dynalock, 0);
        lock_inited = 1;
    }

    return ret;
}
void FreeDynablockList(dynablocklist_t** dynablocks)
{
    if(!dynablocks)
        return;
    if(!*dynablocks)
        return;
    dynablock_t* db;
    kh_foreach_value((*dynablocks)->blocks, db,
        free(db);
    );
    kh_destroy(dynablocks, (*dynablocks)->blocks);
    free(*dynablocks);
    *dynablocks = NULL;
}

/* 
    return NULL if block is not found / cannot be created. 
    Don't create if create==0
*/
dynablock_t* DBGetBlock(x86emu_t* emu, uintptr_t addr, int create)
{
    dynablocklist_t *dynablocks = emu->context->dynablocks;
    // use and atomic lock to avoid concurent access to dynablocks list
    int tmp, ret;
    do {
        tmp = 0;
    } while(!atomic_compare_exchange_weak(&dynalock, &tmp, 1));
    khint_t k = kh_get(dynablocks, dynablocks->blocks, addr);
    // check if the block exist, that would be easy
    if(k!=kh_end(dynablocks->blocks) && kh_exist(dynablocks->blocks, k)) {
        atomic_store(&dynalock, 0);
        return kh_value(dynablocks->blocks, k);
    }
    // Blocks doesn't exist. If creation is not allow, just return NULL
    if(!create) {
        atomic_store(&dynalock, 0);
        return NULL;
    }
    // create and add new block
    dynarec_log(LOG_DEBUG, "Ask for DynaRec Block creation @%p\n", addr);
    k = kh_put(dynablocks, dynablocks->blocks, addr, &ret);
    dynablock_t* block = kh_value(dynablocks->blocks, k) = (dynablock_t*)calloc(1, sizeof(dynablock_t));
    // create an empty block first, so if other thread want to execute the same block, they can, but using interpretor path
    atomic_store(&dynalock, 0);
    // fill the block
    FillBlock(emu, block, addr);

    return block;
}
