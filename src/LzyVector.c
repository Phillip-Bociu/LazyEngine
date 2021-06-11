#include "LzyVector.h"
#include "LzyMemory.h"
#include <stdlib.h>

void* _lzy_vector_create(u64 uCapacity, u64 uStride)
{
	LzyVector* pVector = lzy_alloc(sizeof(LzyVector) + uCapacity * uStride, 8, LZY_MEMORY_TAG_VECTOR);
	pVector->uCapacity = uCapacity;
	pVector->uStride = uStride;
	return pVector->pData;
}

void* _lzy_vector_init(LzyVector* pVector, u64 uCapacity, u64 uStride)
{
	pVector->uCapacity = uCapacity;
	pVector->uStride = uStride;
	pVector->uSize = 0;
	return pVector->pData;
}

void* _lzy_vector_emplace_back(LzyVector* pVector, u64 uElementCount)
{
	void* retval = &pVector->pData[pVector->uStride * (pVector->uSize)];
	pVector->uSize += uElementCount;
	return retval;
}

void* _lzy_vector_grow(LzyVector* pVector)
{
	if (pVector->uSize == pVector->uCapacity)
	{
		LzyVector* pNewVector = lzy_alloc((pVector->uCapacity << 1) * pVector->uStride + sizeof(LzyVector), 8, LZY_MEMORY_TAG_VECTOR);
		lzy_memcpy(pNewVector, pVector, pVector->uSize * pVector->uStride + sizeof(LzyVector));
		lzy_free(pVector, pVector->uCapacity * pVector->uStride + sizeof(LzyVector), LZY_MEMORY_TAG_VECTOR);
		return pNewVector->pData;
	}
	else
	{
		return pVector->pData;
	}
}

void* _lzy_vector_reserve(LzyVector* pVector, u64 uDesiredCapacity)
{
	if (pVector->uCapacity < uDesiredCapacity)
	{
		LzyVector* pNewVector = lzy_alloc(uDesiredCapacity * pVector->uStride + sizeof(LzyVector), 8, LZY_MEMORY_TAG_VECTOR);
		lzy_memcpy(pNewVector, pVector, pVector->uSize * pVector->uStride + sizeof(LzyVector));
		lzy_free(pVector, pVector->uCapacity * pVector->uStride + sizeof(LzyVector), LZY_MEMORY_TAG_VECTOR);
		pVector = pNewVector;
	}
	return pVector;
}

void _lzy_vector_free(LzyVector* pVector)
{
	lzy_free(pVector, pVector->uCapacity * pVector->uStride + sizeof(LzyVector), LZY_MEMORY_TAG_VECTOR);
}

