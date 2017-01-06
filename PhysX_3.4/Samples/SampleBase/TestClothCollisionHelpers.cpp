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
#include "TestClothCollisionHelpers.h"

#include "cloth/PxCloth.h"
#include "cloth/PxClothCollisionData.h"

//needed for sync primitives.
#if PX_WINDOWS
#define NOMINMAX
#	include <windows.h>
#elif defined(_XBOX)
#	include "xbox360\xtl_XBOX.h"
#endif

//----------------------------------------------------------------------------//
Test::ClothCollision::ClothCollision() :
    mClothPose(PxVec3(0.0f), PxQuat(PxIdentity))
{
}

//----------------------------------------------------------------------------//
void Test::ClothCollision::release()
{
	delete this;
}

//----------------------------------------------------------------------------//

Test::ClothCollision::~ClothCollision()
{
	mSphereMotion.clear();
}

//----------------------------------------------------------------------------//
PxU32 Test::ClothCollision::addSphere(const PxVec3& position, PxReal radius)
{
    PxClothCollisionSphere sphere;

    sphere.pos = position;
    sphere.radius = radius;

    mSpheres.pushBack(sphere);
	mOrigPositions.pushBack(sphere.pos);
	mWorldPositions.pushBack(sphere.pos);

	MotionGenerator mg;

	mg.mTransform.p = position;
	mSphereMotion.pushBack(mg);

	return mSpheres.size() - 1;
}

//----------------------------------------------------------------------------//
PxU32 Test::ClothCollision::addCapsuleIndex(PxU32 i0, PxU32 i1)
{
    mCapsuleIndices.pushBack(i0);
    mCapsuleIndices.pushBack(i1);

	MotionGenerator mg;

	mg.mTransform.p = 0.5f * (mOrigPositions[i0] + mOrigPositions[i1]);
	mCapsuleMotion.pushBack(mg);

	PX_ASSERT(( mCapsuleIndices.size() % 2) == 0);
	return (mCapsuleIndices.size() / 2) - 1;
}

//----------------------------------------------------------------------------//
PxU32 Test::ClothCollision::addCapsule(const PxVec3& position1, PxReal radius1, 
										 const PxVec3& position2, PxReal radius2)
{
	PxU32 i0 = addSphere(position1, radius1);
	PxU32 i1 = addSphere(position2, radius2);

	return addCapsuleIndex(i0, i1);
}

//----------------------------------------------------------------------------//
void Test::ClothCollision::setClothPose(const PxTransform &clothPose)
{
    mClothPose = clothPose;
    applyLocalTransform();
}

//----------------------------------------------------------------------------//
void Test::ClothCollision::applyLocalTransform()
{
	PxTransform clothInv = mClothPose.getInverse();

	PX_ASSERT(mSpheres.size() == mWorldPositions.size());

	Test::Array<PxClothCollisionSphere>::Iterator sp = mSpheres.begin();
	Test::Array<PxVec3>::Iterator o = mWorldPositions.begin();

	for ( ; sp != mSpheres.end(); ++sp, ++o)
	{
		sp->pos = clothInv.transform(*o); 
	}
}

//----------------------------------------------------------------------------//
PxU32 Test::ClothCollision::getNbSpheres() const
{
	return mSpheres.size();
}

//----------------------------------------------------------------------------//
PxU32 Test::ClothCollision::getNbCapsules() const
{
	return mCapsuleIndices.size()/2;
}

//----------------------------------------------------------------------------//
const PxClothCollisionSphere* Test::ClothCollision::getSpheres() const
{
	return mSpheres.begin();
}

//----------------------------------------------------------------------------//
const PxU32* Test::ClothCollision::getCapsules() const
{
	return mCapsuleIndices.begin();
}

//----------------------------------------------------------------------------//
PxBounds3 Test::ClothCollision::getWorldBounds() const
{
	PxBounds3 totalBounds = PxBounds3::empty();
    for (PxU32 i = 0; i < mSpheres.size(); i++)
    {
		PxVec3 p = mClothPose.transform(mSpheres[i].pos);
		PxBounds3 bounds = PxBounds3::centerExtents(p, PxVec3(mSpheres[i].radius));
		totalBounds.minimum = totalBounds.minimum.minimum(bounds.minimum);
		totalBounds.maximum = totalBounds.maximum.maximum(bounds.maximum);
    }
	return totalBounds;
}

//----------------------------------------------------------------------------//
void Test::ClothCollision::setSphereMotion(PxU32 id, const PxVec3 &linear)
{
    mSphereMotion[id].mLinear = linear;
}

//----------------------------------------------------------------------------//
void Test::ClothCollision::setCapsuleMotion(PxU32 id, const PxVec3 &linear, const PxVec3& angular)
{
    mCapsuleMotion[id].mLinear = linear;
    mCapsuleMotion[id].mAngular = angular;
}

//----------------------------------------------------------------------------//
void Test::ClothCollision::updateMotion(PxReal time, PxReal timestep)
{
    for (PxU32 i = 0; i < mSphereMotion.size(); i++)
		mSphereMotion[i].update(time, timestep);

    for (PxU32 i = 0; i < mCapsuleMotion.size(); i++)
		mCapsuleMotion[i].update(time, timestep);

	// update independent sphere positions first
    PxU32 nbSpheres = mWorldPositions.size();
    for (PxU32 i = 0; i < nbSpheres; i++)
        mWorldPositions[i] = mSphereMotion[i].mTransform.p;

	// update capsule position and orientation
	PxU32 numCapsules = mCapsuleIndices.size() / 2;
	PX_ASSERT((mCapsuleIndices.size() % 2) == 0);
	for (PxU32 i = 0; i < numCapsules; i++)
	{
		PxU32 i0 = mCapsuleIndices[i*2];
		PxU32 i1 = mCapsuleIndices[i*2+1];

		PxTransform transform = mCapsuleMotion[i].mTransform;
		PxVec3 o0 = mOrigPositions[i0];
		PxVec3 o1 = mOrigPositions[i1];
		PxVec3 c = 0.5f * (o0 + o1);

		mWorldPositions[i0] = transform.rotate(o0 - c) + transform.p;
		mWorldPositions[i1] = transform.rotate(o1 - c) + transform.p;
	}

    applyLocalTransform();
}

