#ifndef __DYNABLOCK_H_
#define __DYNABLOCK_H_

typedef struct box86context_s box86context_t;

typedef struct dynablock_s dynablock_t;

typedef struct dynablocklist_s dynablocklist_t;

dynablocklist_t* NewDynablockList();
void FreeDynablockList(dynablocklist_t** dynablocks);


// Handling of Dynarec block (i.e. an exectable chunk of x86 translated code)
dynablock_t* DBGetBlock(box86context_t* context, uintptr_t addr, int create);   // return NULL if block is not found / cannot be created. Don't create if create==0

#endif //__DYNABLOCK_H_