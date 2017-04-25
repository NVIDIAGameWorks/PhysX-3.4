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

#ifndef PXS_SIMPLE_ISLAND_GEN_H
#define PXS_SIMPLE_ISLAND_GEN_H

#include "PxsIslandSim.h"
#include "CmTask.h"

namespace physx
{

namespace Sc
{
	class Interaction;
}
namespace IG
{

	class SimpleIslandManager;

class ThirdPassTask : public Cm::Task
{
	SimpleIslandManager& mIslandManager;
	IslandSim& mIslandSim;

public:

	ThirdPassTask(PxU64 contextID, SimpleIslandManager& islandManager, IslandSim& islandSim);

	virtual void runInternal();

	virtual const char* getName() const
	{
		return "ThirdPassIslandGenTask";
	}

private:
	PX_NOCOPY(ThirdPassTask)
};

class PostThirdPassTask : public Cm::Task
{
	SimpleIslandManager& mIslandManager;

public:

	PostThirdPassTask(PxU64 contextID, SimpleIslandManager& islandManager);

	virtual void runInternal();

	virtual const char* getName() const
	{
		return "PostThirdPassTask";
	}
private:
	PX_NOCOPY(PostThirdPassTask)
};

class SimpleIslandManager
{

	HandleManager<PxU32> mNodeHandles;						//! Handle manager for nodes
	HandleManager<EdgeIndex> mEdgeHandles;					//! Handle manager for edges

	//An array of destroyed nodes
	Ps::Array<NodeIndex> mDestroyedNodes;
	Ps::Array<Sc::Interaction*> mInteractions;
	

	//Edges destroyed this frame
	Ps::Array<EdgeIndex> mDestroyedEdges;
	Ps::Array<PartitionEdge*> mFirstPartitionEdges;
	Ps::Array<PartitionEdge*> mDestroyedPartitionEdges;
	//KS - stores node indices for a given edge. Node index 0 is at 2* edgeId and NodeIndex1 is at 2*edgeId + 1
	//can also be used for edgeInstance indexing so there's no need to figure out outboundNode ID either!
	Ps::Array<NodeIndex> mEdgeNodeIndices;

	Ps::Array<ConstraintOrContactManager> mConstraintOrCm;	//! Pointers to either the constraint or Cm for this pair

	Cm::BitMap mConnectedMap;

	IslandSim mIslandManager;
	IslandSim mSpeculativeIslandManager;

	ThirdPassTask mSpeculativeThirdPassTask;
	ThirdPassTask mAccurateThirdPassTask;

	PostThirdPassTask mPostThirdPassTask;
	PxU32 mMaxDirtyNodesPerFrame;

	PxU64	mContextID;
public:

	SimpleIslandManager(bool useEnhancedDeterminism, PxU64 contextID);

	~SimpleIslandManager();

	NodeIndex addRigidBody(PxsRigidBody* body, bool isKinematic, bool isActive);

	void removeNode(const NodeIndex index);

	NodeIndex addArticulation(Sc::ArticulationSim* articulation, Dy::Articulation* llArtic, bool isActive);

	EdgeIndex addContactManager(PxsContactManager* manager, NodeIndex nodeHandle1, NodeIndex nodeHandle2, Sc::Interaction* interaction);

	EdgeIndex addConstraint(Dy::Constraint* constraint, NodeIndex nodeHandle1, NodeIndex nodeHandle2, Sc::Interaction* interaction);

	bool isConnected(EdgeIndex edgeIndex) const { return !!mConnectedMap.test(edgeIndex); }

	PX_FORCE_INLINE NodeIndex getEdgeIndex(EdgeInstanceIndex edgeIndex) const { return mEdgeNodeIndices[edgeIndex]; }

	void activateNode(NodeIndex index);
	void deactivateNode(NodeIndex index);
	void putNodeToSleep(NodeIndex index);

	void removeConnection(EdgeIndex edgeIndex);
	
	void firstPassIslandGen();
	void additionalSpeculativeActivation();
	void secondPassIslandGen();
	void thirdPassIslandGen(PxBaseTask* continuation);

	void clearDestroyedEdges();

	void setEdgeConnected(EdgeIndex edgeIndex);
	void setEdgeDisconnected(EdgeIndex edgeIndex);

	bool getIsEdgeConnected(EdgeIndex edgeIndex);

	void setEdgeRigidCM(const EdgeIndex edgeIndex, PxsContactManager* cm);

	void clearEdgeRigidCM(const EdgeIndex edgeIndex);

	void setKinematic(IG::NodeIndex nodeIndex);

	void setDynamic(IG::NodeIndex nodeIndex);

	const IslandSim& getSpeculativeIslandSim() const { return mSpeculativeIslandManager; }
	const IslandSim& getAccurateIslandSim() const { return mIslandManager; }

	IslandSim& getAccurateIslandSim() { return mIslandManager; }

	PX_FORCE_INLINE PxU32 getNbEdgeHandles() const { return mEdgeHandles.getTotalHandles(); }

	PX_FORCE_INLINE PxU32 getNbNodeHandles() const { return mNodeHandles.getTotalHandles(); }

	void deactivateEdge(const EdgeIndex edge);

	PX_FORCE_INLINE PxsContactManager* getContactManager(IG::EdgeIndex edgeId) const { return mConstraintOrCm[edgeId].mCm; }
	PX_FORCE_INLINE PxsContactManager* getContactManagerUnsafe(IG::EdgeIndex edgeId) const { return mConstraintOrCm[edgeId].mCm; }
	PX_FORCE_INLINE Dy::Constraint* getConstraint(IG::EdgeIndex edgeId) const { return mConstraintOrCm[edgeId].mConstraint; }
	PX_FORCE_INLINE Dy::Constraint* getConstraintUnsafe(IG::EdgeIndex edgeId) const { return mConstraintOrCm[edgeId].mConstraint; }

	PX_FORCE_INLINE Sc::Interaction* getInteraction(IG::EdgeIndex edgeId) const { return mInteractions[edgeId]; }

	PX_FORCE_INLINE	PxU64			getContextId() const { return mContextID; }

	bool checkInternalConsistency();


private:

	friend class ThirdPassTask;
	friend class PostThirdPassTask;

	bool validateDeactivations() const;

	PX_NOCOPY(SimpleIslandManager)
};



}
}

#endif
