#ifndef __DYNABLOCK_PRIVATE_H_
#define __DYNABLOCK_PRIVATE_H_

typedef struct dynablocklist_s dynablocklist_t;

typedef struct dynablock_s {
    dynablocklist_t *parent;
    void*       block;
    int         size;
    void*       x86_addr;
    int         x86_size;
    uint32_t    hash;
    int         need_test;
    uintptr_t*  table;
    int         tablesz;
    int         done;
    int         isize;
} dynablock_t;

typedef struct kh_dynablocks_s kh_dynablocks_t;

typedef struct dynablocklist_s {
    kh_dynablocks_t     *blocks;
    pthread_rwlock_t    rwlock_blocks;
    uintptr_t           base;
    uintptr_t           text;
    int                 textsz;
    int                 nolinker;    // in case this dynablock can disapear (also, block memory are allocated with a temporary scheme)
    dynablock_t         **direct;    // direct mapping (waste of space, so not always there)
} dynablocklist_t;

#endif //__DYNABLOCK_PRIVATE_H_