#pragma once
#include "LzyFile_linux.h"
#include "LzyDefines.h"

typedef enum LzyFileMode
{
	LZY_FILE_MODE_READ = 1 << 0,
	LZY_FILE_MODE_WRITE = 1 << 1,
	LZY_FILE_MODE_BINARY = 1 << 2,
	LZY_FILE_MODE_APPEND = 1 << 3,
	LZY_FILE_MODE_CREATE = 1 << 4,
	LZY_FILE_MODE_TEMPORARY = 1 << 5,
	LZY_FILE_MODE_MAX
}LzyFileMode;

LAPI b8 lzy_file_open(LzyFile* pFile, const char* pFileName, LzyFileMode fileMode);
LAPI b8 lzy_file_close(LzyFile file);
LAPI void* lzy_file_map(LzyFile file, u64* pSize);
LAPI b8 lzy_file_unmap(LzyFile file, void* pMap);
LAPI b8 lzy_file_write(LzyFile file, void* pSrc, u64 uSize);
LAPI b8 lzy_file_read(LzyFile file, void* pDest, u64 uSize);
LAPI b8 lzy_file_get_size(LzyFile file, u64* pSize);
