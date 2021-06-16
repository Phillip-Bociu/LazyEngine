#pragma once
#include "LzyThread_linux.h"
#include "LzyThread_win32.h"
#include "LzyDefines.h"

LAPI b8 lzy_thread_create(LzyThread* pThread, void*(*fpRoutine)(void* pData), void* pArgs);

LAPI b8 lzy_mutex_init(LzyMutex* pMutex);
LAPI b8 lzy_mutex_lock(LzyMutex* pMutex);
LAPI b8 lzy_mutex_unlock(LzyMutex* pMutex);

LAPI b8 lzy_semaphore_init(LzySemaphore* pSemaphore, u64 uInitialValue);
LAPI b8 lzy_semaphore_signal(LzySemaphore* pSemaphore);
LAPI b8 lzy_semaphore_wait(LzySemaphore* pSemaphore);
LAPI b8 lzy_semaphore_value(LzySemaphore* pSemaphore, i32* pValue);
