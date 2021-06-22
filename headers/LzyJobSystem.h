#pragma once
#include "LzyThread.h"


#define LZY_JOB_ID_INVALID (-1u)

typedef struct LzyJobResult
{
	void* pResult;
}LzyJobResult;

typedef (*LzyJobFunc)(LzyJob* pJob, void* pArgs);

typedef struct LzyJob
{
	LzyJobFunc fpJobFunc;
	void* pArgs;
	u32 uJobID;
	u32 uParentID;
	u32 uUnfinishedChildJobs;
}LzyJob;



b8 lzy_job_system_init();
void lzy_job_system_shutdown();

LAPI b8 lzy_job_system_enque(LzyJob* pJob);

LAPI b8 lzy_job_system_enque_free_job(LzyJobFunc fpJobFunc, void* pArgs, u32 uParentID);
//returns job ID
LAPI u32 lzy_job_system_enque_parent_job(LzyJobFunc fpJobFunc, void* pArgs, u32 uChildJobCount);
LAPI b8 lzy_job_system_is_idle();
LAPI void lzy_job_system_wait_for_idle();