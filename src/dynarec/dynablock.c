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
#include "elfloader.h"
#ifdef ARM
#include "dynarec_arm.h"
#else
#error Unsupported architecture!
#endif

#include "khash.h"

KHASH_MAP_INIT_INT(dynablocks, dynablock_t*)


dynablocklist_t* NewDynablockList()
{
    dynablocklist_t* ret = (dynablocklist_t*)calloc(1, sizeof(dynablocklist_t));
    ret->blocks = kh_init(dynablocks);

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
        free(db->table);
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
    dynablocklist_t *dynablocks = GetDynablocksFromAddress(emu->context, addr);
    if(!dynablocks)
        return NULL;
    pthread_mutex_lock(&emu->context->mutex_blocks);
    int ret;
    dynablock_t* block = NULL;
    khint_t k = kh_get(dynablocks, dynablocks->blocks, addr);
    // check if the block exist
    if(k!=kh_end(dynablocks->blocks)) {
        /*atomic_store(&dynalock, 0);*/
        block = kh_value(dynablocks->blocks, k);
        pthread_mutex_unlock(&emu->context->mutex_blocks);
        return block;
    }
    // Blocks doesn't exist. If creation is not allow, just return NULL
    if(!create) {
        pthread_mutex_unlock(&emu->context->mutex_blocks);
        return block;
    }
    // create and add new block
    dynarec_log(LOG_DEBUG, "Ask for DynaRec Block creation @%p\n", addr);
    k = kh_put(dynablocks, dynablocks->blocks, addr, &ret);
    block = kh_value(dynablocks->blocks, k) = (dynablock_t*)calloc(1, sizeof(dynablock_t));
    // create an empty block first, so if other thread want to execute the same block, they can, but using interpretor path
    pthread_mutex_unlock(&emu->context->mutex_blocks);
    // fill the block
    FillBlock(emu, block, addr);
    dynarec_log(LOG_DEBUG, " --- DynaRec Block created @%p (%p, 0x%x bytes)\n", addr, block->block, block->size);

    //pthread_mutex_unlock(&emu->context->mutex_blocks);

    return block;
}
