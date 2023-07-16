#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/mman.h>

#include "custommem.h"
#include "bridge.h"
#include "bridge_private.h"
#include "wrapper.h"
#include "khash.h"
#include "debug.h"
#include "x86emu.h"
#include "box86context.h"
#include "elfloader.h"
#ifdef DYNAREC
#include "dynablock.h"
#endif

KHASH_MAP_INIT_INT(bridgemap, uintptr_t)

#define NBRICK  (4096/sizeof(onebridge_t))
typedef struct brick_s brick_t;
typedef struct brick_s {
    onebridge_t *b;
    int         sz;
    brick_t     *next;
} brick_t;

typedef struct bridge_s {
    brick_t         *head;
    brick_t         *last;      // to speed up
    kh_bridgemap_t  *bridgemap;
} bridge_t;

void* my_mmap(x86emu_t* emu, void* addr, unsigned long length, int prot, int flags, int fd, int offset);
int my_munmap(x86emu_t* emu, void* addr, unsigned long length);
brick_t* NewBrick(void* old)
{
    brick_t* ret = (brick_t*)box_calloc(1, sizeof(brick_t));
    if(old)
        old = old + NBRICK * sizeof(onebridge_t);
    void* ptr = my_mmap(NULL, old, NBRICK * sizeof(onebridge_t), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(ptr == MAP_FAILED)
        ptr = my_mmap(NULL, NULL, NBRICK * sizeof(onebridge_t), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(ptr == MAP_FAILED) {
        printf_log(LOG_NONE, "Warning, cannot allocate 0x%x aligned bytes for bridge, will probably crash later\n", NBRICK*sizeof(onebridge_t));
    }
    #ifdef DYNAREC
    setProtection((uintptr_t)ptr, NBRICK * sizeof(onebridge_t), PROT_READ | PROT_WRITE | PROT_EXEC | PROT_NOPROT);
    #endif
    dynarec_log(LOG_INFO, "New Bridge brick at %p (size 0x%x)\n", ptr, NBRICK*sizeof(onebridge_t));
    ret->b = ptr;
    return ret;
}

bridge_t *NewBridge()
{
    bridge_t *b = (bridge_t*)box_calloc(1, sizeof(bridge_t));
    b->head = NewBrick(NULL);
    b->last = b->head;
    b->bridgemap = kh_init(bridgemap);

    return b;
}
void FreeBridge(bridge_t** bridge)
{
    if(!bridge || !*bridge) {
        return;
    }
    bridge_t* br = *bridge;
    *bridge = NULL;

    brick_t *b = br->head;
    while(b) {
        brick_t *n = b->next;
        my_munmap(NULL, b->b, NBRICK*sizeof(onebridge_t));
        box_free(b);
        b = n;
    }
    kh_destroy(bridgemap, br->bridgemap);
    box_free(br);
}

#ifdef HAVE_TRACE
void addBridgeName(void* addr, const char* name);
#endif

uintptr_t AddBridge(bridge_t* bridge, wrapper_t w, void* fnc, int N, const char* name)
{
    if(!bridge) return 0;
    brick_t *b = NULL;
    int sz = -1;
    int ret;

    mutex_lock(&my_context->mutex_bridge);
    b = bridge->last;
    if(b->sz == NBRICK) {
        b->next = NewBrick(b->b);
        b = b->next;
        bridge->last = b;
    }
    sz = b->sz;
    b->sz++;
    // add bridge to map, for fast recovery
    khint_t k = kh_put(bridgemap, bridge->bridgemap, (uintptr_t)fnc, &ret);
    kh_value(bridge->bridgemap, k) = (uintptr_t)&b->b[sz].CC;
    mutex_unlock(&my_context->mutex_bridge);

    b->b[sz].CC = 0xCC;
    b->b[sz].S = 'S'; b->b[sz].C='C';
    b->b[sz].w = w;
    b->b[sz].f = (uintptr_t)fnc;
    b->b[sz].C3 = N?0xC2:0xC3;
    b->b[sz].N = N;
    #ifdef HAVE_TRACE
    if(name)
        addBridgeName(fnc, name);
    #endif

    return (uintptr_t)&b->b[sz].CC;
}

uintptr_t CheckBridged(bridge_t* bridge, void* fnc)
{
    // check if function alread have a bridge (the function wrapper will not be tested)
    khint_t k = kh_get(bridgemap, bridge->bridgemap, (uint32_t)fnc);
    if(k==kh_end(bridge->bridgemap))
        return 0;
    return kh_value(bridge->bridgemap, k);
}

int IsBridge(void* fnc) {
    onebridge_t *b = (onebridge_t*)fnc;
    if(!b || b->CC != 0xCC || b->S!='S' || b->C!='C' || (b->C3!=0xC3 && b->C3!=0xC2)) {
        return 0;
    }
    return 1;
}

uintptr_t AddCheckBridge(bridge_t* bridge, wrapper_t w, void* fnc, int N, const char* name)
{
    if(!fnc && w)
        return 0;
    uintptr_t ret = CheckBridged(bridge, fnc);
    if(!ret)
        ret = AddBridge(bridge, w, fnc, N, name);
    return ret;
}

uintptr_t AddAutomaticBridge(x86emu_t* emu, bridge_t* bridge, wrapper_t w, void* fnc, int N, const char* name)
{
    if(!fnc)
        return 0;
    uintptr_t ret = CheckBridged(bridge, fnc);
    if(!ret)
        ret = AddBridge(bridge, w, fnc, N, name);
    if(!hasAlternate(fnc)) {
        printf_log(LOG_DEBUG, "Adding AutomaticBridge for %p to %p\n", fnc, (void*)ret);
        addAlternate(fnc, (void*)ret);
#ifdef DYNAREC
        // now, check if dynablock at native address exist
        DBAlternateBlock(emu, (uintptr_t)fnc, ret);
#else
        (void)emu;
#endif
    }
    return ret;
}

void* GetNativeFnc(uintptr_t fnc)
{
    if(!fnc) return NULL;
    // check if function exist in some loaded lib
    if(!FindElfAddress(my_context, fnc)) {
        Dl_info info;
        if(dladdr((void*)fnc, &info))
            return (void*)fnc;
    }
    if(!getProtection(fnc))
        return NULL;
    // check if it's an indirect jump
    #define PK(a)       *(uint8_t*)(fnc+a)
    #define PK32(a)     *(uint32_t*)(fnc+a)
    if(PK(0)==0xff && PK(1)==0x25) {  // absolute jump, maybe the GOT
        uintptr_t a1 = (PK32(2));   // need to add a check to see if the address is from the GOT !
        a1 = *(uintptr_t*)a1;
        if(a1 && a1>0x10000) {
            a1 = (uintptr_t)GetNativeFnc(a1);
            if(a1)
                return (void*)a1;
        }
    }
    #undef PK
    #undef PK32
    // check if bridge exist
    if (IsBridge((void*)fnc)) {
        return (void*)((onebridge_t*)fnc)->f;
    }
    return NULL;
}

void* GetNativeFncOrFnc(uintptr_t fnc)
{
    if (IsBridge((void*)fnc)) {
        return (void*)((onebridge_t*)fnc)->f;
    } else {
        return (void*)fnc;
    }
}

#ifdef HAVE_TRACE
KHASH_MAP_INIT_INT64(bridgename, const char*)
static kh_bridgename_t *bridgename;
void initBridgeName()
{
    bridgename = kh_init(bridgename);
}
void finiBridgeName()
{
    kh_destroy(bridgename, bridgename);
    bridgename = NULL;
}
void addBridgeName(void* addr, const char* name)
{
    int ret;
    khint_t k = kh_put(bridgename, bridgename, (uintptr_t)addr, &ret);
    if(!ret) // already there
        return;
    kh_value(bridgename, k) = name;
}
const char* getBridgeName(void* addr)
{
    khint_t k = kh_get(bridgename, bridgename, (uintptr_t)addr);
    if(k!=kh_end(bridgename))
        return kh_value(bridgename, k);
    return NULL;
}
#endif


// Alternate address handling
KHASH_MAP_INIT_INT(alternate, void*)
static kh_alternate_t *my_alternates = NULL;

int hasAlternate(void* addr) {
    if(!my_alternates)
        return 0;
    khint_t k = kh_get(alternate, my_alternates, (uintptr_t)addr);
    if(k==kh_end(my_alternates))
        return 0;
    return 1;
}

void* getAlternate(void* addr) {
    if(!my_alternates)
        return addr;
    khint_t k = kh_get(alternate, my_alternates, (uintptr_t)addr);
    if(k!=kh_end(my_alternates))
        return kh_value(my_alternates, k);
    return addr;
}
void addAlternate(void* addr, void* alt) {
    if(!my_alternates) {
        my_alternates = kh_init(alternate);
    }
    int ret;
    khint_t k = kh_put(alternate, my_alternates, (uintptr_t)addr, &ret);
    if(!ret)    // already there
        return;
    kh_value(my_alternates, k) = alt;
}

void cleanAlternate() {
    if(my_alternates) {
        kh_destroy(alternate, my_alternates);
        my_alternates = NULL;
    }
}

void init_bridge_helper()
{
    #ifdef HAVE_TRACE
    initBridgeName();
    #endif
}

void fini_bridge_helper()
{
    cleanAlternate();
    #ifdef HAVE_TRACE
    finiBridgeName();
    #endif
}
