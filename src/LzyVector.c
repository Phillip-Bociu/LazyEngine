#include "LzyVector.h"
#include "LzyMemory.h"
#include <stdlib.h>

void* _lzy_vector_create(u64 uInitialSize, u64 uStride)
{
	LzyVector* pVector = lzy_alloc(sizeof(LzyVector) + uInitialSize * uStride, 8, LZY_MEMORY_TAG_VECTOR);

	if(!pVector)
	{
		LCOREERROR("Could not create vector");
		return NULL;
	}

	pVector->uCapacity = uInitialSize;
	pVector->uSize = uInitialSize;
	pVector->uStride = uStride;

	return pVector->pData;
}

void* _lzy_vector_grow_if_needed(LzyVector* pVector)
{
	if(pVector->uCapacity <= pVector->uSize)
	{
		u64 uNewCap = max(pVector->uSize, pVector->uCapacity * 2);
		LzyVector* pNewVec = lzy_realloc(pVector, pVector->uCapacity * pVector->uStride + sizeof(LzyVector), uNewCap * pVector->uStride + sizeof(LzyVector), 8, LZY_MEMORY_TAG_VECTOR);
		if(!pNewVec)
		{
			LCOREERROR("Could not realloc vector while growing");
			return NULL;
		}
		pNewVec->uCapacity = uNewCap;
		pVector = pNewVec;
	}
	return pVector->pData;
}
void* _lzy_vector_grow_one(LzyVector* pVector)
{
	pVector->uSize++;
	return _lzy_vector_grow_if_needed(pVector);
}
void* _lzy_vector_reserve(LzyVector* pVector, u64 uMinCap)
{
	if(pVector->uCapacity < uMinCap)
	{
		LzyVector* pNewVec = lzy_realloc(pVector, pVector->uCapacity * pVector->uStride + sizeof(LzyVector), uMinCap * pVector->uStride + sizeof(LzyVector), 8, LZY_MEMORY_TAG_VECTOR);
		if(!pNewVec)
		{
			LCOREERROR("Could not realloc vector while reserving");
			return NULL;
		}
		pNewVec->uCapacity = uMinCap;
		pVector = pNewVec;
	}

	return pVector->pData;
}

void _lzy_vector_free(LzyVector* pVector)
{
	lzy_free(pVector, pVector->uCapacity * pVector->uStride + sizeof(LzyVector), LZY_MEMORY_TAG_VECTOR);
}
