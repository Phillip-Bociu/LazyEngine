#include <LzyLog.h> 
#include <LzyRenderer.h>
#include <LzyEntryPoint.h>

b8 gameStart(LzyGame* pGame)
{
    return true;
}


b8 gameUpdate(LzyGame* pGame, f32 fDeltaTime)
{
    return true;
}


b8 gameRender(LzyGame* pGame, f32 fDeltaTime)
{
    if(!lzy_renderer_loop(fDeltaTime))
    {
        LFATAL("Rendering Error");
        return false;
    }
    return true;
}

void onResize(LzyGame* pGame, u16 uResX, u16 uResy)
{
}

b8 create_game(LzyGame* pGame)
{
    pGame->appConfig.pApplicationName = "Game :DDD";
    pGame->appConfig.uResX = 1270;
    pGame->appConfig.uResY = 720;
    
    pGame->fpStart = gameStart;
    pGame->fpUpdate = gameUpdate;
    pGame->fpRender = gameRender;
    pGame->fpOnResize = onResize;
 
    return true;
}



