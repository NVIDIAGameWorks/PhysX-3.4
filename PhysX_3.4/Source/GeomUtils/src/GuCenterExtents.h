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

#ifndef GU_CENTER_EXTENTS_H
#define GU_CENTER_EXTENTS_H

/** \addtogroup geomutils
@{
*/

#include "CmMatrix34.h"
#include "CmUtils.h"
#include "PsUserAllocated.h"

namespace physx
{
namespace Gu
{
	class CenterExtents : public physx::shdfnd::UserAllocated
	{
		public:
		PX_FORCE_INLINE				CenterExtents()						{}
		PX_FORCE_INLINE				CenterExtents(const PxBounds3& b)	{ mCenter = b.getCenter();	mExtents = b.getExtents();	}
		PX_FORCE_INLINE				~CenterExtents()					{}

		PX_FORCE_INLINE	void		getMin(PxVec3& min)		const		{ min = mCenter - mExtents;					}
		PX_FORCE_INLINE	void		getMax(PxVec3& max)		const		{ max = mCenter + mExtents;					}

		PX_FORCE_INLINE	float		getMin(PxU32 axis)		const		{ return mCenter[axis] - mExtents[axis];	}
		PX_FORCE_INLINE	float		getMax(PxU32 axis)		const		{ return mCenter[axis] + mExtents[axis];	}

		PX_FORCE_INLINE	PxVec3		getMin()				const		{ return mCenter - mExtents;				}
		PX_FORCE_INLINE	PxVec3		getMax()				const		{ return mCenter + mExtents;				}

		PX_FORCE_INLINE	void		setMinMax(const PxVec3& min, const PxVec3& max)
									{
										mCenter = (max + min)*0.5f;
										mExtents = (max - min)*0.5f;
									}

		PX_FORCE_INLINE	PxU32		isInside(const CenterExtents& box)	const
									{
										if(box.getMin(0)>getMin(0))	return 0;
										if(box.getMin(1)>getMin(1))	return 0;
										if(box.getMin(2)>getMin(2))	return 0;
										if(box.getMax(0)<getMax(0))	return 0;
										if(box.getMax(1)<getMax(1))	return 0;
										if(box.getMax(2)<getMax(2))	return 0;
										return 1;
									}

		PX_FORCE_INLINE	void		setEmpty()
									{
										mExtents = PxVec3(-PX_MAX_BOUNDS_EXTENTS);
									}

		PX_FORCE_INLINE	bool		isEmpty()	const
									{
										return Cm::isEmpty(mCenter, mExtents);
									}

		PX_FORCE_INLINE	bool		isFinite()	const
									{
										return mCenter.isFinite() && mExtents.isFinite();
									}

		PX_FORCE_INLINE	bool		isValid()	const
									{
										return Cm::isValid(mCenter, mExtents);
									}

		PX_FORCE_INLINE	PxBounds3	transformFast(const PxMat33& matrix)	const
									{
										PX_ASSERT(isValid());
										return PxBounds3::basisExtent(matrix * mCenter, matrix, mExtents);
									}

		PX_INLINE		PxBounds3	transformSafe(const Cm::Matrix34& matrix)	const
									{
										if(isEmpty())
											return PxBounds3::centerExtents(mCenter, mExtents);
										else
											return Cm::basisExtent(matrix.transform(mCenter), matrix.m.column0, matrix.m.column1, matrix.m.column2, mExtents);
									}

						PxVec3		mCenter;
						PxVec3		mExtents;
	};

	//! A padded version of CenterExtents, to safely load its data using SIMD
	class CenterExtentsPadded : public CenterExtents
	{
	public:
		PX_FORCE_INLINE CenterExtentsPadded()	{}
		PX_FORCE_INLINE ~CenterExtentsPadded()	{}
		PxU32	padding;
	};
	PX_COMPILE_TIME_ASSERT(sizeof(CenterExtentsPadded) == 7*4);

}

}

/** @} */
#endif
