#pragma once
#ifdef __linux__
#include "LzyDefines.h"

typedef struct LzyFile
{
	i32 fd;
	i32 mode;
	b32 bIsBinary;
}LzyFile;

#endif