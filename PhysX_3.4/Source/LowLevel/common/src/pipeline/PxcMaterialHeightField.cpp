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
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "PxTriangleMesh.h"
#include "PxvGeometry.h"
#include "PxsMaterialManager.h"
#include "PxcNpThreadContext.h"
#include "GuHeightField.h"

using namespace physx;
using namespace Gu;

namespace physx
{
	bool PxcGetMaterialShapeHeightField(const PxsShapeCore* shape0, const PxsShapeCore* shape1, PxcNpThreadContext& context, PxsMaterialInfo* materialInfo);
	bool PxcGetMaterialHeightField(const PxsShapeCore* shape, const PxU32 index, PxcNpThreadContext& context, PxsMaterialInfo* materialInfo);
	PxU32 GetMaterialIndex(const Gu::HeightFieldData* hfData, PxU32 triangleIndex);
}

physx::PxU32 physx::GetMaterialIndex(const Gu::HeightFieldData* hfData, PxU32 triangleIndex)
{
	const PxU32 sampleIndex = triangleIndex >> 1;
	const bool isFirstTriangle = (triangleIndex & 0x1) == 0;

	//get sample
	const PxHeightFieldSample* hf = &hfData->samples[sampleIndex];
	return isFirstTriangle ? hf->materialIndex0 : hf->materialIndex1;
}

bool physx::PxcGetMaterialHeightField(const PxsShapeCore* shape, const PxU32 index, PxcNpThreadContext& context, PxsMaterialInfo* materialInfo)
{
	PX_ASSERT(index == 1);
	PX_UNUSED(index);
	const ContactBuffer& contactBuffer = context.mContactBuffer;
	const PxHeightFieldGeometryLL& hfGeom = shape->geometry.get<const PxHeightFieldGeometryLL>();
	if(hfGeom.materials.numIndices <= 1)
	{
		for(PxU32 i=0; i< contactBuffer.count; ++i)
		{
			(&materialInfo[i].mMaterialIndex0)[index] = shape->materialIndex;
		}
	}
	else
	{
		const PxU16* materialIndices = hfGeom.materials.indices;
			
		const Gu::HeightFieldData* hf = hfGeom.heightFieldData;
		
		for(PxU32 i=0; i< contactBuffer.count; ++i)
		{
			const Gu::ContactPoint& contact = contactBuffer.contacts[i];
			const PxU32 localMaterialIndex = GetMaterialIndex(hf, contact.internalFaceIndex1);
			(&materialInfo[i].mMaterialIndex0)[index] = materialIndices[localMaterialIndex];
		}
	}
	return true;
}

bool physx::PxcGetMaterialShapeHeightField(const PxsShapeCore* shape0, const PxsShapeCore* shape1, PxcNpThreadContext& context,  PxsMaterialInfo* materialInfo)
{
	const ContactBuffer& contactBuffer = context.mContactBuffer;
	const PxHeightFieldGeometryLL& hfGeom = shape1->geometry.get<const PxHeightFieldGeometryLL>();
	if(hfGeom.materials.numIndices <= 1)
	{
		for(PxU32 i=0; i< contactBuffer.count; ++i)
		{
			materialInfo[i].mMaterialIndex0 = shape0->materialIndex;
			materialInfo[i].mMaterialIndex1 = shape1->materialIndex;
		}
	}
	else
	{
		const PxU16* materialIndices = hfGeom.materials.indices;
			
		const Gu::HeightFieldData* hf = hfGeom.heightFieldData;
		
		for(PxU32 i=0; i< contactBuffer.count; ++i)
		{
			const Gu::ContactPoint& contact = contactBuffer.contacts[i];
			materialInfo[i].mMaterialIndex0 = shape0->materialIndex;
			//contact.featureIndex0 = shape0->materialIndex;
			const PxU32 localMaterialIndex = GetMaterialIndex(hf, contact.internalFaceIndex1);
			//contact.featureIndex1 = materialIndices[localMaterialIndex];
			PX_ASSERT(localMaterialIndex<hfGeom.materials.numIndices);
			materialInfo[i].mMaterialIndex1 = materialIndices[localMaterialIndex];
		}
	}
	return true;
}

