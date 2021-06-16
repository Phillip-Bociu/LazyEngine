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



// j1 j2
// j3
// j4 

b8 lzy_job_system_init();
void lzy_job_system_shutdown();

LAPI b8 lzy_job_system_enque(LzyJob* pJob);
LAPI b8 lzy_job_system_is_idle();
LAPI void lzy_job_system_wait_for_idle();