#pragma once
#include "LzyDefines.h"

MAKE_HANDLE(LzyPlatform);

b8 lzy_platform_create(LzyPlatform* pPlatform, const char* pWindowTitle,u16 uResX, u16 uResY);
b8 lzy_platform_poll_events(LzyPlatform platform);
void lzy_platform_shutdown(LzyPlatform platform);
void* lzy_platform_alloc(u64 uSize, u8 uAlignment);
void lzy_platform_free(void* ptr, u8 uAlignment);
void* lzy_platform_memcpy(void* pDst, void* pSrc, u64 uSize);
void* lzy_platform_memset(void* pDst, u8 uVal, u64 uSize);
void* lzy_platform_memzero(void* pDst, u64 uSize);
void lzy_platform_sleep(u64 uMs);
f64 lzy_platform_get_time();