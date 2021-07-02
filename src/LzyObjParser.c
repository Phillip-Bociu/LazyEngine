#include "LzyObjParser.h"
#include "LzyVector.h"
#include "LzyMemory.h"
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <immintrin.h>
#include "deps/xxhash/xxhash.h"

#define LZY_HASH_SEED (4789200987532141674ull)

typedef struct LzyObjKeyValue
{
	u64 uKey[3];
	u64 uVertexIndex;
}LzyObjKeyValue;

typedef union Packed16
{
	__m128i xmm;
	i8 bytes[16];
}Packed16;


typedef struct LzyObjHashMap
{
	u64 uSize;
	u64 uCapacity;
	Packed16* pMeta;
	LzyObjKeyValue* pMap;
}LzyObjHashMap;

internal_func u64 lzy_obj_hash_map_hash_func(u64 uKey[3])
{
	return XXH64(uKey, sizeof(uKey[0]) * 3, LZY_HASH_SEED);
}

internal_func b8 lzy_obj_hash_map_create(LzyObjHashMap* pHashMap, u64 uInitialCapacity)
{
	for(u64 uPow = 4; uPow < 64; uPow++)
	{
		if(uInitialCapacity <= (1ull << uPow))
		{
			uInitialCapacity = 1ull << uPow;
			break;
		}
	}

	pHashMap->uSize = 0;
	pHashMap->uCapacity = uInitialCapacity;
	pHashMap->pMeta = lzy_alloc(uInitialCapacity, 16, LZY_MEMORY_TAG_VECTOR);
	lzy_memset(pHashMap->pMeta, 255, uInitialCapacity);
	pHashMap->pMap = lzy_alloc(uInitialCapacity * sizeof(LzyObjKeyValue), 8, LZY_MEMORY_TAG_VECTOR);

	if(!pHashMap->pMeta || !pHashMap->pMap)
	{			
		LCOREERROR("Could not create obj hash map");
		return false;
	}
	return true;
}

internal_func u64 lzy_obj_hash_map_find(LzyObjHashMap* pHashMap, u64 uKey[3])
{
	u64 uHash = lzy_obj_hash_map_hash_func(uKey);
	u64 uIndex = (uHash & (pHashMap->uCapacity - 1)) / 16;
	i8 iMetaHash = uHash & 0b1111111;
	__m128i cmp = _mm_set1_epi8(iMetaHash);
	
	u64 uRetval = ~0ull;
	do
	{
		__m128i cmpres = _mm_cmpeq_epi8(pHashMap->pMeta[uIndex].xmm, cmp);
		i32 iCmpRes = _mm_movemask_epi8(cmpres);
		if(iCmpRes != 0)
		{
			uRetval = uIndex * 16 + __builtin_ffs(iCmpRes) - 1;
			break;
		} else
		{
			return (~0ull);
		}
	}while(true);

	do
	{
		if(pHashMap->pMap[uRetval].uKey[0] == uKey[0] && 
		   pHashMap->pMap[uRetval].uKey[1] == uKey[1] &&
		   pHashMap->pMap[uRetval].uKey[2] == uKey[2])
		   break;

		if(pHashMap->pMeta[uRetval / 16].bytes[uRetval & 15] == -1)
			return ~0ull;
		uRetval++;
	}while(true);
	return uRetval;
}

internal_func b8 lzy_obj_hash_map_insert(LzyObjHashMap* pHashMap, const u64 uKey[3], u64 uVertexIndex);

internal_func void lzy_obj_hash_map_grow(LzyObjHashMap* pHashMap)
{
	Packed16* pOldMeta = pHashMap->pMeta; 	  
	LzyObjKeyValue* pOldMap = pHashMap->pMap;
	u64 uOldCap = pHashMap->uCapacity;

	pHashMap->pMeta = lzy_alloc(pHashMap->uCapacity * 2, 16, LZY_MEMORY_TAG_VECTOR);
	lzy_memset(pHashMap->pMeta, 255, pHashMap->uCapacity * 2);
	pHashMap->pMap  = lzy_alloc(pHashMap->uCapacity * 2 * sizeof(LzyObjKeyValue), 8, LZY_MEMORY_TAG_VECTOR);
	pHashMap->uCapacity *= 2;

	for(u64 i = 0; i < 1 + pHashMap->uCapacity / 16; i++)
	{
		__m128i res = _mm_set1_epi8(-1);
		res = _mm_cmpeq_epi8(res, pOldMeta[i].xmm);
		i32 iRes =_mm_movemask_epi8(res); 
		if(iRes != -1)
		{
			for(i32 j = __builtin_ffs(iRes) - 1; j < 16; j++)
			{
				if(pOldMeta[i].bytes[j] != -1)
				{
					lzy_obj_hash_map_insert(pHashMap, pOldMap[i * 16 + j].uKey, pOldMap[i * 16 + j].uVertexIndex);
				}
			}	
		}
	}
	
	lzy_free(pOldMeta, pHashMap->uCapacity, LZY_MEMORY_TAG_VECTOR);
	lzy_free(pOldMap, pHashMap->uCapacity * sizeof(LzyObjKeyValue), LZY_MEMORY_TAG_VECTOR);
}

internal_func b8 lzy_obj_hash_map_insert(LzyObjHashMap* pHashMap, const u64 uKey[3], u64 uVertexIndex)
{
	
	if(((f64)pHashMap->uSize + 1.0) / (f64)pHashMap->uCapacity >= 0.875)
	{
		lzy_obj_hash_map_grow(pHashMap);
	}
	
	const u64 uHash = lzy_obj_hash_map_hash_func(uKey);
	u64 uIndex = (uHash & (pHashMap->uCapacity - 1)) / 16;

	__m128i cmp = _mm_set1_epi8(-1);
	while(true)
	{
		__m128i cmpres = _mm_cmpeq_epi8(cmp, pHashMap->pMeta[uIndex].xmm);
		i32 iRes = _mm_movemask_epi8(cmpres);
		if(iRes != 0)
		{
			uIndex = uIndex * 16 + __builtin_ffs(iRes) - 1;
			break;
		}
		uIndex++;
		if(uIndex == 1 + pHashMap->uCapacity / 16)
		{
			return false;
		}
	}

	pHashMap->pMeta[uIndex / 16].bytes[uIndex & 15] = uHash & 0b1111111ull;
	pHashMap->pMap[uIndex].uKey[0] = uKey[0];
	pHashMap->pMap[uIndex].uKey[1] = uKey[1];
	pHashMap->pMap[uIndex].uKey[2] = uKey[2];
	pHashMap->pMap[uIndex].uVertexIndex = uVertexIndex;
	pHashMap->uSize++;

	return true;
}


internal_func b8 lzy_char_is_endofline(c8 c)
{
	return c == '\r' || c == '\n' || c == '\t';
}

internal_func c8 *lzy_parse_triangle(c8 *pIter, LzyObjHashMap* pHashMap, LzyObjVertex *pVertices,u32* pIndices, const f32 *pPositions, const f32 *pTextureCoords, const f32 *pNormals)
{

	u64 uKey[3];

	u64 v1 = strtouq(pIter, &pIter, 10) - 1;
	u64 vt1 = strtouq(pIter + 1, &pIter, 10);
	if (vt1 != 0)
		vt1--;
	u64 vn1 = strtouq(pIter + 1, &pIter, 10) - 1;
	uKey[0] =  v1;
	uKey[1] = vt1;
	uKey[2] = vn1;

	u64 uIndex = lzy_obj_hash_map_find(pHashMap, uKey); 
	if(uIndex != ~0ull)
	{
		lzy_vector_emplace(pIndices) = uIndex;
	} else
	{
		lzy_vector_emplace(pIndices) = lzy_vector_size(pVertices);
		lzy_vector_emplace(pVertices);

		lzy_vector_back(pVertices).fPosX = pPositions[v1 * 3 + 0];
		lzy_vector_back(pVertices).fPosY = pPositions[v1 * 3 + 1];
		lzy_vector_back(pVertices).fPosZ = pPositions[v1 * 3 + 2];

		lzy_vector_back(pVertices).fU = pTextureCoords[vt1 * 2 + 0];
		lzy_vector_back(pVertices).fV = pTextureCoords[vt1 * 2 + 1];

		lzy_vector_back(pVertices).fNormalX = pNormals[vn1 * 3 + 0];
		lzy_vector_back(pVertices).fNormalY = pNormals[vn1 * 3 + 1];
		lzy_vector_back(pVertices).fNormalZ = pNormals[vn1 * 3 + 2];
	}

	u64 v2 = strtouq(pIter + 1, &pIter, 10) - 1;
	u64 vt2 = strtouq(pIter + 1, &pIter, 10) - 1;
	if (vt2 != 0)
		vt2--;
	u64 vn2 = strtouq(pIter + 1, &pIter, 10) - 1;

	uKey[0] =  v2;
	uKey[1] = vt2;
	uKey[2] = vn2;

	uIndex = lzy_obj_hash_map_find(pHashMap, uKey); 
	if(uIndex != ~0ull)
	{
		lzy_vector_emplace(pIndices) = uIndex;
	} else
	{
		lzy_vector_emplace(pIndices) = lzy_vector_size(pVertices);
		lzy_vector_emplace(pVertices);

		lzy_vector_back(pVertices).fPosX = pPositions[v2 * 3 + 0];
		lzy_vector_back(pVertices).fPosY = pPositions[v2 * 3 + 1];
		lzy_vector_back(pVertices).fPosZ = pPositions[v2 * 3 + 2];

		lzy_vector_back(pVertices).fU = pTextureCoords[vt2 * 2 + 0];
		lzy_vector_back(pVertices).fV = pTextureCoords[vt2 * 2 + 1];

		lzy_vector_back(pVertices).fNormalX = pNormals[vn2 * 3 + 0];
		lzy_vector_back(pVertices).fNormalY = pNormals[vn2 * 3 + 1];
		lzy_vector_back(pVertices).fNormalZ = pNormals[vn2 * 3 + 2];
	}


	u64 v3 = strtouq(pIter + 1, &pIter, 10) - 1;
	u64 vt3 = strtouq(pIter + 1, &pIter, 10) - 1;
	if (vt3 != 0)
		vt3--;

	u64 vn3 = strtouq(pIter + 1, &pIter, 10) - 1;

	uKey[0] =  v3;
	uKey[1] = vt3;
	uKey[2] = vn3;
	
	uIndex = lzy_obj_hash_map_find(pHashMap, uKey); 
	if(uIndex != ~0ull)
	{
		lzy_vector_emplace(pIndices) = uIndex;
	} else
	{
		lzy_vector_emplace(pIndices) = lzy_vector_size(pVertices);
		lzy_vector_emplace(pVertices);

		lzy_vector_back(pVertices).fPosX = pPositions[v3 * 3 + 0];
		lzy_vector_back(pVertices).fPosY = pPositions[v3 * 3 + 1];
		lzy_vector_back(pVertices).fPosZ = pPositions[v3 * 3 + 2];

		lzy_vector_back(pVertices).fU = pTextureCoords[vt3 * 2 + 0];
		lzy_vector_back(pVertices).fV = pTextureCoords[vt3 * 2 + 1];

		lzy_vector_back(pVertices).fNormalX = pNormals[vn3 * 3 + 0];
		lzy_vector_back(pVertices).fNormalY = pNormals[vn3 * 3 + 1];
		lzy_vector_back(pVertices).fNormalZ = pNormals[vn3 * 3 + 2];
	}

	return pIter;
}

internal_func c8 *lzy_parse_position(const c8 *pIter, f32 *pPositions)
{
	lzy_vector_emplace(pPositions) = strtof(pIter, &pIter);
	lzy_vector_emplace(pPositions) = strtof(pIter + 1, &pIter);
	lzy_vector_emplace(pPositions) = strtof(pIter + 1, &pIter);

	return pIter;
}

internal_func c8 *lzy_parse_texture_coords(const c8 *pIter, f32 *pTextureCoords)
{
	lzy_vector_emplace(pTextureCoords) = strtof(pIter, &pIter);
	lzy_vector_emplace(pTextureCoords) = strtof(pIter + 1, &pIter);

	return pIter;
}

internal_func c8 *lzy_parse_normals(const c8 *pIter, f32 *pNormals)
{
	lzy_vector_emplace(pNormals) = strtof(pIter, &pIter);
	lzy_vector_emplace(pNormals) = strtof(pIter + 1, &pIter);
	lzy_vector_emplace(pNormals) = strtof(pIter + 1, &pIter);

	return pIter;
}

internal_func c8 *lzy_next_line(const c8 *pIter, const c8* const pEnd)
{
	while (*pIter != NULL)
		pIter++;

	while (*pIter == NULL && pIter != pEnd)
		pIter++;

	return pIter;
}

b8 lzy_obj_load_file(LzyFile file, LzyObjContents *pContents)
{


	f32 *pPositions;
	f32 *pTextureCoords;
	f32 *pNormals;

	lzy_vector_create(pPositions, 0);
	lzy_vector_reserve(pPositions, 128);

	lzy_vector_create(pTextureCoords, 0);
	lzy_vector_reserve(pTextureCoords, 128);

	lzy_vector_create(pNormals, 0);
	lzy_vector_reserve(pNormals, 128);

	lzy_vector_create(pContents->pVertices, 0);
	lzy_vector_reserve(pContents->pVertices, 512);

	lzy_vector_create(pContents->pIndices, 0);
	lzy_vector_reserve(pContents->pIndices, 1024);

	if (!pPositions || !pTextureCoords || !pNormals || !pContents->pVertices)
	{
		LCOREERROR("Could not create vectors for .obj parsing");
		return false;
	}

	u64 uFileSize;

	if (!lzy_file_get_size(file, &uFileSize))
	{
		return false;
	}
 
	c8 *pFileContents = lzy_alloc(uFileSize + 1, 1, LZY_MEMORY_TAG_STRING);
	lzy_file_read(file, pFileContents, uFileSize);
	pFileContents[uFileSize] = NULL;

	c8* pSave;
	c8 *pIter = strtok_r(pFileContents, "\r\n", &pSave);

	const c8 *const pEnd = pFileContents + uFileSize + 1;

	u32 line = 1;

	for (; pIter != NULL; line++, pIter = strtok_r(NULL, "\r\n", &pSave))
	{
		

		if(lzy_vector_size(pPositions) == 126)
		{
			LINFO("Crash?");
		}

		if(line == 44)
		{
			LINFO("Gonna break");
		}
		if (*pIter == 'v')
		{
			switch (*(pIter + 1))
			{
			case ' ':
			{
				LTRACE("Pos Size:%llu", lzy_vector_size(pPositions));
				LTRACE("Pos Cap :%llu", lzy_vector_capacity(pPositions));

				if(lzy_vector_capacity(pPositions) - lzy_vector_size(pPositions) <= 3)
					lzy_vector_reserve(pPositions, lzy_vector_capacity(pPositions) * 2);
				pIter = lzy_parse_position(pIter + 2, pPositions);
			}
			break;

			case 't':
			{
				LTRACE("Tex Size:%llu", lzy_vector_size    (pTextureCoords));
				LTRACE("Tex Cap :%llu", lzy_vector_capacity(pTextureCoords));

				if(lzy_vector_capacity(pTextureCoords) - lzy_vector_size(pTextureCoords) <= 3)
					lzy_vector_reserve(pTextureCoords, lzy_vector_capacity(pTextureCoords) * 2);
				pIter = lzy_parse_texture_coords(pIter + 3, pTextureCoords);
			}
			break;

			case 'n':
			{
				LTRACE("Normals Size:%llu", lzy_vector_size	   (pNormals));
				LTRACE("Normals Cap :%llu", lzy_vector_capacity(pNormals));

				if(lzy_vector_capacity(pNormals) - lzy_vector_size(pNormals) <= 3)
					lzy_vector_reserve(pNormals, lzy_vector_capacity(pNormals) * 2);
				pIter = lzy_parse_normals(pIter + 3, pNormals);
			}
			break;
			}
		}
		else if (*pIter == 'f')
		{
			break;
		}
	}

	LzyObjHashMap hashMap;

	lzy_obj_hash_map_create(&hashMap, lzy_vector_size(pPositions) + lzy_vector_size(pPositions) / 2);

	for (; pIter != NULL; pIter = strtok_r(NULL, "\r\n", &pSave))
	{
		if (*pIter == 'f')
		{
			if(lzy_vector_capacity(pContents->pVertices) - lzy_vector_size   (pContents->pVertices) <= 3)
				lzy_vector_reserve(pContents->pVertices, lzy_vector_capacity (pContents->pVertices) * 2);

			if(lzy_vector_capacity(pContents->pIndices) - lzy_vector_size    (pContents->pIndices) <= 3)
				lzy_vector_reserve(pContents->pIndices,   lzy_vector_capacity(pContents->pIndices) * 2);
			pIter = lzy_parse_triangle(pIter + 2, &hashMap, pContents->pVertices, pContents->pIndices, pPositions, pTextureCoords, pNormals);
		}
	}

	lzy_free(pFileContents, uFileSize + 1, LZY_MEMORY_TAG_STRING);
	lzy_vector_free(pNormals);
	lzy_vector_free(pTextureCoords);
	lzy_vector_free(pPositions);
	lzy_free(hashMap.pMeta, hashMap.uCapacity, LZY_MEMORY_TAG_VECTOR);
	lzy_free(hashMap.pMap, hashMap.uCapacity * sizeof(LzyObjKeyValue), LZY_MEMORY_TAG_VECTOR);
	return true;
}

void lzy_obj_free(LzyObjContents* pContents)
{
	lzy_vector_free(pContents->pIndices);
	lzy_vector_free(pContents->pVertices);
}