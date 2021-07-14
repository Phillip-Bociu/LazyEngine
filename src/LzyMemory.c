#include "LzyMemory.h"
#include "LzyPlatform.h"
#include "LzyLog.h"
#include <stdlib.h>

typedef struct LzyLinearAllocator
{
    void* uCapacity;
    void* pEnd;
    u8 pData[];
}LzyLinearAllocator;

typedef struct LzyMemStats
{
    u64 uTotalAllocs;
    u64 uTaggedAllocs[LZY_MEMORY_TAG_MAX];
}LzyMemStats;

typedef struct LzyMemory
{
    u64 uMemorySize;
    void* pMemory;
    struct
    {
        void* pBegin;
        void* pEnd;
    }ppMemorySections[LZY_MEMORY_TAG_MAX];

}LzyMemory;

global LzyMemStats memStats;
global LzyMemory memory;
global const string pTagStrings[] = 
{
	"Unknown",
	"Game",
	"Event System",
	"Vector",
	"String",
	"Application",
	"Platform State",
	"Renderer Init",
	"RendererState"
};



b8 lzy_memory_init(const LzyMemoryConfig* pConfig)
{
    LINFO("Memory Subsystem Initialized");
    lzy_platform_memzero(&memStats, sizeof(memStats));
    lzy_platform_memzero(&memory, sizeof(memory));

    //memory.uMemorySize = pConfig->uTotalMemorySize;
    //memory.pMemory = lzy_platform_alloc(pConfig->uTotalMemorySize, 8);
//
    //if (!memory.pMemory)
    //{
    //    LCOREFATAL("Not enough memory");
    //    return false;
    //}
    //memory.ppMemorySections[LZY_MEMORY_TAG_PLATFORM_STATE].pBegin = memory.pMemory;
    //memory.ppMemorySections[LZY_MEMORY_TAG_PLATFORM_STATE].pEnd = memory.pMemory;
//
    //memory.ppMemorySections[LZY_MEMORY_TAG_EVENT_SYSTEM_STATE].pBegin = memory.pMemory;
    //memory.ppMemorySections[LZY_MEMORY_TAG_EVENT_SYSTEM_STATE].pEnd   = (u8*)memory.pMemory + pConfig->pTaggedMemorySize[LZY_MEMORY_TAG_EVENT_SYSTEM_STATE];
//
    //memory.ppMemorySections[LZY_MEMORY_TAG_RENDERER_STATE].pBegin = (u8*)memory.pMemory + pConfig->pTaggedMemorySize[LZY_MEMORY_TAG_EVENT_SYSTEM_STATE];
    //memory.ppMemorySections[LZY_MEMORY_TAG_RENDERER_STATE].pEnd   = (u8*)memory.pMemory + pConfig->pTaggedMemorySize[LZY_MEMORY_TAG_EVENT_SYSTEM_STATE] + pConfig->pTaggedMemorySize[LZY_MEMORY_TAG_RENDERER_STATE];
//
    //memory.ppMemorySections[LZY_MEMORY_TAG_RENDERER_INIT].pBegin = (u8*)memory.pMemory + pConfig->pTaggedMemorySize[LZY_MEMORY_TAG_EVENT_SYSTEM_STATE] + pConfig->pTaggedMemorySize[LZY_MEMORY_TAG_RENDERER_STATE];
    //memory.ppMemorySections[LZY_MEMORY_TAG_RENDERER_INIT].pEnd   = (u8*)memory.pMemory + memory.uMemorySize;

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

void* lzy_realloc(void* ptr, u64 uOldSize, u64 uNewSize, u8 uAlignment, LzyMemoryTag memTag)
{
    if(memTag == LZY_MEMORY_TAG_UNKNOWN)
    {
        LCOREWARN("Untagged memory allocation!");
    }

    memStats.uTotalAllocs += (i64)(uNewSize - uOldSize);
    memStats.uTaggedAllocs[memTag] += (i64)(uNewSize - uOldSize);

    void* retval = lzy_platform_realloc(ptr, uNewSize);
    LCOREASSERT(retval != NULL, "Out of Memory.");
    lzy_platform_memzero((u8*)retval + uOldSize, (i64)(uNewSize - uOldSize));
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

    lzy_platform_free(ptr, uSize,8);
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

b8 lzy_get_memstats(c8* pBuffer, u64 uBufferCapacity)
{
    
    c8* pWrite = pBuffer;
    u64 uWriteAllocationSize = 0;
    if(!pBuffer)
    {
        pWrite = lzy_alloc(128 * LZY_MEMORY_TAG_MAX * sizeof(c8), 1, LZY_MEMORY_TAG_STRING);
        uWriteAllocationSize = 128 * LZY_MEMORY_TAG_MAX * sizeof(c8);
    }

    const c8* ppByteAfixes[] = {"B", "KB", "MB", "GB", "TB"};


    f64 fTotalAllocs = memStats.uTotalAllocs;
    u32 uOrder;

    lzy_get_resulting_bytes(&fTotalAllocs, &uOrder);
    LCOREINFO("Total allocations: %f%s", fTotalAllocs, ppByteAfixes[uOrder]);
    

    for(LzyMemoryTag tag = LZY_MEMORY_TAG_UNKNOWN; tag != LZY_MEMORY_TAG_MAX; tag++)
    {
        fTotalAllocs = memStats.uTaggedAllocs[LZY_MEMORY_TAG_RENDERER_INIT];
        lzy_get_resulting_bytes(&fTotalAllocs, &uOrder);
        LCOREERROR("%s: %f%s",  pTagStrings[tag],fTotalAllocs, ppByteAfixes[uOrder]);
    }

    
    return true;
}
