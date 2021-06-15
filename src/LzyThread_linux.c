#ifdef __linux__
#include "LzyThread.h"
#include "LzyLog.h"

b8 lzy_thread_init()
{

	return true;
}
void lzy_thread_shutdown()
{

}

b8 lzy_thread_create(LzyThread* pThread, void*(*fpRoutine)(void* pData), void* pArgs)
{
	if(pthread_create(pThread,NULL, fpRoutine, pArgs) != 0)
	{
		LCOREFATAL("Could not create thread");
		return false;
	}
	return true;
}

b8 lzy_mutex_init(LzyMutex* pMutex)
{
	i32 iErrorCode = pthread_mutex_init(pMutex, NULL); 
	if(iErrorCode != 0)
	{
		LCOREFATAL("Could not initialize mutex (error: %d)", iErrorCode);
		return false;
	}

	return true;
}
b8 lzy_mutex_lock(LzyMutex* pMutex)
{
	pthread_mutex_lock(pMutex);
	return true;
}
b8 lzy_mutex_unlock(LzyMutex* pMutex)
{
	pthread_mutex_unlock(pMutex);
	return true;
}


b8 lzy_semaphore_init(LzySemaphore* pSemaphore)
{
	i32 iErrorCode = sem_init(pSemaphore, 0, 0); 
	if(iErrorCode != 0)
	{
		LCOREFATAL("Could not initialize semaphore");
		return false;
	}

	return true;
}
b8 lzy_semaphore_signal(LzySemaphore* pSemaphore)
{
	return sem_post(pSemaphore) == 0;
}
b8 lzy_semaphore_wait(LzySemaphore* pSemaphore)
{
	return sem_wait(pSemaphore) == 0;
}
#endif