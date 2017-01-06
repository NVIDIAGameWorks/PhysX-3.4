// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.

#include "SnConvX.h"
#include <assert.h>

using namespace physx;

void Sn::ConvX::resetUnions()
{
	mUnions.clear();
}

bool Sn::ConvX::registerUnion(const char* name)
{
	displayMessage(PxErrorCode::eDEBUG_INFO, "Registering union: %s\n", name);

	Sn::Union u;
	u.mName	= name;

	mUnions.pushBack(u);
	return true;
}

bool Sn::ConvX::registerUnionType(const char* unionName, const char* typeName, int typeValue)
{
	const PxU32 nb = mUnions.size();
	for(PxU32 i=0;i<nb;i++)
	{
		if(strcmp(mUnions[i].mName, unionName)==0)
		{
			UnionType t;
			t.mTypeName		= typeName;
			t.mTypeValue	= typeValue;
			mUnions[i].mTypes.pushBack(t);			
			displayMessage(PxErrorCode::eDEBUG_INFO, "Registering union type: %s | %s | %d\n", unionName, typeName, typeValue);
			return true;
		}
	}

	displayMessage(PxErrorCode::eINTERNAL_ERROR, "PxBinaryConverter: union not found: %s, please check the source metadata.\n", unionName);
	return false;
}

const char* Sn::ConvX::getTypeName(const char* unionName, int typeValue)
{
	const PxU32 nb = mUnions.size();
	for(PxU32 i=0;i<nb;i++)
	{
		if(strcmp(mUnions[i].mName, unionName)==0)
		{
			const PxU32 nbTypes = mUnions[i].mTypes.size();
			for(PxU32 j=0;j<nbTypes;j++)
			{
				const UnionType& t = mUnions[i].mTypes[j];
				if(t.mTypeValue==typeValue)
					return t.mTypeName;
			}
			break;
		}
	}
	displayMessage(PxErrorCode::eINTERNAL_ERROR,
		"PxBinaryConverter: union type not found: %s, type %d, please check the source metadata.\n", unionName, typeValue);		
	assert(0);
	return NULL;
}
