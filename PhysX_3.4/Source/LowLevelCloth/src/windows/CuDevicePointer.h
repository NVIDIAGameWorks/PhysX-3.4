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

#include <cuda.h>
#include "CuCheckSuccess.h"

namespace physx
{

namespace cloth
{

template <typename T>
struct RemoveConst
{
	typedef T Type;
};
template <typename T>
struct RemoveConst<const T>
{
	typedef T Type;
};

template <typename>
class CuDeviceReference; // forward declare

// pointer to POD type in CUDA device memory
template <typename T>
class CuDevicePointer
{
	template <typename>
	friend class CuDevicePointer;

	typedef typename RemoveConst<T>::Type ValueType;

  public:
	// c'tors
	CuDevicePointer() : mPointer(0)
	{
	}
	template <class U>
	explicit CuDevicePointer(U* ptr)
	: mPointer(ptr)
	{
	}
	CuDevicePointer(const CuDevicePointer<ValueType>& ptr) : mPointer(ptr.get())
	{
	}

	// conversion
	template <typename U>
	operator CuDevicePointer<U>(void) const
	{
		return CuDevicePointer<U>(static_cast<U*>(mPointer));
	}
	T* get() const
	{
		return mPointer;
	}
	CUdeviceptr dev() const
	{
		return reinterpret_cast<CUdeviceptr>(mPointer);
	}

	// operators
	CuDevicePointer operator+(const ptrdiff_t& rhs) const
	{
		return CuDevicePointer(mPointer + rhs);
	}
	CuDevicePointer operator-(const ptrdiff_t& rhs) const
	{
		return CuDevicePointer(mPointer - rhs);
	}
	CuDevicePointer& operator++(void)
	{
		++mPointer;
		return *this;
	}
	CuDevicePointer operator++(int)
	{
		CuDevicePointer copy(*this);
		++(*this);
		return copy;
	}
	CuDevicePointer& operator--(void)
	{
		--mPointer;
		return *this;
	}
	CuDevicePointer operator--(int)
	{
		CuDevicePointer copy(*this);
		--(*this);
		return copy;
	}
	CuDevicePointer& operator+=(ptrdiff_t rhs)
	{
		mPointer += rhs;
		return *this;
	}
	CuDevicePointer& operator-=(ptrdiff_t rhs)
	{
		mPointer -= rhs;
		return *this;
	}
	ptrdiff_t operator-(const CuDevicePointer& rhs) const
	{
		return mPointer - rhs.mPointer;
	}

	template <typename U>
	bool operator==(const CuDevicePointer<U>& other) const
	{
		return mPointer == other.mPointer;
	}
	template <typename U>
	bool operator!=(const CuDevicePointer<U>& other) const
	{
		return mPointer != other.mPointer;
	}

	// dereference
	CuDeviceReference<T> operator[](const ptrdiff_t&) const; // (implemented below)
	CuDeviceReference<T> operator*(void) const
	{
		return operator[](0);
	}

  private:
	T* mPointer;
};

template <typename T>
class CuDeviceReference
{
	template <typename>
	friend class CuDeviceReference;
	template <typename>
	friend class CuDevicePointer;

	typedef typename RemoveConst<T>::Type ValueType;

	template <typename U>
	CuDeviceReference(CuDevicePointer<U> pointer)
	: mPointer(static_cast<T*>(pointer.get()))
	{
	}

  public:
	template <typename U>
	CuDeviceReference(CuDeviceReference<U> reference)
	: mPointer(static_cast<T*>(reference.mPointer))
	{
	}

	CuDevicePointer<T> operator&() const
	{
		return CuDevicePointer<T>(mPointer);
	}

	CuDeviceReference& operator=(const T& v)
	{
		checkSuccess(cuMemcpyHtoD(CUdeviceptr(mPointer), &v, sizeof(T)));
		return *this;
	}
	CuDeviceReference& operator=(const CuDeviceReference& ref)
	{
		checkSuccess(cuMemcpyDtoD(CUdeviceptr(mPointer), CUdeviceptr(ref.mPointer), sizeof(T)));
		return *this;
	}
	operator ValueType() const
	{
		ValueType result;
		checkSuccess(cuMemcpyDtoH(&result, CUdeviceptr(mPointer), sizeof(T)));
		return result;
	}

  private:
	T* mPointer;
};
}

template <typename T>
cloth::CuDeviceReference<T> cloth::CuDevicePointer<T>::operator[](const ptrdiff_t& i) const
{
	return CuDeviceReference<T>(*this + i);
}
}
