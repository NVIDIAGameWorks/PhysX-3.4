/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#pragma once

#include "Solver.h"
#include "Allocator.h"
#include "SwInterCollision.h"
#include "CmTask.h"

namespace nvidia
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
		using PxLightCpuTask::mRefCount;
		using PxLightCpuTask::mTm;

		virtual void runInternal();
		virtual const char* getName() const;

		SwSolver* mSolver;
	};

	struct EndSimulationTask : public Cm::Task
	{
		using PxLightCpuTask::mRefCount;

		virtual void runInternal();
		virtual const char* getName() const;

		SwSolver* mSolver;
#if !APEX_UE4
		float mDt;
#endif
	};

	struct CpuClothSimulationTask : public Cm::Task
	{
#if APEX_UE4
		void* operator new(size_t n){ return allocate(n); }
		void operator delete(void* ptr) { return deallocate(ptr); }

		CpuClothSimulationTask(SwCloth&, SwSolver&);
		~CpuClothSimulationTask();

		void simulate(float dt);

		SwSolver* mSolver;
#else
		CpuClothSimulationTask(SwCloth&, EndSimulationTask&);

		EndSimulationTask* mContinuation;
#endif
		virtual void runInternal();
		virtual const char* getName() const;
		virtual void release();

		SwCloth* mCloth;

		uint32_t mScratchMemorySize;
		void* mScratchMemory;
		float mInvNumIterations;
	};

  public:
	SwSolver(nvidia::profile::PxProfileZone*, PxTaskManager*);
	virtual ~SwSolver();

	virtual void addCloth(Cloth*);
	virtual void removeCloth(Cloth*);

	virtual PxBaseTask& simulate(float dt, PxBaseTask&);

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

	virtual uint32_t getNumSharedPositions( const Cloth* ) const 
	{ 
		return uint32_t(-1); 
	}

	virtual bool hasError() const
	{
		return false;
	}

#if APEX_UE4
	static void simulate(void*, float);
#endif

  private:
	void beginFrame() const;
	void endFrame() const;

	void interCollision();

  private:
	StartSimulationTask mStartSimulationTask;

#if APEX_UE4
	typedef Vector<CpuClothSimulationTask*>::Type CpuClothSimulationTaskVector;
	float mDt;
#else
	typedef Vector<CpuClothSimulationTask>::Type CpuClothSimulationTaskVector;
#endif

	CpuClothSimulationTaskVector mCpuClothSimulationTasks;

	EndSimulationTask mEndSimulationTask;

	profile::PxProfileZone* mProfiler;
	uint16_t mSimulateEventId;

	float mInterCollisionDistance;
	float mInterCollisionStiffness;
	uint32_t mInterCollisionIterations;
	InterCollisionFilter mInterCollisionFilter;

	void* mInterCollisionScratchMem;
	uint32_t mInterCollisionScratchMemSize;
	nvidia::Array<SwInterCollisionData> mInterCollisionInstances;

};
}
}
