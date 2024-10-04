#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

typedef void(*vFpUp_t)      (void*, uint64_t, void*);

#define ADDED_FUNCTIONS()                           \
    GO(vkDestroySamplerYcbcrConversion, vFpUp_t)    \

#include "generated/wrappedvulkantypes.h"

#define ADDED_STRUCT()                              \
    void* currentInstance;  // track current instance. If using multiple instance, that will be a mess!

#define ADDED_SUPER 1
#include "wrappercallback.h"

void updateInstance(x86emu_t* emu, vulkan_my_t* my)
{
    void* p;
    #define GO(A, W) p = my_context->vkprocaddress(my->currentInstance, #A); if(p) my->A = p;
    SUPER()
    #undef GO
    symbol1_t* s;
    kh_foreach_value_ref(emu->context->vkwrappers, s, s->resolved = 0;)
}

void fillVulkanProcWrapper(box86context_t*);
void freeVulkanProcWrapper(box86context_t*);

static symbol1_t* getWrappedSymbol(x86emu_t* emu, const char* rname, int warning)
{
    khint_t k = kh_get(symbolmap, emu->context->vkwrappers, rname);
    if(k==kh_end(emu->context->vkwrappers) && strstr(rname, "KHR")==NULL) {
        // try again, adding KHR at the end if not present
        char tmp[200];
        strcpy(tmp, rname);
        strcat(tmp, "KHR");
        k = kh_get(symbolmap, emu->context->vkwrappers, tmp);
    }
    if(k==kh_end(emu->context->vkwrappers)) {
        if(warning) {
            printf_dlsym(LOG_DEBUG, "%p\n", NULL);
            printf_dlsym(LOG_INFO, "Warning, no wrapper for %s\n", rname);
        }
        return NULL;
    }
    return &kh_value(emu->context->vkwrappers, k);
}

static void* resolveSymbol(x86emu_t* emu, void* symbol, const char* rname)
{
    // get wrapper
    symbol1_t *s = getWrappedSymbol(emu, rname, 1);
    if(!s->resolved) {
        khint_t k = kh_get(symbolmap, emu->context->vkwrappers, rname);
        const char* constname = kh_key(emu->context->vkwrappers, k);
        s->addr = AddBridge(emu->context->system, s->w, symbol, 0, constname);
        s->resolved = 1;
    }
    void* ret = (void*)s->addr;
    printf_dlsym(LOG_DEBUG, "%p (%p)\n", ret, symbol);
    return ret;
}

EXPORT void* my_vkGetDeviceProcAddr(x86emu_t* emu, void* device, void* name) 
{
    khint_t k;
    const char* rname = (const char*)name;

    printf_dlsym(LOG_DEBUG, "Calling my_vkGetDeviceProcAddr(%p, \"%s\") => ", device, rname);
    if(!emu->context->vkwrappers)
        fillVulkanProcWrapper(emu->context);
    symbol1_t* s = getWrappedSymbol(emu, rname, 0);
    if(s && s->resolved) {
        void* ret = (void*)s->addr;
        printf_dlsym(LOG_DEBUG, "%p (cached)\n", ret);
        return ret;
    }
    k = kh_get(symbolmap, emu->context->vkmymap, rname);
    int is_my = (k==kh_end(emu->context->vkmymap))?0:1;
    void* symbol = my->vkGetDeviceProcAddr(device, name);
    if(symbol && is_my) {   // only wrap if symbol exist
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        symbol = dlsym(my_context->box86lib, tmp);
        // need to update symbol link maybe
        #define GO(A, W) if(!strcmp(rname, #A)) my->A = (W)my->vkGetDeviceProcAddr(device, name);
        SUPER()
        #undef GO
    } 
    if(!symbol) {
        printf_dlsym(LOG_DEBUG, "%p\n", NULL);
        return NULL;    // easy
    }
    return resolveSymbol(emu, symbol, rname);
}

EXPORT void* my_vkGetInstanceProcAddr(x86emu_t* emu, void* instance, void* name) 
{
    khint_t k;
    const char* rname = (const char*)name;

    printf_dlsym(LOG_DEBUG, "Calling my_vkGetInstanceProcAddr(%p, \"%s\") => ", instance, rname);
    if(!emu->context->vkwrappers)
        fillVulkanProcWrapper(emu->context);
    if(instance!=my->currentInstance) {
        my->currentInstance = instance;
        updateInstance(emu, my);
    }
    symbol1_t* s = getWrappedSymbol(emu, rname, 0);
    if(s && s->resolved) {
        void* ret = (void*)s->addr;
        printf_dlsym(LOG_DEBUG, "%p (cached)\n", ret);
        return ret;
    }
    // check if vkprocaddress is filled, and search for lib and fill it if needed
    // get proc adress using actual glXGetProcAddress
    k = kh_get(symbolmap, emu->context->vkmymap, rname);
    int is_my = (k==kh_end(emu->context->vkmymap))?0:1;
    void* symbol = my_context->vkprocaddress(instance, rname);
    if(!symbol) {
        printf_dlsym(LOG_DEBUG, "%p\n", NULL);
        return NULL;    // easy
    }
    if(is_my) {
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        symbol = dlsym(my_context->box86lib, tmp);
        // need to update symbol link maybe
        #define GO(A, W) if(!strcmp(rname, #A)) my->A = (W)my_context->vkprocaddress(instance, rname);;
        SUPER()
        #undef GO
    }
    return resolveSymbol(emu, symbol, rname);
}

void* my_GetVkProcAddr(x86emu_t* emu, void* name, void*(*getaddr)(const char*))
{
    khint_t k;
    const char* rname = (const char*)name;

    printf_dlsym(LOG_DEBUG, "Calling my_GetVkProcAddr(\"%s\", %p) => ", rname, getaddr);
    if(!emu->context->vkwrappers)
        fillVulkanProcWrapper(emu->context);
    symbol1_t* s = getWrappedSymbol(emu, rname, 0);
    if(s && s->resolved) {
        void* ret = (void*)s->addr;
        printf_dlsym(LOG_DEBUG, "%p (cached)\n", ret);
        return ret;
    }
    // check if vkprocaddress is filled, and search for lib and fill it if needed
    // get proc adress using actual glXGetProcAddress
    k = kh_get(symbolmap, emu->context->vkmymap, rname);
    int is_my = (k==kh_end(emu->context->vkmymap))?0:1;
    void* symbol = getaddr(rname);
    if(!symbol) {
        printf_dlsym(LOG_DEBUG, "%p\n", NULL);
        return NULL;    // easy
    }
    if(is_my) {
        // try again, by using custom "my_" now...
        char tmp[200];
        strcpy(tmp, "my_");
        strcat(tmp, rname);
        symbol = dlsym(my_context->box86lib, tmp);
        // need to update symbol link maybe
        #define GO(A, W) if(!strcmp(rname, #A)) my->A = (W)getaddr(rname);
        SUPER()
        #undef GO
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

typedef struct my_VkDebugReportCallbackCreateInfoEXT_s {
    int         sType;
    void*       pNext;
    uint32_t    flags;
    void*       pfnCallback;
    void*       pUserData;
} my_VkDebugReportCallbackCreateInfoEXT_t;

typedef struct my_VkDebugUtilsMessengerCreateInfoEXT_s {
    int          sType;
    const void*  pNext;
    int          flags;
    int          messageSeverity;
    int          messageType;
    void*        pfnUserCallback;
    void*        pUserData;
} my_VkDebugUtilsMessengerCreateInfoEXT_t;

typedef struct my_VkStruct_s {
    int         sType;
    struct my_VkStruct_s* pNext;
} my_VkStruct_t;

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// Allocation ...
#define GO(A)   \
static uintptr_t my_Allocation_fct_##A = 0;                                             \
static void* my_Allocation_##A(void* a, size_t b, size_t c, int d)                      \
{                                                                                       \
    return (void*)RunFunctionFmt(my_Allocation_fct_##A, "pLLi", a, b, c, d);\
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
static uintptr_t my_Reallocation_fct_##A = 0;                                                   \
static void* my_Reallocation_##A(void* a, void* b, size_t c, size_t d, int e)                   \
{                                                                                               \
    return (void*)RunFunctionFmt(my_Reallocation_fct_##A, "ppLLi", a, b, c, d, e);  \
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
    RunFunctionFmt(my_Free_fct_##A, "pp", a, b);\
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
static uintptr_t my_InternalAllocNotification_fct_##A = 0;                                  \
static void my_InternalAllocNotification_##A(void* a, size_t b, int c, int d)               \
{                                                                                           \
    RunFunctionFmt(my_InternalAllocNotification_fct_##A, "pLii", a, b, c, d);   \
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
static uintptr_t my_InternalFreeNotification_fct_##A = 0;                               \
static void my_InternalFreeNotification_##A(void* a, size_t b, int c, int d)            \
{                                                                                       \
    RunFunctionFmt(my_InternalFreeNotification_fct_##A, "pLii", a, b, c, d);\
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
// DebugReportCallbackEXT ...
#define GO(A)   \
static uintptr_t my_DebugReportCallbackEXT_fct_##A = 0;                                                        \
static int my_DebugReportCallbackEXT_##A(int a, int b, uint64_t c, size_t d, int e, void* f, void* g, void* h) \
{                                                                                                              \
    return RunFunctionFmt(my_DebugReportCallbackEXT_fct_##A, "iiULippp", a, b, c, d, e, f, g, h);  \
}
SUPER()
#undef GO
static void* find_DebugReportCallbackEXT_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DebugReportCallbackEXT_fct_##A == (uintptr_t)fct) return my_DebugReportCallbackEXT_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DebugReportCallbackEXT_fct_##A == 0) {my_DebugReportCallbackEXT_fct_##A = (uintptr_t)fct; return my_DebugReportCallbackEXT_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Vulkan DebugReportCallbackEXT callback\n");
    return NULL;
}
// DebugUtilsMessengerCallback ...
#define GO(A)   \
static uintptr_t my_DebugUtilsMessengerCallback_fct_##A = 0;                            \
static int my_DebugUtilsMessengerCallback_##A(int a, int b, void* c, void* d)           \
{                                                                                       \
    return RunFunctionFmt(my_DebugUtilsMessengerCallback_fct_##A, "iipp", a, b, c, d);  \
}
SUPER()
#undef GO
static void* find_DebugUtilsMessengerCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_DebugUtilsMessengerCallback_fct_##A == (uintptr_t)fct) return my_DebugUtilsMessengerCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_DebugUtilsMessengerCallback_fct_##A == 0) {my_DebugUtilsMessengerCallback_fct_##A = (uintptr_t)fct; return my_DebugUtilsMessengerCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for Vulkan DebugUtilsMessengerCallback callback\n");
    return NULL;
}

#undef SUPER

//#define PRE_INIT if(libGL) {lib->w.lib = dlopen(libGL, RTLD_LAZY | RTLD_GLOBAL); lib->path = box_strdup(libGL);} else

#define PRE_INIT        \
    if(box86_novulkan)  \
        return -1;

#define CUSTOM_INIT \
    my_lib = lib; \
    getMy(lib);  \
    lib->w.priv = dlsym(lib->w.lib, "vkGetInstanceProcAddr"); \
    box86->vkprocaddress = lib->w.priv;

#define CUSTOM_FINI \
    my_lib = NULL;  \
    freeMy();

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
        kh_value(symbolmap, k).w = vulkansymbolmap[i].w;
        kh_value(symbolmap, k).resolved = 0;
    }
    // and the my_ symbols map
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, vulkanmysymbolmap[i].name, &ret);
        kh_value(symbolmap, k).w = vulkanmysymbolmap[i].w;
        kh_value(symbolmap, k).resolved = 0;
    }
    context->vkwrappers = symbolmap;
    // my_* map
    symbolmap = kh_init(symbolmap);
    cnt = sizeof(MAPNAME(mysymbolmap))/sizeof(map_onesymbol_t);
    for (int i=0; i<cnt; ++i) {
        k = kh_put(symbolmap, symbolmap, vulkanmysymbolmap[i].name, &ret);
        kh_value(symbolmap, k).w = vulkanmysymbolmap[i].w;
        kh_value(symbolmap, k).resolved = 0;
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
    (void)emu;                                                                                                          \
    my_VkAllocationCallbacks_t my_alloc;                                                                                \
    return my->A(device, pAllocateInfo, find_VkAllocationCallbacks(&my_alloc, pAllocator), p);                          \
}
#define DESTROY(A)   \
EXPORT void my_##A(x86emu_t* emu, void* device, void* p, my_VkAllocationCallbacks_t* pAllocator)                        \
{                                                                                                                       \
    (void)emu;                                                                                                          \
    my_VkAllocationCallbacks_t my_alloc;                                                                                \
    my->A(device, p, find_VkAllocationCallbacks(&my_alloc, pAllocator));                                                \
}
#define DESTROY64(A)   \
EXPORT void my_##A(x86emu_t* emu, void* device, uint64_t p, my_VkAllocationCallbacks_t* pAllocator)                     \
{                                                                                                                       \
    (void)emu;                                                                                                          \
    my_VkAllocationCallbacks_t my_alloc;                                                                                \
    my->A(device, p, find_VkAllocationCallbacks(&my_alloc, pAllocator));                                                \
}

CREATE(vkAllocateMemory)
CREATE(vkCreateBuffer)
CREATE(vkCreateBufferView)
CREATE(vkCreateCommandPool)

EXPORT int my_vkCreateComputePipelines(x86emu_t* emu, void* device, uint64_t pipelineCache, uint32_t count, void* pCreateInfos, my_VkAllocationCallbacks_t* pAllocator, void* pPipelines)
{
    (void)emu;
    my_VkAllocationCallbacks_t my_alloc;
    void* aligned;
    const char* desc="upiSupiiUppUUi";
    aligned = vkalignStruct(pCreateInfos, desc, count);
    int ret = my->vkCreateComputePipelines(device, pipelineCache, count, aligned, find_VkAllocationCallbacks(&my_alloc, pAllocator), pPipelines);
    vkunalignStruct(aligned, desc, count);
    return ret;
}

CREATE(vkCreateDescriptorPool)
CREATE(vkCreateDescriptorSetLayout)
CREATE(vkCreateDescriptorUpdateTemplate)
CREATE(vkCreateDescriptorUpdateTemplateKHR)
CREATE(vkCreateDevice)

EXPORT int my_vkCreateDisplayModeKHR(x86emu_t* emu, void* physical, uint64_t display, void* pCreateInfo, my_VkAllocationCallbacks_t* pAllocator, void* pMode)
{
    (void)emu;
    my_VkAllocationCallbacks_t my_alloc;
    return my->vkCreateDisplayModeKHR(physical, display, pCreateInfo, find_VkAllocationCallbacks(&my_alloc, pAllocator), pMode);
}

CREATE(vkCreateDisplayPlaneSurfaceKHR)
CREATE(vkCreateEvent)
CREATE(vkCreateFence)
CREATE(vkCreateFramebuffer)

EXPORT int my_vkCreateGraphicsPipelines(x86emu_t* emu, void* device, uint64_t pipelineCache, uint32_t count, void* pCreateInfos, my_VkAllocationCallbacks_t* pAllocator, void* pPipelines)
{
    (void)emu;
    my_VkAllocationCallbacks_t my_alloc;
    void* aligned;
    const char* desc="upiuppppppppppUUuUi"; //TODO: check if any substruct need alignement!
    aligned = vkalignStruct(pCreateInfos, desc, count);
    int ret = my->vkCreateGraphicsPipelines(device, pipelineCache, count, aligned, find_VkAllocationCallbacks(&my_alloc, pAllocator), pPipelines);
    vkunalignStruct(aligned, desc, count);
    return ret;
}

CREATE(vkCreateImage)
CREATE(vkCreateImageView)

#define VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT 1000011000
#define VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT 1000128004
EXPORT int my_vkCreateInstance(x86emu_t* emu, void* pCreateInfos, my_VkAllocationCallbacks_t* pAllocator, void* pInstance)
{
    my_VkAllocationCallbacks_t my_alloc;
    my_VkStruct_t *p = (my_VkStruct_t*)pCreateInfos;
    void* old[20] = {0};
    int old_i = 0;
    while(p) {
        if(p->sType==VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT) {
            my_VkDebugReportCallbackCreateInfoEXT_t* vk = (my_VkDebugReportCallbackCreateInfoEXT_t*)p;
            old[old_i] = vk->pfnCallback;
            vk->pfnCallback = find_DebugReportCallbackEXT_Fct(old[old_i]);
            old_i++;
        } else if(p->sType==VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT) {
            my_VkDebugUtilsMessengerCreateInfoEXT_t* vk = (my_VkDebugUtilsMessengerCreateInfoEXT_t*)p;
            old[old_i] = vk->pfnUserCallback;
            vk->pfnUserCallback = find_DebugUtilsMessengerCallback_Fct(old[old_i]);
            old_i++;
        }
        p = p->pNext;
    }
    int ret = my->vkCreateInstance(pCreateInfos, find_VkAllocationCallbacks(&my_alloc, pAllocator), pInstance);
    if(old_i) {// restore, just in case it's re-used?
        p = (my_VkStruct_t*)pCreateInfos;
        old_i = 0;
        while(p) {
            if(p->sType==VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT) {
                my_VkDebugReportCallbackCreateInfoEXT_t* vk = (my_VkDebugReportCallbackCreateInfoEXT_t*)p;
                vk->pfnCallback = old[old_i];
                old_i++;
            } else if(p->sType==VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT) {
                my_VkDebugUtilsMessengerCreateInfoEXT_t* vk = (my_VkDebugUtilsMessengerCreateInfoEXT_t*)p;
                vk->pfnUserCallback = old[old_i];
                old_i++;
            }
            p = p->pNext;
        }
    }
    return ret;
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
    (void)emu;
    my_VkAllocationCallbacks_t my_alloc;
    void* aligned;
    const char* desc="upiUuiiuuuiiupiiiiU";
    aligned = vkalignStruct(pCreateInfos, desc, count);
    int ret = my->vkCreateSharedSwapchainsKHR(device, count, aligned, find_VkAllocationCallbacks(&my_alloc, pAllocator), pSwapchains);
    vkunalignStruct(aligned, desc, count);
    return ret;
}

CREATE(vkCreateSwapchainKHR)
CREATE(vkCreateWaylandSurfaceKHR)
CREATE(vkCreateXcbSurfaceKHR)
CREATE(vkCreateXlibSurfaceKHR)
CREATE(vkCreateRenderPass2)
CREATE(vkCreateRenderPass2KHR)

EXPORT int my_vkRegisterDeviceEventEXT(x86emu_t* emu, void* device, void* info, my_VkAllocationCallbacks_t* pAllocator, void* pFence)
{
    (void)emu;
    my_VkAllocationCallbacks_t my_alloc;
    return my->vkRegisterDeviceEventEXT(device, info, find_VkAllocationCallbacks(&my_alloc, pAllocator), pFence);
}
EXPORT int my_vkRegisterDisplayEventEXT(x86emu_t* emu, void* device, uint64_t disp, void* info, my_VkAllocationCallbacks_t* pAllocator, void* pFence)
{
    (void)emu;
    my_VkAllocationCallbacks_t my_alloc;
    return my->vkRegisterDisplayEventEXT(device, disp, info, find_VkAllocationCallbacks(&my_alloc, pAllocator), pFence);
}

CREATE(vkCreateValidationCacheEXT)

DESTROY64(vkDestroyBuffer)
DESTROY64(vkDestroyBufferView)
DESTROY64(vkDestroyCommandPool)
DESTROY64(vkDestroyDescriptorPool)
DESTROY64(vkDestroyDescriptorSetLayout)
DESTROY64(vkDestroyDescriptorUpdateTemplate)
DESTROY64(vkDestroyDescriptorUpdateTemplateKHR)

EXPORT void my_vkDestroyDevice(x86emu_t* emu, void* pDevice, my_VkAllocationCallbacks_t* pAllocator)
{
    (void)emu;
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
    (void)emu;
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

DESTROY64(vkDestroyValidationCacheEXT)

CREATE(vkCreatePrivateDataSlot)
CREATE(vkCreatePrivateDataSlotEXT)
DESTROY64(vkDestroyPrivateDataSlot)
DESTROY64(vkDestroyPrivateDataSlotEXT)

CREATE(vkCreateOpticalFlowSessionNV)
DESTROY64(vkDestroyOpticalFlowSessionNV)

CREATE(vkCreateVideoSessionKHR)
CREATE(vkCreateVideoSessionParametersKHR)
DESTROY64(vkDestroyVideoSessionKHR)
DESTROY64(vkDestroyVideoSessionParametersKHR)


EXPORT void my_vkGetPhysicalDeviceProperties(x86emu_t* emu, void* device, void* pProps)
{
    (void)emu;
    static const char* desc = 
        "uuuuiYB"
        "SuuuuuuuuuuuUUuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuffuuuffuLUUUiuiuffuuuuiiiiuiiiiiuifuuuuffffffiiUUU"
        "Siiiii";
    void* my_props = malloc(vkalignSize(desc));
    my->vkGetPhysicalDeviceProperties(device, my_props);
    vkunalignNewStruct(pProps, my_props, desc, 0);
}

EXPORT void my_vkGetPhysicalDeviceSparseImageFormatProperties(x86emu_t* emu, void* device, int format, int type, int samples, int usage, int tiling, uint32_t* count, void** pProps)
{
    (void)emu;
    static const char* desc = "iuuui";
    if(!pProps)
        return my->vkGetPhysicalDeviceSparseImageFormatProperties(device, format, type, samples, usage, tiling, count, pProps);

    int sz = vkalignSize(desc);
    uint32_t cnt;
    my->vkGetPhysicalDeviceSparseImageFormatProperties(device, format, type, samples, usage, tiling, &cnt, NULL);
    void* my_props = malloc(cnt*sz);
    my->vkGetPhysicalDeviceSparseImageFormatProperties(device, format, type, samples, usage, tiling, count, my_props);
    vkunalignNewStruct(pProps, my_props, desc, cnt);
}

EXPORT void my_vkUpdateDescriptorSets(x86emu_t* emu, void* device, uint32_t writeCount, void* writeSet, uint32_t copyCount, void* copySet)
{
    (void)emu;
    static const char* writeDesc = "upUuuuippp";
    static const char* copyDesc = "upUuuUuuu";

    void* writeAligned = writeCount?vkalignStruct(writeSet, writeDesc, writeCount):NULL; // TODO: Align pNext....
    void* copyAligned = copyCount?vkalignStruct(copySet, copyDesc, copyCount):NULL; // TODO: Align pNext....
    my->vkUpdateDescriptorSets(device, writeCount, writeAligned, copyCount, copyAligned);
    if(writeCount)  vkunalignStruct(writeAligned, writeDesc, writeCount);
    if(copyCount)   vkunalignStruct(copyAligned, copyDesc, copyCount);
}

EXPORT int my_vkGetDisplayPlaneCapabilitiesKHR(x86emu_t* emu, void* device, uint64_t mode, uint32_t index, void* pCap)
{
    (void)emu;
    static const char* desc = "iuuuuuuuuuuuuuuuu";

    void* aligned = vkalignStruct(pCap, desc, 1);
    int ret = my->vkGetDisplayPlaneCapabilitiesKHR(device, mode, index, aligned);
    vkunalignStruct(aligned, desc, 0);
    return ret;
}

EXPORT int my_vkGetPhysicalDeviceDisplayPropertiesKHR(x86emu_t* emu, void* device, uint32_t* count, void* pProp)
{
    (void)emu;
    static const char* desc = "Upuuuuiii";
    if(!pProp)
        return my->vkGetPhysicalDeviceDisplayPropertiesKHR(device, count, pProp);

    int32_t cnt = 0;
    my->vkGetPhysicalDeviceDisplayPropertiesKHR(device, &cnt, NULL);
    void* aligned = vkalignStruct(pProp, desc, cnt);
    int ret = my->vkGetPhysicalDeviceDisplayPropertiesKHR(device, count, aligned);
    vkunalignStruct(aligned, desc, cnt);
    return ret;
}

EXPORT void my_vkGetPhysicalDeviceMemoryProperties(x86emu_t* emu, void* device, void* pProps)
{
    (void)emu;
    static const char* desc = 
    "u" //uint32_t        memoryTypeCount;
    "iuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiuiu"  //VkMemoryType    memoryTypes[VK_MAX_MEMORY_TYPES]; //32
    "u" //uint32_t        memoryHeapCount;
    "UiUiUiUiUiUiUiUiUiUiUiUiUiUiUiUi"; //VkMemoryHeap    memoryHeaps[VK_MAX_MEMORY_HEAPS]; //16

    void* aligned = vkalignStruct(pProps, desc, 1);
    my->vkGetPhysicalDeviceMemoryProperties(device, aligned);
    vkunalignStruct(aligned, desc, 0);
}

EXPORT void my_vkCmdPipelineBarrier(x86emu_t* emu, void* device, int src, int dst, int dep, 
    uint32_t barrierCount, void* pBarriers, uint32_t bufferCount, void* pBuffers, uint32_t imageCount, void* pImages)
{
    (void)emu;
    static const char* desc = "upiiiiuuUiuuuu";

    void* aligned = (imageCount)?vkalignStruct(pImages, desc, imageCount):NULL;
    my->vkCmdPipelineBarrier(device, src, dst, dep, barrierCount, pBarriers, bufferCount, pBuffers, imageCount, aligned);
    if(imageCount) vkunalignStruct(aligned, desc, imageCount);
}

EXPORT int my_vkCreateDebugReportCallbackEXT(x86emu_t* emu, void* instance, 
                                             my_VkDebugReportCallbackCreateInfoEXT_t* create, 
                                             my_VkAllocationCallbacks_t* alloc, void* callback)
{
    (void)emu;
    my_VkDebugReportCallbackCreateInfoEXT_t dbg = *create;
    my_VkAllocationCallbacks_t my_alloc; 
    dbg.pfnCallback = find_DebugReportCallbackEXT_Fct(dbg.pfnCallback);
    return my->vkCreateDebugReportCallbackEXT(instance, &dbg, find_VkAllocationCallbacks(&my_alloc, alloc), callback);
}

EXPORT int my_vkDestroyDebugReportCallbackEXT(x86emu_t* emu, void* instance, void* callback, void* alloc)
{
    (void)emu;
    my_VkAllocationCallbacks_t my_alloc;
    return my->vkDestroyDebugReportCallbackEXT(instance, callback, find_VkAllocationCallbacks(&my_alloc, alloc));
}

CREATE(vkCreateHeadlessSurfaceEXT)

EXPORT int my_vkGetPastPresentationTimingGOOGLE(x86emu_t* emu, void* device, uint64_t swapchain, uint32_t* count, void* timings)
{
    (void)emu;
    static const char* desc = "uUUUU";

    void* aligned = (timings)?vkalignStruct(timings, desc, *count):NULL;
    int ret = my->vkGetPastPresentationTimingGOOGLE(device, swapchain, count, aligned);
    if(timings) vkunalignStruct(aligned, desc, *count);
    return ret;
}

EXPORT int my_vkGetBufferMemoryRequirements2(x86emu_t* emu, void* device, void* pInfo, void* pMemoryRequirement)
{
    (void)emu;
    static const char* desc = "uPSUUu";
    void* m = vkalignStruct(pMemoryRequirement, desc, 1);
    int ret = my->vkGetBufferMemoryRequirements2(device, pInfo, m);
    vkunalignStruct(m, desc, 1);
    return ret;
}
EXPORT int my_vkGetBufferMemoryRequirements2KHR(x86emu_t* emu, void* device, void* pInfo, void* pMemoryRequirement) 
__attribute__((alias("my_vkGetBufferMemoryRequirements2")));

EXPORT void my_vkGetImageMemoryRequirements2(x86emu_t* emu, void* device, void* pInfo, void* pMemoryRequirement)
{
    (void)emu;
    static const char* desc = "uPSUUu";
    void* m = vkalignStruct(pMemoryRequirement, desc, 1);
    my->vkGetImageMemoryRequirements2(device, pInfo, m);
    vkunalignStruct(m, desc, 1);
}
EXPORT void my_vkGetImageMemoryRequirements2KHR(x86emu_t* emu, void* device, void* pInfo, void* pMemoryRequirement) 
__attribute__((alias("my_vkGetImageMemoryRequirements2")));

EXPORT int my_vkGetPhysicalDeviceImageFormatProperties2(x86emu_t* emu, void* device, void* pInfo, void* pImageFormatProperties)
{
    (void)emu;
    static const char* desc = "uPSSuuuuuiU";
    void* m = vkalignStruct(pImageFormatProperties, desc, 1);
    int ret = my->vkGetPhysicalDeviceImageFormatProperties2(device, pInfo, m);
    vkunalignStruct(m, desc, 1);
    return ret;
}
EXPORT int my_vkGetPhysicalDeviceImageFormatProperties2KHR(x86emu_t* emu, void* device, void* pInfo, void* pImageFormatProperties) 
__attribute__((alias("my_vkGetPhysicalDeviceImageFormatProperties2")));


EXPORT void my_vkGetPhysicalDeviceProperties2(x86emu_t* emu, void* device, void* pProp)
{
    (void)emu;
    static const char* desc = "uP"
        "uuuuiYB"
        "SuuuuuuuuuuuUUuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuffuuuffuLUUUiuiuffuuuuiiiiuiiiiiuifuuuuffffffiiUUU"
        "iiiii";
    void* m = vkalignStruct(pProp, desc, 1);
    my->vkGetPhysicalDeviceProperties2(device, m);
    vkunalignStruct(m, desc, 1);
    
}
EXPORT void my_vkGetPhysicalDeviceProperties2KHR(x86emu_t* emu, void* device, void* pProp) 
__attribute__((alias("my_vkGetPhysicalDeviceProperties2")));

EXPORT int my_vkGetPhysicalDeviceSurfaceCapabilities2EXT(x86emu_t* emu, void* device, uint64_t surface, void* pSurfaceCapabilities)
{
    (void)emu;
    static const char* desc = "uPuuuuuuuuuiiiii";
    void* m = vkalignStruct(pSurfaceCapabilities, desc, 1);
    int ret = my->vkGetPhysicalDeviceSurfaceCapabilities2EXT(device, surface, m);
    vkunalignStruct(m, desc, 1);
    return ret;
}

EXPORT int my_vkGetPhysicalDeviceSurfaceCapabilities2KHR(x86emu_t* emu, void* device, void* pInfo, void* pSurfaceCapabilities) 
{
    (void)emu;
    static const char* desc = "uPuuuuuuuuuiiii";
    void* m = vkalignStruct(pSurfaceCapabilities, desc, 1);
    int ret = my->vkGetPhysicalDeviceSurfaceCapabilities2KHR(device, pInfo, m);
    vkunalignStruct(m, desc, 1);
    return ret;
}

EXPORT void my_vkGetDeviceImageSparseMemoryRequirements(x86emu_t* emu, void* device, void* pInfo, uint32_t* count, void* pSparseMemoryRequirements)
{
    (void)emu;
    static const char* desc = "uPiuuuiuUUU";
    void* m = vkalignStruct(pSparseMemoryRequirements, desc, *count);
    my->vkGetDeviceImageSparseMemoryRequirements(device, pInfo, count, m);
    vkunalignStruct(m, desc, *count);   // bad things will happens if *count is changed while pSparseMemoryRequirements is not NULL
}
EXPORT void vkGetDeviceImageSparseMemoryRequirementsKHR(x86emu_t* emu, void* device, void* pInfo, uint32_t* count, void* pSparseMemoryRequirements)
__attribute__((alias("my_vkGetDeviceImageSparseMemoryRequirements")));

EXPORT int my_vkQueueSubmit2(x86emu_t* emu, void* queue, uint32_t count, void* pSubmits, uint64_t fence)
{
    (void)emu;
    static const char* desc = "uPiuQuQuQ";
    void* m = vkalignStruct(pSubmits, desc, count);
    int ret = my->vkQueueSubmit2(queue, count, m, fence);
    vkunalignStruct(m, desc, count);
    return ret;
}
EXPORT int my_vkQueueSubmit2EXT(x86emu_t* emu, void* queue, uint32_t count, void* pSubmits, uint64_t fence)
__attribute__((alias("my_vkQueueSubmit2")));

EXPORT int my_vkGetPhysicalDeviceOpticalFlowImageFormatsNV(x86emu_t* emu, void* device, void* pInfo, uint32_t* count, void* pImageFormatProperties)
{
    (void)emu;
    static const char* desc = "uPiuuuiuUUU";
    void* m = vkalignStruct(pImageFormatProperties, desc, *count);
    int ret = my->vkGetPhysicalDeviceOpticalFlowImageFormatsNV(device, pInfo, count, m);
    vkunalignStruct(m, desc, *count);   // bad things will happens if *count is changed while pSparseMemoryRequirements is not NULL
    return ret;
}

EXPORT int my_vkBindVideoSessionMemoryKHR(x86emu_t* emu, void* device, uint64_t session, uint32_t count, void* pInfos)
{
    (void)emu;
    static const char* desc = "uPuUUU";
    void* m = vkalignStruct(pInfos, desc, count);
    int ret = my->vkBindVideoSessionMemoryKHR(device, session, count, m);
    vkunalignStruct(m, desc, count);
    return ret;
}