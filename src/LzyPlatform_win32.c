#include "LzyDefines.h"
#include "LzyLog.h"
#include "LzyPlatform.h"
#include "LzyMemory.h"
#ifdef _WIN32
#include <Windows.h>
#include <stdlib.h>
#include <string.h>

typedef struct LzyPlatform_impl
{
	HWND hWindow;
}LzyPlatform_impl;

global LARGE_INTEGER iPerfFreq;

internal_func LRESULT LzyWindowProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	switch (msg)
	{
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		//lResult = DefWindowProc(hWindow, msg, wParam, lParam);
	}break;
	case WM_KEYDOWN:
	{
		lzy_get_memstats();
	}break;
	default:
	{
		//lResult = DefWindowProc(hWindow, msg, wParam, lParam);

	}break;
	}
	return DefWindowProc(hWindow, msg, wParam, lParam);
}

b8 lzy_platform_create(LzyPlatform* pPlatform, const char* pWindowTitle, u16 uResX, u16 uResY)
{
	QueryPerformanceFrequency(&iPerfFreq);
	wchar_t pLongWindowTitle[256];
	i32 iTitleLen = strlen(pWindowTitle);
	for (i32 i = 0; i <= iTitleLen; i++)
	{
		pLongWindowTitle[i] = pWindowTitle[i];
	}

	*pPlatform = lzy_alloc(sizeof(LzyPlatform_impl), 8, LZY_MEMORY_TAG_PLATFORM_STATE);
	LzyPlatform_impl* pState = *pPlatform;

	WNDCLASS windowClass;

	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = LzyWindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.hIcon = NULL;
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = L"LzyWindowClass";

	if (!RegisterClass(&windowClass))
	{
		u32 uErrorCode = GetLastError();
		LERROR("Win32 window class registering error (code#%u)", uErrorCode);
		return false;
	}
	LPCTSTR;

	pState->hWindow = CreateWindow(L"LzyWindowClass",
								   pLongWindowTitle,
								   WS_OVERLAPPEDWINDOW | WS_VISIBLE,
								   CW_USEDEFAULT, CW_USEDEFAULT,
								   uResX, uResY,
								   NULL, NULL,
								   windowClass.hInstance,
								   NULL);
	if (!pState->hWindow) {
		u32 uErrorCode = GetLastError();
		LERROR("Win32 window creation error (code#%u)", uErrorCode);
		return false;
	}

	return true;
}

u64 lzy_platform_get_implementation_size()
{
	return sizeof(LzyPlatform_impl);
}

void lzy_platform_get_surface_create_info(LzyPlatform platform, LzyWindowSurfaceCreateInfo* pSurface)
{
	LzyPlatform_impl* pState = platform;
	pSurface->sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	pSurface->hinstance = GetModuleHandle(NULL);
	pSurface->hwnd = pState->hWindow;
}
VkResult lzy_platform_create_surface(VkInstance instance, const LzyWindowSurfaceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocs, VkSurfaceKHR* pSurface)
{
	return vkCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocs, pSurface);
}


b8 lzy_platform_poll_events(LzyPlatform platform)
{
	LzyPlatform_impl* pState = platform;
	MSG msg;
	while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return true;
		}
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	return false;
}

void lzy_platform_shutdown(LzyPlatform platform)
{

}

void lzy_platform_get_framebuffer_size(LzyPlatform platform, u16* pX, u16* pY)
{
	LzyPlatform_impl* pState = platform;
	RECT rect;
	GetWindowRect(pState->hWindow, &rect);
	if (pX)
		*pX = rect.right - rect.left;
	if (pY)
		*pY = rect.top - rect.bottom;
}

void* lzy_platform_alloc(u64 uSize, u8 uAlignment)
{
	return VirtualAlloc(NULL, uSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void lzy_platform_free(void* ptr, u64 uSize, u8 uAlignment)
{
	VirtualFree(ptr, 0, MEM_RELEASE);
}

void* lzy_platform_memcpy(void* pDst, void* pSrc, u64 uSize)
{
	return memcpy(pDst, pSrc, uSize);
}

void* lzy_platform_memset(void* pDst, u8 uVal, u64 uSize)
{
	return memset(pDst, uVal, uSize);
}

void* lzy_platform_memzero(void* pDst, u64 uSize)
{
	return ZeroMemory(pDst, uSize);
}

void lzy_platform_sleep(u64 uMs)
{
	Sleep(uMs);
}

f64 lzy_platform_get_time()
{
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	return (f64)t.QuadPart / (f64)iPerfFreq.QuadPart;
}

#endif