#pragma once
#include "LzyThread.h"


#define LZY_JOB_ID_INVALID (~0u)

typedef struct LzyJobResult
{
	void* pResult;
}LzyJobResult;



typedef struct LzyJob
{
	void (*fpJobFunc)(struct LzyJob* pJob, void* pArgs);
	void* pArgs;
	u32 uJobID;
	u32 uParentID;
	u32 uUnfinishedChildJobs;
}LzyJob;


typedef void (*LzyJobFunc)(LzyJob* pJob, void* pArgs);


b8 lzy_job_system_init();
void lzy_job_system_shutdown();


LAPI b8 lzy_job_system_enque_free_job(LzyJobFunc fpJobFunc, void* pArgs, u32 uParentID);
//returns job ID
LAPI u32 lzy_job_system_enque_parent_job(LzyJobFunc fpJobFunc, void* pArgs, u32 uChildJobCount);
LAPI b8 lzy_job_system_is_idle();
