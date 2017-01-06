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


#include "ExtSphericalJoint.h"
#include "ExtConstraintHelper.h"
#include "CmConeLimitHelper.h"
#include "CmRenderOutput.h"

namespace physx
{
namespace Ext
{
	PxU32 SphericalJointSolverPrep(Px1DConstraint* constraints,
		PxVec3& body0WorldOffset,
		PxU32 /*maxConstraints*/,
		PxConstraintInvMassScale &invMassScale,
		const void* constantBlock,							  
		const PxTransform& bA2w,
		const PxTransform& bB2w)
	{

		using namespace joint;
		const SphericalJointData& data = *reinterpret_cast<const SphericalJointData*>(constantBlock);
		invMassScale = data.invMassScale;


		PxTransform cA2w = bA2w * data.c2b[0];
		PxTransform cB2w = bB2w * data.c2b[1];

		if(cB2w.q.dot(cA2w.q)<0) 
			cB2w.q = -cB2w.q;

		body0WorldOffset = cB2w.p-bA2w.p;
		joint::ConstraintHelper ch(constraints, cB2w.p - bA2w.p, cB2w.p - bB2w.p);

		if(data.jointFlags & PxSphericalJointFlag::eLIMIT_ENABLED)
		{
			PxQuat swing, twist;
			Ps::separateSwingTwist(cA2w.q.getConjugate() * cB2w.q, swing, twist);
			PX_ASSERT(PxAbs(swing.x)<1e-6f);

			Cm::ConeLimitHelper coneHelper(data.tanQZLimit, data.tanQYLimit, data.tanQPad);

			PxVec3 axis;
			PxReal error;
			if(coneHelper.getLimit(swing, axis, error))
				ch.angularLimit(cA2w.rotate(axis),error,data.limit);

		}

		ch.prepareLockedAxes(cA2w.q, cB2w.q, cA2w.transformInv(cB2w.p), 7, 0);

		return ch.getCount();
	}
}//namespace

}

//~PX_SERIALIZATION

