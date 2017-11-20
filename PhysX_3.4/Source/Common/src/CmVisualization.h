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

#ifndef PX_PHYSICS_COMMON_VISUALIZATION
#define PX_PHYSICS_COMMON_VISUALIZATION

#include "foundation/PxTransform.h"
#include "CmPhysXCommon.h"
#include "PxConstraintDesc.h"
#include "CmRenderOutput.h"

namespace physx
{
namespace Cm
{
	PX_PHYSX_COMMON_API void visualizeJointFrames(RenderOutput& out,
							  PxReal scale,
							  const PxTransform& parent,
							  const PxTransform& child);

	PX_PHYSX_COMMON_API void visualizeLinearLimit(RenderOutput& out,
							  PxReal scale,
							  const PxTransform& t0,
							  const PxTransform& t1,
							  PxReal value,
							  bool active);

	PX_PHYSX_COMMON_API void visualizeAngularLimit(RenderOutput& out,
							   PxReal scale,
							   const PxTransform& t0,
							   PxReal lower,
							   PxReal upper,
							   bool active);

	PX_PHYSX_COMMON_API void visualizeLimitCone(RenderOutput& out,
							PxReal scale,
							const PxTransform& t,
							PxReal ySwing,
							PxReal zSwing,
							bool active);

	PX_PHYSX_COMMON_API void visualizeDoubleCone(RenderOutput& out,
							 PxReal scale,
							 const PxTransform& t,
							 PxReal angle,
							 bool active);
	
	struct ConstraintImmediateVisualizer : public PxConstraintVisualizer
	{
		PxF32			mFrameScale;
		PxF32			mLimitScale;
		RenderOutput&	mCmOutput;

		//Not possible to implement
		ConstraintImmediateVisualizer& operator=( const ConstraintImmediateVisualizer& );

		ConstraintImmediateVisualizer( PxF32 _frameScale, PxF32 _limitScale, RenderOutput& _output )
			: mFrameScale( _frameScale )
			, mLimitScale( _limitScale )
			, mCmOutput( _output )
		{
		}

		virtual void visualizeJointFrames( const PxTransform& parent, const PxTransform& child )
		{
			Cm::visualizeJointFrames(mCmOutput, mFrameScale, parent, child );
		}

		virtual void visualizeLinearLimit( const PxTransform& t0, const PxTransform& t1, PxReal value, bool active )
		{
			Cm::visualizeLinearLimit( mCmOutput, mLimitScale, t0, t1, value, active );
		}

		virtual void visualizeAngularLimit( const PxTransform& t0, PxReal lower, PxReal upper, bool active)
		{
			Cm::visualizeAngularLimit( mCmOutput, mLimitScale, t0, lower, upper, active );
		}

		virtual void visualizeLimitCone( const PxTransform& t, PxReal ySwing, PxReal zSwing, bool active)
		{
			Cm::visualizeLimitCone( mCmOutput, mLimitScale, t, ySwing, zSwing, active );
		}

		virtual void visualizeDoubleCone( const PxTransform& t, PxReal angle, bool active)
		{
			Cm::visualizeDoubleCone( mCmOutput, mLimitScale, t, angle, active );
		}

		virtual void visualizeLine( const PxVec3& p0, const PxVec3& p1, PxU32 color)
		{
			mCmOutput << color;
			mCmOutput.outputSegment(p0, p1);
		}
	};
}

}

#endif
