#include "LzyDefines.h"


typedef struct LzyVector
{
	u64 uCapacity;
	u64 uSize;
	u64 uStride;
	u8 pData[];
}LzyVector;


LAPI void* _lzy_vector_create(u64 uCapacity, u64 uStride);

LAPI void* _lzy_vector_init(LzyVector* pVector, u64 uCapacity, u64 uStride);

LAPI void* _lzy_vector_grow(LzyVector* pVector);
LAPI void* _lzy_vector_reserve(LzyVector* pVector, u64 uDesiredCapacity); 
LAPI void _lzy_vector_free(LzyVector* pVector);

#define lzy_vector_create(pVector, uCapacity) pVector = _lzy_vector_create(uCapacity, sizeof(*pVector))
#define lzy_vector_raw(pVector) ((LzyVector*)((u64*)(pVector) - 3))
#define lzy_vector_capacity(pVector)(*((u64*)(pVector) - 3)))
#define lzy_vector_size(pVector)(*((u64*)(pVector) - 2))
//At least (uCapacity * sizeof(*pVector) + sizeof(LzyVector)) bytes must be allocated at the address pointed to by pVector
#define lzy_vector_init(pVector, uCapacity) pVector = _lzy_vector_init(pVector, uCapacity, sizeof(*pVector))
#define lzy_vector_push_back(pVector, x) pVector = _lzy_vector_grow(lzy_vector_raw(pVector));\
										 pVector[lzy_vector_size(pVector)++] = x