#include "LzyMemory.h"
#include "LzyPlatform.h"
#include "LzyLog.h"
#include <stdlib.h>

typedef struct LzyMemStats
{
    u64 uTotalAllocs;
    u64 uTaggedAllocs[LZY_MEMORY_TAG_MAX];
}LzyMemStats;

typedef struct LzyMemory
{
    u64 uMemorySize;
    void* pMemory;
}LzyMemory;

global LzyMemStats memStats;

b8 lzy_memory_init(const LzyMemoryConfig* pConfig)
{
    LINFO("Memory Subsystem Initialized");
    lzy_platform_memzero(&memStats, sizeof(memStats));
    lzy_platform_alloc(pConfig->uTotalMemorySize, 8);

    return true;
}

void lzy_memory_shutdown()
{

}


void* lzy_alloc(u64 uSize, u8 uAlignment, LzyMemoryTag memTag)
{
    if(memTag == LZY_MEMORY_TAG_UNKNOWN)
    {
        LCOREWARN("Untagged memory allocation!");
    }

    memStats.uTotalAllocs += uSize;
    memStats.uTaggedAllocs[memTag] += uSize;

    void* retval = lzy_platform_alloc(uSize, uAlignment);
    LCOREASSERT(retval != NULL, "Out of Memory.");
    lzy_platform_memzero(retval, uSize);
    return retval;
}

void lzy_free(void* ptr, u64 uSize, LzyMemoryTag memTag)
{
    if(memTag == LZY_MEMORY_TAG_UNKNOWN)
    {
        LCOREWARN("Untagged memory deallocation!");
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


internal_func void lzy_get_resulting_bytes(f64* pBytes, u32* pOrder)
{
    *pOrder = 0;

    while (*pBytes > 1024.0)
    {
        *pBytes /= 1024.0;
        (*pOrder)++;
    }

}

//TODO
char* lzy_get_memstats()
{
    //char* retval = lzy_alloc(LZY_MEMORY_TAG_MAX * 64, 1, LZY_MEMORY_TAG_STRING);

    const char* ppByteAfixes[] = {"B", "KB", "MB", "GB", "TB"};


    f64 fTotalAllocs = memStats.uTotalAllocs;
    u32 uOrder;

    lzy_get_resulting_bytes(&fTotalAllocs, &uOrder);
    LCOREERROR("Total allocations: %f%s", fTotalAllocs, ppByteAfixes[uOrder]);
    
    fTotalAllocs = memStats.uTaggedAllocs[LZY_MEMORY_TAG_RENDERER_INIT];
    lzy_get_resulting_bytes(&fTotalAllocs, &uOrder);

    LCOREERROR("Renderer Init allocations: %f%s", fTotalAllocs, ppByteAfixes[uOrder]);

    
    return NULL;
}

