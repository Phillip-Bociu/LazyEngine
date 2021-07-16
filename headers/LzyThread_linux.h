#pragma once
#ifdef __linux__
#include <pthread.h>
#include <semaphore.h>

typedef pthread_t LzyThread; 
typedef pthread_mutex_t LzyMutex;
typedef sem_t LzySemaphore;

LAPI b8 lzy_thread_create(LzyThread* pThread, void*(*fpRoutine)(void* pData), void* pArgs);

#endif