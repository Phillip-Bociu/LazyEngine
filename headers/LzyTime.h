#pragma once
#include "LzyDefines.h"


typedef struct LzyTime
{
	f64 fLastTime;
	f64 fCurrentTime;
	f64 fTimeScale;
}LzyTime;


b8 lzy_time_start(LzyTime* pTime);
void lzy_time_step(LzyTime* pTime);

f64 lzy_time_get_deltatime(LzyTime time);