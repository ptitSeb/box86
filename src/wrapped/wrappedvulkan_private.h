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
// Warning with vkGetXXX function, the structure that get the asnwer cannot be 'P'

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
GO(vkEnumeratePhysicalDeviceGroups, iFppP)
GOM(vkGetBufferMemoryRequirements2, iFEpPp)
GOM(vkGetImageMemoryRequirements2, vFEpPp)
GO(vkGetImageSparseMemoryRequirements2, vFpppp)
GO(vkGetDescriptorSetLayoutSupport, vFpPp)
GO(vkGetDeviceGroupPeerMemoryFeatures, vFpuuup)
GO(vkGetDeviceQueue2, vFpPp)
GO(vkGetPhysicalDeviceExternalBufferProperties, vFpPp)
GO(vkGetPhysicalDeviceExternalFenceProperties, vFpPp)
GO(vkGetPhysicalDeviceExternalSemaphoreProperties, vFpPp)
GO(vkGetPhysicalDeviceFeatures2, vFpp)  // VkPhysicalDeviceFeatures seems OK
GO(vkGetPhysicalDeviceFormatProperties2, vFpip)
GOM(vkGetPhysicalDeviceImageFormatProperties2, iFEpPp)
GO(vkGetPhysicalDeviceMemoryProperties2, vFpp)
GOM(vkGetPhysicalDeviceProperties2, vFEpp)
GO(vkGetPhysicalDeviceQueueFamilyProperties2, iFppp)    //VkQueueFamilyProperties2 seems OK
GO(vkGetPhysicalDeviceSparseImageFormatProperties2, vFpPpp) //VkSparseImageFormatProperties2 seems OK
GO(vkGetPhysicalDeviceToolProperties, iFppp)
GO(vkUpdateDescriptorSetWithTemplate, vFpUUp)
GO(vkTrimCommandPool, vFpUi)

// VK_VERSION_1_2
GO(vkResetQueryPool, vFpUuu)
GO(vkCmdBeginRenderPass2, vFpPP)
GO(vkCmdEndRenderPass2, vFpP)
GO(vkCmdNextSubpass2, vFpPP)
GOM(vkCreateRenderPass2, iFEpPpp)
GO(vkCmdDrawIndexedIndirectCount, vFpUUUUuu)
GO(vkCmdDrawIndirectCount, vFpUUUUuu)
GO(vkGetBufferDeviceAddress, UFpp)
GO(vkGetBufferOpaqueCaptureAddress, UFpp)
GO(vkGetDeviceMemoryOpaqueCaptureAddress, UFpp)
GO(vkGetSemaphoreCounterValue, iFpUp)
GO(vkSignalSemaphore, iFpP)
GO(vkWaitSemaphores, iFpPU)

// VK_VERSION_1_3
GO(vkCmdBeginRendering, vFpP)
GO(vkCmdEndRendering, vFp)
GO(vkCmdBlitImage2, vFpP)
GO(vkCmdCopyBuffer2, vFpP)
GO(vkCmdCopyBufferToImage2, vFpP)
GO(vkCmdCopyImage2, vFpP)
GO(vkCmdCopyImageToBuffer2, vFpP)
GO(vkCmdResolveImage2, vFpP)
GO(vkCmdSetDepthBiasEnable, vFpi)
GO(vkCmdSetLogicOp, vFpi)
GO(vkCmdSetPatchControlPoints, vFpu)
GO(vkCmdSetPrimitiveRestartEnable, vFpi)
GO(vkCmdSetRasterizerDiscardEnable, vFpi)
GOM(vkCreatePrivateDataSlot, iFEpPpp)
GOM(vkDestroyPrivateDataSlot, vFEpUp)
GO(vkGetPrivateData, vFpiUUp)
GO(vkSetPrivateData, iFpiUUU)
GO(vkGetDeviceBufferMemoryRequirements, vFpPP)
GO(vkGetDeviceImageMemoryRequirements, vFpPP)
GOM(vkGetDeviceImageSparseMemoryRequirements, vFEpPpp)
GO(vkCmdPipelineBarrier2, vFpP)
GO(vkCmdResetEvent2, vFpUU)
GO(vkCmdSetEvent2, vFpUP)
GO(vkCmdWaitEvents2, vFpupP)
GO(vkCmdWriteTimestamp2, vFpUUu)
GOM(vkQueueSubmit2, iFEpupU)
GO(vkCmdBindVertexBuffers2, vFpuupppp)
GO(vkCmdSetCullMode, vFpu)
GO(vkCmdSetDepthBoundsTestEnable, vFpu)
GO(vkCmdSetDepthCompareOp, vFpu)
GO(vkCmdSetDepthTestEnable, vFpu)
GO(vkCmdSetDepthWriteEnable, vFpu)
GO(vkCmdSetFrontFace, vFpu)
GO(vkCmdSetPrimitiveTopology, vFpi)
GO(vkCmdSetScissorWithCount, vFpup)
GO(vkCmdSetStencilOp, vFpuiiii)
GO(vkCmdSetStencilTestEnable, vFpu)
GO(vkCmdSetViewportWithCount, vFpup)

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
GO(vkGetPhysicalDeviceExternalBufferPropertiesKHR, vFpPp)

// VK_KHR_get_physical_device_properties2
GO(vkGetPhysicalDeviceFeatures2KHR, vFpp)
GO(vkGetPhysicalDeviceFormatProperties2KHR, vFpip)
GOM(vkGetPhysicalDeviceImageFormatProperties2KHR, iFEpPp)
GO(vkGetPhysicalDeviceMemoryProperties2KHR, vFpp)
GOM(vkGetPhysicalDeviceProperties2KHR, vFEpp)
GO(vkGetPhysicalDeviceQueueFamilyProperties2KHR, vFppp)
GO(vkGetPhysicalDeviceSparseImageFormatProperties2KHR, vFpPpp)  //VkSparseImageFormatProperties2 seems OK

// VK_KHR_get_surface_capabilities2
GOM(vkGetPhysicalDeviceSurfaceCapabilities2KHR, iFEpPp)
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
GO(vkGetDeviceGroupPresentCapabilitiesKHR, iFpp)
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
GOM(vkGetPhysicalDeviceSurfaceCapabilities2EXT, iFEpUp)

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
GOM(vkGetBufferMemoryRequirements2KHR, iFEpPp)
GOM(vkGetImageMemoryRequirements2KHR, vFEpPp)
GO(vkGetImageSparseMemoryRequirements2KHR, vFpppp)

// VK_KHR_external_fence_capabilities
GO(vkGetPhysicalDeviceExternalFencePropertiesKHR, vFpPp)

// VK_KHR_external_semaphore_capabilities
GO(vkGetPhysicalDeviceExternalSemaphorePropertiesKHR, vFpPp)

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
GO(vkGetCalibratedTimestampsEXT, iFpuPpp)
GO(vkGetPhysicalDeviceCalibrateableTimeDomainsEXT, iFppp)

// VK_EXT_sample_locations
GO(vkCmdSetSampleLocationsEXT, vFpP)
GO(vkGetPhysicalDeviceMultisamplePropertiesEXT, vFpip)

// VK_EXT_headless_surface
GOM(vkCreateHeadlessSurfaceEXT, iFEpPpp)

//VK_KHR_performance_query
GO(vkAcquireProfilingLockKHR, iFpP)
GO(vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR, iFpuppp)
GO(vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR, vFppp)
GO(vkReleaseProfilingLockKHR, vFp)

// VK_NV_cooperative_matrix
GO(vkGetPhysicalDeviceCooperativeMatrixPropertiesNV, iFppp)

// VK_KHR_fragment_shading_rate
GO(vkCmdSetFragmentShadingRateKHR, vFppp)
GO(vkGetPhysicalDeviceFragmentShadingRatesKHR, iFppp)

// VK_NV_coverage_reduction_mode
GO(vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV, iFppp)

// VK_EXT_tooling_info
GO(vkGetPhysicalDeviceToolPropertiesEXT, iFppp)

// VK_KHR_create_renderpass2
GO(vkCmdBeginRenderPass2KHR, vFpPP)
GO(vkCmdEndRenderPass2KHR, vFpP)
GO(vkCmdNextSubpass2KHR, vFpPP)
GOM(vkCreateRenderPass2KHR, iFEpPpp)

// VK_KHR_draw_indirect_count
GO(vkCmdDrawIndexedIndirectCountKHR, vFpUUUUuu)
GO(vkCmdDrawIndirectCountKHR, vFpUUUUuu)

// VK_AMD_draw_indirect_count
GO(vkCmdDrawIndexedIndirectCountAMD, vFpUUUUuu)
GO(vkCmdDrawIndirectCountAMD, vFpUUUUuu)

// VK_AMD_buffer_marker
GO(vkCmdWriteBufferMarkerAMD, vFpuUUu)

// VK_AMD_shader_info
GO(vkGetShaderInfoAMD, iFpUuupp)

// VK_EXT_debug_marker
GO(vkCmdDebugMarkerBeginEXT, vFpP)
GO(vkCmdDebugMarkerEndEXT, vFp)
GO(vkCmdDebugMarkerInsertEXT, vFpP)
GO(vkDebugMarkerSetObjectNameEXT, iFpP)
GO(vkDebugMarkerSetObjectTagEXT, iFpP)

// VK_EXT_discard_rectangles
GO(vkCmdSetDiscardRectangleEXT, vFpuup)

// VK_EXT_display_control
GO(vkDisplayPowerControlEXT, iFpUP)
GO(vkGetSwapchainCounterEXT, iFpUup)
GOM(vkRegisterDeviceEventEXT, iFEpPpp)
GOM(vkRegisterDisplayEventEXT, iFEpUPpp)

// VK_EXT_external_memory_host
GO(vkGetMemoryHostPointerPropertiesEXT, iFpupp)

// VK_EXT_hdr_metadata
GO(vkSetHdrMetadataEXT, vFpupp)

// VK_EXT_validation_cache
GOM(vkCreateValidationCacheEXT, iFEpPpp)
GOM(vkDestroyValidationCacheEXT, vFEpUp)
GO(vkGetValidationCacheDataEXT, iFpUpp)
GO(vkMergeValidationCachesEXT, iFpUup)

// VK_GOOGLE_display_timing
GOM(vkGetPastPresentationTimingGOOGLE, iFEpUpP)
GO(vkGetRefreshCycleDurationGOOGLE, iFpUp)

// VK_KHR_external_fence_fd
GO(vkGetFenceFdKHR, iFpPp)
GO(vkImportFenceFdKHR, iFpP)

// VK_KHR_external_semaphore_fd
GO(vkGetSemaphoreFdKHR, iFpPp)
GO(vkImportSemaphoreFdKHR, iFpP)

// VK_KHR_push_descriptor
GO(vkCmdPushDescriptorSetKHR, vFpiUuup) // Array of P at the end, but base structure doesn't seems to need aligning...

// VK_KHR_shared_presentable_image
GO(vkGetSwapchainStatusKHR, iFpU)

// VK_NV_clip_space_w_scaling
GO(vkCmdSetViewportWScalingNV, vFpuup)  // Array of P at the end, but base structure doesn't seems to need aligning...

// VK_EXT_acquire_xlib_display
GO(vkAcquireXlibDisplayEXT, iFppU)
GO(vkGetRandROutputDisplayEXT, iFppLp)

// VK_KHR_timeline_semaphore
GO(vkGetSemaphoreCounterValueKHR, iFpUp)
GO(vkSignalSemaphoreKHR, iFpP)
GO(vkWaitSemaphoresKHR, iFpPU)

// VK_KHR_copy_commands2
GO(vkCmdBlitImage2KHR, vFpP)
GO(vkCmdCopyBuffer2KHR, vFpP)
GO(vkCmdCopyBufferToImage2KHR, vFpP)
GO(vkCmdCopyImage2KHR, vFpP)
GO(vkCmdCopyImageToBuffer2KHR, vFpP)
GO(vkCmdResolveImage2KHR, vFpP)

// VK_KHR_buffer_device_address
GO(vkGetBufferDeviceAddressKHR, UFpp)
GO(vkGetBufferOpaqueCaptureAddressKHR, UFpp)
GO(vkGetDeviceMemoryOpaqueCaptureAddressKHR, UFpp)

// VK_EXT_buffer_device_address
GO(vkGetBufferDeviceAddressEXT, UFpp)

// VK_KHR_dynamic_rendering
GO(vkCmdBeginRenderingKHR, vFpP)
GO(vkCmdEndRenderingKHR, vFp)

// VK_EXT_extended_dynamic_state2
GO(vkCmdSetDepthBiasEnableEXT, vFpi)
GO(vkCmdSetLogicOpEXT, vFpi)
GO(vkCmdSetPatchControlPointsEXT, vFpu)
GO(vkCmdSetPrimitiveRestartEnableEXT, vFpi)
GO(vkCmdSetRasterizerDiscardEnableEXT, vFpi)

// VK_EXT_private_data
GOM(vkCreatePrivateDataSlotEXT, iFEpPpp)
GOM(vkDestroyPrivateDataSlotEXT, vFEpUp)
GO(vkGetPrivateDataEXT, vFpiUUp)
GO(vkSetPrivateDataEXT, iFpiUUU)

// VK_KHR_maintenance4
GO(vkGetDeviceBufferMemoryRequirementsKHR, vFpPP)
GO(vkGetDeviceImageMemoryRequirementsKHR, vFpPP)
GOM(vkGetDeviceImageSparseMemoryRequirementsKHR, vFEpPpp)

// VK_KHR_synchronization2
GO(vkCmdPipelineBarrier2KHR, vFpP)
GO(vkCmdResetEvent2KHR, vFpUU)
GO(vkCmdSetEvent2KHR, vFpUP)
GO(vkCmdWaitEvents2KHR, vFpupP)
GO(vkCmdWriteTimestamp2KHR, vFpUUu)
GOM(vkQueueSubmit2KHR, iFEpupU)

// VK_EXT_extended_dynamic_state
GO(vkCmdBindVertexBuffers2EXT, vFpuupppp)
GO(vkCmdSetCullModeEXT, vFpu)
GO(vkCmdSetDepthBoundsTestEnableEXT, vFpu)
GO(vkCmdSetDepthCompareOpEXT, vFpu)
GO(vkCmdSetDepthTestEnableEXT, vFpu)
GO(vkCmdSetDepthWriteEnableEXT, vFpu)
GO(vkCmdSetFrontFaceEXT, vFpu)
GO(vkCmdSetPrimitiveTopologyEXT, vFpi)
GO(vkCmdSetScissorWithCountEXT, vFpup)
GO(vkCmdSetStencilOpEXT, vFpuiiii)
GO(vkCmdSetStencilTestEnableEXT, vFpu)
GO(vkCmdSetViewportWithCountEXT, vFpup)

// VK_EXT_shader_module_identifier
GO(vkGetShaderModuleCreateInfoIdentifierEXT, vFpPP)
GO(vkGetShaderModuleIdentifierEXT, vFpUp)

// VK_NV_optical_flow
GO(vkBindOpticalFlowSessionImageNV, iFpUiUi)
GO(vkCmdOpticalFlowExecuteNV, vFpUP)
GOM(vkCreateOpticalFlowSessionNV, iFEpPpp)
GOM(vkDestroyOpticalFlowSessionNV, vFEpUp)
GOM(vkGetPhysicalDeviceOpticalFlowImageFormatsNV, iFEpPpp)

// VK_EXT_extended_dynamic_state3
GO(vkCmdSetAlphaToCoverageEnableEXT, vFpi)
GO(vkCmdSetAlphaToOneEnableEXT, vFpi)
GO(vkCmdSetColorBlendAdvancedEXT, vFpuup)
GO(vkCmdSetColorBlendEnableEXT, vFpuup)
GO(vkCmdSetColorBlendEquationEXT, vFpuup)
GO(vkCmdSetColorWriteMaskEXT, vFpuup)
GO(vkCmdSetConservativeRasterizationModeEXT, vFpi)
GO(vkCmdSetCoverageModulationModeNV, vFpi)
GO(vkCmdSetCoverageModulationTableEnableNV, vFpi)
GO(vkCmdSetCoverageModulationTableNV, vFpup)
GO(vkCmdSetCoverageReductionModeNV, vFpi)
GO(vkCmdSetCoverageToColorEnableNV, vFpi)
GO(vkCmdSetCoverageToColorLocationNV, vFpu)
GO(vkCmdSetDepthClampEnableEXT, vFpi)
GO(vkCmdSetDepthClipEnableEXT, vFpi)
GO(vkCmdSetDepthClipNegativeOneToOneEXT, vFpi)
GO(vkCmdSetExtraPrimitiveOverestimationSizeEXT, vFpf)
GO(vkCmdSetLineRasterizationModeEXT, vFpi)
GO(vkCmdSetLineStippleEnableEXT, vFpi)
GO(vkCmdSetLogicOpEnableEXT, vFpi)
GO(vkCmdSetPolygonModeEXT, vFpi)
GO(vkCmdSetProvokingVertexModeEXT, vFpi)
GO(vkCmdSetRasterizationSamplesEXT, vFpi)
GO(vkCmdSetRasterizationStreamEXT, vFpu)
GO(vkCmdSetRepresentativeFragmentTestEnableNV, vFpi)
GO(vkCmdSetSampleLocationsEnableEXT, vFpi)
GO(vkCmdSetSampleMaskEXT, vFpip)
GO(vkCmdSetShadingRateImageEnableNV, vFpi)
GO(vkCmdSetTessellationDomainOriginEXT, vFpi)
GO(vkCmdSetViewportSwizzleNV, vFpuup)
GO(vkCmdSetViewportWScalingEnableNV, vFpi)

// VK_EXT_swapchain_maintenance1
GO(vkReleaseSwapchainImagesEXT, iFpp)

// VK_EXT_depth_bias_control
GO(vkCmdSetDepthBias2EXT, vFpP)

// VK_KHR_present_wait
GO(vkWaitForPresentKHR, iFpUUU)

// VK_KHR_cooperative_matrix
GO(vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR, iFppp)    // last arg should be P, but it's an array, with size as second arg *uint32_t and struct looks fine

// VK_KHR_maintenance5
GO(vkCmdBindIndexBuffer2KHR, vFpUUUu)
GO(vkGetDeviceImageSubresourceLayoutKHR, vFpPP)
GO(vkGetImageSubresourceLayout2KHR, vFpUPP)
GO(vkGetRenderingAreaGranularityKHR, vFpPp)

// VK_KHR_calibrated_timestamps
GO(vkGetCalibratedTimestampsKHR, iFpuPpp)
GO(vkGetPhysicalDeviceCalibrateableTimeDomainsKHR, iFppp)

// VK_KHR_video_queue
GOM(vkBindVideoSessionMemoryKHR, iFEpUup)
GO(vkCmdBeginVideoCodingKHR, vFpP)
GO(vkCmdControlVideoCodingKHR, vFpP)
GO(vkCmdEndVideoCodingKHR, vFpP)
GOM(vkCreateVideoSessionKHR, iFEpppp)
GOM(vkCreateVideoSessionParametersKHR, iFEpppp)
GOM(vkDestroyVideoSessionKHR, vFEpUp)
GOM(vkDestroyVideoSessionParametersKHR, vFEpUp)
GO(vkGetPhysicalDeviceVideoCapabilitiesKHR, iFpPp)
GO(vkGetPhysicalDeviceVideoFormatPropertiesKHR, iFpPpp)
GO(vkGetVideoSessionMemoryRequirementsKHR, iFpUpp)
GO(vkUpdateVideoSessionParametersKHR, iFpUp)

//VK_KHR_video_encode_queue
GO(vkCmdEncodeVideoKHR, vFpP)
GO(vkGetEncodedVideoSessionParametersKHR, iFpPPpp)
GO(vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR, iFpPP)
