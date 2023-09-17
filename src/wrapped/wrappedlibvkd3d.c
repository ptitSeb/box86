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

#define ADDED_FUNCTIONS()           \

#include "generated/wrappedlibvkd3dtypes.h"

#include "wrappercallback.h"

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
    return (int)RunFunctionFmt(origin_signal_event, "p", event);
}
static uintptr_t origin_create_thread = 0;
static void* my_create_thread(PFN_vkd3d_thread thread_main, void *data)
{
    return (void*)RunFunctionFmt(origin_create_thread, "pp", thread_main, data);
}
static uintptr_t origin_join_thread = 0;
static int my_join_thread(void *thread)
{
    return (int)RunFunctionFmt(origin_join_thread, "p", thread);
}
EXPORT int my_vkd3d_create_instance(x86emu_t* emu, void *create_info, void *instance)
{
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
    getMy(lib);

#define CUSTOM_FINI \
    freeMy();

#include "wrappedlib_init.h"