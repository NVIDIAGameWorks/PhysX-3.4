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

#include "SampleCharacterCloth.h"

#include "SampleCharacterClothPlatform.h"

//////////////////////////////////////////////////////////////////////////////
// create a kinematic capsule 
SampleCharacterClothPlatform* 
SampleCharacterCloth::createPlatform(const PxTransform &pose, const PxGeometry &geom, PxReal travelTime, const PxVec3 &offset, RenderMaterial *renderMaterial)
{
	PxRigidDynamic* actor = getPhysics().createRigidDynamic(pose);
	actor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

	PxShape* shape = PxRigidActorExt::createExclusiveShape(*actor, geom, getDefaultMaterial());

	PxScene& scene = getActiveScene();
	scene.addActor(*actor);

	createRenderObjectFromShape(actor, shape, renderMaterial);

	SampleCharacterClothPlatform* platform = SAMPLE_NEW(SampleCharacterClothPlatform);
	platform->init(actor, travelTime, offset);

	mPlatforms.pushBack(platform);

	return platform;
}

//////////////////////////////////////////////////////////////////////////////
// create moving platforms
void SampleCharacterCloth::createPlatforms()
{
	SampleCharacterClothPlatform* platform = 0;

	// initial deck
	PxVec3 p0(0,0,-4);
	platform = createPlatform(PxTransform(p0, PxQuat(PxIdentity)),
				   PxBoxGeometry(PxVec3(2,1,8)), 2.0f, PxVec3(0,0,0), mPlatformMaterial);

	// moving platform (moving in x)
	PxVec3 p1 = p0 + PxVec3(-4, 0, -6);
	PxVec3 p1offset(-10,0,0);
	platform = createPlatform(PxTransform(p1, PxQuat(PxIdentity)),
				   PxBoxGeometry(PxVec3(2,1,2)), 4.0f, p1offset, mPlatformMaterial);

	// moving platform triggered by contact
	PxVec3 p2 = p1 + p1offset + PxVec3(-4,2,0);
	PxVec3 p2offset(0,0,-10);
	platform = createPlatform(PxTransform(p2, PxQuat(PxIdentity)),
				   PxBoxGeometry(PxVec3(2,1,2)), 5.0f, p2offset, mPlatformMaterial);
	platform->setType(SampleCharacterClothPlatform::ePLATFORM_TYPE_TRIGGERED);

	// trampoline-like platform
	PxVec3 p3 = p2 + p2offset + PxVec3(-4,-3,0);
	PxVec3 p3offset(0,10, 0);
	platform = createPlatform(PxTransform(p3, PxQuat(PxIdentity)),
				   PxBoxGeometry(PxVec3(1,1,1)), 5.0f, p3offset, mRockMaterial);
	platform->setType(SampleCharacterClothPlatform::ePLATFORM_TYPE_TRIGGERED);

	// a wide deck with a hole
	PxVec3 p4 = p3 + PxVec3(-10, 5, -4);
	PxVec3 p4offset(0.0f);

	platform = createPlatform(PxTransform(p4+PxVec3(0,0,-4), PxQuat(PxIdentity)),
				   PxBoxGeometry(PxVec3(6,0.5,2)), 5.0f, p4offset, mWoodMaterial);
	platform = createPlatform(PxTransform(p4+PxVec3(0,0,4), PxQuat(PxIdentity)),
				   PxBoxGeometry(PxVec3(6,0.5,2)), 5.0f, p4offset, mWoodMaterial);

	platform = createPlatform(PxTransform(p4+PxVec3(-4,0,0), PxQuat(PxIdentity)),
				   PxBoxGeometry(PxVec3(2,0.5,2)), 5.0f, p4offset, mWoodMaterial);
	platform = createPlatform(PxTransform(p4+PxVec3(4,0,0), PxQuat(PxIdentity)),
				   PxBoxGeometry(PxVec3(2,0.5,2)), 5.0f, p4offset, mWoodMaterial);

	// walls
	platform = createPlatform(PxTransform(p4+PxVec3(0,5.5,-6), PxQuat(PxIdentity)),
				   PxBoxGeometry(PxVec3(6,6,0.5)), 1.0f, p4offset, mRockMaterial);
	platform = createPlatform(PxTransform(p4+PxVec3(-6,5.5,0), PxQuat(PxIdentity)),
				   PxBoxGeometry(PxVec3(0.5,6,6)), 1.0f, p4offset, mRockMaterial);

	// moving platform under the deck
	PxVec3 p5 = p4 + PxVec3(0,-6,0);
	PxVec3 p5offset(15,0,-15);
	platform = createPlatform(PxTransform(p5, PxQuat(PxIdentity)),
				   PxBoxGeometry(PxVec3(3,1,3)), 5.0f, p5offset, mPlatformMaterial);

	// steps made of spheres
	PxVec3 sphere0 = p5 + p5offset + PxVec3(4,1,-4);
	platform = createPlatform(PxTransform(sphere0, PxQuat(PxIdentity)),
				  PxSphereGeometry(1.5f), 3.0f, PxVec3(0), mRockMaterial);

	PxVec3 sphere1 = sphere0 + PxVec3(4,1,-3);
	platform = createPlatform(PxTransform(sphere1, PxQuat(PxIdentity)),
				  PxSphereGeometry(1.5f), 3.0f, PxVec3(0), mRockMaterial);

	PxVec3 sphere2 = sphere1 + PxVec3(-2,1,-4);
	platform = createPlatform(PxTransform(sphere2, PxQuat(PxIdentity)),
				  PxSphereGeometry(1.5f), 3.0f, PxVec3(0), mRockMaterial);

	PxVec3 sphere3 = sphere2 + PxVec3(-5,1,1);
	platform = createPlatform(PxTransform(sphere3, PxQuat(PxIdentity)),
				  PxSphereGeometry(1.5f), 3.0f, PxVec3(0), mRockMaterial);

	PxVec3 sphere4 = sphere3 + PxVec3(-2,1,4);
	platform = createPlatform(PxTransform(sphere4, PxQuat(PxIdentity)),
				  PxSphereGeometry(1.5f), 3.0f, PxVec3(0), mRockMaterial);

	PxVec3 sphere5 = sphere4 + PxVec3(3,1,1);
	platform = createPlatform(PxTransform(sphere5, PxQuat(PxIdentity)),
				  PxSphereGeometry(1.5f), 3.0f, PxVec3(0), mRockMaterial);

	PxVec3 sphere6 = sphere5 + PxVec3(3,1,-3);
	platform = createPlatform(PxTransform(sphere6, PxQuat(PxIdentity)),
				  PxSphereGeometry(1.5f), 3.0f, PxVec3(0), mRockMaterial);


	// capsule platform
	PxVec3 capsule0 = sphere6 + PxVec3(12, 1, -12);
	platform = createPlatform(PxTransform(capsule0, PxQuat(PxPi / 4.0f, PxVec3(0,1.0f, 0.0f))),
				  PxCapsuleGeometry(1.5f,12.0f), 2.0f, PxVec3(0.0f), mSnowMaterial);

	// second capsule base
	PxVec3 capsule1 = capsule0 + PxVec3(12, 0, -22);
	platform = createPlatform(PxTransform(capsule1, PxQuat(PxPi / 2.0f, PxVec3(0,1.0f, 0.0f))),
				  PxCapsuleGeometry(1.5f,10.0f), 2.0f, PxVec3(0.0f), mPlatformMaterial);

	PxVec3 csphere0 = capsule1 + PxVec3(3,0.5,10);
	platform = createPlatform(PxTransform(csphere0, PxQuat(PxIdentity)),
				  PxSphereGeometry(1.5f), 1.0f, PxVec3(-6,0,0), mPlatformMaterial);

	PxVec3 csphere1 = capsule1 + PxVec3(-3,0.5,6);
	platform = createPlatform(PxTransform(csphere1, PxQuat(PxIdentity)),
				  PxSphereGeometry(1.5f), 1.0f, PxVec3(6,0,0), mPlatformMaterial);

	PxVec3 csphere2 = capsule1 + PxVec3(-3,0.5,2);
	platform = createPlatform(PxTransform(csphere2, PxQuat(PxIdentity)),
				  PxSphereGeometry(1.5f), 1.0f, PxVec3(6,0,0), mPlatformMaterial);

	PxVec3 csphere3 = capsule1 + PxVec3(3,0.5,-2);
	platform = createPlatform(PxTransform(csphere3, PxQuat(PxIdentity)),
				  PxSphereGeometry(1.5f), 1.0f, PxVec3(-6,0,0), mPlatformMaterial);

	PxVec3 csphere4 = capsule1 + PxVec3(3,0.5,-6);
	platform = createPlatform(PxTransform(csphere4, PxQuat(PxIdentity)),
				  PxSphereGeometry(1.5f), 1.0f, PxVec3(-6,0,0), mPlatformMaterial);

	PxVec3 csphere5 = capsule1 + PxVec3(-3,0.5,-10);
	platform = createPlatform(PxTransform(csphere5, PxQuat(PxIdentity)),
				  PxSphereGeometry(1.5f), 1.0f, PxVec3(6,0,0), mPlatformMaterial);

	// jump deck
	PxVec3 jumpDeck = capsule1 + PxVec3(0,1, 0) + PxVec3(-10,0,-10);
	platform = createPlatform(PxTransform(jumpDeck, PxQuat(-PxPi / 4.0f, PxVec3(0,1.0f, 0.0f))),
				   PxBoxGeometry(PxVec3(6,1,2)), 5.0f, PxVec3(0.0f), mWoodMaterial);

	// wind tunnel
	PxVec3 windTunnel = jumpDeck + PxVec3(0,0,0) + PxVec3(-6,0,-6);
	platform = createPlatform(PxTransform(windTunnel, PxQuat(-PxPi / 4.0f, PxVec3(0,1.0f, 0.0f))),
				   PxBoxGeometry(PxVec3(2,0.5,2)), 5.0f, PxVec3(0.0f), mPlatformMaterial);

	// landing deck
	PxVec3 landingDeck = jumpDeck + PxVec3(0,2, 0) + PxVec3(-15,0,-15);
	platform = createPlatform(PxTransform(landingDeck, PxQuat(-PxPi / 4.0f, PxVec3(0,1.0f, 0.0f))),
				   PxBoxGeometry(PxVec3(4,1,2)), 5.0f, PxVec3(0.0f), mRockMaterial);

	// reset platforms
	const size_t nbPlatforms = mPlatforms.size();
	for(PxU32 i = 0;i < nbPlatforms;i++)
		mPlatforms[i]->reset();

	// debugging cheat to start from later levels
//	mControllerInitialPosition = jumpDeck + PxVec3(0,5,0);

}
