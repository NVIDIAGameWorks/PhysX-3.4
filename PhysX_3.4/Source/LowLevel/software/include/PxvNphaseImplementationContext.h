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


#ifndef PXV_NPHASE_IMPLEMENTATION_CONTEXT_H
#define PXV_NPHASE_IMPLEMENTATION_CONTEXT_H

#include "PxSceneDesc.h"
#include "PxsContactManagerState.h"
#include "PsArray.h"

#if PX_SUPPORT_GPU_PHYSX
#include "Pxg.h"
#endif

namespace physx
{

namespace IG
{
	class SimpleIslandManager;
	class IslandSim;
	typedef PxU32 EdgeIndex;
}

namespace Dy
{
	class Context;
}

class PxBaseTask;
class PxsContext;
struct PxsShapeCore;
class PxsMaterialCore;
struct PxgDynamicsMemoryConfig;
class PxsContactManager;
struct PxsContactManagerOutput;
class PxsKernelWranglerManager;
class PxsHeapMemoryAllocatorManager;


struct PxsContactManagerBase
{
	static const PxU32 NEW_CONTACT_MANAGER_MASK = 0x80000000;
	static const PxU32 GPU_NP_OFFSET = 0x4;

	static const PxU32 MaxBucketBits = 3;

	const PxU32									mBucketId;

	PxsContactManagerBase(const PxU32 bucketId) : mBucketId(bucketId)
	{
		PX_ASSERT(bucketId < (1<<MaxBucketBits));
	}


	PX_FORCE_INLINE PxU32 computeId(const PxU32 index) const { PX_ASSERT(index < PxU32(1 << (32 - (MaxBucketBits-1)))); return (index << MaxBucketBits) | (mBucketId); }
	static PX_FORCE_INLINE PxU32 computeIndexFromId(const PxU32 id) { return id >> MaxBucketBits; }
	static PX_FORCE_INLINE PxU32 computeBucketIndexFromId(const PxU32 id) { return id & ((1<<MaxBucketBits)-1); }

private:
	PX_NOCOPY(PxsContactManagerBase)
};

class PxsContactManagerOutputIterator
{
	PxU32 mOffsets[1<<PxsContactManagerBase::MaxBucketBits];
	PxsContactManagerOutput* mOutputs;

public:

	PxsContactManagerOutputIterator() : mOutputs(NULL)
	{
	}

	PxsContactManagerOutputIterator(PxU32* offsets, PxU32 nbOffsets, PxsContactManagerOutput* outputs) : mOutputs(outputs)
	{
		PX_ASSERT(nbOffsets <= (1<<PxsContactManagerBase::MaxBucketBits));

		for(PxU32 a = 0; a < nbOffsets; ++a)
		{
			mOffsets[a] = offsets[a];
		}
	}

	PX_FORCE_INLINE PxsContactManagerOutput& getContactManager(PxU32 id)
	{
		PX_ASSERT((id & PxsContactManagerBase::NEW_CONTACT_MANAGER_MASK) == 0);
		PxU32 bucketId = PxsContactManagerBase::computeBucketIndexFromId(id);
		PxU32 cmOutId = PxsContactManagerBase::computeIndexFromId(id);
		return mOutputs[mOffsets[bucketId] + cmOutId];
	}

	PxU32 getIndex(PxU32 id)
	{
		PX_ASSERT((id & PxsContactManagerBase::NEW_CONTACT_MANAGER_MASK) == 0);
		PxU32 bucketId = PxsContactManagerBase::computeBucketIndexFromId(id);
		PxU32 cmOutId = PxsContactManagerBase::computeIndexFromId(id);
		return mOffsets[bucketId] + cmOutId;
	}
};


class PxvNphaseImplementationContext
{
	private:
												PX_NOCOPY(PxvNphaseImplementationContext)
public:
	
	PxvNphaseImplementationContext(PxsContext& context): mContext(context) {}
	virtual ~PxvNphaseImplementationContext() {}
	virtual void						destroy() = 0;
	virtual void						updateContactManager(PxReal dt, bool hasBoundsArrayChanged, bool hasContactDistanceChanged, PxBaseTask* continuation, PxBaseTask* firstPassContinuation) = 0;
	virtual void						postBroadPhaseUpdateContactManager() = 0;
	virtual void						secondPassUpdateContactManager(PxReal dt, PxBaseTask* continuation) = 0;
	virtual void						fetchUpdateContactManager() = 0;
	
	virtual void						registerContactManager(PxsContactManager* cm, PxI32 touching, PxU32 patchCount) = 0;
	virtual void						registerContactManagers(PxsContactManager** cm, PxU32 nbContactManagers, PxU32 maxContactManagerId) = 0;
	virtual void						unregisterContactManager(PxsContactManager* cm) = 0;
	virtual void						refreshContactManager(PxsContactManager* cm) = 0;

	virtual void						registerShape(const PxsShapeCore& shapeCore) = 0;
	virtual void						unregisterShape(const PxsShapeCore& shapeCore) = 0;

	virtual void						registerMaterial(const PxsMaterialCore& materialCore) = 0;
	virtual void						updateMaterial(const PxsMaterialCore& materialCore) = 0;
	virtual void						unregisterMaterial(const PxsMaterialCore& materialCore) = 0;

	virtual void						updateShapeMaterial(const PxsShapeCore& shapeCore) = 0;

	virtual PxsContactManagerOutput*	getGPUContactManagerOutputBase() = 0;
	
	virtual void						startNarrowPhaseTasks() = 0;

	virtual void						appendContactManagers() = 0;	

	virtual PxsContactManagerOutput&	getNewContactManagerOutput(PxU32 index) = 0;

	virtual PxsContactManagerOutputIterator	getContactManagerOutputs() = 0;

	virtual void							setContactModifyCallback(PxContactModifyCallback* callback) = 0;

	virtual void							acquireContext() = 0;
	virtual void							releaseContext() = 0;
	virtual void							preallocateNewBuffers(PxU32 nbNewPairs, PxU32 maxIndex) = 0;

	
			
		
protected:

	PxsContext&					mContext;
};

class PxvNphaseImplementationFallback
{
	private:
												PX_NOCOPY(PxvNphaseImplementationFallback)
public:
	
	PxvNphaseImplementationFallback() {}
	virtual ~PxvNphaseImplementationFallback() {}
	virtual void				processContactManager(PxReal dt, PxsContactManagerOutput* cmOutputs, PxBaseTask* continuation) = 0;
	virtual void				processContactManagerSecondPass(PxReal dt, PxBaseTask* continuation) = 0;

	virtual void				registerContactManager(PxsContactManager* cm, PxI32 touching, PxU32 numPatches) = 0;
	virtual void				unregisterContactManagerFallback(PxsContactManager* cm, PxsContactManagerOutput* cmOutputs) = 0;

	virtual void				refreshContactManagerFallback(PxsContactManager* cm, PxsContactManagerOutput* cmOutputs) = 0;

	virtual PxsContactManagerOutput& getNewContactManagerOutput(PxU32 npId) = 0;

	virtual void				appendContactManagersFallback(PxsContactManagerOutput* outputs) = 0;

	virtual void				setContactModifyCallback(PxContactModifyCallback* callback) = 0;

	virtual void				removeContactManagersFallback(PxsContactManagerOutput* cmOutputs) = 0;

};

class PxvNphaseImplementationContextUsableAsFallback: public PxvNphaseImplementationContext, public PxvNphaseImplementationFallback
{
	private:
												PX_NOCOPY(PxvNphaseImplementationContextUsableAsFallback)
public:
	PxvNphaseImplementationContextUsableAsFallback(PxsContext& context): PxvNphaseImplementationContext(context) {}
	virtual ~PxvNphaseImplementationContextUsableAsFallback() {}
};

PxvNphaseImplementationContextUsableAsFallback* createNphaseImplementationContext(PxsContext& context, IG::IslandSim* islandSim);

}

#endif
