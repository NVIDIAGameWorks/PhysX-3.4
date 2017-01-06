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

#ifndef GU_PCM_CONTACT_MESH_CALLBACK_H
#define GU_PCM_CONTACT_MESH_CALLBACK_H

#include "GuMidphaseInterface.h"
#include "GuEntityReport.h"
#include "GuHeightFieldUtil.h"
#include "GuTriangleCache.h"
#include "GuConvexEdgeFlags.h"

namespace physx
{

namespace Gu
{

template <typename Derived>
struct PCMMeshContactGenerationCallback : MeshHitCallback<PxRaycastHit>
{
public:
	const Cm::FastVertex2ShapeScaling&		mMeshScaling;
	const PxU8* PX_RESTRICT					mExtraTrigData;
	bool									mIdtMeshScale;
	static const PxU32 CacheSize = 16;
	Gu::TriangleCache<CacheSize>			mCache;

	PCMMeshContactGenerationCallback(const Cm::FastVertex2ShapeScaling& meshScaling, const PxU8* extraTrigData, bool idtMeshScale)
	:	MeshHitCallback<PxRaycastHit>(CallbackMode::eMULTIPLE),
		mMeshScaling(meshScaling), mExtraTrigData(extraTrigData), mIdtMeshScale(idtMeshScale)
	{
	}

	void flushCache() 
	{
		if (!mCache.isEmpty())
		{
			(static_cast<Derived*>(this))->template processTriangleCache< CacheSize >(mCache);
			mCache.reset();
		}
	}

	virtual PxAgain processHit(
		const PxRaycastHit& hit, const PxVec3& v0, const PxVec3& v1, const PxVec3& v2, PxReal&, const PxU32* vinds)
	{
		if (!(static_cast<Derived*>(this))->doTest(v0, v1, v2))
			return true;

		PxVec3 v[3];
		if(mIdtMeshScale)
		{
			v[0] = v0;
			v[1] = v1;
			v[2] = v2;
		}
		else
		{
			const PxI32 winding = mMeshScaling.flipsNormal() ? 1 : 0;
			v[0] = mMeshScaling * v0;
			v[1 + winding] = mMeshScaling * v1;
			v[2 - winding] = mMeshScaling * v2;
		}

		const PxU32 triangleIndex = hit.faceIndex;
		const PxU8 extraData = getConvexEdgeFlags(mExtraTrigData, triangleIndex);

		if (mCache.isFull())
		{
			(static_cast<Derived*>(this))->template processTriangleCache< CacheSize >(mCache);
			mCache.reset();
		}
		mCache.addTriangle(v, vinds, triangleIndex, extraData);

		return true;
	}

protected:
	PCMMeshContactGenerationCallback& operator=(const PCMMeshContactGenerationCallback&);
};

template <typename Derived>
struct PCMHeightfieldContactGenerationCallback : Gu::EntityReport<PxU32>
{
public:
	const Gu::HeightFieldUtil&	mHfUtil;
	const PxTransform&			mHeightfieldTransform;

	PCMHeightfieldContactGenerationCallback(const Gu::HeightFieldUtil& hfUtil, const PxTransform& heightfieldTransform)	:
		mHfUtil(hfUtil), mHeightfieldTransform(heightfieldTransform)
	{

	}

	// PT: TODO: refactor/unify with similar code in other places
	virtual PxAgain onEvent(PxU32 nb, PxU32* indices)
	{
		const PxU32 CacheSize = 16;
		Gu::TriangleCache<CacheSize> cache;

		const PxU32 nbPasses = (nb+(CacheSize-1))/CacheSize;
		PxU32 nbTrigs = nb;
		PxU32* inds0 = indices;

		const PxU8 nextInd[] = {2,0,1};

		for(PxU32 i = 0; i < nbPasses; ++i)
		{
			cache.mNumTriangles = 0;
			PxU32 trigCount = PxMin(nbTrigs, CacheSize);
			nbTrigs -= trigCount;
			while(trigCount--)
			{
				PxU32 triangleIndex = *(inds0++);
				PxU32 vertIndices[3];

				PxTriangle currentTriangle;	// in world space

				PxU32 adjInds[3];
				mHfUtil.getTriangle(mHeightfieldTransform, currentTriangle, vertIndices, adjInds, triangleIndex, false, false);

				PxVec3 normal;
				currentTriangle.normal(normal);

				PxU8 triFlags = 0; //KS - temporary until we can calculate triFlags for HF

				for(PxU32 a = 0; a < 3; ++a)
				{

					if(adjInds[a] != 0xFFFFFFFF)
					{
						PxTriangle adjTri;
						PxU32 inds[3];
						mHfUtil.getTriangle(mHeightfieldTransform, adjTri, inds, NULL, adjInds[a], false, false);
						//We now compare the triangles to see if this edge is active

						PX_ASSERT(inds[0] == vertIndices[a] || inds[1] == vertIndices[a] || inds[2] == vertIndices[a]);
						PX_ASSERT(inds[0] == vertIndices[(a+1)%3] || inds[1] == vertIndices[(a+1)%3] || inds[2] == vertIndices[(a+1)%3]);


						PxVec3 adjNormal;
						adjTri.denormalizedNormal(adjNormal);
						PxU32 otherIndex = nextInd[a];
						PxF32 projD = adjNormal.dot(currentTriangle.verts[otherIndex] - adjTri.verts[0]);

						if(projD < 0.f)
						{
							adjNormal.normalize();

							PxF32 proj = adjNormal.dot(normal);

							if(proj < 0.997f)
							{
								triFlags |= (1 << (a+3));
							}
						}
					}
					else
						triFlags |= (1 << (a+3));
				}

				cache.addTriangle(currentTriangle.verts, vertIndices, triangleIndex, triFlags);
			}
			PX_ASSERT(cache.mNumTriangles <= 16);

			(static_cast<Derived*>(this))->template processTriangleCache< CacheSize >(cache);
		}
		return true;
	}	
protected:
	PCMHeightfieldContactGenerationCallback& operator=(const PCMHeightfieldContactGenerationCallback&);
};

}//Gu
}//physx

#endif
