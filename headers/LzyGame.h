#pragma once
#include "LzyDefines.h"
#include "LzyApplication.h"

typedef struct LzyGame
{
    b8(*fpStart)(struct LzyGame* pGame);
    b8(*fpUpdate)(struct LzyGame* pGame, f32 fDeltaTime);
    b8(*fpRender)(struct LzyGame* pGame, f32 fDeltaTime);
    void(*fpOnResize)(struct LzyGame* pGame, u16 uResX, u16 uResY);
    void* pData;
    LzyApplicationConfig appConfig;
}LzyGame;

