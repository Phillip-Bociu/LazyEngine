#pragma once
#ifdef _WIN32
#include "LzyDefines.h"
#include <Windows.h>

typedef HANDLE LzyThread;
typedef CRITICAL_SECTION LzyMutex;
typedef HANDLE LzySemaphore;

#endif
