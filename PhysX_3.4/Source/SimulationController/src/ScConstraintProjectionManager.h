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


#ifndef PX_PHYSICS_SCP_CONSTRAINT_PROJECTION_MANAGER
#define PX_PHYSICS_SCP_CONSTRAINT_PROJECTION_MANAGER

#include "PsPool.h"
#include "PsHashSet.h"
#include "ScConstraintGroupNode.h"

namespace physx
{
	class PxcScratchAllocator;

namespace Sc
{
	class ConstraintSim;
	class BodySim;
	template<typename T, const PxU32 elementsPerBlock = 64> class ScratchAllocatorList;

	class ConstraintProjectionManager : public Ps::UserAllocated
	{
	public:
		ConstraintProjectionManager();
		~ConstraintProjectionManager() {}

		void addToPendingGroupUpdates(ConstraintSim& s);
		void removeFromPendingGroupUpdates(ConstraintSim& s);

		void addToPendingTreeUpdates(ConstraintGroupNode& n);
		void removeFromPendingTreeUpdates(ConstraintGroupNode& n);

		void processPendingUpdates(PxcScratchAllocator&);
		void invalidateGroup(ConstraintGroupNode& node, ConstraintSim* constraintDeleted);

	private:
		PX_INLINE Sc::ConstraintGroupNode* createGroupNode(BodySim& b);

		void addToGroup(BodySim& b, BodySim* other, ConstraintSim& c);
		void groupUnion(ConstraintGroupNode& root0, ConstraintGroupNode& root1);
		void markConnectedConstraintsForUpdate(BodySim& b, ConstraintSim* c);
		PX_FORCE_INLINE void processConstraintForGroupBuilding(ConstraintSim* c, ScratchAllocatorList<ConstraintSim*>&);


	private:
		Ps::Pool<ConstraintGroupNode>				mNodePool;
		Ps::CoalescedHashSet<ConstraintSim*>		mPendingGroupUpdates; //list of constraints for which constraint projection groups need to be generated/updated
		Ps::CoalescedHashSet<ConstraintGroupNode*>	mPendingTreeUpdates;	//list of constraint groups that need their projection trees rebuilt. Note: non of the
																			//constraints in those groups are allowed to be in mPendingGroupUpdates at the same time
																			//because a group update will automatically trigger tree rebuilds.
	};

} // namespace Sc

}

#endif
