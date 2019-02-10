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

void AddNativeRW(x86emu_t* emu, SDL1_RWops_t* ops)
{
    if(!ops)
        return;
    uintptr_t fnc;
    bridge_t* system = emu->context->system;

    // get or create wrapper, add it to map and change to the emulated one if rw
    #define GO(A, W) \
    fnc = CheckBridged(system, ops->A); \
    if(!fnc) fnc = AddBridge(system, W, ops->A); \
    ops->A = (sdl1_##A)fnc;

    GO(seek, iFpii)
    GO(read, iFppii)
    GO(write, iFppii)
    GO(close, iFp)

    #undef GO
}

// check if one of the function is a pure emulated one (i.e. not present in dictionnary)
static int isAnyEmulated(SDL1_RWops_t *ops)
{
    #define GO(A) if(!GetNativeFnc((uintptr_t)ops->A)) return 1
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
    if(!ops)
        return;
    
    save->anyEmu = isAnyEmulated(ops);
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
        ops->A = GetNativeFnc((uintptr_t)ops->A);
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
    if(!ops)
        return;

    ops->seek = save->seek;
    ops->read = save->read;
    ops->write = save->write;
    ops->close = save->close;
    if(save->anyEmu) {
        save->s1 = ops->hidden.mem.base;
        save->s2 = ops->hidden.mem.here;
    }
}
