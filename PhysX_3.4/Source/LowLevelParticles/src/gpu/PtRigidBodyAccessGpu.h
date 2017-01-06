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

#ifndef PT_GPU_RIGID_BODY_ACCESS_H
#define PT_GPU_RIGID_BODY_ACCESS_H

#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API
#if PX_SUPPORT_GPU_PHYSX

#include "PxRigidBodyAccessGpu.h"
#include "PsUserAllocated.h"

namespace physx
{

namespace Pt
{

class BodyTransformVault;

class RigidBodyAccessGpu : public Ps::UserAllocated, public PxRigidBodyAccessGpu
{
  public:
	virtual void copyShapeProperties(ShapeProperties& shapeProperties, const size_t shape, const size_t body) const;
	virtual void copyBodyProperties(BodyProperties& bodyProperties, const size_t* bodies, PxU32 numBodies) const;

  public:
	RigidBodyAccessGpu(const BodyTransformVault& transformVault) : mTransformVault(transformVault)
	{
	}
	virtual ~RigidBodyAccessGpu()
	{
	}

  private:
	RigidBodyAccessGpu& operator=(const RigidBodyAccessGpu&);
	const BodyTransformVault& mTransformVault;
};

} // namespace Pt
} // namespace physx

#endif // PX_SUPPORT_GPU_PHYSX
#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PT_GPU_RIGID_BODY_ACCESS_H
