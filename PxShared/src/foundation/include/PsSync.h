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

#ifndef PSFOUNDATION_PSSYNC_H
#define PSFOUNDATION_PSSYNC_H

#include "PsAllocator.h"

namespace physx
{
namespace shdfnd
{
/*!
Implementation notes:
* - Calling set() on an already signaled Sync does not change its state.
* - Calling reset() on an already reset Sync does not change its state.
* - Calling set() on a reset Sync wakes all waiting threads (potential for thread contention).
* - Calling wait() on an already signaled Sync will return true immediately.
* - NOTE: be careful when pulsing an event with set() followed by reset(), because a
*   thread that is not waiting on the event will miss the signal.
*/
class PX_FOUNDATION_API SyncImpl
{
  public:
	static const uint32_t waitForever = 0xffffffff;

	SyncImpl();

	~SyncImpl();

	/** Wait on the object for at most the given number of ms. Returns
	*  true if the object is signaled. Sync::waitForever will block forever
	*  or until the object is signaled.
	*/

	bool wait(uint32_t milliseconds = waitForever);

	/** Signal the synchronization object, waking all threads waiting on it */

	void set();

	/** Reset the synchronization object */

	void reset();

	/**
	Size of this class.
	*/
	static uint32_t getSize();
};

/*!
Implementation notes:
* - Calling set() on an already signaled Sync does not change its state.
* - Calling reset() on an already reset Sync does not change its state.
* - Calling set() on a reset Sync wakes all waiting threads (potential for thread contention).
* - Calling wait() on an already signaled Sync will return true immediately.
* - NOTE: be careful when pulsing an event with set() followed by reset(), because a
*   thread that is not waiting on the event will miss the signal.
*/
template <typename Alloc = ReflectionAllocator<SyncImpl> >
class SyncT : protected Alloc
{
  public:
	static const uint32_t waitForever = SyncImpl::waitForever;

	SyncT(const Alloc& alloc = Alloc()) : Alloc(alloc)
	{
		mImpl = reinterpret_cast<SyncImpl*>(Alloc::allocate(SyncImpl::getSize(), __FILE__, __LINE__));
		PX_PLACEMENT_NEW(mImpl, SyncImpl)();
	}

	~SyncT()
	{
		mImpl->~SyncImpl();
		Alloc::deallocate(mImpl);
	}

	/** Wait on the object for at most the given number of ms. Returns
	*  true if the object is signaled. Sync::waitForever will block forever
	*  or until the object is signaled.
	*/

	bool wait(uint32_t milliseconds = SyncImpl::waitForever)
	{
		return mImpl->wait(milliseconds);
	}

	/** Signal the synchronization object, waking all threads waiting on it */

	void set()
	{
		mImpl->set();
	}

	/** Reset the synchronization object */

	void reset()
	{
		mImpl->reset();
	}

  private:
	class SyncImpl* mImpl;
};

typedef SyncT<> Sync;

} // namespace shdfnd
} // namespace physx

#endif // #ifndef PSFOUNDATION_PSSYNC_H
