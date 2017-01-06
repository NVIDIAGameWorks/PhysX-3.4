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

#ifndef PSFOUNDATION_PSHASHSET_H
#define PSFOUNDATION_PSHASHSET_H

#include "PsHashInternals.h"

// TODO: make this doxy-format

// This header defines two hash sets. Hash sets
// * support custom initial table sizes (rounded up internally to power-of-2)
// * support custom static allocator objects
// * auto-resize, based on a load factor (i.e. a 64-entry .75 load factor hash will resize
//                                        when the 49th element is inserted)
// * are based on open hashing
//
// Sets have STL-like copying semantics, and properly initialize and destruct copies of objects
//
// There are two forms of set: coalesced and uncoalesced. Coalesced sets keep the entries in the
// initial segment of an array, so are fast to iterate over; however deletion is approximately
// twice as expensive.
//
// HashSet<T>:
//		bool		insert(const T& k)						amortized O(1) (exponential resize policy)
// 		bool		contains(const T& k)	const;			O(1)
//		bool		erase(const T& k);						O(1)
//		uint32_t		size()					const;			constant
//		void		reserve(uint32_t size);					O(MAX(size, currentOccupancy))
//		void		clear();								O(currentOccupancy) (with zero constant for objects without
// destructors)
//      Iterator    getIterator();
//
// Use of iterators:
//
// for(HashSet::Iterator iter = test.getIterator(); !iter.done(); ++iter)
//			myFunction(*iter);
//
// CoalescedHashSet<T> does not support getIterator, but instead supports
// 		const Key *getEntries();
//
// insertion into a set already containing the element fails returning false, as does
// erasure of an element not in the set
//

namespace physx
{
namespace shdfnd
{
template <class Key, class HashFn = Hash<Key>, class Allocator = NonTrackingAllocator>
class HashSet : public internal::HashSetBase<Key, HashFn, Allocator, false>
{
  public:
	typedef internal::HashSetBase<Key, HashFn, Allocator, false> HashSetBase;
	typedef typename HashSetBase::Iterator Iterator;

	HashSet(uint32_t initialTableSize = 64, float loadFactor = 0.75f) : HashSetBase(initialTableSize, loadFactor)
	{
	}
	HashSet(uint32_t initialTableSize, float loadFactor, const Allocator& alloc)
	: HashSetBase(initialTableSize, loadFactor, alloc)
	{
	}
	HashSet(const Allocator& alloc) : HashSetBase(64, 0.75f, alloc)
	{
	}
	Iterator getIterator()
	{
		return Iterator(HashSetBase::mBase);
	}
};

template <class Key, class HashFn = Hash<Key>, class Allocator = NonTrackingAllocator>
class CoalescedHashSet : public internal::HashSetBase<Key, HashFn, Allocator, true>
{
  public:
	typedef typename internal::HashSetBase<Key, HashFn, Allocator, true> HashSetBase;

	CoalescedHashSet(uint32_t initialTableSize = 64, float loadFactor = 0.75f)
	: HashSetBase(initialTableSize, loadFactor)
	{
	}

	CoalescedHashSet(uint32_t initialTableSize, float loadFactor, const Allocator& alloc)
	: HashSetBase(initialTableSize, loadFactor, alloc)
	{
	}
	CoalescedHashSet(const Allocator& alloc) : HashSetBase(64, 0.75f, alloc)
	{
	}

	const Key* getEntries() const
	{
		return HashSetBase::mBase.getEntries();
	}
};

} // namespace shdfnd
} // namespace physx

#endif // #ifndef PSFOUNDATION_PSHASHSET_H
