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


#include "ScClothShape.h"
#if PX_USE_CLOTH_API

#include "ScNPhaseCore.h"
#include "ScScene.h"

#include "ScClothSim.h"
#include "PxsContext.h"
#include "BpSimpleAABBManager.h"
#include "ScSqBoundsManager.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sc::ClothShape::ClothShape(ClothSim& cloth) :
	ElementSim(cloth, ElementType::eCLOTH),
    mClothCore(cloth.getCore()),
	mHasCollision(mClothCore.getClothFlags() & PxClothFlag::eSCENE_COLLISION)
{
	if (mHasCollision)
		createLowLevelVolume();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sc::ClothShape::~ClothShape()
{
	if (isInBroadPhase())
		destroyLowLevelVolume();

	PX_ASSERT(!isInBroadPhase());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sc::ClothShape::getFilterInfo(PxFilterObjectAttributes& filterAttr, PxFilterData& filterData) const
{
	filterAttr = 0;
	ElementSim::setFilterObjectAttributeType(filterAttr, PxFilterObjectType::eCLOTH);
	filterData = mClothCore.getSimulationFilterData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sc::ClothShape::updateBoundsInAABBMgr()
{
	if(~mClothCore.getClothFlags() & PxClothFlag::eSCENE_COLLISION)
	{
		if(mHasCollision)
		{
			destroyLowLevelVolume();
			mHasCollision = false;
		}
		return;
	}

	if(!mHasCollision)
	{
		createLowLevelVolume();
		mHasCollision = true;
	}

	Scene& scene = getScene();

	PxBounds3 worldBounds = mClothCore.getWorldBounds();
	worldBounds.fattenSafe(mClothCore.getContactOffset()); // fatten for fast moving colliders
    scene.getBoundsArray().setBounds(worldBounds, getElementID());
	scene.getAABBManager()->getChangedAABBMgActorHandleMap().growAndSet(getElementID());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sc::ClothShape::createLowLevelVolume()
{
	PX_ASSERT(getWorldBounds().isFinite());
	getScene().getBoundsArray().setBounds(getWorldBounds(), getElementID());	
	addToAABBMgr(0, Bp::FilterGroup::eCLOTH_NO_PARTICLE_INTERACTION, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sc::ClothShape::destroyLowLevelVolume()
{
	if (!isInBroadPhase())
		return;

	Sc::Scene& scene = getScene();
	PxsContactManagerOutputIterator outputs = scene.getLowLevelContext()->getNphaseImplementationContext()->getContactManagerOutputs();
	scene.getNPhaseCore()->onVolumeRemoved(this, 0, outputs, scene.getPublicFlags() & PxSceneFlag::eADAPTIVE_FORCE);
	removeFromAABBMgr();
}


#endif	// PX_USE_CLOTH_API
