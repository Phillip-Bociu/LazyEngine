#ifdef __linux__
#include "LzyFile.h"
#include "LzyLog.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

//TODO: replace mmap with lzy_alloc(..,.. LZY_MEMORY_TAG_FILE_MAPPING)


internal_func i32 lzyfilemode_to_posix_filemode(LzyFileMode fileMode)
{
	i32 retval = 0;
	retval |= O_APPEND * ((fileMode & LZY_FILE_MODE_APPEND) != 0);
	retval |= O_RDWR * ((fileMode & (LZY_FILE_MODE_READ | LZY_FILE_MODE_WRITE)) == (LZY_FILE_MODE_READ | LZY_FILE_MODE_WRITE));
	retval |= O_RDONLY * ((fileMode & (LZY_FILE_MODE_READ | LZY_FILE_MODE_WRITE)) == LZY_FILE_MODE_READ);
	retval |= O_WRONLY * ((fileMode & (LZY_FILE_MODE_READ | LZY_FILE_MODE_WRITE)) == LZY_FILE_MODE_WRITE);
	retval |= O_CREAT * ((fileMode & LZY_FILE_MODE_CREATE) != 0);
	retval |= __O_TMPFILE * ((fileMode & LZY_FILE_MODE_TEMPORARY) != 0);

	return retval;
}

b8 lzy_file_open(LzyFile* pFile, const char* pFilePath, LzyFileMode fileMode)
{
	LASSERT(pFile != NULL, "pointer to file handle is NULL");

	pFile->mode = lzyfilemode_to_posix_filemode(fileMode);
	pFile->fd = open(pFilePath, pFile->mode);
	if(pFile->fd == -1)
	{
		LCOREERROR("Could not open file at path %s", pFilePath);
		return false;
	}
	pFile->bIsBinary = (fileMode & LZY_FILE_MODE_BINARY) != 0;
	return true;
}

b8 lzy_file_close(LzyFile file)
{
	return close(file.fd) == 0;
}

void* lzy_file_map(LzyFile file, u64* pSize)
{
	struct stat st;
	if(fstat(file.fd, &st) == -1)
	{
		LCOREERROR("Could not get file size while mapping");
		return MAP_FAILED;
	}
	
	i32 iMapProt = 0;
	iMapProt |= PROT_READ * ((file.mode & O_WRONLY) == 0);
	iMapProt |= PROT_WRITE * ((file.mode & (O_WRONLY | O_RDONLY)) != 0);

	void* retval = mmap(NULL, st.st_size, iMapProt, MAP_SHARED, file.fd, 0);

	if(retval == MAP_FAILED)
		return MAP_FAILED;

	if(pSize)
		*pSize = st.st_size;

	return retval;
}

b8 lzy_file_unmap(LzyFile file, void* pMap)
{
	struct stat st;
	if(fstat(file.fd, &st) == -1)
	{
		LCOREERROR("Could not get file size when unmapping");
		return false;
	}

	return munmap(pMap, st.st_size) == 0;
}

b8 lzy_file_write(LzyFile file, void* pSrc, u64 uSize)
{
	return write(file.fd, pSrc, uSize) != -1;
}

b8 lzy_file_read(LzyFile file, void* pDest, u64 uSize)
{
	return read(file.fd, pDest, uSize) != -1;
}

b8 lzy_file_get_size(LzyFile file, u64* pSize)
{
	struct stat st;
	if(fstat(file.fd, &st) == -1)
	{
		LCOREERROR("Could not get file size");
		return false;
	}
	
	if(!pSize)
		return false;

	*pSize = st.st_size;
	return true;
}

#endif