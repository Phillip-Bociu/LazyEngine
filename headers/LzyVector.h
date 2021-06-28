#pragma once
#include "LzyDefines.h"


typedef struct LzyVector
{
	u64 uCapacity;
	u64 uSize;
	u64 uStride;
	u8 pData[];
}LzyVector;

LAPI void* _lzy_vector_create(u64 uInitialSize, u64 uStride);
LAPI void* _lzy_vector_grow_if_needed(LzyVector* pVector);
LAPI void* _lzy_vector_grow_one(LzyVector* pVector);
LAPI void* _lzy_vector_reserve(LzyVector* pVector, u64 uMinCap);
LAPI void _lzy_vector_free(LzyVector* pVector);

#define lzy_vector_create(pVec, uInitialSize) pVec = _lzy_vector_create(uInitialSize, sizeof(*pVec))

#define lzy_vector_raw(pVec) ((u64*)pVec - 3)
#define lzy_vector_capacity(pVec) (*((u64*)pVec - 3))
#define lzy_vector_size(pVec) (*((u64*)pVec - 2))
#define lzy_vector_back(pVec) (pVec[lzy_vector_size(pVec)])

#define lzy_vector_push(pVec, element) pVec = _lzy_vector_grow_if_needed((LzyVector*)lzy_vector_raw(pVec)); lzy_vector_back(pVec) = element
#define lzy_vector_emplace(pVec) pVec = _lzy_vector_grow_one((LzyVector*)lzy_vector_raw(pVec)); pVec[lzy_vector_size(pVec) - 1]
#define lzy_vector_pop(pVec) lzy_vector_size(pVec)--;

#define lzy_vector_resize(pVec, uNewSize) lzy_vector_size(pVec) = uNewSize; _lzy_vector_grow_if_needed((LzyVector*)lzy_vector_raw(pVec))
#define lzy_vector_reserve(pVec, uMinCap) pVec = _lzy_vector_reserve((LzyVector*)lzy_vector_raw(pVec), uMinCap);

#define lzy_vector_clear(pVec) lzy_vector_size(pVec) = 0
#define lzy_vector_free(pVec) _lzy_vector_free(pVec) 

