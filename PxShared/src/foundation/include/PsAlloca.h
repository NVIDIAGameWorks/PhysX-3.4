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

#ifndef PSFOUNDATION_PSALLOCA_H
#define PSFOUNDATION_PSALLOCA_H

#include "PsTempAllocator.h"

namespace physx
{
namespace shdfnd
{
template <typename T, typename Alloc = TempAllocator>
class ScopedPointer : private Alloc
{
  public:
	~ScopedPointer()
	{
		if(mOwned)
			Alloc::deallocate(mPointer);
	}

	operator T*() const
	{
		return mPointer;
	}

	T* mPointer;
	bool mOwned;
};

} // namespace shdfnd
} // namespace physx

/*! Stack allocation for \c count instances of \c type. Falling back to temp allocator if using more than 1kB. */
#ifdef __SPU__
#define PX_ALLOCA(var, type, count) type* var = reinterpret_cast<type*>(PxAlloca(sizeof(type) * (count)))
#else
#define PX_ALLOCA(var, type, count)                                                                                    \
	physx::shdfnd::ScopedPointer<type> var;                                                                            \
	{                                                                                                                  \
		uint32_t size = sizeof(type) * (count);                                                                        \
		var.mOwned = size > 1024;                                                                                      \
		if(var.mOwned)                                                                                                 \
			var.mPointer = reinterpret_cast<type*>(physx::shdfnd::TempAllocator().allocate(size, __FILE__, __LINE__)); \
		else                                                                                                           \
			var.mPointer = reinterpret_cast<type*>(PxAlloca(size));                                                    \
	}
#endif
#endif // #ifndef PSFOUNDATION_PSALLOCA_H
