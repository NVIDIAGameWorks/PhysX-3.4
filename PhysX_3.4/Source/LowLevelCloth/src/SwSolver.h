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

#pragma once

#include "Solver.h"
#include "Allocator.h"
#include "SwInterCollision.h"
#include "CmTask.h"

namespace physx
{

namespace cloth
{

class SwCloth;
class SwFactory;

/// CPU/SSE based cloth solver
class SwSolver : public UserAllocated, public Solver
{
	struct StartSimulationTask : public Cm::Task
	{
		using physx::PxLightCpuTask::mRefCount;
		using physx::PxLightCpuTask::mTm;

		StartSimulationTask() : Cm::Task(0)	{}

		virtual void runInternal();
		virtual const char* getName() const;
		SwSolver* mSolver;
	};

	struct EndSimulationTask : public Cm::Task
	{
		using physx::PxLightCpuTask::mRefCount;

		EndSimulationTask() : Cm::Task(0)	{}

		virtual void runInternal();
		virtual const char* getName() const;
		SwSolver* mSolver;
		float mDt;
	};

	struct CpuClothSimulationTask : public Cm::Task
	{
		CpuClothSimulationTask(SwCloth&, EndSimulationTask&);
		virtual void runInternal();
		virtual const char* getName() const;
		virtual void release();

		SwCloth* mCloth;
		EndSimulationTask* mContinuation;
		uint32_t mScratchMemorySize;
		void* mScratchMemory;
		float mInvNumIterations;
	};

  public:
	SwSolver(physx::PxTaskManager*);
	virtual ~SwSolver();

	virtual void addCloth(Cloth*);
	virtual void removeCloth(Cloth*);

	virtual physx::PxBaseTask& simulate(float dt, physx::PxBaseTask&);

	virtual void setInterCollisionDistance(float distance)
	{
		mInterCollisionDistance = distance;
	}
	virtual float getInterCollisionDistance() const
	{
		return mInterCollisionDistance;
	}

	virtual void setInterCollisionStiffness(float stiffness)
	{
		mInterCollisionStiffness = stiffness;
	}
	virtual float getInterCollisionStiffness() const
	{
		return mInterCollisionStiffness;
	}

	virtual void setInterCollisionNbIterations(uint32_t nbIterations)
	{
		mInterCollisionIterations = nbIterations;
	}
	virtual uint32_t getInterCollisionNbIterations() const
	{
		return mInterCollisionIterations;
	}

	virtual void setInterCollisionFilter(InterCollisionFilter filter)
	{
		mInterCollisionFilter = filter;
	}

	virtual bool hasError() const
	{
		return false;
	}

  private:
	void beginFrame() const;
	void endFrame() const;

	void interCollision();

  private:
	StartSimulationTask mStartSimulationTask;

	typedef Vector<CpuClothSimulationTask>::Type CpuClothSimulationTaskVector;
	CpuClothSimulationTaskVector mCpuClothSimulationTasks;

	EndSimulationTask mEndSimulationTask;

	float mInterCollisionDistance;
	float mInterCollisionStiffness;
	uint32_t mInterCollisionIterations;
	InterCollisionFilter mInterCollisionFilter;

	void* mInterCollisionScratchMem;
	uint32_t mInterCollisionScratchMemSize;
	shdfnd::Array<SwInterCollisionData> mInterCollisionInstances;
};
}
}
