#pragma once
#include "LzyDefines.h"
#include "LzyFile.h"

//TODO create fobj format (metadata at start for no reallocations)
//A collection of LzyVector pointers

typedef struct LzyObjVertex
{
	f32 fPosX, fPosY, fPosZ;
	f32 fU, fV;
	f32 fNormalX, fNormalY, fNormalZ;
}LzyObjVertex;

typedef struct LzyObjContents
{
	LzyObjVertex* pVertices;
	u32* pIndices;
}LzyObjContents;


LAPI b8 lzy_obj_load_file(LzyFile file ,LzyObjContents* pContents);
LAPI void lzy_obj_free(LzyObjContents* pContents);