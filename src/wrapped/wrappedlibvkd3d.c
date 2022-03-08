#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>
#include "wrappedlibs.h"
#include "wrapper.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "emu/x86emu_private.h"
#include "callback.h"
#include "x86emu.h"
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include "debug.h"
const char* libvkd3dName = "libvkd3d.so.1";
#define LIBNAME libvkd3d
typedef int32_t (*iFpp_t)(void*, void*);
static library_t* my_lib = NULL;
#define SUPER() \
	GO(vkd3d_create_instance, iFpp_t)
typedef struct libvkd3d_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} libvkd3d_my_t;
void* getVkd3dMy(library_t* lib)
{
    libvkd3d_my_t* my = (libvkd3d_my_t*)calloc(1, sizeof(libvkd3d_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
#undef SUPER
void freeVkd3dMy(void* lib)
{
}
typedef int (*PFN_vkd3d_signal_event)(void *event);
typedef void * (*PFN_vkd3d_thread)(void *data);
typedef void * (*PFN_vkd3d_create_thread)(PFN_vkd3d_thread thread_main, void *data);
typedef int (*PFN_vkd3d_join_thread)(void *thread);
typedef struct vkd3d_instance_create_info
{
    int type;
    const void *next;
    PFN_vkd3d_signal_event pfn_signal_event;
    PFN_vkd3d_create_thread pfn_create_thread;
    PFN_vkd3d_join_thread pfn_join_thread;
    size_t wchar_size;
    /* If set to NULL, libvkd3d loads libvulkan. */
    void* pfn_vkGetInstanceProcAddr;
    const char * const *instance_extensions;
    uint32_t instance_extension_count;
}vkd3d_instance_create_info;
static uintptr_t origin_signal_event = 0;
static int my_signal_event(void *event)
{
    return (int)RunFunction(my_context, origin_signal_event, 1, event);
}
static uintptr_t origin_create_thread = 0;
static void* my_create_thread(PFN_vkd3d_thread thread_main, void *data)
{
    return (void*)RunFunction(my_context, origin_create_thread, 2, thread_main, data);
}
static uintptr_t origin_join_thread = 0;
static int my_join_thread(void *thread)
{
    return (int)RunFunction(my_context, origin_join_thread, 1, thread);
}
EXPORT int my_vkd3d_create_instance(x86emu_t* emu, void *create_info, void *instance)
{
    libvkd3d_my_t *my = (libvkd3d_my_t*)my_lib->priv.w.p2;
    vkd3d_instance_create_info* my_create_info = (vkd3d_instance_create_info*)create_info;
    origin_signal_event = (uintptr_t)my_create_info->pfn_signal_event;
    origin_create_thread = (uintptr_t)my_create_info->pfn_create_thread;
    origin_join_thread = (uintptr_t)my_create_info->pfn_join_thread;
    my_create_info->pfn_signal_event = my_signal_event;
    my_create_info->pfn_create_thread = my_create_thread;
    my_create_info->pfn_join_thread = my_join_thread;
    my_create_info->pfn_vkGetInstanceProcAddr = emu->context->vkprocaddress;
    int ret = my->vkd3d_create_instance(create_info, instance);
    return ret;
}
#define CUSTOM_INIT \
    my_lib = lib;   \
    lib->priv.w.p2 = getVkd3dMy(lib);
#define CUSTOM_FINI \
    freeVkd3dMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"