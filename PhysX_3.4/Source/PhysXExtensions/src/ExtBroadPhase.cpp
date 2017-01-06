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


#include "foundation/PxBounds3.h"
#include "PxBroadPhaseExt.h"
#include "PsFoundation.h"
#include "CmPhysXCommon.h"

using namespace physx;

PxU32 PxBroadPhaseExt::createRegionsFromWorldBounds(PxBounds3* regions, const PxBounds3& globalBounds, PxU32 nbSubdiv, PxU32 upAxis)
{
	PX_CHECK_MSG(globalBounds.isValid(), "PxBroadPhaseExt::createRegionsFromWorldBounds(): invalid bounds provided!");
	PX_CHECK_MSG(upAxis<3, "PxBroadPhaseExt::createRegionsFromWorldBounds(): invalid up-axis provided!");

	const PxVec3& min = globalBounds.minimum;
	const PxVec3& max = globalBounds.maximum;
	const float dx = (max.x - min.x) / float(nbSubdiv);
	const float dy = (max.y - min.y) / float(nbSubdiv);
	const float dz = (max.z - min.z) / float(nbSubdiv);
	PxU32 nbRegions = 0;
	PxVec3 currentMin, currentMax;
	for(PxU32 j=0;j<nbSubdiv;j++)
	{
		for(PxU32 i=0;i<nbSubdiv;i++)
		{
			if(upAxis==0)
			{
				currentMin = PxVec3(min.x, min.y + dy * float(i),   min.z + dz * float(j));
				currentMax = PxVec3(max.x, min.y + dy * float(i+1), min.z + dz * float(j+1));
			}
			else if(upAxis==1)
			{
				currentMin = PxVec3(min.x + dx * float(i),   min.y, min.z + dz * float(j));
				currentMax = PxVec3(min.x + dx * float(i+1), max.y, min.z + dz * float(j+1));
			}
			else if(upAxis==2)
			{
				currentMin = PxVec3(min.x + dx * float(i),   min.y + dy * float(j), min.z);
				currentMax = PxVec3(min.x + dx * float(i+1), min.y + dy * float(j+1), max.z);
			}

			regions[nbRegions++] = PxBounds3(currentMin, currentMax);
		}
	}
	return nbRegions;
}
