#pragma once
#include "LzyDefines.h"
#include "LzyLog.h"


//TODO: make custom allocator for pre-allocated memory
typedef enum LzyMemoryTag
{
	LZY_MEMORY_TAG_UNKNOWN,
	LZY_MEMORY_TAG_GAME,
	LZY_MEMORY_TAG_EVENT_SYSTEM_STATE,
	LZY_MEMORY_TAG_VECTOR,
	LZY_MEMORY_TAG_STRING,
	LZY_MEMORY_TAG_APPLICATION,
	LZY_MEMORY_TAG_PLATFORM_STATE,
	LZY_MEMORY_TAG_RENDERER_INIT,
	LZY_MEMORY_TAG_RENDERER_STATE,
	LZY_MEMORY_TAG_FILE_MAPPING,
	LZY_MEMORY_TAG_MAX
}LzyMemoryTag;




//All LZY_MEMORY_TAG_*_INIT tags are ignored
typedef struct LzyMemoryConfig
{
	u64 uTotalMemorySize;
	u64 pTaggedMemorySize[LZY_MEMORY_TAG_MAX];
}LzyMemoryConfig;

b8 lzy_memory_init(const LzyMemoryConfig* pMemoryConfig);
void lzy_memory_shutdown();

LAPI void* lzy_alloc(u64 uSize, u8 uAlignment, LzyMemoryTag memTag);
LAPI void* lzy_realloc(void* ptr, u64 uOldSize, u64 uNewSize, u8 uAlignment, LzyMemoryTag memTag);
LAPI void lzy_free(void* ptr, u64 uSize, LzyMemoryTag memTag);
LAPI void* lzy_memzero(void* ptr, u64 uSize);
LAPI void* lzy_memset(void* ptr, u8 uVal, u64 uSize);
LAPI void* lzy_memcpy(void* pDst, void* pSrc, u64 uSize);
LAPI b8 lzy_get_memstats(char* pBuffer, u64 uBufferCapacity);

void* lzy_realloc_fastobj(void* ptr, u64 uSize);
void lzy_free_fastobj(void* ptr);
