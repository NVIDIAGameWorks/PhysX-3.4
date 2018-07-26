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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#include "foundation/PxProfiler.h"
#include "SwSolver.h"
#include "SwCloth.h"
#include "ClothImpl.h"
#include "SwFabric.h"
#include "SwFactory.h"
#include "SwClothData.h"
#include "SwSolverKernel.h"
#include "SwInterCollision.h"
#include "PsFPU.h"
#include "PsFoundation.h"
#include "PsSort.h"

namespace physx
{
namespace cloth
{
bool neonSolverKernel(SwCloth const&, SwClothData&, SwKernelAllocator&, IterationStateFactory&);
}
}

using namespace physx;

#if NV_SIMD_SIMD
typedef Simd4f Simd4fType;
#else
typedef Scalar4f Simd4fType;
#endif

cloth::SwSolver::SwSolver(physx::PxTaskManager* taskMgr)
: mInterCollisionDistance(0.0f)
, mInterCollisionStiffness(1.0f)
, mInterCollisionIterations(1)
, mInterCollisionScratchMem(NULL)
, mInterCollisionScratchMemSize(0)
{
	mStartSimulationTask.mSolver = this;
	mEndSimulationTask.mSolver = this;

	PX_UNUSED(taskMgr);
}

cloth::SwSolver::~SwSolver()
{
	if(mInterCollisionScratchMem)
		PX_FREE(mInterCollisionScratchMem);

	PX_ASSERT(mCpuClothSimulationTasks.empty());
}

namespace
{
template <typename T>
bool clothSizeGreater(const T& t0, const T& t1)
{
	return t0.mCloth->mCurParticles.size() > t1.mCloth->mCurParticles.size();
}

template <typename T>
void sortTasks(shdfnd::Array<T, physx::shdfnd::NonTrackingAllocator>& tasks)
{
	shdfnd::sort(tasks.begin(), tasks.size(), &clothSizeGreater<T>);
}
}

void cloth::SwSolver::addCloth(Cloth* cloth)
{
	SwCloth& swCloth = static_cast<SwClothImpl&>(*cloth).mCloth;

	mCpuClothSimulationTasks.pushBack(CpuClothSimulationTask(swCloth, mEndSimulationTask));

	sortTasks(mCpuClothSimulationTasks);
}

void cloth::SwSolver::removeCloth(Cloth* cloth)
{
	SwCloth& swCloth = static_cast<SwClothImpl&>(*cloth).mCloth;

	CpuClothSimulationTaskVector::Iterator tIt = mCpuClothSimulationTasks.begin();
	CpuClothSimulationTaskVector::Iterator tEnd = mCpuClothSimulationTasks.end();
	while(tIt != tEnd && tIt->mCloth != &swCloth)
		++tIt;

	if(tIt != tEnd)
	{
		deallocate(tIt->mScratchMemory);
		mCpuClothSimulationTasks.replaceWithLast(tIt);
		sortTasks(mCpuClothSimulationTasks);
	}
}

physx::PxBaseTask& cloth::SwSolver::simulate(float dt, physx::PxBaseTask& continuation)
{
	if(mCpuClothSimulationTasks.empty())
	{
		continuation.addReference();
		return continuation;
	}

	mEndSimulationTask.setContinuation(&continuation);
	mEndSimulationTask.mDt = dt;

	mStartSimulationTask.setContinuation(&mEndSimulationTask);

	mEndSimulationTask.removeReference();

	return mStartSimulationTask;
}

void cloth::SwSolver::interCollision()
{
	if(!mInterCollisionIterations || mInterCollisionDistance == 0.0f)
		return;

	float elasticity = 1.0f;

	// rebuild cloth instance array
	mInterCollisionInstances.resize(0);
	for(uint32_t i = 0; i < mCpuClothSimulationTasks.size(); ++i)
	{
		SwCloth* c = mCpuClothSimulationTasks[i].mCloth;
		float invNumIterations = mCpuClothSimulationTasks[i].mInvNumIterations;

		mInterCollisionInstances.pushBack(SwInterCollisionData(
		    c->mCurParticles.begin(), c->mPrevParticles.begin(),
		    c->mSelfCollisionIndices.empty() ? c->mCurParticles.size() : c->mSelfCollisionIndices.size(),
		    c->mSelfCollisionIndices.empty() ? NULL : &c->mSelfCollisionIndices[0], c->mTargetMotion,
		    c->mParticleBoundsCenter, c->mParticleBoundsHalfExtent, elasticity * invNumIterations, c->mUserData));
	}

	const uint32_t requiredTempMemorySize = uint32_t(SwInterCollision<Simd4fType>::estimateTemporaryMemory(
	    &mInterCollisionInstances[0], mInterCollisionInstances.size()));

	// realloc temp memory if necessary
	if(mInterCollisionScratchMemSize < requiredTempMemorySize)
	{
		if(mInterCollisionScratchMem)
			PX_FREE(mInterCollisionScratchMem);

		mInterCollisionScratchMem = PX_ALLOC(requiredTempMemorySize, "cloth::SwSolver::mInterCollisionScratchMem");
		mInterCollisionScratchMemSize = requiredTempMemorySize;
	}

	SwKernelAllocator allocator(mInterCollisionScratchMem, mInterCollisionScratchMemSize);

	// run inter-collision
	SwInterCollision<Simd4fType> collider(mInterCollisionInstances.begin(), mInterCollisionInstances.size(),
	                                      mInterCollisionDistance, mInterCollisionStiffness, mInterCollisionIterations,
	                                      mInterCollisionFilter, allocator);

	collider();
}

void cloth::SwSolver::beginFrame() const
{
	PX_PROFILE_START_CROSSTHREAD("cloth::SwSolver::simulate", 0);
}

void cloth::SwSolver::endFrame() const
{
	PX_PROFILE_STOP_CROSSTHREAD("cloth::SwSolver::simulate", 0);
}

void cloth::SwSolver::StartSimulationTask::runInternal()
{
	mSolver->beginFrame();

	CpuClothSimulationTaskVector::Iterator tIt = mSolver->mCpuClothSimulationTasks.begin();
	CpuClothSimulationTaskVector::Iterator tEnd = mSolver->mCpuClothSimulationTasks.end();

	for(; tIt != tEnd; ++tIt)
	{
		if(!tIt->mCloth->isSleeping())
		{
			tIt->setContinuation(mCont);
			tIt->removeReference();
		}
	}
}

const char* cloth::SwSolver::StartSimulationTask::getName() const
{
	return "cloth.SwSolver.startSimulation";
}

void cloth::SwSolver::EndSimulationTask::runInternal()
{
	mSolver->interCollision();
	mSolver->endFrame();
}

const char* cloth::SwSolver::EndSimulationTask::getName() const
{
	return "cloth.SwSolver.endSimulation";
}

cloth::SwSolver::CpuClothSimulationTask::CpuClothSimulationTask(SwCloth& cloth, EndSimulationTask& continuation)
: Cm::Task(0), mCloth(&cloth), mContinuation(&continuation), mScratchMemorySize(0), mScratchMemory(0), mInvNumIterations(0.0f)
{
}

void cloth::SwSolver::CpuClothSimulationTask::runInternal()
{
	// check if we need to reallocate the temp memory buffer
	// (number of shapes may have changed)
	uint32_t requiredTempMemorySize = uint32_t(SwSolverKernel<Simd4fType>::estimateTemporaryMemory(*mCloth));

	if(mScratchMemorySize < requiredTempMemorySize)
	{
		deallocate(mScratchMemory);

		mScratchMemory = allocate(requiredTempMemorySize);
		mScratchMemorySize = requiredTempMemorySize;
	}

	if(mContinuation->mDt == 0.0f)
		return;

	IterationStateFactory factory(*mCloth, mContinuation->mDt);
	mInvNumIterations = factory.mInvNumIterations;

	shdfnd::SIMDGuard simdGuard;

	SwClothData data(*mCloth, mCloth->mFabric);
	SwKernelAllocator allocator(mScratchMemory, uint32_t(mScratchMemorySize));

// construct kernel functor and execute
#if PX_ANDROID
// if(!neonSolverKernel(cloth, data, allocator, factory))
#endif
	SwSolverKernel<Simd4fType>(*mCloth, data, allocator, factory)();

	data.reconcile(*mCloth); // update cloth
}

const char* cloth::SwSolver::CpuClothSimulationTask::getName() const
{
	return "cloth.SwSolver.cpuClothSimulation";
}

void cloth::SwSolver::CpuClothSimulationTask::release()
{
	mCloth->mMotionConstraints.pop();
	mCloth->mSeparationConstraints.pop();

	if(!mCloth->mTargetCollisionSpheres.empty())
	{
		swap(mCloth->mStartCollisionSpheres, mCloth->mTargetCollisionSpheres);
		mCloth->mTargetCollisionSpheres.resize(0);
	}

	if(!mCloth->mTargetCollisionPlanes.empty())
	{
		swap(mCloth->mStartCollisionPlanes, mCloth->mTargetCollisionPlanes);
		mCloth->mTargetCollisionPlanes.resize(0);
	}

	if(!mCloth->mTargetCollisionTriangles.empty())
	{
		swap(mCloth->mStartCollisionTriangles, mCloth->mTargetCollisionTriangles);
		mCloth->mTargetCollisionTriangles.resize(0);
	}

	mContinuation->removeReference();
}
