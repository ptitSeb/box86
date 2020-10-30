#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error meh!
#endif

//vkDeviceSize == uint64_t
//VkImageLayout == enum
//VK_DEFINE_NON_DISPATCHABLE_HANDLE == uint64_t !!!!
// VkBuffer = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkBufferView = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkCommandPool = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkDescriptorPool = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkDescriptorSet = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkDescriptorSetLayout = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkDescriptorUpdateTemplate = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkDeviceMemory = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkDisplayKHR = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkDisplayModeKHR = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkEvent = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkFence = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkFramebuffer = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkImage = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkImageView = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkPipeline = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkPipelineCache = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkPipelineLayout = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkQueryPool = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkRenderPass = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkSampler = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkSamplerYcbcrConversion = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkSemaphore = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkShaderModule = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkSurfaceKHR = VK_DEFINE_NON_DISPATCHABLE_HANDLE
// VkSwapchainKHR = VK_DEFINE_NON_DISPATCHABLE_HANDLE

// VK_VERSION_1_0
GO(vkAllocateCommandBuffers, iFppp)
GO(vkAllocateDescriptorSets, iFppp)
GOM(vkAllocateMemory, iFEpppp)
GO(vkBeginCommandBuffer, iFpp)
GO(vkBindBufferMemory, iFppUU)
GO(vkBindImageMemory, iFpUUU)
GO(vkCmdBeginQuery, vFpUui)
GO(vkCmdBeginRenderPass, vFppp)
GO(vkCmdBindDescriptorSets, vFpiUuupup)
GO(vkCmdBindIndexBuffer, vFpUUi)
GO(vkCmdBindPipeline, vFppU)
GO(vkCmdBindVertexBuffers, vFpuupp)
GO(vkCmdBlitImage, vFpUiUiupi)
GO(vkCmdClearAttachments, vFpupup)
GO(vkCmdClearColorImage, vFpUipup)
GO(vkCmdClearDepthStencilImage, vFpUipup)
GO(vkCmdCopyBuffer, vFpUUup)
GO(vkCmdCopyBufferToImage, vFpUUiup)
GO(vkCmdCopyImage, vFpUiUiup)
GO(vkCmdCopyImageToBuffer, vFpUiUup)
GO(vkCmdCopyQueryPoolResults, vFpUuuUUUi)
GO(vkCmdDispatch, vFpuuu)
GO(vkCmdDispatchIndirect, vFpUU)
GO(vkCmdDraw, vFpuuuu)
GO(vkCmdDrawIndexed, vFpuuuiu)
GO(vkCmdDrawIndexedIndirect, vFpUUuu)
GO(vkCmdDrawIndirect, vFpUUuu)
GO(vkCmdEndQuery, vFpUu)
GO(vkCmdEndRenderPass, vFp)
GO(vkCmdExecuteCommands, vFpup)
GO(vkCmdFillBuffer, vFpUUUu)
GO(vkCmdNextSubpass, vFpi)
GO(vkCmdPipelineBarrier, vFpiiiupupup)
GO(vkCmdPushConstants, vFpUiuup)
GO(vkCmdResetEvent, vFpUi)
GO(vkCmdResetQueryPool, vFpUuu)
GO(vkCmdResolveImage, vFpUiUiup)
GO(vkCmdSetBlendConstants, vFpp)
GO(vkCmdSetDepthBias, vFpfff)
GO(vkCmdSetDepthBounds, vFpff)
GO(vkCmdSetEvent, vFpUi)
GO(vkCmdSetLineWidth, vFpf)
GO(vkCmdSetScissor, vFpuup)
GO(vkCmdSetStencilCompareMask, vFpiu)
GO(vkCmdSetStencilReference, vFpiu)
GO(vkCmdSetStencilWriteMask, vFpiu)
GO(vkCmdSetViewport, vFpuup)
GO(vkCmdUpdateBuffer, vFpUUUp)
GO(vkCmdWaitEvents, vFpupiiupupup)
GO(vkCmdWriteTimestamp, vFpiUu)
GOM(vkCreateBuffer, iFEpppp)
GOM(vkCreateBufferView, iFEpppp)
GOM(vkCreateCommandPool, iFEpppp)
GOM(vkCreateComputePipelines, iFEpUuppp)
GOM(vkCreateDescriptorPool, iFEpppp)
GOM(vkCreateDescriptorSetLayout, iFEpppp)
GOM(vkCreateDevice, iFEpppp)
GOM(vkCreateEvent, iFEpppp)
GOM(vkCreateFence, iFEpppp)
GOM(vkCreateFramebuffer, iFEpppp)
GOM(vkCreateGraphicsPipelines, iFEpUuppp)
GOM(vkCreateImage, iFEpppp)
GOM(vkCreateImageView, iFEpppp)
GOM(vkCreateInstance, iFEppp)
GOM(vkCreatePipelineCache, iFEpppp)
GOM(vkCreatePipelineLayout, iFEpppp)
GOM(vkCreateQueryPool, iFEpppp)
GOM(vkCreateRenderPass, iFEpppp)
GOM(vkCreateSampler, iFEpppp)
GOM(vkCreateSemaphore, iFEpppp)
GOM(vkCreateShaderModule, iFEpppp)
GOM(vkDestroyBuffer, vFEpUp)
GOM(vkDestroyBufferView, vFEpUp)
GOM(vkDestroyCommandPool, vFEpUp)
GOM(vkDestroyDescriptorPool, vFEpUp)
GOM(vkDestroyDescriptorSetLayout, vFEpUp)
GOM(vkDestroyDevice, vFEpp)
GOM(vkDestroyEvent, vFEpUp)
GOM(vkDestroyFence, vFEpUp)
GOM(vkDestroyFramebuffer, vFEpUp)
GOM(vkDestroyImage, vFEpUp)
GOM(vkDestroyImageView, vFEpUp)
GOM(vkDestroyInstance, vFEpp)
GOM(vkDestroyPipeline, vFEpUp)
GOM(vkDestroyPipelineCache, vFEpUp)
GOM(vkDestroyPipelineLayout, vFEpUp)
GOM(vkDestroyQueryPool, vFEpUp)
GOM(vkDestroyRenderPass, vFEpUp)
GOM(vkDestroySampler, vFEpUp)
GOM(vkDestroySemaphore, vFEpUp)
GOM(vkDestroyShaderModule, vFEpUp)
GO(vkDeviceWaitIdle, iFp)
GO(vkEndCommandBuffer, iFp)
GO(vkEnumerateDeviceExtensionProperties, iFpppp)
GO(vkEnumerateDeviceLayerProperties, iFppp)
GO(vkEnumerateInstanceExtensionProperties, iFppp)
GO(vkEnumerateInstanceLayerProperties, iFpp)
GO(vkEnumeratePhysicalDevices, iFppp)
GO(vkFlushMappedMemoryRanges, iFpup)
GO(vkFreeCommandBuffers, vFpUup)
GO(vkFreeDescriptorSets, iFpUup)
GOM(vkFreeMemory, iFEpUp)
GO(vkGetBufferMemoryRequirements, iFpUp)
GO(vkGetDeviceMemoryCommitment, vFpUp)
GOM(vkGetDeviceProcAddr, pFEpp)
GO(vkGetDeviceQueue, vFpuup)
GO(vkGetEventStatus, iFpU)
GO(vkGetFenceStatus, iFpU)
GO(vkGetImageMemoryRequirements, vFpUp)
GO(vkGetImageSparseMemoryRequirements, vFpUpp)
GO(vkGetImageSubresourceLayout, vFpUpp)
GOM(vkGetInstanceProcAddr, pFEpp)
GO(vkGetPhysicalDeviceFeatures, vFpp)
GO(vkGetPhysicalDeviceFormatProperties, vFpip)
GO(vkGetPhysicalDeviceImageFormatProperties, vFpiiiiip)
GO(vkGetPhysicalDeviceMemoryProperties, vFpp)
GO(vkGetPhysicalDeviceProperties, vFpp)
GO(vkGetPhysicalDeviceQueueFamilyProperties, vFppp)
GO(vkGetPhysicalDeviceSparseImageFormatProperties, vFpiiiiipp)
GO(vkGetPipelineCacheData, iFpUpp)
GO(vkGetQueryPoolResults, iFpUuuLpUi)
GO(vkGetRenderAreaGranularity, vFpUp)
GO(vkInvalidateMappedMemoryRanges, iFpup)
GO(vkMapMemory, iFpUUUip)
GO(vkMergePipelineCaches, iFpUup)
GO(vkQueueBindSparse, iFpupU)
GO(vkQueueSubmit, iFpupU)
GO(vkQueueWaitIdle, iFp)
GO(vkResetCommandBuffer, iFpi)
GO(vkResetCommandPool, iFpUi)
GO(vkResetDescriptorPool, iFpUi)
GO(vkResetEvent, iFpU)
GO(vkResetFences, iFpup)
GO(vkSetEvent, iFpU)
GO(vkUnmapMemory, vFpU)
GO(vkUpdateDescriptorSets, vFpupup)
GO(vkWaitForFences, iFpupiU)

// VK_VERSION_1_1
GO(vkBindBufferMemory2, iFpup)
GO(vkBindImageMemory2, iFpup)
GO(vkCmdDispatchBase, vFpuuuuuu)
GO(vkCmdSetDeviceMask, vFpu)
GOM(vkCreateDescriptorUpdateTemplate, iFEpppp)
GOM(vkCreateSamplerYcbcrConversion, iFEpppp)
GOM(vkDestroyDescriptorUpdateTemplate, vFEpUp)
GO(vkEnumerateInstanceVersion, iFp)
GO(vkEnumeratePhysicalDeviceGroups, iFppp)
GO(vkGetBufferMemoryRequirements2, iFppp)
GO(vkGetImageMemoryRequirements2, vFppp)
GO(vkGetImageSparseMemoryRequirements2, vFpppp)
GO(vkGetDescriptorSetLayoutSupport, vFppp)
GO(vkGetDeviceGroupPeerMemoryFeatures, vFpuuup)
GO(vkGetDeviceQueue2, vFppp)
GO(vkGetPhysicalDeviceExternalBufferProperties, vFppp)
GO(vkGetPhysicalDeviceExternalFenceProperties, vFppp)
GO(vkGetPhysicalDeviceExternalSemaphoreProperties, vFppp)
GO(vkGetPhysicalDeviceFeatures2, vFpp)
GO(vkGetPhysicalDeviceFormatProperties2, vFpip)
GO(vkGetPhysicalDeviceImageFormatProperties2, vFppp)
GO(vkGetPhysicalDeviceMemoryProperties2, vFpp)
GO(vkGetPhysicalDeviceProperties2, vFpp)
GO(vkGetPhysicalDeviceQueueFamilyProperties2, vFppp)
GO(vkGetPhysicalDeviceSparseImageFormatProperties2, vFpppp)
GO(vkUpdateDescriptorSetWithTemplate, vFpUUp)
GO(vkTrimCommandPool, vFpUi)

// VK_VERSION_1_2

// VK_EXT_debug_report
//GOM(vkCreateDebugReportCallbackEXT, iFEpppp)
GO(vkDebugReportMessageEXT, vFpiiULipp)
//GOM(vkDestroyDebugReportCallbackEXT, iFEppp)

//VK_EXT_debug_utils
GO(vkCmdBeginDebugUtilsLabelEXT, vFpp)
GO(vkCmdEndDebugUtilsLabelEXT, vFp)
GO(vkCmdInsertDebugUtilsLabelEXT, vFpp)
GOM(vkCreateDebugUtilsMessengerEXT, iFEpppp)
GOM(vkDestroyDebugUtilsMessengerEXT, vFEppp)
GO(vkQueueBeginDebugUtilsLabelEXT, vFpp)
GO(vkQueueEndDebugUtilsLabelEXT, vFp)
GO(vkQueueInsertDebugUtilsLabelEXT, vFpp)
GO(vkSetDebugUtilsObjectNameEXT, iFpp)
GO(vkSetDebugUtilsObjectTagEXT, iFpp)
//GOM(vkSubmitDebugUtilsMessageEXT, vFEpppp)    // callback in last arguments

// VK_KHR_external_memory_capabilities
GO(vkGetPhysicalDeviceExternalBufferPropertiesKHR, vFppp)

// VK_KHR_get_physical_device_properties2
GO(vkGetPhysicalDeviceFeatures2KHR, vFpp)
GO(vkGetPhysicalDeviceFormatProperties2KHR, vFpip)
GO(vkGetPhysicalDeviceImageFormatProperties2KHR, vFppp)
GO(vkGetPhysicalDeviceMemoryProperties2KHR, vFpp)
GO(vkGetPhysicalDeviceProperties2KHR, vFpp)
GO(vkGetPhysicalDeviceQueueFamilyProperties2KHR, vFppp)
GO(vkGetPhysicalDeviceSparseImageFormatProperties2KHR, vFpppp)

// VK_KHR_get_surface_capabilities2
GO(vkGetPhysicalDeviceSurfaceCapabilities2KHR, iFppp)
GO(vkGetPhysicalDeviceSurfaceFormats2KHR, iFpppp)

// VK_KHR_surface
GOM(vkDestroySurfaceKHR, vFEpUp)
GO(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, iFpUp)
GO(vkGetPhysicalDeviceSurfaceFormatsKHR, iFpUpp)
GO(vkGetPhysicalDeviceSurfacePresentModesKHR, iFpUpp)
GO(vkGetPhysicalDeviceSurfaceSupportKHR, iFpuUp)

// VK_KHR_xcb_surface
GOM(vkCreateXcbSurfaceKHR, iFEpppp)
GO(vkGetPhysicalDeviceXcbPresentationSupportKHR, iFpupp)

// VK_KHR_xlib_surface
GOM(vkCreateXlibSurfaceKHR, iFEpppp)
GO(vkGetPhysicalDeviceXlibPresentationSupportKHR, iFpupp)

// VK_KHR_swapchain
GO(vkAcquireNextImageKHR, iFpUUUUp)
GO(vkAcquireNextImage2KHR, iFppp)
GOM(vkCreateSwapchainKHR, iFEpppp)
GOM(vkDestroySwapchainKHR, vFEpUp)
GO(vkGetDeviceGroupPresentCapabilitiesKHR, iFpp)
GO(vkGetDeviceGroupSurfacePresentModesKHR, iFpUp)
GO(vkGetPhysicalDevicePresentRectanglesKHR, iFpUpp)
GO(vkGetSwapchainImagesKHR, iFpUpp)
GO(vkQueuePresentKHR, iFpp)

// VK_KHR_bind_memory2
GO(vkBindBufferMemory2KHR, iFpup)
GO(vkBindImageMemory2KHR, iFpup)

// VK_KHR_display
GOM(vkCreateDisplayModeKHR, iFEpUppp)
GOM(vkCreateDisplayPlaneSurfaceKHR, iFEpppp)
GO(vkGetDisplayModePropertiesKHR, iFpUpp)
GO(vkGetDisplayPlaneCapabilitiesKHR, iFpUup)
GO(vkGetDisplayPlaneSupportedDisplaysKHR, iFpupp)
GO(vkGetPhysicalDeviceDisplayPlanePropertiesKHR, iFppp)
GO(vkGetPhysicalDeviceDisplayPropertiesKHR, iFppp)

// VK_KHR_descriptor_update_template
GOM(vkCreateDescriptorUpdateTemplateKHR, iFEpppp)
GOM(vkDestroyDescriptorUpdateTemplateKHR, vFEpUp)
GO(vkUpdateDescriptorSetWithTemplateKHR, vFpUUp)
GO(vkCmdPushDescriptorSetWithTemplateKHR, vFpUUup)

// VK_EXT_display_surface_counter
GO(vkGetPhysicalDeviceSurfaceCapabilities2EXT, iFpUp)

// VK_KHR_get_display_properties2
GO(vkGetDisplayModeProperties2KHR, iFpUpp)
GO(vkGetDisplayPlaneCapabilities2KHR, iFppp)
GO(vkGetPhysicalDeviceDisplayPlaneProperties2KHR, iFppp)
GO(vkGetPhysicalDeviceDisplayProperties2KHR, iFppp)

// VK_KHR_device_group
GO(vkCmdDispatchBaseKHR, vFpuuuuuu)
GO(vkCmdSetDeviceMaskKHR, vFpu)
GO(vkGetDeviceGroupPeerMemoryFeaturesKHR, vFpuuup)

// VK_KHR_sampler_ycbcr_conversion
GOM(vkCreateSamplerYcbcrConversionKHR, iFEpppp)
GOM(vkDestroySamplerYcbcrConversionKHR, vFEpUp)

// VK_KHR_display_swapchain
GOM(vkCreateSharedSwapchainsKHR, iFEpuppp)

// VK_KHR_wayland_surface
GOM(vkCreateWaylandSurfaceKHR, iFEpppp)
GO(vkGetPhysicalDeviceWaylandPresentationSupportKHR, iFpup)

// VK_KHR_device_group_creation
GO(vkEnumeratePhysicalDeviceGroupsKHR, iFppp)

// VK_KHR_get_memory_requirements2
GO(vkGetBufferMemoryRequirements2KHR, iFppp)
GO(vkGetImageMemoryRequirements2KHR, vFppp)
GO(vkGetImageSparseMemoryRequirements2KHR, vFpppp)

// VK_KHR_external_fence_capabilities
GO(vkGetPhysicalDeviceExternalFencePropertiesKHR, vFppp)

// VK_KHR_external_semaphore_capabilities
GO(vkGetPhysicalDeviceExternalSemaphorePropertiesKHR, vFppp)

// VK_KHR_maintenance1
GO(vkTrimCommandPoolKHR, vFpUi)

// VK_KHR_maintenance3
GO(vkGetDescriptorSetLayoutSupportKHR, vFppp)

