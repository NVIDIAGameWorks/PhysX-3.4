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

#include "foundation/PxMemory.h"
#include "CuFactory.h"
#include "CuFabric.h"
#include "CuCloth.h"
#include "CuSolver.h"
#include "ClothImpl.h"
#include "CuCheckSuccess.h"
#include "CuContextLock.h"
#include "PsAllocator.h"
#include "Array.h"
#include "PsFoundation.h"
#include <cuda.h>

#if PX_VC
#pragma warning(disable : 4061 4062) // enumerator 'identifier' in switch of enum 'enumeration' is not handled
#endif

using namespace physx;
using namespace shdfnd;

namespace physx
{
namespace cloth
{
// defined in Factory.cpp
uint32_t getNextFabricId();

typedef Vec4T<uint32_t> Vec4u;
}
}

void cloth::checkSuccessImpl(CUresult err, const char* file, const int line)
{
	if(err != CUDA_SUCCESS)
	{
		const char* code = "Unknown";
		switch(err)
		{
#define ADD_CASE(X)                                                                                                    \
	case X:                                                                                                            \
		code = #X;                                                                                                     \
		break
			ADD_CASE(CUDA_ERROR_INVALID_VALUE);
			ADD_CASE(CUDA_ERROR_OUT_OF_MEMORY);
			ADD_CASE(CUDA_ERROR_NOT_INITIALIZED);
			ADD_CASE(CUDA_ERROR_DEINITIALIZED);
			ADD_CASE(CUDA_ERROR_NO_DEVICE);
			ADD_CASE(CUDA_ERROR_INVALID_DEVICE);
			ADD_CASE(CUDA_ERROR_INVALID_IMAGE);
			ADD_CASE(CUDA_ERROR_INVALID_CONTEXT);
			ADD_CASE(CUDA_ERROR_MAP_FAILED);
			ADD_CASE(CUDA_ERROR_UNMAP_FAILED);
			ADD_CASE(CUDA_ERROR_ARRAY_IS_MAPPED);
			ADD_CASE(CUDA_ERROR_ALREADY_MAPPED);
			ADD_CASE(CUDA_ERROR_NO_BINARY_FOR_GPU);
			ADD_CASE(CUDA_ERROR_ALREADY_ACQUIRED);
			ADD_CASE(CUDA_ERROR_NOT_MAPPED);
			ADD_CASE(CUDA_ERROR_NOT_MAPPED_AS_ARRAY);
			ADD_CASE(CUDA_ERROR_NOT_MAPPED_AS_POINTER);
			ADD_CASE(CUDA_ERROR_ECC_UNCORRECTABLE);
			ADD_CASE(CUDA_ERROR_UNSUPPORTED_LIMIT);
			ADD_CASE(CUDA_ERROR_INVALID_SOURCE);
			ADD_CASE(CUDA_ERROR_FILE_NOT_FOUND);
			ADD_CASE(CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND);
			ADD_CASE(CUDA_ERROR_SHARED_OBJECT_INIT_FAILED);
			ADD_CASE(CUDA_ERROR_OPERATING_SYSTEM);
			ADD_CASE(CUDA_ERROR_INVALID_HANDLE);
			ADD_CASE(CUDA_ERROR_NOT_FOUND);
			ADD_CASE(CUDA_ERROR_NOT_READY);
			ADD_CASE(CUDA_ERROR_LAUNCH_FAILED);
			ADD_CASE(CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES);
			ADD_CASE(CUDA_ERROR_LAUNCH_TIMEOUT);
			ADD_CASE(CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING);
		default:
			ADD_CASE(CUDA_ERROR_UNKNOWN);
#undef ADD_CASE
		}
		shdfnd::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, file, line, "CUDA error: %s", code);
	}
}

namespace
{
// returns max threads as specified by launch bounds in CuSolverKernel.cu
uint32_t getMaxThreadsPerBlock(const physx::PxCudaContextManager& contextManager)
{
	if(contextManager.supportsArchSM30())
		return 1024;

	if(contextManager.supportsArchSM20())
		return 512;

	return 192;
}
}

cloth::CuFactory::CuFactory(physx::PxCudaContextManager* contextManager)
: Factory(CUDA)
, mContextManager(contextManager)
, mNumThreadsPerBlock(getMaxThreadsPerBlock(*contextManager))
, mMaxThreadsPerBlock(mNumThreadsPerBlock)
{
}

cloth::CuFactory::~CuFactory()
{
}

cloth::Fabric* cloth::CuFactory::createFabric(uint32_t numParticles, Range<const uint32_t> phases,
                                              Range<const uint32_t> sets, Range<const float> restvalues,
                                              Range<const uint32_t> indices, Range<const uint32_t> anchors,
                                              Range<const float> tetherLengths, Range<const uint32_t> triangles)
{
	return new CuFabric(*this, numParticles, phases, sets, restvalues, indices, anchors, tetherLengths, triangles,
	                    getNextFabricId());
}

cloth::Cloth* cloth::CuFactory::createCloth(Range<const PxVec4> particles, Fabric& fabric)
{
	return new CuClothImpl(*this, fabric, particles);
}

cloth::Solver* cloth::CuFactory::createSolver(physx::PxTaskManager*)
{
	CuSolver* solver = new CuSolver(*this);

	if(solver->hasError())
	{
		delete solver;
		return NULL;
	}

	return solver;
}

// CuFactory::clone() implemented in CuClothClone.cpp

void cloth::CuFactory::copyToHost(const void* srcIt, const void* srcEnd, void* dstIt) const
{
	CuContextLock contextLock(*this);

	checkSuccess(cuMemcpyDtoH(dstIt, CUdeviceptr(srcIt), size_t(intptr_t(srcEnd) - intptr_t(srcIt))));
}

void cloth::CuFactory::extractFabricData(const Fabric& fabric, Range<uint32_t> phases, Range<uint32_t> sets,
                                         Range<float> restvalues, Range<uint32_t> indices, Range<uint32_t> anchors,
                                         Range<float> tetherLengths, Range<uint32_t> triangles) const
{
	CuContextLock contextLock(*this);

	const CuFabric& cuFabric = static_cast<const CuFabric&>(fabric);

	if(!phases.empty())
	{
		PX_ASSERT(phases.size() == cuFabric.mPhases.size());
		const uint32_t* devicePhases = cuFabric.mPhases.begin().get();
		copyToHost(devicePhases, devicePhases + cuFabric.mPhases.size(), phases.begin());
	}

	if(!restvalues.empty())
	{
		PX_ASSERT(restvalues.size() == cuFabric.mRestvalues.size());
		const float* deviceRestvalues = cuFabric.mRestvalues.begin().get();
		copyToHost(deviceRestvalues, deviceRestvalues + cuFabric.mRestvalues.size(), restvalues.begin());
	}

	if(!sets.empty())
	{
		PX_ASSERT(sets.size() == cuFabric.mSets.size() - 1);
		const uint32_t* deviceSets = cuFabric.mSets.begin().get();
		copyToHost(deviceSets + 1, deviceSets + cuFabric.mSets.size(), sets.begin());
	}

	if(!indices.empty())
	{
		PX_ASSERT(indices.size() == cuFabric.mIndices.size());
		const uint16_t* deviceIndices = cuFabric.mIndices.begin().get();
		uint16_t* hostIndices = reinterpret_cast<uint16_t*>(indices.begin());
		copyToHost(deviceIndices, deviceIndices + cuFabric.mIndices.size(), hostIndices);

		// convert from 16bit to 32bit indices
		for(uint32_t i = indices.size(); 0 < i--;)
			indices[i] = hostIndices[i];
	}

	if(!anchors.empty() || !tetherLengths.empty())
	{
		uint32_t numTethers = uint32_t(cuFabric.mTethers.size());
		Vector<CuTether>::Type tethers(numTethers, CuTether(0, 0));
		const CuTether* deviceTethers = cuFabric.mTethers.begin().get();
		copyToHost(deviceTethers, deviceTethers + numTethers, tethers.begin());

		PX_ASSERT(anchors.empty() || anchors.size() == tethers.size());
		for(uint32_t i = 0; !anchors.empty(); ++i, anchors.popFront())
			anchors.front() = tethers[i].mAnchor;

		PX_ASSERT(tetherLengths.empty() || tetherLengths.size() == tethers.size());
		for(uint32_t i = 0; !tetherLengths.empty(); ++i, tetherLengths.popFront())
			tetherLengths.front() = tethers[i].mLength * cuFabric.mTetherLengthScale;
	}

	if(!triangles.empty())
	{
		// todo triangles
	}
}

void cloth::CuFactory::extractCollisionData(const Cloth& cloth, Range<PxVec4> spheres, Range<uint32_t> capsules,
                                            Range<PxVec4> planes, Range<uint32_t> convexes, Range<PxVec3> triangles) const
{
	PX_ASSERT(&cloth.getFactory() == this);

	const CuCloth& cuCloth = static_cast<const CuClothImpl&>(cloth).mCloth;

	PX_ASSERT(spheres.empty() || spheres.size() == cuCloth.mStartCollisionSpheres.size());
	PX_ASSERT(capsules.empty() || capsules.size() == cuCloth.mCapsuleIndices.size() * 2);
	PX_ASSERT(planes.empty() || planes.size() == cuCloth.mStartCollisionPlanes.size());
	PX_ASSERT(convexes.empty() || convexes.size() == cuCloth.mConvexMasks.size());
	PX_ASSERT(triangles.empty() || triangles.size() == cuCloth.mStartCollisionTriangles.size());

	// collision spheres are in pinned memory, so memcpy directly
	if(!cuCloth.mStartCollisionSpheres.empty() && !spheres.empty())
		PxMemCopy(spheres.begin(), &cuCloth.mStartCollisionSpheres.front(),
		       cuCloth.mStartCollisionSpheres.size() * sizeof(PxVec4));

	if(!cuCloth.mCapsuleIndices.empty() && !capsules.empty())
		PxMemCopy(capsules.begin(), &cuCloth.mCapsuleIndices.front(), cuCloth.mCapsuleIndices.size() * sizeof(IndexPair));

	if(!cuCloth.mStartCollisionPlanes.empty() && !planes.empty())
		PxMemCopy(planes.begin(), &cuCloth.mStartCollisionPlanes.front(),
		       cuCloth.mStartCollisionPlanes.size() * sizeof(PxVec4));

	if(!cuCloth.mConvexMasks.empty() && !convexes.empty())
		PxMemCopy(convexes.begin(), &cuCloth.mConvexMasks.front(), cuCloth.mConvexMasks.size() * sizeof(uint32_t));

	if(!cuCloth.mStartCollisionTriangles.empty() && !triangles.empty())
		PxMemCopy(triangles.begin(), &cuCloth.mStartCollisionTriangles.front(),
		       cuCloth.mStartCollisionTriangles.size() * sizeof(PxVec3));
}

void cloth::CuFactory::extractMotionConstraints(const Cloth& cloth, Range<PxVec4> destConstraints) const
{
	PX_ASSERT(&cloth.getFactory() == this);

	const CuCloth& cuCloth = static_cast<const CuClothImpl&>(cloth).mCloth;

	if(cuCloth.mMotionConstraints.mHostCopy.size())
	{
		PX_ASSERT(destConstraints.size() == cuCloth.mMotionConstraints.mHostCopy.size());

		PxMemCopy(destConstraints.begin(), cuCloth.mMotionConstraints.mHostCopy.begin(),
		          sizeof(PxVec4) * cuCloth.mMotionConstraints.mHostCopy.size());
	}
	else
	{
		CuContextLock contextLock(*this);

		CuDeviceVector<PxVec4> const& srcConstraints = !cuCloth.mMotionConstraints.mTarget.empty()
		                                                   ? cuCloth.mMotionConstraints.mTarget
		                                                   : cuCloth.mMotionConstraints.mStart;

		PX_ASSERT(destConstraints.size() == srcConstraints.size());

		copyToHost(srcConstraints.begin().get(), srcConstraints.end().get(), destConstraints.begin());
	}
}

void cloth::CuFactory::extractSeparationConstraints(const Cloth& cloth, Range<PxVec4> destConstraints) const
{
	PX_ASSERT(&cloth.getFactory() == this);

	const CuCloth& cuCloth = static_cast<const CuClothImpl&>(cloth).mCloth;

	if(cuCloth.mSeparationConstraints.mHostCopy.size())
	{
		PX_ASSERT(destConstraints.size() == cuCloth.mSeparationConstraints.mHostCopy.size());

		PxMemCopy(destConstraints.begin(), cuCloth.mSeparationConstraints.mHostCopy.begin(),
		          sizeof(PxVec4) * cuCloth.mSeparationConstraints.mHostCopy.size());
	}
	else
	{
		CuContextLock contextLock(*this);

		CuDeviceVector<PxVec4> const& srcConstraints = !cuCloth.mSeparationConstraints.mTarget.empty()
		                                                   ? cuCloth.mSeparationConstraints.mTarget
		                                                   : cuCloth.mSeparationConstraints.mStart;

		PX_ASSERT(destConstraints.size() == srcConstraints.size());

		copyToHost(srcConstraints.begin().get(), srcConstraints.end().get(), destConstraints.begin());
	}
}

void cloth::CuFactory::extractParticleAccelerations(const Cloth& cloth, Range<PxVec4> destAccelerations) const
{
	PX_ASSERT(&cloth.getFactory() == this);

	const CuCloth& cuCloth = static_cast<const CuClothImpl&>(cloth).mCloth;

	if(cuCloth.mParticleAccelerationsHostCopy.size())
	{
		PX_ASSERT(destAccelerations.size() == cuCloth.mParticleAccelerationsHostCopy.size());

		PxMemCopy(destAccelerations.begin(), cuCloth.mParticleAccelerationsHostCopy.begin(),
		          sizeof(PxVec4) * cuCloth.mParticleAccelerationsHostCopy.size());
	}
}

void cloth::CuFactory::extractVirtualParticles(const Cloth& cloth, Range<uint32_t[4]> destIndices,
                                               Range<PxVec3> destWeights) const
{
	PX_ASSERT(&cloth.getFactory() == this);

	CuContextLock contextLock(*this);

	const CuCloth& cuCloth = static_cast<const CuClothImpl&>(cloth).mCloth;

	if(destWeights.size() > 0)
	{
		uint32_t numWeights = cloth.getNumVirtualParticleWeights();

		Vector<PxVec4>::Type hostWeights(numWeights, PxVec4(0.0f));
		copyToHost(cuCloth.mVirtualParticleWeights.begin().get(), cuCloth.mVirtualParticleWeights.end().get(),
		           &hostWeights.front());

		// convert weights to Vec3f
		PxVec3* destIt = reinterpret_cast<PxVec3*>(destWeights.begin());
		Vector<PxVec4>::Type::ConstIterator srcIt = hostWeights.begin();
		Vector<PxVec4>::Type::ConstIterator srcEnd = srcIt + numWeights;
		for(; srcIt != srcEnd; ++srcIt, ++destIt)
			*destIt = reinterpret_cast<const PxVec3&>(*srcIt);

		PX_ASSERT(destIt <= destWeights.end());
	}

	if(destIndices.size() > 0)
	{
		uint32_t numIndices = cloth.getNumVirtualParticles();

		Vector<Vec4us>::Type hostIndices(numIndices);
		copyToHost(cuCloth.mVirtualParticleIndices.begin().get(), cuCloth.mVirtualParticleIndices.end().get(),
		           &hostIndices.front());

		// convert indices to 32 bit
		Vec4u* destIt = reinterpret_cast<Vec4u*>(destIndices.begin());
		Vector<Vec4us>::Type::ConstIterator srcIt = hostIndices.begin();
		Vector<Vec4us>::Type::ConstIterator srcEnd = srcIt + numIndices;
		for(; srcIt != srcEnd; ++srcIt, ++destIt)
			*destIt = Vec4u(*srcIt);

		PX_ASSERT(&array(*destIt) <= destIndices.end());
	}
}

void cloth::CuFactory::extractSelfCollisionIndices(const Cloth& cloth, Range<uint32_t> destIndices) const
{
	const CuCloth& cuCloth = static_cast<const CuClothImpl&>(cloth).mCloth;
	PX_ASSERT(destIndices.size() == cuCloth.mSelfCollisionIndices.size());
	copyToHost(cuCloth.mSelfCollisionIndices.begin().get(), cuCloth.mSelfCollisionIndices.end().get(),
	           destIndices.begin());
}

void cloth::CuFactory::extractRestPositions(const Cloth& cloth, Range<PxVec4> destRestPositions) const
{
	const CuCloth& cuCloth = static_cast<const CuClothImpl&>(cloth).mCloth;
	PX_ASSERT(destRestPositions.size() == cuCloth.mRestPositions.size());
	copyToHost(cuCloth.mRestPositions.begin().get(), cuCloth.mRestPositions.end().get(), destRestPositions.begin());
}
