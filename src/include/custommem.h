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
int isJumpTableDefault(void* addr);
uintptr_t getJumpTable();
uintptr_t getJumpTableAddress(uintptr_t addr);
#endif

#define PROT_DYNAREC    0x80
#define PROT_DYNAREC_R  0x40
#define PROT_NOPROT     0x20
#define PROT_CUSTOM     (PROT_DYNAREC | PROT_DYNAREC_R | PROT_NOPROT)

void updateProtection(uintptr_t addr, uintptr_t size, uint32_t prot);
void setProtection(uintptr_t addr, uintptr_t size, uint32_t prot);
uint32_t getProtection(uintptr_t addr);
void forceProtection(uintptr_t addr, uintptr_t size, uint32_t prot);
void freeProtection(uintptr_t addr, uintptr_t size);
void loadProtectionFromMap();
#ifdef DYNAREC
void protectDB(uintptr_t addr, uintptr_t size);
void unprotectDB(uintptr_t addr, uintptr_t size, int mark); // if mark==0, the blocks are not marked as potentially dirty
int isprotectedDB(uintptr_t addr, size_t size);
int IsInHotPage(uintptr_t addr);
int AreaInHotPage(uintptr_t start, uintptr_t end);
void AddHotPage(uintptr_t addr);
#endif
void* find32bitBlock(size_t size);
void* findBlockNearHint(void* hint, size_t size);

// unlock mutex that are locked by current thread (for signal handling). Return a mask of unlock mutex
int unlockCustommemMutex();
// relock the muxtex that were unlocked
void relockCustommemMutex(int locks);

void init_custommem_helper(box86context_t* ctx);
void fini_custommem_helper(box86context_t* ctx);

#ifdef DYNAREC
// ---- StrongMemoryModel
void addLockAddress(uintptr_t addr);    // add an address to the list of "LOCK"able
int isLockAddress(uintptr_t addr);  // return 1 is the address is used as a LOCK, 0 else
#endif

#endif //__CUSTOM_MEM__H_