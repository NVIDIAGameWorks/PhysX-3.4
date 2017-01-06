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

#ifndef GU_DISTANCE_POINT_SEGMENT_H
#define GU_DISTANCE_POINT_SEGMENT_H

#include "PxPhysXCommonConfig.h"
#include "GuSegment.h"

namespace physx
{
namespace Gu
{
	// dir = p1 - p0
	PX_FORCE_INLINE PxReal distancePointSegmentSquaredInternal(const PxVec3& p0, const PxVec3& dir, const PxVec3& point, PxReal* param=NULL)
	{
		PxVec3 diff = point - p0;
		PxReal fT = diff.dot(dir);

		if(fT<=0.0f)
		{
			fT = 0.0f;
		}
		else
		{
			const PxReal sqrLen = dir.magnitudeSquared();
			if(fT>=sqrLen)
			{
				fT = 1.0f;
				diff -= dir;
			}
			else
			{
				fT /= sqrLen;
				diff -= fT*dir;
			}
		}

		if(param)
			*param = fT;

		return diff.magnitudeSquared();
	}

	/**
	A segment is defined by S(t) = mP0 * (1 - t) + mP1 * t, with 0 <= t <= 1
	Alternatively, a segment is S(t) = Origin + t * Direction for 0 <= t <= 1.
	Direction is not necessarily unit length. The end points are Origin = mP0 and Origin + Direction = mP1.
	*/
	PX_FORCE_INLINE PxReal distancePointSegmentSquared(const PxVec3& p0, const PxVec3& p1, const PxVec3& point, PxReal* param=NULL)
	{
		return distancePointSegmentSquaredInternal(p0, p1 - p0, point, param);
	}

	PX_INLINE PxReal distancePointSegmentSquared(const Gu::Segment& segment, const PxVec3& point, PxReal* param=NULL)
	{
		return distancePointSegmentSquared(segment.p0, segment.p1, point, param);
	}

} // namespace Gu

}

#endif
