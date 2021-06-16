#pragma once
#include "LzyDefines.h"
#ifdef _WIN32
#include <Windows.h>

typedef HANDLE LzyThread;
typedef CRITICAL_SECTION LzyMutex;
typedef CONDITION_VARIABLE LzySemaphore;

#endif
