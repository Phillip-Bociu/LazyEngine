#pragma once
#include "LzyDefines.h"
#include "LzyApplication.h"

typedef struct LzyGame
{
    b8(*fpStart)(struct LazyGame* pGame);
    b8(*fpUpdate)(struct LazyGame* pGame, f32 fDeltaTime);
    b8(*fpRender)(struct LazyGame* pGame, f32 fDeltaTime);
    void(*fpOnResize)(struct LazyGame* pGame, u16 uResX, u16 uResY);
    void* pData;
    LzyApplicationConfig appConfig;
}LzyGame;

