#include "LzyDefines.h"


typedef struct LzyVector
{
	u64 uCapacity;
	u64 uSize;
	u64 uStride;
	u8 pData[];
}LzyVector;


LAPI void* _lzy_vector_create(u64 uCapacity, u64 uStride);

LAPI void _lzy_vector_init(LzyVector* pVector, u64 uCapacity, u64 uStride);

LAPI void* _lzy_vector_emplace_back(LzyVector* pVector, u64 uElementCount);
LAPI void* _lzy_vector_push_back(LzyVector* pVector, u64 uElementCount, void* pSrc);
LAPI void lzy_vector_pop_back(LzyVector* pVector, u64 uElementCount);
LAPI void* _lzy_vector_reserve(LzyVector* pVector, u64 uDesiredCapacity); 
LAPI void* _lzy_vector_resize(LzyVector* pVector, u64 uNewSize);
LAPI void lzy_vector_free(LzyVector* pVector);

#define lzy_vector_create(pVector, uCapacity) pVector = _lzy_vector_create(uCapacity, sizeof(*pVector))
//At least (uCapacity * sizeof(*pVector) + sizeof(LzyVector)) bytes must be allocated at the address pointed to by pVector
#define lzy_vector_init(pVector, uCapacity) _lzy_vector_init((LzyVector*)pVector, uCapacity, sizeof(*pVector))
#define lzy_vector_emplace_back(pVector, uElementCount) pVector = _lzy_vector_emplace_back((LzyVector*)pVector, uElementCount);