#include "LzyThread_linux.h"
#include "LzyThread_win32.h"
#include "LzyDefines.h"

b8 lzy_thread_init();
void lzy_thread_shutdown();


LAPI b8 lzy_thread_create(LzyThread* pThread, void*(*fpRoutine)(void* pData), void* pArgs);

LAPI b8 lzy_mutex_init(LzyMutex* pMutex);
LAPI b8 lzy_mutex_lock(LzyMutex* pMutex);
LAPI b8 lzy_mutex_unlock(LzyMutex* pMutex);

LAPI b8 lzy_semaphore_init(LzySemaphore* pSemaphore);
LAPI b8 lzy_semaphore_signal(LzySemaphore* pSemaphore);
LAPI b8 lzy_semaphore_wait(LzySemaphore* pSemaphore);

