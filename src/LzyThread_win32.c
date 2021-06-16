#ifdef _WIN32
#include "LzyThread.h"

b8 lzy_thread_create(LzyThread* pThread, void* (*fpRoutine)(void* pData), void* pArgs)
{
	*pThread = CreateThread(NULL, 0, fpRoutine, pArgs, 0, NULL);
}
b8 lzy_mutex_init(LzyMutex* pMutex)
{
	InitializeCriticalSection(pMutex);
	return true;
}
b8 lzy_mutex_lock(LzyMutex* pMutex)
{
	EnterCriticalSection(pMutex);
	return true;
}
b8 lzy_mutex_unlock(LzyMutex* pMutex)
{
	LeaveCriticalSection(pMutex);
	return true;
}

b8 lzy_semaphore_init(LzySemaphore* pSemaphore)
{
	InitializeConditionVariable(pSemaphore);
}
b8 lzy_semaphore_signal(LzySemaphore* pSemaphore)
{
	WakeConditionVariable(pSemaphore);
	return true;
}
b8 lzy_semaphore_wait(LzySemaphore* pSemaphore, LzyMutex* pMutex)
{
	SleepConditionVariableCS(pSemaphore, pMutex, INFINITE);
	return true;
}

#endif