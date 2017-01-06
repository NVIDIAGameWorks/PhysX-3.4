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

#include "PxsSimulationController.h"

#ifndef SC_SIMULATION_CONTROLLER_H
#define	SC_SIMULATION_CONTROLLER_H


namespace physx
{

class PxsHeapMemoryAllocator;

namespace Sc
{
	class SimulationController : public PxsSimulationController
	{
		PX_NOCOPY(SimulationController)
	public:
		SimulationController(PxsSimulationControllerCallback* callback): PxsSimulationController(callback)
		{
		}
	
		virtual ~SimulationController(){}
		virtual void addJoint(const PxU32 /*edgeIndex*/, Dy::Constraint* /*constraint*/, IG::IslandSim& /*islandSim*/, Ps::Array<PxU32, Ps::VirtualAllocator>& /*jointIndices*/,
		Ps::Array<PxgSolverConstraintManagerConstants, Ps::VirtualAllocator>& /*managerIter*/, PxU32 /*uniqueId*/){}
		virtual void removeJoint(const PxU32 /*edgeIndex*/, Dy::Constraint* /*constraint*/, Ps::Array<PxU32, Ps::VirtualAllocator>& /*jointIndices*/, IG::IslandSim& /*islandSim*/){}
		virtual void addShape(PxsShapeSim* /*shapeSim*/, const PxU32 /*index*/){}
		virtual void removeShape(const PxU32 /*index*/){}
		virtual void addDynamic(PxsRigidBody* /*rigidBody*/, const PxU32 /*nodeIndex*/){}
		virtual void addDynamics(PxsRigidBody** /*rigidBody*/, const PxU32* /*nodeIndex*/, PxU32 /*nbBodies*/) {}
		virtual void updateJoint(const PxU32 /*edgeIndex*/, Dy::Constraint* /*constraint*/){}
		virtual void updateBodies(PxsRigidBody** /*rigidBodies*/,  PxU32* /*nodeIndices*/, const PxU32 /*nbBodies*/) {}
		virtual void updateBody(PxsRigidBody* /*rigidBody*/, const PxU32 /*nodeIndex*/) {}
		virtual void updateBodiesAndShapes(PxBaseTask* /*continuation*/, bool /*extrudeHeightfields*/){}
		virtual void update(const PxU32 /*bitMapWordCounts*/){}
		virtual void gpuDmabackData(PxsTransformCache& /*cache*/, Bp::BoundsArray& /*boundArray*/, Cm::BitMapPinned&  /*changedAABBMgrHandles*/){}
		virtual void udpateScBodyAndShapeSim(PxsTransformCache& cache, Bp::BoundsArray& boundArray, PxBaseTask* continuation);
		virtual PxU32* getActiveBodies() { return NULL; }
		virtual PxU32* getDeactiveBodies() { return NULL; }
		virtual PxsBodySim* getBodySims() { return NULL; }
		virtual PxU32	getNbBodies() { return 0; }
		virtual PxU32	getNbFrozenShapes() { return 0; }
		virtual PxU32	getNbUnfrozenShapes() { return 0; }

		virtual PxU32*	getUnfrozenShapes() { return NULL; }
		virtual PxU32*	getFrozenShapes() { return NULL; }
		virtual PxsShapeSim** getShapeSims() { return NULL; }
		virtual PxU32	getNbShapes()	{ return 0; }

		virtual void	clear() { }
		virtual void	setBounds(Bp::BoundsArray* /*boundArray*/){}
		virtual void	reserve(const PxU32 /*nbBodies*/) {}
		
	};
}

}

#endif
