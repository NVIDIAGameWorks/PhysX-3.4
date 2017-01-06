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

#include "PsFoundation.h"
#include "PsAllocator.h"
#include "PsHashMap.h"
#include "PsArray.h"
#include "PsMutex.h"

namespace physx
{
namespace shdfnd
{

#if PX_USE_NAMED_ALLOCATOR
namespace
{
typedef HashMap<const NamedAllocator*, const char*, Hash<const NamedAllocator*>, NonTrackingAllocator> AllocNameMap;
PX_INLINE AllocNameMap& getMap()
{
	return getFoundation().getNamedAllocMap();
}
PX_INLINE Foundation::Mutex& getMutex()
{
	return getFoundation().getNamedAllocMutex();
}
}

NamedAllocator::NamedAllocator(const PxEMPTY)
{
	Foundation::Mutex::ScopedLock lock(getMutex());
	getMap().insert(this, 0);
}

NamedAllocator::NamedAllocator(const char* name)
{
	Foundation::Mutex::ScopedLock lock(getMutex());
	getMap().insert(this, name);
}

NamedAllocator::NamedAllocator(const NamedAllocator& other)
{
	Foundation::Mutex::ScopedLock lock(getMutex());
	const AllocNameMap::Entry* e = getMap().find(&other);
	PX_ASSERT(e);
	const char* name = e->second; // The copy is important because insert might invalidate the referenced hash entry
	getMap().insert(this, name);
}

NamedAllocator::~NamedAllocator()
{
	Foundation::Mutex::ScopedLock lock(getMutex());
	bool erased = getMap().erase(this);
	PX_UNUSED(erased);
	PX_ASSERT(erased);
}

NamedAllocator& NamedAllocator::operator=(const NamedAllocator& other)
{
	Foundation::Mutex::ScopedLock lock(getMutex());
	const AllocNameMap::Entry* e = getMap().find(&other);
	PX_ASSERT(e);
	getMap()[this] = e->second;
	return *this;
}

void* NamedAllocator::allocate(size_t size, const char* filename, int line)
{
	if(!size)
		return 0;
	Foundation::Mutex::ScopedLock lock(getMutex());
	const AllocNameMap::Entry* e = getMap().find(this);
	PX_ASSERT(e);
	return getAllocator().allocate(size, e->second, filename, line);
}

void NamedAllocator::deallocate(void* ptr)
{
	if(ptr)
		getAllocator().deallocate(ptr);
}

#endif // PX_DEBUG

void* Allocator::allocate(size_t size, const char* file, int line)
{
	if(!size)
		return 0;
	return getAllocator().allocate(size, "", file, line);
}
void Allocator::deallocate(void* ptr)
{
	if(ptr)
		getAllocator().deallocate(ptr);
}

} // namespace shdfnd
} // namespace physx
