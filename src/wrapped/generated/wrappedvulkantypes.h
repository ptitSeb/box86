/*****************************************************************
 * File automatically generated by rebuild_wrappers.py (v1.2.0.09)
 *****************************************************************/
#ifndef __wrappedvulkanTYPES_H_
#define __wrappedvulkanTYPES_H_

#ifndef LIBNAME
#error You should only #include this file inside a wrapped*.c file
#endif
#ifndef ADDED_FUNCTIONS
#define ADDED_FUNCTIONS() 
#endif

typedef void (*vFpp_t)(void*, void*);
typedef void* (*pFpp_t)(void*, void*);
typedef void (*vFpUp_t)(void*, uint64_t, void*);
typedef void (*vFppp_t)(void*, void*, void*);
typedef int32_t (*iFpUp_t)(void*, uint64_t, void*);
typedef int32_t (*iFppp_t)(void*, void*, void*);
typedef int32_t (*iFPpp_t)(void*, void*, void*);
typedef int32_t (*iFpUup_t)(void*, uint64_t, uint32_t, void*);
typedef int32_t (*iFpppp_t)(void*, void*, void*, void*);
typedef int32_t (*iFpPpp_t)(void*, void*, void*, void*);
typedef void (*vFpupup_t)(void*, uint32_t, void*, uint32_t, void*);
typedef int32_t (*iFpuppp_t)(void*, uint32_t, void*, void*, void*);
typedef int32_t (*iFpUPpp_t)(void*, uint64_t, void*, void*, void*);
typedef int32_t (*iFpUuppp_t)(void*, uint64_t, uint32_t, void*, void*, void*);
typedef void (*vFpiiiiipp_t)(void*, int32_t, int32_t, int32_t, int32_t, int32_t, void*, void*);
typedef void (*vFpiiiupupup_t)(void*, int32_t, int32_t, int32_t, uint32_t, void*, uint32_t, void*, uint32_t, void*);

#define SUPER() ADDED_FUNCTIONS() \
	GO(vkDestroyDevice, vFpp_t) \
	GO(vkDestroyInstance, vFpp_t) \
	GO(vkGetPhysicalDeviceMemoryProperties, vFpp_t) \
	GO(vkGetPhysicalDeviceProperties, vFpp_t) \
	GO(vkGetDeviceProcAddr, pFpp_t) \
	GO(vkGetInstanceProcAddr, pFpp_t) \
	GO(vkDestroyBuffer, vFpUp_t) \
	GO(vkDestroyBufferView, vFpUp_t) \
	GO(vkDestroyCommandPool, vFpUp_t) \
	GO(vkDestroyDescriptorPool, vFpUp_t) \
	GO(vkDestroyDescriptorSetLayout, vFpUp_t) \
	GO(vkDestroyDescriptorUpdateTemplate, vFpUp_t) \
	GO(vkDestroyDescriptorUpdateTemplateKHR, vFpUp_t) \
	GO(vkDestroyEvent, vFpUp_t) \
	GO(vkDestroyFence, vFpUp_t) \
	GO(vkDestroyFramebuffer, vFpUp_t) \
	GO(vkDestroyImage, vFpUp_t) \
	GO(vkDestroyImageView, vFpUp_t) \
	GO(vkDestroyPipeline, vFpUp_t) \
	GO(vkDestroyPipelineCache, vFpUp_t) \
	GO(vkDestroyPipelineLayout, vFpUp_t) \
	GO(vkDestroyQueryPool, vFpUp_t) \
	GO(vkDestroyRenderPass, vFpUp_t) \
	GO(vkDestroySampler, vFpUp_t) \
	GO(vkDestroySamplerYcbcrConversionKHR, vFpUp_t) \
	GO(vkDestroySemaphore, vFpUp_t) \
	GO(vkDestroyShaderModule, vFpUp_t) \
	GO(vkDestroySurfaceKHR, vFpUp_t) \
	GO(vkDestroySwapchainKHR, vFpUp_t) \
	GO(vkDestroyDebugUtilsMessengerEXT, vFppp_t) \
	GO(vkFreeMemory, iFpUp_t) \
	GO(vkGetPhysicalDeviceDisplayPropertiesKHR, iFppp_t) \
	GO(vkCreateInstance, iFPpp_t) \
	GO(vkGetDisplayPlaneCapabilitiesKHR, iFpUup_t) \
	GO(vkCreateDebugUtilsMessengerEXT, iFpppp_t) \
	GO(vkCreateWaylandSurfaceKHR, iFpppp_t) \
	GO(vkAllocateMemory, iFpPpp_t) \
	GO(vkCreateBuffer, iFpPpp_t) \
	GO(vkCreateBufferView, iFpPpp_t) \
	GO(vkCreateCommandPool, iFpPpp_t) \
	GO(vkCreateDescriptorPool, iFpPpp_t) \
	GO(vkCreateDescriptorSetLayout, iFpPpp_t) \
	GO(vkCreateDescriptorUpdateTemplate, iFpPpp_t) \
	GO(vkCreateDescriptorUpdateTemplateKHR, iFpPpp_t) \
	GO(vkCreateDevice, iFpPpp_t) \
	GO(vkCreateDisplayPlaneSurfaceKHR, iFpPpp_t) \
	GO(vkCreateEvent, iFpPpp_t) \
	GO(vkCreateFence, iFpPpp_t) \
	GO(vkCreateFramebuffer, iFpPpp_t) \
	GO(vkCreateImage, iFpPpp_t) \
	GO(vkCreateImageView, iFpPpp_t) \
	GO(vkCreatePipelineCache, iFpPpp_t) \
	GO(vkCreatePipelineLayout, iFpPpp_t) \
	GO(vkCreateQueryPool, iFpPpp_t) \
	GO(vkCreateRenderPass, iFpPpp_t) \
	GO(vkCreateSampler, iFpPpp_t) \
	GO(vkCreateSamplerYcbcrConversion, iFpPpp_t) \
	GO(vkCreateSamplerYcbcrConversionKHR, iFpPpp_t) \
	GO(vkCreateSemaphore, iFpPpp_t) \
	GO(vkCreateShaderModule, iFpPpp_t) \
	GO(vkCreateSwapchainKHR, iFpPpp_t) \
	GO(vkCreateXcbSurfaceKHR, iFpPpp_t) \
	GO(vkCreateXlibSurfaceKHR, iFpPpp_t) \
	GO(vkUpdateDescriptorSets, vFpupup_t) \
	GO(vkCreateSharedSwapchainsKHR, iFpuppp_t) \
	GO(vkCreateDisplayModeKHR, iFpUPpp_t) \
	GO(vkCreateComputePipelines, iFpUuppp_t) \
	GO(vkCreateGraphicsPipelines, iFpUuppp_t) \
	GO(vkGetPhysicalDeviceSparseImageFormatProperties, vFpiiiiipp_t) \
	GO(vkCmdPipelineBarrier, vFpiiiupupup_t)

#endif // __wrappedvulkanTYPES_H_
