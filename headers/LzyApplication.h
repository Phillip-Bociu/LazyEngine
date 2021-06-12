#pragma once
#include "LzyDefines.h"

struct LzyGame;

typedef struct LzyApplicationConfig
{
	char* pApplicationName;
	u16 uResX;
	u16 uResY;
}LzyApplicationConfig;

LAPI b8 lzy_application_create(struct LzyGame* pAppConfig);
LAPI b8 lzy_application_run();

