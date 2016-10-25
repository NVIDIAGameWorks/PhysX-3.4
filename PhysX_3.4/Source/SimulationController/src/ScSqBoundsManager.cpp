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
// Copyright (c) 2008-2016 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "CmPhysXCommon.h"
#include "ScSqBoundsManager.h"
#include "ScBodySim.h"
#include "ScShapeSim.h"
#include "ScShapeIterator.h"
#include "CmTransformUtils.h"
#include "PxsTransformCache.h"
#include <stdio.h>

namespace physx
{
namespace Sc
{

	SqBoundsManager::SqBoundsManager() : 
		mShapes(PX_DEBUG_EXP("SqBoundsManager::mRefs")),
		mRefs(PX_DEBUG_EXP("SqBoundsManager::mRefs")),				
		mBoundsIndices(PX_DEBUG_EXP("SqBoundsManager::mRefs")),
		mRefless(PX_DEBUG_EXP("SqBoundsManager::mRefs"))

{
}

void SqBoundsManager::addShape(ShapeSim& shape)
{
	PX_ASSERT(shape.getFlags() & PxShapeFlag::eSCENE_QUERY_SHAPE);
	PX_ASSERT(!shape.getBodySim()->usingSqKinematicTarget());
	PX_ASSERT(!shape.getBodySim()->isFrozen());

	PxU32 id = mShapes.size();
	mShapes.pushBack(&shape);
	mRefs.pushBack(PX_INVALID_U32);
	mBoundsIndices.pushBack(shape.getElementID());
	mRefless.insert(&shape);

	shape.setSqBoundsId(id);
}
		
void SqBoundsManager::removeShape(ShapeSim& shape)
{
	PxU32 id = shape.getSqBoundsId();
	if(mRefs[id] == PX_INVALID_U32)
	{
		PX_ASSERT(mRefless.contains(&shape));
		mRefless.erase(&shape);
	}

	shape.setSqBoundsId(PX_INVALID_U32);
	mShapes[id] = mShapes.back();
	mBoundsIndices[id] = mBoundsIndices.back();
	mRefs[id] = mRefs.back();
		
	if(id+1 != mShapes.size())
		mShapes[id]->setSqBoundsId(id);

	mShapes.popBack();
	mRefs.popBack();
	mBoundsIndices.popBack();
}


void SqBoundsManager::syncBounds(SqBoundsSync& sync, SqRefFinder& finder, const PxBounds3* bounds, PxU64 contextID)
{
	PX_PROFILE_ZONE("Sim.sceneQuerySyncBounds", contextID);
	PX_UNUSED(contextID);

#if PX_DEBUG
	for(PxU32 i=0;i<mShapes.size();i++)
	{
		ShapeSim& shape = *mShapes[i];
		PX_UNUSED(shape);
		PX_ASSERT(shape.getFlags() & PxShapeFlag::eSCENE_QUERY_SHAPE);
		PX_ASSERT(!shape.getBodySim()->usingSqKinematicTarget());
		PX_ASSERT(!shape.getBodySim()->isFrozen());
	}
#endif

	ShapeSim*const * shapes = mRefless.getEntries();
	for(PxU32 i=0, size = mRefless.size();i<size;i++)
	{
		PxU32 id = shapes[i]->getSqBoundsId();
		PX_ASSERT(mRefs[id] == PX_INVALID_U32);
		mRefs[id] = finder.find(static_cast<PxRigidBody*>(shapes[i]->getBodySim()->getPxActor()), shapes[i]->getPxShape());
	}
	mRefless.clear();

	sync.sync(mRefs.begin(), mBoundsIndices.begin(), bounds, mShapes.size());
}


}

}
