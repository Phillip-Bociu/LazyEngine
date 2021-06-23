#include "LzyPlatform.h"
#include "LzyApplication.h"
#include "LzyRenderer.h"
#include "LzyGame.h"
#include "LzyMemory.h"
#include "LzyLog.h"
#include "LzyEvent.h"
#include "LzyJobSystem.h"
#include "LzyTime.h"
#include <stdlib.h>
#include <string.h>


typedef struct LzyApplication
{
    LzyGame *pGame;
    LzyTime clock;    
    LzyPlatform platform;
    u16 uResX;
    u16 uResY;
    b8 bIsRunning;
    b8 bIsSuspended;
} LzyApplication;

global b8 bIsInitialized = false;
global LzyApplication lzyApp;

internal_func void _lzy_event_init(LzyJob* pJob, void* pArgs)
{
    *(b8*)pArgs = lzy_event_init();
}

internal_func void _lzy_renderer_init(LzyJob* pJob, void* pArgs)
{
    *(b8*)pArgs = lzy_renderer_init();
}

internal_func void _lzy_platform_init(LzyJob* pJob, void* pArgs)
{
    struct arg_t
    {
        LzyGame* pGame;
        b8* pSucceded;
    }*pArgs_ = pArgs;

    pArgs_->pSucceded = lzy_platform_create(&lzyApp.platform, 
                                            pArgs_->pGame->appConfig.pApplicationName,
                                            pArgs_->pGame->appConfig.uResX,
                                            pArgs_->pGame->appConfig.uResY
                                            );
}



b8 lzy_application_create(LzyGame* pGame)
{
    if (bIsInitialized)
    {
        LCOREERROR("%s", "Application was already created");
        return false;
    }

    LCORETRACE("lol %f", 3.14f);
    LCOREINFO("lol %f", 3.14f);
    LCOREWARN("lol %f", 3.14f);
    LCOREERROR("lol %f", 3.14f);
    LCOREFATAL("lol %f", 3.14f);
    LCOREASSERT(false, "Assertion Test");


    if (!lzy_memory_init(&pGame->appConfig.memoryConfig))
    {
        LCOREFATAL("Could not initialize memory subsystem!");
        return false;
    }

    if (!lzy_job_system_init())
    {
        LCOREFATAL("Could not initialize job system!");
        return false;
    }

    if (!lzy_platform_create(&lzyApp.platform,
                             pGame->appConfig.pApplicationName,
                             pGame->appConfig.uResX,
                             pGame->appConfig.uResY))
    {
        return false;
    }
    lzyApp.pGame = pGame;
    lzy_time_start(&lzyApp.clock);
    lzyApp.uResX = pGame->appConfig.uResX;
    lzyApp.uResY = pGame->appConfig.uResY;
    lzyApp.bIsRunning = true;
    lzyApp.bIsSuspended = false;



    //Subsystem Initializations
#if 1

    b8 bEventInitialized;
    b8 bRendererInitialized = true;

    lzy_job_system_enque_free_job(_lzy_renderer_init, &bRendererInitialized, LZY_JOB_ID_INVALID);
    lzy_job_system_enque_free_job(_lzy_event_init, &bEventInitialized, LZY_JOB_ID_INVALID);

    while (!lzy_job_system_is_idle())
    {
    }

    if (!bEventInitialized)
    {
        LCOREFATAL("Could not initialize event subsystem!");
        return false;
    }

    if (!bRendererInitialized)
    {
        LCOREFATAL("Could not initialize renderer subsystem!");
        return false;
    }


    LCOREINFO("Gata");
#else

    if(!lzy_event_init())
    {
        LCOREFATAL("Could not initialize event subsystem!");
        return false;
    }

    if(!lzy_renderer_init())
    {
        LCOREFATAL("Could not initialize renderer subsystem!");
        return false;
    }
#endif

    //TODO Memory Stage 2

    if (!pGame->fpStart(pGame))
    {
        LCOREFATAL("%s", "Could not start the game");
        return false;
    }

    bIsInitialized = true;

    return true;
}

void lzy_application_get_framebuffer_size(u16* pX, u16* pY)
{
    lzy_platform_get_framebuffer_size(lzyApp.platform, pX, pY);
}

void lzy_application_get_surface_create_info(LzyWindowSurfaceCreateInfo* pSurface)
{
    lzy_platform_get_surface_create_info(lzyApp.platform, pSurface);
}

VkResult lzy_application_create_surface(VkInstance instance, const LzyWindowSurfaceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocs, VkSurfaceKHR* pSurface)
{
    return lzy_platform_create_surface(instance, pCreateInfo, pAllocs, pSurface);
}

b8 lzy_application_run()
{
    f64 fTime = 0.0;
    u64 uFrameCounter = 0;
    while (lzyApp.bIsRunning)
    {
        lzy_time_step(&lzyApp.clock);
        fTime += lzy_time_get_deltatime(lzyApp.clock);
        if (fTime >= 1.0)
        {
            fTime = 0;
            LCOREINFO("FPS:%llu", uFrameCounter);
            uFrameCounter = 0;
        }
        lzyApp.bIsRunning = !lzy_platform_poll_events(lzyApp.platform);
        
        if(!lzyApp.bIsSuspended)
        {
            if(!lzyApp.pGame->fpUpdate(lzyApp.pGame, lzy_time_get_deltatime(lzyApp.clock)))
            {
                LCOREFATAL("%s","Game update failed!");
                break;
            }

            if(!lzyApp.pGame->fpRender(lzyApp.pGame, lzy_time_get_deltatime(lzyApp.clock)))
            {
                LCOREFATAL("%s","Game Render failed!");
                break;
            }

            //lzy_renderer_loop();
        }

        uFrameCounter++;
        
    }

    lzyApp.bIsRunning = false;

    lzy_platform_shutdown(lzyApp.platform);

    return true;
}