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

#include "PxPhysXConfig.h"
#if PX_USE_CLOTH_API

#include "foundation/PxVec4.h"
#include "foundation/PxMemory.h"
#include "foundation/PxStrideIterator.h"
#include "extensions/PxClothTetherCooker.h"
#include "PsArray.h"
// from shared foundation
#include <PsFoundation.h>

using namespace physx;

struct physx::PxClothSimpleTetherCookerImpl
{
	PxClothSimpleTetherCookerImpl(const PxClothMeshDesc& desc);

	void	getTetherData(PxU32* userTetherAnchors, PxReal* userTetherLengths) const;

public:
	// output
	shdfnd::Array<PxU32>	mTetherAnchors;
	shdfnd::Array<PxReal>	mTetherLengths;

protected:
	void	createTetherData(const PxClothMeshDesc &desc);
};

PxClothSimpleTetherCooker::PxClothSimpleTetherCooker(const PxClothMeshDesc& desc)
: mImpl(new PxClothSimpleTetherCookerImpl(desc))
{
}

PxClothSimpleTetherCooker::~PxClothSimpleTetherCooker()
{
	delete mImpl;
}

void PxClothSimpleTetherCooker::getTetherData(PxU32* userTetherAnchors, PxReal* userTetherLengths) const
{
	mImpl->getTetherData(userTetherAnchors, userTetherLengths);
}

///////////////////////////////////////////////////////////////////////////////
PxClothSimpleTetherCookerImpl::PxClothSimpleTetherCookerImpl(const PxClothMeshDesc &desc)
{
	createTetherData(desc);
}

///////////////////////////////////////////////////////////////////////////////
void PxClothSimpleTetherCookerImpl::createTetherData(const PxClothMeshDesc &desc)
{
	PxU32 numParticles = desc.points.count;

	if (!desc.invMasses.data)
		return;

	// assemble points
	shdfnd::Array<PxVec4> particles;
	particles.reserve(numParticles);
	PxStrideIterator<const PxVec3> pIt(reinterpret_cast<const PxVec3*>(desc.points.data), desc.points.stride);
	PxStrideIterator<const PxReal> wIt(reinterpret_cast<const PxReal*>(desc.invMasses.data), desc.invMasses.stride);
	for(PxU32 i=0; i<numParticles; ++i)
		particles.pushBack(PxVec4(*pIt++, wIt.ptr() ? *wIt++ : 1.0f));

	// compute tether data
	shdfnd::Array<PxU32> attachedIndices;
	for(PxU32 i=0; i < numParticles; ++i)
		if(particles[i].w == 0.0f)
			attachedIndices.pushBack(i);

	PxU32 n = attachedIndices.empty() ? 0 : numParticles;
	for(PxU32 i=0; i < n; ++i)
	{
		mTetherAnchors.reserve(numParticles);
		mTetherLengths.reserve(numParticles);

		PxVec3 position = reinterpret_cast<const PxVec3&>(particles[i]);
		float minSqrDist = FLT_MAX;
		PxU32 minIndex = numParticles;
		const PxU32 *aIt, *aEnd = attachedIndices.end();
		for(aIt = attachedIndices.begin(); aIt != aEnd; ++aIt)
		{
			float sqrDist = (reinterpret_cast<const PxVec3&>(
				particles[*aIt]) - position).magnitudeSquared();
			if(minSqrDist > sqrDist)
			{
				minSqrDist = sqrDist;
				minIndex = *aIt;
			}
		}

		mTetherAnchors.pushBack(minIndex);
		mTetherLengths.pushBack(PxSqrt(minSqrDist));
	}

	PX_ASSERT(mTetherAnchors.size() == mTetherLengths.size());

}

///////////////////////////////////////////////////////////////////////////////
void  
PxClothSimpleTetherCookerImpl::getTetherData(PxU32* userTetherAnchors, PxReal* userTetherLengths) const
{
	PxMemCopy(userTetherAnchors, mTetherAnchors.begin(), mTetherAnchors.size() * sizeof(PxU32));
	PxMemCopy(userTetherLengths, mTetherLengths.begin(), mTetherLengths.size() * sizeof(PxReal));
}


#endif //PX_USE_CLOTH_API


