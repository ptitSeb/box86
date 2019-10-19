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


dynablocklist_t* NewDynablockList(uintptr_t base)
{
    dynablocklist_t* ret = (dynablocklist_t*)calloc(1, sizeof(dynablocklist_t));
    ret->blocks = kh_init(dynablocks);
    ret->base = base;
    pthread_mutex_init(&ret->mutex_blocks, NULL);

    return ret;
}

void FreeDynablockList_internal(dynablocklist_t* dynablocks)
{
    if(!dynablocks)
        return;
    // don't free individual blocks, it's only copies
    dynarec_log(LOG_DEBUG, "With a %d Blocks (with %d buckets) Accelerator\n", kh_size(dynablocks->blocks), kh_n_buckets(dynablocks->blocks));
    kh_destroy(dynablocks, dynablocks->blocks);
    pthread_mutex_destroy(&dynablocks->mutex_blocks);

    free(dynablocks);
}

void FreeDynablockList(dynablocklist_t** dynablocks)
{
    if(!dynablocks)
        return;
    if(!*dynablocks)
        return;
    dynarec_log(LOG_INFO, "Free %d Blocks from Dynablocklist (with %d buckets)\n", kh_size((*dynablocks)->blocks), kh_n_buckets((*dynablocks)->blocks));
    dynablock_t* db;
    kh_foreach_value((*dynablocks)->blocks, db,
        free(db->table);
        FreeDynablockList_internal(db->dynablocklists);
        free(db);
    );
    kh_destroy(dynablocks, (*dynablocks)->blocks);

    pthread_mutex_destroy(&(*dynablocks)->mutex_blocks);

    free(*dynablocks);
    *dynablocks = NULL;
}

#define TOOHOT  50000
static void addBlock2Block(dynablock_t* current, dynablock_t* block, uintptr_t addr)
{
    int ret;
    khint_t k;
    pthread_mutex_lock(&current->dynablocklists->mutex_blocks);
    k = kh_put(dynablocks, current->dynablocklists->blocks, addr, &ret);
    if(ret) {   // don't try to insert if already done...
        kh_value(current->dynablocklists->blocks, k) = block;
    }
    pthread_mutex_unlock(&current->dynablocklists->mutex_blocks);

}

static void handlingBlockHeat(dynablock_t* block)
{
    if(!block || !block->done)
        return;
    // is the block getting a lot of call?
    if(!block->dynablocklists && block->hot>TOOHOT)
        block->dynablocklists = NewDynablockList(0); // yes, create a local block hash
    else
        ++block->hot;
}
/* 
    return NULL if block is not found / cannot be created. 
    Don't create if create==0
*/
dynablock_t* DBGetBlock(x86emu_t* emu, uintptr_t addr, int create, dynablock_t* current)
{
    if(current && current->dynablocklists) {
        // local block list, use it first
        dynablocklist_t *dynablocks = current->dynablocklists;
        pthread_mutex_lock(&dynablocks->mutex_blocks);
        int ret;
        dynablock_t* block = NULL;
        khint_t k = kh_get(dynablocks, dynablocks->blocks, addr);
        if(k!=kh_end(dynablocks->blocks)) {
            block = kh_value(dynablocks->blocks, k);
            pthread_mutex_unlock(&dynablocks->mutex_blocks);
            return block;
        }
        pthread_mutex_unlock(&dynablocks->mutex_blocks);
        // not found, let's do regular way
    }
    dynablocklist_t *dynablocks = GetDynablocksFromAddress(emu->context, addr);
    if(!dynablocks)
        return NULL;
    pthread_mutex_lock(&dynablocks->mutex_blocks);
    handlingBlockHeat(current);
    int ret;
    dynablock_t* block = NULL;
    khint_t k = kh_get(dynablocks, dynablocks->blocks, addr-dynablocks->base);
    // check if the block exist
    if(k!=kh_end(dynablocks->blocks)) {
        /*atomic_store(&dynalock, 0);*/
        block = kh_value(dynablocks->blocks, k);
        handlingBlockHeat(block);
        pthread_mutex_unlock(&dynablocks->mutex_blocks);
        if(current && current->dynablocklists)
            addBlock2Block(current, block, addr);
        return block;
    }
    // Blocks doesn't exist. If creation is not allow, just return NULL
    if(!create) {
        pthread_mutex_unlock(&dynablocks->mutex_blocks);
        return block;
    }
    // create and add new block
    dynarec_log(LOG_DEBUG, "Ask for DynaRec Block creation @%p\n", addr);
    k = kh_put(dynablocks, dynablocks->blocks, addr-dynablocks->base, &ret);
    if(!ret) {
        // Ooops, block is already there? retreive it and bail out
        block = kh_value(dynablocks->blocks, k);    
        pthread_mutex_unlock(&dynablocks->mutex_blocks);
        return block;
    }
    block = kh_value(dynablocks->blocks, k) = (dynablock_t*)calloc(1, sizeof(dynablock_t));
    // create an empty block first, so if other thread want to execute the same block, they can, but using interpretor path
    pthread_mutex_unlock(&dynablocks->mutex_blocks);

    if(current && current->dynablocklists)
        addBlock2Block(current, block, addr);
    
    // fill the block
    FillBlock(emu, block, addr);
    dynarec_log(LOG_DEBUG, " --- DynaRec Block created @%p (%p, 0x%x bytes)\n", addr, block->block, block->size);

    return block;
}
