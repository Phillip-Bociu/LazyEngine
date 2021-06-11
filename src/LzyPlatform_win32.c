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

	*pPlatform = malloc(sizeof(LzyPlatform_impl));
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
								   WS_VISIBLE,//WS_OVERLAPPEDWINDOW | WS_VISIBLE,
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
	return (f64)t.QuadPart / (f64)iPerfFreq.QuadPart;
}

#endif