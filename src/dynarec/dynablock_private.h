#ifndef __DYNABLOCK_PRIVATE_H_
#define __DYNABLOCK_PRIVATE_H_

typedef struct dynablock_s {
    void*   block;
    int32_t size;
} dynablock_t;

typedef struct kh_dynablocks_s kh_dynablocks_t;

typedef struct dynablocklist_s {
    kh_dynablocks_t     *blocks;
} dynablocklist_t;

#endif //__DYNABLOCK_PRIVATE_H_