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

#ifndef GU_BOUNDS_H
#define GU_BOUNDS_H

#include "foundation/PxBounds3.h"
#include "foundation/PxFlags.h"
#include "GuSIMDHelpers.h"
#include <stddef.h>
#include "PxGeometry.h"
#include "GuBox.h"
#include "GuCenterExtents.h"
#include "GuSphere.h"
#include "GuCapsule.h"

namespace physx
{
class PxGeometry;

namespace Gu
{

//For spheres, planes, capsules and boxes just set localSpaceBounds to NULL.
//For convex meshes, triangle meshes, and heightfields set localSpaceBounds to the relevant pointer if it has already been pre-fetched.  
//For convex meshes, triangle meshes, and heightfields set localSpaceBounds to NULL if it has not already been pre-fetched.   computeBounds will synchronously 
//prefetch the local space bounds if localSpaceBounds is NULL.
//'contactOffset' and 'inflation' should not be used at the same time, i.e. either contactOffset==0.0f, or inflation==1.0f
PX_PHYSX_COMMON_API void computeBounds(PxBounds3& bounds, const PxGeometry& geometry, const PxTransform& transform, float contactOffset, const CenterExtentsPadded* PX_RESTRICT localSpaceBounds, float inflation, bool extrudeHeightfields);	//AABB in world space.

//For spheres, planes, capsules and boxes just set localSpaceBounds to NULL.
//For convex meshes, triangle meshes, and heightfields set localSpaceBounds to the relevant pointer if it has not already been pre-fetched.  
//For convex meshes, triangle meshes, and heightfields set localSpaceBounds to NULL if it has not already been pre-fetched.   computeBounds will synchronously 
//prefetch the local space bounds if localSpaceBounds is NULL.
PX_PHYSX_COMMON_API PxF32 computeBoundsWithCCDThreshold(Vec3p& origin, Vec3p& extent, const PxGeometry& geometry, const PxTransform& transform, const CenterExtentsPadded* PX_RESTRICT localSpaceBounds);	//AABB in world space.


PX_FORCE_INLINE PxBounds3 computeBounds(const PxGeometry& geometry, const PxTransform& pose, bool extrudeHeightfields)
{
	PxBounds3 bounds;
	computeBounds(bounds, geometry, pose, 0.0f, NULL, 1.0f, extrudeHeightfields);
	return bounds;
}

class ShapeData
{
public:

	PX_PHYSX_COMMON_API						ShapeData(const PxGeometry& g, const PxTransform& t, PxReal inflation);	

	// PT: used by overlaps (box, capsule, convex)
	PX_FORCE_INLINE const PxVec3&			getPrunerBoxGeomExtentsInflated()	const	{ return mPrunerBoxGeomExtents; }

	// PT: used by overlaps (box, capsule, convex)
	PX_FORCE_INLINE const PxVec3&			getPrunerWorldPos()					const	{ return mGuBox.center;			}

	PX_FORCE_INLINE const PxBounds3&		getPrunerInflatedWorldAABB()		const	{ return mPrunerInflatedAABB;	}

	// PT: used by overlaps (box, capsule, convex)
	PX_FORCE_INLINE const PxMat33&			getPrunerWorldRot33()				const	{ return mGuBox.rot;			}

	// PT: this one only used by overlaps so far (for sphere shape, pruner level)
	PX_FORCE_INLINE const Gu::Sphere&		getGuSphere() const
	{
		PX_ASSERT(mType == PxGeometryType::eSPHERE);
		return reinterpret_cast<const Gu::Sphere&>(mGuSphere);
	}

	// PT: this one only used by sweeps so far (for box shape, NP level)
	PX_FORCE_INLINE const Gu::Box&			getGuBox() const
	{
		PX_ASSERT(mType == PxGeometryType::eBOX);
		return mGuBox;
	}

	// PT: this one used by sweeps (NP level) and overlaps (pruner level) - for capsule shape
	PX_FORCE_INLINE const Gu::Capsule&		getGuCapsule() const
	{
		PX_ASSERT(mType == PxGeometryType::eCAPSULE);
		return reinterpret_cast<const Gu::Capsule&>(mGuCapsule);
	}

	PX_FORCE_INLINE float					getCapsuleHalfHeight() const
	{
		PX_ASSERT(mType == PxGeometryType::eCAPSULE);
		return mGuBox.extents.x;
	}

	PX_FORCE_INLINE	PxU32					isOBB()		const { return PxU32(mIsOBB);				}
	PX_FORCE_INLINE	PxGeometryType::Enum	getType()	const { return PxGeometryType::Enum(mType);	}

	PX_NOCOPY(ShapeData)
private:

	// PT: box: pre-inflated box extents
	//     capsule: pre-inflated extents of box-around-capsule
	//     convex: pre-inflated extents of box-around-convex
	//     sphere: not used
	PxVec3				mPrunerBoxGeomExtents;	// used for pruners. This volume encloses but can differ from the original shape

	// PT:
	//
	// box center = unchanged copy of initial shape's position, except for convex (position of box around convex)
	// SIMD code will load it as a V4 (safe because member is not last of Gu structure)
	//
	// box rot = precomputed PxMat33 version of initial shape's rotation, except for convex (rotation of box around convex)
	// SIMD code will load it as V4s (safe because member is not last of Gu structure)
	//
	// box extents = non-inflated initial box extents for box shape, half-height for capsule, otherwise not used
	Gu::Box				mGuBox;

	PxBounds3			mPrunerInflatedAABB;	// precomputed AABB for the pruner shape
	PxU16				mIsOBB;					// true for OBB, false for AABB. Also used as padding for mPrunerInflatedAABB, don't move.
	PxU16				mType;					// shape's type

	// these union Gu shapes are only precomputed for narrow phase (not pruners), can be different from mPrunerVolume
	// so need separate storage
	union
	{
		PxU8 mGuCapsule[sizeof(Gu::Capsule)];	// 28
		PxU8 mGuSphere[sizeof(Gu::Sphere)];		// 16
	};
};

// PT: please make sure it fits in "one" cache line
PX_COMPILE_TIME_ASSERT(sizeof(ShapeData)==128);

}  // namespace Gu

}
#endif
