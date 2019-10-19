#ifndef __DYNABLOCK_PRIVATE_H_
#define __DYNABLOCK_PRIVATE_H_

typedef struct dynablocklist_s dynablocklist_t;

typedef struct dynablock_s {
    void*       block;
    int         size;
    uintptr_t*  table;
    int         tablesz;
    int         done;
    int         hot;    // hot block detection counter
    dynablocklist_t * dynablocklists;   // once hot, all linker block will go there
} dynablock_t;

typedef struct kh_dynablocks_s kh_dynablocks_t;

typedef struct dynablocklist_s {
    kh_dynablocks_t     *blocks;
    pthread_mutex_t     mutex_blocks;
    uintptr_t           base;
} dynablocklist_t;

#endif //__DYNABLOCK_PRIVATE_H_