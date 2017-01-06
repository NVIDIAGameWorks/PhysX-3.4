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

#include "cudamanager/PxCudaContextManager.h"
#include "cudamanager/PxCudaMemoryManager.h"
#include "Allocator.h"
#include "CuCheckSuccess.h"
#include <cuda.h>

namespace physx
{

namespace cloth
{

struct CuHostAllocator
{
	CuHostAllocator(physx::PxCudaContextManager* ctx = NULL, unsigned int flags = cudaHostAllocDefault)
	: mDevicePtr(0), mFlags(flags), mManager(0)
	{
		PX_ASSERT(ctx);

		if(ctx)
			mManager = ctx->getMemoryManager();
	}

	void* allocate(size_t n, const char*, int)
	{
		physx::PxCudaBufferPtr bufferPtr;

		PX_ASSERT(mManager);

		if(mFlags & cudaHostAllocWriteCombined)
			bufferPtr = mManager->alloc(physx::PxCudaBufferMemorySpace::T_WRITE_COMBINED, n,
			                            PX_ALLOC_INFO("cloth::CuHostAllocator::T_WRITE_COMBINED", CLOTH));
		else if(mFlags & cudaHostAllocMapped)
			bufferPtr = mManager->alloc(physx::PxCudaBufferMemorySpace::T_PINNED_HOST, n,
			                            PX_ALLOC_INFO("cloth::CuHostAllocator::T_PINNED_HOST", CLOTH));
		else
			bufferPtr = mManager->alloc(physx::PxCudaBufferMemorySpace::T_HOST, n,
			                            PX_ALLOC_INFO("cloth::CuHostAllocator::T_HOST", CLOTH));

		if(mFlags & cudaHostAllocMapped)
			checkSuccess(cuMemHostGetDevicePointer(&mDevicePtr, reinterpret_cast<void*>(bufferPtr), 0));

		return reinterpret_cast<void*>(bufferPtr);
	}

	void deallocate(void* p)
	{
		PX_ASSERT(mManager);

		if(mFlags & cudaHostAllocWriteCombined)
			mManager->free(physx::PxCudaBufferMemorySpace::T_WRITE_COMBINED, physx::PxCudaBufferPtr(p));
		else if(mFlags & cudaHostAllocMapped)
			mManager->free(physx::PxCudaBufferMemorySpace::T_PINNED_HOST, physx::PxCudaBufferPtr(p));
		else
			mManager->free(physx::PxCudaBufferMemorySpace::T_HOST, physx::PxCudaBufferPtr(p));

		// don't reset mDevicePtr because Array::recreate deallocates last
	}

	CUdeviceptr mDevicePtr; // device pointer of last allocation
	unsigned int mFlags;
	physx::PxCudaMemoryManager* mManager;
};

template <typename T>
CuHostAllocator getMappedAllocator(physx::PxCudaContextManager* ctx)
{
	return CuHostAllocator(ctx, cudaHostAllocMapped | cudaHostAllocWriteCombined);
}

template <typename T>
struct CuPinnedVector
{
	// note: always use shdfnd::swap() instead of Array::swap()
	// in order to keep cached device pointer consistent
	typedef shdfnd::Array<T, typename physx::cloth::CuHostAllocator> Type;
};

template <typename T>
T* getDevicePointer(shdfnd::Array<T, typename physx::cloth::CuHostAllocator>& vector)
{
	// cached device pointer only valid if non-empty
	return vector.empty() ? 0 : reinterpret_cast<T*>(vector.getAllocator().mDevicePtr);
}

} // namespace cloth

} // namespace physx

namespace physx
{
namespace shdfnd
{
template <typename T>
void swap(Array<T, typename physx::cloth::CuHostAllocator>& left, Array<T, typename physx::cloth::CuHostAllocator>& right)
{
	swap(left.getAllocator(), right.getAllocator());
	left.swap(right);
}
}
}
