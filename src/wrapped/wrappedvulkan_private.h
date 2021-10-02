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

// P = Vulkan input Param Structure

// VK_VERSION_1_0
GO(vkAllocateCommandBuffers, iFpPp)
GO(vkAllocateDescriptorSets, iFpPp)
GOM(vkAllocateMemory, iFEpPpp)
GO(vkBeginCommandBuffer, iFpP)
GO(vkBindBufferMemory, iFpUUU)
GO(vkBindImageMemory, iFpUUU)
GO(vkCmdBeginQuery, vFpUui)
GO(vkCmdBeginRenderPass, vFpPp)
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
GOM(vkCmdPipelineBarrier, vFEpiiiupupup)
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
GO(vkCmdWaitEvents, vFpupiiuPuPuP)
GO(vkCmdWriteTimestamp, vFpiUu)
GOM(vkCreateBuffer, iFEpPpp)
GOM(vkCreateBufferView, iFEpPpp)
GOM(vkCreateCommandPool, iFEpPpp)
GOM(vkCreateComputePipelines, iFEpUuppp)
GOM(vkCreateDescriptorPool, iFEpPpp)
GOM(vkCreateDescriptorSetLayout, iFEpPpp)
GOM(vkCreateDevice, iFEpPpp)
GOM(vkCreateEvent, iFEpPpp)
GOM(vkCreateFence, iFEpPpp)
GOM(vkCreateFramebuffer, iFEpPpp)
GOM(vkCreateGraphicsPipelines, iFEpUuppp)
GOM(vkCreateImage, iFEpPpp)
GOM(vkCreateImageView, iFEpPpp)
GOM(vkCreateInstance, iFEPpp)
GOM(vkCreatePipelineCache, iFEpPpp)
GOM(vkCreatePipelineLayout, iFEpPpp)
GOM(vkCreateQueryPool, iFEpPpp)
GOM(vkCreateRenderPass, iFEpPpp)
GOM(vkCreateSampler, iFEpPpp)
GOM(vkCreateSemaphore, iFEpPpp)
GOM(vkCreateShaderModule, iFEpPpp)
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
GO(vkFlushMappedMemoryRanges, iFpup)    // should wrap the array of VkMappedMemoryRange
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
GO(vkGetPhysicalDeviceImageFormatProperties, iFpiiiiip) //VkImageFormatProperties sems OK
GOM(vkGetPhysicalDeviceMemoryProperties, vFEpp)
GOM(vkGetPhysicalDeviceProperties, vFEpp)
GO(vkGetPhysicalDeviceQueueFamilyProperties, vFppp)   //VkQueueFamilyProperties is OK
GOM(vkGetPhysicalDeviceSparseImageFormatProperties, vFEpiiiiipp)
GO(vkGetPipelineCacheData, iFpUpp)
GO(vkGetQueryPoolResults, iFpUuuLpUi)
GO(vkGetRenderAreaGranularity, vFpUp)
GO(vkInvalidateMappedMemoryRanges, iFpup)   //VkMappedMemoryRange seems OK
GO(vkMapMemory, iFpUUUip)
GO(vkMergePipelineCaches, iFpUup)
GO(vkQueueBindSparse, iFpuPU)
GO(vkQueueSubmit, iFpuPU)
GO(vkQueueWaitIdle, iFp)
GO(vkResetCommandBuffer, iFpi)
GO(vkResetCommandPool, iFpUi)
GO(vkResetDescriptorPool, iFpUi)
GO(vkResetEvent, iFpU)
GO(vkResetFences, iFpup)
GO(vkSetEvent, iFpU)
GO(vkUnmapMemory, vFpU)
GOM(vkUpdateDescriptorSets, vFEpupup)
GO(vkWaitForFences, iFpupiU)

// VK_VERSION_1_1
GO(vkBindBufferMemory2, iFpuP)
GO(vkBindImageMemory2, iFpuP)
GO(vkCmdDispatchBase, vFpuuuuuu)
GO(vkCmdSetDeviceMask, vFpu)
GOM(vkCreateDescriptorUpdateTemplate, iFEpPpp)
GOM(vkCreateSamplerYcbcrConversion, iFEpPpp)
GOM(vkDestroyDescriptorUpdateTemplate, vFEpUp)
GO(vkEnumerateInstanceVersion, iFp)
GO(vkEnumeratePhysicalDeviceGroups, iFppp)  //VkPhysicalDeviceGroupProperties seems OK
GO(vkGetBufferMemoryRequirements2, iFpPP)
GO(vkGetImageMemoryRequirements2, vFpPP)
GO(vkGetImageSparseMemoryRequirements2, vFpppp)
GO(vkGetDescriptorSetLayoutSupport, vFpPp)
GO(vkGetDeviceGroupPeerMemoryFeatures, vFpuuup)
GO(vkGetDeviceQueue2, vFpPp)
GO(vkGetPhysicalDeviceExternalBufferProperties, vFpPP)
GO(vkGetPhysicalDeviceExternalFenceProperties, vFpPP)
GO(vkGetPhysicalDeviceExternalSemaphoreProperties, vFpPP)
GO(vkGetPhysicalDeviceFeatures2, vFpP)
GO(vkGetPhysicalDeviceFormatProperties2, vFpiP)
GO(vkGetPhysicalDeviceImageFormatProperties2, vFpPP)
GO(vkGetPhysicalDeviceMemoryProperties2, vFpP)
GO(vkGetPhysicalDeviceProperties2, vFpP)
GO(vkGetPhysicalDeviceQueueFamilyProperties2, vFppp)    //VkQueueFamilyProperties2 seems OK
GO(vkGetPhysicalDeviceSparseImageFormatProperties2, vFpPpp) //VkSparseImageFormatProperties2 seems OK
GO(vkUpdateDescriptorSetWithTemplate, vFpUUp)
GO(vkTrimCommandPool, vFpUi)

// VK_VERSION_1_2
GO(vkResetQueryPool, vFpUuu)

// VK_EXT_debug_report
GOM(vkCreateDebugReportCallbackEXT, iFEpppp)
GO(vkDebugReportMessageEXT, vFpiiULipp)
GOM(vkDestroyDebugReportCallbackEXT, iFEppp)

//VK_EXT_debug_utils
GO(vkCmdBeginDebugUtilsLabelEXT, vFpp)  //TODO: Cehck alignement of this extension
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
GO(vkGetPhysicalDeviceExternalBufferPropertiesKHR, vFpPP)

// VK_KHR_get_physical_device_properties2
GO(vkGetPhysicalDeviceFeatures2KHR, vFpP)
GO(vkGetPhysicalDeviceFormatProperties2KHR, vFpiP)
GO(vkGetPhysicalDeviceImageFormatProperties2KHR, vFpPP)
GO(vkGetPhysicalDeviceMemoryProperties2KHR, vFpP)
GO(vkGetPhysicalDeviceProperties2KHR, vFpP)
GO(vkGetPhysicalDeviceQueueFamilyProperties2KHR, vFppp)
GO(vkGetPhysicalDeviceSparseImageFormatProperties2KHR, vFpPpp)  //VkSparseImageFormatProperties2 seems OK

// VK_KHR_get_surface_capabilities2
GO(vkGetPhysicalDeviceSurfaceCapabilities2KHR, iFpPP)
GO(vkGetPhysicalDeviceSurfaceFormats2KHR, iFpPpp)   //VkSurfaceFormat2KHR seems OK (but array)

// VK_KHR_surface
GOM(vkDestroySurfaceKHR, vFEpUp)
GO(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, iFpUp)    //VkSurfaceCapabilitiesKHR seems OK
GO(vkGetPhysicalDeviceSurfaceFormatsKHR, iFpUpp)
GO(vkGetPhysicalDeviceSurfacePresentModesKHR, iFpUpp)
GO(vkGetPhysicalDeviceSurfaceSupportKHR, iFpuUp)

// VK_KHR_xcb_surface
GOM(vkCreateXcbSurfaceKHR, iFEpPpp)
GO(vkGetPhysicalDeviceXcbPresentationSupportKHR, iFpupp)

// VK_KHR_xlib_surface
GOM(vkCreateXlibSurfaceKHR, iFEpPpp)
GO(vkGetPhysicalDeviceXlibPresentationSupportKHR, iFpupp)

// VK_KHR_swapchain
GO(vkAcquireNextImageKHR, iFpUUUUp)
GO(vkAcquireNextImage2KHR, iFpPp)
GOM(vkCreateSwapchainKHR, iFEpPpp)
GOM(vkDestroySwapchainKHR, vFEpUp)
GO(vkGetDeviceGroupPresentCapabilitiesKHR, iFpP)
GO(vkGetDeviceGroupSurfacePresentModesKHR, iFpUp)
GO(vkGetPhysicalDevicePresentRectanglesKHR, iFpUpp)
GO(vkGetSwapchainImagesKHR, iFpUpp)
GO(vkQueuePresentKHR, iFpP)

// VK_KHR_bind_memory2
GO(vkBindBufferMemory2KHR, iFpuP)
GO(vkBindImageMemory2KHR, iFpuP)

// VK_KHR_display
GOM(vkCreateDisplayModeKHR, iFEpUPpp)
GOM(vkCreateDisplayPlaneSurfaceKHR, iFEpPpp)
GO(vkGetDisplayModePropertiesKHR, iFpUpp)   //VkDisplayModePropertiesKHR seems OK
GOM(vkGetDisplayPlaneCapabilitiesKHR, iFEpUup)
GO(vkGetDisplayPlaneSupportedDisplaysKHR, iFpupp)
GO(vkGetPhysicalDeviceDisplayPlanePropertiesKHR, iFppp) //VkDisplayPlanePropertiesKHR is OK
GOM(vkGetPhysicalDeviceDisplayPropertiesKHR, iFEppp)

// VK_KHR_descriptor_update_template
GOM(vkCreateDescriptorUpdateTemplateKHR, iFEpPpp)
GOM(vkDestroyDescriptorUpdateTemplateKHR, vFEpUp)
GO(vkUpdateDescriptorSetWithTemplateKHR, vFpUUp)
GO(vkCmdPushDescriptorSetWithTemplateKHR, vFpUUup)

// VK_EXT_display_surface_counter
GO(vkGetPhysicalDeviceSurfaceCapabilities2EXT, iFpUP)

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
GOM(vkCreateSamplerYcbcrConversionKHR, iFEpPpp)
GOM(vkDestroySamplerYcbcrConversionKHR, vFEpUp)

// VK_KHR_display_swapchain
GOM(vkCreateSharedSwapchainsKHR, iFEpuppp)

// VK_KHR_wayland_surface
GOM(vkCreateWaylandSurfaceKHR, iFEpppp)
GO(vkGetPhysicalDeviceWaylandPresentationSupportKHR, iFpup)

// VK_KHR_device_group_creation
GO(vkEnumeratePhysicalDeviceGroupsKHR, iFppp)

// VK_KHR_get_memory_requirements2
GO(vkGetBufferMemoryRequirements2KHR, iFpPP)
GO(vkGetImageMemoryRequirements2KHR, vFpPP)
GO(vkGetImageSparseMemoryRequirements2KHR, vFpppp)

// VK_KHR_external_fence_capabilities
GO(vkGetPhysicalDeviceExternalFencePropertiesKHR, vFpPP)

// VK_KHR_external_semaphore_capabilities
GO(vkGetPhysicalDeviceExternalSemaphorePropertiesKHR, vFpPP)

// VK_KHR_maintenance1
GO(vkTrimCommandPoolKHR, vFpUi)

// VK_KHR_maintenance2
// no functions

// VK_KHR_maintenance3
GO(vkGetDescriptorSetLayoutSupportKHR, vFpPp)

// VK_KHR_external_memory_fd
GO(vkGetMemoryFdKHR, iFppp)
GO(vkGetMemoryFdPropertiesKHR, iFpiip)

// VK_KHR_dedicated_allocation
// no functions

// VK_KHR_image_format_list
// no functions

// VK_KHR_shader_draw_parameters
// no functions

// VK_EXT_conditional_rendering
GO(vkCmdBeginConditionalRenderingEXT, vFpP)
GO(vkCmdEndConditionalRenderingEXT, vFp)

// VK_EXT_depth_clip_enable
// no functions

// VK_EXT_host_query_reset
GO(vkResetQueryPoolEXT, vFpUuu)

// VK_EXT_memory_priority
// no functions

// VK_EXT_shader_demote_to_helper_invocation
// no functions

// VK_EXT_transform_feedback
GO(vkCmdBeginQueryIndexedEXT, vFpUuiu)
GO(vkCmdBeginTransformFeedbackEXT, vFpuupp)
GO(vkCmdBindTransformFeedbackBuffersEXT, vFpuuppp)
GO(vkCmdDrawIndirectByteCountEXT, vFpuuUUuu)
GO(vkCmdEndQueryIndexedEXT, vFpUuu)
GO(vkCmdEndTransformFeedbackEXT, vFpuupp)

// VK_EXT_vertex_attribute_divisor
// no functions

// VK_EXT_full_screen_exclusive
GO(vkAcquireFullScreenExclusiveModeEXT, iFpU)
GO(vkGetPhysicalDeviceSurfacePresentModes2EXT, iFpPpp)
GO(vkReleaseFullScreenExclusiveModeEXT, iFpU)
GO(vkGetDeviceGroupSurfacePresentModes2EXT, iFpPp)

// VK_EXT_calibrated_timestamps
GO(vkGetCalibratedTimestampsEXT, iFpuppp)
GO(vkGetPhysicalDeviceCalibrateableTimeDomainsEXT, iFppp)

// VK_EXT_sample_locations
GO(vkCmdSetSampleLocationsEXT, vFpP)
GO(vkGetPhysicalDeviceMultisamplePropertiesEXT, vFpiP)

// VK_EXT_headless_surface
GOM(vkCreateHeadlessSurfaceEXT, iFEpPpp)