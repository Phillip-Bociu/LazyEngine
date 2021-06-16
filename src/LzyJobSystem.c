#include "LzyJobSystem.h"
#include "LzyLog.h"
#include "LzyMemory.h"

#define LZY_THREAD_POOL_SIZE 256
#define LZY_JOB_QUEUE_SIZE 1024

typedef struct LzyJobQueue
{
	u32 uFront;
	u32 uRear;
	LzyJob queue[LZY_JOB_QUEUE_SIZE];
}LzyJobQueue;



typedef struct LzyThreadPool
{
	LzyMutex mutexQueue;
	LzySemaphore semFull;
	LzySemaphore semEmpty;
	LzyThread threads[LZY_THREAD_POOL_SIZE];
	LzyJobQueue jobQueue;
	b8 bShouldClose;
} LzyThreadPool;

global LzyThreadPool threadPool;


internal_func void lzy_job_queue_enque(LzyJobQueue* pQueue, LzyJob* pJob)
{
	pQueue->queue[pQueue->uRear] = *pJob;
	pQueue->uRear = (pQueue->uRear + 1) % LZY_JOB_QUEUE_SIZE;
}

internal_func LzyJob lzy_job_queue_deque(LzyJobQueue* pQueue)
{
	LzyJob retval = pQueue->queue[pQueue->uFront];
	pQueue->uFront = (pQueue->uFront + 1) % LZY_JOB_QUEUE_SIZE;
	sem_
	return retval;
}


internal_func void *jobFunc(void *_)
{
	while (!threadPool.bShouldClose)
	{
		lzy_semaphore_wait(&threadPool.semFull);
		lzy_mutex_lock(&threadPool.mutexQueue);
		if (threadPool.bShouldClose)
			break;

		LzyJob job = lzy_job_queue_deque(&threadPool.jobQueue);

		lzy_mutex_unlock(&threadPool.mutexQueue);
		lzy_semaphore_signal(&threadPool.semEmpty);
		job.fpJob(job.pArgs);
	}
	lzy_mutex_unlock(&threadPool.mutexQueue);
	lzy_semaphore_signal(&threadPool.semEmpty);
	return NULL;
}

b8 lzy_job_system_is_idle()
{
	i32 iVal;
	lzy_semaphore_value(&threadPool.semFull, &iVal);
	return iVal == 0;
}

b8 lzy_job_system_init()
{
	lzy_memzero(&threadPool, sizeof(threadPool));

	if (!lzy_semaphore_init(&threadPool.semFull, 0))
	{
		LCOREFATAL("Could not initialize semaphore");
		return false;
	}

	if (!lzy_semaphore_init(&threadPool.semEmpty, LZY_JOB_QUEUE_SIZE))
	{
		LCOREFATAL("Could not initialize semaphore");
		return false;
	}

	if (!lzy_mutex_init(&threadPool.mutexQueue))
	{
		LCOREFATAL("Could not initialize semaphore");
		return false;
	}

	for (u32 i = 0; i < LZY_THREAD_POOL_SIZE; i++)
	{
		lzy_thread_create(&threadPool.threads[i], jobFunc, NULL);
	}
}

b8 lzy_job_system_enque(LzyJob* pJob)
{
	lzy_semaphore_wait(&threadPool.semEmpty);
	lzy_mutex_lock(&threadPool.mutexQueue);

	lzy_job_queue_enque(&threadPool.jobQueue, pJob);

	lzy_mutex_unlock(&threadPool.mutexQueue);
	lzy_semaphore_signal(&threadPool.semFull);

	return true;
}