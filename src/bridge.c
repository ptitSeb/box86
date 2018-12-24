#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "bridge.h"
#include "bridge_private.h"
#include "wrapper.h"

#define NBRICK  16
typedef struct brick_s brick_t;
typedef struct brick_s {
    onebridge_t b[NBRICK];
    int         sz;
    brick_t     *next;
} brick_t;

typedef struct bridge_s {
    brick_t     *head;
    brick_t     *last;      // to speed up
} bridge_t;


bridge_t *NewBridge()
{
    bridge_t *b = (bridge_t*)calloc(1, sizeof(bridge_t));
    b->head = (brick_t*)calloc(1, sizeof(brick_t));
    b->last = b->head;

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
    free(*bridge);
    *bridge = NULL;
}

uintptr_t AddBridge(bridge_t* bridge, wrapper_t w, void* fnc)
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
    b->b[b->sz].C3 = 0xC3;

    return (uintptr_t)&b->b[b->sz++].CC;
}
