
#include <LzyEntryPoint.h>

b8 gameStart(LzyGame *pGame)
{
	printf("Game Started!\n");
	return true;
}

b8 gameUpdate(LzyGame *pGame, f32 fDeltaTime)
{
	//printf("Game Updated!\n");
	return true;
}

b8 gameRender(LzyGame *pGame, f32 fDeltaTime)
{
	//printf("Game Rendered!\n");
	return true;
}

void gameResize(LzyGame *pGame, u16 uResX, u16 uResY)
{
	printf("Game Resized!\n");
}

b8 create_game(LzyGame *pGame)
{
	pGame->fpStart = gameStart;
	pGame->fpUpdate = gameUpdate;
	pGame->fpRender = gameRender;
	pGame->fpOnResize = gameResize;
	pGame->appConfig.pApplicationName = "Game :D";
	pGame->appConfig.uResX = 1280;
	pGame->appConfig.uResY = 720;
	return true;
}
