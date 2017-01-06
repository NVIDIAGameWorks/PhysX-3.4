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


#ifndef PX_PHYSICS_SCP_CONSTRAINT_PROJECTION_TREE
#define PX_PHYSICS_SCP_CONSTRAINT_PROJECTION_TREE

#include "PsArray.h"
#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"

namespace physx
{
namespace Sc
{
	struct ConstraintGroupNode;
	class ConstraintSim;
	class BodySim;
	class BodyRank;

	class ConstraintProjectionTree
	{
		/**
		 This class serves both the static administration of an articulation and the actual articulation itself.
		 An Articulation object holds several articulation root nodes which make up a simulation island that
		 is further connected with lagrange joints.
		*/
		public:
			ConstraintProjectionTree() {}
			~ConstraintProjectionTree() {}

			static	void		buildProjectionTrees(ConstraintGroupNode& root);	
			static	void		purgeProjectionTrees(ConstraintGroupNode& root);

			static void			projectPose(ConstraintGroupNode& root, Ps::Array<BodySim*>& projectedBodies);

		private:
			static	PxU32					projectionTreeBuildStep(ConstraintGroupNode& node, ConstraintSim* cToParent, ConstraintGroupNode** nodeStack);

			static	void					getConstraintStatus(const ConstraintSim& c, const BodySim* b, BodySim*& otherBody, PxU32& projectToBody, PxU32& projectToOtherBody);
			static	void					rankConstraint(ConstraintSim&, BodyRank&, PxU32& dominanceTracking, PxU32& constraintsToProjectCount);
			static	void					projectPoseForTree(ConstraintGroupNode& node, Ps::Array<BodySim*>& projectedBodies);
	};

} // namespace Sc

}

#endif
