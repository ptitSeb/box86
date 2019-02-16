#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "library_private.h"
#include "x86emu.h"
#include "x86emu_private.h"
#include "callback.h"
#include "box86context.h"
#include "librarian.h"

const char* vorbisfileName = "libvorbisfile.so.3";
#define LIBNAME vorbisfile

typedef struct {
  size_t (*read_func)  (void *ptr, size_t size, size_t nmemb, void *datasource);
  int    (*seek_func)  (void *datasource, int64_t offset, int whence);
  int    (*close_func) (void *datasource);
  long   (*tell_func)  (void *datasource);
} ov_callbacks;


typedef int32_t (*iFpppiC_t)(void*, void*, void*, int32_t, ov_callbacks);

typedef struct vorbisfile_my_s {
    // functions
    iFpppiC_t       ov_open_callbacks;
} vorbisfile_my_t;

void* getVorbisfileMy(library_t* lib)
{
    vorbisfile_my_t* my = (vorbisfile_my_t*)calloc(1, sizeof(vorbisfile_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(ov_open_callbacks, iFpppiC_t)
    #undef GO
    return my;
}

void freeVorbisfileMy(void* lib)
{
    vorbisfile_my_t *my = (vorbisfile_my_t *)lib;
}

int32_t my_ov_open_callbacks(x86emu_t* emu, void* datasource, void* vf, void* initial, int32_t ibytes, void* read, void* seek, void* close, void* tell);

#define CUSTOM_INIT \
    lib->priv.w.p2 = getVorbisfileMy(lib);

#define CUSTOM_FINI \
    freeVorbisfileMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

typedef union ui64_s {
    int64_t     i;
    uint64_t    u;
    uint32_t    d[2];
} ui64_t;

size_t my_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    x86emu_t *emu = (x86emu_t*)datasource;
    SetCallbackNArg(emu, 4);
    SetCallbackArg(emu, 0, ptr);
    SetCallbackArg(emu, 1, (void*)size);
    SetCallbackArg(emu, 2, (void*)nmemb);
    void* fnc = GetCallbackArg(emu, 5);
    SetCallbackAddress(emu, (uintptr_t)fnc);
    return RunCallback(emu);
}
int my_seek_func(void *datasource, int64_t offset, int whence)
{
    x86emu_t *emu = (x86emu_t*)datasource;
    SetCallbackNArg(emu, 4);    // because offset is 64bits...
    SetCallbackArg(emu, 0, GetCallbackArg(emu, 3));
    ui64_t ofs;
    ofs.i = offset;
    SetCallbackArg(emu, 1, (void*)ofs.d[0]);
    SetCallbackArg(emu, 2, (void*)ofs.d[1]);
    SetCallbackArg(emu, 3, (void*)whence);
    void* fnc = GetCallbackArg(emu, 6);
    SetCallbackAddress(emu, (uintptr_t)fnc);
    return RunCallback(emu);
}
int my_close_func(void *datasource)
{
    x86emu_t *emu = (x86emu_t*)datasource;
    SetCallbackNArg(emu, 1);
    SetCallbackArg(emu, 0, GetCallbackArg(emu, 3));
    void* fnc = GetCallbackArg(emu, 7);
    int r = 0;
    if(fnc) {
        SetCallbackAddress(emu, (uintptr_t)fnc);
        r = RunCallback(emu);
    }
    FreeCallback(emu);
    return r;
}
long my_tell_func(void *datasource)
{
    x86emu_t *emu = (x86emu_t*)datasource;
    SetCallbackNArg(emu, 1);
    SetCallbackArg(emu, 0, GetCallbackArg(emu, 3));
    void* fnc = GetCallbackArg(emu, 8);
    SetCallbackAddress(emu, (uintptr_t)fnc);
    return RunCallback(emu);
}

EXPORT int32_t my_ov_open_callbacks(x86emu_t* emu, void* datasource, void* vf, void* initial, int32_t ibytes, void* read, void* seek, void* close, void* tell)
{
    library_t * lib = GetLib(emu->context->maplib, vorbisfileName);
    vorbisfile_my_t *my = (vorbisfile_my_t*)lib->priv.w.p2;
    // wrap all callbacks, add close if not there to free the callbackemu
    ov_callbacks cbs = {0};
    x86emu_t* cbemu = AddCallback(emu, (uintptr_t)read, 3, NULL, NULL, NULL, datasource);
    SetCallbackArg(emu, 5, read);
    SetCallbackArg(emu, 6, seek);
    SetCallbackArg(emu, 7, close);
    SetCallbackArg(emu, 8, tell);
    cbs.read_func = my_read_func;
    if(seek) cbs.seek_func = my_seek_func;
    cbs.close_func = my_close_func;
    if(tell) cbs.tell_func = my_tell_func;
    return my->ov_open_callbacks(cbemu, vf, initial, ibytes, cbs);
}