#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "sdl2rwops.h"
#include "debug.h"
#include "wrapper.h"
#include "box86context.h"
#include "x86run.h"
#include "x86emu.h"
#include "x86emu_private.h"
#include "library_private.h"
#include "bridge.h"
#include "khash.h"

typedef struct SDL2_RWops_s SDL2_RWops_t;

typedef int64_t (*sdl2_size)(SDL2_RWops_t *context);
typedef int64_t (*sdl2_seek)(SDL2_RWops_t *context, int64_t offset, int32_t whence);
typedef int32_t (*sdl2_read)(SDL2_RWops_t *context, void *ptr, int32_t size, int32_t maxnum);
typedef int32_t (*sdl2_write)(SDL2_RWops_t *context, const void *ptr, int32_t size, int32_t num);
typedef int32_t (*sdl2_close)(SDL2_RWops_t *context);

typedef struct SDL2_RWops_s {
    sdl2_size  size;
    sdl2_seek  seek;
    sdl2_read  read;
    sdl2_write write;

    sdl2_close close;

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
} SDL2_RWops_t;

KHASH_MAP_INIT_INT(sdl2sizemap, sdl2_size)
KHASH_MAP_INIT_INT(sdl2seekmap, sdl2_seek)
KHASH_MAP_INIT_INT(sdl2readmap, sdl2_read)
KHASH_MAP_INIT_INT(sdl2writemap, sdl2_write)
KHASH_MAP_INIT_INT(sdl2closemap, sdl2_close)


typedef struct sdl2rwops_s {
    khash_t(sdl2sizemap)        *sizemap;
    khash_t(sdl2seekmap)        *seekmap;
    khash_t(sdl2readmap)        *readmap;
    khash_t(sdl2writemap)       *writemap;
    khash_t(sdl2closemap)       *closemap;
} sdl2rwops_t;

sdl2rwops_t* NewSDL2RWops()
{
    sdl2rwops_t* rw = (sdl2rwops_t*)calloc(1, sizeof(sdl2rwops_t));
    rw->sizemap = kh_init(sdl2sizemap);
    rw->seekmap = kh_init(sdl2seekmap);
    rw->readmap = kh_init(sdl2readmap);
    rw->writemap = kh_init(sdl2writemap);
    rw->closemap = kh_init(sdl2closemap);
    return rw;
}

void FreeSDL2RWops(sdl2rwops_t **rw)
{
    // no cleaning of bridges, as they can be used multiple time?
    kh_destroy(sdl2sizemap, (*rw)->sizemap);
    kh_destroy(sdl2seekmap, (*rw)->seekmap);
    kh_destroy(sdl2readmap, (*rw)->readmap);
    kh_destroy(sdl2writemap, (*rw)->writemap);
    kh_destroy(sdl2closemap, (*rw)->closemap);
    free(*rw);
    *rw = NULL;
}

void AddNativeRW2(x86emu_t* emu, SDL2_RWops_t* ops)
{
    if(!ops)
        return;
    uintptr_t fnc;
    sdl2rwops_t* rw = (sdl2rwops_t*)emu->context->sdl2lib->priv.w.priv;
    bridge_t* system = emu->context->system;
    khint_t k;
    int ret;

    // get or create wrapper, add it to map and change to the emulated one if rw
    #define GO(A, W) \
    fnc = CheckBridged(system, ops->A); \
    if(!fnc) fnc = AddBridge(system, W, ops->A); \
    k = kh_put(sdl2##A##map, rw->A##map, fnc, &ret); \
    kh_value(rw->A##map, k) = ops->A; \
    ops->A = (sdl2_##A)fnc;

    GO(size, IFp)
    GO(seek, IFpIi)
    GO(read, iFppii)
    GO(write, iFppii)
    GO(close, iFp)

    #undef GO
}

// check if one of the function is a pure emulated one (i.e. not present in dictionnary)
static int isAnyEmulated(sdl2rwops_t* rw, SDL2_RWops_t *ops)
{
    #define GO(A) if((kh_get(sdl2##A##map, rw->A##map, (uintptr_t)ops->A))==(kh_end(rw->A##map))) return 1
    GO(size);
    GO(seek);
    GO(read);
    GO(write);
    GO(close);
    #undef GO
    return 0;
}

// put Native RW function, wrapping emulated (callback style) ones if needed
void RWNativeStart2(x86emu_t* emu, SDL2_RWops_t* ops, SDL2RWSave_t* save)
{
    if(!ops)
        return;
    sdl2rwops_t* rw = (sdl2rwops_t*)emu->context->sdl2lib->priv.w.priv;
    
    save->anyEmu = isAnyEmulated(rw, ops);
    save->size = ops->size;
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
        k = kh_get(sdl2##A##map, rw->A##map, (uintptr_t)ops->A); \
        ops->A = kh_value(rw->A##map, k);
        GO(size)
        GO(seek)
        GO(read)
        GO(write)
        GO(close)
        #undef GO
    }
}

// put back emulated function back in place
void RWNativeEnd2(x86emu_t* emu, SDL2_RWops_t* ops, SDL2RWSave_t* save)
{
    if(!ops)
        return;
    sdl2rwops_t* rw = (sdl2rwops_t*)emu->context->sdl2lib->priv.w.priv;

    ops->size = save->size;
    ops->seek = save->seek;
    ops->read = save->read;
    ops->write = save->write;
    ops->close = save->close;
    if(save->anyEmu) {
        save->s1 = ops->hidden.mem.base;
        save->s2 = ops->hidden.mem.here;
    }
}
