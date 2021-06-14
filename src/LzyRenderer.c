
#include "LzyLog.h"
#include "LzyRenderer.h"
#include "LzyApplication.h"
#include "LzyMemory.h"
#include <vulkan/vulkan.h>
#include <string.h>
#include <stdio.h>

typedef struct LzyQueueFamilyIndices
{
	u8 uGraphicsIndex;
	u8 uPresentIndex;
} LzyQueueFamilyIndices;

typedef struct LzySwapchainSupportDetails
{
	VkSurfaceFormatKHR *pFormats;
	VkPresentModeKHR *pPresentModes;
	u32 uFormatCount;
	u32 uPresentModeCount;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
} LzySwapchainSupportDetails;

typedef struct LzyRendererState
{
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSurfaceKHR surface;
	VkExtent2D swapchainExtent;
	VkFormat swapchainImageFormat;
	VkSwapchainKHR swapchain;
	u32 uSwapchainImageCount;
	VkImage *pSwapchainImages;
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer;
	VkSemaphore acquireSemaphore;
	VkSemaphore releaseSemaphore;
} LzyRendererState;

global b8 bIsInitialized = false;
global LzyRendererState rendererState;

internal_func b8 lzy_create_command_pool(VkCommandPool *pCommandPool, u32 uFamilyIndex)
{

	VkCommandPoolCreateInfo createInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	createInfo.queueFamilyIndex = uFamilyIndex;
	createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	if (vkCreateCommandPool(rendererState.device, &createInfo, NULL, pCommandPool) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create Command Pool");
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

internal_func LzySwapchainSupportDetails lzy_get_swapchain_support_details(VkPhysicalDevice physicalDevice)
{
	LzySwapchainSupportDetails retval = {0};

	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, rendererState.surface, &retval.uFormatCount, NULL);
	if (retval.uFormatCount != 0)
	{
		retval.pFormats = lzy_alloc(sizeof(VkSurfaceFormatKHR) * retval.uFormatCount, 8, LZY_MEMORY_TAG_RENDERER_INIT);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, rendererState.surface, &retval.uFormatCount, retval.pFormats);
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, rendererState.surface, &retval.uPresentModeCount, NULL);
	if (retval.uPresentModeCount != 0)
	{
		retval.pPresentModes = lzy_alloc(sizeof(VkPresentModeKHR) * retval.uPresentModeCount, 8, LZY_MEMORY_TAG_RENDERER_INIT);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, rendererState.surface, &retval.uPresentModeCount, retval.pPresentModes);
	}

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, rendererState.surface, &retval.surfaceCapabilities);

	return retval;
}

internal_func b8 lzy_qfam_is_complete(LzyQueueFamilyIndices qFams)
{
	if (qFams.uGraphicsIndex == -1 || qFams.uPresentIndex == -1)
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

//TODO improve this
internal_func b8 lzy_is_device_suitable(VkPhysicalDevice physicalDevice, const char **ppExtensionNames, u16 uNameCount, LzySwapchainSupportDetails *pDetails)
{

	VkPhysicalDeviceProperties props;
	//VkPhysicalDeviceFeatures feats;
	vkGetPhysicalDeviceProperties(physicalDevice, &props);
	if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
	{
		if (lzy_check_device_extension_support(physicalDevice, ppExtensionNames, uNameCount))
		{
			LzySwapchainSupportDetails details = lzy_get_swapchain_support_details(physicalDevice);
			if (details.uFormatCount != 0 && details.uPresentModeCount != 0)
			{
				*pDetails = details;
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

internal_func VkInstance lzy_create_instance()
{
	u32 uSupportedInstanceVersion;
	vkEnumerateInstanceVersion(&uSupportedInstanceVersion);

	VkApplicationInfo appInfo = {0};

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	//appInfo.apiVersion = max(uSupportedInstanceVersion, VK_API_VERSION_1_2);
	appInfo.apiVersion = VK_API_VERSION_1_1;

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

	const char *ppExtensionNames[] = {LZY_SURFACE_EXT_NAME, "VK_KHR_surface"};

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
	LzyQueueFamilyIndices qFams;
	qFams.uGraphicsIndex = -1;
	qFams.uPresentIndex = -1;
	u32 uQueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(rendererState.physicalDevice, &uQueueFamilyCount, NULL);
	VkQueueFamilyProperties *pQueueFamilies = lzy_alloc(uQueueFamilyCount * sizeof(VkQueueFamilyProperties), 4, LZY_MEMORY_TAG_RENDERER_INIT);
	vkGetPhysicalDeviceQueueFamilyProperties(rendererState.physicalDevice, &uQueueFamilyCount, pQueueFamilies);

	b32 bSurfaceSupport;

	for (u32 i = 0; i < uQueueFamilyCount; i++)
	{
		if (pQueueFamilies[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT))
			qFams.uGraphicsIndex = i;
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
	u32 qFamilyIndices[] = {qFams.uGraphicsIndex, qFams.uPresentIndex};
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

b8 lzy_renderer_init()
{
	LCOREASSERT(!bIsInitialized, "Renderer Subsystem already initialized");

	const char *ppExtensionNames[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	if (!lzy_check_vulkan_version())
		return false;

	if (!lzy_create_instance())
		return false;

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

	VkSurfaceFormatKHR chosenFormat = details.pFormats[0];

	for (u32 i = 0; i != details.uFormatCount; i++)
	{
		if (details.pFormats[i].format == VK_FORMAT_B8G8R8_SRGB &&
			details.pFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			chosenFormat = details.pFormats[i];
			break;
		}
	}

	VkPresentModeKHR chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	for (u32 i = 0; i != details.uPresentModeCount; i++)
	{
		if (details.pPresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			chosenPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
	}

	VkExtent2D chosenExtent;

	if (details.surfaceCapabilities.currentExtent.width != -1)
	{
		chosenExtent = details.surfaceCapabilities.currentExtent;
	}
	else
	{
		u16 uWidth, uHeight;
		lzy_application_get_framebuffer_size(&uWidth, &uHeight);

		chosenExtent.width = max(details.surfaceCapabilities.minImageExtent.width, min(details.surfaceCapabilities.maxImageExtent.width, uWidth));
		chosenExtent.height = max(details.surfaceCapabilities.minImageExtent.height, min(details.surfaceCapabilities.maxImageExtent.height, uHeight));
	}

	
	rendererState.swapchainImageFormat = chosenFormat.format;
	rendererState.swapchainExtent = chosenExtent;

	u32 uImageCount = details.surfaceCapabilities.minImageCount + 1;
	if (uImageCount - details.surfaceCapabilities.maxImageCount < uImageCount)
		uImageCount = details.surfaceCapabilities.maxImageCount;

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

	createInfo.preTransform = details.surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = chosenPresentMode;
	createInfo.oldSwapchain = VK_NULL_HANDLE;


	if (vkCreateSwapchainKHR(rendererState.device, &createInfo, NULL, &rendererState.swapchain) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create swapchain");
		return false;
	}

	vkGetSwapchainImagesKHR(rendererState.device, rendererState.swapchain, &rendererState.uSwapchainImageCount, NULL);
	rendererState.pSwapchainImages = lzy_alloc(sizeof(VkImage) * rendererState.uSwapchainImageCount, 8, LZY_MEMORY_TAG_RENDERER_STATE);
	vkGetSwapchainImagesKHR(rendererState.device, rendererState.swapchain, &rendererState.uSwapchainImageCount, rendererState.pSwapchainImages);

	if (!lzy_create_semaphore(&rendererState.acquireSemaphore) || !lzy_create_semaphore(&rendererState.releaseSemaphore))
	{
		return false;
	}

	if (!lzy_create_command_pool(&rendererState.commandPool, qFams.uGraphicsIndex))
		return false;

	VkCommandBufferAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	allocInfo.commandPool = rendererState.commandPool;
	allocInfo.commandBufferCount = 1;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	if (vkAllocateCommandBuffers(rendererState.device, &allocInfo, &rendererState.commandBuffer) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create command buffer");
		return false;
	}

	bIsInitialized = true;
	LCOREINFO("Renderer subsystem initialized");
	return true;
}

b8 lzy_renderer_loop()
{
	u32 uImageIndex = 0;
	
	if(vkAcquireNextImageKHR(rendererState.device, rendererState.swapchain, (u64)-1, rendererState.acquireSemaphore, VK_NULL_HANDLE, &uImageIndex) != VK_SUCCESS)
	{
		LCOREFATAL("Could not acquire image index");
		return false;
	}


	if(vkResetCommandPool(rendererState.device, rendererState.commandPool, 0) != VK_SUCCESS)
	{
		LCOREFATAL("Could not reset command pool");
		return false;
	}

	VkClearColorValue color = {0,1,0,1};

	VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VkImageSubresourceRange range = {0};

	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.levelCount = 1;
	range.layerCount = 1;

	vkBeginCommandBuffer(rendererState.commandBuffer, &beginInfo);
	vkCmdClearColorImage(rendererState.commandBuffer, rendererState.pSwapchainImages[uImageIndex], VK_IMAGE_LAYOUT_GENERAL, &color, 1, &range);
	vkEndCommandBuffer(rendererState.commandBuffer);

	VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &rendererState.acquireSemaphore;
	submitInfo.pWaitDstStageMask = &stageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &rendererState.commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &rendererState.releaseSemaphore;

	vkQueueSubmit(rendererState.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

	VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &rendererState.releaseSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &rendererState.swapchain;
	presentInfo.pImageIndices = &uImageIndex;

	vkQueuePresentKHR(rendererState.presentQueue, &presentInfo);
	vkDeviceWaitIdle(rendererState.device);
	return true;
}
