#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "bridge.h"
#include "bridge_private.h"
#include "wrapper.h"
#include "khash.h"

KHASH_MAP_INIT_INT(bridgemap, uintptr_t)

#define NBRICK  16
typedef struct brick_s brick_t;
typedef struct brick_s {
    onebridge_t b[NBRICK];
    int         sz;
    brick_t     *next;
} brick_t;

typedef struct bridge_s {
    brick_t         *head;
    brick_t         *last;      // to speed up
    kh_bridgemap_t  *bridgemap;
} bridge_t;


bridge_t *NewBridge()
{
    bridge_t *b = (bridge_t*)calloc(1, sizeof(bridge_t));
    b->head = (brick_t*)calloc(1, sizeof(brick_t));
    b->last = b->head;
    b->bridgemap = kh_init(bridgemap);

    return b;
}
void FreeBridge(bridge_t** bridge)
{
    brick_t *b = (*bridge)->head;
    while(b) {
        brick_t *n = b->next;
        free(b);
        b = n;
    }
    kh_destroy(bridgemap, (*bridge)->bridgemap);
    free(*bridge);
    *bridge = NULL;
}

uintptr_t AddBridge(bridge_t* bridge, wrapper_t w, void* fnc, int N)
{
    brick_t *b = bridge->last;
    if(b->sz == NBRICK) {
        b->next = (brick_t*)calloc(1, sizeof(brick_t));
        b = b->next;
        bridge->head = b;
    }
    b->b[b->sz].CC = 0xCC;
    b->b[b->sz].S = 'S'; b->b[b->sz].C='C';
    b->b[b->sz].w = w;
    b->b[b->sz].f = (uintptr_t)fnc;
    b->b[b->sz].C3 = N?0xC2:0xC3;
    b->b[b->sz].N = N;
    // add bridge to map, for fast recovery
    int ret;
    khint_t k = kh_put(bridgemap, bridge->bridgemap, (uintptr_t)fnc, &ret);
    kh_value(bridge->bridgemap, k) = (uintptr_t)&b->b[b->sz].CC;

    return (uintptr_t)&b->b[b->sz++].CC;
}

uintptr_t CheckBridged(bridge_t* bridge, void* fnc)
{
    // check if function alread have a bridge (the function wrapper will not be tested)
    khint_t k = kh_get(bridgemap, bridge->bridgemap, (uint32_t)fnc);
    if(k==kh_end(bridge->bridgemap))
        return 0;
    return kh_value(bridge->bridgemap, k);
}

uintptr_t AddCheckBridge(bridge_t* bridge, wrapper_t w, void* fnc, int N)
{
    if(!fnc)
        return 0;
    uintptr_t ret = CheckBridged(bridge, fnc);
    if(!ret)
        ret = AddBridge(bridge, w, fnc, N);
    return ret;
}

void* GetNativeFnc(uintptr_t fnc)
{
    if(!fnc) return NULL;
    onebridge_t *b = (onebridge_t*)fnc;
    if(b->CC != 0xCC || b->S!='S' || b->C!='C' || (b->C3!=0xC3 && b->C3!=0xC2))
        return NULL;    // not a bridge?!
    return (void*)b->f;
}

void* GetNativeFncOrFnc(uintptr_t fnc)
{
    onebridge_t *b = (onebridge_t*)fnc;
    if(b->CC != 0xCC || b->S!='S' || b->C!='C' || (b->C3!=0xC3 && b->C3!=0xC2))
        return (void*)fnc;    // not a bridge?!
    return (void*)b->f;
}