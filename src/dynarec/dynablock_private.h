#ifndef __DYNABLOCK_PRIVATE_H_
#define __DYNABLOCK_PRIVATE_H_

typedef struct jmptable_s {
    uintptr_t to;   // address to jump
    void*     real; // actual jumping address (can be epilog)
    int       resolved; // is jmp address resolved? If not, then real is epilog
} jmptable_t;

typedef struct dynablock_s {
    void*       block;
    int         size;
    jmptable_t* table;
    int         tablesz;
} dynablock_t;

typedef struct kh_dynablocks_s kh_dynablocks_t;

typedef struct dynablocklist_s {
    kh_dynablocks_t     *blocks;
} dynablocklist_t;

#endif //__DYNABLOCK_PRIVATE_H_