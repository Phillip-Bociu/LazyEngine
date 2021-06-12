
#include "LzyLog.h"
#include "LzyRenderer.h"
#include "LzyMemory.h"
#include <vulkan/vulkan.h>
#include <string.h>

typedef struct LzyRendererState
{
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSurfaceKHR surface;
} LzyRendererState;

global b8 bIsInitialized = false;
global LzyRendererState rendererState;

typedef struct LzyQueueFamilyIndices
{
	u8 uGraphicsIndex;
	u8 uPresentIndex;
}LzyQueueFamilyIndices;

internal_func b8 lzy_qfam_is_complete(LzyQueueFamilyIndices qFams)
{
	if (qFams.uGraphicsIndex == -1 || qFams.uPresentIndex == -1)
		return false;
	else
		return true;
}


//TODO improve this
internal_func b8 is_device_suitable(VkPhysicalDevice physicalDevice)
{

	VkPhysicalDeviceProperties props;
	//VkPhysicalDeviceFeatures feats;
	vkGetPhysicalDeviceProperties(physicalDevice, &props);
	if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
	{
		//vkGetPhysicalDeviceFeatures(physicalDevice, &feats);
		return true;
	}

	return false;

}

internal_func b8 check_vulkan_version()
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

internal_func VkInstance create_instance()
{
	u32 uSupportedInstanceVersion;
	vkEnumerateInstanceVersion(&uSupportedInstanceVersion);

	VkApplicationInfo appInfo = { 0 };

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = max(uSupportedInstanceVersion, VK_API_VERSION_1_2);

	u32 uFoundLayers = 0;
#ifndef _DEBUG
	const char** ppValidationLayers = { 0 };
#else
	const char* ppValidationLayers[] = { "VK_LAYER_KHRONOS_validation" };
	u32 uLayerCount = 0;

	vkEnumerateInstanceLayerProperties(&uLayerCount, NULL);
	VkLayerProperties* pLayerProperties = lzy_alloc(uLayerCount * sizeof(VkLayerProperties), 8, LZY_MEMORY_TAG_RENDERER_INIT);
	vkEnumerateInstanceLayerProperties(&uLayerCount, pLayerProperties);

	for (u32 i = 0; i < uLayerCount && uFoundLayers < sizeof(ppValidationLayers) / sizeof(const char*); i++)
	{
		for (u32 j = 0; j < sizeof(ppValidationLayers) / sizeof(const char*); j++)
			if (strcmp(pLayerProperties[i].layerName, ppValidationLayers[j]) == 0)
			{
				uFoundLayers++;
				break;
			}
	}

	if (uFoundLayers != sizeof(ppValidationLayers) / sizeof(const char*))
	{
		LCOREFATAL("Not all required validation layers supported");
		return false;
	}
#endif
	u32 uFoundExtensions = 0;
	u32 uExtensionCount = 0;

	const char* ppExtensionNames[] = { LZY_SURFACE_EXT_NAME, "VK_KHR_surface" };

	vkEnumerateInstanceExtensionProperties(NULL, &uExtensionCount, NULL);
	VkExtensionProperties* pExtensions = lzy_alloc(sizeof(VkExtensionProperties) * uExtensionCount, 4, LZY_MEMORY_TAG_RENDERER_INIT);
	vkEnumerateInstanceExtensionProperties(NULL, &uExtensionCount, pExtensions);

	for (u32 i = 0; i < uExtensionCount && uFoundExtensions < sizeof(ppExtensionNames) / sizeof(const char*); i++)
	{
		for (u32 j = 0; j < sizeof(ppExtensionNames) / sizeof(const char*); j++)
			if (strcmp(pExtensions[i].extensionName, ppExtensionNames[j]) == 0)
			{
				uFoundExtensions++;
				break;
			}
	}

	if (uFoundExtensions != sizeof(ppExtensionNames) / sizeof(const char*))
	{
		LCOREFATAL("Not all required vulkan extensions supported");
		return false;
	}


	VkInstanceCreateInfo instanceCreateInfo = { 0 };

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

internal_func b8 pick_physical_device()
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
	VkPhysicalDevice* pPhysicalDevices = lzy_alloc(uPhysicalDeviceCount * sizeof(VkPhysicalDevice), sizeof(VkPhysicalDevice), LZY_MEMORY_TAG_RENDERER_INIT);
	vkEnumeratePhysicalDevices(rendererState.instance, &uPhysicalDeviceCount, pPhysicalDevices);

	for (u32 i = 0; i < uPhysicalDeviceCount; i++)
	{
		if (is_device_suitable(pPhysicalDevices[i]))
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

internal_func b8 create_surface(LzyPlatform platform)
{

	LzyWindowSurfaceCreateInfo surfaceCreateInfo = { 0 };

	lzy_platform_get_surface_create_info(platform, &surfaceCreateInfo);
	if (lzy_platform_create_surface(rendererState.instance, &surfaceCreateInfo, NULL, &rendererState.surface) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create window surface");
		return false;
	}

	return true;
}

internal_func b8 find_queue_families(LzyQueueFamilyIndices* pQfams)
{
	LzyQueueFamilyIndices qFams;
	qFams.uGraphicsIndex = -1;
	qFams.uPresentIndex = -1;
	u32 uQueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(rendererState.physicalDevice, &uQueueFamilyCount, NULL);
	VkQueueFamilyProperties* pQueueFamilies = lzy_alloc(uQueueFamilyCount * sizeof(VkQueueFamilyProperties), 4, LZY_MEMORY_TAG_RENDERER_INIT);
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


internal_func b8 create_device(LzyQueueFamilyIndices qFams, VkDevice* pDevice)
{
	u32 qFamilyIndices[] = { qFams.uGraphicsIndex, qFams.uPresentIndex };
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

	VkPhysicalDeviceFeatures physicalDeviceFeatures = { 0 };
	VkDeviceCreateInfo deviceCreateInfo = { 0 };

	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = uUniqueIndicesCount;
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos;
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

	if (vkCreateDevice(rendererState.physicalDevice, &deviceCreateInfo, NULL, &rendererState.device) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create logical device");
		return false;
	}
	return true;
}

b8 lzy_renderer_init(LzyPlatform platform)
{
	LCOREASSERT(!bIsInitialized, "Renderer Subsystem already initialized");

	if (!check_vulkan_version())
		return false;

	if (!create_instance())
		return false;

	if (!pick_physical_device())
		return false;

	if (!create_surface(platform))
		return false;

	LzyQueueFamilyIndices qFams;

	if (!find_queue_families(&qFams))
		return false;

	if (!create_device(qFams, &rendererState.device))
		return false;
	
	vkGetDeviceQueue(rendererState.device, qFams.uGraphicsIndex, 0, &rendererState.graphicsQueue);
	vkGetDeviceQueue(rendererState.device, qFams.uPresentIndex, 0, &rendererState.presentQueue);





	bIsInitialized = true;
	LCOREINFO("Renderer subsystem initialized");
	return true;
}
