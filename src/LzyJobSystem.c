#include "LzyJobSystem.h"
#include "LzyLog.h"
#include "LzyMemory.h"

#define LZY_THREAD_POOL_SIZE 8
#define LZY_JOB_QUEUE_SIZE 512
#define LZY_JOB_QUEUE_MOD(x) ((x) & (LZY_JOB_QUEUE_SIZE - 1))

typedef struct LzyJobQueue
{
	u32 uFront;
	u32 uRear;
	LzyJob queue[LZY_JOB_QUEUE_SIZE];
}LzyJobQueue;


typedef struct LzyThreadPool
{
	LzyMutex mutexFreeQueue;
	LzyMutex mutexParentQueue;
	LzySemaphore semFreeFull;
	LzySemaphore semFreeEmpty;
	LzySemaphore semParentEmpty;
	LzyThread threads[LZY_THREAD_POOL_SIZE];
	LzyJobQueue freeJobQueue;
	LzyJobQueue parentJobQueue;
	b8 bShouldClose;
} LzyThreadPool;

global LzyThreadPool threadPool;

internal_func void lzy_job_queue_enque(LzyJobQueue* pQueue, LzyJob* pJob)
{
	pQueue->queue[pQueue->uRear] = *pJob;
	pQueue->uRear = LZY_JOB_QUEUE_MOD(pQueue->uRear + 1);

}

internal_func LzyJob lzy_job_queue_deque(LzyJobQueue* pQueue)
{
	LzyJob retval = pQueue->queue[pQueue->uFront];
	pQueue->uFront = (pQueue->uFront + 1) % LZY_JOB_QUEUE_SIZE;
	return retval;
}

internal_func void lzy_job_system_finish_job(u32 uJobParentID)
{
	if (uJobParentID == LZY_JOB_ID_INVALID)
		return;
	i32 toMove = -1;
	lzy_mutex_lock(&threadPool.mutexParentQueue);
	for (i32 i = threadPool.parentJobQueue.uFront; i != threadPool.parentJobQueue.uRear; i = LZY_JOB_QUEUE_MOD(i + 1))
	{
		if (threadPool.parentJobQueue.queue[i].uJobID == uJobParentID)
		{
			threadPool.parentJobQueue.queue[i].uUnfinishedChildJobs--;
			if (threadPool.parentJobQueue.queue[i].uUnfinishedChildJobs == 0)
			{
				lzy_mutex_lock(&threadPool.mutexFreeQueue);
				lzy_job_queue_enque(&threadPool.freeJobQueue, &threadPool.parentJobQueue.queue[i]);
				lzy_semaphore_signal(&threadPool.semFreeFull);
				lzy_mutex_unlock(&threadPool.mutexFreeQueue);
				LzyJob aux = threadPool.parentJobQueue.queue[i];
				threadPool.parentJobQueue.queue[i] = threadPool.parentJobQueue.queue[threadPool.parentJobQueue.uFront];
				threadPool.parentJobQueue.queue[threadPool.parentJobQueue.uFront] = aux;
				lzy_job_queue_deque(&threadPool.parentJobQueue);
				lzy_semaphore_signal(&threadPool.semParentEmpty);
			}
			break;
		}
	}
	lzy_mutex_unlock(&threadPool.mutexParentQueue);
}



internal_func void *jobFunc(void *_)
{
	while (!threadPool.bShouldClose)
	{
		lzy_semaphore_wait(&threadPool.semFreeFull);
		lzy_mutex_lock(&threadPool.mutexFreeQueue);
		if (threadPool.bShouldClose)
			break;
		LCOREINFO("Executing Job!");
		LzyJob job = lzy_job_queue_deque(&threadPool.freeJobQueue);

		lzy_mutex_unlock(&threadPool.mutexFreeQueue);
		lzy_semaphore_signal(&threadPool.semFreeEmpty);
		job.fpJobFunc(&job, job.pArgs);
		lzy_job_system_finish_job(job.uParentID);
	}
	lzy_mutex_unlock(&threadPool.mutexFreeQueue);
	lzy_semaphore_signal(&threadPool.semFreeEmpty);
	return NULL;
}

b8 lzy_job_system_is_idle()
{
	return threadPool.freeJobQueue.uFront == threadPool.freeJobQueue.uRear && 
	threadPool.parentJobQueue.uFront == threadPool.parentJobQueue.uRear;
}

b8 lzy_job_system_init()
{
	lzy_memzero(&threadPool, sizeof(threadPool));

	if (!lzy_semaphore_init(&threadPool.semFreeFull, 0))
	{
		LCOREFATAL("Could not initialize semaphore");
		return false;
	}

	if (!lzy_semaphore_init(&threadPool.semFreeEmpty, LZY_JOB_QUEUE_SIZE))
	{
		LCOREFATAL("Could not initialize semaphore");
		return false;
	}

	if (!lzy_semaphore_init(&threadPool.semParentEmpty, LZY_JOB_QUEUE_SIZE))
	{
		LCOREFATAL("Could not initialize semaphore");
		return false;
	}

	if (!lzy_mutex_init(&threadPool.mutexFreeQueue))
	{
		LCOREFATAL("Could not initialize mutex");
		return false;
	}

	if (!lzy_mutex_init(&threadPool.mutexParentQueue))
	{
		LCOREFATAL("Could not initialize semaphore");
		return false;
	}

	threadPool.bShouldClose = false;


	for (u32 i = 0; i < LZY_THREAD_POOL_SIZE; i++)
	{
		lzy_thread_create(&threadPool.threads[i], jobFunc, NULL);
	}

	return true;
}

void lzy_job_system_shutdown()
{	
	threadPool.bShouldClose = true;
}


b8 lzy_job_system_enque_free_job(LzyJobFunc fpJobFunc, void* pArgs, u32 uParentID)
{
	LzyJob job = {.fpJobFunc = fpJobFunc, .pArgs = pArgs, .uParentID = uParentID};

	lzy_semaphore_wait(&threadPool.semFreeEmpty);
	lzy_mutex_lock(&threadPool.mutexFreeQueue);

	lzy_job_queue_enque(&threadPool.freeJobQueue, &job);

	lzy_mutex_unlock(&threadPool.mutexFreeQueue);
	lzy_semaphore_signal(&threadPool.semFreeFull);

	return true;
}

u32 lzy_job_system_enque_parent_job(LzyJobFunc fpJobFunc, void* pArgs, u32 uChildJobCount)
{
	static u32 uID = 0;
					
	LzyJob job = {.fpJobFunc = fpJobFunc, .pArgs = pArgs,.uJobID = uID, .uUnfinishedChildJobs = uChildJobCount, };
	uID = (uID + 1) % LZY_JOB_ID_INVALID;

	lzy_semaphore_wait(&threadPool.semParentEmpty);
	lzy_mutex_lock(&threadPool.mutexParentQueue);

	lzy_job_queue_enque(&threadPool.parentJobQueue, &job);

	lzy_mutex_unlock(&threadPool.mutexParentQueue);

	
	return job.uJobID;
}

