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

           
#include "ScIterators.h"
#include "ScBodySim.h"
#include "ScShapeSim.h"
#include "ScShapeInteraction.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////

Sc::ContactIterator::Pair::Pair(const void*& contactPatches, const void*& contactPoints, PxU32 /*contactDataSize*/, const PxReal*& forces, PxU32 numContacts, PxU32 numPatches,
	ShapeSim& shape0, ShapeSim& shape1)
: mIndex(0)
, mNumContacts(numContacts)
, mIter(reinterpret_cast<const PxU8*>(contactPatches), reinterpret_cast<const PxU8*>(contactPoints), reinterpret_cast<const PxU32*>(forces + numContacts), numPatches, numContacts)
, mForces(forces)
, mShape0(&shape0)
, mShape1(&shape1)
{	
	mCurrentContact.shape0 = shape0.getPxShape();
	mCurrentContact.shape1 = shape1.getPxShape();
	mCurrentContact.normalForceAvailable = (forces != NULL);
}

Sc::ContactIterator::Pair* Sc::ContactIterator::getNextPair()
{ 
	if(mCurrent < mLast)
	{
		ShapeInteraction* si = static_cast<ShapeInteraction*>(*mCurrent);

		const void* contactPatches = NULL;
		const void* contactPoints = NULL;
		PxU32 contactDataSize = 0;
		const PxReal* forces = NULL;
		PxU32 numContacts = 0;
		PxU32 numPatches = 0;

		PxU32 nextOffset = si->getContactPointData(contactPatches, contactPoints, contactDataSize, numContacts, numPatches, forces, mOffset, *mOutputs);

		if (nextOffset == mOffset)
			++mCurrent;
		else
			mOffset = nextOffset;

		mCurrentPair = Pair(contactPatches, contactPoints, contactDataSize, forces, numContacts, numPatches, si->getShape0(), si->getShape1());
		return &mCurrentPair;
	}
	else
		return NULL;
}

Sc::Contact* Sc::ContactIterator::Pair::getNextContact()
{
	if(mIndex < mNumContacts)
	{
		if(!mIter.hasNextContact())
		{
			if(!mIter.hasNextPatch())
				return NULL;
			mIter.nextPatch();
		}
		PX_ASSERT(mIter.hasNextContact());
		mIter.nextContact();

		mCurrentContact.normal = mIter.getContactNormal();
		mCurrentContact.point = mIter.getContactPoint();
		mCurrentContact.separation = mIter.getSeparation();
		mCurrentContact.normalForce = mForces ? mForces[mIndex] : 0;
		mCurrentContact.faceIndex0 = mIter.getFaceIndex0();
		mCurrentContact.faceIndex1 = mIter.getFaceIndex1();

		mIndex++;
		return &mCurrentContact;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
