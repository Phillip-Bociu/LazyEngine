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
    LzyApplicationConfig appConfig = {};
    appConfig.pApplicationName = "Game :DDD";
    appConfig.uResX = 1270;
    appConfig.uResY = 720;
    
    return true;
}



