#pragma once
#include "LzyPlatform.h"
#include "LzyDefines.h"
#include "LzyMemory.h"
struct LzyGame;

typedef struct LzyApplicationConfig
{
	char* pApplicationName;
	LzyMemoryConfig memoryConfig;
	u16 uResX;
	u16 uResY;
}LzyApplicationConfig;

LAPI b8 lzy_application_create(struct LzyGame* pGame);
LAPI b8 lzy_application_run();
LAPI void lzy_application_get_framebuffer_size(u16* pX, u16* pY);

void lzy_application_get_surface_create_info(LzyWindowSurfaceCreateInfo* pSurface);
VkResult lzy_application_create_surface(VkInstance instance, const LzyWindowSurfaceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocs, VkSurfaceKHR* pSurface);
void lzy_application_set_framebuffer_size(u16 uX, u16 uY);

