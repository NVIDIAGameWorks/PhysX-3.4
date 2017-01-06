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


#ifndef PX_PHYSICS_COMMON_TMPMEM
#define PX_PHYSICS_COMMON_TMPMEM

#include "CmPhysXCommon.h"
#include "PsAllocator.h"

namespace physx
{
namespace Cm
{
	// dsequeira: we should be able to use PX_ALLOCA or Ps::InlineArray for this, but both seem slightly flawed:
	//
	//	PX_ALLOCA has non-configurable fallback threshold and uses _alloca, which means the allocation is necessarily
	//	function-scope rather than block-scope (sometimes useful, mostly not.)
	//
	//	Ps::InlineArray touches all memory on resize (a general flaw in the array class which badly needs fixing)
	//
	//	Todo: fix both the above issues.

	template<typename T, PxU32 stackLimit>
	class TmpMem
	{

	public:
		PX_FORCE_INLINE TmpMem(PxU32 size):
		  mPtr(size<=stackLimit?mStackBuf : reinterpret_cast<T*>(PX_ALLOC(size*sizeof(T), "char")))
		{
		}

		PX_FORCE_INLINE ~TmpMem()
		{
			if(mPtr!=mStackBuf)
				PX_FREE(mPtr);
		}

		PX_FORCE_INLINE T& operator*() const
	    {
			return *mPtr;
		}

		PX_FORCE_INLINE T* operator->() const
		{
			return mPtr;
		}
	
		PX_FORCE_INLINE T& operator[](PxU32 index)
		{
			return mPtr[index];
		}

		T* getBase()
		{
			return mPtr;
		}

	private:
		T mStackBuf[stackLimit];
		T* mPtr;
	};
}

}

#endif
