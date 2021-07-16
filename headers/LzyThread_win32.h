#pragma once
#ifdef _WIN32
#include "LzyDefines.h"
#include <Windows.h>

typedef HANDLE LzyThread;
typedef CRITICAL_SECTION LzyMutex;
typedef HANDLE LzySemaphore;


LAPI b8 lzy_thread_create(LzyThread* pThread, unsigned long(*fpRoutine)(void* pData), void* pArgs);

#endif
