#ifdef _WIN32
#include "LzyFile.h"
#include "LzyMemory.h"


b8 lzy_file_open(LzyFile* pFile, const char* pFileName, LzyFileMode fileMode)
{
	DWORD dwDesiredAccess = 0;
	dwDesiredAccess |= GENERIC_READ * ((fileMode & LZY_FILE_MODE_READ) != 0);
	dwDesiredAccess |= GENERIC_WRITE * ((fileMode & LZY_FILE_MODE_WRITE) != 0);

	DWORD dwCreationDisposition = 0;

	dwCreationDisposition |= (CREATE_NEW * ((fileMode & LZY_FILE_MODE_CREATE) != 0));
	dwCreationDisposition |= (OPEN_EXISTING * ((fileMode & LZY_FILE_MODE_CREATE) == 0));
	
	DWORD dwFlagsAndAttributes = 0;
	dwFlagsAndAttributes |= (FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE) * ((fileMode & LZY_FILE_MODE_TEMPORARY) != 0);
	dwFlagsAndAttributes |= FILE_ATTRIBUTE_NORMAL * ((fileMode & LZY_FILE_MODE_TEMPORARY) == 0);

	pFile->hFile = CreateFileA(
		pFileName, 
		dwDesiredAccess,
		FILE_SHARE_READ  |
		FILE_SHARE_WRITE |
		FILE_SHARE_DELETE,
		NULL,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		NULL);

	if (pFile->hFile == INVALID_HANDLE_VALUE)
	{
		LCOREERROR("Could not open file %s", pFileName);
		return false;
	}

	pFile->bIsBinary = (fileMode & LZY_FILE_MODE_BINARY) != 0;
	return true;
}

b8 lzy_file_close(LzyFile file)
{
	return CloseHandle(file.hFile) != 0;
}
//TODO
void* lzy_file_map(LzyFile file, u64* pSize)
{
	return NULL;
}
b8 lzy_file_unmap(LzyFile file, void* pMap)
{
	return true;
}
b8 lzy_file_write(LzyFile file, void* pSrc, u64 uSize)
{
	DWORD dwBytesWritten = 0;
	return WriteFile(file.hFile, pSrc, uSize, &dwBytesWritten, NULL);
}
b8 lzy_file_read(LzyFile file, void* pDest, u64 uSize)
{
	DWORD dwBytesRead = 0;
	return ReadFile(file.hFile, pDest, uSize, &dwBytesRead, NULL);
}
b8 lzy_file_get_size(LzyFile file, u64* pSize)
{
	LARGE_INTEGER lint = { 0 };
	b8 retval = GetFileSizeEx(file.hFile, &lint) != 0;
	*pSize = lint.QuadPart;
	return retval;
}


#endif