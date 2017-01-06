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

#include "foundation/PxMath.h" // for swap
#include "cudamanager/PxCudaMemoryManager.h"
#include "cudamanager/PxCudaContextManager.h"
#include "CuDevicePointer.h"
#include "PsArray.h"
#include "PsUtilities.h"

namespace physx
{
#if PX_VC
#pragma warning(push)
#pragma warning(disable : 4365) // 'action' : conversion from 'type_1' to 'type_2', signed/unsigned mismatch
#endif

namespace cloth
{

// STL-style vector that holds POD types in CUDA device memory. The interface
// is not complete, add whatever you need from the std::vector interface.
template <typename T>
class CuDeviceVector
{
  public:
	typedef CuDevicePointer<T> iterator;
	typedef CuDevicePointer<const T> const_iterator;

	CuDeviceVector(physx::PxCudaContextManager* ctx) : mManager(0)
	{
		PX_ASSERT(ctx);

		if(ctx)
			mManager = ctx->getMemoryManager();
	}

	CuDeviceVector(const CuDeviceVector& other) : mManager(other.getMemoryManager())
	{
		PX_ASSERT(mManager);

		operator=(other);
	}

	CuDeviceVector(physx::PxCudaContextManager* ctx, const T* first, const T* last) : mManager(0)
	{
		PX_ASSERT(ctx);

		if(ctx)
		{
			mManager = ctx->getMemoryManager();
			assign(first, last);
		}
	}

	template <typename Alloc>
	CuDeviceVector(const shdfnd::Array<T, Alloc>& other)
	{
		operator=(other);
	}

	~CuDeviceVector()
	{
		PX_ASSERT(mManager);

		mManager->free(physx::PxCudaBufferMemorySpace::T_GPU, mFirst.dev());
	}

	CuDeviceVector& operator=(const CuDeviceVector& other)
	{
		resize(other.size());
		checkSuccess(cuMemcpyDtoD(mFirst.dev(), other.mFirst.dev(), other.size() * sizeof(T)));
		return *this;
	}

	template <typename Alloc>
	CuDeviceVector& operator=(const shdfnd::Array<T, Alloc>& other)
	{
		const T* first = other.empty() ? 0 : &other.front();
		assign(first, first + other.size());
		return *this;
	}

	bool empty() const
	{
		return mLast == mFirst;
	}
	size_t size() const
	{
		return size_t(mLast - mFirst);
	}
	size_t capacity() const
	{
		return mEnd - mFirst;
	}

	iterator begin()
	{
		return mFirst;
	}
	iterator end()
	{
		return mLast;
	}
	const_iterator begin() const
	{
		return mFirst;
	}
	const_iterator end() const
	{
		return mLast;
	}

	void push_back(const T& v)
	{
		if(mLast == mEnd)
			reserve(PxMax<size_t>(1, capacity() * 2));

		*mLast++ = v;
	}

	void push_back(const T* first, const T* last)
	{
		if(mEnd - mLast < last - first)
			reserve(PxMax<size_t>(2 * capacity(), mLast - mFirst + last - first));

		if(first != last)
			checkSuccess(cuMemcpyHtoD(mLast.dev(), first, sizeof(T) * (last - first)));

		mLast += last - first;
	}

	void erase(iterator it)
	{
		size_t byteSize = (mLast - it - 1) * sizeof(T);
		if(byteSize)
		{
			CUdeviceptr tmp = 0, dst = it.dev();

			PX_ASSERT(mManager);

			tmp = mManager->alloc(physx::PxCudaBufferMemorySpace::T_GPU, byteSize,
			                      PX_ALLOC_INFO("cloth::CuDeviceVector::T_GPU", CLOTH));
			checkSuccess(cuMemcpyDtoD(tmp, dst + sizeof(T), byteSize));
			checkSuccess(cuMemcpyDtoD(dst, tmp, byteSize));
			mManager->free(physx::PxCudaBufferMemorySpace::T_GPU, tmp);
		}
		--mLast;
	}

	void reserve(size_t n)
	{
		if(n <= capacity())
			return;

		CUdeviceptr newFirst = 0, oldFirst = mFirst.dev();

		PX_ASSERT(mManager);

		newFirst = mManager->alloc(physx::PxCudaBufferMemorySpace::T_GPU, sizeof(T) * n,
		                           PX_ALLOC_INFO("cloth::CuDeviceVector::T_GPU", CLOTH));
		checkSuccess(cuMemcpyDtoD(newFirst, oldFirst, sizeof(T) * size()));
		mManager->free(physx::PxCudaBufferMemorySpace::T_GPU, oldFirst);

		iterator first(reinterpret_cast<T*>(newFirst));
		mEnd = first + n;
		mLast = first + size();
		mFirst = first;
	}

	void resize(size_t n)
	{
		if(capacity() < n)
			reserve(PxMax(n, capacity() * 2));

		mLast = mFirst + n;
	}

	void assign(const T* first, const T* last)
	{
		size_t n = last - first;
		resize(n);
		checkSuccess(cuMemcpyHtoD(mFirst.dev(), first, n * sizeof(T)));
	}

	void swap(CuDeviceVector& other)
	{
		shdfnd::swap(mFirst, other.mFirst);
		shdfnd::swap(mLast, other.mLast);
		shdfnd::swap(mEnd, other.mEnd);
	}

	// match PxArray interface
	void remove(size_t i)
	{
		erase(begin() + i);
	}
	void pushBack(const T& v)
	{
		push_back(v);
	}

	physx::PxCudaMemoryManager* getMemoryManager() const
	{
		return mManager;
	}

  private:
	iterator mFirst, mLast, mEnd;
	physx::PxCudaMemoryManager* mManager;
};

} // namespace cloth
} // namespace physx

#if PX_VC
#pragma warning(pop)
#endif

namespace physx
{
namespace shdfnd
{
template <typename T>
void swap(physx::cloth::CuDeviceVector<T>& first, physx::cloth::CuDeviceVector<T>& second)
{
	first.swap(second);
}
}
}
