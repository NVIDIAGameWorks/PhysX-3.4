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


#ifndef PX_PHYSICS_SCB_SCENE_BUFFER
#define PX_PHYSICS_SCB_SCENE_BUFFER

#include "CmPhysXCommon.h"

#include "ScScene.h"

#define PxClientBehaviorFlag_eNOT_BUFFERED	PxClientBehaviorFlags(0xff)

namespace physx
{
namespace Scb
{

struct SceneBuffer
{
public:
	static const PxU32 sMaxNbDominanceGroups = 32;

	PX_INLINE SceneBuffer();

	PX_INLINE void clearDominanceBuffer();
	PX_INLINE void setDominancePair(PxU32 group1, PxU32 group2, const PxDominanceGroupPair& dominance);
	PX_INLINE bool getDominancePair(PxU32 group1, PxU32 group2, PxDominanceGroupPair& dominance) const;
	PX_INLINE void syncDominancePairs(Sc::Scene& scene);

	PX_INLINE void clearVisualizationParams();

	PxReal								mVisualizationParam[PxVisualizationParameter::eNUM_VALUES];
	PxU8								mVisualizationParamChanged[PxVisualizationParameter::eNUM_VALUES];
	PxBounds3							mVisualizationCullingBox;
private:
	PxU32								mDominancePairFlag[sMaxNbDominanceGroups - 1];
	PxU32								mDominancePairValues[sMaxNbDominanceGroups];
public:
	PxVec3								mGravity;
	PxReal								mBounceThresholdVelocity;
	PxSceneFlags						mFlags;
	PxU32								mSolverBatchSize;
	PxU32								mNumClientsCreated;
	Ps::Array<PxClientBehaviorFlags>	mClientBehaviorFlags;	//a value is buffered if it is not -1.
};

PX_INLINE SceneBuffer::SceneBuffer() :
	mNumClientsCreated	(0),
	mClientBehaviorFlags(PX_DEBUG_EXP("clientBehaviorFlags"))
{
	clearDominanceBuffer();
	clearVisualizationParams();
	mClientBehaviorFlags.pushBack(PxClientBehaviorFlag_eNOT_BUFFERED);	//need member for default client, PxClientBehaviorFlag_eNOT_BUFFERED means its not storing anything.
}

PX_FORCE_INLINE void SceneBuffer::clearDominanceBuffer()
{
	PxMemZero(&mDominancePairFlag, (sMaxNbDominanceGroups - 1) * sizeof(PxU32));
}

PX_FORCE_INLINE void SceneBuffer::clearVisualizationParams()
{
	PxMemZero(mVisualizationParamChanged, PxVisualizationParameter::eNUM_VALUES * sizeof(PxU8));
}

PX_INLINE void SceneBuffer::setDominancePair(PxU32 group1, PxU32 group2, const PxDominanceGroupPair& dominance)
{
	PX_ASSERT(group1 != group2);
	PX_ASSERT(group1 < sMaxNbDominanceGroups);
	PX_ASSERT(group2 < sMaxNbDominanceGroups);

	if(group1 < group2)
		mDominancePairFlag[group1] |= (1 << group2);
	else
		mDominancePairFlag[group2] |= (1 << group1);

	if(dominance.dominance0 != 0.0f)
		mDominancePairValues[group1] |= (1 << group2);
	else
		mDominancePairValues[group1] &= ~(1 << group2);

	if(dominance.dominance1 != 0.0f)
		mDominancePairValues[group2] |= (1 << group1);
	else
		mDominancePairValues[group2] &= ~(1 << group1);
}

PX_INLINE bool SceneBuffer::getDominancePair(PxU32 group1, PxU32 group2, PxDominanceGroupPair& dominance) const
{
	PX_ASSERT(group1 != group2);
	PX_ASSERT(group1 < sMaxNbDominanceGroups);
	PX_ASSERT(group2 < sMaxNbDominanceGroups);

	PxU32 isBuffered;
	if(group1 < group2)
		isBuffered = mDominancePairFlag[group1] & (1 << group2);
	else
		isBuffered = mDominancePairFlag[group2] & (1 << group1);

	if(!isBuffered)
		return false;

	dominance.dominance0 = PxU8((mDominancePairValues[group1] & (1 << group2)) >> group2);
	dominance.dominance1 = PxU8((mDominancePairValues[group2] & (1 << group1)) >> group1);
	return true;
}

PX_INLINE void SceneBuffer::syncDominancePairs(Sc::Scene& scene)
{
	for(PxU32 i=0; i<(sMaxNbDominanceGroups - 1); i++)
	{
		if(mDominancePairFlag[i])
		{
			for(PxU32 j=(i+1); j<sMaxNbDominanceGroups; j++)
			{
				PxDominanceGroupPair dominance(0, 0);
				if(getDominancePair(i, j, dominance))
					scene.setDominanceGroupPair(PxDominanceGroup(i), PxDominanceGroup(j), dominance);
			}
		}
	}

	clearDominanceBuffer();
}


}  // namespace Scb

}

#endif
