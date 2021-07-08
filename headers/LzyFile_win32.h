#pragma once
#ifdef _WIN32

#include <Windows.h>
#include "LzyDefines.h"

typedef struct LzyFile
{
	HANDLE hFile;
	b8 bIsBinary;
}LzyFile;

#endif