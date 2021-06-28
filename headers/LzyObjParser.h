#pragma once
#include "LzyDefines.h"
#include "LzyFile.h"

//TODO create fobj format (metadata at start for no reallocations)
//A collection of LzyVector pointers
typedef struct LzyObjContents
{
	f32* pPositions;
	f32* pTextureCoords;
	f32* pNormals;
}LzyObjContents;


LAPI b8 lzy_obj_load_file(LzyFile file ,LzyObjContents* pContents);
LAPI void lzy_obj_free(LzyObjContents* pContents);