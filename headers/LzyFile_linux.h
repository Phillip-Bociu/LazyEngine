#pragma once
#ifdef __linux__
#include "LzyDefines.h"

typedef struct LzyFile
{
	i32 fd;
	i32 mode;
	b32 bIsBinary;
}LzyFile;


LAPI b8 lzy_thread_create(LzyThread* pThread, void*(*fpRoutine)(void* pData), void* pArgs);

#endif