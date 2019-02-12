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

void AddNativeRW2(x86emu_t* emu, SDL2_RWops_t* ops)
{
    if(!ops)
        return;
    uintptr_t fnc;
    bridge_t* system = emu->context->system;

    // get or create wrapper, add it to map and change to the emulated one if rw
    #define GO(A, W) \
    fnc = CheckBridged(system, ops->A); \
    if(!fnc) fnc = AddBridge(system, W, ops->A, 0); \
    ops->A = (sdl2_##A)fnc;

    GO(size, IFp)
    GO(seek, IFpIi)
    GO(read, iFppii)
    GO(write, iFppii)
    GO(close, iFp)

    #undef GO
}

// check if one of the function is a pure emulated one (i.e. not present in dictionnary)
static int isAnyEmulated2(SDL2_RWops_t *ops)
{
    #define GO(A) if(!GetNativeFnc((uintptr_t)ops->A)) return 1
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
    
    save->anyEmu = isAnyEmulated2(ops);
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
        #define GO(A) \
        ops->A = GetNativeFnc((uintptr_t)ops->A);
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
