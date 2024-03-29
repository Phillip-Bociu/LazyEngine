#include <assert.h>
#include "LzyVector.h"
#include "LzyLog.h"
#include "LzyObjParser.h"
#include "LzyRenderer.h"
#include "LzyApplication.h"
#include "LzyMemory.h"
#include "LzyFile.h"
#include "deps/vma/vk_mem_alloc.h"
#include <vulkan/vulkan.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct LzyTestPushConstants
{
    f32 x,y;
}LzyTestPushConstants;

typedef struct LzyQueueFamilyIndices
{
	u8 uGraphicsIndex;
	u8 uPresentIndex;
    u8 uTransferIndex;
    u8 uComputeIndex;
} LzyQueueFamilyIndices;

//TODO: Optimize this with bitsets
typedef struct LzySwapchainSupportDetails
{
	VkSurfaceFormatKHR pFormats[VK_FORMAT_RANGE_SIZE];
	VkPresentModeKHR pPresentModes[VK_PRESENT_MODE_RANGE_SIZE_KHR];
	u32 uFormatCount;
	u32 uPresentModeCount;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
} LzySwapchainSupportDetails;

typedef struct LzyVulkanBuffer
{
	VkBuffer buffer;
    void* pData;
    u64 uSize;
	VmaAllocation allocation;    
    VmaAllocationInfo allocInfo;
} LzyVulkanBuffer;

typedef struct LzyVulkanImage
{
    VkImage image;
    VmaAllocation allocation;
}LzyVulkanImage;

typedef struct LzySwapchain
{
	VkExtent2D swapchainExtent;
	VkFormat swapchainImageFormat;
	VkSwapchainKHR swapchain;
    VkCommandPool commandPool;
	VkImage* pSwapchainImages;
    LzyVulkanImage* pDepthImages;
    VkImageView* pDepthImageViews;
	VkFramebuffer *pSwapchainFramebuffers;
	VkImageView *pSwapchainImageViews;
    VkSemaphore* pImageAcquireSemaphores;
    VkSemaphore* pRenderFinishedSemaphores;
    VkFence* pInFlightFences;
    VkFence* pImagesInFlight;
    VkCommandBuffer* pCommandBuffers;
	u32 uSwapchainImageCount;
}LzySwapchain;


typedef struct LzyRendererState
{
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	
    VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;
    VkQueue computeQueue;
    
    VkSurfaceKHR surface;
    LzyQueueFamilyIndices queueFamilies;
    VmaAllocator vmaAllocator;
    
    LzySwapchain swapchainInfo;
    
	VkRenderPass renderPass;
	VkShaderModule triangleVertexShader;
	VkShaderModule triangleFragmentShader;
    VkPipelineLayout trianglePipelineLayout;
	VkPipeline trianglePipeline;
	VkDebugReportCallbackEXT debugMessenger;
	VkCommandPool transferPool;
    VkCommandPool computePool;
    LzyVulkanBuffer vertexBuffer;
	LzyVulkanBuffer indexBuffer;
    VkDescriptorSetLayout vertexBufferLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet vertexBufferDescriptorSet;
    
    u32 uFrameIndex;
} LzyRendererState;


global b8 bRendererInitialized = false;
global LzyRendererState rendererState;

internal_func
b8 lzy_create_vma_allocator(VmaAllocator* pAllocator)
{
    VmaAllocatorCreateInfo createInfo = {};
    createInfo.physicalDevice = rendererState.physicalDevice;
    createInfo.device = rendererState.device;
    createInfo.instance = rendererState.instance;
    createInfo.vulkanApiVersion = VK_API_VERSION_1_2;
    
    if(vmaCreateAllocator(&createInfo,
                          pAllocator) != VK_SUCCESS)
    {
        LCOREFATAL("Could not create allocator");
        return false;    
    }
    
    return true;
}

internal_func
b8 lzy_create_exclusive_image(LzyVulkanImage* pImage,
                              VkFormat imageFormat,          
                              VkImageUsageFlags usageFlags,
                              VmaMemoryUsage memoryUsageFlags,
                              VkMemoryPropertyFlags requiredMemoryFlags,
                              VkExtent3D imageExtent)
{   
    
    
    VkImageCreateInfo createInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.format = imageFormat;
    createInfo.extent = imageExtent;
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = 1;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = usageFlags;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsageFlags;
    allocInfo.requiredFlags = requiredMemoryFlags;
    
    VmaAllocationInfo allocResult;
    
    if(vmaCreateImage(rendererState.vmaAllocator,
                      &createInfo,
                      &allocInfo,
                      &pImage->image,
                      &pImage->allocation,
                      &allocResult) != VK_SUCCESS)
    {
        LCOREFATAL("Could not create image");
        return false;
    }
    
    return true;
}


internal_func
b8 lzy_create_concurrent_image(VkImage* pImage,
                               VkFormat imageFormat,          
                               VkImageUsageFlags usageFlags,
                               VkExtent3D imageExtent)
{
    
    VkImageCreateInfo createInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.format = imageFormat;
    createInfo.extent = imageExtent;
    createInfo.mipLevels = 1;
    createInfo.arrayLayers = 1;
    createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = usageFlags;
    createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    if(vkCreateImage(rendererState.device, &createInfo, NULL, pImage) != VK_SUCCESS)
    {
        LCOREFATAL("Could not create Image");
        return false;
    }
    return true;
}


internal_func
VkDescriptorSetLayoutBinding
lzy_create_descriptor_set_layout_binding(u32 uBinding,
                                         VkDescriptorType descriptorType,
                                         u32 uDescriptorCount,
                                         VkShaderStageFlags stageFlags)
{
    
    VkDescriptorSetLayoutBinding retval = {0};
    
    retval.binding = uBinding;
    retval.descriptorType = descriptorType;
    retval.descriptorCount = uDescriptorCount;
    retval.stageFlags = stageFlags;
    retval.pImmutableSamplers = 0;
    
    return retval;
}

internal_func b8 lzy_create_descriptor_set_layout(VkDescriptorSetLayout* pLayout, 
                                                  VkDescriptorSetLayoutCreateFlags flags, 
                                                  VkDescriptorSetLayoutBinding* pBindings, 
                                                  u32 uBindingCount)
{
    
    VkDescriptorSetLayoutCreateInfo createInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    
    createInfo.flags = flags;
    createInfo.bindingCount = uBindingCount;
    createInfo.pBindings = pBindings;
    
    if(vkCreateDescriptorSetLayout(rendererState.device, &createInfo, NULL, pLayout) != VK_SUCCESS)
    {
        LCOREERROR("Could not create descriptor set layout");
        return false;
    }
    
    return true;
}

internal_func b8 lzy_create_descriptor_pool(VkDescriptorPool* pPool, 
                                            VkDescriptorPoolCreateFlags flags, 
                                            u32 uMaxSetCount, 
                                            VkDescriptorPoolSize* pSizes, 
                                            u32 uPoolSizeCount)
{
    VkDescriptorPoolCreateInfo createInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    
    createInfo.flags = flags;
    createInfo.maxSets = uMaxSetCount;
    createInfo.poolSizeCount = uPoolSizeCount;
    createInfo.pPoolSizes = pSizes;
    
    if(vkCreateDescriptorPool(rendererState.device, &createInfo, NULL, pPool) != VK_SUCCESS)
    {
        LCOREERROR("Could not create Descriptor Pool");
        return false;
    }
    
    return true;
}


internal_func b8 lzy_create_descriptor_set(VkDescriptorSet* pDescriptorSets, 
                                           u32 uDescriptorSetCount, 
                                           VkDescriptorPool pool, 
                                           VkDescriptorSetLayout* pLayouts)
{
    
    VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = uDescriptorSetCount;
    allocInfo.pSetLayouts = pLayouts;
    
    if(vkAllocateDescriptorSets(rendererState.device, &allocInfo, pDescriptorSets) != VK_SUCCESS)
    {
        LCOREERROR("Could not allocate Descriptor Sets");
        return false;
    }
    
    return true;
}




internal_func u32 lzy_select_memory_type_bits(const VkPhysicalDeviceMemoryProperties *pMemoryProps, u32 uMemoryTypeBits, VkMemoryPropertyFlags flags)
{
	for (u32 i = 0; i < pMemoryProps->memoryTypeCount; i++)
	{
		if ((uMemoryTypeBits & (1 << i)) && (pMemoryProps->memoryTypes[i].propertyFlags & flags) == flags)
		{
			return i;
		}
	}
    
	return ~0u;
}

internal_func
b8 lzy_create_buffer(LzyVulkanBuffer *pBuffer,
                     const VkPhysicalDeviceMemoryProperties *pMemoryProps,
                     u64 uSize,
                     VkBufferUsageFlags usage,
                     VmaMemoryUsage memoryLocation)
{
	VkBufferCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = uSize,
		.usage = usage};
    
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryLocation;
    
    if(vmaCreateBuffer(rendererState.vmaAllocator,
                       &createInfo,
                       &allocInfo,
                       &pBuffer->buffer,
                       &pBuffer->allocation,
                       &pBuffer->allocInfo) != VK_SUCCESS)
    {
        LCOREFATAL("Could not create buffer");
        return false;
    }
    
    pBuffer->uSize = uSize;
    pBuffer->pData = NULL;
    
    LCOREINFO("Before Mapping");
    
    if(memoryLocation == VMA_MEMORY_USAGE_CPU_TO_GPU || memoryLocation == VMA_MEMORY_USAGE_GPU_TO_CPU)
        vmaMapMemory(rendererState.vmaAllocator,
                     pBuffer->allocation,
                     &pBuffer->pData);
    
    LCOREINFO("After Mapping");
    
    return true;
}

internal_func void lzy_destroy_buffer(LzyVulkanBuffer *pBuffer)
{
	vmaDestroyBuffer(rendererState.vmaAllocator,
                     pBuffer->buffer,
                     pBuffer->allocation);
}

internal_func b8 lzy_create_graphics_pipeline_layout(VkPipelineLayout *pLayout,
                                                     u32 uSetLayoutCount,
                                                     VkDescriptorSetLayout* pSetLayouts,
                                                     u32 uPushContantRangeCount,
                                                     VkPushConstantRange* pPushConstantRanges)
{
    
	VkPipelineLayoutCreateInfo createInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    createInfo.setLayoutCount = uSetLayoutCount;
    createInfo.pSetLayouts = pSetLayouts;
    createInfo.pushConstantRangeCount = uPushContantRangeCount;
    createInfo.pPushConstantRanges = pPushConstantRanges;
    
	if (vkCreatePipelineLayout(rendererState.device, &createInfo, NULL, pLayout) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create graphics pipeline layout");
		return false;
	}
    
	return true;
}

internal_func b8 lzy_create_graphics_pipeline(VkPipeline *pPipeline, VkShaderModule shaderModuleVertex, VkShaderModule shaderModuleFragment)
{
    
    VkPushConstantRange pushConstant;
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstant.offset = 0;
    pushConstant.size = sizeof(LzyTestPushConstants);
    
    
	if (!lzy_create_graphics_pipeline_layout(&rendererState.trianglePipelineLayout,
                                             1,
                                             &rendererState.vertexBufferLayout,
                                             1,
                                             &pushConstant))
	{
		return false;
	}
    
	VkGraphicsPipelineCreateInfo createInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    
	VkPipelineShaderStageCreateInfo shaderStageCreateInfos[2] = {0};
	shaderStageCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageCreateInfos[0].module = shaderModuleVertex;
	shaderStageCreateInfos[0].pName = "main";
    
	shaderStageCreateInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageCreateInfos[1].module = shaderModuleFragment;
	shaderStageCreateInfos[1].pName = "main";
    
	createInfo.stageCount = countof(shaderStageCreateInfos);
	createInfo.pStages = shaderStageCreateInfos;
    
	VkPipelineVertexInputStateCreateInfo inputStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	createInfo.pVertexInputState = &inputStateCreateInfo;
    
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    
	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.scissorCount = 1;
	createInfo.pViewportState = &viewportStateCreateInfo;
    
	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
	rasterizationStateCreateInfo.lineWidth = 1.0f;
	createInfo.pRasterizationState = &rasterizationStateCreateInfo;
    
	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
	multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.pMultisampleState = &multisampleStateCreateInfo;
    
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.minDepthBounds = 0.0f;
    depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
    depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
    
    createInfo.pDepthStencilState = &depthStencilStateCreateInfo;
    
    
    
	VkPipelineColorBlendAttachmentState colorAttachment = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
    
	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
	colorBlendStateCreateInfo.attachmentCount = 1;
	colorBlendStateCreateInfo.pAttachments = &colorAttachment;
	createInfo.pColorBlendState = &colorBlendStateCreateInfo;
    
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    
	VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	dynamicStateCreateInfo.dynamicStateCount = countof(dynamicStates);
	dynamicStateCreateInfo.pDynamicStates = dynamicStates;
    
	createInfo.pDynamicState = &dynamicStateCreateInfo;
	createInfo.layout = rendererState.trianglePipelineLayout;
	createInfo.renderPass = rendererState.renderPass;
    
	if (vkCreateGraphicsPipelines(rendererState.device, VK_NULL_HANDLE, 1, &createInfo, NULL, pPipeline) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create graphics pipeline");
		return false;
	}
	return true;
}

internal_func b8 lzy_create_shader_module(VkShaderModule *pShaderModule, const c8 *pShaderPath)
{
	
    LzyFile file;
	if (!lzy_file_open(&file, pShaderPath, LZY_FILE_MODE_READ | LZY_FILE_MODE_BINARY))
	{
		return false;
	}
    
	u64 uFileSize;
	lzy_file_get_size(file, &uFileSize);
	u32* pBuffer = lzy_alloc(uFileSize, 4, LZY_MEMORY_TAG_STRING);
	lzy_file_read(file, pBuffer, uFileSize);
    
	if ((u64)pBuffer == ~0ull)
	{
		LCOREFATAL("Could not read shader file %s", pShaderPath);
		return false;
	}
    
	VkShaderModuleCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = uFileSize,
		.pCode = pBuffer,
	};
    
	if (vkCreateShaderModule(rendererState.device, &createInfo, NULL, pShaderModule) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create shader module for file %s", pShaderModule);
		return false;
	}
    
	lzy_free(pBuffer, uFileSize, LZY_MEMORY_TAG_STRING);
	lzy_file_close(file);
	return true;
}

internal_func b8 lzy_create_command_pool(VkCommandPool *pCommandPool, u32 uFamilyIndex)
{
	VkCommandPoolCreateInfo createInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	createInfo.queueFamilyIndex = uFamilyIndex;
	createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
	if (vkCreateCommandPool(rendererState.device, &createInfo, NULL, pCommandPool) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create Command Pool");
		return false;
	}
	return true;
}

internal_func
b8 lzy_create_fence(VkFence* pFence, VkFenceCreateFlags flags)
{
    VkFenceCreateInfo createInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    createInfo.flags = flags;
    
    if(vkCreateFence(rendererState.device,
                     &createInfo,
                     NULL,
                     pFence) != VK_SUCCESS)
    {
        LCOREERROR("Could not createFence");
        return false;
    }
    
    return true;
}

internal_func b8 lzy_create_semaphore(VkSemaphore *pSemaphore)
{
	VkSemaphoreCreateInfo createInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    
	if (vkCreateSemaphore(rendererState.device, &createInfo, NULL, pSemaphore) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create Semaphore");
		return false;
	}
	return true;
}

internal_func
LzySwapchainSupportDetails
lzy_get_swapchain_support_details(VkPhysicalDevice physicalDevice)
{
	LzySwapchainSupportDetails retval = {0};
    
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, rendererState.surface, &retval.uFormatCount, NULL);
	if (retval.uFormatCount != 0)
	{
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, rendererState.surface, &retval.uFormatCount, retval.pFormats);
	}
    
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, rendererState.surface, &retval.uPresentModeCount, NULL);
	if (retval.uPresentModeCount != 0)
	{
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, rendererState.surface, &retval.uPresentModeCount, retval.pPresentModes);
	}
    
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, rendererState.surface, &retval.surfaceCapabilities);
	return retval;
}

internal_func b8 lzy_qfam_is_complete(LzyQueueFamilyIndices qFams)
{
	if (qFams.uGraphicsIndex == 255 || 
        qFams.uPresentIndex  == 255 ||
        qFams.uComputeIndex  == 255 ||
        qFams.uTransferIndex == 255)
		return false;
	else
		return true;
}

internal_func b8 lzy_check_device_extension_support(VkPhysicalDevice physicalDevice, const char **ppExtensionNames, u16 uNameCount)
{
	u32 uExtensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &uExtensionCount, NULL);
	VkExtensionProperties *pExtensions = lzy_alloc(sizeof(VkExtensionProperties) * uExtensionCount, 8, LZY_MEMORY_TAG_RENDERER_INIT);
	vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &uExtensionCount, pExtensions);
    
	for (u16 j = 0; j < uNameCount; j++)
	{
		b8 bFoundExtension = false;
		for (u32 i = 0; i < uExtensionCount; i++)
		{
			if (strcmp(pExtensions[i].extensionName, ppExtensionNames[j]) == 0)
			{
				bFoundExtension = true;
				break;
			}
		}
		if (!bFoundExtension)
			return false;
	}
    
	return true;
}

internal_func VkImageMemoryBarrier lzy_create_image_memory_barrier(VkImage image, VkAccessFlags accessMaskSrc, VkAccessFlags accessMaskDst, VkImageLayout imageLayoutOld, VkImageLayout imageLayoutNew)
{
	VkImageMemoryBarrier retval = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    
	retval.srcAccessMask = accessMaskSrc;
	retval.dstAccessMask = accessMaskDst;
	retval.oldLayout = imageLayoutOld;
	retval.newLayout = imageLayoutNew;
	retval.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	retval.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	retval.image = image;
	retval.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	retval.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	retval.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    
	return retval;
}

internal_func b8 lzy_is_device_suitable(VkPhysicalDevice physicalDevice, const char **ppExtensionNames, u16 uNameCount, LzySwapchainSupportDetails *pDetails)
{
    
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(physicalDevice, &props);
	if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
	{
		if (lzy_check_device_extension_support(physicalDevice, ppExtensionNames, uNameCount))
		{
			LzySwapchainSupportDetails details = lzy_get_swapchain_support_details(physicalDevice);
			if (details.uFormatCount != 0 && details.uPresentModeCount != 0)
			{
				*pDetails = details;
				LCOREINFO("Picking physical device: %s", props.deviceName);
				return true;
			}
		}
	}
    
	return false;
}

internal_func b8 lzy_check_vulkan_version()
{
	u32 uSupportedInstanceVersion = 0;
	vkEnumerateInstanceVersion(&uSupportedInstanceVersion);
	if (uSupportedInstanceVersion < VK_MAKE_VERSION(1, 2, 0))
	{
		u16 major, minor, patch;
		major = VK_VERSION_MAJOR(uSupportedInstanceVersion);
		minor = VK_VERSION_MINOR(uSupportedInstanceVersion);
		patch = VK_VERSION_PATCH(uSupportedInstanceVersion);
        
		LTRACE("(%u, %u, %u)", major, minor, patch);
        
		LCOREFATAL("Currently installed vulkan version (%u.%u.%u) is older than the minimun required (1.2.0)", major, minor, patch);
		return false;
	}
    
	return true;
}

internal_func b8 lzy_create_instance()
{
	u32 uSupportedInstanceVersion;
	vkEnumerateInstanceVersion(&uSupportedInstanceVersion);
    
	VkApplicationInfo appInfo = {0};
    
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_API_VERSION_1_2;
    
	u32 uFoundLayers = 0;
#ifndef _DEBUG
	const char **ppValidationLayers = {0};
#else
	const char *ppValidationLayers[] = {"VK_LAYER_KHRONOS_validation"};
	u32 uLayerCount = 0;
    
	vkEnumerateInstanceLayerProperties(&uLayerCount, NULL);
	VkLayerProperties *pLayerProperties = lzy_alloc(uLayerCount * sizeof(VkLayerProperties), 8, LZY_MEMORY_TAG_RENDERER_INIT);
	vkEnumerateInstanceLayerProperties(&uLayerCount, pLayerProperties);
    
	for (u32 i = 0; i < uLayerCount && uFoundLayers < sizeof(ppValidationLayers) / sizeof(const char *); i++)
	{
		for (u32 j = 0; j < sizeof(ppValidationLayers) / sizeof(const char *); j++)
			if (strcmp(pLayerProperties[i].layerName, ppValidationLayers[j]) == 0)
        {
            uFoundLayers++;
            break;
        }
	}
    
	if (uFoundLayers != sizeof(ppValidationLayers) / sizeof(const char *))
	{
		LCOREFATAL("Not all required validation layers supported");
		return false;
	}
#endif
	u32 uFoundExtensions = 0;
	u32 uExtensionCount = 0;
    
#ifdef _DEBUG
	const char *ppExtensionNames[] = {LZY_SURFACE_EXT_NAME, "VK_KHR_surface", VK_EXT_DEBUG_REPORT_EXTENSION_NAME};
#else
	const char *ppExtensionNames[] = {LZY_SURFACE_EXT_NAME, "VK_KHR_surface"};
#endif
    
	vkEnumerateInstanceExtensionProperties(NULL, &uExtensionCount, NULL);
	VkExtensionProperties *pExtensions = lzy_alloc(sizeof(VkExtensionProperties) * uExtensionCount, 4, LZY_MEMORY_TAG_RENDERER_INIT);
	vkEnumerateInstanceExtensionProperties(NULL, &uExtensionCount, pExtensions);
    
	for (u32 i = 0; i < uExtensionCount && uFoundExtensions < sizeof(ppExtensionNames) / sizeof(const char *); i++)
	{
		for (u32 j = 0; j < sizeof(ppExtensionNames) / sizeof(const char *); j++)
			if (strcmp(pExtensions[i].extensionName, ppExtensionNames[j]) == 0)
        {
            uFoundExtensions++;
            break;
        }
	}
    
	if (uFoundExtensions != sizeof(ppExtensionNames) / sizeof(const char *))
	{
		LCOREFATAL("Not all required vulkan extensions supported");
		return false;
	}
    
	VkInstanceCreateInfo instanceCreateInfo = {0};
    
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = uFoundExtensions;
	instanceCreateInfo.ppEnabledExtensionNames = ppExtensionNames;
	instanceCreateInfo.enabledLayerCount = uFoundLayers;
	instanceCreateInfo.ppEnabledLayerNames = ppValidationLayers;
    
	if (vkCreateInstance(&instanceCreateInfo, NULL, &rendererState.instance) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create vulkan instance");
		return false;
	}
	return true;
}

internal_func VkBool32 lzy_debug_callback(
                                          VkDebugReportFlagsEXT flags,
                                          VkDebugReportObjectTypeEXT objectType,
                                          uint64_t object,
                                          size_t location,
                                          int32_t messageCode,
                                          const char *pLayerPrefix,
                                          const char *pMessage,
                                          void *pUserData)
{
	
    if (flags & (VK_DEBUG_REPORT_ERROR_BIT_EXT))
	{
		LCOREFATAL("%s", pMessage);
		exit(-1);
	}
	
    else if (flags & (VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT))
	{
		LCOREWARN("%s", pMessage);
	}
	
    else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	{
		LCORETRACE("%s", pMessage);
	}
    
	return VK_FALSE;
}

internal_func b8 lzy_create_debug_messenger(VkDebugReportCallbackEXT *pDebugMessenger)
{
	VkDebugReportCallbackCreateInfoEXT createInfo = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT};
	createInfo.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT |
        VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	createInfo.pfnCallback = lzy_debug_callback;
    
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(rendererState.instance, "vkCreateDebugReportCallbackEXT");
	if (vkCreateDebugReportCallbackEXT(rendererState.instance, &createInfo, NULL, pDebugMessenger) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create vulkan debug messenger");
		return false;
	}
	return true;
}

internal_func b8 lzy_pick_physical_device(const char **ppExtensionNames, u16 uNameCount, LzySwapchainSupportDetails *pDetails)
{
    
	u32 uPhysicalDeviceCount = 0;
	if (vkEnumeratePhysicalDevices(rendererState.instance, &uPhysicalDeviceCount, NULL) != VK_SUCCESS)
	{
		return false;
	}
    
	if (uPhysicalDeviceCount == 0)
	{
		LCOREFATAL("No GPUs found that support vulkan");
		return false;
	}
    
	VkPhysicalDevice *pPhysicalDevices = lzy_alloc(uPhysicalDeviceCount * sizeof(VkPhysicalDevice), sizeof(VkPhysicalDevice), LZY_MEMORY_TAG_RENDERER_INIT);
	vkEnumeratePhysicalDevices(rendererState.instance, &uPhysicalDeviceCount, pPhysicalDevices);
    
	for (u32 i = 0; i < uPhysicalDeviceCount; i++)
	{
		if (lzy_is_device_suitable(pPhysicalDevices[i], ppExtensionNames, uNameCount, pDetails))
		{
			rendererState.physicalDevice = pPhysicalDevices[i];
			break;
		}
	}
    
	if (!rendererState.physicalDevice)
	{
		LCOREFATAL("Could not find a suitable physical device");
		return false;
	}
    
	return true;
}

internal_func b8 lzy_create_surface()
{
    
	LzyWindowSurfaceCreateInfo surfaceCreateInfo = {0};
    
	lzy_application_get_surface_create_info(&surfaceCreateInfo);
	if (lzy_application_create_surface(rendererState.instance, &surfaceCreateInfo, NULL, &rendererState.surface) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create window surface");
		return false;
	}
    
	return true;
}

internal_func b8 lzy_find_queue_families(LzyQueueFamilyIndices *pQfams)
{
	LzyQueueFamilyIndices qFams = {-1, -1, -1, -1};
    u32 uQueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(rendererState.physicalDevice, &uQueueFamilyCount, NULL);
	VkQueueFamilyProperties *pQueueFamilies = lzy_alloc(uQueueFamilyCount * sizeof(VkQueueFamilyProperties), 4, LZY_MEMORY_TAG_RENDERER_INIT);
	vkGetPhysicalDeviceQueueFamilyProperties(rendererState.physicalDevice, &uQueueFamilyCount, pQueueFamilies);
    
	b32 bSurfaceSupport;
    
    u32 uMinTransferScore = ~0u;
    
	for (u32 i = 0; i < uQueueFamilyCount; i++)
	{
        u32 uCurrentTransferScore = 0;
        
		if (pQueueFamilies[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT))
		{	
            qFams.uGraphicsIndex = i;
            uCurrentTransferScore++;
        }
        
        if(pQueueFamilies[i].queueFlags & (VK_QUEUE_COMPUTE_BIT))
        {
            qFams.uComputeIndex = i;
            uCurrentTransferScore++;
        }
        
        if(pQueueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        { 
            if(uCurrentTransferScore < uMinTransferScore)
            {
                uMinTransferScore = uCurrentTransferScore;
                qFams.uTransferIndex = i;
            }
		}
        
        if (vkGetPhysicalDeviceSurfaceSupportKHR(rendererState.physicalDevice, i, rendererState.surface, &bSurfaceSupport) != VK_SUCCESS)
		{
			LCOREFATAL("Window surface failure");
			return false;
		}
		else
		{
			if (bSurfaceSupport)
				qFams.uPresentIndex = i;
		}
	}
    
	if (!lzy_qfam_is_complete(qFams))
	{
		LCOREFATAL("Required queue families not found");
		return false;
	}
    
	*pQfams = qFams;
	return true;
}

internal_func b8 lzy_create_device(LzyQueueFamilyIndices qFams, const char **ppExtensionNames, u16 uNameCount, VkDevice *pDevice)
{
	u32 qFamilyIndices[] = {qFams.uGraphicsIndex, qFams.uPresentIndex, qFams.uTransferIndex, qFams.uComputeIndex};
	u32 pSeenIndices[countof(qFamilyIndices)] = {0};
	VkDeviceQueueCreateInfo deviceQueueCreateInfos[countof(qFamilyIndices)] = {0};
	u32 uUniqueIndicesCount = 0;
    
	float fQuePrio = 1.0f;
	for (u32 i = 0; i < countof(qFamilyIndices); i++)
	{
		b8 isUnique = true;
		for (u32 j = 0; j < uUniqueIndicesCount; j++)
		{
			if (qFamilyIndices[i] == pSeenIndices[j])
			{
				isUnique = false;
				break;
			}
		}
		if (isUnique)
		{
			deviceQueueCreateInfos[uUniqueIndicesCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			deviceQueueCreateInfos[uUniqueIndicesCount].queueFamilyIndex = qFamilyIndices[i];
			deviceQueueCreateInfos[uUniqueIndicesCount].queueCount = 1;
			deviceQueueCreateInfos[uUniqueIndicesCount].pQueuePriorities = &fQuePrio;
			pSeenIndices[uUniqueIndicesCount] = qFamilyIndices[i];
			uUniqueIndicesCount++;
		}
	}
    
	VkPhysicalDeviceFeatures physicalDeviceFeatures = {0};
	VkDeviceCreateInfo deviceCreateInfo = {0};
    
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = uUniqueIndicesCount;
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos;
	deviceCreateInfo.enabledExtensionCount = uNameCount;
	deviceCreateInfo.ppEnabledExtensionNames = ppExtensionNames;
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
    
	if (vkCreateDevice(rendererState.physicalDevice, &deviceCreateInfo, NULL, &rendererState.device) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create logical device");
		return false;
	}
	return true;
}

internal_func b8 lzy_create_swapchain(VkSwapchainKHR *pSwapchain,
                                      LzyQueueFamilyIndices qFams,
                                      LzySwapchainSupportDetails *pDetails)
{
	rendererState.queueFamilies = qFams;
    
    VkSurfaceFormatKHR chosenFormat = pDetails->pFormats[0];
    
	if (pDetails->uFormatCount == 1 && pDetails->pFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		chosenFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
		chosenFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	}
    
	for (u32 i = 0; i != pDetails->uFormatCount; i++)
	{
		if (pDetails->pFormats[i].format == VK_FORMAT_B8G8R8_SRGB &&
			pDetails->pFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			chosenFormat = pDetails->pFormats[i];
			break;
		}
	}
    
	VkPresentModeKHR chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    
	for (u32 i = 0; i != pDetails->uPresentModeCount; i++)
	{
		if (pDetails->pPresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			chosenPresentMode = pDetails->pPresentModes[i];
			break;
		}
	}
    
	VkExtent2D chosenExtent;
    
	if (pDetails->surfaceCapabilities.currentExtent.width != -1)
	{
		chosenExtent = pDetails->surfaceCapabilities.currentExtent;
	}
	else
	{
		u16 uWidth, uHeight;
		lzy_application_get_framebuffer_size(&uWidth, &uHeight);
        
		chosenExtent.width = max(pDetails->surfaceCapabilities.minImageExtent.width, min(pDetails->surfaceCapabilities.maxImageExtent.width, uWidth));
		chosenExtent.height = max(pDetails->surfaceCapabilities.minImageExtent.height, min(pDetails->surfaceCapabilities.maxImageExtent.height, uHeight));
	}
    
	rendererState.swapchainInfo.swapchainImageFormat = chosenFormat.format;
	rendererState.swapchainInfo.swapchainExtent = chosenExtent;
    
	u32 uImageCount = pDetails->surfaceCapabilities.minImageCount + 1;
	if (uImageCount - pDetails->surfaceCapabilities.maxImageCount < uImageCount)
		uImageCount = pDetails->surfaceCapabilities.maxImageCount;
    
    
    VkSwapchainCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    
    createInfo.surface = rendererState.surface;
    createInfo.minImageCount = uImageCount;
    createInfo.imageFormat = chosenFormat.format;
    createInfo.imageColorSpace = chosenFormat.colorSpace;
    createInfo.imageExtent = chosenExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    
    u32 pQueueFamilyIndices[] = {qFams.uGraphicsIndex, qFams.uPresentIndex};
    
    if (qFams.uGraphicsIndex != qFams.uPresentIndex)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = pQueueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }
    
    createInfo.preTransform = pDetails->surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = chosenPresentMode;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    if (vkCreateSwapchainKHR(rendererState.device,
                             &createInfo,
                             NULL,
                             pSwapchain) != VK_SUCCESS)
    {
        LCOREFATAL("Could not create swapchain");
        return false;
    }
    
    return true;
}

internal_func b8 lzy_create_image_view(VkImageView *pImageView, 
                                       VkImage image,
                                       VkFormat format,
                                       VkImageAspectFlags imageAspect)
{
    
	VkImageViewCreateInfo createInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.subresourceRange.aspectMask = imageAspect;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.layerCount = 1;
    
	if (vkCreateImageView(rendererState.device, &createInfo, NULL, pImageView) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create image view");
		return false;
	}
    
	return true;
}


internal_func b8 
lzy_create_swapchain_depth_buffers()
{
    
    const u32 uImageCount = rendererState.swapchainInfo.uSwapchainImageCount;
	rendererState.swapchainInfo.pDepthImages = lzy_alloc(sizeof(LzyVulkanImage) * uImageCount,
                                                         8,
                                                         LZY_MEMORY_TAG_RENDERER_STATE);
    
    for(u32 i = 0; i < uImageCount; i++)
    {
        VkExtent3D depthImageExtent = 
        {
            rendererState.swapchainInfo.swapchainExtent.width,
            rendererState.swapchainInfo.swapchainExtent.height,
            1
        };
        
        if(!lzy_create_exclusive_image(&rendererState.swapchainInfo.pDepthImages[i],
                                       VK_FORMAT_D32_SFLOAT,
                                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                       VMA_MEMORY_USAGE_GPU_ONLY,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                       depthImageExtent))
        {
            return false;
        }
    }
    
    
    rendererState.swapchainInfo.pDepthImageViews = lzy_alloc(sizeof(VkImageView) * uImageCount,
                                                             8,
                                                             LZY_MEMORY_TAG_RENDERER_STATE);
    
    
    for(u32 i = 0; i < uImageCount; i++)
    {
        if(!lzy_create_image_view(&rendererState.swapchainInfo.pDepthImageViews[i],
                                  rendererState.swapchainInfo.pDepthImages[i].image,
                                  VK_FORMAT_D32_SFLOAT,
                                  VK_IMAGE_ASPECT_DEPTH_BIT))
        {
            return false;
        }
    }
    
    
    return true;
}

internal_func b8 lzy_create_render_pass(VkRenderPass *pRenderPass)
{
    
    VkAttachmentReference colorReference;
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference depthReference;
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    
    
    VkAttachmentDescription colorAttachment = {0};
    
	colorAttachment.format = rendererState.swapchainInfo.swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    
    VkAttachmentDescription depthAttachment = {0};
    
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentDescription colorAttachments[] = {colorAttachment};
    VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment};
    
	VkSubpassDescription subpassDescription = {0};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;
    
	VkRenderPassCreateInfo createInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    
	createInfo.attachmentCount = 2;
	createInfo.pAttachments = attachments;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpassDescription;
    
	if (vkCreateRenderPass(rendererState.device, &createInfo, NULL, pRenderPass) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create render pass");
		return false;
	}
	return true;
}


internal_func b8 lzy_create_framebuffer(VkFramebuffer *pFramebuffer, VkImageView* pImageViews, u32 uImageViewCount, u32 uWidth, u32 uHeight)
{
    VkFramebufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	createInfo.renderPass = rendererState.renderPass;
	createInfo.attachmentCount = uImageViewCount;
	createInfo.pAttachments = pImageViews;
	createInfo.width = uWidth;
	createInfo.height = uHeight;
	createInfo.layers = 1;
    
	if (vkCreateFramebuffer(rendererState.device, &createInfo, NULL, pFramebuffer) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create framebuffer");
		return false;
	}
	return true;
}


b8 lzy_renderer_recreate_swapchain()
{
    vkDeviceWaitIdle(rendererState.device);
    
    for (u32 i = 0; i < rendererState.swapchainInfo.uSwapchainImageCount; i++) 
    {
        vkDestroyFramebuffer(rendererState.device,
                             rendererState.swapchainInfo.pSwapchainFramebuffers[i],
                             NULL);
    }
    
    for (u32 i = 0; i < rendererState.swapchainInfo.uSwapchainImageCount; i++) {
        
        vkDestroyImageView(rendererState.device,
                           rendererState.swapchainInfo.pSwapchainImageViews[i],
                           NULL);
        
        vkDestroyImageView(rendererState.device,
                           rendererState.swapchainInfo.pDepthImageViews[i],
                           NULL);
        
        vkDestroyImage(rendererState.device,
                       rendererState.swapchainInfo.pDepthImages[i].image,
                       NULL);
    }
    
    vkDestroySwapchainKHR(rendererState.device, 
                          rendererState.swapchainInfo.swapchain, 
                          NULL);
    
    LzySwapchainSupportDetails details = lzy_get_swapchain_support_details(rendererState.physicalDevice);
    if(!lzy_create_swapchain(&rendererState.swapchainInfo.swapchain,
                             rendererState.queueFamilies,
                             &details) != VK_SUCCESS)
    {
        LCOREERROR("Could not recreate swapchain");
        return false;
    }
    
	vkGetSwapchainImagesKHR(rendererState.device, rendererState.swapchainInfo.swapchain, &rendererState.swapchainInfo.uSwapchainImageCount, rendererState.swapchainInfo.pSwapchainImages);
    
    
	rendererState.swapchainInfo.pSwapchainFramebuffers = lzy_alloc(sizeof(VkFramebuffer) * rendererState.swapchainInfo.uSwapchainImageCount, 
                                                                   8,
                                                                   LZY_MEMORY_TAG_RENDERER_STATE);
	rendererState.swapchainInfo.pSwapchainImageViews = lzy_alloc(sizeof(VkImageView) * rendererState.swapchainInfo.uSwapchainImageCount, 
                                                                 8,
                                                                 LZY_MEMORY_TAG_RENDERER_STATE);
	
    u16 uWidth;
    u16 uHeight;
    
    lzy_application_get_framebuffer_size(&uWidth, &uHeight);
    
    lzy_create_swapchain_depth_buffers();
    
    for (u32 i = 0; i < rendererState.swapchainInfo.uSwapchainImageCount; i++)
	{
		if(!lzy_create_image_view (&rendererState.swapchainInfo.pSwapchainImageViews[i],
                                   rendererState.swapchainInfo.pSwapchainImages[i],
                                   rendererState.swapchainInfo.swapchainImageFormat,
                                   VK_IMAGE_ASPECT_COLOR_BIT) != VK_SUCCESS)
        {
            LCOREERROR("Could not create image view");
            return false;
        }
        VkImageView fbAttachments[] = 
        {
            rendererState.swapchainInfo.pSwapchainImageViews[i],
            rendererState.swapchainInfo.pDepthImageViews[i],
        };
		if(!lzy_create_framebuffer(&rendererState.swapchainInfo.pSwapchainFramebuffers[i],
                                   fbAttachments,
                                   2,
                                   uWidth,
                                   uHeight) != VK_SUCCESS)
        {
            LCOREERROR("Could not create framebuffers");
            return false;
        }
	}  
    
    return true;
}

b8 lzy_renderer_init()
{
	LCOREASSERT(!bRendererInitialized, "Renderer Subsystem already initialized");
    
	u16 uWidth, uHeight;
	lzy_application_get_framebuffer_size(&uWidth, &uHeight);
    
	const char *ppExtensionNames[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    
	if (!lzy_check_vulkan_version())
		return false;
    
	if (!lzy_create_instance())
		return false;
    
#ifdef _DEBUG
	if (!lzy_create_debug_messenger(&rendererState.debugMessenger))
		return false;
#endif
    
	if (!lzy_create_surface())
		return false;
    
	LzySwapchainSupportDetails details;
    
	if (!lzy_pick_physical_device(ppExtensionNames, countof(ppExtensionNames), &details))
		return false;
    
	LzyQueueFamilyIndices qFams;
    
	if (!lzy_find_queue_families(&qFams))
		return false;
    
	if (!lzy_create_device(qFams, ppExtensionNames, countof(ppExtensionNames), &rendererState.device))
		return false;
    
    
	vkGetDeviceQueue(rendererState.device, qFams.uGraphicsIndex, 0, &rendererState.graphicsQueue);
	vkGetDeviceQueue(rendererState.device, qFams.uPresentIndex, 0, &rendererState.presentQueue);
	vkGetDeviceQueue(rendererState.device, qFams.uTransferIndex, 0, &rendererState.transferQueue);
	vkGetDeviceQueue(rendererState.device, qFams.uComputeIndex, 0, &rendererState.computeQueue);
    
    
    
    if(!lzy_create_vma_allocator(&rendererState.vmaAllocator))
    {
        return false;
    }
	if (!lzy_create_swapchain(&rendererState.swapchainInfo.swapchain,
                              qFams,
                              &details))
		return false;
    
	vkGetSwapchainImagesKHR(rendererState.device,
                            rendererState.swapchainInfo.swapchain,
                            &rendererState.swapchainInfo.uSwapchainImageCount,
                            NULL);
	
    rendererState.swapchainInfo.pSwapchainImages = lzy_alloc(sizeof(VkImage) * rendererState.swapchainInfo.uSwapchainImageCount,
                                                             8,
                                                             LZY_MEMORY_TAG_RENDERER_STATE);
	
    vkGetSwapchainImagesKHR(rendererState.device,
                            rendererState.swapchainInfo.swapchain,
                            &rendererState.swapchainInfo.uSwapchainImageCount, 
                            rendererState.swapchainInfo.pSwapchainImages);
    
    rendererState.swapchainInfo.pImageAcquireSemaphores = lzy_alloc(sizeof(VkSemaphore) * rendererState.swapchainInfo.uSwapchainImageCount,
                                                                    8,
                                                                    LZY_MEMORY_TAG_RENDERER_STATE);
    
    rendererState.swapchainInfo.pRenderFinishedSemaphores = lzy_alloc(sizeof(VkSemaphore) * rendererState.swapchainInfo.uSwapchainImageCount,
                                                                      8,
                                                                      LZY_MEMORY_TAG_RENDERER_STATE);
    
    for(u32 i = 0; i < rendererState.swapchainInfo.uSwapchainImageCount; i++)
    {
        if (!lzy_create_semaphore(&rendererState.swapchainInfo.pImageAcquireSemaphores[i]))
        {
            return false;
        }
        
        if(!lzy_create_semaphore(&rendererState.swapchainInfo.pRenderFinishedSemaphores[i]))
        {
            return false;        
        }        
    }
    
    rendererState.swapchainInfo.pInFlightFences = lzy_alloc(sizeof(VkFence) * rendererState.swapchainInfo.uSwapchainImageCount,
                                                            8,
                                                            LZY_MEMORY_TAG_RENDERER_STATE);
    
    rendererState.swapchainInfo.pImagesInFlight = lzy_alloc(sizeof(VkFence) * rendererState.swapchainInfo.uSwapchainImageCount,
                                                            8,
                                                            LZY_MEMORY_TAG_RENDERER_STATE);
    
    
    for(u32 i = 0; i < rendererState.swapchainInfo.uSwapchainImageCount; i++)
    {
        if(!lzy_create_fence(&rendererState.swapchainInfo.pInFlightFences[i],
                             VK_FENCE_CREATE_SIGNALED_BIT))
        {
            return false;       
        }       
    }
    
    
    for(u32 i = 0; i < rendererState.swapchainInfo.uSwapchainImageCount; i++)
    {
        if(!lzy_create_fence(&rendererState.swapchainInfo.pImagesInFlight[i],
                             0))
        {
            return false;       
        }       
    }
    
    
    
	rendererState.swapchainInfo.pSwapchainFramebuffers = lzy_alloc(sizeof(VkFramebuffer) * rendererState.swapchainInfo.uSwapchainImageCount,
                                                                   8,
                                                                   LZY_MEMORY_TAG_RENDERER_STATE);
	
    rendererState.swapchainInfo.pSwapchainImageViews = lzy_alloc(sizeof(VkImageView) * rendererState.swapchainInfo.uSwapchainImageCount,
                                                                 8,
                                                                 LZY_MEMORY_TAG_RENDERER_STATE);
    
    if(!lzy_create_swapchain_depth_buffers())
    {
        return false;
    }
    
	if (!lzy_create_render_pass(&rendererState.renderPass))
		return false;
    
    
    for (u32 i = 0; i < rendererState.swapchainInfo.uSwapchainImageCount; i++)
	{
		
        if(!lzy_create_image_view (&rendererState.swapchainInfo.pSwapchainImageViews[i],
                                   rendererState.swapchainInfo.pSwapchainImages[i],
                                   rendererState.swapchainInfo.swapchainImageFormat,
                                   VK_IMAGE_ASPECT_COLOR_BIT))
        {
            LCOREFATAL("Could not create swapchain image views");
            return false;
        }
        
        VkImageView fbAttachments[] = 
        {
            rendererState.swapchainInfo.pSwapchainImageViews[i],
            rendererState.swapchainInfo.pDepthImageViews[i]
        };
		
        if(!lzy_create_framebuffer(&rendererState.swapchainInfo.pSwapchainFramebuffers[i],
                                   fbAttachments,
                                   2,
                                   uWidth,
                                   uHeight) != VK_SUCCESS)
        {
            LCOREFATAL("Could not create framebuffers");
            return false;
        }
	}
    
    
	if (!lzy_create_command_pool(&rendererState.swapchainInfo.commandPool, qFams.uGraphicsIndex))
		return false;
    
	if (!lzy_create_command_pool(&rendererState.transferPool, qFams.uTransferIndex))
		return false;
    
	if (!lzy_create_command_pool(&rendererState.computePool, qFams.uComputeIndex))
		return false;
    
    
    LCOREINFO("Swapchain Image Count: %u", rendererState.swapchainInfo.uSwapchainImageCount);
    
	if (!lzy_create_shader_module(&rendererState.triangleVertexShader, "shaders/triangle.vert.spv") ||
		!lzy_create_shader_module(&rendererState.triangleFragmentShader, "shaders/triangle.frag.spv"))
	{
		return false;
	}
    
    VkDescriptorSetLayoutBinding vertexBinding = lzy_create_descriptor_set_layout_binding(0, 
                                                                                          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                                                          1,
                                                                                          VK_SHADER_STAGE_VERTEX_BIT);
    
    if(!lzy_create_descriptor_set_layout(&rendererState.vertexBufferLayout,
                                         0,
                                         &vertexBinding,
                                         1))
    {
        return false;
    }
    
    
	if (!lzy_create_graphics_pipeline(&rendererState.trianglePipeline, rendererState.triangleVertexShader, rendererState.triangleFragmentShader))
	{
		return false;
	}
    
    VkDescriptorPoolSize size = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10};
    
    if(!lzy_create_descriptor_pool(&rendererState.descriptorPool,
                                   0,
                                   10,
                                   &size,
                                   1) != VK_SUCCESS)
    {
        return false;
    }
    
    
    if(!lzy_create_descriptor_set(&rendererState.vertexBufferDescriptorSet,
                                  1,
                                  rendererState.descriptorPool,
                                  &rendererState.vertexBufferLayout) != VK_SUCCESS)
    {
        return false;
    }
    
    
    
	LzyObjContents contents;
	LzyFile objFile;
	if (!lzy_file_open(&objFile, "kitten.obj", LZY_FILE_MODE_READ))
	{
		LCOREFATAL("Could not open file kitten.obj");
		return false;
	}
    
	lzy_obj_load_file(objFile, &contents);
    
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(rendererState.physicalDevice, &memProps);
    
    LzyVulkanBuffer vertexStagingBuffer;
    LzyVulkanBuffer indexStagingBuffer;
    
    
    
    
    if(!lzy_create_buffer(&vertexStagingBuffer,
                          &memProps,
                          lzy_vector_size(contents.pVertices) * sizeof(*contents.pVertices),
                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VMA_MEMORY_USAGE_CPU_TO_GPU) ||
       
       !lzy_create_buffer(&indexStagingBuffer,
                          &memProps,
                          lzy_vector_size(contents.pIndices) * sizeof(*contents.pIndices),
                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VMA_MEMORY_USAGE_CPU_TO_GPU) )
    {
        return false;
    }
    
    lzy_memcpy(vertexStagingBuffer.pData, contents.pVertices, lzy_vector_size(contents.pVertices) * sizeof(*contents.pVertices));
    lzy_memcpy(indexStagingBuffer.pData, contents.pIndices, lzy_vector_size(contents.pIndices) * sizeof(*contents.pIndices));
	
    
	if (!lzy_create_buffer(&rendererState.vertexBuffer,
                           &memProps, 
                           lzy_vector_size(contents.pVertices) * sizeof(*contents.pVertices),
                           VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                           VMA_MEMORY_USAGE_GPU_ONLY) ||
		
        !lzy_create_buffer(&rendererState.indexBuffer, 
                           &memProps, 
                           lzy_vector_size(contents.pIndices) * sizeof(*contents.pIndices), 
                           VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                           VMA_MEMORY_USAGE_GPU_ONLY))
	{
		return false;
	}
    
    lzy_obj_free(&contents);
	
    
    VkCommandBuffer stagingCommandBuffers[2];
    
	VkCommandBufferAllocateInfo staginBufferAllocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	staginBufferAllocInfo.commandPool = rendererState.transferPool;
	staginBufferAllocInfo.commandBufferCount = 2;
	staginBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
	if (vkAllocateCommandBuffers(rendererState.device,
                                 &staginBufferAllocInfo,
                                 stagingCommandBuffers) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create command buffer");
		return false;
	} 
    
    VkCommandBufferBeginInfo transferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    
    transferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;    
    
    if(vkBeginCommandBuffer(stagingCommandBuffers[0],
                            &transferBeginInfo) != VK_SUCCESS)
    {
        LCOREFATAL("Could not begin command buffer");
        return false;
    }
    
    VkBufferCopy bufferCopy;
    bufferCopy.srcOffset = 0;
    bufferCopy.dstOffset = 0;
    bufferCopy.size = vertexStagingBuffer.uSize;
    
    vkCmdCopyBuffer(stagingCommandBuffers[0],
                    vertexStagingBuffer.buffer,
                    rendererState.vertexBuffer.buffer,
                    1,
                    &bufferCopy);
    
    
    vkEndCommandBuffer(stagingCommandBuffers[0]);
	
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &stagingCommandBuffers[0];
    
    vkQueueSubmit(rendererState.transferQueue,
                  1,
                  &submitInfo,
                  VK_NULL_HANDLE);
    
    
    if(vkBeginCommandBuffer(stagingCommandBuffers[1],
                            &transferBeginInfo) != VK_SUCCESS)
    {
        LCOREFATAL("Could not begin command buffer");
        return false;
    }
    
    bufferCopy.srcOffset = 0;
    bufferCopy.dstOffset = 0;
    bufferCopy.size = indexStagingBuffer.uSize;
    
    vkCmdCopyBuffer(stagingCommandBuffers[1],
                    indexStagingBuffer.buffer,
                    rendererState.indexBuffer.buffer,
                    1,
                    &bufferCopy);
    
    
    vkEndCommandBuffer(stagingCommandBuffers[1]);
    
    
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &stagingCommandBuffers[1];
    
    vkQueueSubmit(rendererState.transferQueue,
                  1,
                  &submitInfo,
                  VK_NULL_HANDLE);
    
    
    VkDescriptorBufferInfo bufferInfo = {0};
    bufferInfo.buffer = rendererState.vertexBuffer.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = rendererState.vertexBuffer.uSize;
    
    VkWriteDescriptorSet write = {0};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = rendererState.vertexBufferDescriptorSet;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write.pBufferInfo = &bufferInfo;
    
    vkUpdateDescriptorSets(rendererState.device,
                           1,
                           &write,
                           0,
                           NULL);
    
    
    rendererState.swapchainInfo.pCommandBuffers = lzy_alloc(sizeof(VkCommandBuffer) * rendererState.swapchainInfo.uSwapchainImageCount,
                                                            8,
                                                            LZY_MEMORY_TAG_RENDERER_STATE);
    
	VkCommandBufferAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	allocInfo.commandPool = rendererState.swapchainInfo.commandPool;
	allocInfo.commandBufferCount = rendererState.swapchainInfo.uSwapchainImageCount;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
	if (vkAllocateCommandBuffers(rendererState.device,
                                 &allocInfo,
                                 rendererState.swapchainInfo.pCommandBuffers) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create command buffer");
		return false;
	} 
    
    vkQueueWaitIdle(rendererState.transferQueue);
    
    vkFreeCommandBuffers(rendererState.device,
                         rendererState.transferPool,
                         2,
                         stagingCommandBuffers);
    lzy_destroy_buffer(&vertexStagingBuffer);
    lzy_destroy_buffer(&indexStagingBuffer);
    
	bRendererInitialized = true;
	LCOREINFO("Renderer subsystem initialized");
    return true;
    
}

b8 lzy_renderer_loop(f64 fDeltaTime)
{    
    vkWaitForFences(rendererState.device,
                    1,
                    &rendererState.swapchainInfo.pInFlightFences[rendererState.uFrameIndex],
                    VK_TRUE,
                    ~0ull);

    u32 uImageIndex;
    VkResult result = vkAcquireNextImageKHR(rendererState.device,
                                            rendererState.swapchainInfo.swapchain,
                                            ~0ull,
                                            rendererState.swapchainInfo.pImageAcquireSemaphores[rendererState.uFrameIndex],
                                            VK_NULL_HANDLE,
                                            &uImageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        if(!lzy_renderer_recreate_swapchain())
        {
            return false;
        }
        LCOREINFO("Recreated Swapchain");
        return true;    
    } else if(result != VK_SUCCESS)
    {
        LCOREFATAL("Could not acquire Image Index");
        return false;
    } 
    
    
    if(rendererState.swapchainInfo.pImagesInFlight[uImageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(rendererState.device,
                        1,
                        &rendererState.swapchainInfo.pInFlightFences[uImageIndex],
                        VK_TRUE,
                        ~0ull);
    }
    
    rendererState.swapchainInfo.pImagesInFlight[uImageIndex] = rendererState.swapchainInfo.pInFlightFences[rendererState.uFrameIndex];
    
    vkResetFences(rendererState.device,
                  1,
                  &rendererState.swapchainInfo.pInFlightFences[rendererState.uFrameIndex]);
	
	VkClearValue colorClear = {.color = {48.0f / 255.0f, 10.0f / 255.0f, 36.0f / 255.0f, 1.0f}};
    VkClearValue depthClear = {.depthStencil = {.depth = 1.0f}};
    
    VkClearValue clearValues[] = {colorClear, depthClear};
    
	VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VkImageSubresourceRange range = {0};
    
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.levelCount = 1;
	range.layerCount = 1;
    
	vkBeginCommandBuffer(rendererState.swapchainInfo.pCommandBuffers[uImageIndex], &beginInfo);
    
    VkImageMemoryBarrier rendererBeginBarrier = 
        lzy_create_image_memory_barrier(rendererState.swapchainInfo.pSwapchainImages[uImageIndex],
                                        0,
                                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 
                                        VK_IMAGE_LAYOUT_UNDEFINED,
                                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	
    vkCmdPipelineBarrier(rendererState.swapchainInfo.pCommandBuffers[uImageIndex],
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT,
                         0, NULL, 0, NULL, 1, &rendererBeginBarrier);
    
    
	VkRenderPassBeginInfo renderPassBeginInfo = 
    {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = rendererState.renderPass,
		.framebuffer = rendererState.swapchainInfo.pSwapchainFramebuffers[uImageIndex],
		.renderArea.extent = rendererState.swapchainInfo.swapchainExtent,
		.clearValueCount = 2,
		.pClearValues = clearValues
    };
    
	u16 uWindowWidth, uWindowHeight;
	lzy_application_get_framebuffer_size(&uWindowWidth, &uWindowHeight);
	VkViewport viewport = {
		.width =  (f32)uWindowWidth,
		.height = (f32)uWindowHeight,
		.minDepth = 0,
		.maxDepth = 1};
    
	VkRect2D scissor = {.offset = {0, 0}, .extent = {uWindowWidth, uWindowHeight}};
    
	vkCmdBeginRenderPass(rendererState.swapchainInfo.pCommandBuffers[uImageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    
	vkCmdSetViewport(rendererState.swapchainInfo.pCommandBuffers[uImageIndex], 0, 1, &viewport);
	vkCmdSetScissor(rendererState.swapchainInfo.pCommandBuffers[uImageIndex], 0, 1, &scissor);
    
	vkCmdBindPipeline(rendererState.swapchainInfo.pCommandBuffers[uImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, rendererState.trianglePipeline);
	VkDeviceSize uOffset = 0;
    
	vkCmdBindDescriptorSets(rendererState.swapchainInfo.pCommandBuffers[uImageIndex],
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            rendererState.trianglePipelineLayout,
                            0,
                            1,
                            &rendererState.vertexBufferDescriptorSet,
                            0,
                            NULL);
    
    static LzyTestPushConstants pc = {0,0};
    
    static f64 fTime = 0;
    static f64 fTime2 = 3.14159;
    
    fTime += fDeltaTime;
    fTime2 += fDeltaTime;
    LCOREINFO("%fs",fTime);
    
    pc.x = fTime;
    pc.y = fTime;
	
    vkCmdPushConstants(rendererState.swapchainInfo.pCommandBuffers[uImageIndex],
                       rendererState.trianglePipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT,
                       0,
                       sizeof(LzyTestPushConstants),
                       &pc);
    
    
    vkCmdBindIndexBuffer(rendererState.swapchainInfo.pCommandBuffers[uImageIndex], rendererState.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	
    vkCmdDrawIndexed(rendererState.swapchainInfo.pCommandBuffers[uImageIndex], rendererState.indexBuffer.uSize / sizeof(u32), 1, 0, 0, 0);
	
    
    pc.x = fTime2;
    pc.y = fTime2;	
    
    vkCmdPushConstants(rendererState.swapchainInfo.pCommandBuffers[uImageIndex],
                       rendererState.trianglePipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT,
                       0,
                       sizeof(LzyTestPushConstants),
                       &pc);
    
    
    vkCmdDrawIndexed(rendererState.swapchainInfo.pCommandBuffers[uImageIndex], rendererState.indexBuffer.uSize / sizeof(u32), 1, 0, 0, 0);
	
    
    vkCmdEndRenderPass(rendererState.swapchainInfo.pCommandBuffers[uImageIndex]);
    
	VkImageMemoryBarrier renderEndBarrier = lzy_create_image_memory_barrier(rendererState.swapchainInfo.pSwapchainImages[uImageIndex], VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	vkCmdPipelineBarrier(
                         rendererState.swapchainInfo.pCommandBuffers[uImageIndex],
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT,
                         0, NULL, 0, NULL, 1, &renderEndBarrier);
    
	if (vkEndCommandBuffer(rendererState.swapchainInfo.pCommandBuffers[uImageIndex]) != VK_SUCCESS)
	{
		LCOREFATAL("Could not end command buffer");
		return false;
	}
    
	VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    
	VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores =  &rendererState.swapchainInfo.pImageAcquireSemaphores[rendererState.uFrameIndex];
	submitInfo.pWaitDstStageMask = &stageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &rendererState.swapchainInfo.pCommandBuffers[uImageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &rendererState.swapchainInfo.pRenderFinishedSemaphores[rendererState.uFrameIndex];
    
    vkQueueSubmit(rendererState.graphicsQueue,
                  1,
                  &submitInfo,
                  rendererState.swapchainInfo.pInFlightFences[rendererState.uFrameIndex]);
    
    VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &rendererState.swapchainInfo.pRenderFinishedSemaphores[rendererState.uFrameIndex];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &rendererState.swapchainInfo.swapchain;
	presentInfo.pImageIndices = &uImageIndex;
    
	vkQueuePresentKHR(rendererState.presentQueue, &presentInfo);
    
	rendererState.uFrameIndex = (rendererState.uFrameIndex + 1) % rendererState.swapchainInfo.uSwapchainImageCount;
    return true;
}
