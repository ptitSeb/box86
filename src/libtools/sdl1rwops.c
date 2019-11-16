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
#include "emu/x86emu_private.h"
#include "librarian/library_private.h"
#include "bridge.h"
#include "callback.h"
#include "khash.h"

typedef struct SDL1_RWops_s SDL1_RWops_t;

typedef int32_t (*sdl1_seek)(SDL1_RWops_t *context, int32_t offset, int32_t whence);
typedef int32_t (*sdl1_read)(SDL1_RWops_t *context, void *ptr, int32_t size, int32_t maxnum);
typedef int32_t (*sdl1_write)(SDL1_RWops_t *context, const void *ptr, int32_t size, int32_t num);
typedef int32_t (*sdl1_close)(SDL1_RWops_t *context);

#define BOX86RW 0x72 // random signature value

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
        struct {
            SDL1_RWops_t *orig;
            sdl1_freerw custom_free;
            x86emu_t *emu;
        } my;
    } hidden;
} SDL1_RWops_t;


EXPORT int32_t my_native_seek(SDL1_RWops_t *context, int32_t offset, int32_t whence)
{
    return context->hidden.my.orig->seek(context->hidden.my.orig, offset, whence);
}
EXPORT int32_t my_native_read(SDL1_RWops_t *context, void *ptr, int32_t size, int32_t maxnum)
{
    return context->hidden.my.orig->read(context->hidden.my.orig, ptr, size, maxnum);
}
EXPORT int32_t my_native_write(SDL1_RWops_t *context, const void *ptr, int32_t size, int32_t num)
{
    return context->hidden.my.orig->write(context->hidden.my.orig, ptr, size, num);
}
EXPORT int32_t my_native_close(SDL1_RWops_t *context)
{
    int32_t ret = context->hidden.my.orig->close(context->hidden.my.orig);
    context->hidden.my.custom_free(context);
    return ret;
}
EXPORT int32_t my_emulated_seek(SDL1_RWops_t *context, int32_t offset, int32_t whence)
{
    x86emu_t *emu = context->hidden.my.emu;
    SetCallbackNArg(emu, 3);
    SetCallbackArg(emu, 0, context->hidden.my.orig);
    SetCallbackArg(emu, 1, (void*)offset);
    SetCallbackArg(emu, 2, (void*)whence);
    SetCallbackAddress(emu, (uintptr_t)context->hidden.my.orig->seek);
    RunCallback(emu);
    return (int32_t)GetEAX(emu);
}
EXPORT int32_t my_emulated_read(SDL1_RWops_t *context, void *ptr, int32_t size, int32_t maxnum)
{
    x86emu_t *emu = context->hidden.my.emu;
    SetCallbackNArg(emu, 4);
    SetCallbackArg(emu, 0, context->hidden.my.orig);
    SetCallbackArg(emu, 1, ptr);
    SetCallbackArg(emu, 2, (void*)size);
    SetCallbackArg(emu, 3, (void*)maxnum);
    SetCallbackAddress(emu, (uintptr_t)context->hidden.my.orig->read);
    RunCallback(emu);
    return (int32_t)GetEAX(emu);
}
EXPORT int32_t my_emulated_write(SDL1_RWops_t *context, const void *ptr, int32_t size, int32_t num)
{
    x86emu_t *emu = context->hidden.my.emu;
    SetCallbackNArg(emu, 4);
    SetCallbackArg(emu, 0, context->hidden.my.orig);
    SetCallbackArg(emu, 1, (void*)ptr);
    SetCallbackArg(emu, 2, (void*)size);
    SetCallbackArg(emu, 3, (void*)num);
    SetCallbackAddress(emu, (uintptr_t)context->hidden.my.orig->write);
    RunCallback(emu);
    return (int32_t)GetEAX(emu);
}
EXPORT int32_t my_emulated_close(SDL1_RWops_t *context)
{
    x86emu_t *emu = context->hidden.my.emu;
    SetCallbackNArg(emu, 1);
    SetCallbackArg(emu, 0, context->hidden.my.orig);
    SetCallbackAddress(emu, (uintptr_t)context->hidden.my.orig->close);
    int32_t ret = RunCallback(emu);
    FreeCallback(emu);
    return ret;
}

SDL1_RWops_t* AddNativeRW(x86emu_t* emu, SDL1_RWops_t* ops)
{
    if(!ops)
        return NULL;
    uintptr_t fnc;
    bridge_t* system = emu->context->system;

    sdl1_allocrw Alloc = (sdl1_allocrw)emu->context->sdl1allocrw;
    sdl1_freerw Free = (sdl1_freerw)emu->context->sdl1freerw;

    SDL1_RWops_t* newrw = Alloc();
    newrw->type = BOX86RW;
    newrw->hidden.my.orig = ops;
    newrw->hidden.my.custom_free = Free;

    // get or create wrapper, add it to map and change to the emulated one if rw
    #define GO(A, W) \
    fnc = CheckBridged(system, my_native_##A); \
    if(!fnc) fnc = AddBridge(system, W, my_native_##A, 0); \
    newrw->A = (sdl1_##A)fnc;

    GO(seek, iFpii)
    GO(read, iFppii)
    GO(write, iFppii)
    GO(close, iFp)

    #undef GO

    return newrw;
}

// put Native RW function, wrapping emulated (callback style) ones if needed
SDL1_RWops_t* RWNativeStart(x86emu_t* emu, SDL1_RWops_t* ops)
{
    if(!ops)
        return NULL;

    if(ops->type == BOX86RW)
        return ops->hidden.my.orig;

    sdl1_allocrw Alloc = (sdl1_allocrw)emu->context->sdl1allocrw;
    sdl1_freerw Free = (sdl1_freerw)emu->context->sdl1freerw;

    SDL1_RWops_t* newrw = Alloc();
    newrw->type = BOX86RW;
    newrw->hidden.my.orig = ops;
    newrw->hidden.my.emu = AddSmallCallback(emu, 0, 0, NULL, NULL, NULL, NULL);
   
    // create wrapper
    #define GO(A, W) \
    newrw->A = my_emulated_##A;

    GO(seek, iFpii)
    GO(read, iFppii)
    GO(write, iFppii)
    GO(close, iFp)

    #undef GO

    return newrw;
}

// put back emulated function back in place
void RWNativeEnd(SDL1_RWops_t* ops)
{
    if(!ops)
        return;

    if(ops->type != BOX86RW)
        return; // do nothing

    FreeCallback(ops->hidden.my.emu);
    ops->hidden.my.custom_free(ops);
}

int32_t RWNativeSeek(SDL1_RWops_t *ops, int32_t offset, int32_t whence)
{
    return ops->seek(ops, offset, whence);
}
uint32_t RWNativeRead(SDL1_RWops_t* ops, void* ptr, uint32_t size, uint32_t maxnum)
{
    return ops->read(ops, ptr, size, maxnum);
}
int32_t RWNativeWrite(SDL1_RWops_t *ops, const void *ptr, int32_t size, int32_t num)
{
    return ops->write(ops, ptr, size, num);
}
int32_t RWNativeClose(SDL1_RWops_t* ops)
{
    return ops->close(ops);
}