#include "LzyTime.h"
#include "LzyLog.h"
#include "LzyMemory.h"
#include "LzyPlatform.h"

b8 lzy_time_start(LzyTime* pTime)
{
	LCOREASSERT(pTime != NULL, "pTime pointer is NULL");
	pTime->fCurrentTime = 0.0;
	pTime->fLastTime = 0.0;
	pTime->fTimeScale = 1.0;
	return true;
}
void lzy_time_step(LzyTime* pTime)
{
	pTime->fLastTime = pTime->fCurrentTime;
	pTime->fCurrentTime = lzy_platform_get_time();	
}

f64 lzy_time_get_deltatime(LzyTime time)
{
	return time.fTimeScale * (time.fCurrentTime - time.fLastTime);
}