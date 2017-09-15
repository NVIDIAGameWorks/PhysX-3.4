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

#ifndef PSFOUNDATION_PSSLIST_H
#define PSFOUNDATION_PSSLIST_H

#include "foundation/Px.h"
#include "foundation/PxAssert.h"
#include "PsAlignedMalloc.h"

#if PX_P64_FAMILY
#define PX_SLIST_ALIGNMENT 16
#else
#define PX_SLIST_ALIGNMENT 8
#endif

namespace physx
{
namespace shdfnd
{

#if PX_VC
#pragma warning(push)
#pragma warning(disable : 4324) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif

#if !PX_GCC_FAMILY
__declspec(align(PX_SLIST_ALIGNMENT))
#endif
    class SListEntry
{
	friend struct SListImpl;

  public:
	SListEntry() : mNext(NULL)
	{
		PX_ASSERT((size_t(this) & (PX_SLIST_ALIGNMENT - 1)) == 0);
	}

	// Only use on elements returned by SList::flush()
	// because the operation is not atomic.
	SListEntry* next()
	{
		return mNext;
	}

  private:
	SListEntry* mNext;
}
#if PX_GCC_FAMILY
__attribute__((aligned(PX_SLIST_ALIGNMENT)));
#else
;
#endif

#if PX_VC
#pragma warning(pop)
#endif

// template-less implementation
struct PX_FOUNDATION_API SListImpl
{
	SListImpl();
	~SListImpl();
	void push(SListEntry* entry);
	SListEntry* pop();
	SListEntry* flush();
	static uint32_t getSize();
};

template <typename Alloc = ReflectionAllocator<SListImpl> >
class SListT : protected Alloc
{
  public:
	SListT(const Alloc& alloc = Alloc()) : Alloc(alloc)
	{
		mImpl = reinterpret_cast<SListImpl*>(Alloc::allocate(SListImpl::getSize(), __FILE__, __LINE__));
		PX_ASSERT((size_t(mImpl) & (PX_SLIST_ALIGNMENT - 1)) == 0);
		PX_PLACEMENT_NEW(mImpl, SListImpl)();
	}
	~SListT()
	{
		mImpl->~SListImpl();
		Alloc::deallocate(mImpl);
	}

	// pushes a new element to the list
	void push(SListEntry& entry)
	{
		mImpl->push(&entry);
	}

	// pops an element from the list
	SListEntry* pop()
	{
		return mImpl->pop();
	}

	// removes all items from list, returns pointer to first element
	SListEntry* flush()
	{
		return mImpl->flush();
	}

  private:
	SListImpl* mImpl;
};

typedef SListT<> SList;

} // namespace shdfnd
} // namespace physx

#endif // #ifndef PSFOUNDATION_PSSLIST_H
