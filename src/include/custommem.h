#ifndef __CUSTOM_MEM__H_
#define __CUSTOM_MEM__H_
#include <unistd.h>
#include <stdint.h>


typedef struct box86context_s box86context_t;

void* customMalloc(size_t size);
void* customCalloc(size_t n, size_t size);
void* customRealloc(void* p, size_t size);
void customFree(void* p);

#define kcalloc     customCalloc
#define kmalloc     customMalloc
#define krealloc    customRealloc
#define kfree       customFree

#ifdef DYNAREC
typedef struct dynablock_s dynablock_t;
typedef struct dynablocklist_s dynablocklist_t;
// custom protection flag to mark Page that are Write protected for Dynarec purpose
uintptr_t AllocDynarecMap(dynablock_t* db, int size);
void FreeDynarecMap(dynablock_t* db, uintptr_t addr, uint32_t size);

void addDBFromAddressRange(uintptr_t addr, uintptr_t size);
void cleanDBFromAddressRange(uintptr_t addr, uintptr_t size, int destroy);

dynablocklist_t* getDB(uintptr_t idx);
void addJumpTableIfDefault(void* addr, void* jmp);
void setJumpTableDefault(void* addr);
uintptr_t getJumpTable();
uintptr_t getJumpTableAddress(uintptr_t addr);
#endif

#define PROT_DYNAREC 0x10000
void updateProtection(uintptr_t addr, uintptr_t size, uint32_t prot);
void setProtection(uintptr_t addr, uintptr_t size, uint32_t prot);
uint32_t getProtection(uintptr_t addr);
#ifdef DYNAREC
void protectDB(uintptr_t addr, uintptr_t size);
void unprotectDB(uintptr_t addr, uintptr_t size);
#endif


void init_custommem_helper(box86context_t* ctx);
void fini_custommem_helper(box86context_t* ctx);

#endif //__CUSTOM_MEM__H_