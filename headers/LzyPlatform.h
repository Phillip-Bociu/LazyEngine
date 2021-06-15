#pragma once
#include "LzyDefines.h"
#include <vulkan/vulkan.h>
#ifdef __linux__
#include <xcb/xcb.h>
#include <vulkan/vulkan_xcb.h>
typedef VkXcbSurfaceCreateInfoKHR LzyWindowSurfaceCreateInfo;
#define LZY_SURFACE_EXT_NAME "VK_KHR_xcb_surface"
#elif _WIN32
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
typedef VkWin32SurfaceCreateInfoKHR LzyWindowSurfaceCreateInfo;
#define LZY_SURFACE_EXT_NAME "VK_KHR_win32_surface"
#endif


MAKE_PLATFORM_INDEPENDENT_HANDLE(LzyPlatform);

b8 lzy_platform_create(LzyPlatform* pPlatform, const char* pWindowTitle,u16 uResX, u16 uResY);
b8 lzy_platform_poll_events(LzyPlatform platform);
void lzy_platform_get_surface_create_info(LzyPlatform platform, LzyWindowSurfaceCreateInfo* pSurface);
VkResult lzy_platform_create_surface(VkInstance instance, const LzyWindowSurfaceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocs, VkSurfaceKHR* pSurface);
void lzy_platform_get_framebuffer_size(LzyPlatform platform, u16* pX, u16* pY);
void lzy_platform_shutdown(LzyPlatform platform);
void* lzy_platform_alloc(u64 uSize, u8 uAlignment);
void lzy_platform_free(void* ptr, u64 uSize, u8 uAlignment);
void* lzy_platform_memcpy(void* pDst, void* pSrc, u64 uSize);
void* lzy_platform_memset(void* pDst, u8 uVal, u64 uSize);
void* lzy_platform_memzero(void* pDst, u64 uSize);
void lzy_platform_sleep(u64 uMs);
f64 lzy_platform_get_time();
