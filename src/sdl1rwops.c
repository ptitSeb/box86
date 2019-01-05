#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "sdl1rwops.h"
#include "debug.h"
#include "wrapper.h"
#include "box86context.h"
#include "x86run.h"
#include "x86emu.h"
#include "x86emu_private.h"
#include "library_private.h"
#include "bridge.h"
#include "khash.h"

typedef struct SDL1_RWops_s SDL1_RWops_t;

typedef int32_t (*sdl1_seek)(SDL1_RWops_t *context, int32_t offset, int32_t whence);
typedef int32_t (*sdl1_read)(SDL1_RWops_t *context, void *ptr, int32_t size, int32_t maxnum);
typedef int32_t (*sdl1_write)(SDL1_RWops_t *context, const void *ptr, int32_t size, int32_t num);
typedef int32_t (*sdl1_close)(SDL1_RWops_t *context);

typedef struct SDL1_RWops_s {
    sdl1_seek  seek;
    sdl1_read  read;
    sdl1_write write;

    sdl1_close close;

    uint32_t type;
    union {
        struct {
            int autoclose;
            FILE *fp;
        } stdio;
        struct {
            uint8_t *base;
            uint8_t *here;
            uint8_t *stop;
        } mem;
        struct {
            void *data1;
        } unknown;
    } hidden;
} SDL1_RWops_t;

KHASH_MAP_INIT_INT(sdl1seekmap, sdl1_seek)
KHASH_MAP_INIT_INT(sdl1readmap, sdl1_read)
KHASH_MAP_INIT_INT(sdl1writemap, sdl1_write)
KHASH_MAP_INIT_INT(sdl1closemap, sdl1_close)


typedef struct sdl1rwops_s {
    khash_t(sdl1seekmap)        *seekmap;
    khash_t(sdl1readmap)        *readmap;
    khash_t(sdl1writemap)       *writemap;
    khash_t(sdl1closemap)       *closemap;
} sdl1rwops_t;

sdl1rwops_t* NewSDL1RWops()
{
    sdl1rwops_t* rw = (sdl1rwops_t*)calloc(1, sizeof(sdl1rwops_t));
    rw->seekmap = kh_init(sdl1seekmap);
    rw->readmap = kh_init(sdl1readmap);
    rw->writemap = kh_init(sdl1writemap);
    rw->closemap = kh_init(sdl1closemap);
    return rw;
}

void FreeSDL1RWops(sdl1rwops_t **rw)
{
    // no cleaning of bridges, as they can be used multiple time?
    kh_destroy(sdl1seekmap, (*rw)->seekmap);
    kh_destroy(sdl1readmap, (*rw)->readmap);
    kh_destroy(sdl1writemap, (*rw)->writemap);
    kh_destroy(sdl1closemap, (*rw)->closemap);
    free(*rw);
    *rw = NULL;
}

void AddNativeRW(x86emu_t* emu, SDL1_RWops_t* ops)
{
    uintptr_t fnc;
    sdl1rwops_t* rw = (sdl1rwops_t*)emu->context->sdl1lib->priv.w.priv;
    bridge_t* system = emu->context->system;
    khint_t k;
    int ret;

    // get or create wrapper, add it to map and change to the emulated one if rw
    #define GO(A, W) \
    fnc = CheckBridged(system, ops->A); \
    if(!fnc) fnc = AddBridge(system, W, ops->A); \
    k = kh_put(sdl1##A##map, rw->A##map, fnc, &ret); \
    kh_value(rw->A##map, k) = ops->A; \
    ops->A = (sdl1_##A)fnc;

    GO(seek, iFpii)
    GO(read, iFppii)
    GO(write, iFppii)
    GO(close, iFp)

    #undef GO
}

// check if one of the function is a pure emulated one (i.e. not present in dictionnary)
static int isAnyEmulated(sdl1rwops_t* rw, SDL1_RWops_t *ops)
{
    #define GO(A) if((kh_get(sdl1##A##map, rw->A##map, (uintptr_t)ops->A))==(kh_end(rw->A##map))) return 1
    GO(seek);
    GO(read);
    GO(write);
    GO(close);
    #undef GO
    return 0;
}

// put Native RW function, wrapping emulated (callback style) ones if needed
void RWNativeStart(x86emu_t* emu, SDL1_RWops_t* ops, SDLRWSave_t* save)
{
    sdl1rwops_t* rw = (sdl1rwops_t*)emu->context->sdl1lib->priv.w.priv;
    
    save->anyEmu = isAnyEmulated(rw, ops);
    save->seek = ops->seek;
    save->read = ops->read;
    save->write = ops->write;
    save->close = ops->close;
    save->s1 = ops->hidden.mem.base;    // used when wrapping native if other are emulated
    save->s2 = ops->hidden.mem.here;
    if(save->anyEmu) {
        // wrap all function, including the native ones
        printf("ERROR: Emulated RWops function not implemented yet\n");
        emu->quit = 1;
    } else {
        // don't wrap, get back normal functions
        khint_t k;
        #define GO(A) \
        k = kh_get(sdl1##A##map, rw->A##map, (uintptr_t)ops->A); \
        ops->A = kh_value(rw->A##map, k);
        GO(seek)
        GO(read)
        GO(write)
        GO(close)
        #undef GO
    }
}

// put back emulated function back in place
void RWNativeEnd(x86emu_t* emu, SDL1_RWops_t* ops, SDLRWSave_t* save)
{
    sdl1rwops_t* rw = (sdl1rwops_t*)emu->context->sdl1lib->priv.w.priv;

    ops->seek = save->seek;
    ops->read = save->read;
    ops->write = save->write;
    ops->close = save->close;
    if(save->anyEmu) {
        save->s1 = ops->hidden.mem.base;
        save->s2 = ops->hidden.mem.here;
    }
}
