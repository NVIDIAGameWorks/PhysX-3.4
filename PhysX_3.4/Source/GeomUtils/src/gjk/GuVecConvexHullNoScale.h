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

#ifndef GU_VEC_CONVEXHULL_NOSCALE_H
#define GU_VEC_CONVEXHULL_NOSCALE_H

#include "foundation/PxUnionCast.h"
#include "PxPhysXCommonConfig.h"
#include "GuVecConvexHull.h"


namespace physx
{
namespace Gu
{

	class ConvexHullNoScaleV : public ConvexHullV
	{


		public:
		/**
		\brief Constructor
		*/
		PX_SUPPORT_INLINE ConvexHullNoScaleV(): ConvexHullV()
		{
		}

		PX_FORCE_INLINE Ps::aos::Vec3V supportPoint(const PxI32 index, Ps::aos::FloatV* /*marginDif*/)const
		{
			using namespace Ps::aos;
			return V3LoadU_SafeReadW(verts[index]);	// PT: safe because of the way vertex memory is allocated in ConvexHullData (and 'verts' is initialized with ConvexHullData::getHullVertices())
		}


		//This funcation is just to load the PxVec3 to Vec3V. However, for GuVecConvexHul.h, this is used to transform all the verts from vertex space to shape space
		PX_SUPPORT_INLINE void populateVerts(const PxU8* inds, PxU32 numInds, const PxVec3* originalVerts, Ps::aos::Vec3V* verts_)const
		{
			using namespace Ps::aos;

			for(PxU32 i=0; i<numInds; ++i)
				verts_[i] = V3LoadU_SafeReadW(originalVerts[inds[i]]);	// PT: safe because of the way vertex memory is allocated in ConvexHullData (and 'populateVerts' is always called with polyData.mVerts)
		}
		

		//This function is used in epa
		//dir is in the shape space
		PX_SUPPORT_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir)const
		{
			using namespace Ps::aos;
			const PxU32 maxIndex = supportVertexIndex(dir);
			return V3LoadU_SafeReadW(verts[maxIndex]);	// PT: safe because of the way vertex memory is allocated in ConvexHullData (and 'verts' is initialized with ConvexHullData::getHullVertices())
		}

		//this is used in the sat test for the full contact gen
		PX_SUPPORT_INLINE void supportLocal(const Ps::aos::Vec3VArg dir, Ps::aos::FloatV& min, Ps::aos::FloatV& max)const
		{
			supportVertexMinMax(dir, min, max);
		}


		//This function is used in epa
		PX_SUPPORT_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob, const Ps::aos::PsMatTransformV& aTobT) const
		{
			using namespace Ps::aos;
		
			//transform dir into the shape space
			const Vec3V _dir = aTobT.rotate(dir);//relTra.rotateInv(dir);
			const Vec3V maxPoint = supportLocal(_dir);
			//translate maxPoint from shape space of a back to the b space
			return aTob.transform(maxPoint);//relTra.transform(maxPoint);
		}

		//dir in the shape space, this function is used in gjk
		PX_SUPPORT_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir, PxI32& index, Ps::aos::FloatV* /*marginDif*/)const
		{
			using namespace Ps::aos;
			//scale dir and put it in the vertex space, for non-uniform scale, we don't want the scale in the dir, therefore, we are using
			//the transpose of the inverse of shape2Vertex(which is vertex2shape). This will allow us igore the scale and keep the rotation
			//get the extreme point index
			const PxU32 maxIndex = supportVertexIndex(dir);
			index = PxI32(maxIndex);
			return V3LoadU_SafeReadW(verts[index]);	// PT: safe because of the way vertex memory is allocated in ConvexHullData (and 'verts' is initialized with ConvexHullData::getHullVertices())
		}

		//this function is used in gjk
		PX_SUPPORT_INLINE Ps::aos::Vec3V supportRelative(	const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob,
															const Ps::aos::PsMatTransformV& aTobT, PxI32& index, Ps::aos::FloatV* marginDif)const
		{
			using namespace Ps::aos;

			//transform dir from b space to the shape space of a space
			const Vec3V _dir = aTobT.rotate(dir);//relTra.rotateInv(dir);//M33MulV3(skewInvRot, dir);
			const Vec3V p = supportLocal(_dir, index, marginDif);
			//transfrom from a to b space
			return aTob.transform(p);
		}

		
		PX_SUPPORT_INLINE void bruteForceSearchMinMax(const Ps::aos::Vec3VArg _dir, Ps::aos::FloatV& min, Ps::aos::FloatV& max)const 
		{
			using namespace Ps::aos;
			//brute force
			//get the support point from the orignal margin
			FloatV _max = V3Dot(V3LoadU_SafeReadW(verts[0]), _dir);	// PT: safe because of the way vertex memory is allocated in ConvexHullData (and 'verts' is initialized with ConvexHullData::getHullVertices())
			FloatV _min = _max;

			for(PxU32 i = 1; i < numVerts; ++i)
			{ 
				Ps::prefetchLine(&verts[i], 128);
				const Vec3V vertex = V3LoadU_SafeReadW(verts[i]);	// PT: safe because of the way vertex memory is allocated in ConvexHullData (and 'verts' is initialized with ConvexHullData::getHullVertices())
				const FloatV dist = V3Dot(vertex, _dir);

				_max = FMax(dist, _max);
				_min = FMin(dist, _min);
			}

			min = _min;
			max = _max;
		}

		//This function support no scaling, dir is in the shape space(the same as vertex space)
		PX_SUPPORT_INLINE void supportVertexMinMax(const Ps::aos::Vec3VArg dir, Ps::aos::FloatV& min, Ps::aos::FloatV& max)const
		{
			using namespace Ps::aos;

			if(data)
			{
				const PxU32 maxIndex= hillClimbing(dir);
				const PxU32 minIndex= hillClimbing(V3Neg(dir));
				const Vec3V maxPoint= V3LoadU_SafeReadW(verts[maxIndex]);	// PT: safe because of the way vertex memory is allocated in ConvexHullData (and 'verts' is initialized with ConvexHullData::getHullVertices())
				const Vec3V minPoint= V3LoadU_SafeReadW(verts[minIndex]);	// PT: safe because of the way vertex memory is allocated in ConvexHullData (and 'verts' is initialized with ConvexHullData::getHullVertices())
				min = V3Dot(dir, minPoint);
				max = V3Dot(dir, maxPoint);
			}
			else
			{
				bruteForceSearchMinMax(dir, min, max);
			}
		}  


	};

	#define PX_CONVEX_TO_NOSCALECONVEX(x)			(static_cast<ConvexHullNoScaleV*>(x))
}

}

#endif	// 
