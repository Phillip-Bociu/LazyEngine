#ifdef _WIN32
#include "LzyDefines.h"
#include "LzyLog.h"
#include "LzyPlatform.h"
#include "LzyMemory.h"
#include "LzyEvent.h"
#include <Windows.h>
#include <stdlib.h>
#include <string.h>

typedef struct LzyPlatform_impl
{
	HWND hWindow;
}LzyPlatform_impl;

global LARGE_INTEGER iPerfFreq;

void lzy_application_set_framebuffer_size(u16 uX, u16 uY);


internal_func
LRESULT LzyWindowProc(HWND hWindow, 
                      UINT msg,
                      WPARAM wParam,
                      LPARAM lParam)
{
	LRESULT lResult;
	switch (msg)
	{
        
        case WM_CLOSE:
        {
            PostQuitMessage(0);
        }break;
        
        case WM_KEYDOWN:
        {
            LzyEventData eData = {.u64 = {wParam, lParam & 0xFFFF}};
            lzy_event_emit(LZY_EVENT_CODE_KEY_PRESS, (void*)1, eData);
        }break;
        
        case WM_KEYUP:
        {
            LzyEventData eData = {.u64 = {wParam, lParam & 0xFFFF}};
            lzy_event_emit(LZY_EVENT_CODE_KEY_RELEASE, (void*)1, eData);
        }break;
        
        case WM_SIZE:
        {
            u32 uWidth  = LOWORD(lParam);
            u32 uHeight = HIWORD(lParam);
            LCORETRACE("New Window Size: %u, %u", uWidth, uHeight);
            lzy_application_set_framebuffer_size(uWidth, uHeight);
            return 0;    
        }break;
        
        default:
        {
        }break;    
        
    }
	return DefWindowProc(hWindow, msg, wParam, lParam);
}

b8 lzy_platform_create(LzyPlatform* pPlatform, const char* pWindowTitle, u16 uResX, u16 uResY)
{
	QueryPerformanceFrequency(&iPerfFreq);

	*pPlatform = lzy_alloc(sizeof(LzyPlatform_impl), 8, LZY_MEMORY_TAG_PLATFORM_STATE);
	LzyPlatform_impl* pState = *(LzyPlatform_impl**)pPlatform;

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
	windowClass.lpszClassName = "LzyWindowClass";

	if (!RegisterClass(&windowClass))
	{
		u32 uErrorCode = GetLastError();
		LERROR("Win32 window class registering error (code#%u)", uErrorCode);
		return false;
	}

	RECT res = {
		.left = 100,
		.right = 100 + uResX,
		.top = 100,
		.bottom = 100 + uResY 
	};

	if (!AdjustWindowRect(&res, WS_OVERLAPPEDWINDOW, FALSE))
	{
		LCOREFATAL("Could not adjust window rect size");
		return false;
	}


	pState->hWindow = CreateWindow("LzyWindowClass",
								   pWindowTitle,
								   WS_OVERLAPPEDWINDOW | WS_VISIBLE,
								   CW_USEDEFAULT, CW_USEDEFAULT,
								   res.right - res.left, res.bottom - res.top,
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
	LzyPlatform_impl* pState = (LzyPlatform_impl*)platform;
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
	LzyPlatform_impl* pState = (LzyPlatform_impl*)platform;
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
	LzyPlatform_impl* pState = (LzyPlatform_impl*)platform;
	RECT rect;
	GetWindowRect(pState->hWindow, &rect);
	if (pX)
		*pX = rect.right - rect.left;
	if (pY)
		*pY = rect.bottom - rect.top;
}

void* lzy_platform_alloc(u64 uSize, u8 uAlignment)
{
	//return VirtualAlloc(NULL, uSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	return malloc(uSize);
}

void lzy_platform_free(void* ptr, u64 uSize, u8 uAlignment)
{
	//VirtualFree(ptr, 0, MEM_RELEASE);
	free(ptr);
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

void* lzy_platform_realloc(void* ptr, u64 uSize)
{
	return realloc(ptr, uSize);
}

f64 lzy_platform_get_time()
{
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	return (f64)t.QuadPart / (f64)iPerfFreq.QuadPart;
}

b8 lzy_platform_change_title(LzyPlatform platform, const char* pWindowTitle)
{
    LzyPlatform_impl* pState = (LzyPlatform_impl*)platform;
    return (SetWindowTextA(pState->hWindow, pWindowTitle) == 0);
}

#endif