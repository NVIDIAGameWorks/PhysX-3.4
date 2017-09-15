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


#ifndef PXS_SIMULATION_CONTROLLER_H
#define PXS_SIMULATION_CONTROLLER_H

#include "foundation/PxSimpleTypes.h"
#include "foundation/PxPreprocessor.h"
#include "foundation/PxTransform.h"
#include "CmBitMap.h"
#include "PsArray.h"


namespace physx
{
	namespace Dy
	{
		class Context;
		struct Constraint;
	}

	namespace Cm
	{
		class EventProfiler;
	}

	namespace Bp
	{
		class BoundsArray;
		class BroadPhase;
	}

	namespace IG
	{
		class SimpleIslandManager;
		class IslandSim;
	}

	class PxGpuDispatcher;
	class PxsTransformCache;
	class PxvNphaseImplementationContext;
	class PxBaseTask;

	struct PxsBodySim;
	struct PxsShapeSim;
	class PxsRigidBody;
	class PxsKernelWranglerManager;
	class PxsHeapMemoryAllocatorManager;

	template<typename T> class PxgIterator;
	struct PxgSolverConstraintManagerConstants;
	

	class PxsSimulationControllerCallback
	{
	public:
		virtual void updateScBodyAndShapeSim(PxBaseTask* continuation) = 0;
		virtual PxU32	getNbCcdBodies() = 0;

		virtual ~PxsSimulationControllerCallback() {}
	};


	class PxsSimulationController
	{
	public:
		PxsSimulationController(PxsSimulationControllerCallback* callback): mCallback(callback){}
		virtual ~PxsSimulationController(){}

		virtual void addJoint(const PxU32 edgeIndex, Dy::Constraint* constraint, IG::IslandSim& islandSim, Ps::Array<PxU32, Ps::VirtualAllocator>& jointIndices, 
			Ps::Array<PxgSolverConstraintManagerConstants, Ps::VirtualAllocator>& managerIter, PxU32 uniqueId) = 0;
		virtual void removeJoint(const PxU32 edgeIndex, Dy::Constraint* constraint, Ps::Array<PxU32, Ps::VirtualAllocator>& jointIndices, IG::IslandSim& islandSim) = 0;
		virtual void addShape(PxsShapeSim* shapeSim, const PxU32 index) = 0;
		virtual void removeShape(const PxU32 index) = 0;
		virtual void addDynamic(PxsRigidBody* rigidBody, const PxU32 nodeIndex) = 0;
		virtual void addDynamics(PxsRigidBody** rigidBody, const PxU32* nodeIndex, PxU32 nbToProcess) = 0;
		virtual void updateJoint(const PxU32 edgeIndex, Dy::Constraint* constraint) = 0;
		virtual void updateBodies(PxsRigidBody** rigidBodies, PxU32* nodeIndices, const PxU32 nbBodies) = 0;
		virtual void updateBody(PxsRigidBody* rigidBodies, const PxU32 nodeIndex) = 0;
		virtual void updateBodiesAndShapes(PxBaseTask* continuation, bool extrudeHeightfields) = 0;
		virtual void update(const PxU32 bitMapWordCounts) = 0;
		virtual void gpuDmabackData(PxsTransformCache& cache, Bp::BoundsArray& boundArray, Cm::BitMapPinned&  changedAABBMgrHandles) = 0;
		virtual void udpateScBodyAndShapeSim(PxsTransformCache& cache, Bp::BoundsArray& boundArray, PxBaseTask* continuation) = 0;
		virtual PxU32* getActiveBodies() = 0;
		virtual PxU32* getDeactiveBodies() = 0;
		virtual PxsBodySim* getBodySims() = 0;
		virtual PxU32	getNbBodies() = 0;

		virtual PxU32*	getUnfrozenShapes() = 0;
		virtual PxU32*	getFrozenShapes() = 0;
		virtual PxsShapeSim** getShapeSims() = 0;
		virtual PxU32	getNbFrozenShapes() = 0;
		virtual PxU32	getNbUnfrozenShapes() = 0;

		virtual void	clear() = 0;
		virtual void	setBounds(Bp::BoundsArray* boundArray) = 0;
		virtual void	reserve(const PxU32 nbBodies) = 0;

	protected:
		PxsSimulationControllerCallback* mCallback;

	};

	PxsSimulationController* createSimulationController(PxsSimulationControllerCallback* callback);
}

#endif
