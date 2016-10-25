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

	PxReal								visualizationParam[PxVisualizationParameter::eNUM_VALUES];
	PxU8								visualizationParamChanged[PxVisualizationParameter::eNUM_VALUES];
	PxBounds3							visualizationCullingBox;
	PxU8								visualizationCullingBoxChanged;
	PxU32								dominancePairFlag[sMaxNbDominanceGroups - 1];
	PxU32								dominancePairValues[sMaxNbDominanceGroups];
	PxVec3								gravity;
	PxReal								bounceThresholdVelocity;
	PxSceneFlags						flags;
	PxU32								solverBatchSize;
	PxU32								numClientsCreated;
	Ps::Array<PxClientBehaviorFlags>	clientBehaviorFlags;	//a value is buffered if it is not -1.
};


PX_INLINE SceneBuffer::SceneBuffer() : clientBehaviorFlags(PX_DEBUG_EXP("clientBehaviorFlags"))
{
	clearDominanceBuffer();
	clearVisualizationParams();
	numClientsCreated = 0;
	clientBehaviorFlags.pushBack(PxClientBehaviorFlag_eNOT_BUFFERED);	//need member for default client, PxClientBehaviorFlag_eNOT_BUFFERED means its not storing anything.
}


PX_INLINE void SceneBuffer::clearDominanceBuffer()
{
	PxMemSet(&dominancePairFlag, 0, (sMaxNbDominanceGroups - 1) * sizeof(PxU32));
}


PX_INLINE void SceneBuffer::clearVisualizationParams()
{
	PxMemZero(visualizationParamChanged, PxVisualizationParameter::eNUM_VALUES * sizeof(PxU8));
}


PX_INLINE void SceneBuffer::setDominancePair(PxU32 group1, PxU32 group2, const PxDominanceGroupPair& dominance)
{
	PX_ASSERT(group1 != group2);
	PX_ASSERT(group1 < sMaxNbDominanceGroups);
	PX_ASSERT(group2 < sMaxNbDominanceGroups);

	if (group1 < group2)
		dominancePairFlag[group1] = dominancePairFlag[group1] | (1 << group2);
	else
		dominancePairFlag[group2] = dominancePairFlag[group2] | (1 << group1);

	if (dominance.dominance0 != 0.0f)
		dominancePairValues[group1] = dominancePairValues[group1] | (1 << group2);
	else
		dominancePairValues[group1] = dominancePairValues[group1] & (~(1 << group2));

	if (dominance.dominance1 != 0.0f)
		dominancePairValues[group2] = dominancePairValues[group2] | (1 << group1);
	else
		dominancePairValues[group2] = dominancePairValues[group2] & (~(1 << group1));
}


PX_INLINE bool SceneBuffer::getDominancePair(PxU32 group1, PxU32 group2, PxDominanceGroupPair& dominance) const
{
	PX_ASSERT(group1 != group2);
	PX_ASSERT(group1 < sMaxNbDominanceGroups);
	PX_ASSERT(group2 < sMaxNbDominanceGroups);

	PxU32 isBuffered = 0;
	if (group1 < group2)
		isBuffered = dominancePairFlag[group1] & (1 << group2);
	else
		isBuffered = dominancePairFlag[group2] & (1 << group1);

	if (isBuffered)
	{
		dominance.dominance0 = PxU8((dominancePairValues[group1] & (1 << group2)) >> group2 );
		dominance.dominance1 = PxU8((dominancePairValues[group2] & (1 << group1)) >> group1 );
		return true;
	}
	
	return false;
}


PX_INLINE void SceneBuffer::syncDominancePairs(Sc::Scene& scene)
{
	for(PxU32 i=0; i < (sMaxNbDominanceGroups - 1); i++)
	{
		if (dominancePairFlag[i])
		{
			for(PxU32 j=(i+1); j < sMaxNbDominanceGroups; j++)
			{
				PxDominanceGroupPair dominance(0, 0);
				if (getDominancePair(i, j, dominance))
				{
					scene.setDominanceGroupPair(PxDominanceGroup(i), PxDominanceGroup(j), dominance);
				}
			}
		}
	}
}


}  // namespace Scb

}

#endif
