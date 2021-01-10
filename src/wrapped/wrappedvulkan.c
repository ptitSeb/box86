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
#include "libtools/vkalign.h"

//extern char* libvulkan;

const char* vulkanName = "libvulkan.so.1";
#define LIBNAME vulkan
static library_t *my_lib = NULL;

typedef void(*vFpp_t)       (void*, void*);
typedef void*(*pFpp_t)      (void*, void*);
typedef int (*iFppp_t)      (void*, void*, void*);
typedef void(*vFppp_t)      (void*, void*, void*);
typedef void(*vFpUp_t)      (void*, uint64_t, void*);
typedef int (*iFpppp_t)     (void*, void*, void*, void*);
typedef int (*iFpUup_t)     (void*, uint64_t, uint32_t, void*);
typedef int (*iFppppp_t)    (void*, void*, void*, void*, void*);
typedef int (*iFpuppp_t)    (void*, uint32_t, void*, void*, void*);
typedef int (*iFpUppp_t)    (void*, uint64_t, void*, void*, void*);
typedef void(*vFpupup_t)    (void*, uint32_t, void*, uint32_t, void*);
typedef int (*iFpUuppp_t)   (void*, uint64_t, uint32_t, void*, void*, void*);
typedef void(*vFpiiiiipp_t) (void*, int, int, int, int, int, void*, void*);
typedef void(*vFpiiiupupup_t)(void*, int, int, int, uint32_t, void*, uint32_t, void*, uint32_t, void*);

#define SUPER() \
    GO(vkAllocateMemory, iFpppp_t)                  \
    GO(vkCreateBuffer, iFpppp_t)                    \
    GO(vkCreateBufferView, iFpppp_t)                \
    GO(vkCreateCommandPool, iFpppp_t)               \
    GO(vkCreateComputePipelines, iFpUuppp_t)        \
    GO(vkCreateDescriptorPool, iFpppp_t)            \
    GO(vkCreateDescriptorSetLayout, iFpppp_t)       \
    GO(vkCreateDescriptorUpdateTemplate, iFpppp_t)  \
    GO(vkCreateDescriptorUpdateTemplateKHR, iFpppp_t)\
    GO(vkCreateDevice, iFpppp_t)                    \
    GO(vkCreateDisplayModeKHR, iFpUppp_t)           \
    GO(vkCreateDisplayPlaneSurfaceKHR, iFpppp_t)    \
    GO(vkCreateEvent, iFpppp_t)                     \
    GO(vkCreateFence, iFpppp_t)                     \
    GO(vkCreateFramebuffer, iFpppp_t)               \
    GO(vkCreateGraphicsPipelines, iFpUuppp_t)       \
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
    GO(vkDestroyBuffer, vFpUp_t)                    \
    GO(vkDestroyBufferView, vFpUp_t)                \
    GO(vkDestroyCommandPool, vFpUp_t)               \
    GO(vkDestroyDescriptorPool, vFpUp_t)            \
    GO(vkDestroyDescriptorSetLayout, vFpUp_t)       \
    GO(vkDestroyDescriptorUpdateTemplate, vFpUp_t)  \
    GO(vkDestroyDescriptorUpdateTemplateKHR, vFpUp_t)\
    GO(vkDestroyDevice, vFpp_t)                     \
    GO(vkDestroyEvent, vFpUp_t)                     \
    GO(vkDestroyFence, vFpUp_t)                     \
    GO(vkDestroyFramebuffer, vFpUp_t)               \
    GO(vkDestroyImage, vFpUp_t)                     \
    GO(vkDestroyImageView, vFpUp_t)                 \
    GO(vkDestroyInstance, vFpp_t)                   \
    GO(vkDestroyPipeline, vFpUp_t)                  \
    GO(vkDestroyPipelineCache, vFpUp_t)             \
    GO(vkDestroyPipelineLayout, vFpUp_t)            \
    GO(vkDestroyQueryPool, vFpUp_t)                 \
    GO(vkDestroyRenderPass, vFpUp_t)                \
    GO(vkDestroySampler, vFpUp_t)                   \
    GO(vkDestroySamplerYcbcrConversion, vFpUp_t)    \
    GO(vkDestroySamplerYcbcrConversionKHR, vFpUp_t) \
    GO(vkDestroySemaphore, vFpUp_t)                 \
    GO(vkDestroyShaderModule, vFpUp_t)              \
    GO(vkDestroySurfaceKHR, vFpUp_t)                \
    GO(vkDestroySwapchainKHR, vFpUp_t)              \
    GO(vkFreeMemory, vFpUp_t)                       \
    GO(vkGetDeviceProcAddr, pFpp_t)                 \
    GO(vkGetInstanceProcAddr, pFpp_t)               \
    GO(vkCreateDebugUtilsMessengerEXT, iFpppp_t)    \
    GO(vkDestroyDebugUtilsMessengerEXT, vFppp_t)    \
    GO(vkGetPhysicalDeviceProperties, vFpp_t)       \
    GO(vkGetPhysicalDeviceQueueFamilyProperties, vFppp_t)               \
    GO(vkGetPhysicalDeviceSparseImageFormatProperties, vFpiiiiipp_t)    \
    GO(vkUpdateDescriptorSets, vFpupup_t)           \
    GO(vkGetDisplayPlaneCapabilitiesKHR, iFpUup_t)  \
    GO(vkGetPhysicalDeviceDisplayPropertiesKHR, iFppp_t)                \
    GO(vkGetPhysicalDeviceMemoryProperties, vFpp_t) \
    GO(vkCmdPipelineBarrier, vFpiiiupupup_t)        \

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
    if(dlsym_error && box86_log<LOG_DEBUG) printf_log(LOG_NONE, "%p (%p)\n", (void*)ret, symbol);
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
    k = kh_get(symbolmap, emu->context->vkmymap, rname);
    int is_my = (k==kh_end(emu->context->vkmymap))?0:1;
    void* symbol;
    if(is_my) {
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        symbol = dlsym(emu->context->box86lib, tmp);
        // need to update symbol link maybe
        #define GO(A, W) if(!strcmp(rname, #A)) my->A = (W)my->vkGetDeviceProcAddr(device, name);
        SUPER()
        #undef GO
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
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;

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
        // need to update symbol link maybe
        #define GO(A, W) if(!strcmp(rname, #A)) my->A = (W)emu->context->vkprocaddress(instance, rname);;
        SUPER()
        #undef GO
    } else 
        symbol = emu->context->vkprocaddress(instance, rname);
    if(!symbol) {
        if(dlsym_error && box86_log<LOG_DEBUG) printf_log(LOG_NONE, "%p\n", NULL);
        return NULL;    // easy
    }
    return resolveSymbol(emu, symbol, rname);
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

//#define PRE_INIT if(libGL) {lib->priv.w.lib = dlopen(libGL, RTLD_LAZY | RTLD_GLOBAL); lib->path = strdup(libGL);} else

#define PRE_INIT        \
    if(box86_novulkan)  \
        return -1;

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
EXPORT void my_##A(x86emu_t* emu, void* device, void* p, my_VkAllocationCallbacks_t* pAllocator)                         \
{                                                                                                                       \
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;                                                                  \
    my_VkAllocationCallbacks_t my_alloc;                                                                                \
    my->A(device, p, find_VkAllocationCallbacks(&my_alloc, pAllocator));                                         \
}
#define DESTROY64(A)   \
EXPORT void my_##A(x86emu_t* emu, void* device, uint64_t p, my_VkAllocationCallbacks_t* pAllocator)                        \
{                                                                                                                       \
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;                                                                  \
    my_VkAllocationCallbacks_t my_alloc;                                                                                \
    my->A(device, p, find_VkAllocationCallbacks(&my_alloc, pAllocator));                                                \
}

CREATE(vkAllocateMemory)
CREATE(vkCreateBuffer)
CREATE(vkCreateBufferView)
CREATE(vkCreateCommandPool)

EXPORT int my_vkCreateComputePipelines(x86emu_t* emu, void* device, uint64_t pipelineCache, uint32_t count, void* pCreateInfos, my_VkAllocationCallbacks_t* pAllocator, void* pPipelines)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    my_VkAllocationCallbacks_t my_alloc;
    void* aligned;
    const char* desc="upiSupiiUppUUi";
    aligned = vkalignStruct(pCreateInfos, desc, count);
    int ret = my->vkCreateComputePipelines(device, pipelineCache, count, aligned, find_VkAllocationCallbacks(&my_alloc, pAllocator), pPipelines);
    vkunalignStruct(pCreateInfos, aligned, desc, count);
    return ret;
}

CREATE(vkCreateDescriptorPool)
CREATE(vkCreateDescriptorSetLayout)
CREATE(vkCreateDescriptorUpdateTemplate)
CREATE(vkCreateDescriptorUpdateTemplateKHR)
CREATE(vkCreateDevice)

EXPORT int my_vkCreateDisplayModeKHR(x86emu_t* emu, void* physical, uint64_t display, void* pCreateInfo, my_VkAllocationCallbacks_t* pAllocator, void* pMode)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    my_VkAllocationCallbacks_t my_alloc;
    return my->vkCreateDisplayModeKHR(physical, display, pCreateInfo, find_VkAllocationCallbacks(&my_alloc, pAllocator), pMode);
}

CREATE(vkCreateDisplayPlaneSurfaceKHR)
CREATE(vkCreateEvent)
CREATE(vkCreateFence)
CREATE(vkCreateFramebuffer)

EXPORT int my_vkCreateGraphicsPipelines(x86emu_t* emu, void* device, uint64_t pipelineCache, uint32_t count, void* pCreateInfos, my_VkAllocationCallbacks_t* pAllocator, void* pPipelines)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    my_VkAllocationCallbacks_t my_alloc;
    void* aligned;
    const char* desc="upiuppppppppppUUuUi"; //TODO: check if any substruct need alignement!
    aligned = vkalignStruct(pCreateInfos, desc, count);
    int ret = my->vkCreateGraphicsPipelines(device, pipelineCache, count, aligned, find_VkAllocationCallbacks(&my_alloc, pAllocator), pPipelines);
    vkunalignStruct(pCreateInfos, aligned, desc, count);
    return ret;
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

EXPORT int my_vkCreateSharedSwapchainsKHR(x86emu_t* emu, void* device, uint32_t count, void** pCreateInfos, my_VkAllocationCallbacks_t* pAllocator, void* pSwapchains)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    my_VkAllocationCallbacks_t my_alloc;
    void* aligned;
    const char* desc="upiUuiiuuuiiupiiiiU";
    aligned = vkalignStruct(pCreateInfos, desc, count);
    int ret = my->vkCreateSharedSwapchainsKHR(device, count, aligned, find_VkAllocationCallbacks(&my_alloc, pAllocator), pSwapchains);
    vkunalignStruct(pCreateInfos, aligned, desc, count);
    return ret;
}

CREATE(vkCreateSwapchainKHR)
CREATE(vkCreateWaylandSurfaceKHR)
CREATE(vkCreateXcbSurfaceKHR)
CREATE(vkCreateXlibSurfaceKHR)

DESTROY64(vkDestroyBuffer)
DESTROY64(vkDestroyBufferView)
DESTROY64(vkDestroyCommandPool)
DESTROY64(vkDestroyDescriptorPool)
DESTROY64(vkDestroyDescriptorSetLayout)
DESTROY64(vkDestroyDescriptorUpdateTemplate)
DESTROY64(vkDestroyDescriptorUpdateTemplateKHR)

EXPORT void my_vkDestroyDevice(x86emu_t* emu, void* pDevice, my_VkAllocationCallbacks_t* pAllocator)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    my_VkAllocationCallbacks_t my_alloc;
    my->vkDestroyDevice(pDevice, find_VkAllocationCallbacks(&my_alloc, pAllocator));
}

DESTROY64(vkDestroyEvent)
DESTROY64(vkDestroyFence)
DESTROY64(vkDestroyFramebuffer)
DESTROY64(vkDestroyImage)
DESTROY64(vkDestroyImageView)

EXPORT void my_vkDestroyInstance(x86emu_t* emu, void* instance, my_VkAllocationCallbacks_t* pAllocator)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    my_VkAllocationCallbacks_t my_alloc;
    my->vkDestroyInstance(instance, find_VkAllocationCallbacks(&my_alloc, pAllocator));
}

DESTROY64(vkDestroyPipeline)
DESTROY64(vkDestroyPipelineCache)
DESTROY64(vkDestroyPipelineLayout)
DESTROY64(vkDestroyQueryPool)
DESTROY64(vkDestroyRenderPass)
DESTROY64(vkDestroySampler)
DESTROY64(vkDestroySamplerYcbcrConversion)
DESTROY64(vkDestroySemaphore)
DESTROY64(vkDestroyShaderModule)
DESTROY64(vkDestroySwapchainKHR)

DESTROY64(vkFreeMemory)

CREATE(vkCreateDebugUtilsMessengerEXT)
DESTROY(vkDestroyDebugUtilsMessengerEXT)

DESTROY64(vkDestroySurfaceKHR)

DESTROY64(vkDestroySamplerYcbcrConversionKHR)

EXPORT void my_vkGetPhysicalDeviceProperties(x86emu_t* emu, void* device, void* pProps)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    static const char* desc = 
    "uuuuiYB"
    "SuuuuuuuuuuuUUuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuffuuuffuLUUUiuiuffuuuuiiiiuiiiiiuifuuuuffffffiiUUU"
    "iiiii";
    void* my_props = malloc(vkalignSize(desc));
    my->vkGetPhysicalDeviceProperties(device, my_props);
    vkunalignStruct(pProps, my_props, desc, 0);
}

EXPORT void my_vkGetPhysicalDeviceSparseImageFormatProperties(x86emu_t* emu, void* device, int format, int type, int samples, int usage, int tiling, uint32_t* count, void** pProps)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    static const char* desc = "iuuui";
    if(!pProps)
        return my->vkGetPhysicalDeviceSparseImageFormatProperties(device, format, type, samples, usage, tiling, count, pProps);

    int sz = vkalignSize(desc);
    uint32_t cnt;
    my->vkGetPhysicalDeviceSparseImageFormatProperties(device, format, type, samples, usage, tiling, &cnt, NULL);
    void* my_props = malloc(cnt*sz);
    my->vkGetPhysicalDeviceSparseImageFormatProperties(device, format, type, samples, usage, tiling, count, my_props);
    vkunalignStruct(pProps, my_props, desc, cnt);
}

EXPORT void my_vkUpdateDescriptorSets(x86emu_t* emu, void* device, uint32_t writeCount, void* writeSet, uint32_t copyCount, void* copySet)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    static const char* writeDesc = "upUuuuippp";
    static const char* copyDesc = "upUuuUuuu";

    void* writeAligned = writeCount?vkalignStruct(writeSet, writeDesc, writeCount):NULL; // TODO: Align pNext....
    void* copyAligned = copyCount?vkalignStruct(copySet, copyDesc, copyCount):NULL; // TODO: Align pNext....
    my->vkUpdateDescriptorSets(device, writeCount, writeAligned, copyCount, copyAligned);
    if(writeCount)  vkunalignStruct(writeSet, writeAligned, writeDesc, writeCount);
    if(copyCount)   vkunalignStruct(copySet, copyAligned, copyDesc, copyCount);
}

EXPORT int my_vkGetDisplayPlaneCapabilitiesKHR(x86emu_t* emu, void* device, uint64_t mode, uint32_t index, void* pCap)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    static const char* desc = "iuuuuuuuuuuuuuuuu";

    void* aligned = vkalignStruct(pCap, desc, 0);
    int ret = my->vkGetDisplayPlaneCapabilitiesKHR(device, mode, index, aligned);
    vkunalignStruct(pCap, aligned, desc, 0);
    return ret;
}

EXPORT int my_vkGetPhysicalDeviceDisplayPropertiesKHR(x86emu_t* emu, void* device, uint32_t* count, void* pProp)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    static const char* desc = "Upuuuuiii";
    if(!pProp)
        return my->vkGetPhysicalDeviceDisplayPropertiesKHR(device, count, pProp);

    int32_t cnt = 0;
    my->vkGetPhysicalDeviceDisplayPropertiesKHR(device, &cnt, NULL);
    void* aligned = vkalignStruct(pProp, desc, cnt);
    int ret = my->vkGetPhysicalDeviceDisplayPropertiesKHR(device, count, aligned);
    vkunalignStruct(pProp, aligned, desc, cnt);
    return ret;
}

EXPORT void my_vkGetPhysicalDeviceMemoryProperties(x86emu_t* emu, void* device, void* pProps)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    static const char* desc = 
    "u" //uint32_t        memoryTypeCount;
    "iuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiu"  //VkMemoryType    memoryTypes[VK_MAX_MEMORY_TYPES]; //32
    "u" //uint32_t        memoryHeapCount;
    "UiUiUiUiUiUiUiUiUiUiUiUiUiUiUiUi"; //VkMemoryHeap    memoryHeaps[VK_MAX_MEMORY_HEAPS]; //16

    void* aligned = vkalignStruct(pProps, desc, 0);
    my->vkGetPhysicalDeviceMemoryProperties(device, aligned);
    vkunalignStruct(pProps, aligned, desc, 0);
}

EXPORT void my_vkCmdPipelineBarrier(x86emu_t* emu, void* device, int src, int dst, int dep, 
    uint32_t barrierCount, void* pBarriers, uint32_t bufferCount, void* pBuffers, uint32_t imageCount, void* pImages)
{
    vulkan_my_t* my = (vulkan_my_t*)my_lib->priv.w.p2;
    static const char* desc = "upiiiiuuUiuuuu";

    void* aligned = (imageCount)?vkalignStruct(pImages, desc, imageCount):NULL;
    my->vkCmdPipelineBarrier(device, src, dst, dep, barrierCount, pBarriers, bufferCount, pBuffers, imageCount, aligned);
    if(imageCount) vkunalignStruct(pImages, aligned, desc, imageCount);
}