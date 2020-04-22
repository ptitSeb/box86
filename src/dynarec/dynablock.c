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


dynablocklist_t* NewDynablockList(uintptr_t base, uintptr_t text, int textsz, int nolinker, int direct)
{
    dynablocklist_t* ret = (dynablocklist_t*)calloc(1, sizeof(dynablocklist_t));
    ret->blocks = kh_init(dynablocks);
    ret->base = base;
    ret->text = text;
    ret->textsz = textsz;
    ret->nolinker = nolinker;
    pthread_rwlock_init(&ret->rwlock_blocks, NULL);
    if(direct && textsz)
        ret->direct = (dynablock_t**)calloc(textsz, sizeof(dynablock_t*));

    return ret;
}

void FreeDynablock(dynablock_t* db, int nolinker)
{
    if(db) {
        free(db->table);
        free(db);
        if(nolinker)
            FreeDynarecMap((uintptr_t)db->block, db->size);
    }
}

void FreeDynablockList(dynablocklist_t** dynablocks)
{
    if(!dynablocks)
        return;
    if(!*dynablocks)
        return;
    int nolinker = (*dynablocks)->nolinker;
    dynarec_log(LOG_INFO, "Free %d Blocks from Dynablocklist (with %d buckets, nolinker=%d) %s\n", kh_size((*dynablocks)->blocks), kh_n_buckets((*dynablocks)->blocks), (*dynablocks)->nolinker, ((*dynablocks)->direct)?" With Direct mapping enabled":"");
    dynablock_t* db;
    kh_foreach_value((*dynablocks)->blocks, db, 
        FreeDynablock(db, nolinker);
    );
    kh_destroy(dynablocks, (*dynablocks)->blocks);
    if((*dynablocks)->direct) {
        for (int i=0; i<(*dynablocks)->textsz; ++i) {
            FreeDynablock((*dynablocks)->direct[i], nolinker);
        }
        free((*dynablocks)->direct);
    }
    (*dynablocks)->direct = 0;

    pthread_rwlock_destroy(&(*dynablocks)->rwlock_blocks);

    free(*dynablocks);
    *dynablocks = NULL;
}

uintptr_t StartDynablockList(dynablocklist_t* db)
{
    if(db)
        return db->text;
    return 0;
}
uintptr_t EndDynablockList(dynablocklist_t* db)
{
    if(db)
        return db->text+db->textsz;
    return 0;
}
void FreeDirectDynablock(dynablocklist_t* dynablocks, uintptr_t addr, uintptr_t size)
{
    uintptr_t startdb = dynablocks->text;
    uintptr_t enddb = dynablocks->text + dynablocks->textsz;
    uintptr_t start = addr;
    uintptr_t end = addr+size;
    if(start<startdb)
        start = startdb;
    if(end>enddb)
        end = enddb;
    if(end>startdb && start<enddb)
        for(uintptr_t i = start; i<end; ++i)
            if(dynablocks->direct[i-startdb]) {
                FreeDynablock(dynablocks->direct[i-startdb], dynablocks->nolinker);
                dynablocks->direct[i-startdb] = NULL;
            }
}


void ConvertHash2Direct(dynablocklist_t* dynablocks)
{
    if(dynablocks->textsz==0 || dynablocks->text==0)
        return; // nothing to do
    // create the new set
    dynablock_t **direct = (dynablock_t**)calloc(dynablocks->textsz, sizeof(dynablock_t*));
    kh_dynablocks_t *blocks = kh_init(dynablocks);
    // transfert
    int ret;
    khint_t k;
    dynablock_t* db;
    uintptr_t key;
    uintptr_t start = dynablocks->text-dynablocks->base;
    uintptr_t end = dynablocks->text + dynablocks->textsz-dynablocks->base;
    kh_foreach(dynablocks->blocks, key, db,
        if(key>=start && key<end)
            direct[key-start] = db;
        else {
            k = kh_put(dynablocks, blocks, key, &ret);
            if(ret) {   // don't try to insert if already done...
                kh_value(blocks, k) = db;
            }
        }
    );
    // destroy old and do the swap (should swap before destroy, but hash access is always behind mutex)
    kh_destroy(dynablocks, dynablocks->blocks);
    dynablocks->blocks = blocks;
    dynablocks->direct = direct;
}

#define MAGIC_SIZE 256
/* 
    return NULL if block is not found / cannot be created. 
    Don't create if create==0
*/
dynablock_t* DBGetBlock(x86emu_t* emu, uintptr_t addr, int create, dynablock_t* current)
{
    // try the quickest way first: get parent of current and check if ok!
    dynablocklist_t *dynablocks = NULL;
    dynablock_t* block = NULL;
    if(current) {
        dynablocks = current->parent;    
        if(dynablocks->direct && (addr>=dynablocks->text) && (addr<=(dynablocks->text+dynablocks->textsz))) {
            block = dynablocks->direct[addr-dynablocks->text];
            if(block)
                return block;
        }
        if(!(addr>=dynablocks->text && addr<=(dynablocks->text+dynablocks->textsz)))
            dynablocks = NULL;
    }
    // nope, lets do the long way
    if(!dynablocks)
        dynablocks = GetDynablocksFromAddress(emu->context, addr);
    if(!dynablocks)
        return NULL;
    // check direct first, without lock
    if(dynablocks->direct && (addr>=dynablocks->text) && (addr<=(dynablocks->text+dynablocks->textsz)))
        block = dynablocks->direct[addr-dynablocks->text];
    if(block)
        return block;
    // nope, put rwlock in read mode and check hash
    pthread_rwlock_rdlock(&dynablocks->rwlock_blocks);
    // but first, check again just in case it has been created while waiting for mutex
    if(dynablocks->direct && (addr>=dynablocks->text) && (addr<=(dynablocks->text+dynablocks->textsz)))
        block = dynablocks->direct[addr-dynablocks->text];
    if(block) {
        pthread_rwlock_unlock(&dynablocks->rwlock_blocks);
        return block;
    }
    // check if the block exist
    int ret;
    khint_t k = kh_get(dynablocks, dynablocks->blocks, addr-dynablocks->base);
    if(k!=kh_end(dynablocks->blocks)) {
        /*atomic_store(&dynalock, 0);*/
        block = kh_value(dynablocks->blocks, k);
        pthread_rwlock_unlock(&dynablocks->rwlock_blocks);
        return block;
    }
    // unlock now
    pthread_rwlock_unlock(&dynablocks->rwlock_blocks);
    // Blocks doesn't exist. If creation is not allow, just return NULL
    if(!create)
        return block;
    // Lock as write now!
    pthread_rwlock_wrlock(&dynablocks->rwlock_blocks);
    // create and add new block
    dynarec_log(LOG_DEBUG, "Ask for DynaRec Block creation @%p\n", (void*)addr);
    if(dynablocks->direct && (addr>=dynablocks->text) && (addr<=(dynablocks->text+dynablocks->textsz))) {
        block = dynablocks->direct[addr-dynablocks->text] = (dynablock_t*)calloc(1, sizeof(dynablock_t));
    } else {
        k = kh_put(dynablocks, dynablocks->blocks, addr-dynablocks->base, &ret);
        if(!ret) {
            // Ooops, block is already there? retreive it and bail out
            block = kh_value(dynablocks->blocks, k);    
            pthread_rwlock_unlock(&dynablocks->rwlock_blocks);
            return block;
        }
        block = kh_value(dynablocks->blocks, k) = (dynablock_t*)calloc(1, sizeof(dynablock_t));
        // check if size of hash map == magic size, if yes, convert to direct before unlocking
        if(!dynablocks->direct && kh_size(dynablocks->blocks)==MAGIC_SIZE)
            ConvertHash2Direct(dynablocks);
    }
    block->parent = dynablocks;
    // create an empty block first, so if other thread want to execute the same block, they can, but using interpretor path
    pthread_rwlock_unlock(&dynablocks->rwlock_blocks);

    // fill the block
    FillBlock(emu, block, addr);
    dynarec_log(LOG_DEBUG, " --- DynaRec Block created @%p (%p, 0x%x bytes)\n", (void*)addr, block->block, block->size);

    return block;
}
