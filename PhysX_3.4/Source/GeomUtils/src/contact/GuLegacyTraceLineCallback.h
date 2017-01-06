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


#ifndef GU_CONTACTTRACESEGMENTCALLBACK_H
#define GU_CONTACTTRACESEGMENTCALLBACK_H

#include "CmMatrix34.h"
#include "GuGeometryUnion.h"

#include "GuHeightFieldUtil.h"
#include "CmRenderOutput.h"
#include "GuContactBuffer.h"

namespace physx
{
namespace Gu
{
#define DISTANCE_BASED_TEST

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct GuContactTraceSegmentCallback
{
	PxVec3				mLine;
	ContactBuffer&	mContactBuffer;
	Cm::Matrix34		mTransform;
	PxReal				mContactDistance;
	PxU32				mPrevTriangleIndex; // currently only used to indicate first callback
//	Cm::RenderOutput&	mRO;

	PX_INLINE GuContactTraceSegmentCallback(
		const PxVec3& line, Gu::ContactBuffer& contactBuffer,
		Cm::Matrix34 transform, PxReal contactDistance
		/*, Cm::RenderOutput& ro*/)
	: mLine(line), mContactBuffer(contactBuffer), mTransform(transform),
	mContactDistance(contactDistance), mPrevTriangleIndex(0xFFFFffff)//, mRO(ro)
	{
	}

	bool onEvent(PxU32 , PxU32* )
	{
		return true;
	}

	PX_INLINE bool faceHit(const Gu::HeightFieldUtil& /*hfUtil*/, const PxVec3& /*hitPoint*/, PxU32 /*triangleIndex*/, PxReal, PxReal) { return true; }

	// x,z is the point of projected face entry intercept in hf coords, rayHeight is at that same point
	PX_INLINE bool underFaceHit(
		const Gu::HeightFieldUtil& hfUtil, const PxVec3& triangleNormal,
		const PxVec3& crossedEdge, PxF32 x, PxF32 z, PxF32 rayHeight, PxU32 triangleIndex)
	{
		if (mPrevTriangleIndex == 0xFFFFffff) // we only record under-edge contacts so we need at least 2 face hits to have the edge
		{
			mPrevTriangleIndex = triangleIndex;
			//mPrevTriangleNormal = hfUtil.getTriangleNormal(triangleIndex);
			return true;
		}

		const Gu::HeightField& hf = hfUtil.getHeightField();
		PxF32 y = hfUtil.getHeightAtShapePoint(x, z); // TODO: optmization opportunity - this can be derived cheaply inside traceSegment
		PxF32 dy = rayHeight - y;

		if (!hf.isDeltaHeightInsideExtent(dy, mContactDistance))
			return true;

		// add contact
		PxVec3 n = crossedEdge.cross(mLine);
		if (n.y < 0) // Make sure cross product is facing correctly before clipping
			n = -n;

		if (n.y < 0) // degenerate case
			return true;

		const PxReal ll = n.magnitudeSquared();
		if (ll > 0) // normalize
			n *= PxRecipSqrt(ll);
		else // degenerate case
			return true; 

		// Scale delta height so it becomes the "penetration" along the normal
		dy *= n.y;
		if (hf.getThicknessFast() > 0)
		{
			n = -n;
			dy = -dy;
		}

		// compute the contact point
		const PxVec3 point(x, rayHeight, z);
		//mRO << PxVec3(1,0,0) << Gu::Debug::convertToPxMat44(mTransform)
		//	<< Cm::RenderOutput::LINES << point << point + triangleNormal;
#ifdef DISTANCE_BASED_TEST
		mContactBuffer.contact(
			mTransform.transform(point), mTransform.rotate(triangleNormal), dy, triangleIndex);
#else
		// add gContactDistance to compensate for fact that we don't support dist based contacts in box/convex-hf!
		// See comment at start of those functs.
		mContactBuffer.contact(
			mTransform.transform(point), mTransform.rotate(triangleNormal),
			dy + mContactDistance, PXC_CONTACT_NO_FACE_INDEX, triangleIndex);
#endif
		mPrevTriangleIndex = triangleIndex;
		//mPrevTriangleNormal = triangleNormal;
		return true;
	}

private:
	GuContactTraceSegmentCallback& operator=(const GuContactTraceSegmentCallback&);
};

class GuContactHeightfieldTraceSegmentHelper
{
	PX_NOCOPY(GuContactHeightfieldTraceSegmentHelper)
public:
	GuContactHeightfieldTraceSegmentHelper(const HeightFieldUtil& hfUtil)
		: mHfUtil(hfUtil)
	{
		mHfUtil.computeLocalBounds(mLocalBounds);
	}

	PX_INLINE void traceSegment(const PxVec3& aP0, const PxVec3& aP1, GuContactTraceSegmentCallback* aCallback) const
	{
		mHfUtil.traceSegment<GuContactTraceSegmentCallback, true, false>(aP0, aP1 - aP0, 1.0f, aCallback, mLocalBounds, true, NULL);
	}

private:
	const HeightFieldUtil&	mHfUtil;
	PxBounds3				mLocalBounds;
};

}//Gu
}//physx

#endif
