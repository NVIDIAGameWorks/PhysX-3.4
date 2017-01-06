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


#ifndef PX_RIGID_BODY_ACCESS_GPU_H
#define PX_RIGID_BODY_ACCESS_GPU_H

#include "foundation/PxTransform.h"
#include "foundation/PxStrideIterator.h"
#include "GuGeometryUnion.h"

namespace physx
{

class PxRigidBodyAccessGpu
{
public:

	struct ShapeProperties
	{
		PxTransform* ownerToWorld;
		PxTransform* shapeToOwner;
		Gu::GeometryUnion* geometry;
	};

	struct BodyProperties
	{
		PxStrideIterator<PxTransform>		currentTransforms;
		PxStrideIterator<PxTransform>		previousTransforms;
		PxStrideIterator<PxVec3>			linearVelocities;
		PxStrideIterator<PxVec3>			angularVelocities;
		PxStrideIterator<PxTransform>		body2ActorTransforms;
		PxStrideIterator<size_t>			cpuBodyHandle;
	};

	virtual void copyShapeProperties(ShapeProperties& shapeProperties, const size_t shape, const size_t body)	const = 0;
	virtual void copyBodyProperties(BodyProperties& bodyProperties, const size_t* bodies, PxU32 numBodies)		const = 0;
};

}

#endif // PX_RIGID_BODY_ACCESS_GPU_H
