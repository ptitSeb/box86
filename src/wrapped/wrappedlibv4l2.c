#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x86emu.h"
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include "debug.h"
const char* libv4l2Name = "libv4l2.so.0";
#define LIBNAME libv4l2

typedef int32_t (*iFpL_t)(void*, uintptr_t);
typedef void* (*pFpLiiii_t)(void*, uintptr_t, int32_t, int32_t, int32_t, int32_t);

static library_t* my_lib = NULL;

#define SUPER() \
	GO(v4l2_munmap, iFpL_t) \
	GO(v4l2_mmap, pFpLiiii_t)

typedef struct libv4l2_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} libv4l2_my_t;

void* getV4l2My(library_t* lib)
{
    libv4l2_my_t* my = (libv4l2_my_t*)calloc(1, sizeof(libv4l2_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER

void freeV4l2My(void* lib)
{

}


EXPORT void* my_v4l2_mmap(x86emu_t* emu, void *addr, unsigned long length, int prot, int flags, int fd, int offset)
{
    libv4l2_my_t *my = (libv4l2_my_t*)my_lib->priv.w.p2;
    if(prot&PROT_WRITE) 
        prot|=PROT_READ;    // PROT_READ is implicit with PROT_WRITE on i386
    if(box86_log<LOG_DEBUG) dynarec_log(LOG_DEBUG, "%s: %p :v4l2_mmap(%p, %lu, 0x%x, 0x%x, %d, %d) =>", __FUNCTION__, my->v4l2_mmap, addr, length, prot, flags, fd, offset);

    void* ret = my->v4l2_mmap(addr, length, prot, flags, fd, offset);
    if (ret == (void*)-1) 
    {
        dynarec_log(LOG_DEBUG, "v4l2_mmap failed:%s errno = %d, try mmap\n", strerror(errno), errno);
        ret = mmap(addr, length, prot, flags, fd, offset);
        dynarec_log(LOG_DEBUG, "mmap ret = %p\n", ret);
        
    }
    if(box86_log<LOG_DEBUG) {dynarec_log(LOG_DEBUG, "%p\n", ret);}
    #ifdef DYNAREC
    if(box86_dynarec && ret!=(void*)-1) {
        if(flags&0x100000 && addr!=ret)
        {
            // program used MAP_FIXED_NOREPLACE but the host linux didn't support it
            // and responded with a different address, so ignore it
        } else {
            if(prot& PROT_EXEC)
                addDBFromAddressRange((uintptr_t)ret, length);
            else
                cleanDBFromAddressRange((uintptr_t)ret, length, prot?0:1);
        }
    } 
    #endif
    if(ret!=(void*)-1)
        setProtection((uintptr_t)ret, length, prot);
    return ret;
}

EXPORT int my_v4l2_munmap(x86emu_t* emu, void* addr, unsigned long length)
{
    libv4l2_my_t *my = (libv4l2_my_t*)my_lib->priv.w.p2;
    dynarec_log(LOG_DEBUG, "v4l2_munmap(%p, %lu)\n", addr, length);
    #ifdef DYNAREC
    if(box86_dynarec) {
        cleanDBFromAddressRange((uintptr_t)addr, length, 1);
    }
    #endif
    int ret = my->v4l2_munmap(addr, length);
    if(!ret)
        setProtection((uintptr_t)addr, length, 0);
    return ret;
}

#define CUSTOM_INIT \
    my_lib = lib;   \
    lib->priv.w.p2 = getV4l2My(lib);

#define CUSTOM_FINI \
    freeV4l2My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"