#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x86emu.h"
#include "emu/x86emu_private.h"
#include "callback.h"
#include "librarian.h"
#include "box86context.h"
#include "emu/x86emu_private.h"
#include "khash.h"

const char* libsndfileName = "libsndfile.so.1";
#define LIBNAME libsndfile

typedef int   (*iFp_t)(void*);
typedef void* (*pFpipp_t)(void*, int32_t, void*, void*);

KHASH_MAP_INIT_INT(virtualio, x86emu_t*);

#define SUPER() \
    GO(sf_open_virtual, pFpipp_t)    \
    GO(sf_close, iFp_t)

typedef struct sndfile_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
    kh_virtualio_t  *map;
} sndfile_my_t;

void* getSndfileMy(library_t* lib)
{
    sndfile_my_t* my = (sndfile_my_t*)calloc(1, sizeof(sndfile_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    my->map = kh_init(virtualio);
    return my;
}
#undef SUPER

void freeSndfileMy(void* lib)
{
    sndfile_my_t *my = (sndfile_my_t *)lib;

    kh_destroy(virtualio, my->map); // clean the map?
}

typedef int64_t  (*sf_vio_get_filelen) (void *user_data) ;
typedef int64_t  (*sf_vio_seek)        (int64_t offset, int whence, void *user_data) ;
typedef int64_t  (*sf_vio_read)        (void *ptr, int64_t count, void *user_data) ;
typedef int64_t  (*sf_vio_write)       (const void *ptr, int64_t count, void *user_data) ;
typedef int64_t  (*sf_vio_tell)        (void *user_data) ;
typedef struct my_sfvirtual_io_s
{   sf_vio_get_filelen  get_filelen;
    sf_vio_seek         seek ;
    sf_vio_read         read ;
    sf_vio_write        write ;
    sf_vio_tell         tell ;
} my_sfvirtual_io_t;

static int64_t  my_sf_vio_get_filelen (void *user_data) {
    x86emu_t* emu = (x86emu_t*)user_data;
    SetCallbackNArg(emu, 1);
    SetCallbackArg(emu, 0, GetCallbackArg(emu, 4));
    SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 5));
    uint64_t ret = RunCallback(emu);
    ret |= ((uint64_t)R_EDX)<<32;
    return (int64_t)ret;
}
static int64_t  my_sf_vio_seek        (int64_t offset, int whence, void *user_data) {
    x86emu_t* emu = (x86emu_t*)user_data;
    SetCallbackNArg(emu, 4);
    SetCallbackArg(emu, 0, (void*)(uintptr_t)(offset&0xffffffff));
    SetCallbackArg(emu, 1, (void*)(uintptr_t)(offset>>32));
    SetCallbackArg(emu, 2, (void*)whence);

    SetCallbackArg(emu, 3, GetCallbackArg(emu, 4));
    SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 6));
    uint64_t ret = RunCallback(emu);
    ret |= ((uint64_t)R_EDX)<<32;
    return (int64_t)ret;
}
static int64_t  my_sf_vio_read        (void *ptr, int64_t count, void *user_data) {
    x86emu_t* emu = (x86emu_t*)user_data;
    SetCallbackNArg(emu, 4);
    SetCallbackArg(emu, 0, ptr);
    SetCallbackArg(emu, 1, (void*)(uintptr_t)(count&0xffffffff));
    SetCallbackArg(emu, 2, (void*)(uintptr_t)(count>>32));

    SetCallbackArg(emu, 3, GetCallbackArg(emu, 4));
    SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 7));
    uint64_t ret = RunCallback(emu);
    ret |= ((uint64_t)R_EDX)<<32;
    return (int64_t)ret;
}
static int64_t  my_sf_vio_write       (const void *ptr, int64_t count, void *user_data) {
    x86emu_t* emu = (x86emu_t*)user_data;
    SetCallbackNArg(emu, 4);
    SetCallbackArg(emu, 0, (void*)ptr);
    SetCallbackArg(emu, 1, (void*)(uintptr_t)(count&0xffffffff));
    SetCallbackArg(emu, 2, (void*)(uintptr_t)(count>>32));

    SetCallbackArg(emu, 3, GetCallbackArg(emu, 4));
    SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 8));
    uint64_t ret = RunCallback(emu);
    ret |= ((uint64_t)R_EDX)<<32;
    return (int64_t)ret;
}
static int64_t  my_sf_vio_tell        (void *user_data) {
    x86emu_t* emu = (x86emu_t*)user_data;
    SetCallbackNArg(emu, 1);
    SetCallbackArg(emu, 0, GetCallbackArg(emu, 4));
    SetCallbackAddress(emu, (uintptr_t)GetCallbackArg(emu, 9));
    uint64_t ret = RunCallback(emu);
    ret |= ((uint64_t)R_EDX)<<32;
    return (int64_t)ret;
}

EXPORT void* my_sf_open_virtual(x86emu_t* emu, my_sfvirtual_io_t* sfvirtual, int mode, void* sfinfo, void* data)
{
    my_sfvirtual_io_t native = {0};
    native.get_filelen = my_sf_vio_get_filelen;
    native.seek = my_sf_vio_seek;
    native.read = my_sf_vio_read;
    native.write = my_sf_vio_write;
    native.tell = my_sf_vio_tell;

    library_t * lib = GetLib(emu->context->maplib, libsndfileName);
    sndfile_my_t *my = (sndfile_my_t*)lib->priv.w.p2;

    x86emu_t *cb = AddCallback(emu, 0, 0, NULL, NULL, NULL, NULL);
    SetCallbackArg(cb, 4, data);
    SetCallbackArg(cb, 5, sfvirtual->get_filelen);
    SetCallbackArg(cb, 6, sfvirtual->seek);
    SetCallbackArg(cb, 7, sfvirtual->read);
    SetCallbackArg(cb, 8, sfvirtual->write);
    SetCallbackArg(cb, 9, sfvirtual->tell);
    void* sf = my->sf_open_virtual(&native, mode, sfinfo, cb);
    int ret;
    khint_t k;
    k = kh_put(virtualio, my->map, (uintptr_t)sf, &ret);
    kh_value(my->map, k) = cb;
    return sf;
}

EXPORT int my_sf_close(x86emu_t* emu, void* sf)
{
    library_t * lib = GetLib(emu->context->maplib, libsndfileName);
    sndfile_my_t *my = (sndfile_my_t*)lib->priv.w.p2;

    int ret = my->sf_close(sf);
    khint_t k = kh_get(virtualio, my->map, (uintptr_t)sf);
    if(k!=kh_end(my->map) && kh_exist(my->map, k)) {
        FreeCallback(kh_value(my->map, k));
        kh_del(virtualio, my->map, k);
    }
    return ret;
}

#define CUSTOM_INIT \
    lib->priv.w.p2 = getSndfileMy(lib);

#define CUSTOM_FINI \
    freeSndfileMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

