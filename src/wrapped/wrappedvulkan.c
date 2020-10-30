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
#include "box86context.h"
#include "librarian.h"
#include "callback.h"

//extern char* libvulkan;

const char* vulkanName = "libvulkan.so.1";
#define LIBNAME vulkan
static library_t *my_lib = NULL;

typedef int (*iFpp_t)       (void*, void*);
typedef void*(*pFpp_t)      (void*, void*);
typedef int (*iFppp_t)      (void*, void*, void*);
typedef int (*iFpppp_t)     (void*, void*, void*, void*);
typedef int (*iFppppp_t)    (void*, void*, void*, void*, void*);
typedef int (*iFpuppp_t)    (void*, uint32_t, void*, void*, void*);
typedef int (*iFppuppp_t)   (void*, void*, uint32_t, void*, void*, void*);

#define SUPER() \
    GO(vkAllocateMemory, iFpppp_t)                  \
    GO(vkCreateBuffer, iFpppp_t)                    \
    GO(vkCreateBufferView, iFpppp_t)                \
    GO(vkCreateCommandPool, iFpppp_t)               \
    GO(vkCreateComputePipelines, iFppuppp_t)        \
    GO(vkCreateDescriptorPool, iFpppp_t)            \
    GO(vkCreateDescriptorSetLayout, iFpppp_t)       \
    GO(vkCreateDescriptorUpdateTemplate, iFpppp_t)  \
    GO(vkCreateDevice, iFpppp_t)                    \
    GO(vkCreateDisplayModeKHR, iFppppp_t)           \
    GO(vkCreateDisplayPlaneSurfaceKHR, iFpppp_t)    \
    GO(vkCreateEvent, iFpppp_t)                     \
    GO(vkCreateFence, iFpppp_t)                     \
    GO(vkCreateFramebuffer, iFpppp_t)               \
    GO(vkCreateGraphicsPipelines, iFppuppp_t)       \
    GO(vkCreateImage, iFpppp_t)                     \
    GO(vkCreateImageView, iFpppp_t)                 \
    GO(vkCreateInstance, iFppp_t)                   \
    GO(vkCreatePipelineCache, iFpppp_t)             \
    GO(vkCreatePipelineLayout, iFpppp_t)            \
    GO(vkCreateQueryPool, iFpppp_t)                 \
    GO(vkCreateRenderPass, iFpppp_t)                \
    GO(vkCreateSampler, iFpppp_t)                   \
    GO(vkCreateSamplerYcbcrConversion, iFpppp_t)    \
    GO(vkCreateSemaphore, iFpppp_t)                 \
    GO(vkCreateShaderModule, iFpppp_t)              \
    GO(vkCreateSharedSwapchainsKHR, iFpuppp_t)      \
    GO(vkCreateSwapchainKHR, iFpppp_t)              \
    GO(vkCreateWaylandSurfaceKHR, iFpppp_t)         \
    GO(vkCreateXcbSurfaceKHR, iFpppp_t)             \
    GO(vkCreateXlibSurfaceKHR, iFpppp_t)            \
    GO(vkDestroyBuffer, iFppp_t)                    \
    GO(vkDestroyBufferView, iFppp_t)                \
    GO(vkDestroyCommandPool, iFppp_t)               \
    GO(vkDestroyDescriptorPool, iFppp_t)            \
    GO(vkDestroyDescriptorSetLayout, iFppp_t)       \
    GO(vkDestroyDescriptorUpdateTemplate, iFppp_t)  \
    GO(vkDestroyDevice, iFpp_t)                     \
    GO(vkDestroyEvent, iFppp_t)                     \
    GO(vkDestroyFence, iFppp_t)                     \
    GO(vkDestroyFramebuffer, iFppp_t)               \
    GO(vkDestroyImage, iFppp_t)                     \
    GO(vkDestroyImageView, iFppp_t)                 \
    GO(vkDestroyInstance, iFpp_t)                   \
    GO(vkDestroyPipeline, iFppp_t)                  \
    GO(vkDestroyPipelineCache, iFppp_t)             \
    GO(vkDestroyPipelineLayout, iFppp_t)            \
    GO(vkDestroyQueryPool, iFppp_t)                 \
    GO(vkDestroyRenderPass, iFppp_t)                \
    GO(vkDestroySampler, iFppp_t)                   \
    GO(vkDestroySamplerYcbcrConversion, iFppp_t)    \
    GO(vkDestroySemaphore, iFppp_t)                 \
    GO(vkDestroyShaderModule, iFppp_t)              \
    GO(vkDestroySurfaceKHR, iFppp_t)                \
    GO(vkDestroySwapchainKHR, iFppp_t)              \
    GO(vkFreeMemory, iFppp_t)                       \
    GO(vkGetDeviceProcAddr, pFpp_t)                 \
    GO(vkGetInstanceProcAddr, pFpp_t)               \

typedef struct vulkan_my_s {
    // functions
    #define GO(A, B)    B   A;
    SUPER()
    #undef GO
} vulkan_my_t;

void* getVulkanMy(library_t* lib)
{
    vulkan_my_t* my = (vulkan_my_t*)calloc(1, sizeof(vulkan_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    SUPER()
    #undef GO
    return my;
}
void freeVulkanMy(void* p)
{
    //vulkan_my_t* my = (vulkan_my_t*)p;
}
#undef SUPER

typedef struct my_VkAllocationCallbacks_s {
    void*   pUserData;
    void*   pfnAllocation;
    void*   pfnReallocation;
    void*   pfnFree;
    void*   pfnInternalAllocation;
    void*   pfnInternalFree;
} my_VkAllocationCallbacks_t;


#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// Allocation ...
#define GO(A)   \
static uintptr_t my_Allocation_fct_##A = 0;                                         \
static void* my_Allocation_##A(void* a, size_t b, size_t c, int d)                  \
{                                                                                   \
    return (void*)RunFunction(my_context, my_Allocation_fct_##A, 4, a, b, c, d);    \
}
SUPER()
#undef GO
static void* find_Allocation_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Allocation_fct_##A == (uintptr_t)fct) return my_Allocation_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Allocation_fct_##A == 0) {my_Allocation_fct_##A = (uintptr_t)fct; return my_Allocation_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Vulkan Allocation callback\n");
    return NULL;
}
// Reallocation ...
#define GO(A)   \
static uintptr_t my_Reallocation_fct_##A = 0;                                           \
static void* my_Reallocation_##A(void* a, void* b, size_t c, size_t d, int e)           \
{                                                                                       \
    return (void*)RunFunction(my_context, my_Reallocation_fct_##A, 5, a, b, c, d, e);   \
}
SUPER()
#undef GO
static void* find_Reallocation_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Reallocation_fct_##A == (uintptr_t)fct) return my_Reallocation_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Reallocation_fct_##A == 0) {my_Reallocation_fct_##A = (uintptr_t)fct; return my_Reallocation_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Vulkan Reallocation callback\n");
    return NULL;
}
// Free ...
#define GO(A)   \
static uintptr_t my_Free_fct_##A = 0;                       \
static void my_Free_##A(void* a, void* b)                   \
{                                                           \
    RunFunction(my_context, my_Free_fct_##A, 2, a, b);      \
}
SUPER()
#undef GO
static void* find_Free_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_Free_fct_##A == (uintptr_t)fct) return my_Free_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_Free_fct_##A == 0) {my_Free_fct_##A = (uintptr_t)fct; return my_Free_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Vulkan Free callback\n");
    return NULL;
}
// InternalAllocNotification ...
#define GO(A)   \
static uintptr_t my_InternalAllocNotification_fct_##A = 0;                          \
static void my_InternalAllocNotification_##A(void* a, size_t b, int c, int d)       \
{                                                                                   \
    RunFunction(my_context, my_InternalAllocNotification_fct_##A, 4, a, b, c, d);   \
}
SUPER()
#undef GO
static void* find_InternalAllocNotification_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_InternalAllocNotification_fct_##A == (uintptr_t)fct) return my_InternalAllocNotification_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_InternalAllocNotification_fct_##A == 0) {my_InternalAllocNotification_fct_##A = (uintptr_t)fct; return my_InternalAllocNotification_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Vulkan InternalAllocNotification callback\n");
    return NULL;
}
// InternalFreeNotification ...
#define GO(A)   \
static uintptr_t my_InternalFreeNotification_fct_##A = 0;                           \
static void my_InternalFreeNotification_##A(void* a, size_t b, int c, int d)        \
{                                                                                   \
    RunFunction(my_context, my_InternalFreeNotification_fct_##A, 4, a, b, c, d);    \
}
SUPER()
#undef GO
static void* find_InternalFreeNotification_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_InternalFreeNotification_fct_##A == (uintptr_t)fct) return my_InternalFreeNotification_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_InternalFreeNotification_fct_##A == 0) {my_InternalFreeNotification_fct_##A = (uintptr_t)fct; return my_InternalFreeNotification_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Vulkan InternalFreeNotification callback\n");
    return NULL;
}

#undef SUPER

void fillVulkanProcWrapper(box86context_t*);
void freeVulkanProcWrapper(box86context_t*);

static void* resolveSymbol(x86emu_t* emu, void* symbol, const char* rname)
{
    // check if alread bridged
    uintptr_t ret = CheckBridged(emu->context->system, symbol);
    if(ret) {
        if(dlsym_error && box86_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", (void*)ret);
        return (void*)ret; // already bridged
    }
    // get wrapper    
    khint_t k = kh_get(symbolmap, emu->context->vkwrappers, rname);
    if(k==kh_end(emu->context->vkwrappers) && strstr(rname, "KHR")==NULL) {
        // try again, adding KHR at the end if not present
        char tmp[200];
        strcpy(tmp, rname);
        strcat(tmp, "KHR");
        k = kh_get(symbolmap, emu->context->vkwrappers, tmp);
    }
    if(k==kh_end(emu->context->vkwrappers)) {
        if(dlsym_error && box86_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", NULL);
        if(dlsym_error && box86_log<LOG_INFO) printf_log(LOG_NONE, "Warning, no wrapper for %s\n", rname);
        return NULL;
    }
    AddOffsetSymbol(emu->context->maplib, symbol, rname);
    ret = AddBridge(emu->context->system, kh_value(emu->context->vkwrappers, k), symbol, 0);
    if(dlsym_error && box86_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", (void*)ret);
    return (void*)ret;
}

EXPORT void* my_vkGetDeviceProcAddr(x86emu_t* emu, void* device, void* name) 
{
    khint_t k;
    const char* rname = (const char*)name;
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;

    if(dlsym_error && box86_log<LOG_DEBUG) printf_log(LOG_NONE, "Calling my_vkGetDeviceProcAddr(%p, \"%s\") => ", device, rname);
    if(!emu->context->vkwrappers)
        fillVulkanProcWrapper(emu->context);
    // check if glxprocaddress is filled, and search for lib and fill it if needed
    // get proc adress using actual glXGetProcAddress
    k = kh_get(symbolmap, emu->context->vkmymap, rname);
    int is_my = (k==kh_end(emu->context->vkmymap))?0:1;
    void* symbol;
    if(is_my) {
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        symbol = dlsym(emu->context->box86lib, tmp);
    } else 
        symbol = my->vkGetDeviceProcAddr(device, name);
    if(!symbol) {
        if(dlsym_error && box86_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", NULL);
        return NULL;    // easy
    }
    return resolveSymbol(emu, symbol, rname);
}

EXPORT void* my_vkGetInstanceProcAddr(x86emu_t* emu, void* instance, void* name) 
{
    khint_t k;
    const char* rname = (const char*)name;

    if(dlsym_error && box86_log<LOG_DEBUG) printf_log(LOG_NONE, "Calling my_vkGetInstanceProcAddr(%p, \"%s\") => ", instance, rname);
    if(!emu->context->vkwrappers)
        fillVulkanProcWrapper(emu->context);
    // check if glxprocaddress is filled, and search for lib and fill it if needed
    // get proc adress using actual glXGetProcAddress
    k = kh_get(symbolmap, emu->context->vkmymap, rname);
    int is_my = (k==kh_end(emu->context->vkmymap))?0:1;
    void* symbol;
    if(is_my) {
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        symbol = dlsym(emu->context->box86lib, tmp);
    } else 
        symbol = emu->context->vkprocaddress(instance, rname);
    if(!symbol) {
        if(dlsym_error && box86_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", NULL);
        return NULL;    // easy
    }
    return resolveSymbol(emu, symbol, rname);
}

//#define PRE_INIT if(libGL) {lib->priv.w.lib = dlopen(libGL, RTLD_LAZY | RTLD_GLOBAL); lib->path = strdup(libGL);} else

#define CUSTOM_INIT \
    my_lib = lib; \
    lib->priv.w.p2 = getVulkanMy(lib);  \
    lib->priv.w.priv = dlsym(lib->priv.w.lib, "vkGetInstanceProcAddr"); \
    box86->vkprocaddress = lib->priv.w.priv;

#define CUSTOM_FINI \
    my_lib = NULL;  \
    freeVulkanMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

void fillVulkanProcWrapper(box86context_t* context)
{
    int cnt, ret;
    khint_t k;
    kh_symbolmap_t * symbolmap = kh_init(symbolmap);
    // populates maps...
    cnt = sizeof(vulkansymbolmap)/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, vulkansymbolmap[i].name, &ret);
        kh_value(symbolmap, k) = vulkansymbolmap[i].w;
    }
    // and the my_ symbols map
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, vulkanmysymbolmap[i].name, &ret);
        kh_value(symbolmap, k) = vulkanmysymbolmap[i].w;
    }
    context->vkwrappers = symbolmap;
    // my_* map
    symbolmap = kh_init(symbolmap);
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, vulkanmysymbolmap[i].name, &ret);
        kh_value(symbolmap, k) = vulkanmysymbolmap[i].w;
    }
    context->vkmymap = symbolmap;
}
void freeVulkanProcWrapper(box86context_t* context)
{
    if(!context)
        return;
    if(context->vkwrappers)
        kh_destroy(symbolmap, context->vkwrappers);
    if(context->vkmymap)
        kh_destroy(symbolmap, context->vkmymap);
    context->vkwrappers = NULL;
    context->vkmymap = NULL;
}

my_VkAllocationCallbacks_t* find_VkAllocationCallbacks(my_VkAllocationCallbacks_t* dest, my_VkAllocationCallbacks_t* src)
{
    if(!src) return src;
    dest->pUserData = src->pUserData;
    dest->pfnAllocation = find_Allocation_Fct(src->pfnAllocation);
    dest->pfnReallocation = find_Reallocation_Fct(src->pfnReallocation);
    dest->pfnFree = find_Free_Fct(src->pfnFree);
    dest->pfnInternalAllocation = find_InternalAllocNotification_Fct(src->pfnInternalAllocation);
    dest->pfnInternalFree = find_InternalFreeNotification_Fct(src->pfnInternalFree);
    return dest;
}
// functions....
#define CREATE(A)   \
EXPORT int my_##A(x86emu_t* emu, void* device, void* pAllocateInfo, my_VkAllocationCallbacks_t* pAllocator, void* p)    \
{                                                                                                                       \
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;                                                                  \
    my_VkAllocationCallbacks_t my_alloc;                                                                                \
    return my->A(device, pAllocateInfo, find_VkAllocationCallbacks(&my_alloc, pAllocator), p);                          \
}
#define DESTROY(A)   \
EXPORT int my_##A(x86emu_t* emu, void* device, void* p, my_VkAllocationCallbacks_t* pAllocator)                         \
{                                                                                                                       \
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;                                                                  \
    my_VkAllocationCallbacks_t my_alloc;                                                                                \
    return my->A(device, p, find_VkAllocationCallbacks(&my_alloc, pAllocator));                                         \
}

CREATE(vkAllocateMemory)
CREATE(vkCreateBuffer)
CREATE(vkCreateBufferView)
CREATE(vkCreateCommandPool)

EXPORT int my_vkCreateComputePipelines(x86emu_t* emu, void* device, void* pipelineCache, uint32_t count, void* pCreateInfos, my_VkAllocationCallbacks_t* pAllocator, void* pPipelines)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    my_VkAllocationCallbacks_t my_alloc;
    return my->vkCreateComputePipelines(device, pipelineCache, count, pCreateInfos, find_VkAllocationCallbacks(&my_alloc, pAllocator), pPipelines);
}

CREATE(vkCreateDescriptorPool)
CREATE(vkCreateDescriptorSetLayout)
CREATE(vkCreateDescriptorUpdateTemplate)
CREATE(vkCreateDevice)

EXPORT int my_vkCreateDisplayModeKHR(x86emu_t* emu, void* physical, void* device, void* pCreateInfo, my_VkAllocationCallbacks_t* pAllocator, void* pMode)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    my_VkAllocationCallbacks_t my_alloc;
    return my->vkCreateDisplayModeKHR(physical, device, pCreateInfo, find_VkAllocationCallbacks(&my_alloc, pAllocator), pMode);
}

CREATE(vkCreateDisplayPlaneSurfaceKHR)
CREATE(vkCreateEvent)
CREATE(vkCreateFence)
CREATE(vkCreateFramebuffer)

EXPORT int my_vkCreateGraphicsPipelines(x86emu_t* emu, void* device, void* pipelineCache, uint32_t count, void* pCreateInfos, my_VkAllocationCallbacks_t* pAllocator, void* pPipelines)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    my_VkAllocationCallbacks_t my_alloc;
    return my->vkCreateGraphicsPipelines(device, pipelineCache, count, pCreateInfos, find_VkAllocationCallbacks(&my_alloc, pAllocator), pPipelines);
}

CREATE(vkCreateImage)
CREATE(vkCreateImageView)

EXPORT int my_vkCreateInstance(x86emu_t* emu, void* pCreateInfos, my_VkAllocationCallbacks_t* pAllocator, void* pInstance)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    my_VkAllocationCallbacks_t my_alloc;
    return my->vkCreateInstance(pCreateInfos, find_VkAllocationCallbacks(&my_alloc, pAllocator), pInstance);
}

CREATE(vkCreatePipelineCache)
CREATE(vkCreatePipelineLayout)
CREATE(vkCreateQueryPool)
CREATE(vkCreateRenderPass)
CREATE(vkCreateSampler)
CREATE(vkCreateSamplerYcbcrConversion)
CREATE(vkCreateSemaphore)
CREATE(vkCreateShaderModule)

EXPORT int my_vkCreateSharedSwapchainsKHR(x86emu_t* emu, void* device, uint32_t count, void* pCreateInfos, my_VkAllocationCallbacks_t* pAllocator, void* pSwapchains)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    my_VkAllocationCallbacks_t my_alloc;
    return my->vkCreateSharedSwapchainsKHR(device, count, pCreateInfos, find_VkAllocationCallbacks(&my_alloc, pAllocator), pSwapchains);
}

CREATE(vkCreateSwapchainKHR)
CREATE(vkCreateWaylandSurfaceKHR)
CREATE(vkCreateXcbSurfaceKHR)
CREATE(vkCreateXlibSurfaceKHR)

DESTROY(vkDestroyBuffer)
DESTROY(vkDestroyBufferView)
DESTROY(vkDestroyCommandPool)
DESTROY(vkDestroyDescriptorPool)
DESTROY(vkDestroyDescriptorSetLayout)
DESTROY(vkDestroyDescriptorUpdateTemplate)

EXPORT int my_vkDestroyDevice(x86emu_t* emu, void* pDevice, my_VkAllocationCallbacks_t* pAllocator)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    my_VkAllocationCallbacks_t my_alloc;
    return my->vkDestroyDevice(pDevice, find_VkAllocationCallbacks(&my_alloc, pAllocator));
}

DESTROY(vkDestroyEvent)
DESTROY(vkDestroyFence)
DESTROY(vkDestroyFramebuffer)
DESTROY(vkDestroyImage)
DESTROY(vkDestroyImageView)

EXPORT int my_vkDestroyInstance(x86emu_t* emu, void* instance, my_VkAllocationCallbacks_t* pAllocator)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    my_VkAllocationCallbacks_t my_alloc;
    return my->vkDestroyInstance(instance, find_VkAllocationCallbacks(&my_alloc, pAllocator));
}

DESTROY(vkDestroyPipeline)
DESTROY(vkDestroyPipelineCache)
DESTROY(vkDestroyPipelineLayout)
DESTROY(vkDestroyQueryPool)
DESTROY(vkDestroyRenderPass)
DESTROY(vkDestroySampler)
DESTROY(vkDestroySamplerYcbcrConversion)
DESTROY(vkDestroySemaphore)
DESTROY(vkDestroyShaderModule)
DESTROY(vkDestroySurfaceKHR)
DESTROY(vkDestroySwapchainKHR)

DESTROY(vkFreeMemory)