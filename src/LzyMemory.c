#include "LzyMemory.h"
#include "LzyPlatform.h"
#include "LzyLog.h"
#include <stdlib.h>

struct LzyMemStats
{
    u64 uTotalAllocs;
    u64 uTaggedAllocs[LZY_MEMORY_TAG_MAX];
};

global struct LzyMemStats memStats;

void lzy_memory_init()
{
    LINFO("Memory Subsystem Initialized");
    lzy_platform_memzero(&memStats, sizeof(memStats));
}

void lzy_memory_shutdown()
{

}


void* lzy_alloc(u64 uSize, u8 uAlignment, LzyMemoryTag memTag)
{
    if(memTag == LZY_MEMORY_TAG_UNKNOWN)
    {
        LWARN("Untagged memory allocation!");
    }

    memStats.uTotalAllocs += uSize;
    memStats.uTaggedAllocs[memTag] += uSize;

    void* retval = lzy_platform_alloc(uSize, uAlignment);
    LASSERT(retval != NULL, "Out of Memory.");
    lzy_platform_memzero(retval, uSize);
    return retval;
}

void* lzy_realloc(void* ptr, u64 uOldSize, u64 uNewSize, u8 uAlignment, LzyMemoryTag memTag)
{
    if (memTag == LZY_MEMORY_TAG_UNKNOWN)
    {
        LWARN("Untagged memory reallocation!");
    }

    memStats.uTotalAllocs += (i64)(uNewSize - uOldSize);
    memStats.uTaggedAllocs[memTag] += (i64)(uNewSize - uOldSize);
    void* retval = lzy_platform_realloc(ptr, uNewSize);
    LASSERT(retval != NULL, "Out of Memory");
    lzy_platform_memzero(retval, uNewSize);
    return retval;
}

void lzy_free(void* ptr, u64 uSize, LzyMemoryTag memTag)
{
    if(memTag == LZY_MEMORY_TAG_UNKNOWN)
    {
        LWARN("Untagged memory deallocation!");
    }

    memStats.uTotalAllocs -= uSize;
    memStats.uTaggedAllocs[memTag] -= uSize;

    lzy_platform_free(ptr, 0);
}


void* lzy_memzero(void* ptr, u64 uSize)
{
    return lzy_platform_memzero(ptr,uSize);
}
void* lzy_memset(void* ptr, u8 uVal, u64 uSize)
{
    return lzy_platform_memset(ptr,uVal,uSize);
}
void* lzy_memcpy(void* pDst, void* pSrc, u64 uSize)
{
   return lzy_platform_memcpy(pDst, pSrc, uSize);
}


//TODO
char* lzy_get_memstats()
{
    char* retval = lzy_alloc(LZY_MEMORY_TAG_MAX * 64, 1, LZY_MEMORY_TAG_STRING);

    
    return retval;
}

