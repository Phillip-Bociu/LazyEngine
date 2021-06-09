#include "LzyDefines.h"
#include "LzyLog.h"
#include "LzyPlatform.h"
#ifdef _WIN32
#include <Windows.h>
#include <stdlib.h>
#include <string.h>

typedef struct LzyPlatform_impl
{
	HWND hWindow;
	LARGE_INTEGER iPerfFreq;
}LzyPlatform_impl;


b8 lzy_platform_create(LzyPlatform* pPlatform, const char* pWindowTitle, u16 uResX, u16 uResY)
{
	*pPlatform = malloc(sizeof(LzyPlatform_impl));
	LzyPlatform_impl* pState = *pPlatform;

	WNDCLASSA windowClass;

	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = DefWindowProcA;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.hIcon = NULL;
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = "LzyWindowClass";

	RegisterClassA(&windowClass); 


	pState->hWindow = CreateWindowExA(0,
									  &windowClass,
									  pWindowTitle,
									  WS_CAPTION | WS_BORDER | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
									  0, 0,
									  uResX, uResY,
									  NULL, NULL,
									  windowClass.hInstance,
									  NULL);
	if (!pState->hWindow) {
		u32 uErrorCode = GetLastError();
		LERROR("Win32 window creation error (code#%u)", uErrorCode);
		return false;
	}

	ShowWindow(pState->hWindow, SW_SHOW);

	

	return true;
}
b8 lzy_platform_poll_events(LzyPlatform platform)
{
	return true;
}
void lzy_platform_shutdown(LzyPlatform platform)
{

}
void* lzy_platform_alloc(u64 uSize, u8 uAlignment)
{
	return malloc(uSize);
}
void lzy_platform_free(void* ptr, u8 uAlignment)
{
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
f64 lzy_platform_get_time()
{

	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	t.QuadPart;
}

#endif