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

#ifndef PSFOUNDATION_PSTHREAD_H
#define PSFOUNDATION_PSTHREAD_H

#include "PsUserAllocated.h"

// dsequeira: according to existing comment here (David Black would be my guess)
// "This is useful to reduce bus contention on tight spin locks. And it needs
// to be a macro as the xenon compiler often ignores even __forceinline." What's not
// clear is why a pause function needs inlining...? (TODO: check with XBox team)

// todo: these need to go somewhere else

#if PX_WINDOWS_FAMILY || PX_XBOXONE
#define PxSpinLockPause() __asm pause
#elif PX_LINUX || PX_ANDROID || PX_PS4 || PX_APPLE_FAMILY || PX_SWITCH
#define PxSpinLockPause() asm("nop")
#else
#error "Platform not supported!"
#endif

namespace physx
{
namespace shdfnd
{
struct ThreadPriority // todo: put in some other header file
{
	enum Enum
	{
		/**
	    \brief High priority
	    */
		eHIGH         = 0,

		/**
	    \brief Above Normal priority
	    */
		eABOVE_NORMAL = 1,

		/**
	    \brief Normal/default priority
	    */
		eNORMAL       = 2,

		/**
	    \brief Below Normal priority
	    */
		eBELOW_NORMAL = 3,

		/**
	    \brief Low priority.
	    */
		eLOW          = 4,
		eFORCE_DWORD  = 0xffFFffFF
	};
};

class Runnable
{
  public:
	Runnable()
	{
	}
	virtual ~Runnable()
	{
	}
	virtual void execute(void)
	{
	}
};

class PX_FOUNDATION_API ThreadImpl
{
  public:
	typedef size_t Id; // space for a pointer or an integer
	typedef void* (*ExecuteFn)(void*);

	static uint32_t getDefaultStackSize();
	static Id getId();

	/**
	Construct (but do not start) the thread object. The OS thread object will not be created
	until start() is called. Executes in the context
	of the spawning thread.
	*/

	ThreadImpl();

	/**
	Construct and start the the thread, passing the given arg to the given fn. (pthread style)
	*/

	ThreadImpl(ExecuteFn fn, void* arg);

	/**
	Deallocate all resources associated with the thread. Should be called in the
	context of the spawning thread.
	*/

	~ThreadImpl();

	/**
	Create the OS thread and start it running. Called in the context of the spawning thread.
	If an affinity mask has previously been set then it will be applied after the
	thread has been created.
	*/

	void start(uint32_t stackSize, Runnable* r);

	/**
	Violently kill the current thread. Blunt instrument, not recommended since
	it can leave all kinds of things unreleased (stack, memory, mutexes...) Should
	be called in the context of the spawning thread.
	*/

	void kill();

	/**
	Stop the thread. Signals the spawned thread that it should stop, so the
	thread should check regularly
	*/

	void signalQuit();

	/**
	Wait for a thread to stop. Should be called in the context of the spawning
	thread. Returns false if the thread has not been started.
	*/

	bool waitForQuit();

	/**
	check whether the thread is signalled to quit. Called in the context of the
	spawned thread.
	*/

	bool quitIsSignalled();

	/**
	Cleanly shut down this thread. Called in the context of the spawned thread.
	*/
	void quit();

	/**
	Change the affinity mask for this thread. The mask is a platform
	specific value.

	On Windows, Linux, PS4, XboxOne and Switch platforms, each set mask bit represents
	the index of a logical processor that the OS may schedule thread execution on.
	Bits outside the range of valid logical processors may be ignored or cause
	the function to return an error.

	On Apple platforms, this function has no effect.

	If the thread has not yet been started then the mask is stored
	and applied when the thread is started.

	If the thread has already been started then this method	returns the
	previous affinity mask on success, otherwise it returns zero.
	*/
	uint32_t setAffinityMask(uint32_t mask);

	static ThreadPriority::Enum getPriority(Id threadId);

	/** Set thread priority. */
	void setPriority(ThreadPriority::Enum prio);

	/** set the thread's name */
	void setName(const char* name);

	/** Put the current thread to sleep for the given number of milliseconds */
	static void sleep(uint32_t ms);

	/** Yield the current thread's slot on the CPU */
	static void yield();

	/** Return the number of physical cores (does not include hyper-threaded cores), returns 0 on failure */
	static uint32_t getNbPhysicalCores();

	/**
	Size of this class.
	*/
	static uint32_t getSize();
};

/**
Thread abstraction API
*/
template <typename Alloc = ReflectionAllocator<ThreadImpl> >
class ThreadT : protected Alloc, public UserAllocated, public Runnable
{
  public:
	typedef ThreadImpl::Id Id; // space for a pointer or an integer

	/**
	Construct (but do not start) the thread object. Executes in the context
	of the spawning thread
	*/
	ThreadT(const Alloc& alloc = Alloc()) : Alloc(alloc)
	{
		mImpl = reinterpret_cast<ThreadImpl*>(Alloc::allocate(ThreadImpl::getSize(), __FILE__, __LINE__));
		PX_PLACEMENT_NEW(mImpl, ThreadImpl)();
	}

	/**
	Construct and start the the thread, passing the given arg to the given fn. (pthread style)
	*/
	ThreadT(ThreadImpl::ExecuteFn fn, void* arg, const Alloc& alloc = Alloc()) : Alloc(alloc)
	{
		mImpl = reinterpret_cast<ThreadImpl*>(Alloc::allocate(ThreadImpl::getSize(), __FILE__, __LINE__));
		PX_PLACEMENT_NEW(mImpl, ThreadImpl)(fn, arg);
	}

	/**
	Deallocate all resources associated with the thread. Should be called in the
	context of the spawning thread.
	*/
	virtual ~ThreadT()
	{
		mImpl->~ThreadImpl();
		Alloc::deallocate(mImpl);
	}

	/**
	start the thread running. Called in the context of the spawning thread.
	*/

	void start(uint32_t stackSize = ThreadImpl::getDefaultStackSize())
	{
		mImpl->start(stackSize, this);
	}

	/**
	Violently kill the current thread. Blunt instrument, not recommended since
	it can leave all kinds of things unreleased (stack, memory, mutexes...) Should
	be called in the context of the spawning thread.
	*/

	void kill()
	{
		mImpl->kill();
	}

	/**
	The virtual execute() method is the user defined function that will
	run in the new thread. Called in the context of the spawned thread.
	*/

	virtual void execute(void)
	{
	}

	/**
	stop the thread. Signals the spawned thread that it should stop, so the
	thread should check regularly
	*/

	void signalQuit()
	{
		mImpl->signalQuit();
	}

	/**
	Wait for a thread to stop. Should be called in the context of the spawning
	thread. Returns false if the thread has not been started.
	*/

	bool waitForQuit()
	{
		return mImpl->waitForQuit();
	}

	/**
	check whether the thread is signalled to quit. Called in the context of the
	spawned thread.
	*/

	bool quitIsSignalled()
	{
		return mImpl->quitIsSignalled();
	}

	/**
	Cleanly shut down this thread. Called in the context of the spawned thread.
	*/
	void quit()
	{
		mImpl->quit();
	}

	uint32_t setAffinityMask(uint32_t mask)
	{
		return mImpl->setAffinityMask(mask);
	}

	static ThreadPriority::Enum getPriority(ThreadImpl::Id threadId)
	{
		return ThreadImpl::getPriority(threadId);
	}

	/** Set thread priority. */
	void setPriority(ThreadPriority::Enum prio)
	{
		mImpl->setPriority(prio);
	}

	/** set the thread's name */
	void setName(const char* name)
	{
		mImpl->setName(name);
	}

	/** Put the current thread to sleep for the given number of milliseconds */
	static void sleep(uint32_t ms)
	{
		ThreadImpl::sleep(ms);
	}

	/** Yield the current thread's slot on the CPU */
	static void yield()
	{
		ThreadImpl::yield();
	}

	static uint32_t getDefaultStackSize()
	{
		return ThreadImpl::getDefaultStackSize();
	}

	static ThreadImpl::Id getId()
	{
		return ThreadImpl::getId();
	}

	static uint32_t getNbPhysicalCores()
	{
		return ThreadImpl::getNbPhysicalCores();
	}

  private:
	class ThreadImpl* mImpl;
};

typedef ThreadT<> Thread;

PX_FOUNDATION_API uint32_t TlsAlloc();
PX_FOUNDATION_API void TlsFree(uint32_t index);
PX_FOUNDATION_API void* TlsGet(uint32_t index);
PX_FOUNDATION_API uint32_t TlsSet(uint32_t index, void* value);

} // namespace shdfnd
} // namespace physx

#endif // #ifndef PSFOUNDATION_PSTHREAD_H
