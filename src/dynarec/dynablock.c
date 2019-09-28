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
dynablock_t* DBGetBlock(box86context_t* context, uintptr_t addr, int create)
{
    dynablocklist_t *dynablocks = context->dynablocks;
    khint_t k = kh_get(dynablocks, dynablocks->blocks, addr);
    // check if the block exist, that would be easy
    if(k!=kh_end(dynablocks->blocks) && kh_exist(dynablocks->blocks, k))
        return kh_value(dynablocks->blocks, k);
    // Blocks doesn't exist. If creation is not allow, just return NULL
    if(!create)
        return NULL;
    // create a new block
    dynarec_log(LOG_DEBUG, "Ask for DynaRec Block creation @%p\n", addr);

    return NULL;
}
