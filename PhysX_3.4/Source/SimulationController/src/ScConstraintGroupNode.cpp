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


#include "ScConstraintGroupNode.h"
#include "ScConstraintProjectionManager.h"
#include "PsFoundation.h"
#include "ScBodySim.h"
#include "ScConstraintSim.h"
#include "ScConstraintInteraction.h"

using namespace physx;

Sc::ConstraintGroupNode::ConstraintGroupNode(BodySim& b) : 
	body(&b),
	parent(this),
	tail(this),
	rank(0),
	next(NULL),

	projectionFirstRoot(NULL),
	projectionNextRoot(NULL),
	projectionParent(NULL),
	projectionFirstChild(NULL),
	projectionNextSibling(NULL),
	projectionConstraint(NULL),

	flags(0)
{
}


//
// Implementation of FIND of 
// UNION-FIND algo.
//
Sc::ConstraintGroupNode& Sc::ConstraintGroupNode::getRoot()
{
	PX_ASSERT(parent);

	ConstraintGroupNode* root = parent;

	if (root->parent == root)
		return *root;
	else
	{
		PxU32 nbHops = 1;
		root = root->parent;

		while(root != root->parent)
		{
			root = root->parent;
			nbHops++;
		}

		// Write root to all nodes on the path
		ConstraintGroupNode* curr = this;
		while(nbHops)
		{
			ConstraintGroupNode* n = curr->parent;
			curr->parent = root;
			curr = n;
			nbHops--;
		}

		return *root;
	}
}


void Sc::ConstraintGroupNode::markForProjectionTreeRebuild(ConstraintProjectionManager& cpManager)
{
	ConstraintGroupNode& root = getRoot();
	if (!root.readFlag(ConstraintGroupNode::ePENDING_TREE_UPDATE))
	{
		cpManager.addToPendingTreeUpdates(root);
	}
}


void Sc::ConstraintGroupNode::initProjectionData(ConstraintGroupNode* parent_, ConstraintSim* c)
{
	projectionConstraint = c;

	//add us to parent's child list:
	if (parent_)
	{
		projectionNextSibling = parent_->projectionFirstChild;
		parent_->projectionFirstChild = this;

		projectionParent = parent_;
	}
}


void Sc::ConstraintGroupNode::clearProjectionData()
{
	projectionFirstRoot = NULL;
	projectionNextRoot = NULL;
	projectionParent = NULL;
	projectionFirstChild = NULL;
	projectionNextSibling = NULL;
	projectionConstraint = NULL;
}


void Sc::ConstraintGroupNode::projectPose(ConstraintGroupNode& node, Ps::Array<BodySim*>& projectedBodies)
{
	PX_ASSERT(node.hasProjectionTreeRoot());

	Sc::ConstraintProjectionTree::projectPose(node, projectedBodies);
}
