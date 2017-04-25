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


#ifndef PXS_NPHASE_IMPLEMENTATION_CONTEXT_H
#define PXS_NPHASE_IMPLEMENTATION_CONTEXT_H

#include "PxvNphaseImplementationContext.h" 
#include "PxsContactManagerState.h"
#include "PxcNpCache.h"

namespace physx
{

struct PxsContactManagers : PxsContactManagerBase
{
	Ps::Array<PxsContactManagerOutput>			mOutputContactManagers;
	Ps::Array<PxsContactManager*>				mContactManagerMapping;
	Ps::Array<Gu::Cache>						mCaches;


	PxsContactManagers(const PxU32 bucketId) : PxsContactManagerBase(bucketId),
		mOutputContactManagers(PX_DEBUG_EXP("mOutputContactManagers")),
		mContactManagerMapping(PX_DEBUG_EXP("mContactManagerMapping")),
		mCaches(PX_DEBUG_EXP("mCaches"))
	{
	}
		
	void clear()
	{
		mOutputContactManagers.forceSize_Unsafe(0);
		mContactManagerMapping.forceSize_Unsafe(0);
		mCaches.forceSize_Unsafe(0);
		
	}
private:
	PX_NOCOPY(PxsContactManagers)
};


class PxsNphaseImplementationContext: public PxvNphaseImplementationContextUsableAsFallback
{
public:
	static PxsNphaseImplementationContext*	create(PxsContext& context, IG::IslandSim* islandSim);

	PxsNphaseImplementationContext(PxsContext& context, IG::IslandSim* islandSim, PxU32 index = 0): PxvNphaseImplementationContextUsableAsFallback(context), mNarrowPhasePairs(index), mNewNarrowPhasePairs(index),
										mModifyCallback(NULL), mIslandSim(islandSim) {}
	virtual void				destroy();
	virtual void				updateContactManager(PxReal dt, bool hasBoundsArrayChanged, bool hasContactDistanceChanged, PxBaseTask* continuation, PxBaseTask* firstPassContinuation);
	virtual void				postBroadPhaseUpdateContactManager() {}
	virtual void				secondPassUpdateContactManager(PxReal dt, PxBaseTask* continuation);

	virtual void				registerContactManager(PxsContactManager* cm, PxI32 touching, PxU32 numPatches);
	virtual void				registerContactManagers(PxsContactManager** cm, PxU32 nbContactManagers, PxU32 maxContactManagerId);
	virtual void				unregisterContactManager(PxsContactManager* cm);
	virtual void				unregisterContactManagerFallback(PxsContactManager* cm, PxsContactManagerOutput* cmOutputs);

	
	virtual void				refreshContactManager(PxsContactManager* cm);
	virtual void				refreshContactManagerFallback(PxsContactManager* cm, PxsContactManagerOutput* cmOutputs);

	virtual void				registerShape(const PxsShapeCore& shapeCore);

	virtual void				updateShapeMaterial(const PxsShapeCore& shapeCore);
	virtual void				updateShapeContactOffset(const PxsShapeCore& shapeCore);

	virtual void				unregisterShape(const PxsShapeCore& shapeCore);

	virtual void				registerMaterial(const PxsMaterialCore& materialCore);
	virtual void				updateMaterial(const PxsMaterialCore& materialCore);
	virtual void				unregisterMaterial(const PxsMaterialCore& materialCore);

	virtual void				appendContactManagers();
	virtual void				appendContactManagersFallback(PxsContactManagerOutput* cmOutputs);

	virtual void				removeContactManagersFallback(PxsContactManagerOutput* cmOutputs);

	virtual void				setContactModifyCallback(PxContactModifyCallback* callback) { mModifyCallback = callback; }

	virtual PxsContactManagerOutputIterator getContactManagerOutputs();

	virtual PxsContactManagerOutput& getNewContactManagerOutput(PxU32 npIndex);

	virtual PxsContactManagerOutput*	getGPUContactManagerOutputBase() { return NULL; }

	virtual void							acquireContext(){}
	virtual void							releaseContext(){}
	virtual void				preallocateNewBuffers(PxU32 /*nbNewPairs*/, PxU32 /*maxIndex*/) { /*TODO - implement if it's useful to do so*/}

	void						processContactManager(PxReal dt, PxsContactManagerOutput* cmOutputs, PxBaseTask* continuation);
	void						processContactManagerSecondPass(PxReal dt, PxBaseTask* continuation);
	void						fetchUpdateContactManager() {}

	

	void						startNarrowPhaseTasks() {}

	

	Ps::Array<PxU32>			mRemovedContactManagers;
	PxsContactManagers			mNarrowPhasePairs;
	PxsContactManagers			mNewNarrowPhasePairs;

	PxContactModifyCallback*	mModifyCallback;

	IG::IslandSim*				mIslandSim;

private:

	void						unregisterContactManagerInternal(PxU32 npIndex, PxsContactManagers& managers, PxsContactManagerOutput* cmOutputs);

	PX_NOCOPY(PxsNphaseImplementationContext)
};

}

#endif
