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
#include "dynarec_private.h"
#include "elfloader.h"
#ifdef ARM
#include "dynarec_arm.h"
#else
#error Unsupported architecture!
#endif

#include "khash.h"

KHASH_MAP_INIT_INT(dynablocks, dynablock_t*)

KHASH_SET_INIT_INT(mark)

uint32_t X31_hash_code(void* addr, int len)
{
    if(!len) return 0;
    uint8_t* p = (uint8_t*)addr;
	int32_t h = *p;
	for (--len, ++p; len; --len, ++p) h = (h << 5) - h + (int32_t)*p;
	return (uint32_t)h;
}

dynablocklist_t* NewDynablockList(uintptr_t base, uintptr_t text, int textsz, int nolinker, int direct)
{
    dynablocklist_t* ret = (dynablocklist_t*)calloc(1, sizeof(dynablocklist_t));
    ret->blocks = (nolinker && direct)?NULL:kh_init(dynablocks);
    ret->base = base;
    ret->text = text;
    ret->textsz = textsz;
    ret->nolinker = nolinker;
    pthread_rwlock_init(&ret->rwlock_blocks, NULL);
    if(direct && textsz)
        ret->direct = (dynablock_t**)calloc(textsz, sizeof(dynablock_t*));

    return ret;
}

void FreeDynablock(dynablock_t* db)
{
    if(db) {
        dynarec_log(LOG_DEBUG, "FreeDynablock(%p), db->block=%p x86=%p:%p father=%p, tablesz=%d, %swith %d son(s)\n", db, db->block, db->x86_addr, db->x86_addr+db->x86_size, db->father, db->tablesz, db->marks?"with marks, ":"", db->sons_size);
        db->done = 0;
        // remove from direct if there
        uintptr_t startdb = db->parent->text;
        uintptr_t enddb = db->parent->text + db->parent->textsz;
        if(db->parent->direct) {
            uintptr_t addr = (uintptr_t)db->x86_addr;
            if(addr>=startdb && addr<enddb)
                db->parent->direct[addr-startdb] = NULL;
        }
        if(db->marks) {
            // Follow mark and set arm_linker instead
            khint_t k;
            char s;
            kh_foreach(db->marks, k, s, 
                void** p = (void**)(uintptr_t)k;
                dynarec_log(LOG_DEBUG, " -- resettable(%p)\n", p);
                (void)s;
                resettable(p);
            );
            // free mark
            kh_destroy(mark, db->marks);
            db->marks = NULL;
        }
        for(int i=0; i<db->tablesz; i+=4) {
            dynablock_t* p = (dynablock_t*)db->table[i+3];
            dynarec_log(LOG_DEBUG, "  -- table[%d+3] = %p ", i, p);
            if(p && p!=db && p->marks) {
                khint_t kd = kh_get(mark, p->marks, (uintptr_t)(&db->table[i+0]));
                if(kd!=kh_end(p->marks)) {
                    kh_del(mark, p->marks, kd);
                    dynarec_log(LOG_DEBUG, "cleaned");
                }
                else {dynarec_log(LOG_DEBUG, "not found");}
            }
            dynarec_log(LOG_DEBUG, "\n");
        }
        // remove and free the sons
        for (int i=0; i<db->sons_size; ++i) {
            dynablock_t *son = db->sons[i];
            db->sons[i] = NULL;
            FreeDynablock(son);
        }
        // only the father free the DynarecMap
        if(!db->father) {
            dynarec_log(LOG_DEBUG, " -- FreeDyrecMap(%p, %d)\n", db->block, db->size);
            FreeDynarecMap((uintptr_t)db->block, db->size);
        }
        free(db->sons);
        free(db->table);
        free(db);
    }
}

void FreeDynablockList(dynablocklist_t** dynablocks)
{
    if(!dynablocks)
        return;
    if(!*dynablocks)
        return;
    dynarec_log(LOG_INFO, "Free %d Blocks from Dynablocklist (with %d buckets, nolinker=%d) %s\n", (*dynablocks)->blocks?kh_size((*dynablocks)->blocks):0, (*dynablocks)->blocks?kh_n_buckets((*dynablocks)->blocks):0, (*dynablocks)->nolinker, ((*dynablocks)->direct)?" With Direct mapping enabled":"");
    dynablock_t* db;
    if((*dynablocks)->blocks) {
        // create a list of the parent db to delete (because there are sons in the middle that will be invalid if father is removed)
        dynablock_t** list = (dynablock_t**)calloc(kh_size((*dynablocks)->blocks), sizeof(dynablock_t*));
        int n = 0;
        kh_foreach_value((*dynablocks)->blocks, db, 
            if(!db->father) list[n++] = db;
        );
        kh_destroy(dynablocks, (*dynablocks)->blocks);
        for (int i=0; i<n; ++i)
            FreeDynablock(list[i]);
        free(list);
    }
    if((*dynablocks)->direct) {
        for (int i=0; i<(*dynablocks)->textsz; ++i) {
            if((*dynablocks)->direct[i] && !(*dynablocks)->direct[i]->father) 
                FreeDynablock((*dynablocks)->direct[i]);
        }
        free((*dynablocks)->direct);
    }
    (*dynablocks)->direct = 0;

    pthread_rwlock_destroy(&(*dynablocks)->rwlock_blocks);

    free(*dynablocks);
    *dynablocks = NULL;
}

void MarkDynablock(dynablock_t* db)
{
    if(db) {
        if(db->father)
            db = db->father;    // mark only father
        if(db->marks && !db->need_test) {
            // Follow mark and set arm_linker instead
            khint_t k;
            char s;
            kh_foreach(db->marks, k, s, 
                void** p = (void**)(uintptr_t)k;
                (void)s;
                resettable(p);
            );
            // free mark
            kh_clear(mark, db->marks);
        }
        db->need_test = 1;
    }
}

void ProtectDynablock(dynablock_t* db)
{
    if(db) {
        if(db->father)
            return;    // protect only father, child(ren) will be automatically
        protectDB((uintptr_t)db->x86_addr, db->x86_size);
    }
}

void RemoveMark(void** table)
{
    // remove old value if any
    dynablock_t *old = (dynablock_t*)table[3];
    if(old && old->father)
        old = old->father;
    if(old && old->marks) {
        khint_t kd = kh_get(mark, old->marks, (uintptr_t)table);
        if(kd!=kh_end(old->marks))
            kh_del(mark, old->marks, kd);
    }
    table[3] = NULL;
}

// source is linked to dest (i.e. source->table[x] = dest->block), so add a "mark" in dest, add a "linked" info to source
void AddMark(dynablock_t* source, dynablock_t* dest, void** table)
{
    int ret;
    // remove old value if any
    dynablock_t *old = (dynablock_t*)table[3];
    if(old && old->father)
        old = old->father;
    if(old && old->marks) {
        khint_t kd = kh_get(mark, old->marks, (uintptr_t)table);
        if(kd!=kh_end(old->marks))
            kh_del(mark, old->marks, kd);
    }
    // add mark if needed
    if(dest->father)
        dest = dest->father;
    if(dest->marks) {
        kh_put(mark, dest->marks, (uintptr_t)table, &ret);
        table[3] = (void*)dest;
    } else
        table[3] = NULL;
}

void MarkDynablockList(dynablocklist_t** dynablocks)
{
    if(!dynablocks)
        return;
    if(!*dynablocks)
        return;
    dynarec_log(LOG_DEBUG, "Marked %d Blocks from Dynablocklist (with %d buckets, nolinker=%d) %p:0x%x %s\n", (*dynablocks)->blocks?(kh_size((*dynablocks)->blocks)):0, (*dynablocks)->blocks?(kh_n_buckets((*dynablocks)->blocks)):0, (*dynablocks)->nolinker, (void*)(*dynablocks)->text, (*dynablocks)->textsz, ((*dynablocks)->direct)?" With Direct mapping enabled":"");
    dynablock_t* db;
    if((*dynablocks)->blocks) {
        kh_foreach_value((*dynablocks)->blocks, db, 
            ProtectDynablock(db);
        );
    }
    if((*dynablocks)->direct) {
        for (int i=0; i<(*dynablocks)->textsz; ++i) {
            ProtectDynablock((*dynablocks)->direct[i]);
        }
    }
}

void ProtectkDynablockList(dynablocklist_t** dynablocks)
{
    if(!dynablocks)
        return;
    if(!*dynablocks)
        return;
    dynarec_log(LOG_DEBUG, "Protect %d Blocks from Dynablocklist (with %d buckets, nolinker=%d) %p:0x%x %s\n", (*dynablocks)->blocks?(kh_size((*dynablocks)->blocks)):0, (*dynablocks)->blocks?(kh_n_buckets((*dynablocks)->blocks)):0, (*dynablocks)->nolinker, (void*)(*dynablocks)->text, (*dynablocks)->textsz, ((*dynablocks)->direct)?" With Direct mapping enabled":"");
    dynablock_t* db;
    if((*dynablocks)->blocks) {
        kh_foreach_value((*dynablocks)->blocks, db, 
            MarkDynablock(db);
        );
    }
    if((*dynablocks)->direct) {
        for (int i=0; i<(*dynablocks)->textsz; ++i) {
            MarkDynablock((*dynablocks)->direct[i]);
        }
    }
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
    if(!dynablocks)
        return;
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
                FreeDynablock(dynablocks->direct[i-startdb]);
            }
}
void MarkDirectDynablock(dynablocklist_t* dynablocks, uintptr_t addr, uintptr_t size)
{
    if(!dynablocks)
        return;
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
                MarkDynablock(dynablocks->direct[i-startdb]);
            }
}

void ProtectDirectDynablock(dynablocklist_t* dynablocks, uintptr_t addr, uintptr_t size)
{
    if(!dynablocks)
        return;
    uintptr_t startdb = dynablocks->text;
    uintptr_t enddb = dynablocks->text + dynablocks->textsz;
    uintptr_t start = addr;
    uintptr_t end = addr+size;
    if(start<startdb)
        start = startdb;
    if(end>enddb)
        end = enddb;
    if(end>startdb && start<enddb)
        protectDB(start, end-start+1);
}

void ConvertHash2Direct(dynablocklist_t* dynablocks)
{
    if(dynablocks->textsz==0 || dynablocks->text==0 || !dynablocks->blocks)
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

dynablock_t *AddNewDynablock(dynablocklist_t* dynablocks, uintptr_t addr, int with_marks, int* created)
{
    khint_t k;
    int ret;
    dynablock_t* block = NULL;
    // first, check if it exist in direct access mode
    if(dynablocks->direct && (addr>=dynablocks->text) && (addr<(dynablocks->text+dynablocks->textsz))) {
        block = dynablocks->direct[addr-dynablocks->text];
        if(block) {
            dynarec_log(LOG_DUMP, "Block already exist in Direct Map\n");
            *created = 0;
            return block;
        }
    }
    // Lock as write now!
    pthread_rwlock_wrlock(&dynablocks->rwlock_blocks);
    // create and add new block
    dynarec_log(LOG_DUMP, "Ask for DynaRec Block creation @%p\n", (void*)addr);
    if(dynablocks->direct && (addr>=dynablocks->text) && (addr<(dynablocks->text+dynablocks->textsz))) {
        block = dynablocks->direct[addr-dynablocks->text] = (dynablock_t*)calloc(1, sizeof(dynablock_t));
    } else {
        if(!dynablocks->blocks) {
            dynarec_log(LOG_INFO, "Warning: Refused to create a Direct Block that is out-of-bound: dynablocks=%p (%p:%p), addr=%p\n", dynablocks, (void*)(dynablocks->text), (void*)(dynablocks->text+dynablocks->textsz), (void*)addr);
            pthread_rwlock_unlock(&dynablocks->rwlock_blocks);
            *created = 0;
            return NULL;
        }
        k = kh_put(dynablocks, dynablocks->blocks, addr-dynablocks->base, &ret);
        if(!ret) {
            // Ooops, block is already there? retreive it and bail out
            dynarec_log(LOG_DUMP, "Block already exist in Hash Map\n");
            block = kh_value(dynablocks->blocks, k);    
            pthread_rwlock_unlock(&dynablocks->rwlock_blocks);
            *created = 0;
            return block;
        }
        block = kh_value(dynablocks->blocks, k) = (dynablock_t*)calloc(1, sizeof(dynablock_t));
        // check if size of hash map == magic size, if yes, convert to direct before unlocking
        if(!dynablocks->direct && kh_size(dynablocks->blocks)==MAGIC_SIZE)
            ConvertHash2Direct(dynablocks);
    }
    block->parent = dynablocks;
    // add mark if needed
    if(with_marks)
        block->marks = kh_init(mark);

    // create an empty block first, so if other thread want to execute the same block, they can, but using interpretor path
    pthread_rwlock_unlock(&dynablocks->rwlock_blocks);

    *created = 1;
    return block;
}

/* 
    return NULL if block is not found / cannot be created. 
    Don't create if create==0
*/
static dynablock_t* internalDBGetBlock(x86emu_t* emu, uintptr_t addr, int create, dynablock_t* current)
{
    // try the quickest way first: get parent of current and check if ok!
    dynablocklist_t *dynablocks = NULL;
    dynablock_t* block = NULL;
    if(current) {
        dynablocks = current->parent;    
        if(!(addr>=dynablocks->text && addr<(dynablocks->text+dynablocks->textsz)))
            dynablocks = NULL;
        else if(dynablocks->direct && (addr>=dynablocks->text) && (addr<(dynablocks->text+dynablocks->textsz))) {
            block = dynablocks->direct[addr-dynablocks->text];
            if(block)
                return block;
        }
    }
    // nope, lets do the long way
    if(!dynablocks)
        dynablocks = GetDynablocksFromAddress(emu->context, addr);
    if(!dynablocks)
        return NULL;
    // check direct first, without lock
    if(dynablocks->direct && (addr>=dynablocks->text) && (addr<(dynablocks->text+dynablocks->textsz)))
        block = dynablocks->direct[addr-dynablocks->text];
    if(block)
        return block;
    // nope, put rwlock in read mode and check hash
    pthread_rwlock_rdlock(&dynablocks->rwlock_blocks);
    // check if the block exist
    khint_t k;
    if(dynablocks->blocks) {
        k = kh_get(dynablocks, dynablocks->blocks, addr-dynablocks->base);
        if(k!=kh_end(dynablocks->blocks)) {
            /*atomic_store(&dynalock, 0);*/
            block = kh_value(dynablocks->blocks, k);
            pthread_rwlock_unlock(&dynablocks->rwlock_blocks);
            return block;
        }
    }    
    // unlock now
    pthread_rwlock_unlock(&dynablocks->rwlock_blocks);
    // Blocks doesn't exist. If creation is not allow, just return NULL
    if(!create)
        return block;

    int created = 0;
    block = AddNewDynablock(dynablocks, addr, (dynablocks->nolinker)&&(dynablocks->direct), &created);
    if(!created)
        return block;   // existing block...

    if(box86_dynarec_dump)
        pthread_mutex_lock(&my_context->mutex_lock);
    // fill the block
    block->x86_addr = (void*)addr;
    FillBlock(block);
    if(box86_dynarec_dump)
        pthread_mutex_unlock(&my_context->mutex_lock);

    dynarec_log(LOG_DEBUG, " --- DynaRec Block %s @%p:%p (%p, 0x%x bytes, %swith %d son(s))\n", created?"created":"recycled", (void*)addr, (void*)(addr+block->x86_size), block->block, block->size, block->marks?"with Marks, ":"", block->sons_size);

    return block;
}

dynablock_t* DBGetBlock(x86emu_t* emu, uintptr_t addr, int create, dynablock_t** current)
{
    dynablock_t *db = internalDBGetBlock(emu, addr, create, *current);
    if(db && (db->need_test || (db->father && db->father->need_test))) {
        dynablock_t *father = db->father?db->father:db;
        uint32_t hash = X31_hash_code(father->x86_addr, father->x86_size);
        if(hash!=father->hash) {
            dynarec_log(LOG_DEBUG, "Invalidating block %p from %p:%p (hash:%X/%X)%s with %d son(s)\n", father, father->x86_addr, father->x86_addr+father->x86_size, hash, father->hash, father->marks?" with Mark,":"", father->sons_size);
            // no more current if it gets invalidated too
            if(*current && father->x86_addr>=(*current)->x86_addr && (father->x86_addr+father->x86_size)<(*current)->x86_addr)
                *current = NULL;
            // Free father, it's now invalid!
            FreeDynablock(father);
            // start again... (will create a new block)
            db = internalDBGetBlock(emu, addr, create, *current);
        } else {
            father->need_test = 0;
            protectDB((uintptr_t)father->x86_addr, father->x86_size);
        }
    } 
    return db;
}