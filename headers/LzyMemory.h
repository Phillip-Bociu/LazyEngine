#pragma once
#include "LzyDefines.h"
#include "LzyLog.h"


typedef enum LzyMemoryTag
{
    LZY_MEMORY_TAG_UNKNOWN,
    LZY_MEMORY_TAG_GAME,
    LZY_MEMORY_TAG_EVENT_SYSTEM,
    LZY_MEMORY_TAG_APPLICATION,
    LZY_MEMORY_TAG_MAX
}LzyMemoryTag;


void lzy_memory_init();
void lzy_memory_shutdown();

LAPI void* lzy_alloc(u64 uSize, u8 uAlignment, LzyMemoryTag memTag);
LAPI void lzy_free(void* ptr, u64 uSize, LzyMemoryTag memTag);
LAPI void* lzy_memzero(void* ptr, u64 uSize);
LAPI void* lzy_memset(void* ptr, u8 uVal, u64 uSize);
LAPI void* lzy_memcpy(void* pDst, void* pSrc, u64 uSize);
LAPI char* lzy_get_memstats();