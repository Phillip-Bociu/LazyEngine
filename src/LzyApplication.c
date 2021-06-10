#include "LzyApplication.h"
#include "LzyGame.h"
#include "LzyMemory.h"
#include "LzyLog.h"
#include <stdlib.h>
#include <string.h>


typedef struct LzyApplication
{
    LzyGame *pGame;
    f64 fLastTime;
    LzyPlatform platform;
    u16 uResX;
    u16 uResY;
    b8 bIsRunning;
    b8 bIsSuspended;
} LzyApplication;

global b8 bIsInitialized = false;
global LzyApplication lzyApp;

b8 lzy_application_create(LzyGame *pGame)
{
    if (bIsInitialized)
    {
        LERROR("%s", "Application was already created");
        return false;
    }

    LTRACE("lol %f", 3.14f);
    LINFO("lol %f", 3.14f);
    LWARN("lol %f", 3.14f);
    LERROR("lol %f", 3.14f);
    LFATAL("lol %f", 3.14f);
    LASSERT(false, "Assertion Test");

    if (!lzy_platform_create(&lzyApp.platform,
                             pGame->appConfig.pApplicationName,
                             pGame->appConfig.uResX,
                             pGame->appConfig.uResY))
    {
        return false;
    }
    lzyApp.pGame = pGame;
    lzyApp.fLastTime = lzy_platform_get_time();
    lzyApp.uResX = pGame->appConfig.uResX;
    lzyApp.uResY = pGame->appConfig.uResY;
    lzyApp.bIsRunning = true;
    lzyApp.bIsSuspended = false;

    if(!pGame->fpStart(pGame))
    {
        LFATAL("%s","Could not start the game");
        return false;
    }

    //Subsystem Initializations

    lzy_memory_init();



    bIsInitialized = true;

    return true;
}

b8 lzy_application_run()
{
    lzyApp.fLastTime = lzy_platform_get_time();
    
    while (lzyApp.bIsRunning)
    {
        f64 fCurrentTime = lzy_platform_get_time();
        f32 fDeltaTime = fCurrentTime - lzyApp.fLastTime;
        lzyApp.fLastTime = fCurrentTime;

        lzyApp.bIsRunning = !lzy_platform_poll_events(lzyApp.platform);
        
        if(!lzyApp.bIsSuspended)
        {
            if(!lzyApp.pGame->fpUpdate(lzyApp.pGame, fDeltaTime))
            {
                LFATAL("%s","Game update failed!");
                break;
            }

            if(!lzyApp.pGame->fpRender(lzyApp.pGame, fDeltaTime))
            {
                LFATAL("%s","Game Render failed!");
                break;
            }
        }
    }

    lzyApp.bIsRunning = false;

    lzy_platform_shutdown(lzyApp.platform);

    return true;
}