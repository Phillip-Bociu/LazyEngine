#include "LzyLog.h"
#include "LzyRenderer.h"
#include "LzyMemory.h"
#include <vulkan/vulkan.h>
#include <string.h>

typedef struct LzyRendererState
{
	VkInstance instance;
	VkPhysicalDevice phyisicalDevice;
} LzyRendererState;

global b8 bIsInitialized = false;
global LzyRendererState rendererState;

b8 lzy_renderer_init()
{
	LCOREASSERT(!bIsInitialized, "Renderer Subsystem already initialized");
	u32 uSupportedInstanceVersion = 0;
	vkEnumerateInstanceVersion(&uSupportedInstanceVersion);

	if (uSupportedInstanceVersion < VK_MAKE_VERSION(1, 2, 0))
	{
		u16 major, minor, patch;
		major = VK_VERSION_MAJOR(uSupportedInstanceVersion);
		minor = VK_VERSION_MINOR(uSupportedInstanceVersion);
		patch = VK_VERSION_PATCH(uSupportedInstanceVersion);

		LCOREFATAL("Currently installed vulkan version (%u.%u.%u) is older than the minimun required (1.2.0)", major, minor, patch);
		return false;
	}

	VkApplicationInfo appInfo = {0};

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = max(uSupportedInstanceVersion, VK_MAKE_VERSION(1, 2, 0));

#ifdef _DEBUG
	const char *ppValidationLayers[] = {"VK_LAYER_KHRONOS_validation"};
#else
	const char **ppValidationLayers = NULL;
#endif
	u32 uLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&uLayerCount, NULL);
	VkLayerProperties *pLayerProperties = lzy_alloc(uLayerCount * sizeof(VkLayerProperties), 8, LZY_MEMORY_TAG_RENDERER_INIT);
	vkEnumerateInstanceLayerProperties(&uLayerCount, pLayerProperties);

	u32 uFoundLayers = 0;

	for (u32 i = 0; i < uLayerCount && uFoundLayers < sizeof(ppValidationLayers) / sizeof(const char*); i++)
	{
		for(u32 j = 0; j < sizeof(ppValidationLayers) / sizeof(const char*); j++)
			if(strcmp(pLayerProperties[i].layerName, ppValidationLayers[j]) == 0)
			{
				uFoundLayers++;
				break;
			}
	}

	if(uFoundLayers != sizeof(ppValidationLayers) / sizeof(const char*))
	{
		LCOREFATAL("Not all required validation layers supported");
		return false;
	}

	VkInstanceCreateInfo instanceCreateInfo = {0};

	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = 0;
	instanceCreateInfo.ppEnabledExtensionNames = NULL;
	instanceCreateInfo.enabledLayerCount = sizeof(ppValidationLayers) / sizeof(const char *);
	instanceCreateInfo.ppEnabledLayerNames = ppValidationLayers;

	if (vkCreateInstance(&instanceCreateInfo, NULL, &rendererState.instance) != VK_SUCCESS)
	{
		LCOREFATAL("Could not create vulkan instance");
		return false;
	}

	u32 uPhysicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(rendererState.instance, &uPhysicalDeviceCount, NULL);

	if(uPhysicalDeviceCount == 0)
	{
		LCOREFATAL("No GPUs found that support vulkan");
		return false;
	}

	u32 *pPhysicalDevices = lzy_alloc(uPhysicalDeviceCount * sizeof(u32), sizeof(u32), LZY_MEMORY_TAG_RENDERER_INIT);
	vkEnumeratePhysicalDevices(rendererState.instance, &uPhysicalDeviceCount, pPhysicalDevices);

	bIsInitialized = true;
	LCOREINFO("Renderer subsystem initialized");
	return true;
}
