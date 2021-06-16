#pragma once
#include "LzyThread.h"


typedef struct LzyJobResult
{
	void* pResult;
}LzyJobResult;

typedef struct LzyJob
{
	void(*fpJob)(void* pArgs);
	void* pArgs;
}LzyJob;
 

b8 lzy_job_system_init();
void lzy_job_system_shutdown();

LAPI b8 lzy_job_system_enque(LzyJob* pJob);
LAPI b8 lzy_job_system_is_idle();
LAPI void lzy_job_system_wait_for_idle();