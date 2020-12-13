#ifndef __DYNABLOCK_PRIVATE_H_
#define __DYNABLOCK_PRIVATE_H_

typedef struct dynablocklist_s  dynablocklist_t;

typedef struct instsize_s {
    unsigned int x86:4;
    unsigned int nat:4;
} instsize_t;

typedef struct dynablock_s {
    dynablocklist_t* parent;
    void*           block;
    int             size;
    void*           x86_addr;
    int             x86_size;
    uint32_t        hash;
    uintptr_t*      table;
    int             tablesz;
    uint8_t         need_test;
    uint8_t         done;
    uint8_t         gone;
    uint8_t         nolinker;
    int             isize;
    dynablock_t**   sons;   // sons (kind-of dummy dynablock...)
    int             sons_size;
    dynablock_t*    father; // set only in the case of a son
    instsize_t*     instsize;
} dynablock_t;

typedef struct dynablocklist_s {
    uintptr_t           text;
    int                 textsz;
    uintptr_t           base;
    int                 nolinker;    // in case this dynablock can disapear (also, block memory are allocated with a temporary scheme)
    dynablock_t**       direct;    // direct mapping (waste of space, so not always there)
} dynablocklist_t;

#endif //__DYNABLOCK_PRIVATE_H_